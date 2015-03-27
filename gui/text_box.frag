#version 150 core

out vec4 out_Color;
in vec2 texCo;
uniform sampler2D texUnit;

void main(void)
{
    out_Color = texture(texUnit, texCo);
}
