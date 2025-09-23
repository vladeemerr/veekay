#version 450

layout (location = 0) in vec3 f_color;

layout (location = 0) out vec4 final_color;

layout (push_constant, std430) uniform ShaderConstants {
	float param;
};

void main() {
	vec3 color = vec3(f_color.r, f_color.g, param);
	final_color = vec4(color, 1.0f);
}
