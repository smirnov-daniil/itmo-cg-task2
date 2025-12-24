#pragma once

#include <QMatrix4x4>
#include <QSet>
#include <QVector3D>

class Camera
{
public:
	Camera();
	~Camera() = default;

	void setPerspective(float fov, float aspect, float nearPlane, float farPlane);
	void setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

	void setPosition(const QVector3D & position);
	void setTarget(const QVector3D & target);
	void setUp(const QVector3D & up);

	const QVector3D & getPosition() const { return position_; }
	const QVector3D & getFront() const { return front_; }
	const QVector3D & getUp() const { return up_; }
	const QVector3D & getRight() const { return right_; }

	const QMatrix4x4 & getViewMatrix() const;
	const QMatrix4x4 & getProjectionMatrix() const { return projection_; }
	QMatrix4x4 getViewProjectionMatrix() const;

	void setYaw(float yaw);
	void setPitch(float pitch);
	float getYaw() const { return yaw_; }
	float getPitch() const { return pitch_; }

	void setMouseSensitivity(float sensitivity) { mouseSensitivity_ = sensitivity; }
	void setMoveSpeed(float speed) { moveSpeed_ = speed; }

	void update(float deltaTime);
	void processMouseMovement(float xOffset, float yOffset);
	void processKeyboardInput(const QSet<int> & pressedKeys, float deltaTime);

	void moveForward(float distance);
	void moveBackward(float distance);
	void moveLeft(float distance);
	void moveRight(float distance);
	void moveUp(float distance);
	void moveDown(float distance);

private:
	void updateCameraVectors();
	void updateViewMatrix() const;

	QVector3D position_{0.0f, 0.0f, 0.0f};
	QVector3D front_{0.0f, 0.0f, -1.0f};
	QVector3D up_{0.0f, 1.0f, 0.0f};
	QVector3D right_{1.0f, 0.0f, 0.0f};
	QVector3D worldUp_{0.0f, 1.0f, 0.0f};

	float yaw_{-90.0f};
	float pitch_{0.0f};

	float maxPitch_{89.0f};
	float minPitch_{-89.0f};

	float mouseSensitivity_{0.1f};
	float moveSpeed_{5.0f};

	QMatrix4x4 projection_;
	mutable QMatrix4x4 view_;
	mutable bool viewDirty_ = true;
};
