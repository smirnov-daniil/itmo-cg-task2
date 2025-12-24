#pragma once

#include "Entity.h"
#include "OpenGLContext.h"
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <memory>
#include <vector>

class Camera;

struct Vertex {
	float position[3];
	float normal[3];
	float texCoord[2];
};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	int textureIndex = -1;
};

class ModelEntity : public Entity
{
public:
	ModelEntity(const std::string & name = "Model");
	~ModelEntity() override;

	bool loadFromGLTF(const QString & filePath);

	void setShaderProgram(std::shared_ptr<QOpenGLShaderProgram> program);

	void render(Camera * camera, OpenGLContextPtr context) override;

	const std::vector<Mesh> & getMeshes() const { return meshes_; }
	bool isLoaded() const { return !meshes_.empty(); }

	void setMorphToSphere(bool enable) { morphToSphere_ = enable; }
	bool isMorphingToSphere() const { return morphToSphere_; }

	void setMorphFactor(float factor)
	{
		morphFactor_ = qBound(0.0f, factor, 1.0f);
	}
	float getMorphFactor() const { return morphFactor_; }

	void setSphereRadius(float radius) { sphereRadius_ = radius; }
	float getSphereRadius() const { return sphereRadius_; }

	void setMorphCenter(const QVector3D & center) { morphCenter_ = center; }
	QVector3D getMorphCenter() const { return morphCenter_; }

private:
	void setupMeshBuffers();
	void cleanupResources();

	std::shared_ptr<QOpenGLShaderProgram> shaderProgram_;
	std::vector<std::unique_ptr<QOpenGLTexture>> textures_;
	std::vector<Mesh> meshes_;

	std::vector<std::unique_ptr<QOpenGLBuffer>> vbos_;
	std::vector<std::unique_ptr<QOpenGLBuffer>> ibos_;
	std::vector<std::unique_ptr<QOpenGLVertexArrayObject>> vaos_;

	GLint mvpUniform_ = -1;
	GLint modelUniform_ = -1;
	GLint normalMatrixUniform_ = -1;
	GLint lightPosUniform_ = -1;
	GLint viewPosUniform_ = -1;
	GLint lightColorUniform_ = -1;

	bool morphToSphere_ = false;
	float morphFactor_ = 0.0f;
	float sphereRadius_ = 5.0f;
	QVector3D morphCenter_ = QVector3D(0.0f, 0.0f, 0.0f);

	GLint morphFactorUniform_ = -1;
	GLint morphToSphereUniform_ = -1;
	GLint sphereRadiusUniform_ = -1;
	GLint morphCenterUniform_ = -1;
};
