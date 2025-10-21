#version 450

layout (location = 0) in vec3 f_position;
layout (location = 1) in vec3 f_normal;
layout (location = 2) in vec2 f_uv;

layout (location = 0) out vec4 final_color;

layout (binding = 1, std140) uniform ModelUniforms {
	mat4 model;
	vec3 albedo_color;
};

layout (binding = 2) uniform sampler2D textures[1];

void main() {
	vec4 texel = texture(textures[0], f_uv);
	final_color = vec4(texel.rgb, 1.0f);
}
