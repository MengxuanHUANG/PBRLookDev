#pragma once
#include "core.h"

#include "Core/glewContext.h"

#include <GL/glew.h>
#include <glfw/glfw3.h>

namespace MyCore
{
	struct WindowProps
	{
		std::pair<unsigned int, unsigned int> resolution;
		std::string title;

		WindowProps(const std::pair<unsigned int, unsigned int>& res = { 540, 540 },
					const std::string& title = "New Window")
			: resolution(res), title(title)
		{
		}
	};

	struct WindowData
	{
		bool is_alive;
		unsigned int width;
		unsigned int height;

		WindowData(int w, int h)
			: is_alive(true), width(w), height(h)
		{}
	};

	class WindowsWindow
	{
	public:
		WindowsWindow(const WindowProps& props);
		~WindowsWindow();

		void OnUpdate();
		double GetTime();
		int GetMouseButtonState(int button);
		int GetKeyButtonState(int button);
		std::pair<double, double> GetCursorPosition();
	
	private:
		void Init(const WindowProps& props);

	public:
		GLFWwindow* m_NativeWindow;
		uPtr<GLEWContext> m_glewContext;
		WindowData m_Data;
	};
}