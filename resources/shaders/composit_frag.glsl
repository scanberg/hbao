#version 150

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

in vec2 TexCoord;

out vec4 out_frag0;

void main(void)
{
	vec4 color = 1.0 * texture(texture0, TexCoord);
	float ao = texture(texture1, TexCoord).r;
	ao = pow(ao, 1.3);
	//vec3 normal = vec3(texture(texture2, TexCoord).rg, 0.0);
	//vec3 viewSpacePos = texture(texture3, TexCoord).rgb;
	//normal.z = 1.0 - sqrt(dot(normal.xy, normal.xy));

	//out_frag0 = vec4(color.rgb * ao, color.a);
	//out_frag0 = vec4(color.rgb, 1.0);
	//out_frag0 = vec4(abs(ao - viewSpacePos)*10, 1.0);
	out_frag0 = vec4( vec3(ao), 1.0);
	//out_frag0 = vec4(viewSpacePos, 1.0);
}