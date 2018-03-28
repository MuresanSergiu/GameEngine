#version 450

layout (location = 13) uniform bool showNormals;

layout (location = 0) uniform mat4 model;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 projection;

layout(triangles) in;
layout(line_strip, max_vertices = 9) out;

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

//        if (showNormals) {
//            FragNormal = GeomNormal[i];
//            FragTexCoords = GeomTexCoords[i];
//            FragPos = GeomPos[i];
//            FragVertexPos = GeomVertexPos[i];
//            FragShadowPos = GeomShadowPos[i];
//            gl_Position = projection * view * normalize(model * vec4(GeomVertexPos[(i + 1) % 3] + GeomNormal[(i + 1) % 3], 1.0f));
//            gl_Position = vec4(gl_in[(i + 1) % 3].gl_Position.xyz + 1, 1);
//            EmitVertex();
//        }

        EndPrimitive();
    }


}
