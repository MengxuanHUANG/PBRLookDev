#version 330 core

in vec2 fs_UV;
layout(location = 0) out vec3 fs_Color;

uniform sampler2D u_RenderedTexture;

void main()
{
	vec3 color = texture(u_RenderedTexture, fs_UV).rgb;
	fs_Color = color;
}