#version 430

uniform bool dMapEn;
uniform vec3 user_mv_pos;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
uniform mat4 mvp_matrix;
layout (binding=0) uniform sampler2D samp;
layout (binding=1) uniform sampler2D depthMap;

layout(vertices=6) out;

in vec3 pos_TCS[];
in vec2 tc_TCS[];
in vec3 norm_TCS[];

out vec3 pos_TES[];
out vec2 tc_TES[];
out vec3 norm_TES[];

void main(void)
{
	int cpyFrom[] = {0,1,2,4};
	if(gl_InvocationID<4)
	{
		pos_TES[gl_InvocationID]  = pos_TCS[cpyFrom[gl_InvocationID]];
		tc_TES[gl_InvocationID]   = tc_TCS[cpyFrom[gl_InvocationID]];
		norm_TES[gl_InvocationID] = norm_TCS[cpyFrom[gl_InvocationID]];
	}

	if(gl_InvocationID==0)
	{
		float div=16;
		float lodDist=100;

		mat4 mvp = mv_matrix;
		vec3 p0 = vec3(mvp * vec4(pos_TCS[0]-user_mv_pos,1.0));
		vec3 p1 = vec3(mvp * vec4(pos_TCS[1]-user_mv_pos,1.0));
		vec3 p2 = vec3(mvp * vec4(pos_TCS[2]-user_mv_pos,1.0));
		vec3 p3 = vec3(mvp * vec4(pos_TCS[3]-user_mv_pos,1.0));
		float lheight = min(length(p2.xyz),length(p0.xyz));
		float rheight = min(length(p3.xyz),length(p1.xyz));
		float twidth  = min(length(p3.xyz),length(p2.xyz));
		float bwidth  = min(length(p1.xyz),length(p0.xyz));

		lheight = clamp((lodDist-length(lheight))/10, 1, div);
		rheight = clamp((lodDist-length(rheight))/10, 1, div);
		twidth  = clamp((lodDist-length(twidth)) /10, 1, div);
		bwidth  = clamp((lodDist-length(bwidth)) /10, 1, div);

		gl_TessLevelOuter[0] = lheight;
		gl_TessLevelOuter[1] = bwidth;
		gl_TessLevelOuter[2] = rheight;
		gl_TessLevelOuter[3] = twidth;
		gl_TessLevelInner[0] = max(twidth,bwidth);
		gl_TessLevelInner[1] = max(lheight,rheight);

	}
}
