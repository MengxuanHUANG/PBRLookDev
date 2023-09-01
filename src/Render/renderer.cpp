#include "renderer.h"

#include <array>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/window.h"
#include "Core/eventDispatcher.h"

#define JOIN(a, b) a##b
#define JOIN2(a, b) JOIN(a, b)
#define STR(a) #a
#define STR2(a) STR(a)

const std::string res_base_path = STR2(JOIN2(PROJ_BASE_PATH, /res));

// -X, +X, -Y, +Y, -Z, +Z
glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
const static std::array<glm::mat4, 6> views =
{
	glm::lookAt(glm::vec3(0.f), glm::vec3( 1.f,  0.f,  0.f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.f), glm::vec3(-1.f,  0.f,  0.f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.f), glm::vec3( 0.f,  1.f,  0.f), glm::vec3(0.0f,  0.0f,  1.0f)),
	glm::lookAt(glm::vec3(0.f), glm::vec3( 0.f, -1.f,  0.f), glm::vec3(0.0f,  0.0f, -1.0f)),
	glm::lookAt(glm::vec3(0.f), glm::vec3( 0.f,  0.f,  1.f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.f), glm::vec3( 0.f,  0.f, -1.f), glm::vec3(0.0f, -1.0f,  0.0f))
};

using namespace MyCore;

namespace PBRLookDev
{
	void Renderer::Init(unsigned int w, unsigned int h)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		// Set the color with which the screen is filled at the start of each render call.
		glClearColor(0.5, 0.5, 0.5, 1);

		m_PersCamera = mkU<PerspectiveCamera>(w, h);
		m_CamController = mkU<PerspectiveCameraController>(*m_PersCamera);
		m_VertBuffer = mkU<OpenglBuffer>(GL_ARRAY_BUFFER);
		m_IdxBuffer = mkU<OpenglBuffer>(GL_ELEMENT_ARRAY_BUFFER);
		
		m_CubeVertBuffer = mkU<OpenglBuffer>(GL_ARRAY_BUFFER);
		m_CubeIdxBuffer = mkU<OpenglBuffer>(GL_ELEMENT_ARRAY_BUFFER);

		m_EnvMap = mkU<OpenglTexture>(res_base_path + "/textures/env/limpopo_golf_course_2k.hdr");

		m_TestShader = OpenglShader::Create("TestShader",
												res_base_path + "/shaders/glsl/vertex.glsl",
												res_base_path + "/shaders/glsl/fragment.glsl");
		
		m_EnvMapConversionShader = OpenglShader::Create("CubeMapConversionShader",
															res_base_path + "/shaders/glsl/cubemap.vert.glsl",
															res_base_path + "/shaders/glsl/cubemap_uv_conversion.frag.glsl");
		
		m_EnvMapShader = OpenglShader::Create("EnvMapShader",
												res_base_path + "/shaders/glsl/envMap.vert.glsl",
												res_base_path + "/shaders/glsl/envMap.frag.glsl");

		m_EnvMapFB = mkU<CubeMapFrameBuffer>(1024, 1024, 1.f, true);
	}

	bool Renderer::OnEvent(MyCore::Event& event)
	{
		EventDispatcher dispatcher(event);
		return dispatcher.Dispatch<MyCore::MouseMovedEvent>(std::bind(&PerspectiveCameraController::OnMouseMoved, m_CamController.get(), std::placeholders::_1));
	}

	void Renderer::RenderCubeMapToTexture()
	{
		m_EnvMapConversionShader->Bind();

		m_EnvMapConversionShader->SetUniformInt("u_EquirectangularMap", ENV_MAP_FLAT_TEX_SLOT);
		m_EnvMap->Bind(ENV_MAP_FLAT_TEX_SLOT);

		glViewport(0, 0, m_EnvMapFB->m_Width, m_EnvMapFB->m_Height);
		m_EnvMapFB->BindFrameBuffer();

		m_EnvMapConversionShader->SetUniformMat4("u_Proj", captureProjection);

		for (int i = 0; i < 6; ++i)
		{
			m_EnvMapConversionShader->SetUniformMat4("u_View", views[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_EnvMapFB->m_CubeMapId, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_CubeVertBuffer->Bind();
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);

			m_CubeIdxBuffer->Bind();
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}
		// reset to the default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::OnUpdate()
	{
		glViewport(0, 0, m_PersCamera->width, m_PersCamera->height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// RenderEnvMap
		m_EnvMapShader->Bind();
		m_EnvMapFB->BindToTextureSlot(ENV_MAP_CUBE_TEX_SLOT);
		m_EnvMapShader->SetUniformInt("u_EnvironmentMap", ENV_MAP_CUBE_TEX_SLOT);
		m_EnvMapShader->SetUniformMat4("u_ViewProj", m_PersCamera->GetViewProj_OrientedOnly());
		
		m_CubeVertBuffer->Bind();
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);

		m_CubeIdxBuffer->Bind();
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		// render triangle
		m_TestShader->Bind();
		m_TestShader->SetUniformMat4("u_ViewProjection", m_PersCamera->GetViewProj());
		m_TestShader->SetUniformMat4("u_Transform", glm::mat4(1));
		m_EnvMap->Bind(0);
		m_TestShader->SetUniformInt("u_Texture", 0);
		
		m_VertBuffer->Bind();
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(0));
		
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		
		m_IdxBuffer->Bind();
		
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
	}
}