#include "SceneRenderer.h"
#include "Camera.h"
#include "Entity.h"
#include "ModelEntity.h"
#include "SceneGraph.h"
#include "SkyboxEntity.h"
#include <algorithm>
#include <cmath>

SceneRenderer::SceneRenderer(OpenGLContextPtr context)
	: context_(context)
{
}

void SceneRenderer::setContext(OpenGLContextPtr context)
{
	context_ = context;
}

bool SceneRenderer::initialize()
{
	if (initialized_)
		return true;

	if (!context_ || !context_->isValid())
	{
		return false;
	}

	if (!createShaders())
	{
		return false;
	}

	context_->functions()->glEnable(GL_DEPTH_TEST);
	context_->functions()->glEnable(GL_CULL_FACE);
	context_->functions()->glCullFace(GL_BACK);
	context_->functions()->glFrontFace(GL_CCW);

	initialized_ = true;
	return true;
}

void SceneRenderer::cleanup()
{
	modelShader_.reset();
	skyboxShader_.reset();
	renderBatches_.clear();
	initialized_ = false;
}

void SceneRenderer::setupLightUniforms(QOpenGLShaderProgram * shader)
{
	if (!shader)
		return;

	shader->bind();

	shader->setUniformValue("dirLightDirection", directionalLight_.direction);
	shader->setUniformValue("dirLightColor", directionalLight_.color);
	shader->setUniformValue("dirLightIntensity", directionalLight_.intensity);
	shader->setUniformValue("dirLightEnabled", directionalLight_.enabled);

	shader->setUniformValue("spotLightPosition", spotLight_.position);
	shader->setUniformValue("spotLightDirection", spotLight_.direction);
	shader->setUniformValue("spotLightColor", spotLight_.color);
	shader->setUniformValue("spotLightIntensity", spotLight_.intensity);
	shader->setUniformValue("spotLightCutOff", spotLight_.cutOff);
	shader->setUniformValue("spotLightOuterCutOff", spotLight_.outerCutOff);
	shader->setUniformValue("spotLightEnabled", spotLight_.enabled);

	shader->release();
}

void SceneRenderer::renderScene(SceneGraph * scene, Camera * camera)
{
	if (!initialized_ || !scene || !camera || !context_)
		return;

	context_->functions()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	collectRenderBatches(scene, camera);

	sortBatches(camera);

	renderBatches(camera);

	lastFrameBatchCount_ = renderBatches_.size();
}

void SceneRenderer::collectRenderBatches(SceneGraph * scene, Camera * camera)
{
	renderBatches_.clear();

	if (!scene->getRoot())
		return;

	const QVector3D cameraPos = camera->getPosition();

	scene->getRoot()->traverseVisible([this, &cameraPos](SceneNode * node) {
		auto entity = node->getEntity();
		if (!entity || !entity->isVisible())
			return;

		float distance = (entity->getPosition() - cameraPos).length();

		if (auto modelEntity = std::dynamic_pointer_cast<ModelEntity>(entity))
		{
			if (modelEntity->isLoaded())
			{
				RenderBatch batch;
				batch.type = RenderBatch::MODEL;
				batch.entity = modelEntity.get();
				batch.distance = distance;
				renderBatches_.push_back(batch);
			}
		}
		else if (auto skyboxEntity = std::dynamic_pointer_cast<SkyboxEntity>(entity))
		{
			if (skyboxEntity->isLoaded())
			{
				RenderBatch batch;
				batch.type = RenderBatch::SKYBOX;
				batch.entity = skyboxEntity.get();
				batch.distance = distance;
				renderBatches_.push_back(batch);
			}
		}
	});
}

void SceneRenderer::sortBatches(Camera * /*camera*/)
{
	std::sort(renderBatches_.begin(), renderBatches_.end(),
			  [](const RenderBatch & a, const RenderBatch & b) {
				  if (a.type == RenderBatch::SKYBOX && b.type != RenderBatch::SKYBOX)
					  return true;
				  if (a.type != RenderBatch::SKYBOX && b.type == RenderBatch::SKYBOX)
					  return false;

				  return a.distance < b.distance;
			  });
}

void SceneRenderer::renderBatches(Camera * camera)
{
	lastFrameTriangleCount_ = 0;

	SkyboxEntity * skyboxEntity = nullptr;
	for (const auto & batch: renderBatches_)
	{
		if (batch.type == RenderBatch::SKYBOX)
		{
			skyboxEntity = static_cast<SkyboxEntity *>(batch.entity);
			break;
		}
	}

	if (modelShader_)
	{
		setupLightUniforms(modelShader_.get());
	}

	for (const auto & batch: renderBatches_)
	{
		switch (batch.type)
		{
			case RenderBatch::SKYBOX: {
				auto skybox = static_cast<SkyboxEntity *>(batch.entity);
				skybox->render(camera, context_);
				break;
			}

			case RenderBatch::MODEL: {
				auto modelEntity = static_cast<ModelEntity *>(batch.entity);

				if (skyboxEntity && skyboxEntity->getTexture())
				{
					skyboxEntity->getTexture()->bind(1);
				}

				modelEntity->render(camera, context_);

				if (skyboxEntity && skyboxEntity->getTexture())
				{
					skyboxEntity->getTexture()->release();
				}

				for (const auto & mesh: modelEntity->getMeshes())
				{
					lastFrameTriangleCount_ += mesh.indices.size() / 3;
				}
				break;
			}
		}
	}
}

bool SceneRenderer::createShaders()
{
	modelShader_ = std::make_shared<QOpenGLShaderProgram>();
	if (!modelShader_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/model.vs"))
	{
		return false;
	}

	if (!modelShader_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/model.fs"))
	{
		return false;
	}

	if (!modelShader_->link())
	{
		return false;
	}

	modelShader_->bind();
	modelShader_->setUniformValue("diffuseTexture", 0);// GL_TEXTURE0
	modelShader_->setUniformValue("skybox", 1);        // GL_TEXTURE1

	setupLightUniforms(modelShader_.get());

	modelShader_->release();

	skyboxShader_ = std::make_shared<QOpenGLShaderProgram>();
	if (!skyboxShader_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/skybox.vs"))
	{
		return false;
	}

	if (!skyboxShader_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/skybox.fs"))
	{
		return false;
	}

	if (!skyboxShader_->link())
	{
		return false;
	}

	return true;
}