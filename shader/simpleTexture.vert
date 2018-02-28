#version 450

// Simple shader for drawing a texture to the screen

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texCoords;

out vec2 TexCoords;

layout (location = 0) uniform mat4 model;

void main() {
    gl_Position = vec4(position, 1.0);
    TexCoords = texCoords.xy;
}
