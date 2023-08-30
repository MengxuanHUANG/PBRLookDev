#include <iostream>
#include <functional>

#include "Core/window.h"

#include "Render/shader.h"
#include "Render/cameraController.h"
#include "Core/eventDispatcher.h"

using namespace PBRLookDev;

PerspectiveCamera perspectiveCamera(600, 600);
PerspectiveCameraController camController(perspectiveCamera);

int main()
{
	// Camera
	MyCore::WindowProps props({ 600, 600 }, "PBRLookDev");
	uPtr<MyCore::WindowsWindow> window = mkU<MyCore::WindowsWindow>(props, std::bind(&PerspectiveCameraController::OnEvent, &camController, std::placeholders::_1));

	// load shaders
	ShaderLib shader_lib;

	std::string v_path = "E://Projects//PBRLookDev//res//shaders//glsl//vertex.glsl";
	std::string f_path = "E://Projects//PBRLookDev//res//shaders//glsl//fragment.glsl";

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

	GLuint vbo;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertice.size() * sizeof(float), vertice.data(), GL_STATIC_DRAW);

	GLuint idx;
	glGenBuffers(1, &idx);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

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

		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(0));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx);

		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
	}

	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &idx);

	return 0;
}