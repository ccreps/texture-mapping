#version 330 core
// pasthrough Fragment shader 
// Last update October 30, 2020
// 
// This version is used for testing and debugging
// it simply gets the color passed through from the 
// vertex shader.  No lighting calculations are done.
uniform sampler2D tex;

uniform vec3 ambientLight;
uniform vec3 directionalLightDirection;
uniform vec3 directionalLightColor;
uniform vec3 halfVector;
uniform float shininess;
uniform float strength;
uniform int   mode;

in vec3 Normal;
in vec4 Color;
in vec2 TexCoord;

out vec4 FragColor;
void main()
{
	vec4 texturecolor = texture(tex, TexCoord);
    FragColor = vec4(texturecolor.rgb, 1.0);
}