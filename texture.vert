#version 330 core
//
// Pass through Vertex shader.  
// Passes vertex information through without changing it.
//  Being used for debugging purposes.
// 
uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

in vec4 vColor;
in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexture;

out vec4 Color;
out vec3 Normal;
out vec2 TexCoord;

void main()
{
	Color    = vColor;
	TexCoord = vTexture;
	Normal   = vNormal;
    gl_Position = projectionMatrix * modelingMatrix * vPosition;
}
