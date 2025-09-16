#version 450

layout (location = 0) out vec2 f_uv;

void main() {
	f_uv = vec2(gl_VertexIndex & 1, gl_VertexIndex >> 1);
	gl_Position = vec4(2.0f * f_uv - 1.0f, 0.0f, 1.0f);
}
