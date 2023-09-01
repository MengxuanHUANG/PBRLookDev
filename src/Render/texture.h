#pragma once
#include "Core/core.h"
#include "GL/glew.h"

// Texture slot for the 2D HDR environment map
#define ENV_MAP_FLAT_TEX_SLOT 0
// Texture slot for the 3D HDR environment cube map
#define ENV_MAP_CUBE_TEX_SLOT 1
// Texture slot for the 3D HDR diffuse irradiance map
#define DIFFUSE_IRRADIANCE_CUBE_TEX_SLOT 2
// Texture slot for the 3D HDR glossy irradiance map
#define GLOSSY_IRRADIANCE_CUBE_TEX_SLOT 3
// Texture slot for the BRDF lookup texture
#define BRDF_LUT_TEX_SLOT 4

#define ALBEDO_TEX_SLOT 5
#define METALLIC_TEX_SLOT 6
#define ROUGHNESS_TEX_SLOT 7
#define AO_TEX_SLOT 8
#define NORMALS_TEX_SLOT 9
#define DISPLACEMENT_TEX_SLOT 10

namespace PBRLookDev
{
	class OpenglTexture
	{
	public:
		OpenglTexture(const std::string& path, bool repeat = false);
		~OpenglTexture();

		void Bind(GLuint slot);

	public:
		GLuint m_TextureId;
		float* m_RawData;
		int m_Width, m_Height;
	};
}