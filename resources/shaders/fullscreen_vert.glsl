#version 150

in vec3 in_position;
in vec2 in_texCoord;

out vec2 TexCoord;
out vec2 Position;
 
void main(void)
{
	gl_Position = vec4(in_position, 1.0);
	TexCoord = in_texCoord;
	Position = in_texCoord * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
}