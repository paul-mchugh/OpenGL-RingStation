#version 430

uniform mat4 vMap;
uniform mat4 pMat;
layout (binding=0) uniform samplerCube samp;

in vec3 varyingTc;

out vec4 color;

void main(void)
{
	color = texture(samp,normalize(varyingTc));
}
