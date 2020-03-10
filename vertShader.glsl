#version 430

layout (location=0) in vec3 position;
layout (location=1) in vec2 tc;
layout (location=2) in vec3 norm;
layout (location=3) in vec3 tan;

uniform float zoom;
uniform bool texEn;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
layout (binding=0) uniform sampler2D samp;

out vec4 colorIntr;
out vec2 varyingTc;

void main(void)
{
	gl_Position = proj_matrix * mv_matrix * vec4(position,1.0);
	varyingTc = tc;
}
