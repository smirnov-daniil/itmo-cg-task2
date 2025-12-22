#pragma once

#include <QOpenGLFunctions>
#include <memory>

class OpenGLContext
{
public:
	OpenGLContext(QOpenGLFunctions * functions = nullptr);
	~OpenGLContext() = default;

	void setFunctions(QOpenGLFunctions * functions);
	QOpenGLFunctions * functions() const { return functions_; }

	bool isValid() const { return functions_ != nullptr; }

	QOpenGLFunctions * operator->() const { return functions_; }
	operator QOpenGLFunctions *() const { return functions_; }

private:
	QOpenGLFunctions * functions_;
};

using OpenGLContextPtr = std::shared_ptr<OpenGLContext>;
