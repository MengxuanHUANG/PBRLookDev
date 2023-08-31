#pragma once
#include "Core/core.h"
#include "GL/glew.h"

namespace PBRLookDev
{
	class OpenglTexture
	{
	public:
		OpenglTexture(const std::string& path, bool repeat = true);
		~OpenglTexture();

		void Bind(GLuint slot);

	public:
		GLuint m_TextureId;
		float* m_RawData;
		int m_Width, m_Height;
	};
}