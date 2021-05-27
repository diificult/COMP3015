#version 460

in vec3 Position;
in vec3 Normal;
in vec4 ShadowCoord;
in vec2 TexCoord;

uniform struct MaterialInfo {
vec3 Ka; // Ambient reflectivity
 vec3 Kd; // Diffuse reflectivity
 vec3 Ks; // Specular reflectivity
 float Shininess; // Specular shininess factor
} Material;

uniform struct LightInfo {
vec4 Position;
vec3 Intensity;
}Light;
uniform sampler2D NoiseTex;
layout (location = 0) out vec4 FragColor;

uniform sampler2DShadow ShadowMap;

uniform float LowThreshold;
uniform float HighThreshold;

uniform bool Wall = false;

vec3 phongModelDiffAandSpec()
{

    vec3 n = Normal;
    vec3 s = normalize(vec3(Light.Position) - Position);
    vec3 v = normalize(-Position.xyz);
    vec3 r = reflect(-s, n);
    float sDotN = max(dot(s, n), 0.0);
    vec3 diffuse =Light.Intensity * Material.Kd * sDotN;
 
    vec3 spec = vec3(0.0);
    if (sDotN > 0.0) {
        spec = Light.Intensity * Material.Ks  * pow(max(dot(r, v), 0.0), Material.Shininess);
     }

     return diffuse + spec;
}

subroutine void RenderPassType(bool theash);
subroutine uniform RenderPassType RenderPass;

subroutine (RenderPassType)
void shadeWithShadow(bool thresh) {
    
    vec3 ambient = Light.Intensity * Material.Ka;

    vec3 diffAndSpec = phongModelDiffAandSpec();

    float sum = 0;
    float shadow = 1.0;

    if (ShadowCoord.z > 0) {
        sum += textureProjOffset(ShadowMap, ShadowCoord, ivec2(-1, -1));
        sum += textureProjOffset(ShadowMap, ShadowCoord, ivec2(-1, 1));
        sum += textureProjOffset(ShadowMap, ShadowCoord, ivec2(1, 1));
        sum += textureProjOffset(ShadowMap, ShadowCoord, ivec2(1, -1));
        shadow = sum  * 0.25;
    }


    if (!thresh || Wall){
        FragColor = vec4(diffAndSpec * shadow + ambient, 1.0);

        FragColor = pow(FragColor, vec4(1.0 / 2.2));
    } else {
        FragColor = vec4 (1.0f);
    }

}

subroutine (RenderPassType)
void recordDepth(bool theash) {

}

void main() {
    vec4 noise = texture(NoiseTex,TexCoord);
    bool thresh = false;
   if (noise.a < LowThreshold) thresh = true;
    if (noise.a > HighThreshold) thresh = true;

   RenderPass(thresh);
}
