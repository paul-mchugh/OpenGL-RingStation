#version 430

uniform float zoom;
uniform mat4 mv_matrix;
uniform mat4 proj_mtrix;

in vec4 colorIntr;
out vec4 color;
void main(void)
{
	color = colorIntr;
}

