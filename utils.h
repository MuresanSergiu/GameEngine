//
// Created by Afterwind on 9/18/2017.
//

#ifndef GAMEENGINE_UTILS_H
#define GAMEENGINE_UTILS_H

#include <stdbool.h>
#include <time.h>
#include "types.h"

#define PI 3.14159265f
#define STRINGIFY(X) #X
#define TIME_START2\
    struct timeval tStart, tEnd;\
    gettimeofday(&tStart, NULL)
#define TIME_END2(MSG)\
    gettimeofday(&tEnd, NULL);\
    printf("Time for " MSG ": %.2lfms\n", timeDiff(tEnd, tStart))
#define TIME_START\
    struct timespec tStart, tEnd;\
    clock_gettime(CLOCK_MONOTONIC, &tStart);
#define TIME_END(MSG)\
    clock_gettime(CLOCK_MONOTONIC, &tEnd);\
    printf("Time for " MSG ": %lluns", ((size_t) difftime(tEnd.tv_sec, tStart.tv_sec)) * 1000000000 + tEnd.tv_nsec - tStart.tv_nsec);\
    printf("(~%llums)\n", (((size_t) difftime(tEnd.tv_sec, tStart.tv_sec)) * 1000000000 + tEnd.tv_nsec - tStart.tv_nsec) / 1000000);


void readFile(char* path, char* dest);
void printFace(geVertex* v);
void printVec3(kmVec3* v);
double timeDiff(struct timeval t1, struct timeval t2);
float perlinNoise(unsigned long long x, unsigned long long y, int octaves, float roughness, float scale);
float planeCoordinate(geVertex* v);
void removeFace(gePlane* plane, size_t offsetVertex, size_t numFaces);

#endif //GAMEENGINE_UTILS_H
