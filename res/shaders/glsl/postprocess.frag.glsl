#version 330 core

in vec2 fs_UV;
layout(location = 0) out vec3 fs_Color;

uniform sampler2D u_RenderedTexture;
uniform sampler2D u_PositionTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_DepthRoughnessTexture;

uniform vec3 u_CamPos;
uniform vec3 u_Forward;
uniform vec2 u_ScreenDims;
uniform mat4 u_ViewProj;

const float MAX_DEPTH = 30.f;
const float Step = 0.01f;
const float Thickness = 0.0006f;

float map(float value, float min1, float max1, float min2, float max2) 
{
	return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

vec2 WorldToUV(in vec3 position)
{
	vec4 screen_space = u_ViewProj * vec4(position, 1.f);
	screen_space /= screen_space.w;
	vec2 uv = screen_space.xy;

	return (vec2(1.f) + uv) / vec2(2.f);
}

void main()
{
	vec3 color = texture(u_RenderedTexture, fs_UV).rgb;
	vec3 pos = texture(u_PositionTexture, fs_UV).rgb;
	vec3 nor = normalize(texture(u_NormalTexture, fs_UV).rgb);
	vec3 depth_roughness = texture(u_DepthRoughnessTexture, fs_UV).rgb;

	float cur_depth = depth_roughness.x;
	float roughness = depth_roughness.y;
	float ssr = depth_roughness.z;

	if (cur_depth > 0.f)
	{
		if (ssr == 1.f)
		{
			vec3 woW = normalize(u_CamPos - pos);
			vec3 wiW = reflect(-woW, nor);
			vec3 world_end = pos + wiW * MAX_DEPTH;

			// World space ray marching

			vec3 query = pos;
			
			for (float t = 0.f; t < MAX_DEPTH; t += Step)
			{
				query += Step * wiW;
				float query_depth = length(query - u_CamPos);
				vec2 query_uv = WorldToUV(query);
				if (length(query_uv - fs_UV) < 0.01f) continue;
				if (query_uv.x > 1.f || query_uv.x < 0.f || query_uv.y > 1.f || query_uv.y < 0.f)
				{
					break;
				}
			
				float target_depth = texture(u_DepthRoughnessTexture, query_uv).x;
				if (query_depth - target_depth > 0.f && query_depth - target_depth < 0.04f)
				{
					float atten = 1.0 - t / MAX_DEPTH;
					color = texture(u_RenderedTexture, query_uv).rgb;
					break;
				}
			}
		}
	}
	fs_Color = color;
}