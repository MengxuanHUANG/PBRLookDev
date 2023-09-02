#include "renderer.h"

#include <array>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/window.h"
#include "Core/eventDispatcher.h"

#include <imgui/imgui.h>

#define JOIN(a, b) a##b
#define JOIN2(a, b) JOIN(a, b)
#define STR(a) #a
#define STR2(a) STR(a)

const std::string res_base_path = STR2(JOIN2(PROJ_BASE_PATH, /res));

// -X, +X, -Y, +Y, -Z, +Z
const static glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
const static std::array<glm::mat4, 6> ViewProjs =
{
	captureProjection * glm::lookAt(glm::vec3(0.f), glm::vec3( 1.f,  0.f,  0.f), glm::vec3(0.0f, -1.0f,  0.0f)),
	captureProjection * glm::lookAt(glm::vec3(0.f), glm::vec3(-1.f,  0.f,  0.f), glm::vec3(0.0f, -1.0f,  0.0f)),
	captureProjection * glm::lookAt(glm::vec3(0.f), glm::vec3( 0.f,  1.f,  0.f), glm::vec3(0.0f,  0.0f,  1.0f)),
	captureProjection * glm::lookAt(glm::vec3(0.f), glm::vec3( 0.f, -1.f,  0.f), glm::vec3(0.0f,  0.0f, -1.0f)),
	captureProjection * glm::lookAt(glm::vec3(0.f), glm::vec3( 0.f,  0.f,  1.f), glm::vec3(0.0f, -1.0f,  0.0f)),
	captureProjection * glm::lookAt(glm::vec3(0.f), glm::vec3( 0.f,  0.f, -1.f), glm::vec3(0.0f, -1.0f,  0.0f))
};

using namespace MyCore;

namespace PBRLookDev
{
	void Renderer::Init(unsigned int w, unsigned int h)
	{
		m_ImguiWrapper = mkU<ImguiWrapper>(WindowsWindow::GetWindow()->m_NativeWindow);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		// Set the color with which the screen is filled at the start of each render call.
		glClearColor(0.5, 0.5, 0.5, 1);

		m_PersCamera = mkU<PerspectiveCamera>(w, h, 45.f, glm::vec3(0, 1, 15));
		m_CamController = mkU<PerspectiveCameraController>(*m_PersCamera);
		
		m_CubeVertBuffer = mkU<OpenglBuffer>(GL_ARRAY_BUFFER);
		m_CubeIdxBuffer = mkU<OpenglBuffer>(GL_ELEMENT_ARRAY_BUFFER);
		m_SquareVertBuffer = mkU<OpenglBuffer>(GL_ARRAY_BUFFER);
		m_SquareIdxBuffer = mkU<OpenglBuffer>(GL_ELEMENT_ARRAY_BUFFER);

		m_EnvMap = mkU<OpenglTexture>(res_base_path + "/textures/env/limpopo_golf_course_2k.hdr");
		m_BRDFLookUpTex = mkU<OpenglTexture>(res_base_path + "/textures/brdfLUT.png");
		
		m_EnvMapConversionShader = OpenglShader::Create("CubeMapConversionShader",
															res_base_path + "/shaders/glsl/cubemap.vert.glsl",
															res_base_path + "/shaders/glsl/cubemap_uv_conversion.frag.glsl");
		
		m_EnvMapShader = OpenglShader::Create("EnvMapShader",
												res_base_path + "/shaders/glsl/envMap.vert.glsl",
												res_base_path + "/shaders/glsl/envMap.frag.glsl");

		m_DiffuseConversionsShader = OpenglShader::Create("DiffuseConversionsShader",
															res_base_path + "/shaders/glsl/cubemap.vert.glsl",
															res_base_path + "/shaders/glsl/diffuseConvolution.frag.glsl");

		m_GlossyConversionsShader = OpenglShader::Create("GlossyConversionsShader",
															res_base_path + "/shaders/glsl/cubemap.vert.glsl",
															res_base_path + "/shaders/glsl/glossyConvolution.frag.glsl");

		m_SDF_PBRShader = OpenglShader::Create("SDF_PBRShader",
												res_base_path + "/shaders/glsl/do_nothing.vert.glsl",
												res_base_path + "/shaders/glsl/sdf_pbr.frag.glsl");

		m_EnvMapFB = mkU<CubeMapFrameBuffer>(1024, 1024, 1.f, true);
		m_DiffuseFB = mkU<CubeMapFrameBuffer>(32, 32, 1.f, false);
		m_GlossyFB = mkU<CubeMapFrameBuffer>(512, 512, 1.f, true);
	}

	bool Renderer::OnEvent(MyCore::Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MyCore::MouseMovedEvent>(std::bind(&PerspectiveCameraController::OnMouseMoved, m_CamController.get(), std::placeholders::_1));
		dispatcher.Dispatch<MyCore::WindowResizeEvent>(std::bind(&Renderer::OnWindowResize, this, std::placeholders::_1));

		return false;
	}

	bool Renderer::OnWindowResize(MyCore::WindowResizeEvent& event)
	{
		m_PersCamera->width = event.GetWidth();
		m_PersCamera->height = event.GetHeight();

		return false;
	}

	void Renderer::RenderCubeMapToTexture()
	{
		m_EnvMapConversionShader->Bind();

		m_EnvMapConversionShader->SetUniformInt("u_EquirectangularMap", ENV_MAP_FLAT_TEX_SLOT);
		m_EnvMap->Bind(ENV_MAP_FLAT_TEX_SLOT);

		glViewport(0, 0, m_EnvMapFB->m_Width, m_EnvMapFB->m_Height);
		m_EnvMapFB->BindFrameBuffer();

		for (int i = 0; i < 6; ++i)
		{
			m_EnvMapConversionShader->SetUniformMat4("u_ViewProj", ViewProjs[i]);
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

	void Renderer::RenderDiffuseCubeMap()
	{
		m_DiffuseConversionsShader->Bind();

		m_DiffuseConversionsShader->SetUniformInt("u_EnvironmentMap", ENV_MAP_CUBE_TEX_SLOT);
		m_EnvMapFB->BindToTextureSlot(ENV_MAP_CUBE_TEX_SLOT);

		glViewport(0, 0, m_DiffuseFB->m_Width, m_DiffuseFB->m_Height);
		m_DiffuseFB->BindFrameBuffer();

		for (int i = 0; i < 6; ++i)
		{
			m_DiffuseConversionsShader->SetUniformMat4("u_ViewProj", ViewProjs[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_DiffuseFB->m_CubeMapId, 0);
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

	void Renderer::RenderGlossyCubeMap()
	{
		m_GlossyConversionsShader->Bind();

		m_GlossyConversionsShader->SetUniformInt("u_EnvironmentMap", ENV_MAP_CUBE_TEX_SLOT);
		m_EnvMapFB->BindToTextureSlot(ENV_MAP_CUBE_TEX_SLOT);

		glViewport(0, 0, m_GlossyFB->m_Width, m_GlossyFB->m_Height);
		m_GlossyFB->BindFrameBuffer();

		const unsigned int maxMipLevels = 5;
		for (unsigned int mipLevel = 0; mipLevel < maxMipLevels; ++mipLevel) 
		{
			// Resize our frame buffer according to our mip level
			unsigned int mipWidth = static_cast<unsigned int>(m_GlossyFB->m_Width * std::pow(0.5, mipLevel));
			unsigned int mipHeight = static_cast<unsigned int>(m_GlossyFB->m_Height * std::pow(0.5, mipLevel));
			m_GlossyFB->BindRenderBuffer(mipWidth, mipHeight);
			glViewport(0, 0, mipWidth, mipHeight);

			float roughness = static_cast<float>(mipLevel) / static_cast<float>(maxMipLevels - 1);
			m_GlossyConversionsShader->SetUniformFloat("u_Roughness", roughness);

			for (int i = 0; i < 6; ++i) 
			{
				m_GlossyConversionsShader->SetUniformMat4("u_ViewProj", ViewProjs[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_GlossyFB->m_CubeMapId, mipLevel);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				m_CubeVertBuffer->Bind();
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);

				m_CubeIdxBuffer->Bind();
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			}
		}
		// reset to the default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Renderer::OnUpdate()
	{
		double time = static_cast<float>(WindowsWindow::GetWindow()->GetTime());

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

		// Use SDF to render sphere and plane
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		m_SDF_PBRShader->Bind();

		m_DiffuseFB->BindToTextureSlot(DIFFUSE_IRRADIANCE_CUBE_TEX_SLOT);
		m_SDF_PBRShader->SetUniformInt("u_DiffuseIrradianceMap", DIFFUSE_IRRADIANCE_CUBE_TEX_SLOT);

		m_GlossyFB->BindToTextureSlot(GLOSSY_IRRADIANCE_CUBE_TEX_SLOT);
		m_SDF_PBRShader->SetUniformInt("u_GlossyIrradianceMap", GLOSSY_IRRADIANCE_CUBE_TEX_SLOT);

		m_BRDFLookUpTex->Bind(BRDF_LUT_TEX_SLOT);
		m_SDF_PBRShader->SetUniformInt("u_BRDFLookupTexture", BRDF_LUT_TEX_SLOT);

		m_SDF_PBRShader->SetUniformFloat3("u_CamPos", m_PersCamera->position);
		m_SDF_PBRShader->SetUniformFloat3("u_Forward", m_PersCamera->forward);
		m_SDF_PBRShader->SetUniformFloat3("u_Right", m_PersCamera->right);
		m_SDF_PBRShader->SetUniformFloat3("u_Up", m_PersCamera->up);
		m_SDF_PBRShader->SetUniformFloat2("u_ScreenDims", {m_PersCamera->width, m_PersCamera->height });

		m_SquareVertBuffer->Bind();
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), (void*)(0));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		m_SquareIdxBuffer->Bind();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		m_ImguiWrapper->Begin();
		RenderImGui();
		m_ImguiWrapper->End();

		FrameTime = static_cast<float>(WindowsWindow::GetWindow()->GetTime()) - time;
	}

	void Renderer::RenderImGui()
	{
		ImGui::Begin("Configuration");
		ImGui::Text("Frame Time: %f", FrameTime);
		ImGui::Text("fps: %f", 1.f / FrameTime);
		ImGui::End();

		ImGui::Begin("Material");
		{
			static glm::vec3 albedo(1.f);
			static float metallic = 1.f;
			static float roughness = 0.4f;
			
			ImGui::ColorEdit3("Albedo", &albedo.x);
			ImGui::DragFloat("Metallic", &metallic, 0.01f, 0.f, 1.f);
			ImGui::DragFloat("Roughness", &roughness, 0.01f, 0.f, 1.f);

			m_SDF_PBRShader->SetUniformFloat3("u_Albedo", albedo);
			m_SDF_PBRShader->SetUniformFloat("u_Metallic", metallic);
			m_SDF_PBRShader->SetUniformFloat("u_Roughness", roughness);
			m_SDF_PBRShader->SetUniformFloat("u_AmbientOcclusion", 1.f);

			static float subsurfaceScattering = 0.0f;
			static float distortion = 0.2f;
			static float glow = 6.f;
			static float ambient = 0.0f;
			static float scale = 3.0f;

			ImGui::DragFloat("Subsurface Scattering", &subsurfaceScattering, 0.1f, 0.f, 1.f);
			ImGui::DragFloat("Distrotion", &distortion, 0.05f, 0.f, 1.f);
			ImGui::DragFloat("Glow Scattering", &glow, 0.5f, 0.f, 10.f);
			ImGui::DragFloat("Ambient", &ambient, 0.01f, 0.f, 1.f);
			ImGui::DragFloat("Scale", &scale, 0.5f, 1.f, 10.f);

			m_SDF_PBRShader->SetUniformFloat("u_SScattering", subsurfaceScattering);
			m_SDF_PBRShader->SetUniformFloat("u_Distortion", distortion);
			m_SDF_PBRShader->SetUniformFloat("u_Glow", glow);
			m_SDF_PBRShader->SetUniformFloat("u_Ambient", ambient);
			m_SDF_PBRShader->SetUniformFloat("u_Scale", scale);
			m_SDF_PBRShader->SetUniformFloat("u_K", 2.f);
		}
		ImGui::End();

		ImGui::Begin("Light and Shadow");
		{
			static glm::vec3 light_col(1.f);
			static glm::vec3 light_pos(10, 10, 10);
			static float light_strength = 10.0f;
			static float light_radius = 2.0f;
			static float shadow_darkness = 0.5f;

			ImGui::ColorEdit3("Light Color", &light_col.x);
			ImGui::DragFloat3("Light Position", &light_pos.x);
			ImGui::DragFloat("Light Strength", &light_strength, 0.5f, 1.f, 100.f);
			ImGui::DragFloat("Light Raduis", &light_radius, 0.01f, 0.1f, 5.f);
			ImGui::DragFloat("Shadow Darkness", &shadow_darkness, 0.01f, 0.f, 1.f);

			m_SDF_PBRShader->SetUniformFloat3("u_LightCol", light_strength * light_col);
			m_SDF_PBRShader->SetUniformFloat3("u_LightPos", light_pos);
			m_SDF_PBRShader->SetUniformFloat("u_LightRadius", light_radius);
			m_SDF_PBRShader->SetUniformFloat("u_ShadowDarkness", shadow_darkness);
		}
		ImGui::End();
	}
}