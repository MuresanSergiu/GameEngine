#version 450

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

out vec3 FragNormal;
out vec3 FragTexCoords;
out vec3 FragPos;
out vec3 FragVertexPos;
out vec3 FragShadowPos;

in vec3 GeomNormal[3];
in vec3 GeomTexCoords[3];
in vec3 GeomPos[3];
in vec3 GeomVertexPos[3];
in vec3 GeomShadowPos[3];


void main() {
    int i;

    for (i = 0; i < 3; i++) {
    }

    for (i = 0; i < 3; i++) {
        FragNormal = GeomNormal[i];
        FragTexCoords = GeomTexCoords[i];
        FragPos = GeomPos[i];
        FragVertexPos = GeomVertexPos[i];
        FragShadowPos = GeomShadowPos[i];
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();

        FragNormal = GeomNormal[i];
        FragTexCoords = GeomTexCoords[i];
        FragPos = GeomPos[i];
        FragVertexPos = GeomVertexPos[i];
        FragShadowPos = GeomShadowPos[i];
        gl_Position = gl_in[(i + 1) % 3].gl_Position;
        EmitVertex();
        EndPrimitive();
    }
}
