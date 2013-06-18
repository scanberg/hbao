// This is a bilateral blur shader designed to blur the results of the HBAO
// The weights for the cross bilateral weights is given in the slides given by dice
// http://dice.se/publications/stable-ssao-in-battlefield-3-with-selective-temporal-filtering/

#version 150

uniform sampler2D texture0;

uniform vec2 FullRes = vec2(1024.0, 768.0);
uniform vec2 InvFullRes = 1.0 / vec2(1024.0, 768.0);

uniform vec2 LinMAD;

#define KERNEL_RADIUS 8.0

out vec2 out_frag0;

in vec2 TexCoord;

vec2 SampleAOZ(vec2 uv)
{
    return texture(texture0, TexCoord + uv * InvFullRes).rg;
}

vec2 PointSampleAOZ(vec2 uv)
{
	ivec2 coord = ivec2(round(gl_FragCoord.xy + uv));
	return texelFetch(texture0, coord, 0).rg;
}

float CrossBilateralWeight(float r, float z, float z0)
{
	const float BlurSigma = (KERNEL_RADIUS+1.0f) * 0.5f;
	const float BlurFalloff = 1.0f / (2.0f*BlurSigma*BlurSigma);

	float dz = z0 - z;
	return exp2(-r*r*BlurFalloff - dz*dz);
}

void main(void)
{
	vec2 aoz = SampleAOZ(vec2(0));
	float center_z = aoz.y;

	float w = 1.0;
	float total_ao = aoz.x * w;
	float total_weight = w;
	float i = 1.0;

	for(; i <= KERNEL_RADIUS/2; i += 1.0)
	{
		aoz = SampleAOZ( vec2(0,i) );
		w = CrossBilateralWeight(i, aoz.y, center_z);
		total_ao += aoz.x * w;
		total_weight += w;

		aoz = SampleAOZ( vec2(0,-i) );
		w = CrossBilateralWeight(i, aoz.y, center_z);
		total_ao += aoz.x * w;
		total_weight += w;
	}

	for(; i <= KERNEL_RADIUS; i += 2.0)
	{
		aoz = SampleAOZ( vec2(0,0.5+i) );
		w = CrossBilateralWeight(i, aoz.y, center_z);
		total_ao += aoz.x * w;
		total_weight += w;

		aoz = SampleAOZ( vec2(0,-0.5-i) );
		w = CrossBilateralWeight(i, aoz.y, center_z);
		total_ao += aoz.x * w;
		total_weight += w;
	}

	float ao = total_ao / total_weight;
	out_frag0 = vec2(ao, center_z);
}