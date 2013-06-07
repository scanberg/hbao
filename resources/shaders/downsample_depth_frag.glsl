#version 150

uniform sampler2D texture0;

out float out_frag0;

in vec2 TexCoord;

void main(void)
{
	out_frag0 = 1.0;
	//gl_FragDepth = texelFetch(texture0, ivec2(gl_FragCoord.xy)*2, 0).r;
	//gl_FragDepth = texture(texture0, TexCoord).r;
	gl_FragDepth = sin(TexCoord.x*400.0)*0.5 + 0.5;
}