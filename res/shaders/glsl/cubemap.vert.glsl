#version 330 core
layout(location = 0) in vec3 vs_Pos;

out vec3 fs_Pos;

uniform mat4 u_View;
uniform mat4 u_Proj;

void main() 
{
    fs_Pos = vs_Pos;
    gl_Position = u_Proj * u_View * vec4(fs_Pos, 1.f);
}
