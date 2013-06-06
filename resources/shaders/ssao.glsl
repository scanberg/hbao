uniform sampler2D texture0;

uniform float RenderedTextureWidth = 1024;
uniform float RenderedTextureHeight = 768;

in vec2 TexCoord;

out float out_frag0;

#define PI 3.14159265

float width = RenderedTextureWidth; //texture width
float height = RenderedTextureHeight; //texture height

float near = 0.1; //Z-near
float far = 10.0; //Z-far

int samples = 3; //samples on the each ring (3-7)
int rings = 3; //ring count (2-8)

vec2 texCoord = TexCoord;

vec2 rand(in vec2 coord) //generating random noise
{
	float noiseX = (fract(sin(dot(coord ,vec2(12.9898,78.233))) * 43758.5453));
	float noiseY = (fract(sin(dot(coord ,vec2(12.9898,78.233)*2.0)) * 43758.5453));
	return vec2(noiseX,noiseY)*0.004;
}

float readDepth(in vec2 coord) 
{
	return (2.0 * near) / (far + near - texture2D(DepthTexture, coord ).x * (far-near)); 
}

float compareDepths( in float depth1, in float depth2 )
{
	float aoCap = 1.0;
	float aoMultiplier = 100.0;
	float depthTolerance = 0.0000;
	float aorange = 60.0;// units in space the AO effect extends to (this gets divided by the camera far range
	float diff = sqrt(clamp(1.0-(depth1-depth2) / (aorange/(far-near)),0.0,1.0));
	float ao = min(aoCap,max(0.0,depth1-depth2-depthTolerance) * aoMultiplier) * diff;
	return ao;
}

void main(void)
{	
	float depth = readDepth(texCoord);
	float d;

	float aspect = width/height;
	vec2 noise = rand(texCoord);

	float w = (1.0 / width)/clamp(depth,0.05,1.0)+(noise.x*(1.0-noise.x));
	float h = (1.0 / height)/clamp(depth,0.05,1.0)+(noise.y*(1.0-noise.y));

	float pw;
	float ph;

	float ao;	
	float s;

	for (int i = -rings ; i < rings; i += 1)
	{
		for (int j = -samples ; j < samples; j += 1)
		{
			float step = PI*2.0 / float(samples*i);
			pw = (cos(float(j)*step)*float(i));
			ph = (sin(float(j)*step)*float(i))*aspect;
			d = readDepth( vec2(texCoord.s+pw*w,texCoord.t+ph*h));
			ao += compareDepths(depth,d);	
			s += 1.0;
		}
	}

	ao /= s;
	ao = 1.0-ao;

	out_frag0 = ao;
}