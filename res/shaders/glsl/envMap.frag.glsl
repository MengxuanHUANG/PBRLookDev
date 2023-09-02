#version 330 core
in vec3 fs_Pos;

layout(location = 0) out vec4 fs_Color;
layout(location = 1) out vec4 g_Position;
layout(location = 2) out vec4 g_Normal;
layout(location = 3) out vec4 g_DepthRougness;

uniform samplerCube u_EnvironmentMap;

void main() 
{
    vec3 envColor = texture(u_EnvironmentMap, fs_Pos).rgb;

    envColor = envColor / (envColor + vec3(1.0));// Reinhard
    envColor = pow(envColor, vec3(1.0/2.2));// gamma correction

    fs_Color = vec4(envColor, 1.0);
    g_Position = vec4(0.f);
    g_Normal = vec4(0.f);
    g_DepthRougness = vec4(0.f);
}
