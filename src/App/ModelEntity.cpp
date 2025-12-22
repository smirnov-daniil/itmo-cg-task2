#include "ModelEntity.h"
#include "Camera.h"
#include <QFile>
#include <QImage>
#include <QOpenGLFunctions>
#include <tinygltf/tiny_gltf.h>

ModelEntity::ModelEntity(const std::string & name)
	: Entity(name)
{
}

ModelEntity::~ModelEntity()
{
	cleanupResources();
}

bool ModelEntity::loadFromGLTF(const QString & filePath)
{
	QFile modelFile(filePath);
	if (!modelFile.open(QIODevice::ReadOnly))
	{
		return false;
	}

	QByteArray modelData = modelFile.readAll();
	modelFile.close();

	if (modelData.isEmpty())
	{
		return false;
	}

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err, warn;

	bool success = loader.LoadBinaryFromMemory(&model, &err, &warn,
											   reinterpret_cast<const unsigned char *>(modelData.data()),
											   static_cast<unsigned int>(modelData.size()));

	if (!success)
	{
		return false;
	}

	for (const auto & texture: model.textures)
	{
		if (texture.source >= 0 && texture.source < static_cast<int>(model.images.size()))
		{
			const auto & image = model.images[texture.source];

			QImage qimg;
			if (image.component == 3)
			{
				qimg = QImage(image.image.data(), image.width, image.height, QImage::Format_RGB888);
			}
			else if (image.component == 4)
			{
				qimg = QImage(image.image.data(), image.width, image.height, QImage::Format_RGBA8888);
			}

			auto tex = std::make_unique<QOpenGLTexture>(qimg);
			tex->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);
			tex->setWrapMode(QOpenGLTexture::Repeat);
			tex->generateMipMaps();

			textures_.push_back(std::move(tex));
		}
	}

	for (const auto & mesh: model.meshes)
	{
		for (const auto & primitive: mesh.primitives)
		{
			Mesh meshData;

			if (primitive.attributes.find("POSITION") != primitive.attributes.end())
			{
				const auto & accessor = model.accessors[primitive.attributes.at("POSITION")];
				const auto & bufferView = model.bufferViews[accessor.bufferView];
				const auto & buffer = model.buffers[bufferView.buffer];

				const float * positions = reinterpret_cast<const float *>(
					&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

				meshData.vertices.resize(accessor.count);
				for (size_t i = 0; i < accessor.count; ++i)
				{
					meshData.vertices[i].position[0] = positions[i * 3 + 0];
					meshData.vertices[i].position[1] = positions[i * 3 + 1];
					meshData.vertices[i].position[2] = positions[i * 3 + 2];
				}
			}

			if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
			{
				const auto & accessor = model.accessors[primitive.attributes.at("NORMAL")];
				const auto & bufferView = model.bufferViews[accessor.bufferView];
				const auto & buffer = model.buffers[bufferView.buffer];

				const float * normals = reinterpret_cast<const float *>(
					&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

				for (size_t i = 0; i < accessor.count && i < meshData.vertices.size(); ++i)
				{
					meshData.vertices[i].normal[0] = normals[i * 3 + 0];
					meshData.vertices[i].normal[1] = normals[i * 3 + 1];
					meshData.vertices[i].normal[2] = normals[i * 3 + 2];
				}
			}

			if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
			{
				const auto & accessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
				const auto & bufferView = model.bufferViews[accessor.bufferView];
				const auto & buffer = model.buffers[bufferView.buffer];

				const float * texCoords = reinterpret_cast<const float *>(
					&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

				for (size_t i = 0; i < accessor.count && i < meshData.vertices.size(); ++i)
				{
					meshData.vertices[i].texCoord[0] = texCoords[i * 2 + 0];
					meshData.vertices[i].texCoord[1] = texCoords[i * 2 + 1];
				}
			}

			if (primitive.indices >= 0)
			{
				const auto & accessor = model.accessors[primitive.indices];
				const auto & bufferView = model.bufferViews[accessor.bufferView];
				const auto & buffer = model.buffers[bufferView.buffer];

				meshData.indices.resize(accessor.count);

				if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
				{
					const uint16_t * indices = reinterpret_cast<const uint16_t *>(
						&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
					for (size_t i = 0; i < accessor.count; ++i)
					{
						meshData.indices[i] = indices[i];
					}
				}
				else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
				{
					const uint32_t * indices = reinterpret_cast<const uint32_t *>(
						&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
					for (size_t i = 0; i < accessor.count; ++i)
					{
						meshData.indices[i] = indices[i];
					}
				}
			}

			if (primitive.material >= 0 && primitive.material < static_cast<int>(model.materials.size()))
			{
				const auto & material = model.materials[primitive.material];
				if (material.pbrMetallicRoughness.baseColorTexture.index >= 0)
				{
					meshData.textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
				}
			}

			meshes_.push_back(std::move(meshData));
		}
	}

	setupMeshBuffers();
	return true;
}

void ModelEntity::setShaderProgram(std::shared_ptr<QOpenGLShaderProgram> program)
{
	shaderProgram_ = program;

	if (program)
	{
		program->bind();
		mvpUniform_ = program->uniformLocation("mvp");
		modelUniform_ = program->uniformLocation("model");
		normalMatrixUniform_ = program->uniformLocation("normalMatrix");
		lightPosUniform_ = program->uniformLocation("lightPos");
		viewPosUniform_ = program->uniformLocation("viewPos");
		lightColorUniform_ = program->uniformLocation("lightColor");
		program->release();
	}
}

void ModelEntity::render(Camera * camera, const QVector3D & lightPos, const QVector3D & lightColor, OpenGLContextPtr context)
{
	if (!shaderProgram_ || !camera || !context || meshes_.empty())
		return;

	shaderProgram_->bind();

	const auto & transform = getTransform();
	const auto mvp = camera->getViewProjectionMatrix() * transform;
	const auto normalMatrix = transform.normalMatrix();

	if (mvpUniform_ >= 0)
		shaderProgram_->setUniformValue(mvpUniform_, mvp);
	if (modelUniform_ >= 0)
		shaderProgram_->setUniformValue(modelUniform_, transform);
	if (normalMatrixUniform_ >= 0)
		shaderProgram_->setUniformValue(normalMatrixUniform_, normalMatrix);
	if (lightPosUniform_ >= 0)
		shaderProgram_->setUniformValue(lightPosUniform_, lightPos);
	if (viewPosUniform_ >= 0)
		shaderProgram_->setUniformValue(viewPosUniform_, camera->getPosition());
	if (lightColorUniform_ >= 0)
		shaderProgram_->setUniformValue(lightColorUniform_, lightColor);

	for (size_t i = 0; i < meshes_.size(); ++i)
	{
		const auto & mesh = meshes_[i];

		if (i < vaos_.size() && vaos_[i])
		{
			vaos_[i]->bind();

			if (mesh.textureIndex >= 0 && mesh.textureIndex < static_cast<int>(textures_.size()))
			{
				textures_[mesh.textureIndex]->bind(0);
			}

			context->functions()->glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, nullptr);

			if (mesh.textureIndex >= 0 && mesh.textureIndex < static_cast<int>(textures_.size()))
			{
				textures_[mesh.textureIndex]->release();
			}

			vaos_[i]->release();
		}
	}

	shaderProgram_->release();
}

void ModelEntity::setupMeshBuffers()
{
	if (!shaderProgram_)
		return;

	for (const auto & mesh: meshes_)
	{
		auto vao = std::make_unique<QOpenGLVertexArrayObject>();
		vao->create();
		vao->bind();

		auto vbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::Type::VertexBuffer);
		vbo->create();
		vbo->bind();
		vbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
		vbo->allocate(mesh.vertices.data(), static_cast<int>(mesh.vertices.size() * sizeof(Vertex)));

		auto ibo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::Type::IndexBuffer);
		ibo->create();
		ibo->bind();
		ibo->setUsagePattern(QOpenGLBuffer::StaticDraw);
		ibo->allocate(mesh.indices.data(), static_cast<int>(mesh.indices.size() * sizeof(uint32_t)));

		shaderProgram_->bind();

		shaderProgram_->enableAttributeArray(0);
		shaderProgram_->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));

		shaderProgram_->enableAttributeArray(1);
		shaderProgram_->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, normal), 3, sizeof(Vertex));

		shaderProgram_->enableAttributeArray(2);
		shaderProgram_->setAttributeBuffer(2, GL_FLOAT, offsetof(Vertex, texCoord), 2, sizeof(Vertex));

		shaderProgram_->release();
		vao->release();

		vaos_.push_back(std::move(vao));
		vbos_.push_back(std::move(vbo));
		ibos_.push_back(std::move(ibo));
	}
}

void ModelEntity::cleanupResources()
{
	vaos_.clear();
	vbos_.clear();
	ibos_.clear();
	textures_.clear();
	meshes_.clear();
}
