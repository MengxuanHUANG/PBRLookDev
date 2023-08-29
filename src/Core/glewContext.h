#pragma once
#include "core.h"

#include <GL/glew.h>
#include <glfw/glfw3.h>

namespace MyCore
{
	class GLEWContext
	{
	public:
		GLEWContext(void* window);

		void Init();
		void SwapBuffers();

	private:
		GLFWwindow* m_WindowHandle;
	};
}