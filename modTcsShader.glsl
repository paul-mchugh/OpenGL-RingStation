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

layout(vertices=3) out;

in vec3 pos_TCS[];
in vec2 tc_TCS[];
in vec3 norm_TCS[];
in vec3 tan_TCS[];

out vec3 pos_TES[];
out vec2 tc_TES[];
out vec3 norm_TES[];
out vec3 tan_TES[];

void main(void)
{
	pos_TES[gl_InvocationID]  = pos_TCS[gl_InvocationID];
	tc_TES[gl_InvocationID]   = tc_TCS[gl_InvocationID];
	norm_TES[gl_InvocationID] = norm_TCS[gl_InvocationID];
	tan_TES[gl_InvocationID]  = tan_TCS[gl_InvocationID];

	if(gl_InvocationID==0)
	{
		float divisions=2;

		mat4 mvp = proj_matrix * mv_matrix;
		vec4 p0 = mvp * vec4(pos_TCS[0],1);//bottom-right
		vec4 p1 = mvp * vec4(pos_TCS[1],1);//top
		vec4 p2 = mvp * vec4(pos_TCS[2],1);//bottom-left
		p0 /= p0.w;
		p1 /= p1.w;
		p2 /= p2.w;
		float e12 = length(p2.xy-p1.xy)*divisions+1.0;//top-left /u==1/indx:0
		float e20 = length(p0.xy-p2.xy)*divisions+1.0;//bottom   /v==1/indx:1
		float e01 = length(p1.xy-p0.xy)*divisions+1.0;//top-right/w==1/indx:2

		e01=e12=e20=8;

		gl_TessLevelOuter[0] = e12;
		gl_TessLevelOuter[1] = e20;
		gl_TessLevelOuter[2] = e01;
		gl_TessLevelOuter[3] = e01;
		gl_TessLevelInner[0] = min(min(e12,e20), e01);
		gl_TessLevelInner[1] = min(min(e12,e20), e01);
	}
}
