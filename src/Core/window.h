#pragma once
#include "core.h"

#include "Core/glewContext.h"

#include <GL/glew.h>
#include <glfw/glfw3.h>

#include "event.h"
#include <functional>

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

	class WindowsWindow
	{
	public:
		typedef std::function<bool(Event&)> EventCallbackFn;

		WindowsWindow(const WindowProps& props, EventCallbackFn fn);
		~WindowsWindow();

		void OnUpdate();
		double GetTime();
		int GetMouseButtonState(int button);
		int GetKeyButtonState(int button);
		std::pair<double, double> GetCursorPosition();
	
	private:
		void Init(const WindowProps& props);

	protected:
		struct WindowData
		{
			EventCallbackFn eventCallbackFn;
			WindowProps windowProps;

			WindowData(EventCallbackFn fn, const WindowProps& props)
				:eventCallbackFn(fn), windowProps(props)
			{}
		};

	private:
		static WindowsWindow* s_Instance;

	public:
		GLFWwindow* m_NativeWindow;
		uPtr<GLEWContext> m_glewContext;
		WindowData m_Data;

		
		inline static WindowsWindow* GetWindow() { return s_Instance; }
	};
}