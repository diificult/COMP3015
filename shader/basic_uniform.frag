#version 460

//in variable that receives the diffuse calculation from the vertex shader
in vec3 Position;
in vec3 Normal;

uniform float EdgeThreshold;

const vec3 lum = vec3(0.2126, 0.7152, 0.0722);



//out variable, this typical for all fragment shaders
layout (location = 0) out vec4 FragColor;
layout( binding=0 ) uniform sampler2D RenderTex;




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

vec3 blinnPhong(vec3 position, vec3 normal)
{

    vec3 ambient = Material.Ka * Light.Intensity;
    vec3 s = normalize(Light.Position.xyz - position);

    float sDotN = max(dot(s, normal), 0.0);

    vec3 diffuse = Material.Kd * sDotN;

    vec3 spec = vec3(0.0);
    if (sDotN > 0.0) {
        vec3 v = normalize(-position.xyz);
        vec3 h = (v  + s);
        
        vec3 spec = Material.Ks  * pow(max(dot(h, normal), 0.0), Material.Shininess);
     }

     return ambient * Light.Intensity + (diffuse + spec);
}



void main()
{
    //we pass LightInyensity to outr FragColor, notice the difference between vector types
    // vec3 and vec4 and how we solved the problem
     
    FragColor = vec4(blinnPhong(Position, Normal), 1.0);
}
