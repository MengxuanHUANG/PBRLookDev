#pragma once

#include "Imgui/imguiWrapper.h"

#include "shader.h"
#include "buffer.h"
#include "texture.h"
#include "framebuffer.h"
#include "cameraController.h"

namespace PBRLookDev
{
	class ImguiWrapper;

	class Renderer
	{
	public:
		Renderer() = default;
		~Renderer() = default;

		void Init(unsigned int w, unsigned int h);

		bool OnEvent(MyCore::Event& event);
		void OnUpdate();

		void RenderImGui();

		bool OnWindowResize(MyCore::WindowResizeEvent& event);

	public:
		void RenderCubeMapToTexture();
		void RenderDiffuseCubeMap();
		void RenderGlossyCubeMap();

	public:
		// camera
		uPtr<PerspectiveCamera> m_PersCamera;
		uPtr<PerspectiveCameraController> m_CamController;

		// shaders
		uPtr<OpenglShader> m_EnvMapConversionShader;
		uPtr<OpenglShader> m_EnvMapShader;
		uPtr<OpenglShader> m_DiffuseConversionsShader;
		uPtr<OpenglShader> m_GlossyConversionsShader;
		uPtr<OpenglShader> m_SDF_PBRShader;
		uPtr<OpenglShader> m_PostProcessShader;

		// buffers & textures
		uPtr<OpenglBuffer> m_CubeVertBuffer;
		uPtr<OpenglBuffer> m_CubeIdxBuffer;
		uPtr<OpenglBuffer> m_SquareVertBuffer;
		uPtr<OpenglBuffer> m_SquareIdxBuffer;
		uPtr<OpenglTexture> m_EnvMap;
		uPtr<OpenglTexture> m_BRDFLookUpTex;

		// frame buffers
		uPtr<CubeMapFrameBuffer> m_EnvMapFB;
		uPtr<CubeMapFrameBuffer> m_DiffuseFB;
		uPtr<CubeMapFrameBuffer> m_GlossyFB;
		uPtr<GFrameBuffer> m_GFrameFB;

		uPtr<ImguiWrapper> m_ImguiWrapper;
		float FrameTime;
	};
}