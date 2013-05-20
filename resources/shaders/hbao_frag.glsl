#version 150

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

uniform vec2 UVToViewA;
uniform vec2 UVToViewB;

uniform vec2 AORes = vec2(1024.0, 768.0);
uniform vec2 InvAORes = vec2(1.0/1024.0, 1.0/768.0);

uniform float Radius = 4.0;
uniform float NegInvR2 = - 1.0 / (4*4);

in vec2 TexCoord;

out vec3 out_frag0;

float LinearizeDepth(float d, vec2 nf)
{
  float z_n = 2.0 * d - 1.0;
  return 2.0 * nf.x * nf.y /
  	(nf.y + nf.x - z_n * (nf.y - nf.x));
}

vec3 UVToViewSpace(vec2 uv, float z)
{
	uv = UVToViewA * uv + UVToViewB;
	return vec3(uv * z, z);
}

vec3 GetViewPos(vec2 uv)
{
	//return texture(texture2, uv).rgb;
	float z = LinearizeDepth(texture(texture0, uv).r, vec2(0.1, 10.0));
	return UVToViewSpace(uv, -z);
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
	return -V.z * InvLength(V.xy);
}

float Tangent(vec3 P, vec3 S)
{
    return (P.z - S.z) * InvLength(S.xy - P.xy);
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
						vec2 texelDeltaUV,
						vec3 P,
						vec3 dPdu,
						vec3 dPdv,
						float numSamples)
{
	float ao = 0;

	vec2 uv = TexCoord;
	deltaUV = SnapUVOffset( deltaUV );

	vec3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;

	float tanH = Tangent(T);
	float sinH = TanToSin(tanH);

	for(float s = 1; s <= numSamples; ++s)
	{
		uv += deltaUV;
		vec3 S = GetViewPos(uv);
		float tanS = Tangent(P, S);
		float d2 = Length2(S - P);

		if(d2 < Radius*Radius && tanS > tanH)
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

void main(void)
{
	const float PI = 3.14159265;
	const float numDirections = 12;
	const float numSamples = 6;
	const float strength = 1.0;

	vec2 noiseScale = AORes / 4.0;

	vec3 random = texture(texture3, TexCoord * noiseScale).rgb;
	random.xy = 2.0 * random.xy - vec2(1.0);
	vec3 P = GetViewPos(TexCoord);

	vec3 Pr, Pl, Pt, Pb;
    Pr = GetViewPos(TexCoord + vec2(InvAORes.x, 0));
    Pl = GetViewPos(TexCoord + vec2(-InvAORes.x, 0));
    Pt = GetViewPos(TexCoord + vec2(0, InvAORes.y));
    Pb = GetViewPos(TexCoord + vec2(0, -InvAORes.y));

    vec3 dPdu = MinDiff(P, Pr, Pl);
    vec3 dPdv = MinDiff(P, Pt, Pb) * (AORes.y * InvAORes.x);

	vec2 stepSize = Radius * InvAORes;
	float alpha = 2.0 * PI / numDirections;
	float ao = 0;
	float d;

	for(d = 0; d < numDirections; ++d)
	{
		float theta = alpha * d;
		vec2 dir = RotateDirections(vec2(cos(theta), sin(theta)), random.xy);
		vec2 deltaUV = dir * stepSize;
		vec2 texelDeltaUV = dir * InvAORes;
		ao += HorizonOcclusion(	deltaUV,
								texelDeltaUV,
								P,
								dPdu,
								dPdv,
								numSamples);
	}

	ao = 1.0 - ao / numDirections * strength;
	out_frag0 = 1.0 - ao;
	//out_frag0 = vec3(-P.z);
}