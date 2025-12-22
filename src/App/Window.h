#pragma once

#include "Camera.h"
#include "OpenGLContext.h"
#include "SceneGraph.h"
#include "SceneRenderer.h"
#include <Base/GLWidget.hpp>

#include <QElapsedTimer>
#include <QSet>
#include <QTimer>

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

signals:
	void updateUI();

private:
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
