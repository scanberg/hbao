#version 150

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
 
in vec3 in_position;

out vec3 WorldPos;
 
void main(void)
{
	vec4 worldPos = modelMatrix * vec4(in_position, 1.0);
	gl_Position = projMatrix * viewMatrix * worldPos;
	WorldPos = worldPos.xyz;
}