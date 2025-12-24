#include "Camera.h"
#include <QtCore>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

inline float degreesToRadians(float degrees)
{
	return degrees * static_cast<float>(M_PI) / 180.0f;
}

Camera::Camera()
{
	yaw_ = -90.0f;
	pitch_ = 0.0f;

	updateCameraVectors();
}

void Camera::setPerspective(float fov, float aspect, float nearPlane, float farPlane)
{
	projection_.setToIdentity();
	projection_.perspective(fov, aspect, nearPlane, farPlane);
}

void Camera::setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
	projection_.setToIdentity();
	projection_.ortho(left, right, bottom, top, nearPlane, farPlane);
}

void Camera::setPosition(const QVector3D & position)
{
	position_ = position;
	viewDirty_ = true;
}

void Camera::setTarget(const QVector3D & target)
{
	front_ = (target - position_).normalized();
	viewDirty_ = true;
}

void Camera::setUp(const QVector3D & up)
{
	worldUp_ = up;
	updateCameraVectors();
}

const QMatrix4x4 & Camera::getViewMatrix() const
{
	if (viewDirty_)
	{
		updateViewMatrix();
		viewDirty_ = false;
	}
	return view_;
}

QMatrix4x4 Camera::getViewProjectionMatrix() const
{
	return projection_ * getViewMatrix();
}

void Camera::setYaw(float yaw)
{
	yaw_ = yaw;
	updateCameraVectors();
}

void Camera::setPitch(float pitch)
{
	pitch_ = qBound(minPitch_, pitch, maxPitch_);
	updateCameraVectors();
}

void Camera::update(float /*deltaTime*/)
{
}

void Camera::processMouseMovement(float xOffset, float yOffset)
{
	xOffset *= mouseSensitivity_;
	yOffset *= mouseSensitivity_;

	yaw_ += xOffset;
	pitch_ += yOffset;

	pitch_ = qBound(minPitch_, pitch_, maxPitch_);

	updateCameraVectors();
}

void Camera::processKeyboardInput(const QSet<int> & pressedKeys, float deltaTime)
{
	float velocity = moveSpeed_ * deltaTime;

	if (pressedKeys.contains(Qt::Key_W))
	{
		moveForward(velocity);
	}
	if (pressedKeys.contains(Qt::Key_S))
	{
		moveBackward(velocity);
	}
	if (pressedKeys.contains(Qt::Key_A))
	{
		moveLeft(velocity);
	}
	if (pressedKeys.contains(Qt::Key_D))
	{
		moveRight(velocity);
	}
	if (pressedKeys.contains(Qt::Key_Space))
	{
		moveUp(velocity);
	}
	if (pressedKeys.contains(Qt::Key_Shift))
	{
		moveDown(velocity);
	}
}

void Camera::moveForward(float distance)
{
	position_ += front_ * distance;
	viewDirty_ = true;
}

void Camera::moveBackward(float distance)
{
	position_ -= front_ * distance;
	viewDirty_ = true;
}

void Camera::moveLeft(float distance)
{
	position_ -= right_ * distance;
	viewDirty_ = true;
}

void Camera::moveRight(float distance)
{
	position_ += right_ * distance;
	viewDirty_ = true;
}

void Camera::moveUp(float distance)
{
	position_ += up_ * distance;
	viewDirty_ = true;
}

void Camera::moveDown(float distance)
{
	position_ -= up_ * distance;
	viewDirty_ = true;
}

void Camera::updateCameraVectors()
{
	QVector3D front;
	front.setX(cos(degreesToRadians(yaw_)) * cos(degreesToRadians(pitch_)));
	front.setY(sin(degreesToRadians(pitch_)));
	front.setZ(sin(degreesToRadians(yaw_)) * cos(degreesToRadians(pitch_)));
	front_ = front.normalized();

	right_ = QVector3D::crossProduct(front_, worldUp_).normalized();
	up_ = QVector3D::crossProduct(right_, front_).normalized();

	viewDirty_ = true;
}

void Camera::updateViewMatrix() const
{
	view_.setToIdentity();
	view_.lookAt(position_, position_ + front_, up_);
}
