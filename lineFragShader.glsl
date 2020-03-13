#version 430

uniform mat4 pMat;
uniform mat4 vMat;
uniform vec3 src;
uniform vec3 dst;
uniform vec3 color;

out vec4 oColor;

void main(void)
{
	oColor = vec4(color,1);
}
