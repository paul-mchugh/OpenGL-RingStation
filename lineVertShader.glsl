#version 430

uniform mat4 pMat;
uniform mat4 vMat;
uniform vec3 src;
uniform vec3 dst;
uniform vec3 color;

void main(void)
{
	if(gl_VertexID==0)
		gl_Position = pMat * vMat * vec4(src,1);
	else
		gl_Position = pMat * vMat * vec4(dst,1);
}
