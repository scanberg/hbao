#version 150

out vec4 out_frag0;
out float out_frag1;

in float Depth;
in float Z;

void main(void)
{
	out_frag0 = vec4(1.0, 0.0, 0.0, 1.0);
	//out_frag1 = Depth;
	out_frag1 = Z;
}