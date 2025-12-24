#include "Entity.h"
#include <cmath>

Entity::Entity(const std::string & name)
	: name_(name)
{
}

void Entity::setPosition(const QVector3D & position)
{
	position_ = position;
	markTransformDirty();
}

void Entity::setRotation(const QVector3D & rotation)
{
	rotation_ = rotation;
	markTransformDirty();
}

void Entity::setScale(const QVector3D & scale)
{
	scale_ = scale;
	markTransformDirty();
}

const QMatrix4x4 & Entity::getTransform() const
{
	if (transformDirty_)
	{
		updateTransform();
		transformDirty_ = false;
	}
	return transform_;
}

void Entity::updateTransform() const
{
	transform_.setToIdentity();
	transform_.translate(position_);
	transform_.rotate(rotation_.x(), 1.0f, 0.0f, 0.0f);
	transform_.rotate(rotation_.y(), 0.0f, 1.0f, 0.0f);
	transform_.rotate(rotation_.z(), 0.0f, 0.0f, 1.0f);
	transform_.scale(scale_);
}
