#pragma once

#include <QMatrix4x4>
#include <QVector3D>
#include <memory>
#include <string>
#include <vector>

class Camera;
class OpenGLContext;
using OpenGLContextPtr = std::shared_ptr<OpenGLContext>;

class Entity
{
public:
	Entity(const std::string & name = "Entity");
	virtual ~Entity() = default;

	void setPosition(const QVector3D & position);
	void setRotation(const QVector3D & rotation);
	void setScale(const QVector3D & scale);

	const QVector3D & getPosition() const { return position_; }
	const QVector3D & getRotation() const { return rotation_; }
	const QVector3D & getScale() const { return scale_; }

	const QMatrix4x4 & getTransform() const;

	const std::string & getName() const { return name_; }
	void setName(const std::string & name) { name_ = name; }

	bool isVisible() const { return visible_; }
	void setVisible(bool visible) { visible_ = visible; }

	virtual void update(float /*deltaTime*/) {}

	virtual void render(Camera * camera, OpenGLContextPtr context) = 0;

protected:
	void markTransformDirty() { transformDirty_ = true; }

private:
	void updateTransform() const;

	std::string name_;
	QVector3D position_{0.0f, 0.0f, 0.0f};
	QVector3D rotation_{0.0f, 0.0f, 0.0f};
	QVector3D scale_{1.0f, 1.0f, 1.0f};

	mutable QMatrix4x4 transform_;
	mutable bool transformDirty_ = true;

	bool visible_ = true;
};
