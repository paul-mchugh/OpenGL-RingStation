#version 430

uniform bool dMapEn;
uniform vec3 user_mv_pos;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
uniform mat4 mvp_matrix;
layout (binding=0) uniform sampler2D samp;
layout (binding=1) uniform sampler2D depthMap;

in vec2 varyingTc;

out vec4 color;

void main(void)
{
	color = max(vec4(0.25), texture(samp,varyingTc));
}
