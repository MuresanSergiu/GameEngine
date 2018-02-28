//
// Created by Afterwind on 9/19/2017.
//

#ifndef GAMEENGINE_DRAW_H
#define GAMEENGINE_DRAW_H

#include <kazmath/kazmath.h>
#include <stdbool.h>
#include "types.h"
#include "geometry.h"

GLuint vaos[INDEX_NUM];
GLuint rbos[1];
GLuint fbos[50];
GLuint vbos[2];

geObject objects[774];

void initScene();
void update();
void drawScene();
void drawObject(geObject* obj);
void clearScene();

#endif //GAMEENGINE_DRAW_H
