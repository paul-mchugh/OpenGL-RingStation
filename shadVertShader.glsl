#version 430

layout (location=0) in vec3 position;
layout (location=1) in vec2 tc;

uniform mat4 mvp_matrix;
layout (binding=0) uniform sampler2D samp;

out vec2 varyingTc;

void main(void)
{
	//set position and pass it through to the frag shader
	gl_Position = mvp_matrix * vec4(position,1);
	//pass the texture coordinates
	varyingTc = tc;
//	varyingTc = vec2(1,(mvp_matrix* vec4(position,1)).y);
}
