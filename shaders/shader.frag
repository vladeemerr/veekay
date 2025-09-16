#version 450

layout (location = 0) in vec2 f_uv;

layout (location = 0) out vec4 final_color;

layout (push_constant) uniform globals {
	vec2 resolution;
	float time;
};

// https://www.shadertoy.com/view/W3tSR4

#define BRIGHTNESS 0.002
#define STEPS 50.0
#define DENSITY 5.0

float volume(vec3 p) {
	//Spherical distance
	float l = length(p);
	//Projected sine waves
	vec3 v = cos(abs(p) * 15.0 / max(4.0, l) + time);
	//Combine cosine grid with sphere
	return length(vec4(max(v, v.yzx) - 0.9, l - 4.0)) / DENSITY;
}

vec3 rotate(vec3 p, vec3 a) {
	return a * dot(p, a) + cross(p, a);
}

void main() {
	vec2 center = 2.0 * (f_uv * resolution) - resolution;
	
	//Rotation axis
	vec3 axis = normalize(cos(vec3(0.5 * time + vec3(0, 2, 4))));
	//Rotate ray direction
	vec3 dir = rotate(normalize(vec3(center, resolution.y)), axis);
	
	//Camera position
	vec3 cam = rotate(vec3(0, 0, -8.0), axis);
	//Raymarch sample point
	vec3 pos = cam;
	
	//Output color
	vec3 col = vec3(0.0);
	
	//Glow raymarch loop
	for(float i = 0.0; i < STEPS; i++) {
		//Glow density
		float vol = volume(pos);
		//Step forward
		pos += dir * vol;
		
		//Add sine wave coloring
		col += (cos(pos.z / (1.0 + vol) + time + vec3(6, 1, 2)) + 1.2) / vol;
	}

	col = tanh(BRIGHTNESS * col);
	
	final_color = vec4(col, 1.0f);
}
