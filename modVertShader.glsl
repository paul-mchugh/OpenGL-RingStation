#version 430

layout (location=0) in vec3 position;
layout (location=1) in vec2 tc;
layout (location=2) in vec3 norm;
layout (location=3) in vec3 tan;

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

out vec2 varyingTc;
out vec3 varyingNorm;
out vec3 varyingLightDir[MAX_LIGHTS];
out vec3 varyingHalfVec [MAX_LIGHTS];
out vec3 varyingVPos;

void main(void)
{
	//set position and pass it through to the shader
	vec4 wsVertPos  = mv_matrix*vec4(position,1.0);
	varyingVPos = wsVertPos.xyz;
	gl_Position = proj_matrix * wsVertPos;

	//compute norm and pass it through
	varyingNorm = (norm_matrix * vec4(norm,1.0)).xyz;

	//pass compute and pass each light direction
	for(int i=0;i<MAX_LIGHTS;++i)
	{
		Light l = lights[i];
		switch(l.type)
		{
		case POSITIONAL:
		case SPOTLIGHT:
			varyingLightDir[i] = l.position-varyingVPos;
			break;
		case DIRECTIONAL:
			varyingLightDir[i] = (-(invv_matrix*vec4(l.direction,1))).xyz;
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
