#version 150

uniform sampler2D texture0;
uniform sampler2D texture1;

uniform vec2 LinMAD;

out vec2 out_frag0;

in vec2 TexCoord;

float ViewSpaceZFromDepth(float d)
{
	// [0,1] -> [-1,1] clip space
	d = d * 2.0 - 1.0;

	// Get view space Z
	return -1.0 / (LinMAD.x * d + LinMAD.y);
}

void main(void)
{
	//float d = texelFetch(texture0, ivec2(gl_FragCoord.xy + vec2(0.0)), 0).r;
	float d = texture(texture0, TexCoord).r;
	float z = 30 * ViewSpaceZFromDepth(d);
	//z = -30 * pow(abs(z), 0.6);
	float ao = texture(texture1, TexCoord).r;
	out_frag0 = vec2(ao,z);
}