#version 150

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
 
in vec3 in_position;

out float NormalizedDepth;
out vec3 ViewSpacePos;
 
void main(void)
{
	vec4 viewSpacePos = viewMatrix * modelMatrix * vec4(in_position, 1.0);
	gl_Position = projMatrix * viewSpacePos;
	NormalizedDepth = (-viewSpacePos.z - 0.1) / (10.0 - 0.1);
	ViewSpacePos = viewSpacePos.xyz;
}