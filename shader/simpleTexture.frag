#version 450

in vec2 TexCoords;

uniform sampler2D tex;

layout(location = 0) out vec3 color;

void main() {
    color = vec3(texture(tex, TexCoords));
}
