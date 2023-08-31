#version 330 core

layout(location = 0) out vec4 color;

uniform sampler2D u_Texture;

in vec3 fs_Position;
in vec3 fs_Color;

void main()
{
	color = vec4(texture(u_Texture, vec2(0.25f, 0.0f)).rgb, 1.f);
}