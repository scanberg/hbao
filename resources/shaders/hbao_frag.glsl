#version 150

const float PI = 3.14159265;

uniform sampler2D texture0;
uniform sampler2D texture1;

uniform vec2 FocalLen;
uniform vec2 UVToViewA;
uniform vec2 UVToViewB;

uniform vec2 LinMAD;// = vec2(0.1-10.0, 0.1+10.0) / (2.0*0.1*10.0);

uniform vec2 AORes = vec2(1024.0, 768.0);
uniform vec2 InvAORes = vec2(1.0/1024.0, 1.0/768.0);
uniform vec2 NoiseScale = vec2(1024.0, 768.0) / 4.0;

uniform float R = 0.3;
uniform float R2 = 0.3*0.3;
uniform float NegInvR2 = - 1.0 / (0.3*0.3);
uniform float TanBias = tan(30.0 * PI / 180.0);
uniform float MaxRadiusPixels = 50.0;

uniform int NumDirections = 6;
uniform int NumSamples = 4;

in vec2 TexCoord;
in vec2 Position;

out float out_frag0;

float ViewSpaceZFromDepth(float d)
{
	// [0,1] -> [-1,1] clip space
	d = d * 2.0 - 1.0;

	// Get view space Z
	return -1.0 / (LinMAD.x * d + LinMAD.y);
}

vec3 UVToViewSpace(vec2 uv, float z)
{
	uv = UVToViewA * uv + UVToViewB;
	return vec3(uv * z, z);
}

vec3 GetViewPos(vec2 uv)
{
	//float z = ViewSpaceZFromDepth(texture(texture0, uv).r);
	float z = texture(texture0, uv).r;
	return UVToViewSpace(uv, z);
}

vec3 GetViewPosPoint(ivec2 uv)
{
	ivec2 coord = ivec2(gl_FragCoord.xy) + uv;
	float z = texelFetch(texture0, coord, 0).r;
	return UVToViewSpace(uv, z);
}

float TanToSin(float x)
{
	return x * inversesqrt(x*x + 1.0);
}

float InvLength(vec2 V)
{
	return inversesqrt(dot(V,V));
}

float Tangent(vec3 V)
{
	return V.z * InvLength(V.xy);
}

float BiasedTangent(vec3 V)
{
	return V.z * InvLength(V.xy) + TanBias;
}

float Tangent(vec3 P, vec3 S)
{
    return -(P.z - S.z) * InvLength(S.xy - P.xy);
}

float Length2(vec3 V)
{
	return dot(V,V);
}

vec3 MinDiff(vec3 P, vec3 Pr, vec3 Pl)
{
    vec3 V1 = Pr - P;
    vec3 V2 = P - Pl;
    return (Length2(V1) < Length2(V2)) ? V1 : V2;
}

vec2 SnapUVOffset(vec2 uv)
{
    return round(uv * AORes) * InvAORes;
}

float Falloff(float d2)
{
	return d2 * NegInvR2 + 1.0f;
}

float HorizonOcclusion(	vec2 deltaUV,
						vec3 P,
						vec3 dPdu,
						vec3 dPdv,
						float randstep,
						float numSamples)
{
	float ao = 0;

	vec2 uv = TexCoord + SnapUVOffset(randstep*deltaUV);
	deltaUV = SnapUVOffset( deltaUV );

	vec3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;

	float tanH = BiasedTangent(T);
	float sinH = TanToSin(tanH);

	float tanS;
	float d2;
	vec3 S;

	for(float s = 1; s <= numSamples; ++s)
	{
		uv += deltaUV;
		S = GetViewPos(uv);
		tanS = Tangent(P, S);
		d2 = Length2(S - P);

		if(d2 < R2 && tanS > tanH)
		{
			float sinS = TanToSin(tanS);
			ao += Falloff(d2) * (sinS - sinH);

			tanH = tanS;
			sinH = sinS;
		}
	}
	
	return ao;
}

vec2 RotateDirections(vec2 Dir, vec2 CosSin)
{
    return vec2(Dir.x*CosSin.x - Dir.y*CosSin.y,
                  Dir.x*CosSin.y + Dir.y*CosSin.x);
}

void ComputeSteps(inout vec2 step_size_uv, inout float numSteps, float ray_radius_pix, float rand)
{
    // Avoid oversampling if NUM_STEPS is greater than the kernel radius in pixels
    numSteps = min(NumSamples, ray_radius_pix);

    // Divide by Ns+1 so that the farthest samples are not fully attenuated
    float step_size_pix = ray_radius_pix / (numSteps + 1);

    // Clamp numSteps if it is greater than the max kernel footprint
    float maxNumSteps = MaxRadiusPixels / step_size_pix;
    if (maxNumSteps < numSteps)
    {
        // Use dithering to avoid AO discontinuities
        numSteps = floor(maxNumSteps + rand);
        numSteps = max(numSteps, 1);
        step_size_pix = MaxRadiusPixels / numSteps;
    }

    // Step size in uv space
    step_size_uv = step_size_pix * InvAORes;
}

void main(void)
{
	float numDirections = NumDirections;
	//int numSamples = NumSamples;
	const float strength = 1.9;

	vec3 P, Pr, Pl, Pt, Pb;
	P 	= GetViewPos(TexCoord);
    Pr 	= GetViewPos(TexCoord + vec2( InvAORes.x, 0));
    Pl 	= GetViewPos(TexCoord + vec2(-InvAORes.x, 0));
    Pt 	= GetViewPos(TexCoord + vec2( 0, InvAORes.y));
    Pb 	= GetViewPos(TexCoord + vec2( 0,-InvAORes.y));
 	// P 	= GetViewPosPoint(ivec2(0,0));
  //   Pr 	= GetViewPosPoint(ivec2(1,0));
  //   Pl 	= GetViewPosPoint(ivec2(-1,0));
  //   Pt 	= GetViewPosPoint(ivec2(0,1));
  //   Pb 	= GetViewPosPoint(ivec2(0,-1));

    vec3 dPdu = MinDiff(P, Pr, Pl);
    vec3 dPdv = MinDiff(P, Pt, Pb) * (AORes.y * InvAORes.x);

	vec3 random = texture(texture1, TexCoord.xy * NoiseScale).rgb;
	//vec3 random = vec3(1,0,0);

    vec2 rayRadiusUV = 0.5 * R * FocalLen / -P.z;
    float rayRadiusPix = rayRadiusUV.x * AORes.x;

    float ao = 1.0;

    if(rayRadiusPix > 1.0)
    {
    	ao = 0.0;

    	float numSteps;
    	vec2 stepSizeUV;
    	ComputeSteps(stepSizeUV, numSteps, rayRadiusPix, random.z);

    	//float stepSizePix = rayRadiusPix / (numSamples + 1);
		//vec2 stepSizeUV = stepSizePix * InvAORes;
		float alpha = 2.0 * PI / numDirections;

		for(float d = 0; d < numDirections; ++d)
		{
			float theta = alpha * d;
			vec2 dir = RotateDirections(vec2(cos(theta), sin(theta)), random.xy);
			vec2 deltaUV = dir * stepSizeUV;
			ao += HorizonOcclusion(	deltaUV,
									P,
									dPdu,
									dPdv,
									random.z,
									numSteps);
		}

		ao = 1.0 - ao / numDirections * strength;
		//ao = 0.0;
	}

	//float frontZ = P.z;
	//float backZ = texture(texture1, TexCoord).r;

	out_frag0 = ao;
}