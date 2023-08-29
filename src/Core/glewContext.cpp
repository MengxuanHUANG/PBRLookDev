#include "glewContext.h"

namespace MyCore
{
	GLEWContext::GLEWContext(void* window)
		:m_WindowHandle(reinterpret_cast<GLFWwindow*>(window))
	{
	}

	void GLEWContext::Init()
	{
		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK) {
			ASSERT(false);
		}
	}

	void GLEWContext::SwapBuffers()
	{
		ASSERT(m_WindowHandle);
		glfwSwapBuffers(m_WindowHandle);
	}
}