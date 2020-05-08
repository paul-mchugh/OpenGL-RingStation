#version 430

#define MAX_LIGHTS 10
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
	vec3 absPosition;
	vec3 direction;
	float cutoff;
	float exponent;
	bool enabled;
	uint type;
};

struct Material
{
	vec4  ambient;
	vec4  diffuse;
	vec4  specular;
	float shininess;
};

uniform Light lights[MAX_LIGHTS];
uniform Material material;
uniform bool texEn;
uniform bool dMapEn;
uniform bool nMapEn;
uniform int atLight;
uniform mat4 mv_matrix;
uniform mat4 invv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
layout (binding=0) uniform sampler2D samp;
layout (binding=1) uniform sampler2D depthMap;
layout (binding=2) uniform sampler2D normalMap;
layout (binding=3) uniform sampler3D noiseMap;
uniform float fadeLvl;
uniform vec2 shadNF;
uniform mat4 shadMVP[MAX_LIGHTS];
uniform sampler2DShadow flats[MAX_LIGHTS];
uniform samplerCubeShadow cubes[MAX_LIGHTS];

layout (location=0) in vec3 position;
layout (location=1) in vec2 tc;
layout (location=2) in vec3 norm;
layout (location=3) in vec3 tan;

out vec3 pos_TCS;
out vec2 tc_TCS;
out vec3 norm_TCS;
out vec3 tan_TCS;

void main(void)
{
	//pass through the important vertices
	pos_TCS  = position;
	tc_TCS   = tc;
	norm_TCS = norm;
	tan_TCS  = tan;
}
