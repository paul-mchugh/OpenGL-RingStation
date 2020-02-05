#version 430
in vec4 colorIntr;
out vec4 color;
uniform bool intr;
void main(void)
{
	color = (intr ? colorIntr : vec4(0.0,0.0,1.0,1.0));
}
