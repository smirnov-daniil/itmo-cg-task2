#pragma once

#include "Camera.h"
#include "OpenGLContext.h"
#include "SceneGraph.h"
#include "SceneRenderer.h"
#include <Base/GLWidget.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QElapsedTimer>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSet>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>

#include <functional>
#include <memory>

class ModelEntity;
class SkyboxEntity;

class Window final : public fgl::GLWidget
{
	Q_OBJECT

public:
	Window() noexcept;
	~Window() override;

public:// fgl::GLWidget
	void onInit() override;
	void onRender() override;
	void onResize(size_t width, size_t height) override;

protected:
	void mousePressEvent(QMouseEvent * event) override;
	void mouseMoveEvent(QMouseEvent * event) override;
	void keyPressEvent(QKeyEvent * event) override;
	void keyReleaseEvent(QKeyEvent * event) override;

private:
	class PerfomanceMetricsGuard final
	{
	public:
		explicit PerfomanceMetricsGuard(std::function<void()> callback);
		~PerfomanceMetricsGuard();

		PerfomanceMetricsGuard(const PerfomanceMetricsGuard &) = delete;
		PerfomanceMetricsGuard(PerfomanceMetricsGuard &&) = delete;

		PerfomanceMetricsGuard & operator=(const PerfomanceMetricsGuard &) = delete;
		PerfomanceMetricsGuard & operator=(PerfomanceMetricsGuard &&) = delete;

	private:
		std::function<void()> callback_;
	};

private:
	[[nodiscard]] PerfomanceMetricsGuard captureMetrics();
	void processInput();
	bool initializeScene();
	void createLightControls();
	void updateLightParameters();

private slots:
	void onMorphSliderChanged(int value);
	void onRadiusSliderChanged(int value);
	void onEnableMorphChanged(int state);

	void onDirLightEnabledChanged(int state);
	void onDirLightIntensityChanged(int value);
	void onDirLightColorChanged();

	void onSpotLightEnabledChanged(int state);
	void onSpotLightIntensityChanged(int value);
	void onSpotLightCutOffChanged(int value);
	void onSpotLightOuterCutOffChanged(int value);
	void onSpotLightColorChanged();

signals:
	void updateUI();

private:
	QSlider * morphSlider_ = nullptr;
	QSlider * radiusSlider_ = nullptr;
	QCheckBox * enableMorphCheckbox_ = nullptr;
	QLabel * morphLabel_ = nullptr;
	QLabel * radiusLabel_ = nullptr;

	QGroupBox * dirLightGroup_ = nullptr;
	QCheckBox * dirLightEnabledCheckbox_ = nullptr;
	QSlider * dirLightIntensitySlider_ = nullptr;
	QLabel * dirLightIntensityLabel_ = nullptr;
	QComboBox * dirLightColorCombo_ = nullptr;

	QGroupBox * spotLightGroup_ = nullptr;
	QCheckBox * spotLightEnabledCheckbox_ = nullptr;
	QSlider * spotLightIntensitySlider_ = nullptr;
	QLabel * spotLightIntensityLabel_ = nullptr;
	QSlider * spotLightCutOffSlider_ = nullptr;
	QLabel * spotLightCutOffLabel_ = nullptr;
	QSlider * spotLightOuterCutOffSlider_ = nullptr;
	QLabel * spotLightOuterCutOffLabel_ = nullptr;
	QComboBox * spotLightColorCombo_ = nullptr;

	std::shared_ptr<ModelEntity> model_;

	OpenGLContextPtr openglContext_;
	std::unique_ptr<Camera> camera_;
	std::unique_ptr<SceneGraph> sceneGraph_;
	std::unique_ptr<SceneRenderer> renderer_;

	bool firstMouse_{true};
	QPoint lastMousePos_;

	QSet<int> pressedKeys_;

	QTimer * inputTimer_;
	QElapsedTimer deltaTimer_;

	QElapsedTimer timer_;
	size_t frameCount_ = 0;

	struct {
		size_t fps = 0;
	} ui_;
};