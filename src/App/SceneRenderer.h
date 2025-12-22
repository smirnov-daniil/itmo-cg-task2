#pragma once

#include "OpenGLContext.h"
#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <memory>
#include <vector>

class Camera;
class SceneGraph;
class ModelEntity;
class SkyboxEntity;

struct RenderBatch {
	enum Type
	{
		MODEL,
		SKYBOX
	};

	Type type;
	void * entity;
	float distance;
};

class SceneRenderer
{
public:
	SceneRenderer(OpenGLContextPtr context = nullptr);
	~SceneRenderer() = default;

	void setContext(OpenGLContextPtr context);
	OpenGLContextPtr getContext() const { return context_; }

	bool initialize();
	void cleanup();

	void renderScene(SceneGraph * scene, Camera * camera);


	void setLightPosition(const QVector3D & position) { lightPosition_ = position; }
	void setLightColor(const QVector3D & color) { lightColor_ = color; }

	const QVector3D & getLightPosition() const { return lightPosition_; }
	const QVector3D & getLightColor() const { return lightColor_; }

	std::shared_ptr<QOpenGLShaderProgram> getModelShader() const { return modelShader_; }
	std::shared_ptr<QOpenGLShaderProgram> getSkyboxShader() const { return skyboxShader_; }

	size_t getLastFrameBatchCount() const { return lastFrameBatchCount_; }
	size_t getLastFrameTriangleCount() const { return lastFrameTriangleCount_; }

private:
	void collectRenderBatches(SceneGraph * scene, Camera * camera);

	void sortBatches(Camera * camera);

	void renderBatches(Camera * camera);

	bool createShaders();

	OpenGLContextPtr context_;

	std::shared_ptr<QOpenGLShaderProgram> modelShader_;
	std::shared_ptr<QOpenGLShaderProgram> skyboxShader_;

	QVector3D lightPosition_{0.0f, 10.0f, 0.0f};
	QVector3D lightColor_{1.0f, 1.0f, 1.0f};

	std::vector<RenderBatch> renderBatches_;

	size_t lastFrameBatchCount_ = 0;
	size_t lastFrameTriangleCount_ = 0;

	bool initialized_ = false;
};
