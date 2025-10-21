#version 450

layout (location = 0) in vec3 v_position;

layout (binding = 0, std140) uniform SceneUniforms {
	mat4 view_projection;
};

layout (binding = 1, std140) uniform ModelUniforms {
	mat4 model;
	vec3 albedo_color;
};

void main() {
	vec4 point = vec4(v_position, 1.0f);
	vec4 transformed = model * point;
	vec4 projected = view_projection * transformed;

	gl_Position = projected;
}
