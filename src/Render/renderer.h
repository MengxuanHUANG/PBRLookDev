#pragma once

#include "shader.h"
#include "buffer.h"
#include "texture.h"
#include "framebuffer.h"
#include "cameraController.h"

namespace PBRLookDev
{
	class Renderer
	{
	public:
		Renderer() = default;
		~Renderer() = default;

		void Init(unsigned int w, unsigned int h);

		bool OnEvent(MyCore::Event& event);
		void OnUpdate();
		
	public:
		void RenderCubeMapToTexture();
		void RenderDiffuseCubeMap();
		void RenderGlossyCubeMap();

	public:
		// camera
		uPtr<PerspectiveCamera> m_PersCamera;
		uPtr<PerspectiveCameraController> m_CamController;

		// shaders
		uPtr<OpenglShader> m_TestShader;
		uPtr<OpenglShader> m_EnvMapConversionShader;
		uPtr<OpenglShader> m_EnvMapShader;
		uPtr<OpenglShader> m_DiffuseConversionsShader;
		uPtr<OpenglShader> m_GlossyConversionsShader;
		uPtr<OpenglShader> m_SDF_PBRShader;

		// buffers & textures
		uPtr<OpenglBuffer> m_CubeVertBuffer;
		uPtr<OpenglBuffer> m_CubeIdxBuffer;
		uPtr<OpenglBuffer> m_SquareVertBuffer;
		uPtr<OpenglBuffer> m_SquareIdxBuffer;
		uPtr<OpenglBuffer> m_VertBuffer;
		uPtr<OpenglBuffer> m_IdxBuffer;
		uPtr<OpenglTexture> m_EnvMap;
		uPtr<OpenglTexture> m_BRDFLookUpTex;

		// frame buffers
		uPtr<CubeMapFrameBuffer> m_EnvMapFB;
		uPtr<CubeMapFrameBuffer> m_DiffuseFB;
		uPtr<CubeMapFrameBuffer> m_GlossyFB;
	};
}