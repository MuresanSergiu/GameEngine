#version 450

layout(location = 0) in vec2 position;

void main() {
	gl_Position = vec4(vec3(position, 0.0), 1.0);
}
