#version 460

//in variables, this are in model coordinates
layout (location = 0) in vec3 VertexPosition; 
layout (location = 1) in vec3 VertexNormal; 


//out vector needed for the fragment shader
out vec3 Position; 
out vec3 Normal;

     

//uniforms for matrices required in the shader
uniform mat4 ModelViewMatrix;   //model view matrix
uniform mat3 NormalMatrix;		//normal matrix
uniform mat4 MVP;				//model view projection matrix
 
void main() 
{ 
    Normal = normalize( NormalMatrix * VertexNormal);
	Position = (ModelViewMatrix * vec4(VertexPosition,1.0)).xyz;
    
    gl_Position = MVP * vec4(VertexPosition,1.0);
} 