#include "Window.h"
#include "ModelEntity.h"
#include "SkyboxEntity.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QScreen>
#include <QVBoxLayout>

#include <cmath>

Window::Window() noexcept
{
	const auto formatFPS = [](const auto value) {
		return QString("FPS: %1").arg(QString::number(value));
	};

	auto fps = new QLabel(formatFPS(0), this);
	fps->setStyleSheet("QLabel { color : white; }");

	auto layout = new QVBoxLayout();
	layout->addWidget(fps, 1);

	setLayout(layout);

	timer_.start();
	deltaTimer_.start();

	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);

	inputTimer_ = new QTimer(this);
	connect(inputTimer_, &QTimer::timeout, this, &Window::processInput);
	inputTimer_->start(16);

	connect(this, &Window::updateUI, [=, this] {
		fps->setText(formatFPS(ui_.fps));
	});
}

Window::~Window()
{
	const auto guard = bindContext();

	sceneGraph_.reset();
	renderer_.reset();
	camera_.reset();
}

void Window::onInit()
{
	openglContext_ = std::make_shared<OpenGLContext>(QOpenGLContext::currentContext()->functions());

	camera_ = std::make_unique<Camera>();
	camera_->setPosition(QVector3D(0.0f, 2.0f, 0.0f));
	camera_->setYaw(0.0f);
	camera_->setPitch(-30.0f);
	camera_->setMoveSpeed(25.0f);
	camera_->setMouseSensitivity(0.1f);

	sceneGraph_ = std::make_unique<SceneGraph>();

	renderer_ = std::make_unique<SceneRenderer>(openglContext_);
	if (!renderer_->initialize())
	{
		return;
	}

	if (!initializeScene())
	{
		return;
	}

	openglContext_->functions()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::onRender()
{
	const auto guard = captureMetrics();

	float deltaTime = 0.016f;
	camera_->update(deltaTime);

	sceneGraph_->update(deltaTime);

	renderer_->renderScene(sceneGraph_.get(), camera_.get());

	++frameCount_;

	update();
}

void Window::onResize(const size_t width, const size_t height)
{
	if (openglContext_)
	{
		openglContext_->functions()->glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height));
	}

	if (camera_)
	{
		const auto aspect = static_cast<float>(width) / static_cast<float>(height);
		const auto zNear = 0.1f;
		const auto zFar = 100.0f;
		const auto fov = 60.0f;
		camera_->setPerspective(fov, aspect, zNear, zFar);
	}
}

Window::PerfomanceMetricsGuard::PerfomanceMetricsGuard(std::function<void()> callback)
	: callback_{std::move(callback)}
{
}

Window::PerfomanceMetricsGuard::~PerfomanceMetricsGuard()
{
	if (callback_)
	{
		callback_();
	}
}

auto Window::captureMetrics() -> PerfomanceMetricsGuard
{
	return PerfomanceMetricsGuard{
		[&] {
			if (timer_.elapsed() >= 1000)
			{
				const auto elapsedSeconds = static_cast<float>(timer_.restart()) / 1000.0f;
				ui_.fps = static_cast<size_t>(std::round(frameCount_ / elapsedSeconds));
				frameCount_ = 0;
				emit updateUI();
			}
		}};
}

bool Window::initializeScene()
{
	auto sponzaModel = std::make_shared<ModelEntity>("Sponza");
	sponzaModel->setShaderProgram(renderer_->getModelShader());

	if (!sponzaModel->loadFromGLTF(":/Models/sponza.glb"))
	{
		return false;
	}

	sponzaModel->setScale(QVector3D(0.01f, 0.01f, 0.01f));
	sponzaModel->setPosition(QVector3D(0.0f, 0.0f, 0.0f));

	auto skybox = std::make_shared<SkyboxEntity>("Skybox");
	skybox->setShaderProgram(renderer_->getSkyboxShader());

	QStringList skyboxFaces;
	skyboxFaces << ":/Textures/sky-cube/px.png"
				<< ":/Textures/sky-cube/nx.png"
				<< ":/Textures/sky-cube/py.png"
				<< ":/Textures/sky-cube/ny.png"
				<< ":/Textures/sky-cube/pz.png"
				<< ":/Textures/sky-cube/nz.png";

	if (!skybox->loadCubemap(skyboxFaces))
	{
		return false;
	}

	sceneGraph_->addEntity(skybox, "SkyboxNode");
	sceneGraph_->addEntity(sponzaModel, "SponzaNode");

	return true;
}

void Window::processInput()
{
	if (!camera_)
		return;

	float deltaTime = deltaTimer_.restart() / 1000.0f;

	deltaTime = qMin(deltaTime, 0.1f);

	camera_->processKeyboardInput(pressedKeys_, deltaTime);
}

void Window::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		lastMousePos_ = event->pos();
		firstMouse_ = true;
	}
	fgl::GLWidget::mousePressEvent(event);
}

void Window::mouseMoveEvent(QMouseEvent * event)
{
	if (!camera_)
		return;

	if (event->buttons() & Qt::LeftButton)
	{
		if (firstMouse_)
		{
			lastMousePos_ = event->pos();
			firstMouse_ = false;
		}

		float xOffset = event->pos().x() - lastMousePos_.x();
		float yOffset = lastMousePos_.y() - event->pos().y();

		lastMousePos_ = event->pos();

		camera_->processMouseMovement(xOffset, yOffset);
	}

	fgl::GLWidget::mouseMoveEvent(event);
}

void Window::keyPressEvent(QKeyEvent * event)
{
	pressedKeys_.insert(event->key());
	fgl::GLWidget::keyPressEvent(event);
}

void Window::keyReleaseEvent(QKeyEvent * event)
{
	pressedKeys_.remove(event->key());
	fgl::GLWidget::keyReleaseEvent(event);
}
