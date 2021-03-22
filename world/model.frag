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
  mat3 normalMatrix1 = mat3(viewMatrix * modelMatrix);
	vec3 transformedNormal = normalMatrix1 * out_Normal;

  float temp = abs( (viewMatrix * modelMatrix * vec4(out_Pos,1)).z );

	const vec3 light = vec3(0.58, 0.58, 0.58);

	float res_light = 0.5 + dot(normalize(light), normalize(transformedNormal));

	vec4 tex = texture(texture_diffuse, tex_Co);
	out_Color = vec4(tex.xyz * res_light * ( 1.0 - temp/100.0 ),  tex.w);
}
