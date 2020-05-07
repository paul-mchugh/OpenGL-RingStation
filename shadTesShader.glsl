#version 430

uniform bool dMapEn;
uniform vec3 user_mv_pos;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
uniform mat4 mvp_matrix;
layout (binding=0) uniform sampler2D samp;
layout (binding=1) uniform sampler2D depthMap;

layout(quads, equal_spacing, ccw) in;

in vec3 pos_TES[];
in vec2 tc_TES[];
in vec3 norm_TES[];

out vec2 varyingTc;

vec2 interpV2(vec2 v0, vec2 v1, vec2 v2, vec2 v3)
{
	vec2 bot = mix(v0, v1, gl_TessCoord.x);
	vec2 top = mix(v2, v3, gl_TessCoord.x);
	return mix(bot, top, gl_TessCoord.y);
}

vec3 interpV3(vec3 v0, vec3 v1, vec3 v2, vec3 v3)
{
	vec3 bot = mix(v0, v1, gl_TessCoord.x);
	vec3 top = mix(v2, v3, gl_TessCoord.x);
	return mix(bot, top, gl_TessCoord.y);
}


void main(void)
{
	vec3 position = interpV3( pos_TES[0], pos_TES[1], pos_TES[2], pos_TES[3]);
	vec2 tc       = interpV2(  tc_TES[0],  tc_TES[1],  tc_TES[2],  tc_TES[3]);
	vec3 norm     = interpV3(norm_TES[0],norm_TES[1],norm_TES[2],norm_TES[3]);

	//compute norm and pass it through
	vec3 varyingNorm = (norm_matrix * vec4(norm,1.0)).xyz;
	vec4 wsVertPos = mv_matrix*vec4(position,1.0);

	//test offset position by pushing is direction of the normal depending on amt of blue
	if(dMapEn)
	{
		wsVertPos += vec4(normalize(varyingNorm)*texture(depthMap,tc).r,0);
	}

	//set position and pass it through to the shader
	gl_Position = proj_matrix * wsVertPos;

	//pass the texture coordinates
	varyingTc = tc;
}
