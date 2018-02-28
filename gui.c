//
// Created by Afterwind on 10/15/2017.
//

#include <GL/glew.h>
#include <stdio.h>
#include "gui.h"
#include "shader.h"

/* EXTERNAL FUNCTIONS */

void initGUI() {
    geGUIElements[0].x = 1;
    geGUIElements[0].y = 1;

    geGUIElements[1].x = -1;
    geGUIElements[1].y = 1;

    geGUIElements[2].x = 1;
    geGUIElements[2].y = -1;

    geGUIElements[3].x = -1;
    geGUIElements[3].y = -1;

    glUseProgram(programs[GE_PROGRAM_GUI]);
    glGenBuffers(1, &vboGUI);
    glGenVertexArrays(1, &vaoGUI);

    glBindBuffer(GL_ARRAY_BUFFER, vboGUI);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geGUIElements[0]) * 4, geGUIElements, GL_STATIC_DRAW);

    glBindVertexArray(vaoGUI);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("%s\n", gluErrorString(err));
    }
}

void drawGUI() {
    glUseProgram(programs[GE_PROGRAM_GUI]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vboGUI);
    glBindVertexArray(vaoGUI);
//    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}