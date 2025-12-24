#pragma once

#include "Entity.h"
#include "OpenGLContext.h"
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <memory>

class Camera;

class SkyboxEntity : public Entity
{
public:
	SkyboxEntity(const std::string & name = "Skybox");
	~SkyboxEntity() override;

	bool loadCubemap(const QStringList & faces);

	void setShaderProgram(std::shared_ptr<QOpenGLShaderProgram> program);

	void render(Camera * camera, OpenGLContextPtr context) override;

	QOpenGLTexture * getTexture() const { return texture_.get(); }

	bool isLoaded() const { return texture_ != nullptr; }

private:
	void initializeGeometry();
	void cleanupResources();

	std::shared_ptr<QOpenGLShaderProgram> shaderProgram_;
	std::unique_ptr<QOpenGLTexture> texture_;

	QOpenGLBuffer vbo_{QOpenGLBuffer::Type::VertexBuffer};
	QOpenGLVertexArrayObject vao_;

	GLint viewUniform_ = -1;
	GLint projectionUniform_ = -1;
	GLint textureUniform_ = -1;

	bool initialized_ = false;
};
