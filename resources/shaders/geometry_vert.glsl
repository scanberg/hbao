#version 150

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
 
in vec3 in_position;

out vec3 WorldPos;
out vec3 ViewSpacePos;
 
void main(void)
{
	vec4 worldPos = modelMatrix * vec4(in_position, 1.0);
	vec4 viewSpacePos = viewMatrix * worldPos;
	gl_Position = projMatrix * viewSpacePos;
	ViewSpacePos = viewSpacePos.xyz;
	WorldPos = worldPos.xyz;
}