#version 430

uniform bool dMapEn;
uniform vec3 user_mv_pos;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
uniform mat4 mvp_matrix;
layout (binding=0) uniform sampler2D samp;
layout (binding=1) uniform sampler2D depthMap;

layout (location=0) in vec3 position;
layout (location=1) in vec2 tc;
layout (location=2) in vec2 norm;

out vec2 varyingTc;

void main(void)
{
	//set position and pass it through to the frag shader
	gl_Position = mvp_matrix * vec4(position,1);
	//pass the texture coordinates
	varyingTc = tc;
//	varyingTc = vec2(1,(mvp_matrix* vec4(position,1)).y);
}
