#include <stdlib.h>
#include <stdio.h>
#include <mem.h>
#include <sys/time.h>
#include "world.h"
#include "utils.h"
#include "geometry.h"
#include "simplex_noise.h"

/* EXTERNAL FUNCTIONS */
void geWorldInit(size_t sizeX, size_t sizeY, size_t sizeZ) {
    worldMain.sizeX = sizeX;
    worldMain.sizeY = sizeY;
    worldMain.sizeZ = sizeZ;
    worldMain.numBlocks = 0;

    size_t oX, oZ;
    worldMain.map = calloc(sizeX, sizeof(long long*));
    for (oX = 0; oX < sizeX; oX++) {
        worldMain.map[oX] = calloc(sizeZ, sizeof(long long*));
        for (oZ = 0; oZ < sizeZ; oZ++) {
            worldMain.map[oX][oZ] = calloc(sizeY, sizeof(long long));
        }
    }

    geWorldGenerate(worldMain.sizeY / 2, worldMain.sizeY / 2);
}

void geWorldDestroy() {
    size_t oX, oZ, k, l;
    for (oX = 0; oX < worldMain.sizeX; oX++) {
        for (oZ = 0; oZ < worldMain.sizeZ; oZ++) {
            free(worldMain.map[oX][oZ]);
        }
        free(worldMain.map[oX]);
    }
    free(worldMain.map);

    for (k = 0; k < 6; k++) {
        for (l = 0; l < worldMain.numPlanes[k]; l++) {
            free(worldMain.planes[k][l].vertices);
            free(worldMain.planes[k][l].indices);
        }
        free(worldMain.planes[k]);
        worldMain.numPlanes[k] = 0;
    }
}

void geWorldGenerate(size_t baseHeight, size_t heightOffsetIntesnsity) {
    size_t oX, oZ, oY;

    for (oX = 0; oX < worldMain.sizeX; oX++) {
        for (oZ = 0; oZ < worldMain.sizeZ; oZ++) {
            int noise = (int)(((1 + sdnoise2(((float) oX) / 32.0f, ((float) oZ) / 32.0f, NULL, NULL)) / 2.0f) * heightOffsetIntesnsity + baseHeight);
            for (oY = 0; oY < noise; oY++) {
                worldMain.map[oX][oZ][oY] = 1;
            }
            worldMain.numBlocks += (size_t) (noise);
        }
    }

    geWorldGenerateShape(&worldMain, false);
    geWorldGenerateCulledPlanes(&worldMain);
    geWorldCompressCulledPlanesWithGreedy(&worldMain);
    geWorldShapeFromPlanes(&worldMain);
}

kmVec3 geWorldFind(kmVec3* v) {
    kmVec3 result;
    size_t x, y, z;
    x = (size_t) floorf(v->x + 0.5f);
    y = (size_t) floorf(v->y + 0.5f);
    z = (size_t) floorf(v->z + 0.5f);
    if (0 <= x && x < worldMain.sizeX && 0 <= y && y < worldMain.sizeY && 0 <= z && z < worldMain.sizeZ && worldMain.map[x][z][y] != 0) {
        result.x = x;
        result.y = y;
        result.z = z;
    } else {
        result.x = result.y = result.z = -1;
    }
    return result;
}

void geWorldRemoveBlock(kmVec3* v) {
    size_t x, y, z;
    x = (size_t) v->x;
    y = (size_t) v->y;
    z = (size_t) v->z;

    worldMain.map[x][z][y] = 0;

    worldMain.numBlocks--;

    geWorldGenerateShape(&worldMain, false);
    geWorldGenerateCulledPlanes(&worldMain);
    geWorldCompressCulledPlanesWithGreedy(&worldMain);
    geWorldShapeFromPlanes(&worldMain);
}