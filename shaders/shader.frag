#version 450

// NOTE: out attributes of vertex shader must be in's
// layout (location = 0) in type name;

// NOTE: Pixel color
layout (location = 0) out vec4 final_color;

// NOTE: Must match declaration order of a C struct
layout (push_constant, std430) uniform ShaderConstants {
	mat4 projection;
	mat4 transform;
	vec3 color;
};

void main() {
	final_color = vec4(color, 1.0f);
}
