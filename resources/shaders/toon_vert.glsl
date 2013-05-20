#version 150

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
 
in vec3 in_position;
in vec3 in_normal;

out vec3 Normal;
out vec3 LightDir;

const vec3 lightPos = vec3(0.0,0.0,2.0);
 
void main(void)
{
	vec4 WorldPos = modelMatrix * vec4(in_position, 1.0);
	gl_Position = projMatrix * viewMatrix * WorldPos;
	Normal = (viewMatrix * modelMatrix * vec4(in_normal, 0.0)).xyz;
	LightDir = normalize( (viewMatrix * vec4(lightPos - WorldPos.xyz, 1.0)).xyz );
}