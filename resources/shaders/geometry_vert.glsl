#version 150

uniform mat4 viewMatrix;
uniform mat4 projMatrix;
 
in vec3 in_position;

out float Depth;
 
void main(void)
{
	vec4 depthVec = projMatrix * viewMatrix * vec4(in_position, 1.0);;
	Depth = ((depthVec.z / depthVec.w) + 1.0) * 0.5; 

	gl_Position = depthVec;
}