#version 330 core

in vec3 fs_Pos;
out vec4 out_Col;
uniform sampler2D u_EquirectangularMap;

const vec2 normalize_uv = vec2(0.1591f, 0.3183f);// 1 / 2pi , 1/ pi

vec2 sampleSphericalMap(vec3 v) 
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= normalize_uv;
    uv += 0.5f;
    return uv;
}

void main() 
{
    vec2 uv = sampleSphericalMap(normalize(fs_Pos));
    vec3 color = texture(u_EquirectangularMap, uv).rgb;

    out_Col = vec4(color, 1.0f);
}
