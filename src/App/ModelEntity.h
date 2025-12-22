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

	void render(Camera * camera, const QVector3D & lightPos, const QVector3D & lightColor, OpenGLContextPtr context) override;

	const std::vector<Mesh> & getMeshes() const { return meshes_; }
	bool isLoaded() const { return !meshes_.empty(); }

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
};
