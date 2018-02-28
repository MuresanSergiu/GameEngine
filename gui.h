//
// Created by Afterwind on 10/15/2017.
//

#ifndef GAMEENGINE_GUI_H
#define GAMEENGINE_GUI_H

#include <kazmath/kazmath.h>

kmVec2 geGUIElements[50];
GLuint vboGUI;
GLuint vaoGUI;

void initGUI();
void drawGUI();

#endif //GAMEENGINE_GUI_H
