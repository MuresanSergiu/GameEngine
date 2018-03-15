//
// Created by Afterwind on 9/19/2017.
//

#ifndef GAMEENGINE_DRAW_H
#define GAMEENGINE_DRAW_H

#include <kazmath/kazmath.h>
#include <stdbool.h>
#include "types.h"
#include "geometry.h"

#define MAX_OBJECTS 1024

GLuint vaos[INDEX_NUM];
GLuint rbos[1];
GLuint fbos[50];
GLuint vbos[2];

size_t numObjects;
geObject objects[MAX_OBJECTS];

void addObject(geObject* obj);
void addObjects(geObject* obj, size_t num);

void initScene();
void update();
void drawScene();
void drawObject(geObject* obj);
void clearScene();

#endif //GAMEENGINE_DRAW_H
