#include "Window.h"
#include "ModelEntity.h"
#include "SkyboxEntity.h"

#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QScreen>
#include <QScrollArea>
#include <QVBoxLayout>

#include <cmath>

Window::Window() noexcept
{
	const auto formatFPS = [](const auto value) {
		return QString("FPS: %1").arg(QString::number(value));
	};

	auto fps = new QLabel(formatFPS(0), this);
	fps->setStyleSheet("QLabel { color : white; }");

	auto mainLayout = new QVBoxLayout();
	mainLayout->addWidget(fps, 1);

	auto scrollArea = new QScrollArea(this);
	auto containerWidget = new QWidget();
	auto containerLayout = new QVBoxLayout(containerWidget);

	auto morphGroup = new QGroupBox("Morphing Settings", this);
	auto morphLayout = new QVBoxLayout(morphGroup);

	enableMorphCheckbox_ = new QCheckBox("Enable morphing", this);
	morphLabel_ = new QLabel("Morphing factor: 0.0", this);
	radiusLabel_ = new QLabel("Sphere radius: 1.0", this);

	morphSlider_ = new QSlider(Qt::Horizontal, this);
	morphSlider_->setRange(0, 100);
	morphSlider_->setValue(0);

	radiusSlider_ = new QSlider(Qt::Horizontal, this);
	radiusSlider_->setRange(1, 20);
	radiusSlider_->setValue(11);

	QString labelStyle = "QLabel { color : black; font-size: 12px; }";
	morphLabel_->setStyleSheet(labelStyle);
	radiusLabel_->setStyleSheet(labelStyle);
	enableMorphCheckbox_->setStyleSheet("QCheckBox { color : black; font-size: 12px; }");

	connect(morphSlider_, &QSlider::valueChanged, this, &Window::onMorphSliderChanged);
	connect(radiusSlider_, &QSlider::valueChanged, this, &Window::onRadiusSliderChanged);
	connect(enableMorphCheckbox_, &QCheckBox::stateChanged, this, &Window::onEnableMorphChanged);

	morphLayout->addWidget(enableMorphCheckbox_);
	morphLayout->addWidget(morphLabel_);
	morphLayout->addWidget(morphSlider_);
	morphLayout->addWidget(radiusLabel_);
	morphLayout->addWidget(radiusSlider_);

	createLightControls();

	containerLayout->addWidget(morphGroup);
	containerLayout->addWidget(dirLightGroup_);
	containerLayout->addWidget(spotLightGroup_);
	containerLayout->addStretch();

	scrollArea->setWidget(containerWidget);
	scrollArea->setWidgetResizable(true);
	scrollArea->setMinimumWidth(300);
	scrollArea->setMaximumWidth(400);

	auto horizontalLayout = new QHBoxLayout(this);
	horizontalLayout->addWidget(scrollArea, 1, Qt::AlignmentFlag::AlignLeft);

	setLayout(horizontalLayout);

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

void Window::createLightControls()
{
	dirLightGroup_ = new QGroupBox("Directional Light", this);
	auto dirLightLayout = new QVBoxLayout(dirLightGroup_);

	dirLightEnabledCheckbox_ = new QCheckBox("Enabled", this);
	dirLightEnabledCheckbox_->setChecked(true);

	dirLightIntensityLabel_ = new QLabel("Intensity: 1.0", this);
	dirLightIntensitySlider_ = new QSlider(Qt::Horizontal, this);
	dirLightIntensitySlider_->setRange(0, 200);
	dirLightIntensitySlider_->setValue(100);

	dirLightColorCombo_ = new QComboBox(this);
	dirLightColorCombo_->addItem("White", QColor(255, 255, 255));
	dirLightColorCombo_->addItem("Sunlight", QColor(255, 255, 200));
	dirLightColorCombo_->addItem("Moonlight", QColor(200, 220, 255));
	dirLightColorCombo_->addItem("Custom...", QColor());

	connect(dirLightEnabledCheckbox_, &QCheckBox::stateChanged, this, &Window::onDirLightEnabledChanged);
	connect(dirLightIntensitySlider_, &QSlider::valueChanged, this, &Window::onDirLightIntensityChanged);
	connect(dirLightColorCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Window::onDirLightColorChanged);

	dirLightLayout->addWidget(dirLightEnabledCheckbox_);
	dirLightLayout->addWidget(dirLightIntensityLabel_);
	dirLightLayout->addWidget(dirLightIntensitySlider_);
	dirLightLayout->addWidget(new QLabel("Color:", this));
	dirLightLayout->addWidget(dirLightColorCombo_);

	spotLightGroup_ = new QGroupBox("Spot Light", this);
	auto spotLightLayout = new QVBoxLayout(spotLightGroup_);

	spotLightEnabledCheckbox_ = new QCheckBox("Enabled", this);
	spotLightEnabledCheckbox_->setChecked(true);

	spotLightIntensityLabel_ = new QLabel("Intensity: 1.0", this);
	spotLightIntensitySlider_ = new QSlider(Qt::Horizontal, this);
	spotLightIntensitySlider_->setRange(0, 200);
	spotLightIntensitySlider_->setValue(100);

	spotLightCutOffLabel_ = new QLabel("Cut Off: 12.5", this);
	spotLightCutOffSlider_ = new QSlider(Qt::Horizontal, this);
	spotLightCutOffSlider_->setRange(1, 45);
	spotLightCutOffSlider_->setValue(12);

	spotLightOuterCutOffLabel_ = new QLabel("Outer Cut Off: 17.5", this);
	spotLightOuterCutOffSlider_ = new QSlider(Qt::Horizontal, this);
	spotLightOuterCutOffSlider_->setRange(1, 45);
	spotLightOuterCutOffSlider_->setValue(17);

	spotLightColorCombo_ = new QComboBox(this);
	spotLightColorCombo_->addItem("White", QColor(255, 255, 255));
	spotLightColorCombo_->addItem("Warm White", QColor(255, 240, 220));
	spotLightColorCombo_->addItem("Cool White", QColor(220, 240, 255));
	spotLightColorCombo_->addItem("Custom...", QColor());

	connect(spotLightEnabledCheckbox_, &QCheckBox::stateChanged, this, &Window::onSpotLightEnabledChanged);
	connect(spotLightIntensitySlider_, &QSlider::valueChanged, this, &Window::onSpotLightIntensityChanged);
	connect(spotLightCutOffSlider_, &QSlider::valueChanged, this, &Window::onSpotLightCutOffChanged);
	connect(spotLightOuterCutOffSlider_, &QSlider::valueChanged, this, &Window::onSpotLightOuterCutOffChanged);
	connect(spotLightColorCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Window::onSpotLightColorChanged);

	spotLightLayout->addWidget(spotLightEnabledCheckbox_);
	spotLightLayout->addWidget(spotLightIntensityLabel_);
	spotLightLayout->addWidget(spotLightIntensitySlider_);
	spotLightLayout->addWidget(spotLightCutOffLabel_);
	spotLightLayout->addWidget(spotLightCutOffSlider_);
	spotLightLayout->addWidget(spotLightOuterCutOffLabel_);
	spotLightLayout->addWidget(spotLightOuterCutOffSlider_);
	spotLightLayout->addWidget(new QLabel("Color:", this));
	spotLightLayout->addWidget(spotLightColorCombo_);
}

void Window::updateLightParameters()
{
	if (!renderer_)
		return;

	auto dirLight = renderer_->getDirectionalLight();
	auto spotLight = renderer_->getSpotLight();

	renderer_->setDirectionalLight(dirLight);
	renderer_->setSpotLight(spotLight);
}

void Window::onDirLightEnabledChanged(int state)
{
	if (!renderer_)
		return;

	auto light = renderer_->getDirectionalLight();
	light.enabled = (state == Qt::Checked);
	renderer_->setDirectionalLight(light);
}

void Window::onDirLightIntensityChanged(int value)
{
	if (!renderer_)
		return;

	float intensity = value / 100.0f;
	auto light = renderer_->getDirectionalLight();
	light.intensity = intensity;
	renderer_->setDirectionalLight(light);

	dirLightIntensityLabel_->setText(QString("Intensity: %1").arg(intensity, 0, 'f', 1));
}

void Window::onDirLightColorChanged()
{
	if (!renderer_)
		return;

	int index = dirLightColorCombo_->currentIndex();
	if (index == 3)
	{
		QColor color = QColorDialog::getColor(Qt::white, this, "Select Light Color");
		if (color.isValid())
		{
			auto light = renderer_->getDirectionalLight();
			light.color = QVector3D(color.redF(), color.greenF(), color.blueF());
			renderer_->setDirectionalLight(light);
		}
	}
	else
	{
		QColor color = dirLightColorCombo_->itemData(index).value<QColor>();
		auto light = renderer_->getDirectionalLight();
		light.color = QVector3D(color.redF(), color.greenF(), color.blueF());
		renderer_->setDirectionalLight(light);
	}
}

void Window::onSpotLightEnabledChanged(int state)
{
	if (!renderer_)
		return;

	auto light = renderer_->getSpotLight();
	light.enabled = (state == Qt::Checked);
	renderer_->setSpotLight(light);
}

void Window::onSpotLightIntensityChanged(int value)
{
	if (!renderer_)
		return;

	float intensity = value / 100.0f;
	auto light = renderer_->getSpotLight();
	light.intensity = intensity;
	renderer_->setSpotLight(light);

	spotLightIntensityLabel_->setText(QString("Intensity: %1").arg(intensity, 0, 'f', 1));
}

void Window::onSpotLightCutOffChanged(int value)
{
	if (!renderer_)
		return;

	float cutOff = qCos(qDegreesToRadians(static_cast<float>(value) + 0.5f));
	auto light = renderer_->getSpotLight();
	light.cutOff = cutOff;
	renderer_->setSpotLight(light);

	spotLightCutOffLabel_->setText(QString("Cut Off: %1").arg(value + 0.5f, 0, 'f', 1));

	if (static_cast<int>(qRadiansToDegrees(qAcos(light.outerCutOff)) - 0.5f) < value)
	{
		light.outerCutOff = light.cutOff;
		renderer_->setSpotLight(light);
		spotLightOuterCutOffSlider_->setValue(value);
	}
}

void Window::onSpotLightOuterCutOffChanged(int value)
{
	if (!renderer_)
		return;

	float outerCutOff = qCos(qDegreesToRadians(static_cast<float>(value) + 0.5f));
	auto light = renderer_->getSpotLight();
	light.outerCutOff = outerCutOff;
	renderer_->setSpotLight(light);

	spotLightOuterCutOffLabel_->setText(QString("Outer Cut Off: %1").arg(value + 0.5f, 0, 'f', 1));

	if (static_cast<int>(qRadiansToDegrees(qAcos(light.cutOff)) + 0.5f) > value)
	{
		light.cutOff = light.outerCutOff;
		renderer_->setSpotLight(light);
		spotLightCutOffSlider_->setValue(value);
	}
}

void Window::onSpotLightColorChanged()
{
	if (!renderer_)
		return;

	int index = spotLightColorCombo_->currentIndex();
	if (index == 3)
	{
		QColor color = QColorDialog::getColor(Qt::white, this, "Select Light Color");
		if (color.isValid())
		{
			auto light = renderer_->getSpotLight();
			light.color = QVector3D(color.redF(), color.greenF(), color.blueF());
			renderer_->setSpotLight(light);
		}
	}
	else
	{
		QColor color = spotLightColorCombo_->itemData(index).value<QColor>();
		auto light = renderer_->getSpotLight();
		light.color = QVector3D(color.redF(), color.greenF(), color.blueF());
		renderer_->setSpotLight(light);
	}
}

void Window::onInit()
{
	openglContext_ = std::make_shared<OpenGLContext>(QOpenGLContext::currentContext()->functions());

	camera_ = std::make_unique<Camera>();
	camera_->setPosition(QVector3D(0.0f, 2.0f, 0.0f));
	camera_->setYaw(0.0f);
	camera_->setPitch(-30.0f);
	camera_->setMoveSpeed(5.0f);
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

void Window::onMorphSliderChanged(int value)
{
	if (!model_)
		return;

	float morphFactor = value / 100.0f;
	model_->setMorphFactor(morphFactor);
	morphLabel_->setText(QString("Morphing factor: %1").arg(morphFactor, 0, 'f', 2));
}

void Window::onRadiusSliderChanged(int value)
{
	if (!model_)
		return;

	float radius = value / 10.0f;
	model_->setSphereRadius(radius);
	radiusLabel_->setText(QString("Sphere radius: %1").arg(radius, 0, 'f', 1));
}

void Window::onEnableMorphChanged(int state)
{
	if (!model_)
		return;

	bool enabled = (state == Qt::Checked);
	model_->setMorphToSphere(enabled);
}

bool Window::initializeScene()
{
	model_ = std::make_shared<ModelEntity>("noel");
	model_->setShaderProgram(renderer_->getModelShader());

	if (!model_->loadFromGLTF(":/Models/noel.glb"))
	{
		return false;
	}

	model_->setScale(QVector3D(2.f, 2.f, 2.f));
	//model_->setPosition(QVector3D(0.0f, 1.5f, 0.0f));
	model_->setRotation(QVector3D(90.0f, 0.0f, 0.0f));

	model_->setMorphCenter(QVector3D(0.0f, 1.5f, 0.0f));
	model_->setSphereRadius(1.0f);

	DirectionalLight dirLight;
	dirLight.color = QVector3D(1.0f, 1.0f, 1.0f);
	dirLight.intensity = 1.0f;
	dirLight.enabled = true;

	SpotLight spotLight;
	spotLight.color = QVector3D(1.0f, 1.0f, 1.0f);
	spotLight.intensity = 1.0f;
	spotLight.cutOff = qCos(qDegreesToRadians(12.5f));
	spotLight.outerCutOff = qCos(qDegreesToRadians(17.5f));
	spotLight.enabled = true;

	renderer_->setDirectionalLight(dirLight);
	renderer_->setSpotLight(spotLight);

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
	sceneGraph_->addEntity(model_, "SponzaNode");

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