#version 150

uniform sampler2D texture0;
uniform sampler2D texture1;

in vec2 TexCoord;

out vec4 out_Color;

void main(void)
{
	vec4 textureColor = texture(texture0, TexCoord);
	vec2 sobel = texture(texture1, TexCoord).rg;

	float G = length(sobel) * 1.0;
	vec3 countour = vec3(1.0) * G;

	vec3 finalColor = mix(vec3(1.0), textureColor.rgb - countour, 1.0);

	out_Color = vec4(finalColor, 1.0);
}