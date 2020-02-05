#version 430
uniform float xOff;
uniform float yOff;
uniform float zoom;
out vec4 colorIntr;
void main(void)
{
	float dist = 0.25*(0.75*zoom+0.25);
	switch(gl_VertexID)
	{
	case 0:
		gl_Position = vec4(dist+xOff,-dist+yOff,0.0,1.0);
		colorIntr   = vec4(1.0, 0.0, 0.0, 1.0);
		break;
	case 1:
		gl_Position = vec4(-dist+xOff,-dist+yOff,0.0,1.0);
		colorIntr   = vec4(0.0, 1.0, 0.0, 1.0);
		break;
	default:
	case 2:
		gl_Position = vec4(xOff,dist+yOff,0.0,1.0);
		colorIntr   = vec4(0.0, 0.0, 1.0, 1.0);
		break;
	}
}
