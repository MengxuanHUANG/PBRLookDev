#include "window.h"

namespace MyCore
{
	WindowsWindow* WindowsWindow::s_Instance = nullptr;

	// error callback function
	void errorCallback(int error, const char* description) {
		fprintf(stderr, "%s\n", description);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props, EventCallbackFn fn)
		: m_Data(fn, props)
	{
		ASSERT(s_Instance == nullptr);
		s_Instance = this;
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

		const auto& [w, h] = props.resolution;

		m_NativeWindow = glfwCreateWindow(w, h, props.title.c_str(), NULL, NULL);
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

		// WindowResize Call back
		glfwSetWindowSizeCallback(m_NativeWindow, [](GLFWwindow* window, int width, int height){
			WindowResizeEvent event(width, height);
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			auto& [w, h] = data.windowProps.resolution;
			w = width;
			h = height;

			data.eventCallbackFn(event);
		});
		glfwSetWindowCloseCallback(m_NativeWindow, [](GLFWwindow* window)
		{
			WindowCloseEvent event;
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.eventCallbackFn(event);
		});
		glfwSetMouseButtonCallback(m_NativeWindow, [](GLFWwindow* window, int button, int action, int mods){
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button);
				data.eventCallbackFn(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				data.eventCallbackFn(event);
				break;
			}
			}
		});
		glfwSetScrollCallback(m_NativeWindow, [](GLFWwindow* window, double X_Offset, double Y_Offset){
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)X_Offset, (float)Y_Offset);
			data.eventCallbackFn(event);
		});
		glfwSetCursorPosCallback(m_NativeWindow, [](GLFWwindow* window, double X_Pos, double Y_Pos){
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)X_Pos, (float)Y_Pos);
			data.eventCallbackFn(event);
		});
		glfwSetKeyCallback(m_NativeWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods){
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(key, 0);
				data.eventCallbackFn(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(key);
				data.eventCallbackFn(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent(key, 1);
				break;
			}
			}
		});
		glfwSetCharCallback(m_NativeWindow, [](GLFWwindow* window, unsigned int input_char){
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event(input_char);
			data.eventCallbackFn(event);
		});
	}

	void WindowsWindow::OnUpdate()
	{
		// Swap buffers
		m_glewContext->SwapBuffers();

		// Poll events
		glfwPollEvents();
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