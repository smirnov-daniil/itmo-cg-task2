#pragma once

#include "OpenGLContext.h"
#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <memory>
#include <qmath.h>
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

struct DirectionalLight {
	QVector3D direction = QVector3D(-0.707f, -0.707f, 0.0f);
	QVector3D color = QVector3D(1.0f, 1.0f, 1.0f);
	float intensity = 1.0f;
	bool enabled = true;
};

struct SpotLight {
	QVector3D position = QVector3D(0.0f, 2.0f, -1.0f);
	QVector3D direction = QVector3D(0.0f, -0.707f, 0.707f);
	QVector3D color = QVector3D(1.0f, 1.0f, 1.0f);
	float intensity = 1.0f;
	float cutOff = qCos(qDegreesToRadians(12.5f));
	float outerCutOff = qCos(qDegreesToRadians(17.5f));
	bool enabled = true;
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

	void setDirectionalLight(const DirectionalLight & light) { directionalLight_ = light; }
	const DirectionalLight & getDirectionalLight() const { return directionalLight_; }

	void setSpotLight(const SpotLight & light) { spotLight_ = light; }
	const SpotLight & getSpotLight() const { return spotLight_; }

	std::shared_ptr<QOpenGLShaderProgram> getModelShader() const { return modelShader_; }
	std::shared_ptr<QOpenGLShaderProgram> getSkyboxShader() const { return skyboxShader_; }

	size_t getLastFrameBatchCount() const { return lastFrameBatchCount_; }
	size_t getLastFrameTriangleCount() const { return lastFrameTriangleCount_; }

private:
	void collectRenderBatches(SceneGraph * scene, Camera * camera);
	void sortBatches(Camera * camera);
	void renderBatches(Camera * camera);
	bool createShaders();
	void setupLightUniforms(QOpenGLShaderProgram * shader);

	OpenGLContextPtr context_;

	std::shared_ptr<QOpenGLShaderProgram> modelShader_;
	std::shared_ptr<QOpenGLShaderProgram> skyboxShader_;

	DirectionalLight directionalLight_;
	SpotLight spotLight_;

	std::vector<RenderBatch> renderBatches_;

	size_t lastFrameBatchCount_ = 0;
	size_t lastFrameTriangleCount_ = 0;

	bool initialized_ = false;
};