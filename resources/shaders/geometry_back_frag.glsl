#version 150

out float out_frag0;

in float Depth;

void main(void)
{
	out_frag0 = Depth;
}