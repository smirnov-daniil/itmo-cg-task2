#include "OpenGLContext.h"

OpenGLContext::OpenGLContext(QOpenGLFunctions * functions)
	: functions_(functions)
{
}

void OpenGLContext::setFunctions(QOpenGLFunctions * functions)
{
	functions_ = functions;
}
