#include <iostream>
#include <functional>

#include "Core/window.h"

#include "Render/renderer.h"

#include <ImGui/imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

using namespace PBRLookDev;

void CreateCubeData(Renderer& renderer)
{
	std::vector<glm::vec3> vertices{ glm::vec3(-1, -1, -1),
									glm::vec3(1, -1, -1),
									glm::vec3(1, 1, -1),
									glm::vec3(-1, 1, -1),
									glm::vec3(-1, -1, 1),
									glm::vec3(1, -1, 1),
									glm::vec3(1, 1, 1),
									glm::vec3(-1, 1, 1) };

	std::vector<GLuint> indices{ 1, 0, 3, 1, 3, 2,
								4, 5, 6, 4, 6, 7,
								5, 1, 2, 5, 2, 6,
								7, 6, 2, 7, 2, 3,
								0, 4, 7, 0, 7, 3,
								0, 1, 5, 0, 5, 4 };

	renderer.m_CubeVertBuffer->BufferData(vertices.data(), vertices.size() * sizeof(glm::vec3));
	renderer.m_CubeIdxBuffer->BufferData(indices.data(), indices.size() * sizeof(GLuint));
}

void CreateTestTriangle(Renderer& renderer)
{
	std::vector<float> vertices{
		 0, 1, 0, 1, 0.5, 1,
		 1, 0, 0, 1, 1, 1,
		-1, 0, 0, 1, 1, 0.5
	};

	std::vector<unsigned int> indices{
		 0, 1, 2
	};

	renderer.m_VertBuffer->BufferData(vertices.data(), vertices.size() * sizeof(float));
	renderer.m_IdxBuffer->BufferData(indices.data(), indices.size() * sizeof(unsigned int));
}

void CreateSquare(Renderer& renderer)
{
	std::vector<float> vertices{ 
		-1, -1, 0, 0, 0,
		 1, -1, 0, 1, 0,
		 1,  1, 0, 1, 1,
		-1,  1, 0, 0, 1
	};

	std::vector<unsigned int> indices{ 
		0, 1, 2, 
		0, 2, 3 
	};

	renderer.m_SquareVertBuffer->BufferData(vertices.data(), vertices.size() * sizeof(float));
	renderer.m_SquareIdxBuffer->BufferData(indices.data(), indices.size() * sizeof(unsigned int));
}

int main()
{
	MyCore::WindowProps props({ 600, 600 }, "PBRLookDev");

	Renderer renderer;
	uPtr<MyCore::WindowsWindow> window = mkU<MyCore::WindowsWindow>(props, std::bind(&Renderer::OnEvent, &renderer, std::placeholders::_1));
	renderer.Init(props.resolution.first, props.resolution.second);

	CreateCubeData(renderer);
	CreateTestTriangle(renderer);
	CreateSquare(renderer);

	renderer.RenderCubeMapToTexture();
	renderer.m_EnvMapFB->GenMipMaps();

	renderer.RenderDiffuseCubeMap();
	renderer.RenderGlossyCubeMap();

	while (!glfwWindowShouldClose(window->m_NativeWindow))
	{
		// render
		renderer.OnUpdate();

		window->OnUpdate();
	}

	return 0;
}