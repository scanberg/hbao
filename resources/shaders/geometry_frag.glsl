#version 150

out vec4 out_frag0;
out vec2 out_frag1;
out vec3 out_frag2;

in vec3 WorldPos;

void main(void)
{
	out_frag0 = vec4(1.0, 0.0, 0.0, 1.0);
	out_frag1 = vec2(0.0);
	out_frag2 = WorldPos;
}