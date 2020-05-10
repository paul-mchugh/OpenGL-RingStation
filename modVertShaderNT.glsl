#version 430

layout (location=0) in vec3 position;
layout (location=1) in vec2 tc;
layout (location=2) in vec3 norm;
layout (location=3) in vec3 tan;

#define MAX_LIGHTS  7
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

out vec3 varyingLightDirG[MAX_LIGHTS];
out vec3 varyingHalfVecG [MAX_LIGHTS];
out vec4 shadowCoordG[MAX_LIGHTS];
out vec3 varyingMPosG;
out vec3 varyingVPosG;
out vec2 varyingTcG;
out vec3 varyingNormG;
out vec3 varyingTanG;

void main(void)
{
	//set position and pass it through to the shader
	vec4 wsVertPos  = mv_matrix*vec4(position,1.0);
	varyingMPosG = position;
	varyingVPosG = wsVertPos.xyz;
	gl_Position  = proj_matrix * wsVertPos;

	//compute norm and pass it through
	varyingNormG = (norm_matrix * vec4(norm,1.0)).xyz;
	varyingTanG  = (norm_matrix * vec4(tan, 1.0)).xyz;

	//pass compute and pass each light direction
	for(int i=0;i<MAX_LIGHTS;++i)
	{
		Light l = lights[i];
		switch(l.type)
		{
		case POSITIONAL:
			varyingLightDirG[i] = l.position-varyingVPosG;
			shadowCoordG[i] = vec4(vec3(shadMVP[i]*vec4(position,1.0))-l.absPosition,1);
			break;
		case SPOTLIGHT:
			varyingLightDirG[i] = l.position-varyingVPosG;
			shadowCoordG[i] = shadMVP[i] * vec4(position,1.0);
		break;
		case DIRECTIONAL:
			varyingLightDirG[i] = (-(invv_matrix*vec4(l.direction,1))).xyz;
			shadowCoordG[i] = shadMVP[i] * vec4(position,1.0);
			break;
		case AMBIENT:
		case NO_LIGHT:
			break;
		}
		varyingHalfVecG[i] = normalize(varyingLightDirG[i]) - normalize(varyingVPosG);
	}

	//pass the texture coordinates
	varyingTcG = tc;
}
