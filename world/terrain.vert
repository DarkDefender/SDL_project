#version 150 core

in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_TexCoord;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

flat out vec3 out_Normal;
out vec3 out_Pos;
out vec2 tex_Co;

void main(void)
{
	out_Normal = in_Normal;
	out_Pos = in_Position;
	tex_Co = in_TexCoord;

	//TODO move out the matrix mult to CPU instead
	gl_Position = projectionMatrix*viewMatrix*modelMatrix*vec4(in_Position, 1.0);
}
