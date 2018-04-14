#include <stdlib.h>
#include <stdio.h>
#include <mem.h>
#include <sys/time.h>
#include "world.h"
#include "utils.h"
#include "geometry.h"
#include "simplex_noise.h"

/* EXTERNAL FUNCTIONS */
void initWorld(size_t sizeX, size_t sizeY, size_t sizeZ) {
    world.sizeX = sizeX;
    world.sizeY = sizeY;
    world.sizeZ = sizeZ;
    world.numBlocks = 0;

    size_t oX, oZ;
    world.map = calloc(sizeX, sizeof(long long*));
    for (oX = 0; oX < sizeX; oX++) {
        world.map[oX] = calloc(sizeZ, sizeof(long long*));
        for (oZ = 0; oZ < sizeZ; oZ++) {
            world.map[oX][oZ] = calloc(sizeY, sizeof(long long));
        }
    }

    generateWorld(world.sizeY / 2, world.sizeY / 2);
}

void destroyWorld() {
    size_t oX, oZ, k, l;
    for (oX = 0; oX < world.sizeX; oX++) {
        for (oZ = 0; oZ < world.sizeZ; oZ++) {
            free(world.map[oX][oZ]);
        }
        free(world.map[oX]);
    }
    free(world.map);

    for (k = 0; k < 6; k++) {
        for (l = 0; l < world.numPlanes[k]; l++) {
            free(world.planes[k][l].vertices);
            free(world.planes[k][l].indices);
        }
        free(world.planes[k]);
        world.numPlanes[k] = 0;
    }
}

void generateWorld(size_t baseHeight, size_t heightOffsetIntesnsity) {
    size_t oX, oZ, oY;

    for (oX = 0; oX < world.sizeX; oX++) {
        for (oZ = 0; oZ < world.sizeZ; oZ++) {
            int noise = (int)(((1 + sdnoise2(((float) oX) / 32.0f, ((float) oZ) / 32.0f, NULL, NULL)) / 2.0f) * heightOffsetIntesnsity + baseHeight);
            for (oY = 0; oY < noise; oY++) {
                world.map[oX][oZ][oY] = 1;
            }
            world.numBlocks += (size_t) (noise);
        }
    }

    geWorldGenerateShape(&world, false);
    geWorldGenerateCulledPlanes(&world);
    geWorldCompressCulledPlanesWithGreedy(&world);
    geWorldShapeFromPlanes(&world);
}

kmVec3 findInWorld(kmVec3* v) {
    kmVec3 result;
    size_t x, y, z;
    x = (size_t) floorf(v->x + 0.5f);
    y = (size_t) floorf(v->y + 0.5f);
    z = (size_t) floorf(v->z + 0.5f);
    if (0 <= x && x < world.sizeX && 0 <= y && y < world.sizeY && 0 <= z && z < world.sizeZ && world.map[x][z][y] != 0) {
        result.x = x;
        result.y = y;
        result.z = z;
    } else {
        result.x = result.y = result.z = -1;
    }
    return result;
}

void removeBlockFromWorld(kmVec3* v) {
    size_t x, y, z;
    x = (size_t) v->x;
    y = (size_t) v->y;
    z = (size_t) v->z;

    world.map[x][z][y] = 0;

    world.numBlocks--;

    geWorldGenerateShape(&world, false);
    geWorldGenerateCulledPlanes(&world);
    geWorldCompressCulledPlanesWithGreedy(&world);
    geWorldShapeFromPlanes(&world);
}