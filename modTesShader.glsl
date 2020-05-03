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
uniform int atLight;
uniform mat4 mv_matrix;
uniform mat4 invv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
layout (binding=0) uniform sampler2D samp;
uniform mat4 shadMVP[MAX_LIGHTS];
uniform sampler2DShadow flats[MAX_LIGHTS];
uniform samplerCubeShadow cubes[MAX_LIGHTS];

layout(triangles, fractional_even_spacing, ccw) in;

in vec3 pos_TES[];
in vec2 tc_TES[];
in vec3 norm_TES[];
in vec3 tan_TES[];

out vec2 varyingTc;
out vec3 varyingNorm;
out vec3 varyingLightDir[MAX_LIGHTS];
out vec3 varyingHalfVec [MAX_LIGHTS];
out vec4 shadowCoord[MAX_LIGHTS];
out vec3 varyingVPos;

vec2 interpV2(vec2 v0, vec2 v1, vec2 v2)
{
	vec2 acc = vec2(0);
	acc += gl_TessCoord[0] * v0;
	acc += gl_TessCoord[1] * v1;
	acc += gl_TessCoord[2] * v2;
	return acc;
}

vec3 interpV3(vec3 v0, vec3 v1, vec3 v2)
{
	vec3 acc = vec3(0);
	acc += gl_TessCoord[0] * v0;
	acc += gl_TessCoord[1] * v1;
	acc += gl_TessCoord[2] * v2;
	return acc;
}

void main(void)
{
	vec3 position = interpV3( pos_TES[0], pos_TES[1], pos_TES[2]);
	vec2 tc       = interpV2(  tc_TES[0],  tc_TES[1],  tc_TES[2]);
	vec3 norm     = interpV3(norm_TES[0],norm_TES[1],norm_TES[2]);
	vec3 tan      = interpV3( tan_TES[0], tan_TES[1], tan_TES[2]);

	//compute norm and pass it through
	varyingNorm = (norm_matrix * vec4(norm,1.0)).xyz;
	vec4 wsVertPos  = mv_matrix*vec4(position,1.0);

	//test offset position by pushing is direction of the normal depending on amt of blue
	wsVertPos += vec4(normalize(varyingNorm)*(1-texture(samp,tc).b),1);

	//set position and pass it through to the shader
	varyingVPos = wsVertPos.xyz;
	gl_Position = proj_matrix * wsVertPos;

	//pass compute and pass each light direction
	for(int i=0;i<MAX_LIGHTS;++i)
	{
		Light l = lights[i];
		switch(l.type)
		{
		case POSITIONAL:
			varyingLightDir[i] = l.position-varyingVPos;
			shadowCoord[i] = vec4(vec3(shadMVP[i]*vec4(position,1.0))-l.absPosition,1);
			break;
		case SPOTLIGHT:
			varyingLightDir[i] = l.position-varyingVPos;
			shadowCoord[i] = shadMVP[i] * vec4(position,1.0);
		break;
		case DIRECTIONAL:
			varyingLightDir[i] = (-(invv_matrix*vec4(l.direction,1))).xyz;
			shadowCoord[i] = shadMVP[i] * vec4(position,1.0);
			break;
		case AMBIENT:
		case NO_LIGHT:
			break;
		}
		varyingHalfVec[i] = normalize(varyingLightDir[i]) - normalize(varyingVPos);
	}

	//pass the texture coordinates
	varyingTc = tc;
}
