#version 150 core

out vec4 out_Color;
flat in vec3 out_Normal;
in vec3 out_Pos;
flat in vec2 tex_Co;

uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform sampler2D tile_ids;
uniform sampler2DArray tex_array;

void main(void)
{
    mat3 normalMatrix1 = mat3(viewMatrix * modelMatrix);
	vec3 transformedNormal = normalMatrix1 * out_Normal;

	//tile_ids w,h
    const float w = 256.0;
    const float h = 256.0;

    float temp = abs( (viewMatrix * modelMatrix * vec4(out_Pos,1)).z );

	vec3 light = vec3(0.58, 0.58, 0.58);

	float res_light = 0.4 + dot(normalize(light), normalize(transformedNormal));

	vec3 tile_offset = vec3( texture(tile_ids, tex_Co) );

	vec3 new_tex_Co = vec3( fract( vec2( out_Pos.xz ) ), tile_offset[0]*255);

	vec3 tile = vec3( texture(tex_array, new_tex_Co) );
	out_Color = vec4(tile * res_light * ( 1.0 - temp/100.0 ), 1);
	//out_Color = vec4(tile_offset * res_light, 1);
}
