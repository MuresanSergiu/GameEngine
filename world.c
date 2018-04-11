#include <stdlib.h>
#include <stdio.h>
#include <mem.h>
#include <sys/time.h>
#include "world.h"
#include "utils.h"
#include "simplex_noise.h"

#define MAX_HEIGHT 128

/* INTERNAL FUNCTIONS */

// Given a face gets the coordinate for the first parameter with which face ordering is done
float firstOrder(geVertex* v) {
    if (v->normal.z == 1) { // front
        return v->pos.y;
    } else if (v->normal.z == -1) { // back
        return v->pos.y;
    } else if (v->normal.x == -1) { // left
        return v->pos.y;
    } else if (v->normal.x == 1) { // right
        return v->pos.y;
    } else if (v->normal.y == 1) { // top
        return v->pos.z;
    } else if (v->normal.y == -1) { // bottom
        return v->pos.z;
    } else {
        fprintf(stdout, "Could not find a proper side for quad: \n");
        printVec3(&v->pos);
        return 0;
    }
}

// Given a face gets the coordinate for the second parameter with which face ordering is done
float secondOrder(geVertex* v) {
    if (v->normal.z == 1) { // front
        return v->pos.x;
    } else if (v->normal.z == -1) { // back
        return v->pos.x;
    } else if (v->normal.x == -1) { // left
        return v->pos.z;
    } else if (v->normal.x == 1) { // right
        return v->pos.z;
    } else if (v->normal.y == 1) { // top
        return v->pos.x;
    } else if (v->normal.y == -1) { // bottom
        return v->pos.x;
    } else {
        fprintf(stdout, "Could not find a proper side for quad: \n");
        printVec3(&v->pos);
        return 0;
    }
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
        fprintf(stdout, "Could not find a proper side for quad: \n");
        printVec3(&v->pos);
        return 0;
    }
}

// Adds a face (4x geVertex and 6x indices) inside a sorted plane
void addFaceInOrderedPlane(geVertex* vertices, GLuint* indices, gePlane* planes, size_t* numPlanes) {
    size_t i;

    // Get which plane to add the quad in
    gePlane* destination = NULL;
    for (i = 0; i < *numPlanes; i++) {
        if (planeCoordinate(planes[i].vertices + 0) == planeCoordinate(vertices + 0)) {
            destination = planes + i;
            break;
        }
    }

    // Create it if it doesn't exist
    if (destination == NULL) {
        destination = planes + *numPlanes;
        destination->vertices = calloc(8192 * 4, sizeof(geVertex));
        destination->indices = calloc(8192 * 6, sizeof(GLuint));
        (*numPlanes)++;
    }

    // Find the position at which we'll add the face
    for (i = 0; i < destination->numVertices / 4; i++) {
        geVertex* planePos = destination->vertices + i * 4;
        if (firstOrder(planePos) > firstOrder(vertices + 0) || firstOrder(planePos) == firstOrder(vertices + 0) && secondOrder(planePos) > secondOrder(vertices + 0)) {
            break;
        }
    }

    // Add the face
    memcpy(destination->vertices + (i + 1) * 4, destination->vertices + i * 4, (destination->numVertices - i * 4) * sizeof(geVertex));
    memcpy(destination->indices + (i + 1) * 6, destination->indices + i * 6, (destination->numIndices - i * 6) * sizeof(GLuint));

    memcpy(destination->vertices + i * 4, vertices, sizeof(geVertex) * 4);
    memcpy(destination->indices + i * 6, indices, sizeof(GLuint) * 6);

    destination->numVertices += 4;
    destination->numIndices += 6;
}

gePlane compressPlaneWithGreedy(gePlane* plane) {
    size_t i;

    // The new resulting plane
    gePlane res = {
            .vertices = calloc(plane->numVertices, sizeof(geVertex)),
            .indices = calloc(plane->numIndices, sizeof(GLuint)),
            .numVertices = 0,
            .numIndices = 0
    };

    // Take texture coordinate diffs to expand based on the first vertex' coords
    kmVec3 diffFirstOrder, diffSecondOrder, temp;
    kmVec3Subtract(&diffFirstOrder, &plane->vertices[3].texCoords, &plane->vertices[0].texCoords);
    kmVec3Subtract(&diffSecondOrder, &plane->vertices[1].texCoords, &plane->vertices[0].texCoords);

    // Where we mark which faces have already been added to the new plane
    size_t* hashed = calloc(plane->numVertices / 4, sizeof(size_t));
    for (i = 0; i < plane->numVertices / 4; i++) {
        GLuint* indices = plane->indices + i * 6;
        geVertex* v = NULL;
        geVertex* u = NULL;

        // If already hashed, skip it
        if (hashed[i] != 0) {
            continue;
        }

        // Expand as much as possible on secondOrder
        geVertex* v1 = plane->vertices + i * 4;
        geVertex* v2 = plane->vertices + i * 4;

        while(i + 1 < plane->numVertices / 4 && (hashed[i + 1] == 0 && (firstOrder(v2) == firstOrder(v2 + 4) && secondOrder(v2) == secondOrder(v2 + 4) - 1))) {
            hashed[i + 1] = 1;
            v2 += 4;
            i++;
        }

        // Fix textures after expanding
        kmVec3Scale(&temp, &diffSecondOrder, ((v2 - v1) / 4));
        kmVec3Add(&(v2 + 1)->texCoords, &(v2 + 1)->texCoords, &temp);
        kmVec3Add(&(v2 + 2)->texCoords, &(v2 + 2)->texCoords, &temp);

        // Expand as much as possible on firstOrder as well
        geVertex* v3 = v1;
        geVertex* v4 = v2;

        size_t j = i;
        while(j < plane->numVertices) {
            // Find next v3 based on the last v4
            v = v4;
            while (j < plane->numVertices / 4 - 1 && (firstOrder(v3) == firstOrder(v) || firstOrder(v3) == firstOrder(v) - 1 && secondOrder(v3) > secondOrder(v))) {
                j++;
                v += 4;
            }

            if (hashed[j] != 0 || secondOrder(v3) != secondOrder(v) || firstOrder(v3) != firstOrder(v) - 1) {
                // Exit if there is no next element on the y for v3
                fprintf(stdout, "New v3 not found\n");
                break;
            }

            // Retain new v3 inside u
            u = v;

            // And start searching for the new v4
            while (j < plane->numVertices / 4 - 1 && firstOrder(v4) == firstOrder(v) - 1 && secondOrder(v4) > secondOrder(v)) {
                // The area between the new v3 and new v4 needs to be filled with non-hashed faces
                // Otherwise, there's no new v4
                if (hashed[j] != 0 || secondOrder(v) != secondOrder(v + 4) - 1) {
                    fprintf(stdout, "Non-continuous faces\n");
                    break;
                }

                hashed[j++] = 1;
                v += 4;
            }

            if (hashed[j] != 0 || secondOrder(v4) != secondOrder(v) || firstOrder(v4) != firstOrder(v) - 1) {
                // Exit if there is no next element on the y for v4
                fprintf(stdout, "Error for v4\n");
                break;
            }
            // Mark as hashed, the new v3 and new v4 are valid, assign them and try doing this again for a new row
            hashed[j] = 1;

            // Fix textures after expanding on firstOrder
            kmVec3Add(&(u + 3)->texCoords, &(v3 + 3)->texCoords, &diffFirstOrder);
            kmVec3Add(&(v + 2)->texCoords, &(v4 + 2)->texCoords, &diffFirstOrder);

            v3 = u;
            v4 = v;
        }

        // Revert hashed faces from u to v since last iteration must have failed to mesh
        if (u != v3) {
            size_t k;
            for (k = (u - plane->vertices) / 4; k <= (v - plane->vertices) / 4; k++) {
                hashed[k] = 0;
            }
        }

        // Copy the face to the new plane
        memcpy(res.vertices + res.numVertices, v1 + 0, sizeof(geVertex));
        res.numVertices++;
        memcpy(res.vertices + res.numVertices, v2 + 1, sizeof(geVertex));
        res.numVertices++;
        memcpy(res.vertices + res.numVertices, v4 + 2, sizeof(geVertex));
        res.numVertices++;
        memcpy(res.vertices + res.numVertices, v3 + 3, sizeof(geVertex));
        res.numVertices++;

        memcpy(res.indices + res.numIndices, indices, sizeof(GLuint) * 6);
        res.numIndices += 6;
    }

    free(plane->vertices);
    free(plane->indices);

    // Override the old plane and reallocate to save memory
    realloc(res.vertices, res.numVertices * sizeof(geVertex));
    realloc(res.indices, res.numIndices * sizeof(GLuint));

    plane->vertices = res.vertices;
    plane->indices = res.indices;
    plane->numVertices = res.numVertices;
    plane->numIndices = res.numIndices;

    free(hashed);

    return res;
}

void generateMeshWithGreedy() {
    struct timeval tStart, tEnd;
    float x = 0, y = 0, z = 0, size = 1.0f;
    size_t l, k, j;
    size_t oX, oY, oZ;
    size_t numVertices = 24, numIndices = 36;
    size_t indexOffset = 0, currentBlockIndex = 0;

    gePlane* planes[6]; // front - back - left - right - up - down
    unsigned long long numPlanes[6] = { 0 };

    gettimeofday(&tStart, NULL);

    // <editor-fold> INIT STAGE
    geVertex vertices[] = {
            // FRONT
            {{ x - size / 2, y - size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {0, 0}},
            {{ x + size / 2, y - size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {1, 0}},
            {{ x + size / 2, y + size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {1, 1}},
            {{ x - size / 2, y + size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {0, 1}},
            // BACK
            {{ x - size / 2, y - size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {1, 0}},
            {{ x + size / 2, y - size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {0, 0}},
            {{ x + size / 2, y + size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {0, 1}},
            {{ x - size / 2, y + size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {1, 1}},
            // LEFT
            {{ x - size / 2, y - size / 2, z - size / 2 }, { -1.0f, 0.0f, 0.0f }, {0, 0}},
            {{ x - size / 2, y - size / 2, z + size / 2 }, { -1.0f, 0.0f, 0.0f }, {1, 0}},
            {{ x - size / 2, y + size / 2, z + size / 2 }, { -1.0f, 0.0f, 0.0f }, {1, 1}},
            {{ x - size / 2, y + size / 2, z - size / 2 }, { -1.0f, 0.0f, 0.0f }, {0, 1}},
            // RIGHT
            {{ x + size / 2, y - size / 2, z - size / 2 }, { 1.0f, 0.0f, 0.0f }, {1, 0}},
            {{ x + size / 2, y - size / 2, z + size / 2 }, { 1.0f, 0.0f, 0.0f }, {0, 0}},
            {{ x + size / 2, y + size / 2, z + size / 2 }, { 1.0f, 0.0f, 0.0f }, {0, 1}},
            {{ x + size / 2, y + size / 2, z - size / 2 }, { 1.0f, 0.0f, 0.0f }, {1, 1}},
            // TOP
            {{ x - size / 2, y + size / 2, z - size / 2 }, { 0.0f, 1.0f, 0.0f }, {0, 0}},
            {{ x + size / 2, y + size / 2, z - size / 2 }, { 0.0f, 1.0f, 0.0f }, {1, 0}},
            {{ x + size / 2, y + size / 2, z + size / 2 }, { 0.0f, 1.0f, 0.0f }, {1, 1}},
            {{ x - size / 2, y + size / 2, z + size / 2 }, { 0.0f, 1.0f, 0.0f }, {0, 1}},
            // BOTTOM
            {{ x - size / 2, y - size / 2, z - size / 2 }, { 0.0f, -1.0f, 0.0f }, {0, 0}},
            {{ x + size / 2, y - size / 2, z - size / 2 }, { 0.0f, -1.0f, 0.0f }, {1, 0}},
            {{ x + size / 2, y - size / 2, z + size / 2 }, { 0.0f, -1.0f, 0.0f }, {1, 1}},
            {{ x - size / 2, y - size / 2, z + size / 2 }, { 0.0f, -1.0f, 0.0f }, {0, 1}},
    };

    GLuint indices[] = {
            // FRONT
            0, 1, 2,
            2, 3, 0,
            // BACK
            4, 7, 6,
            6, 5, 4,
            // LEFT
            8, 9, 10,
            10, 11, 8,
            // RIGHT
            12, 15, 14,
            14, 13, 12,
            // TOP
            16, 19, 18,
            18, 17, 16,
            // BOTTOM
            20, 21, 22,
            22, 23, 20
    };


    for (oX = 0; oX < world.sizeX; oX++) {
        for (oZ = 0; oZ < world.sizeZ; oZ++) {
            for (oY = 0; oY < MAX_HEIGHT; oY++) {
                if (world.map[oX][oZ][oY] == 0) {
                    continue;
                }
                for (k = 0; k < numVertices; k++) {
                    geVertex* vertexBlock = vertices + k;
                    geVertex* vertexWorld = world.shape.vertices + (currentBlockIndex * numVertices + k);

                    vertexWorld->normal.x = vertexBlock->normal.x;
                    vertexWorld->normal.y = vertexBlock->normal.y;
                    vertexWorld->normal.z = vertexBlock->normal.z;

                    vertexWorld->pos.x = vertexBlock->pos.x + oX;
                    vertexWorld->pos.y = vertexBlock->pos.y + oY;
                    vertexWorld->pos.z = vertexBlock->pos.z + oZ;

                    vertexWorld->texCoords.x = vertexBlock->texCoords.x;
                    vertexWorld->texCoords.y = vertexBlock->texCoords.y;
                    vertexWorld->texCoords.z = vertexBlock->texCoords.z;
                }
                for (k = 0; k < numIndices; k++) {
                    world.shape.indices[currentBlockIndex * numIndices + k] = (GLuint) (indices[k] /*+ currentBlockIndex * numVertices*/);
                }
                currentBlockIndex++;
            }
        }
    }
    gettimeofday(&tEnd, NULL);
    printf("Time for initializing voxel world: %.2lfms\n", timeDiff(tEnd, tStart));
    gettimeofday(&tStart, NULL);

    // </editor-fold>

    // <editor-fold> CULLING STAGE
    planes[0] = calloc(world.sizeZ, sizeof(gePlane));
    planes[1] = calloc(world.sizeZ, sizeof(gePlane));
    planes[2] = calloc(world.sizeX, sizeof(gePlane));
    planes[3] = calloc(world.sizeX, sizeof(gePlane));
    planes[4] = calloc(MAX_HEIGHT, sizeof(gePlane));
    planes[5] = calloc(MAX_HEIGHT, sizeof(gePlane));

    currentBlockIndex = 0;
    for (oX = 0; oX < world.sizeX; oX++) {
        for (oZ = 0; oZ < world.sizeZ; oZ++) {
            for (oY = 0; oY < MAX_HEIGHT; oY++) {
                if (world.map[oX][oZ][oY] == 0) {
                    continue;
                }

                bool isAdjacent[] = {
                        oZ + 1 < world.sizeZ && world.map[oX][oZ + 1][oY] != 0,
                        oZ != 0 && world.map[oX][oZ - 1][oY] != 0,

                        oX != 0 && world.map[oX - 1][oZ][oY] != 0,
                        oX + 1 < world.sizeX && world.map[oX + 1][oZ][oY] != 0,

                        oY + 1 < MAX_HEIGHT && world.map[oX][oZ][oY + 1] != 0,
                        oY != 0 && world.map[oX][oZ][oY - 1] != 0,
                };

                for (k = 0; k < 6; k++) {
                    if (!isAdjacent[k]) {
                        // Remove indices based on face of the cube in favour of square based indices
                        size_t p;
                        for (p = 0; p < 6; p++) {
                            world.shape.indices[(currentBlockIndex * numIndices + k * 6) + p] -= k * 4;
                        }

                        // Add the quad to an ordered plane
                        addFaceInOrderedPlane(world.shape.vertices + (currentBlockIndex * numVertices + k * 4),
                                              world.shape.indices + (currentBlockIndex * numIndices + k * 6),
                                              planes[k], numPlanes + k);
                    }
                }
                currentBlockIndex++;
            }
        }
    }

    gettimeofday(&tEnd, NULL);
    printf("Time for culling and sorting voxel world: %.2lfms\n", timeDiff(tEnd, tStart));
    // </editor-fold>

    // <editor-fold> GREEDY STAGE
    gettimeofday(&tStart, NULL);
    for (k = 0; k < 6; k++) {
        for (l = 0; l < numPlanes[k]; l++) {
            gePlane* plane = &planes[k][l];

            size_t numIndicesBefore = plane->numIndices, numVerticesBefore = plane->numVertices;
            compressPlaneWithGreedy(plane);
            printf("%llu) Before %llu, %llu and after %llu, %llu\n", k, numVerticesBefore, numIndicesBefore, plane->numVertices, plane->numIndices);
        }
    }

    // Fix indices since all of them are either
    // 0 1 2 2 3 0 or
    // 0 3 2 2 1 0
    for (k = 0; k < 6; k++) {
        for (l = 0; l < numPlanes[k]; l++) {
            gePlane* plane = &planes[k][l];
//            fprintf(stderr, "Test\n");

            fprintf(stderr, "%llu) %llu, %llu\n", k, plane->numIndices, plane->numVertices);
            for (j = 0; j < plane->numVertices / 4; j++) {
//                if (k == 5) {
//                    printFace(plane->vertices + j * 4);
//                    printf("%u ", plane->indices[j * 6]);
//                    printf("%u ", plane->indices[j * 6 + 1]);
//                    printf("%u ", plane->indices[j * 6 + 2]);
//                    printf("%u ", plane->indices[j * 6 + 3]);
//                    printf("%u ", plane->indices[j * 6 + 4]);
//                    printf("%u \n", plane->indices[j * 6 + 5]);
//                }
                plane->indices[j * 6] += indexOffset;
                plane->indices[j * 6 + 1] += indexOffset;
                plane->indices[j * 6 + 2] += indexOffset;
                plane->indices[j * 6 + 3] += indexOffset;
                plane->indices[j * 6 + 4] += indexOffset;
                plane->indices[j * 6 + 5] += indexOffset;
                indexOffset += 4;
            }
            memcpy(world.shape.vertices + indexOffset - plane->numVertices, plane->vertices, plane->numVertices * sizeof(geVertex));
            memcpy(world.shape.indices + indexOffset * 3 / 2 - plane->numIndices, plane->indices,  plane->numIndices * sizeof(GLuint));
        }
    }

    world.shape.numVertices = indexOffset;
    world.shape.numIndices = indexOffset * 3 / 2;

    printf("Total size is %llu and %llu\n", world.shape.numVertices, world.shape.numIndices);

    realloc(world.shape.vertices, world.shape.numVertices * sizeof(geVertex));
    realloc(world.shape.indices, world.shape.numIndices * sizeof(GLuint));

    for (k = 0; k < 6; k++) {
        for (l = 0; l < numPlanes[k]; l++) {
//            for (j = 0; j < planes[k][l].numVertices; j += 4) {
//                printFace(planes[k][l].vertices + j);
//            }
            free(planes[k][l].vertices);
//            for (j = 0; j < planes[k][l].numIndices; j += 6) {
//                printf("%llu) %u %u %u %u %u %u\n", j, planes[k][l].indices[j], planes[k][l].indices[j + 1], planes[k][l].indices[j + 2], planes[k][l].indices[j + 3], planes[k][l].indices[j + 4], planes[k][l].indices[j + 5]);
//            }
            free(planes[k][l].indices);
        }
        free(planes[k]);
    }

    gettimeofday(&tEnd, NULL);
    printf("Time for greedy on voxel world: %.2lfms\n", timeDiff(tEnd, tStart));

    // </editor-fold>
}

/* EXTERNAL FUNCTIONS */

void initWorld(size_t sizeX, size_t sizeY, size_t sizeZ) {
    world.sizeX = sizeX;
    world.sizeY = sizeY;
    world.sizeZ = sizeZ;

    size_t oX, oZ;
    world.map = calloc(sizeX, sizeof(long long*));
    for (oX = 0; oX < sizeX; oX++) {
        world.map[oX] = calloc(sizeZ, sizeof(long long*));
        for (oZ = 0; oZ < sizeZ; oZ++) {
            world.map[oX][oZ] = calloc(sizeY, sizeof(long long));
        }
    }

    generateWorld(32);
}

void destroyWorld() {
    size_t oX, oZ;
    for (oX = 0; oX < world.sizeX; oX++) {
        for (oZ = 0; oZ < world.sizeZ; oZ++) {
            free(world.map[oX][oZ]);
        }
        free(world.map[oX]);
    }
    free(world.map);
}

void generateWorld(size_t heightOffsetIntesnsity) {
    size_t numVertices = 24;
    size_t numIndices = 36;
    size_t oX, oZ, oY, arrayLength = 0;

    for (oX = 0; oX < world.sizeX; oX++) {
        for (oZ = 0; oZ < world.sizeZ; oZ++) {
            int noise = (int)(((1 + sdnoise2(((float) oX) / 32.0f, ((float) oZ) / 32.0f, NULL, NULL)) / 2.0f) * heightOffsetIntesnsity + world.sizeY / 8);
//            int noise = (int) floorf(perlinNoise(oX, oZ, 16));// * MAX_HEIGHT);
//            int noise = (int) floorf(perlinNoise(oX, oZ, 16));// * MAX_HEIGHT);
//            printf("Got noise %f\n", perlinNoise(oX, oZ, 16));
            for (oY = 0; oY < noise; oY++) {
                world.map[oX][oZ][oY] = 1;
            }
            arrayLength += (size_t) (noise);
        }
    }

    world.shape.numVertices = numVertices * arrayLength;
    world.shape.numIndices = numIndices * arrayLength;
    world.shape.vertices = calloc(numVertices * arrayLength, sizeof(geVertex));
    world.shape.indices = calloc(numIndices * arrayLength, sizeof(GLuint));

    printf("Array length for WORLD is %llu\n", arrayLength);

    generateMeshWithGreedy();
}

long long findInWorld(kmVec3* v) {
    size_t x, y, z;
    x = (size_t) (v->x);// + 1.5f);
    y = (size_t) (v->y);// + 0.5f);
    z = (size_t) (v->z);// + 1.5f);

    if (0 <= x && x < world.sizeX && 0 <= y && y < world.sizeY && 0 <= z && z < world.sizeZ) {
        return world.map[x][z][y];
    }
    return -1;
}