#version 150

uniform sampler2D texture0;
uniform sampler2D texture1;

out vec4 out_frag0;

in float Depth;

in vec2 TexCoord;

void main(void)
{
	vec4 diffuseAndMask = texture(texture0, TexCoord);

	if( diffuseAndMask.a < 0.5)
		discard;

	diffuseAndMask.rgb = vec3(1,1,1);

	out_frag0 = vec4(diffuseAndMask.rgb, 1.0);
}