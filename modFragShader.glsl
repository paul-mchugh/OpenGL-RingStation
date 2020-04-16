#version 430

#define MAX_LIGHTS  10
#define NO_LIGHT    0
#define AMBIENT     1
#define POSITIONAL  2
#define DIRECTIONAL 3
#define SPOTLIGHT   4
#define KD 10
#define KC 1
#define KL 0
#define KQ 0.000125

struct Light
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 position;
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

in vec2 varyingTc;
in vec3 varyingNorm;
in vec3 varyingLightDir[MAX_LIGHTS];
in vec3 varyingVPos;

out vec4 color;

void main(void)
{
	//compute norm and pos
	vec3 N = normalize( varyingNorm);
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
				if(l.type==POSITIONAL)
				{
					float dist = length(l.position-varyingVPos);
					float adjDist = max(dist-KD,0);
					diffFactor = 1/(KC+adjDist*KL+adjDist*adjDist*KQ);
					specFactor = 1/(KC+dist*KL+dist*dist*KQ);
				}
				else if(l.type==SPOTLIGHT)
				{
					vec3 D = (invv_matrix*vec4(l.direction,1)).xyz;
					float offAxisCosPhi = dot(D,-L);
					float cfMin = cos(l.cutoff);
					diffFactor=(cfMin<offAxisCosPhi)?pow(offAxisCosPhi,l.exponent):0;
					specFactor=diffFactor;
				}
				//diffuse contribution
				float cosTheta = dot(L,N);
				if(i==atLight)cosTheta=1;
				diffWSum += l.diffuse.xyz * max(cosTheta,0.0)*diffFactor;
				//specular contribution
				vec3 R = normalize(reflect(-L,N));
				float cosPhi = dot(V,R);
				specWSum += l.specular.xyz * pow(max(cosPhi,0.0), material.shininess)*specFactor;
			}
		}
	}
	vec3 lightV =
		ambientSum * material.ambient.xyz+
		diffWSum   * material.diffuse.xyz+
		specWSum   * material.specular.xyz;
	color = vec4(lightV,1) * (texEn ? texture(samp,varyingTc) : vec4(1));
}

