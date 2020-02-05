#version 430
uniform float xOff;
uniform float yOff;
out vec4 colorIntr;
void main(void)
{
	switch(gl_VertexID)
	{
	case 0:
		gl_Position = vec4(0.25+xOff,-0.25+yOff,0.0,1.0);
		colorIntr   = vec4(1.0, 0.0, 0.0, 1.0);
		break;
	case 1:
		gl_Position = vec4(-0.25+xOff,-0.25+yOff,0.0,1.0);
		colorIntr   = vec4(0.0, 1.0, 0.0, 1.0);
		break;
	default:
	case 2:
		gl_Position = vec4(xOff,0.25+yOff,0.0,1.0);
		colorIntr   = vec4(0.0, 0.0, 1.0, 1.0);
		break;
	}
}
