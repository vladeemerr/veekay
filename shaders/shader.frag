#version 450

layout (location = 0) in vec3 f_color;

layout (location = 0) out vec4 final_color;

layout (push_constant) uniform globals {
	float time;
};

void main() {
	vec3 c = vec3(f_color.r, f_color.g, 0.5f + 0.5f * sin(time));
	final_color = vec4(c, 1.0f);
}
