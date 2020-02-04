#version 430
uniform float xOff;
uniform float yOff;
void main(void)
{
	if(gl_VertexID==0) gl_Position = vec4(0.25+xOff,-0.25+yOff,0.0,1.0);
	else if(gl_VertexID==1) gl_Position = vec4(-0.25+xOff,-0.25+yOff,0.0,1.0);
	else gl_Position = vec4(xOff,0.25+yOff,0.0,1.0);
}
