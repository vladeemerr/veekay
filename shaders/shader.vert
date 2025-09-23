#version 450

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_color;

layout (location = 0) out vec3 f_color;

layout (push_constant, std430) uniform ShaderConstants {
	mat4 projection;
	mat4 model;
};

void main() {
	gl_Position = projection * model * vec4(v_position, 1.0f);
	f_color = v_color;
}
