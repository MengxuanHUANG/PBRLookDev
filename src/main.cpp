#include <iostream>
#include <functional>

#include "Core/window.h"

#include "Render/shader.h"
#include "Render/buffer.h"
#include "Render/texture.h"

#include "Render/cameraController.h"
#include "Core/eventDispatcher.h"

using namespace PBRLookDev;

#define JOIN(a, b) a##b
#define JOIN2(a, b) JOIN(a, b)
#define STR(a) #a
#define STR2(a) STR(a)

const std::string res_base_path = STR2(JOIN2(PROJ_BASE_PATH, /res));

PerspectiveCamera perspectiveCamera(600, 600);
PerspectiveCameraController camController(perspectiveCamera);

int main()
{
	// Camera
	MyCore::WindowProps props({ 600, 600 }, "PBRLookDev");
	uPtr<MyCore::WindowsWindow> window = mkU<MyCore::WindowsWindow>(props, std::bind(&PerspectiveCameraController::OnEvent, &camController, std::placeholders::_1));

	// load shaders
	std::string v_path = res_base_path + "/shaders/glsl/vertex.glsl";
	std::string f_path = res_base_path + "/shaders/glsl/fragment.glsl";

	uPtr<OpenglShader> test_shader = OpenglShader::Create("TestShader", v_path, f_path);

	GLuint uniVp = glGetAttribLocation(test_shader->m_RendererID, "u_ViewProjection");
	GLuint uniTrans = glGetAttribLocation(test_shader->m_RendererID, "u_Transform");

	std::vector<float> vertice{
		 0, 1, 0, 1, 0.5, 1,
		 1, 0, 0, 1, 1, 1,
		-1, 0, 0, 1, 1, 0.5
	};

	std::vector<unsigned int> indices{
		 0, 1, 2
	};

	OpenglBuffer vert_buffer(GL_ARRAY_BUFFER);
	OpenglBuffer idx_buffer(GL_ELEMENT_ARRAY_BUFFER);
	vert_buffer.BufferData(vertice.data(), vertice.size() * sizeof(float));
	idx_buffer.BufferData(indices.data(), indices.size() * sizeof(unsigned int));

	std::string tex_path = res_base_path + "/textures/env/checkboard.png";
	OpenglTexture texture(tex_path);

	while (!glfwWindowShouldClose(window->m_NativeWindow))
	{
		window->OnUpdate();

		// ImGUI

		// render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		

		test_shader->Bind();
		test_shader->SetUniformMat4("u_ViewProjection", perspectiveCamera.GetViewProj());
		test_shader->SetUniformMat4("u_Transform", glm::mat4(1));
		texture.Bind(0);
		test_shader->SetUniformInt("u_Texture", 0);

		vert_buffer.Bind();
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(0));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));

		idx_buffer.Bind();

		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
	}

	return 0;
}