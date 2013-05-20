#version 150

in vec3 Normal;
in vec3 LightDir;

out vec3 out_frag0;

void main(void)
{
	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(LightDir);

	float intensity = dot(normal, vec3(0,0,1));

	const float qf = 4.0;
	const float EPSILON = 1.0e-5;

	intensity = float(int(intensity * qf - EPSILON + 0.5)) / qf;

	vec3 color = vec3(1.0, 0.0, 0.0) * intensity;

	out_frag0 = color;
}