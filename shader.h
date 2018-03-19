//
// Created by Afterwind on 9/17/2017.
//

#include "utils.h"

#ifndef GAMEENGINE_SHADER_H
#define GAMEENGINE_SHADER_H
#define _U(X) geGetUniformLocationWithLog(STRINGIFY(X))

typedef enum {
    GE_PROGRAM_MAIN,
    GE_PROGRAM_TEXTURE,
    GE_PROGRAM_SHADOW,
    GE_PROGRAM_GUI,

    GE_NUM_PROGRAMS
} SHADER;

GLuint vertexShaders[10];
GLuint fragmentShaders[10];
GLuint geometryShaders[10];
GLuint programs[10];

void initAllShaders();
GLint geGetUniformLocationWithLog(const char* name);

#endif //GAMEENGINE_SHADER_H
