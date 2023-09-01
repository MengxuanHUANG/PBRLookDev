#pragma once

#include <imgui/imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace PBRLookDev
{
	class ImguiWrapper
	{
	public:
		ImguiWrapper(GLFWwindow* glfw_window);
		~ImguiWrapper();

		void Begin();
		void End();

	public:
		ImGuiIO* io;
	};
}