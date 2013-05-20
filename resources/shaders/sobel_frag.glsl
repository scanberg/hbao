#version 150

uniform sampler2D texture0;

in vec2 TexCoord;

out vec2 out_frag0;

const vec2 texelSize = vec2(1.0/1024.0, 1.0/768.0);

void main(void)
{
	const vec2 tc[9] = vec2[9](vec2(-1,1)*texelSize, vec2(0,1)*texelSize, vec2(1,1)*texelSize,
						vec2(-1,0)*texelSize, vec2(0,0),		   vec2(1,0)*texelSize,
						vec2(-1,-1)*texelSize, vec2(0,-1)*texelSize,vec2(1,-1)*texelSize);

	const float Gx[9] = float[9](1.0, 0.0, -1.0,
						 		 2.0, 0.0, -2.0,
						 		 1.0, 0.0, -1.0);

	const float Gy[9] = float[9](1.0,  2.0,  1.0,
						 		 0.0,  0.0,  0.0,
								-1.0, -2.0, -1.0);

	out_frag0 = vec2(0.0);

	const mat3 RGB_to_YUV = mat3(	vec3(0.2126, -0.09991, 0.615),
									vec3(0.7152, -0.33609, -0.55861),
									vec3(0.0722, 0.436, -0.05639));

	vec3 yuvColor;
	for(int i=0; i<9; i++)
	{
		yuvColor = RGB_to_YUV * texture(texture0, TexCoord + tc[i]).rgb;
		out_frag0.r += Gx[i] * yuvColor.r;
		out_frag0.g += Gy[i] * yuvColor.r;
	}
}