//
// Created by Afterwind on 9/18/2017.
//

#ifndef GAMEENGINE_UTILS_H
#define GAMEENGINE_UTILS_H

#include <stdbool.h>
#include <sys/time.h>
#include "types.h"

#define PI 3.14159265f
#define STRINGIFY(X) #X
#define TIME_START\
    struct timeval tStart, tEnd;\
    gettimeofday(&tStart, NULL)
#define TIME_END(MSG)\
    gettimeofday(&tEnd, NULL);\
    printf("Time for " MSG ": %.2lfms\n", timeDiff(tEnd, tStart))

char** readFile(char* path);
void printFace(geVertex* v);
void printVec3(kmVec3* v);
double timeDiff(struct timeval t1, struct timeval t2);
float perlinNoise(unsigned long long x, unsigned long long y, int octaves, float roughness, float scale);
float planeCoordinate(geVertex* v);
void removeFace(gePlane* plane, size_t offsetVertex, size_t offsetIndex, size_t numFaces);

#endif //GAMEENGINE_UTILS_H
