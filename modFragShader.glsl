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
uniform vec2 shadNF;
uniform mat4 shadMVP[MAX_LIGHTS];
uniform sampler2DShadow flats[MAX_LIGHTS];
uniform samplerCubeShadow cubes[MAX_LIGHTS];

in vec3 varyingLightDir[MAX_LIGHTS];
in vec3 varyingHalfVec [MAX_LIGHTS];
in vec4 shadowCoord[MAX_LIGHTS];
in vec3 varyingVPos;
in vec2 varyingTc;
in vec3 varyingNorm;
in vec3 varyingTan;
out vec4 color;

float lookup(int i, float ox, float oy)
{
	return
		textureProj(flats[i], shadowCoord[i] +
		            vec4(ox*0.001*shadowCoord[i].w,oy*0.001*shadowCoord[i].w,-0.01,0.0));
}

float computeShadowForLight(int i)
{
	float f=0;
	vec2 offset = mod(floor(gl_FragCoord.xy),2.0)*SWIDTH;
	f+=lookup(i, -1.5*SWIDTH+offset.x,  1.5*SWIDTH-offset.y);
	f+=lookup(i, -1.5*SWIDTH+offset.x, -0.5*SWIDTH-offset.y);
	f+=lookup(i,  0.5*SWIDTH+offset.x,  1.5*SWIDTH-offset.y);
	f+=lookup(i,  0.5*SWIDTH+offset.x, -0.5*SWIDTH-offset.y);
	return f/4.0;
}

float computeCubeShadow(int i)
{
	//formula for depth from:
	//https://stackoverflow.com/questions/48654578/omnidirectional-lighting-in-opengl-glsl-4-1
/*
	float n=0.1,f=1000;
	float ndiff = f-n;
	float near  = (f+n)/ndiff*0.5+0.5;	//stored in shadNF[0]
	float far   =-(f*n)/ndiff;			//stored in shadNF[1]
*/
	vec3 L = shadowCoord[i].xyz;
	vec3 absL = abs(L);
	float z = max(absL.x,max(absL.y,absL.z));
	float d = shadNF[0] + shadNF[1] /z;

	return texture(cubes[i], vec4(L,d));
}

vec3 getNormal()
{
	if(nMapEn)
	{
		vec3  norm = normalize(varyingNorm);
		vec3   tan = normalize(varyingTan);
		tan = normalize(tan - dot(tan, norm) * norm);
		vec3 bitan = cross(tan, norm);
		mat3 tbn = mat3(tan, bitan, norm);
		vec3 newNorm = texture(normalMap, varyingTc).xyz;
		newNorm = newNorm * 2 - 1;//to vector from color
		newNorm = tbn * newNorm;//to eye space
		return newNorm;
	}
	else
	{
		return normalize(varyingNorm);
	}
}

void main(void)
{
	//compute norm and pos
	vec3 N = getNormal();
	vec3 V = normalize(-varyingVPos);
	//compute ambient
	vec3 ambientSum = vec3(0);
	vec3 diffWSum   = vec3(0);
	vec3 specWSum   = vec3(0);
	for(int i=0;i<MAX_LIGHTS;++i)
	{
		Light l = lights[i];
		if(l.enabled&&l.type!=NO_LIGHT)
		{
			//ambient contribution
			ambientSum+=l.ambient.xyz;
			if(l.type!=AMBIENT)
			{
				vec3 L = normalize(varyingLightDir[i]);
				float diffFactor = 1, specFactor = 1;
				float shadFactor = computeShadowForLight(i);
				if(l.type==POSITIONAL)
				{
					float dist = length(l.position-varyingVPos);
					float adjDist = max(dist-KD,0);
					diffFactor = 1/(KC+adjDist*(KL+adjDist*KQ));
					specFactor = 1/(KC+   dist*(KL+   dist*KQ));
					shadFactor = computeCubeShadow(i);
				}
				else if(l.type==SPOTLIGHT)
				{
					vec3 D = (invv_matrix*vec4(l.direction,1)).xyz;
					float offAxisCosPhi = dot(D,-L);
					float cfMin = cos(l.cutoff);
					diffFactor=(cfMin<offAxisCosPhi)?pow(offAxisCosPhi,l.exponent):0;
					specFactor=diffFactor;
				}
				else if(l.type==DIRECTIONAL);
				//apply shadows
				diffFactor*=shadFactor;
				specFactor*=shadFactor;
				//diffuse contribution
				float cosTheta = dot(L,N);
				if(i==atLight)cosTheta=1;
				diffWSum += l.diffuse.xyz * max(cosTheta,0.0)*diffFactor;
				//specular contribution
				vec3 H = normalize(varyingHalfVec[i]);
				float cosPhi = dot(H,N);
				specWSum += l.specular.xyz * pow(max(cosPhi,0.0), material.shininess*3)*specFactor;
			}
		}
	}
	vec3 lightV =
		ambientSum * material.ambient.xyz+
		diffWSum   * material.diffuse.xyz+
		specWSum   * material.specular.xyz;
	color = vec4(lightV,1) * (texEn ? texture(samp,varyingTc) : vec4(1));
//	color = vec4(length(lightV)>0.05?1.0f:0.0f,0,1,1);
}
