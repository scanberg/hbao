#version 150

in vec3 in_position;
in vec2 in_texCoord;

out vec2 TexCoord;
 
void main(void)
{
	gl_Position = vec4(in_position, 1.0);
	TexCoord = in_texCoord;
}