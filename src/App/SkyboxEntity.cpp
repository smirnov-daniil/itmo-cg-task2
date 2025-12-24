#include "SkyboxEntity.h"
#include "Camera.h"
#include <QImage>
#include <QOpenGLFunctions>

SkyboxEntity::SkyboxEntity(const std::string & name)
	: Entity(name)
{
}

SkyboxEntity::~SkyboxEntity()
{
	cleanupResources();
}

bool SkyboxEntity::loadCubemap(const QStringList & faces)
{
	if (faces.size() != 6)
	{
		return false;
	}

	texture_ = std::make_unique<QOpenGLTexture>(QOpenGLTexture::TargetCubeMap);
	texture_->create();
	texture_->setSize(1024, 1024);
	texture_->setFormat(QOpenGLTexture::RGBA8_UNorm);
	texture_->allocateStorage();

	for (int i = 0; i < faces.size(); ++i)
	{
		QImage image(faces[i]);
		if (image.isNull())
		{
			image = QImage(512, 512, QImage::Format_RGBA8888);
			QColor colors[] = {Qt::red, Qt::green, Qt::blue, Qt::yellow, Qt::magenta, Qt::cyan};
			image.fill(colors[i]);
		}
		else
		{
			image = image.convertToFormat(QImage::Format_RGBA8888);
		}

		texture_->setData(0, 0, static_cast<QOpenGLTexture::CubeMapFace>(QOpenGLTexture::CubeMapPositiveX + i),
						  QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, image.constBits());
	}

	texture_->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
	texture_->setWrapMode(QOpenGLTexture::ClampToEdge);

	initializeGeometry();
	return true;
}

void SkyboxEntity::setShaderProgram(std::shared_ptr<QOpenGLShaderProgram> program)
{
	shaderProgram_ = program;

	if (program)
	{
		program->bind();
		viewUniform_ = program->uniformLocation("view");
		projectionUniform_ = program->uniformLocation("projection");
		textureUniform_ = program->uniformLocation("skybox");
		program->release();
	}
}

void SkyboxEntity::render(Camera * camera, OpenGLContextPtr context)
{
	if (!camera || !shaderProgram_ || !texture_ || !initialized_ || !context)
		return;

	GLboolean depthMask;
	context->functions()->glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);

	context->functions()->glDepthMask(GL_FALSE);
	context->functions()->glDepthFunc(GL_LEQUAL);
	context->functions()->glDisable(GL_CULL_FACE);

	shaderProgram_->bind();
	vao_.bind();

	if (viewUniform_ >= 0)
		shaderProgram_->setUniformValue(viewUniform_, camera->getViewMatrix());
	if (projectionUniform_ >= 0)
		shaderProgram_->setUniformValue(projectionUniform_, camera->getProjectionMatrix());

	texture_->bind(0);
	if (textureUniform_ >= 0)
	{
		shaderProgram_->setUniformValue(textureUniform_, 0);
	}

	context->functions()->glDrawArrays(GL_TRIANGLES, 0, 36);

	texture_->release();
	vao_.release();
	shaderProgram_->release();

	context->functions()->glDepthMask(depthMask);
	context->functions()->glDepthFunc(GL_LESS);
	context->functions()->glEnable(GL_CULL_FACE);
}

void SkyboxEntity::initializeGeometry()
{
	if (!shaderProgram_)
		return;

	static const float skyboxVertices[] = {
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f};

	vao_.create();
	vao_.bind();

	vbo_.create();
	vbo_.bind();
	vbo_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	vbo_.allocate(skyboxVertices, sizeof(skyboxVertices));

	shaderProgram_->bind();
	shaderProgram_->enableAttributeArray(0);
	shaderProgram_->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(float));
	shaderProgram_->release();

	vao_.release();

	initialized_ = true;
}

void SkyboxEntity::cleanupResources()
{
	vao_.destroy();
	vbo_.destroy();
	texture_.reset();
	initialized_ = false;
}
