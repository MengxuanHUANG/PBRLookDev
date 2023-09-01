#version 330 core

// Compute the irradiance across the entire
// hemisphere aligned with a surface normal
// pointing in the direction of fs_Pos.
// Thus, our surface normal direction
// is normalize(fs_Pos).

in vec3 fs_Pos;
out vec4 out_Col;
uniform samplerCube u_EnvironmentMap;

const float PI = 3.14159265359;

void main()
{
    vec3 normal = normalize(fs_Pos);
    vec3 up = vec3(0.f, 1.f, 0.f);
    vec3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    float sampleDelta = 0.025;
    float numSamples = 0.f;

    vec3 irradiance = vec3(0.f);

    for(float phi = 0.f; phi < 2.f * PI; phi += sampleDelta)
    {
        for(float theta = 0.f; theta < 0.5f * PI; theta += sampleDelta)
        {
            vec3 wi = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 wiW = wi.x * right + wi.y * up + wi.z * normal;

            irradiance += texture(u_EnvironmentMap, wiW).rgb * cos(theta) * sin(theta);
            numSamples += 1.f;
        }
    }
    irradiance *= PI / numSamples;
    out_Col = vec4(irradiance, 1.0);
}
