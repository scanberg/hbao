#version 150

uniform sampler2D texture0;

uniform float BlurDepthThreshold = 1.0;
uniform float BlurRadius = 15.0;

out float out_frag0;

in vec2 TexCoord;

float CrossBilateralWeight(float r, float d, float d0)
{
    return exp2(-r*r*g_BlurFalloff) * (abs(d - d0) < BlurDepthThreshold);
}

void main(void)
{
	out_frag0 = 0.0;

	
}