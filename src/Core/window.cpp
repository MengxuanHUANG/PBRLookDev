#include "window.h"

namespace MyCore
{
	// error callback function
	void errorCallback(int error, const char* description) {
		fprintf(stderr, "%s\n", description);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
		: m_Data(props.resolution.first, props.resolution.second)
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		glfwDestroyWindow(m_NativeWindow);
		glfwTerminate();
	}
	void WindowsWindow::Init(const WindowProps& props)
	{
		glfwSetErrorCallback(errorCallback);

		if (!glfwInit()) {
			exit(EXIT_FAILURE);
		}

		m_NativeWindow = glfwCreateWindow(m_Data.width, m_Data.height, props.title.c_str(), NULL, NULL);
		if (!m_NativeWindow) {
			ASSERT(false);
			glfwTerminate();
		}

		glfwMakeContextCurrent(m_NativeWindow);

		m_glewContext = mkU<GLEWContext>(reinterpret_cast<void*>(m_NativeWindow));

		m_glewContext->Init();

		int display_w, display_h;
		glfwGetFramebufferSize(m_NativeWindow, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);

		glfwSetWindowUserPointer(m_NativeWindow, &m_Data);

		glfwSetWindowCloseCallback(m_NativeWindow, [](GLFWwindow* window) {
			WindowData* data = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
			data->is_alive = false;
		});
	}

	void WindowsWindow::OnUpdate()
	{
		// Poll events
		glfwPollEvents();

		// Swap buffers
		m_glewContext->SwapBuffers();
	}

	double WindowsWindow::GetTime()
	{
		return glfwGetTime();
	}

	int WindowsWindow::GetMouseButtonState(int button)
	{
		return glfwGetMouseButton(m_NativeWindow, button);
	}

	int WindowsWindow::GetKeyButtonState(int keycode)
	{
		return glfwGetKey(m_NativeWindow, keycode);
	}

	std::pair<double, double> WindowsWindow::GetCursorPosition()
	{
		double x, y;
		glfwGetCursorPos(m_NativeWindow, &x, &y);
		return { x, y };
	}
}