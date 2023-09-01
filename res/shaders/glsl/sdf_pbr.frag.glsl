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

//Shadow Light
uniform vec3 u_LightPos;
uniform vec3 u_LightCol;
uniform float u_LightRadius;
uniform float u_ShadowDarkness;

in vec2 fs_UV;
out vec4 fs_Color;

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
    int iteration;
    BSDF bsdf;
};

float dot2(in vec2 v) { return dot(v, v); }
float dot2(in vec3 v) { return dot(v, v); }
float ndot(in vec2 a, in vec2 b) { return a.x * b.x - a.y * b.y; }

float sceneSDF(vec3 query);
vec3 pointLighting(BSDF bsdf, vec3 woW, in vec3 shadowRayDir, in float light_dist);
vec3 metallic_workflow(BSDF bsdf, vec3 wo);
float calculateObstruction(vec3 pos, vec3 shadowRayDir, float lightDist);

void coordinateSystem(in vec3 nor, out vec3 tan, out vec3 bitan)
{
    vec3 up = abs(nor.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(1.f, 0.f, 0.f);
    tan = normalize(cross(up, nor));
    bitan = cross(nor, tan);
}

float DistributionGGX(vec3 normal, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(normal, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 normal, vec3 woW, vec3 wiW, float roughness)
{
    float NdotV = max(dot(normal, woW), 0.0);
    float NdotL = max(dot(normal, -wiW), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
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

float SDF_Sphere(vec3 query, vec3 center, float radius) 
{
    return length(query - center) - radius;
}
float SDF_Plane(vec3 p, vec3 a, vec3 b, vec3 c, vec3 d)
{
    vec3 ba = b - a; vec3 pa = p - a;
    vec3 cb = c - b; vec3 pb = p - b;
    vec3 dc = d - c; vec3 pc = p - c;
    vec3 ad = a - d; vec3 pd = p - d;
    vec3 nor = cross(ba, ad);

    return sqrt(
        (sign(dot(cross(ba, nor), pa)) +
            sign(dot(cross(cb, nor), pb)) +
            sign(dot(cross(dc, nor), pc)) +
            sign(dot(cross(ad, nor), pd)) < 3.0)
        ?
        min(min(min(
            dot2(ba * clamp(dot(ba, pa) / dot2(ba), 0.0, 1.0) - pa),
            dot2(cb * clamp(dot(cb, pb) / dot2(cb), 0.0, 1.0) - pb)),
            dot2(dc * clamp(dot(dc, pc) / dot2(dc), 0.0, 1.0) - pc)),
            dot2(ad * clamp(dot(ad, pd) / dot2(ad), 0.0, 1.0) - pd))
        :
        dot(nor, pa) * dot(nor, pa) / dot2(nor));
}

float subtract(float d1, float d2) 
{
    return max(d1, -d2);
}

float opIntersection(float d1, float d2) 
{
    return max(d1, d2);
}

float opOnion(float sdf, float thickness) 
{
    return abs(sdf) - thickness;
}

vec3 rotateX(vec3 p, float angle) 
{
    angle = angle * 3.14159 / 180.f;
    float c = cos(angle);
    float s = sin(angle);
    return vec3(p.x, c * p.y - s * p.z, s * p.y + c * p.z);
}

vec3 rotateZ(vec3 p, float angle) 
{
    angle = angle * 3.14159 / 180.f;
    float c = cos(angle);
    float s = sin(angle);
    return vec3(c * p.x - s * p.y, s * p.x + c * p.y, p.z);
}

float sceneSDF(vec3 query) 
{
    float sphere_dist = SDF_Sphere(query, vec3(0.f, 0.5f, 0.f), 1.5f);
    float plane_dist = SDF_Plane(query, 
                                vec3(-10.f, -1.f, -10.f),
                                vec3( 10.f, -1.f, -10.f),
                                vec3( 10.f, -1.f,  10.f),
                                vec3(-10.f, -1.f,  10.f));

    return min(sphere_dist, plane_dist);
}

BSDF sceneBSDF(vec3 query) 
{
    float sphere_dist = SDF_Sphere(query, vec3(0.f, 0.5f, 0.f), 1.5f);
    float plane_dist = SDF_Plane(query,
                                    vec3(-10.f, -1.f, -10.f),
                                    vec3(10.f, -1.f, -10.f),
                                    vec3(10.f, -1.f, 10.f),
                                    vec3(-10.f, -1.f, 10.f));
    BSDF result = BSDF(query, SDF_Normal(query), u_Albedo, u_Metallic, u_Roughness, u_AmbientOcclusion);

    if (plane_dist < sphere_dist)
    {
        result.albedo = vec3(1.f);
        result.metallic = 0.f;
        result.roughness = 0.5f;
    }
    return result;
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

    if (result.iteration > 0)
    {
        // shadow ray test
        vec3 shadow_dir = normalize(u_LightPos - pos);
        float light_dist = length(u_LightPos - pos);
        
        float lightStrength = 20.f;
        float obstruction = calculateObstruction(pos, shadow_dir, light_dist);

        float level = (1.f - mix(0.f, u_ShadowDarkness, obstruction));

        vec3 color = pointLighting(bsdf, -ray.direction, shadow_dir, light_dist) +metallic_workflow(bsdf, -ray.direction);
        color = level * color;

        color = color / (color + vec3(1.0)); // Reinhard
        color = pow(color, vec3(1.0 / 2.2)); // Gamma correction
        
        fs_Color = vec4(color, 1.f);
    }
    else fs_Color = vec4(0.f);
    
}

vec3 pointLighting(BSDF bsdf, vec3 woW, in vec3 shadowRayDir, in float light_dist)
{
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, u_Albedo, u_Metallic);

    // compute light intensity falloff
    float falloff = 1.f / (light_dist * light_dist);

    vec3 radiance = falloff * u_LightCol;

    vec3 wiW = -shadowRayDir; // from light to point
    vec3 H = normalize(woW + (-wiW));
    float cosTheta = max(dot(bsdf.nor, -wiW), 0.0);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(bsdf.nor, H, u_Roughness);
    float G = GeometrySmith(bsdf.nor, woW, wiW, u_Roughness);
    vec3 F = fresnelSchlick(clamp(dot(H, woW), 0.0, 1.0), F0);

    vec3 ks = F;
    vec3 kd = (1.f - u_Metallic) * (vec3(1.f) - ks);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(bsdf.nor, woW), 0.0) * max(dot(bsdf.nor, -wiW), 0.0) + 0.0001; // avoid divide by zero
    vec3 specular = numerator / denominator;

    return (kd * u_Albedo / PI + specular) * radiance * cosTheta; // Lo
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

float calculateObstruction(vec3 pos, vec3 shadowRayDir, float lightDist)
{
    float d, t = u_LightRadius * 0.1;
    float obstruction = 0.;
    for (int j = 0; j < 128; j++)
    {
        d = sceneSDF(pos + t * shadowRayDir);
        obstruction = max(0.5 + (-d) * lightDist / (2. * u_LightRadius * t), obstruction);
        if (obstruction >= 1.) break; 

        t += max(d, u_LightRadius * t / lightDist);
        if (t >= lightDist) break;
    }
    return clamp(obstruction, 0., 1.);
}