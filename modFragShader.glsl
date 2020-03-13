#version 430

uniform float zoom;
uniform bool texEn;
uniform mat4 mv_matrix;
uniform mat4 proj_mtrix;
layout (binding=0) uniform sampler2D samp;

in vec2 varyingTc;

out vec4 color;

void main(void)
{
	color = texEn ? texture(samp,varyingTc) : vec4(varyingTc.y,1-varyingTc.y,0,1);
}

