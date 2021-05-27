#version 460

struct LightInfo {
    vec4 Position;
    vec3 Intensity;
};
uniform LightInfo Light;

struct MaterialInfo {
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float Shininess;
};
uniform MaterialInfo Material;

uniform struct LineInfo {
float Width;
vec4 Color;
} Line;

in vec3 GPosition;
in vec3 GNormal;
noperspective in vec3 GEdgeDistance;

layout (location = 0) out vec4 FragColor;

void phoneModel() {

}
vec3 blinnPhong(vec3 position, vec3 normal)
{

    vec3 ambient = Material.Ka * Light.Intensity;
    vec3 s = normalize(Light.Position.xyz - position);

    float sDotN = max(dot(s, normal), 0.0);

    vec3 diffuse = Material.Kd * sDotN;

    vec3 spec = vec3(0.0);
    if (sDotN > 0.0) {
        vec3 v = normalize(-position.xyz);
        vec3 h = normalize(v  + s);
        
        spec = Material.Ks  * pow(max(dot(h, normal), 0.0), Material.Shininess);
     }

     return ambient + Light.Intensity * (diffuse + spec);
}

vec3 phong(vec3 position, vec3 normal) {

    vec3 ambient = Material.Ka * Light.Intensity;
    vec3 s = normalize(Light.Position.xyz - position);
    float sDotN = max(dot(s, normal), 0.0);
    vec3 r = -s + 2 * (sDotN) * normal;
    vec3 v = normalize(-position.xyz);
    vec3 spec = Material.Ks * Light.Intensity * pow(max((dot(r, v)), 0.0), Material.Shininess);
    vec3 diffuse = Material.Kd * Light.Intensity * sDotN;

    return ambient + spec + diffuse;
}

void main()
{
    //calculate the color
    vec4 color = vec4(blinnPhong(GPosition, GNormal), 1.0);
    // Find the smallest distance for the fragment
    float d = min( GEdgeDistance.x, GEdgeDistance.y);
    d = min( d, GEdgeDistance.z );
    float mixVal;
    if( d < Line.Width - 1)
    {
        mixVal = 1.0;
    }
    else if( d > Line.Width + 1)
    {
        mixVal = 0.0;
    }
    else
    {
        float x = d - (Line.Width - 1);
        mixVal = exp2(-2.0 * (x*x));
    }
    FragColor = mix( color, Line.Color, mixVal );
}
