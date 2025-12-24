#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

class Entity;
class Camera;
class OpenGLContext;
using OpenGLContextPtr = std::shared_ptr<OpenGLContext>;

class QVector3D;

class SceneNode
{
public:
	SceneNode(const std::string & name = "Node");
	virtual ~SceneNode() = default;

	void addChild(std::shared_ptr<SceneNode> child);
	void removeChild(std::shared_ptr<SceneNode> child);
	void removeChild(const std::string & name);
	std::shared_ptr<SceneNode> findChild(const std::string & name) const;

	const std::string & getName() const { return name_; }
	void setName(const std::string & name) { name_ = name; }

	SceneNode * getParent() const { return parent_; }
	const std::vector<std::shared_ptr<SceneNode>> & getChildren() const { return children_; }

	void setEntity(std::shared_ptr<Entity> entity);
	std::shared_ptr<Entity> getEntity() const { return entity_; }

	void traverse(const std::function<void(SceneNode *)> & visitor);
	void traverseVisible(const std::function<void(SceneNode *)> & visitor);

	virtual void update(float deltaTime);
	virtual void render(Camera * camera, OpenGLContextPtr context);

	bool isVisible() const { return visible_; }
	void setVisible(bool visible) { visible_ = visible; }

protected:
	std::string name_;
	SceneNode * parent_ = nullptr;
	std::vector<std::shared_ptr<SceneNode>> children_;
	std::shared_ptr<Entity> entity_;
	bool visible_ = true;
};

class SceneGraph
{
public:
	SceneGraph();
	~SceneGraph() = default;

	std::shared_ptr<SceneNode> getRoot() const { return root_; }

	std::shared_ptr<SceneNode> createNode(const std::string & name = "Node");
	std::shared_ptr<SceneNode> addEntity(std::shared_ptr<Entity> entity, const std::string & nodeName = "");

	std::shared_ptr<SceneNode> findNode(const std::string & name) const;

	void update(float deltaTime);

	size_t getNodeCount() const;
	size_t getVisibleNodeCount() const;

private:
	std::shared_ptr<SceneNode> root_;
};
