#version 430

layout (location=0) in vec3 position;

uniform float zoom;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

out vec4 colorIntr;

void main(void)
{
	gl_Position = proj_matrix * mv_matrix * vec4(position,1.0);
	int idM3 = (gl_VertexID/3)%3;

	switch(idM3)
	{
	case 0:
		colorIntr = vec4(1.0, 0.0, 0.0, 1.0);
		break;	
	case 1:
		colorIntr = vec4(0.0, 1.0, 0.0, 1.0);	
		break;	
	case 2:
		colorIntr = vec4(0.0, 0.0, 1.0, 1.0);	
		break;	
	}
}
