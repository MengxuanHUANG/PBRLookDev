#version 330 core

layout(location = 0) out vec4 color;

in vec3 fs_Position;
in vec3 fs_Color;

void main()
{
	color = vec4(fs_Color, 1.f);
}