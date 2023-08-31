#include "texture.h"

#include <stb_image.h>

namespace PBRLookDev
{
	OpenglTexture::OpenglTexture(const std::string& path, bool repeat)
		: m_RawData(nullptr)
	{
		glGenTextures(1, &m_TextureId);
		glBindTexture(GL_TEXTURE_2D, m_TextureId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		const int param = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param);

		// read image
		m_RawData = stbi_loadf(path.c_str(), &m_Width, &m_Height, NULL, 4);
		if (!m_RawData)
		{
			printf("Cannot open %s!\n", path.c_str());
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, m_RawData);
	}

	OpenglTexture::~OpenglTexture()
	{
		if (m_RawData)
		{
			stbi_image_free(m_RawData);
			m_RawData = nullptr;
		}
		glDeleteTextures(1, &m_TextureId);
	}

	void OpenglTexture::Bind(GLuint slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_TextureId);
	}
}