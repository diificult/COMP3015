#version 460

const float PI = 3.14159265;


layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexVelocity;
layout (location = 2) in float VertexAge;

uniform int Pass;

layout (xfb_buffer = 0, xfb_offset = 0) out vec3 Position;
layout (xfb_buffer = 1, xfb_offset = 0) out vec3 Velocity;
layout (xfb_buffer = 2, xfb_offset = 0) out float Age;

out float Transp;
out vec2 TexCoord;

uniform float Time;
uniform float DeltaT;
uniform vec3 Accel;
uniform float ParticleLifetime;
uniform float ParticleSize;
uniform vec3 Emitter = vec3(0);
uniform mat3 EmitterBasis;

uniform mat4 MV;
uniform mat4 Proj;

uniform sampler1D RandomTex;

const vec3 offsets[] = vec3[](
vec3(-0.5,-0.5,0),
vec3(0.5,-0.5,0),
vec3(0.5,0.5,0),
vec3(-0.5,-0.5,0),
vec3(0.5,0.5,0),
vec3(-0.5,0.5,0));

const vec2 texCoords[] = vec2[](
vec2(0,0),
vec2(1,0),
vec2(1,1),
vec2(0,0),
vec2(1,1),
vec2(0,1)
);


uniform float MinParticleSize = 0.1; // Minimum size
uniform float MaxParticleSize = 2.5; // Maximum size


vec3 randomInitialVelocity() {
    float velocity = mix(0.1, 0.5, texelFetch(RandomTex, 2 * gl_VertexID, 0).r);

    return EmitterBasis * vec3(0, velocity, 0);

}

vec3 randomInitialPosition() {
    float offset = mix(-2.0, 2.0, texelFetch(RandomTex, 2 * gl_VertexID + 1, 0).r);
    return Emitter + vec3 (offset, 0, 0);
}

void update(){
    Age = VertexAge + DeltaT;

    if (VertexAge < 0 || VertexAge > ParticleLifetime) {
        Position = Emitter;
        Velocity = randomInitialVelocity();
        if(VertexAge > ParticleLifetime ) Age = (VertexAge - ParticleLifetime)+ DeltaT;
    } else {
        Position = VertexPosition + VertexVelocity * DeltaT;
        Velocity = VertexVelocity + Accel * DeltaT;
    }
}

void render() {
 Transp = 0.0;
 vec3 posCam = vec3(0.0);
 if( VertexAge >= 0.0 ) {
 float agePct = VertexAge / ParticleLifetime;
 Transp = clamp(1.0 - agePct, 0, 1);
 posCam =
 (MV * vec4(VertexPosition,1)).xyz +
 offsets[gl_VertexID] *
 mix(MinParticleSize, MaxParticleSize, agePct);
 }
 TexCoord = texCoords[gl_VertexID];
 gl_Position = Proj * vec4(posCam,1);
}

void main()
{
    if (Pass == 1)
        update();
    else render();
}
