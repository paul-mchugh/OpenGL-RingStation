#version 430

uniform mat4 mvp_matrix;
layout (binding=0) uniform sampler2D samp;

in vec2 varyingTc;

out vec4 color;

void main(void)
{
	color = max(vec4(0.25), texture(samp,varyingTc));
}
