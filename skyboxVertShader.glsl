#version 430

layout (location=0) in vec3 position;

uniform mat4 vMat;
uniform mat4 pMat;
layout (binding=0) uniform samplerCube samp;

out vec3 varyingTc;

void main(void)
{
	mat4 noRot = mat4(mat3(vMat));
	//pass the texture coordinates
	varyingTc = position;
	//set position and pass it through to the frag shader
	gl_Position = pMat * noRot * vec4(position,1);
}
