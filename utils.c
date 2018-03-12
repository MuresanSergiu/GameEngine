//
// Created by Afterwind on 9/18/2017.
//

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <stdbool.h>
#include <math.h>
#include "utils.h"

#define GE_MAX_LINES 4096
#define GE_BUF_SIZE 4096

/* EXTERNAL FUNCTIONS */

char** readFile(char* path) {
    FILE* fp = fopen(path, "r");
    size_t charsRead, i = 0, j;
    char* resultTemp[GE_MAX_LINES];
    char** result;

    if (fp == NULL) {
        fprintf(stdout, "Failed to read file %s\n", path);
        exit(EXIT_FAILURE);
    }
    do {
        char* buffer = calloc(GE_BUF_SIZE, sizeof(char));
        charsRead = fread(buffer, sizeof(char), GE_BUF_SIZE, fp);
        buffer[charsRead] = 0;
        resultTemp[i++] = buffer;
    } while(charsRead > 0);
    result = calloc(i, sizeof(char*));

    for (j = 0; j < i; j++) {
        result[j] = resultTemp[j];
    }
    return result;
}

void printFace(geVertex* v) {
    printf("Positions are \n\t");
    printVec3(&v[0].pos);
    printf("\n\t");
    printVec3(&v[1].pos);
    printf("\n\t");
    printVec3(&v[2].pos);
    printf("\n\t");
    printVec3(&v[3].pos);
    printf("\nAnd normal is \n\t");
    printVec3(&v[0].normal);
    printf("\nAnd texture positions are \n\t");
    printVec3(&v[0].texCoords);
    printf("\n\t");
    printVec3(&v[1].texCoords);
    printf("\n\t");
    printVec3(&v[2].texCoords);
    printf("\n\t");
    printVec3(&v[3].texCoords);
    printf("\n");
}

void printVec3(kmVec3* v) {
    printf("( %f, %f, %f )", v->x, v->y, v->z);
}