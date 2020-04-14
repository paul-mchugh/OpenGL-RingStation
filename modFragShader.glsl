#version 430

#define MAX_LIGHTS  10
#define NO_LIGHT    0
#define AMBIENT     1
#define POSITIONAL  2
#define DIRECTIONAL 3
#define SPOTLIGHT   4

struct Light
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 position;
	vec3 direction;
	float cutoff;
	float exponent;
	uint type;
};

struct Material
{
	vec4  ambient;
	vec4  diffuse;
	vec4  specular;
	float shininess;
};

uniform vec4 globalAmbientLight;
uniform Light lights[MAX_LIGHTS];
uniform Material material;
uniform float zoom;
uniform bool texEn;
uniform mat4 mv_matrix;
uniform mat4 proj_mtrix;
layout (binding=0) uniform sampler2D samp;

in vec2 varyingTc;

out vec4 color;

void main(void)
{
	color = texEn ? texture(samp,varyingTc) : vec4(varyingTc.y,1-varyingTc.y,0,1);
}

