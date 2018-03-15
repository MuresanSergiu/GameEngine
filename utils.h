//
// Created by Afterwind on 9/18/2017.
//

#ifndef GAMEENGINE_UTILS_H
#define GAMEENGINE_UTILS_H

#include <stdbool.h>
#include "types.h"

#define PI 3.14159265f
#define STRINGIFY(X) #X

char** readFile(char* path);
void printFace(geVertex* v);
void printVec3(kmVec3* v);

#endif //GAMEENGINE_UTILS_H
