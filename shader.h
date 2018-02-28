//
// Created by Afterwind on 9/17/2017.
//

#ifndef GAMEENGINE_SHADER_H
#define GAMEENGINE_SHADER_H

typedef enum {
    GE_PROGRAM_MAIN,
    GE_PROGRAM_TEXTURE,
    GE_PROGRAM_SHADOW,
    GE_PROGRAM_GUI,

    GE_NUM_PROGRAMS
} SHADER;

GLuint vertexShaders[10];
GLuint fragmentShaders[10];
GLuint programs[10];

void initAllShaders();

#endif //GAMEENGINE_SHADER_H
