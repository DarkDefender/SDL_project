#version 150 core

out vec4 out_Color;
in vec3 out_Normal;
in vec3 out_Pos;
in vec2 tex_Co;

uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform sampler2D texture_diffuse;

void main(void)
{
	vec4 tex = texture(texture_diffuse, tex_Co);
	if(tex.w < 0.5){
		discard;
	}
	out_Color = tex;
}
