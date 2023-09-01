#version 330 core

uniform vec3 u_CamPos;
uniform vec3 u_Forward, u_Right, u_Up;
uniform vec2 u_ScreenDims;

// metallic workflow attrib
uniform vec3 u_Albedo;
uniform float u_Metallic;
uniform float u_Roughness;
uniform float u_AmbientOcclusion;

// IBL
uniform samplerCube u_DiffuseIrradianceMap;
uniform samplerCube u_GlossyIrradianceMap;
uniform sampler2D u_BRDFLookupTexture;

in vec2 fs_UV;
out vec4 out_Col;

const float PI = 3.14159f;
const float T_MAX = 200.f;
const int MAX_ITERATIONS = 50;
const float ISECT_EPSILON = 0.01f;
const float MAX_REFLECTION_LOD = 4.0;

struct Ray 
{
    vec3 origin;
    vec3 direction;
};

struct BSDF {
    vec3 pos;
    vec3 nor;
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

struct MarchResult {
    float t;
    int hitSomething;
    BSDF bsdf;
};

float dot2(in vec2 v) { return dot(v, v); }
float dot2(in vec3 v) { return dot(v, v); }
float ndot(in vec2 a, in vec2 b) { return a.x * b.x - a.y * b.y; }

float sceneSDF(vec3 query);
vec3 metallic_workflow(BSDF bsdf, vec3 wo);

void coordinateSystem(in vec3 nor, out vec3 tan, out vec3 bitan)
{
    vec3 up = abs(nor.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(1.f, 0.f, 0.f);
    tan = normalize(cross(up, nor));
    bitan = cross(nor, tan);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 SDF_Normal(vec3 query) 
{
    vec2 epsilon = vec2(0.0, 0.001);
    return normalize(vec3(sceneSDF(query + epsilon.yxx) - sceneSDF(query - epsilon.yxx),
        sceneSDF(query + epsilon.xyx) - sceneSDF(query - epsilon.xyx),
        sceneSDF(query + epsilon.xxy) - sceneSDF(query - epsilon.xxy)));
}

float SDF_Sphere(vec3 query, vec3 center, float radius) {
    return length(query - center) - radius;
}

float subtract(float d1, float d2) {
    return max(d1, -d2);
}

float opIntersection(float d1, float d2) {
    return max(d1, d2);
}

float opOnion(float sdf, float thickness) {
    return abs(sdf) - thickness;
}

vec3 rotateX(vec3 p, float angle) {
    angle = angle * 3.14159 / 180.f;
    float c = cos(angle);
    float s = sin(angle);
    return vec3(p.x, c * p.y - s * p.z, s * p.y + c * p.z);
}

vec3 rotateZ(vec3 p, float angle) {
    angle = angle * 3.14159 / 180.f;
    float c = cos(angle);
    float s = sin(angle);
    return vec3(c * p.x - s * p.y, s * p.x + c * p.y, p.z);
}

float sceneSDF(vec3 query) 
{
    return SDF_Sphere(query, vec3(0.), 1.f);
}

BSDF sceneBSDF(vec3 query) 
{
    return BSDF(query, SDF_Normal(query), u_Albedo,
        u_Metallic, u_Roughness, u_AmbientOcclusion);
}

#define FOVY 45 * PI / 180.f
Ray rayCast() 
{
    vec2 ndc = fs_UV;
    ndc = ndc * 2.f - vec2(1.f);

    float aspect = u_ScreenDims.x / u_ScreenDims.y;
    vec3 ref = u_CamPos + u_Forward;
    vec3 V = u_Up * tan(FOVY * 0.5);
    vec3 H = u_Right * tan(FOVY * 0.5) * aspect;
    vec3 p = ref + H * ndc.x + V * ndc.y;

    return Ray(u_CamPos, normalize(p - u_CamPos));
}

#define MAX_ITERATIONS 128
MarchResult raymarch(Ray ray) 
{
    float t = 0.f;
    int it = 0;
    for (; it < MAX_ITERATIONS && t < T_MAX; ++it)
    {
        vec3 query_point = ray.origin + t * ray.direction;
        float distance = sceneSDF(query_point);
        t += distance;

        if (abs(distance) <= ISECT_EPSILON)
        {
            return MarchResult(t, it, sceneBSDF(query_point));
        }
    }
    return MarchResult(-1, 0, BSDF(vec3(0.), vec3(0.), vec3(0.), 0., 0., 0.));
}

void main()
{
    Ray ray = rayCast();
    MarchResult result = raymarch(ray);
    BSDF bsdf = result.bsdf;
    vec3 pos = ray.origin + result.t * ray.direction;

    //vec3 color = 0.5f * (bsdf.nor + 1.f);

    vec3 color = metallic_workflow(bsdf, -ray.direction);

    color = color / (color + vec3(1.0)); // Reinhard
    color = pow(color, vec3(1.0 / 2.2)); // Gamma correction

    out_Col = vec4(color, result.hitSomething > 0 ? 1. : 0.);
}

vec3 metallic_workflow(BSDF bsdf, vec3 wo) {
    vec3 surfacePoint = bsdf.pos;
    vec3 N = bsdf.nor;
    vec3 albedo = bsdf.albedo;
    float metallic = bsdf.metallic;
    float roughness = bsdf.roughness;
    float ambientOcclusion = bsdf.ao;

    vec3 woW = normalize(u_CamPos - surfacePoint);
    vec3 wiW = reflect(-woW, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // compute glossy
    vec3 grossy = textureLod(u_GlossyIrradianceMap, wiW, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 BrdfLookUp = texture(u_BRDFLookupTexture, vec2(max(dot(N, woW), 0.0), roughness)).rg;

    // compute fresnel
    vec3 ks = fresnelSchlick(max(dot(N, woW), 0.f), F0);
    vec3 kd = vec3(1.f) - ks;
    kd *= 1.0 - metallic;

    // compute diffuse
    vec3 irradiance = texture(u_DiffuseIrradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    // compute specular
    vec3 specular = grossy * (ks * BrdfLookUp.x + BrdfLookUp.y);
    vec3 ambient = (kd * diffuse + specular) * ambientOcclusion;
    vec3 color = ambient;

    return color;
}