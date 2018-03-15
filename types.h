//
// Created by Afterwind on 9/23/2017.
//

#ifndef GAMEENGINE_TYPES_H
#define GAMEENGINE_TYPES_H

#include <GL/glew.h>
#include <kazmath/kazmath.h>
#include <stdbool.h>

typedef struct geVertex {
    kmVec3 pos;
    kmVec3 normal;
    kmVec3 texCoords;
} geVertex;

typedef struct geShape {
    geVertex* vertices;
    GLuint* indices;

    unsigned long long numVertices;
    unsigned long long numIndices;

    unsigned long long offsetBytesVertex;
    unsigned long long offsetBytesIndex;

    GLuint vao;
} geShape;

typedef struct geObject {
    GLuint texture;
    geShape* shape;

    kmVec3 pos;
    kmVec3 size;
    kmVec3 rotation;

} geObject;

typedef struct geCamera {
    kmVec3 pos;
    kmVec3 direction;
    kmVec3 up;

    float aspectRatio;
} geCamera;


#endif //GAMEENGINE_TYPES_H
