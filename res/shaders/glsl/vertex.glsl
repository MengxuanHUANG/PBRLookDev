#version 330 core

layout(location = 0) in vec3 vs_Position;
layout(location = 1) in vec3 vs_Color;

uniform mat4  u_ViewProjection;
uniform mat4  u_Transform;

out vec3 fs_Position;
out vec3 fs_Color;

void main()
{
	fs_Position = vs_Position;
	fs_Color = vs_Color;
	gl_Position = u_ViewProjection * u_Transform * vec4(vs_Position, 1.0);
}