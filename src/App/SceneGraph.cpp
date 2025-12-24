#include "SceneGraph.h"
#include "Camera.h"
#include "Entity.h"
#include <QVector3D>
#include <algorithm>

SceneNode::SceneNode(const std::string & name)
	: name_(name)
{
}

void SceneNode::addChild(std::shared_ptr<SceneNode> child)
{
	if (!child || child.get() == this)
		return;

	if (child->parent_)
	{
		auto & siblings = child->parent_->children_;
		siblings.erase(std::remove(siblings.begin(), siblings.end(), child), siblings.end());
	}

	child->parent_ = this;
	children_.push_back(child);
}

void SceneNode::removeChild(std::shared_ptr<SceneNode> child)
{
	if (!child)
		return;

	auto it = std::find(children_.begin(), children_.end(), child);
	if (it != children_.end())
	{
		(*it)->parent_ = nullptr;
		children_.erase(it);
	}
}

void SceneNode::removeChild(const std::string & name)
{
	auto child = findChild(name);
	if (child)
	{
		removeChild(child);
	}
}

std::shared_ptr<SceneNode> SceneNode::findChild(const std::string & name) const
{
	for (const auto & child: children_)
	{
		if (child->getName() == name)
		{
			return child;
		}

		auto found = child->findChild(name);
		if (found)
		{
			return found;
		}
	}
	return nullptr;
}

void SceneNode::setEntity(std::shared_ptr<Entity> entity)
{
	entity_ = entity;
}

void SceneNode::traverse(const std::function<void(SceneNode *)> & visitor)
{
	visitor(this);

	for (auto & child: children_)
	{
		child->traverse(visitor);
	}
}

void SceneNode::traverseVisible(const std::function<void(SceneNode *)> & visitor)
{
	if (!visible_)
		return;

	visitor(this);

	for (auto & child: children_)
	{
		child->traverseVisible(visitor);
	}
}

void SceneNode::update(float deltaTime)
{
	if (!visible_)
		return;

	if (entity_)
	{
		entity_->update(deltaTime);
	}

	for (auto & child: children_)
	{
		child->update(deltaTime);
	}
}

void SceneNode::render(Camera * camera, OpenGLContextPtr context)
{
	if (!visible_)
		return;

	if (entity_ && entity_->isVisible())
	{
		entity_->render(camera, context);
	}

	for (auto & child: children_)
	{
		child->render(camera, context);
	}
}

SceneGraph::SceneGraph()
{
	root_ = std::make_shared<SceneNode>("Root");
}

std::shared_ptr<SceneNode> SceneGraph::createNode(const std::string & name)
{
	return std::make_shared<SceneNode>(name);
}

std::shared_ptr<SceneNode> SceneGraph::addEntity(std::shared_ptr<Entity> entity, const std::string & nodeName)
{
	std::string name = nodeName.empty() ? entity->getName() : nodeName;
	auto node = createNode(name);
	node->setEntity(entity);
	root_->addChild(node);
	return node;
}

std::shared_ptr<SceneNode> SceneGraph::findNode(const std::string & name) const
{
	return root_->findChild(name);
}

void SceneGraph::update(float deltaTime)
{
	root_->update(deltaTime);
}

size_t SceneGraph::getNodeCount() const
{
	size_t count = 0;
	root_->traverse([&count](SceneNode *) { count++; });
	return count;
}

size_t SceneGraph::getVisibleNodeCount() const
{
	size_t count = 0;
	root_->traverseVisible([&count](SceneNode *) { count++; });
	return count;
}
