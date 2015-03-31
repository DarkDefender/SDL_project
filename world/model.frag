#version 150 core

out vec4 out_Color;
in vec3 out_Normal;
in vec3 out_Pos;
in vec2 tex_Co;
uniform sampler2D texture_diffuse1;

void main(void)
{
	//out_Color = texture(texture_diffuse1, tex_Co);
	out_Color = vec4(1);
}
