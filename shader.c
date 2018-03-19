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

    char vertexPath[512];
    char fragmentPath[512];

    sprintf(vertexPath, "../shader/%s.vert", file);
    sprintf(fragmentPath, "../shader/%s.frag", file);

    // Read the source file
    char** vertSrc = readFile(vertexPath);
    GLint* vertLength = calloc(1, sizeof(size_t));
    *vertLength = (GLint) strlen(vertSrc[0]) + 1;

    char** fragSrc = readFile(fragmentPath);
    GLint* fragLength = calloc(1, sizeof(size_t));
    *fragLength = (GLint) strlen(fragSrc[0]) + 1;

    // Initialize shaders
    vertexShaders[shaderID] = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaders[shaderID], 1, (const GLchar *const *) vertSrc, vertLength);
    glCompileShader(vertexShaders[shaderID]);

    fragmentShaders[shaderID] = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaders[shaderID], 1, (const GLchar *const *) fragSrc, fragLength);
    glCompileShader(fragmentShaders[shaderID]);

    GLuint geometryShaderID = 0;
    if (shaderID == GE_PROGRAM_MAIN) {
        char geometryPath[512];

        sprintf(geometryPath, "../shader/%s.geom", file);

        char** geomSrc = readFile(geometryPath);
        GLint* geomLength = calloc(1, sizeof(size_t));
        *geomLength = (GLint) strlen(geomSrc[0]) + 1;

        geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShaderID, 1, (const GLchar* const*) geomSrc, geomLength);
        glCompileShader(geometryShaderID);
    }

    programs[shaderID] = glCreateProgram();
    glAttachShader(programs[shaderID], vertexShaders[shaderID]);
    glAttachShader(programs[shaderID], fragmentShaders[shaderID]);
    if (geometryShaderID != 0) {
        glAttachShader(programs[shaderID], geometryShaderID);
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
}

GLint geGetUniformLocationWithLog(const char* name) {
    GLuint programID;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &programID);
    GLint uniformId = glGetUniformLocation(programID, name);
    if (uniformId == -1) {
//        fprintf(stderr, "Failed to find uniform with name %s in program with ID %u\n", name, programID);
    }
    return uniformId;
}
