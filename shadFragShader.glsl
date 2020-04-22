#version 430

uniform mat4 mvp_matrix;
layout (binding=0) uniform sampler2D samp;

in vec2 varyingTc;

out vec4 color;

void main(void)
{
	color = max(vec4(0.25), texture(samp,varyingTc));
//	color = vec4(gl_FragCoord.z,0,0,1);
//	if(gl_FragCoord.z<0.25)color+=vec4(0,1,0,0);
}
