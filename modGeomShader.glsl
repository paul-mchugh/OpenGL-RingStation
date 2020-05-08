#version 430

#define MAX_LIGHTS  10
#define SHAD_TEX    10
#define NO_LIGHT    0
#define AMBIENT     1
#define POSITIONAL  2
#define DIRECTIONAL 3
#define SPOTLIGHT   4
#define KD 10
#define KC 1
#define KL 0.01
#define KQ 0.005
#define SWIDTH 0.25

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

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vec3 varyingLightDirG[][MAX_LIGHTS];
in vec3 varyingHalfVecG [][MAX_LIGHTS];
in vec4 shadowCoordG[][MAX_LIGHTS];
in vec3 varyingMPosG[];
in vec3 varyingVPosG[];
in vec2 varyingTcG[];
in vec3 varyingNormG[];
in vec3 varyingTanG[];

out vec3 varyingLightDir[MAX_LIGHTS];
out vec3 varyingHalfVec [MAX_LIGHTS];
out vec4 shadowCoord[MAX_LIGHTS];
out vec3 varyingMPos;
out vec3 varyingVPos;
out vec2 varyingTc;
out vec3 varyingNorm;
out vec3 varyingTan;

void main(void)
{

	vec3 avgPos=(varyingMPosG[0]+varyingMPosG[1]+varyingMPosG[2])/3.0;
	if(texture(noiseMap, avgPos).r>fadeLvl)
		return;

	for(int i=0;i<3;i++)
	{
		gl_Position = gl_in[i].gl_Position;
		for(int j=0;j<MAX_LIGHTS;++j)
		{
			varyingLightDir[j]=varyingLightDirG[i][j];
			varyingHalfVec[j]=varyingHalfVecG[i][j];
			shadowCoord[j]=shadowCoordG[i][j];
		}
		varyingMPos=varyingMPosG[i];
		varyingVPos=varyingVPosG[i];
		varyingTc=varyingTcG[i];
		varyingNorm=varyingNormG[i];
		varyingTan=varyingTanG[i];
		EmitVertex();
	}
	EndPrimitive();
}
