//
// Created by Afterwind on 9/18/2017.
//

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <stdbool.h>
#include <math.h>
#include <_timeval.h>
#include <mem.h>
#include "simplex_noise.h"
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
    } while (charsRead > 0);
    result = calloc(i, sizeof(char*));

    for (j = 0; j < i; j++) {
        result[j] = resultTemp[j];
    }
    fclose(fp);
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

double timeDiff(struct timeval t1, struct timeval t2) {
    return (t1.tv_sec - t2.tv_sec) * 1000.0 + (t1.tv_usec - t2.tv_usec) / 1000.0;
}

static int hash[256] = {
        208, 34, 231, 213, 32, 248, 233, 56, 161, 78, 24, 140, 71, 48, 140, 254, 245, 255, 247, 247, 40,
        185, 248, 251, 245, 28, 124, 204, 204, 76, 36, 1, 107, 28, 234, 163, 202, 224, 245, 128, 167, 204,
        9, 92, 217, 54, 239, 174, 173, 102, 193, 189, 190, 121, 100, 108, 167, 44, 43, 77, 180, 204, 8, 81,
        70, 223, 11, 38, 24, 254, 210, 210, 177, 32, 81, 195, 243, 125, 8, 169, 112, 32, 97, 53, 195, 13,
        203, 9, 47, 104, 125, 117, 114, 124, 165, 203, 181, 235, 193, 206, 70, 180, 174, 0, 167, 181, 41,
        164, 30, 116, 127, 198, 245, 146, 87, 224, 149, 206, 57, 4, 192, 210, 65, 210, 129, 240, 178, 105,
        228, 108, 245, 148, 140, 40, 35, 195, 38, 58, 65, 207, 215, 253, 65, 85, 208, 76, 62, 3, 237, 55, 89,
        232, 50, 217, 64, 244, 157, 199, 121, 252, 90, 17, 212, 203, 149, 152, 140, 187, 234, 177, 73, 174,
        193, 100, 192, 143, 97, 53, 145, 135, 19, 103, 13, 90, 135, 151, 199, 91, 239, 247, 33, 39, 145,
        101, 120, 99, 3, 186, 86, 99, 41, 237, 203, 111, 79, 220, 135, 158, 42, 30, 154, 120, 67, 87, 167,
        135, 176, 183, 191, 253, 115, 184, 21, 233, 58, 129, 233, 142, 39, 128, 211, 118, 137, 139, 255,
        114, 20, 218, 113, 154, 27, 127, 246, 250, 1, 8, 198, 250, 209, 92, 222, 173, 21, 88, 102, 219
};

int random(float x, float y) {
    int tmp = hash[((int)y % 256)];
    return hash[(int)(tmp + x) % 256];
}

float sample(float x, float y, float amp) {
    // x and y between 0 and 15
    float r00 = (1 + sdnoise2((x / amp) * amp, (y / amp) * amp, NULL, NULL)) / 2.0f;
    float r10 = (1 + sdnoise2((x / amp) * amp + 1, (y / amp) * amp, NULL, NULL)) / 2.0f;
    float r11 = (1 + sdnoise2((x / amp + 1) * amp, (y / amp + 1) * amp, NULL, NULL)) / 2.0f;
    float r01 = (1 + sdnoise2((x / amp) * amp, (y / amp + 1) * amp, NULL, NULL)) / 2.0f;

    float r0 = r00 * (1 - x / 16) + (x / 16) * r01;
    float r1 = r10 * (1 - x / 16) + (x / 16) * r11;

    return r0 * (1 - y / 16) + (y / 16) * r1;
}

float sample2(float x, float y, float amp) {
    return (1 + sdnoise2(x / amp, y / amp, NULL, NULL)) / 2.0f;
}

float perlinNoise(unsigned long long x, unsigned long long y, int octaves, float roughness, float scale) {
//    x = x % (frequency);
//    y = y % (frequency);
    float noise = 0;
    float weight = 1.0;

    float ampAccumulated = 0;

    int i;
    for (i = 0; i < octaves; i++) {
//        ampAccumulated += amp;
//        noise += sample2(x * weight, y * amp, __max((int) (frequency * amp), 1)) * amp;

        weight *= roughness;
    }

    return noise / ampAccumulated;
}

// Given a face gets the coordinate of the plane in which it resides
float planeCoordinate(geVertex* v) {
    if (v->normal.z == 1) { // front
        return v->pos.z;
    } else if (v->normal.z == -1) { // back
        return v->pos.z;
    } else if (v->normal.x == -1) { // left
        return v->pos.x;
    } else if (v->normal.x == 1) { // right
        return v->pos.x;
    } else if (v->normal.y == 1) { // top
        return v->pos.y;
    } else if (v->normal.y == -1) { // bottom
        return v->pos.y;
    } else {
#ifdef DEBUG_GREEDY
        fprintf(stdout, "Could not find a proper side for quad: \n");
        printVec3(&v->pos);
#endif
        return 0;
    }
}

// Given a shape removes a face
void removeFace(gePlane* plane, size_t offsetVertex, size_t offsetIndex, size_t numFaces) {
    size_t k;
    memcpy(
            plane->vertices + offsetVertex,
            plane->vertices + (offsetVertex + 4 * numFaces),
            sizeof(geVertex) * (plane->numVertices - (offsetVertex + 4 * numFaces))
    );
    memcpy(
            plane->indices + offsetIndex,
            plane->indices + (offsetIndex + 6 * numFaces),
            sizeof(GLuint) * (plane->numIndices - (offsetIndex + 6 * numFaces))
    );
//    for (k = offsetIndex; k < plane->numIndices; k++) {
//        plane->indices[k] -= (4 * numFaces);
//    }
    plane->numVertices -= (4 * numFaces);
    plane->numIndices -= (6 * numFaces);
}
