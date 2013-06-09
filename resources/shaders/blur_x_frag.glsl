#version 150

uniform sampler2D texture0;
uniform sampler2D texture1;

uniform vec2 AORes = vec2(1024.0/2, 768.0/2);
uniform vec2 InvAORes = 1.0 / vec2(1024.0, 768.0);
uniform vec2 InvDepthRes = 1.0 / vec2(1024.0, 768.0);

uniform vec2 LinMAD;

uniform float Threshold = 0.1;
uniform float DepthFalloff = 1.0;

uniform float sigma = 35.0;

out float out_frag0;

in vec2 TexCoord;

float ViewSpaceZFromDepth(float d)
{
	// [0,1] -> [-1,1] clip space
	d = d * 2.0 - 1.0;
	// Linearize the depth value
	return -1.0 / (LinMAD.x * d + LinMAD.y);
}

vec2 SampleAOZ(vec2 uv)
{
    return vec2(texture(texture0, TexCoord + uv*InvAORes).r,
    			ViewSpaceZFromDepth(texture(texture1, TexCoord + uv*InvDepthRes).r));
}

vec2 PointSampleAOZ(vec2 uv)
{
	ivec2 coord = ivec2(round(TexCoord * AORes + uv*InvAORes));
	ivec2 dcoord = ivec2(round(TexCoord * 1.0/InvDepthRes + uv*InvDepthRes));
	return vec2(texelFetch(texture0, coord, 0).r,
    			ViewSpaceZFromDepth(texelFetch(texture1, dcoord, 0).r));
}

float GaussianWeight(vec2 coord)
{
	const float PI = 3.14159265;
	const float A = 1.0 / (2.0 * PI * sigma * sigma);
	const float B = - 1.0 / (2.0 * sigma * sigma);

	return A * exp(dot(coord,coord) * B);
}

float ZWeight(float center, float z)
{
	const float epsilon = 0.01;
	return 1.0 / (epsilon + abs(center - z) * DepthFalloff);
	//return float(abs(center - z) < Threshold);
}

void main(void)
{
	vec2 aoz = SampleAOZ(vec2(0));
	float center_depth = aoz.y;

	float w = GaussianWeight(vec2(0));
	float total_ao = aoz.x * w;
	float total_weight = w;

	float kernelradius = sigma / 6.0;

	for(float i=0.5; i<kernelradius; ++i)
	{
		aoz = SampleAOZ( vec2(i,0) );
		w = GaussianWeight( vec2(i,0) ) * ZWeight(center_depth, aoz.y);
		total_ao += aoz.x * w;
		total_weight += w;

		aoz = SampleAOZ( vec2(-i,0) );
		w = GaussianWeight( vec2(-i,0) ) * ZWeight(center_depth, aoz.y);
		total_ao += aoz.x * w;
		total_weight += w;
	}

	out_frag0 = total_ao / total_weight;
}