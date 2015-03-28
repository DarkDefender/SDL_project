#version 150 core

in vec3 in_Position;
in vec2 in_TexCoord;
uniform mat4 myMatrix;
out vec2 texCo;

void main(void)
{
	gl_Position = myMatrix * vec4( in_Position, 1.0);

	texCo = in_TexCoord;
}
