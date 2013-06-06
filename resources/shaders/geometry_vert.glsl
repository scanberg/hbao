#version 150

uniform mat4 viewMatrix;
uniform mat4 projMatrix;
 
in vec3 in_position;
in vec2 in_texCoord;

out float Depth;
out float Z;

out vec2 TexCoord;
 
void main(void)
{
	vec4 depthVec = projMatrix * viewMatrix * vec4(in_position, 1.0);
	Depth = ((depthVec.z / depthVec.w) + 1.0) * 0.5; 
	Z = (viewMatrix * vec4(in_position, 1.0)).z;

	TexCoord = in_texCoord;

	gl_Position = depthVec;
}