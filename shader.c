//
// Created by Afterwind on 9/17/2017.
//

#include <GL/glew.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "shader.h"
#include "utils.h"

/* INTERNAL FUNCTIONS */

void initShader(SHADER shaderID, const char* file) {
    char compileLog[512];
    GLint success = 0;

    char pathVertex[512];
    char pathFragment[512];
    char pathGeometry[512];

    char sourceVertex[32000];
    char sourceFragment[32000];
    char sourceGeometry[32000];

    char* sourceVertexPointer = sourceVertex;
    char* sourceFragmentPointer = sourceFragment;
    char* sourceGeometryPointer = sourceGeometry;

    GLint lengthVertex;
    GLint lengthFragment;
    GLint lengthGeometry;

    sprintf(pathVertex, "../shader/%s.vert", file);
    sprintf(pathFragment, "../shader/%s.frag", file);
    if (shaderID == GE_PROGRAM_WIREFRAME) {
        sprintf(pathGeometry, "../shader/%s.geom", file);
    }

    // Read the source file
    readFile(pathVertex, sourceVertex);
    lengthVertex = strlen(sourceVertex);

    readFile(pathFragment, sourceFragment);
    lengthFragment = strlen(sourceFragment);

    if (shaderID == GE_PROGRAM_WIREFRAME) {
        readFile(pathGeometry, sourceGeometry);
        lengthGeometry = strlen(sourceGeometry);
    }

    // Initialize shaders
    vertexShaders[shaderID] = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaders[shaderID], 1, (const GLchar *const *) &sourceVertexPointer, &lengthVertex);
    glCompileShader(vertexShaders[shaderID]);

    fragmentShaders[shaderID] = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaders[shaderID], 1, (const GLchar *const *) &sourceFragmentPointer, &lengthFragment);
    glCompileShader(fragmentShaders[shaderID]);

    if (shaderID == GE_PROGRAM_WIREFRAME) {
        geometryShaders[shaderID] = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShaders[shaderID], 1, (const GLchar* const*) &sourceGeometryPointer, &lengthGeometry);
        glCompileShader(geometryShaders[shaderID]);
    }

    programs[shaderID] = glCreateProgram();
    glAttachShader(programs[shaderID], vertexShaders[shaderID]);
    glAttachShader(programs[shaderID], fragmentShaders[shaderID]);
    if (shaderID == GE_PROGRAM_WIREFRAME) {
        glAttachShader(programs[shaderID], geometryShaders[shaderID]);
    }
    glLinkProgram(programs[shaderID]);
    printf("The ID of program %s is %u\n", file, programs[shaderID]);

    glGetProgramiv(programs[shaderID], GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        glGetProgramInfoLog(programs[shaderID], sizeof(compileLog), NULL, compileLog);
        printf("Link errors for %s: \n%s\n", file, compileLog);
    }

    glValidateProgram(programs[shaderID]);
    glGetProgramiv(programs[shaderID], GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE) {
        glGetProgramInfoLog(programs[shaderID], sizeof(compileLog), NULL, compileLog);
        printf("Validation errors for %s: \n%s\n", file, compileLog);
    }
}

/* EXTERNAL FUNCTIONS */

void initAllShaders() {
    initShader(GE_PROGRAM_MAIN, "shader");
    initShader(GE_PROGRAM_TEXTURE, "simpleTexture");
    initShader(GE_PROGRAM_SHADOW, "shadow");
    initShader(GE_PROGRAM_GUI, "gui");
    initShader(GE_PROGRAM_WIREFRAME, "wireframe");
}

GLint geGetUniformLocationWithLog(const char* name) {
    GLuint programID;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &programID);
    GLint uniformId = glGetUniformLocation(programID, name);
    if (uniformId == -1 && strcmp(name, "showNormals") == 0) {
        fprintf(stderr, "Failed to find uniform with name %s in program with ID %u\n", name, programID);
    }
    return uniformId;
}