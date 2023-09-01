#version 330 core
in vec3 fs_Pos;

out vec4 out_Col;

uniform samplerCube u_EnvironmentMap;

void main() 
{
    vec3 envColor = texture(u_EnvironmentMap, fs_Pos).rgb;

    envColor = envColor / (envColor + vec3(1.0));// Reinhard
    envColor = pow(envColor, vec3(1.0/2.2));// gamma correction

    out_Col = vec4(envColor, 1.0);
}
