#include <stdlib.h>
#include <stdio.h>
#include <mem.h>
#include <sys/time.h>
#include "world.h"
#include "utils.h"
#include "geometry.h"
#include "simplex_noise.h"
#include "test.h"

#define PER_CUBE_VERTICES 24
#define PER_CUBE_INDICES 36

static const float x = 0;
static const float y = 0;
static const float z = 0;
static const float size = 1.0f;

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
#ifdef DEBUG_GREEDY
        fprintf(stdout, "Could not find a proper side for quad: \n");
        printVec3(&v->pos);
#endif
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
#ifdef DEBUG_GREEDY
        fprintf(stdout, "Could not find a proper side for quad: \n");
        printVec3(&v->pos);
#endif
        return 0;
    }
}

// Adds a face (4x geVertex and 6x indices) inside a sorted plane
void addFaceInOrderedPlane(geVertex* vertices, GLuint* indices, gePlane* planes, size_t* numPlanes) {
    size_t i;

    // Get which plane to add the quad in
    gePlane* destination = NULL;
    for (i = 0; i < *numPlanes; i++) {
        if (memcmp(&planes[i].vertices->normal, &vertices->normal, sizeof(kmVec3)) != 0) {
            *((int*) NULL);
        }

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

void geWorldGenerateShape(geWorld* world, bool withFullIndices) {
    TIME_START;
    size_t k;
    size_t oX, oY, oZ;
    size_t currentBlockIndex = 0;

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

    free(world->shape.vertices);
    free(world->shape.indices);

    world->shape.vertices = calloc(PER_CUBE_VERTICES * world->numBlocks, sizeof(geVertex));
    world->shape.indices = calloc(PER_CUBE_INDICES * world->numBlocks, sizeof(GLuint));
    world->shape.numVertices = PER_CUBE_VERTICES * world->numBlocks;
    world->shape.numIndices = PER_CUBE_INDICES * world->numBlocks;

    for (oX = 0; oX < world->sizeX; oX++) {
        for (oZ = 0; oZ < world->sizeZ; oZ++) {
            for (oY = 0; oY < world->sizeY; oY++) {
                if (world->map[oX][oZ][oY] == 0) {
                    continue;
                }
                for (k = 0; k < PER_CUBE_VERTICES; k++) {
                    geVertex* vertexBlock = vertices + k;
                    geVertex* vertexWorld = world->shape.vertices + (currentBlockIndex * PER_CUBE_VERTICES + k);

                    vertexWorld->normal.x = vertexBlock->normal.x;
                    vertexWorld->normal.y = vertexBlock->normal.y;
                    vertexWorld->normal.z = vertexBlock->normal.z;

                    vertexWorld->pos.x = vertexBlock->pos.x + oX;
                    vertexWorld->pos.y = vertexBlock->pos.y + oY;
                    vertexWorld->pos.z = vertexBlock->pos.z + oZ;

                    vertexWorld->texCoords.x = vertexBlock->texCoords.x;
                    vertexWorld->texCoords.y = vertexBlock->texCoords.y;
                    vertexWorld->texCoords.z = vertexBlock->texCoords.z;

                    vertexWorld->texCoords.z = world->map[oX][oZ][oY] - 1;
                }
                for (k = 0; k < PER_CUBE_INDICES; k++) {
                    if (withFullIndices) {
                        world->shape.indices[currentBlockIndex * PER_CUBE_INDICES + k] = (GLuint) (indices[k] + currentBlockIndex * PER_CUBE_VERTICES);
                    } else {
                        world->shape.indices[currentBlockIndex * PER_CUBE_INDICES + k] = (GLuint) (indices[k]);
                    }
                }
                currentBlockIndex++;
            }
        }
    }

//    for (i = 0; i < shape.numIndices; i++) {
//        if (i % 6 == 0) {
//            fprintf(stdout, "\n");
//        }
//        fprintf(stdout, "%u ", shape.indices[i]);
//    }
//    for (i = 0; i < 10; i++) {
//        fprintf(stdout, "\n");
//        printVec3(&shape.vertices[i].pos);
//    }
    printf("---------------- BASIC MESH ----------------------\n");
    TIME_END("generating basic mesh");
    printf("Total vertices and indices: %llu, %llu\n", world->shape.numVertices, world->shape.numIndices);
    if (withFullIndices) {
        printf("\n");
    }
}

void gePlaneCompressWithGreedy(gePlane* plane) {
    size_t i;
    size_t newNumVertices = 0, newNumIndices = 0;

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

        while(i + 1 < plane->numVertices / 4 && (hashed[i + 1] == 0 && (firstOrder(v2) == firstOrder(v2 + 4) && secondOrder(v2) == secondOrder(v2 + 4) - 1) && v2->texCoords.z == (v2 + 4)->texCoords.z)) {
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

            if (hashed[j] != 0 || secondOrder(v3) != secondOrder(v) || firstOrder(v3) != firstOrder(v) - 1 || v3->texCoords.z != v->texCoords.z) {
                // Exit if there is no next element on the y for v3
                break;
            }

            // Retain new v3 inside u
            u = v;

            // And start searching for the new v4
            while (j < plane->numVertices / 4 - 1 && firstOrder(v4) == firstOrder(v) - 1 && secondOrder(v4) > secondOrder(v) && v4->texCoords.z == v->texCoords.z) {
                // The area between the new v3 and new v4 needs to be filled with non-hashed faces
                // Otherwise, there's no new v4
                if (hashed[j] != 0 || secondOrder(v) != secondOrder(v + 4) - 1) {
                    break;
                }

                hashed[j++] = 1;
                v += 4;
            }

            if (hashed[j] != 0 || secondOrder(v4) != secondOrder(v) || firstOrder(v4) != firstOrder(v) - 1 || v4->texCoords.z != v->texCoords.z) {
                // Exit if there is no next element on the y for v4
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
        if (u != v3 && u != NULL && v != NULL) {
            size_t k;
            for (k = (u - plane->vertices) / 4; k <= (v - plane->vertices) / 4; k++) {
                hashed[k] = 0;
            }
        }

        // Copy the face to the plane
        memcpy(plane->vertices + newNumVertices, v1 + 0, sizeof(geVertex));
        newNumVertices++;
        memcpy(plane->vertices + newNumVertices, v2 + 1, sizeof(geVertex));
        newNumVertices++;
        memcpy(plane->vertices + newNumVertices, v4 + 2, sizeof(geVertex));
        newNumVertices++;
        memcpy(plane->vertices + newNumVertices, v3 + 3, sizeof(geVertex));
        newNumVertices++;

        memcpy(plane->indices + newNumIndices, indices, sizeof(GLuint) * 6);
        newNumIndices += 6;
    }

    // Reallocate to save on memory
    if (newNumVertices == 0 || realloc(plane->vertices, newNumVertices * sizeof(geVertex)) == NULL) {
        free(plane->vertices);
        plane->vertices = NULL;
        fprintf(stderr, "Failed to reallocate vertices, deallocating\n");
    }
    if (newNumIndices == 0 || realloc(plane->indices, newNumIndices * sizeof(GLuint)) == NULL) {
        free(plane->indices);
        plane->indices = NULL;
        fprintf(stderr, "Failed to reallocate indices, deallocating\n");
    }

    plane->numVertices = newNumVertices;
    plane->numIndices = newNumIndices;

    free(hashed);
}

void geWorldGenerateCulledPlanes(geWorld* world) {
    TIME_START;
    size_t oX, oY, oZ, k, l;
    size_t currentBlockIndex = 0;

    for (k = 0; k < 6; k++) {
        for (l = 0; l < world->numPlanes[k]; l++) {
            free(world->planesUncompressed[k][l].vertices);
            free(world->planesUncompressed[k][l].indices);
        }
        free(world->planesUncompressed[k]);
        world->numPlanes[k] = 0;
    }

    world->planesUncompressed[0] = calloc(world->sizeZ, sizeof(gePlane));
    world->planesUncompressed[1] = calloc(world->sizeZ, sizeof(gePlane));
    world->planesUncompressed[2] = calloc(world->sizeX, sizeof(gePlane));
    world->planesUncompressed[3] = calloc(world->sizeX, sizeof(gePlane));
    world->planesUncompressed[4] = calloc(world->sizeY, sizeof(gePlane));
    world->planesUncompressed[5] = calloc(world->sizeY, sizeof(gePlane));

    for (oX = 0; oX < world->sizeX; oX++) {
        for (oZ = 0; oZ < world->sizeZ; oZ++) {
            for (oY = 0; oY < world->sizeY; oY++) {
                if (world->map[oX][oZ][oY] == 0) {
                    continue;
                }

                bool isAdjacent[] = {
                        oZ < world->sizeZ - 1 && world->map[oX][oZ + 1][oY] != 0,
                        oZ != 0 && world->map[oX][oZ - 1][oY] != 0,

                        oX != 0 && world->map[oX - 1][oZ][oY] != 0,
                        oX < world->sizeX - 1 && world->map[oX + 1][oZ][oY] != 0,

                        oY < world->sizeY - 1 && world->map[oX][oZ][oY + 1] != 0,
                        oY != 0 && world->map[oX][oZ][oY - 1] != 0,
                };

                for (k = 0; k < 6; k++) {
                    if (!isAdjacent[k]) {
                        // Remove indices based on face of the cube in favour of square based indices
                        size_t p;
                        for (p = 0; p < 6; p++) {
                            world->shape.indices[(currentBlockIndex * PER_CUBE_INDICES + k * 6) + p] -= k * 4;
                        }

                        // Add the quad to an ordered plane
                        addFaceInOrderedPlane(world->shape.vertices + (currentBlockIndex * PER_CUBE_VERTICES + k * 4),
                                              world->shape.indices + (currentBlockIndex * PER_CUBE_INDICES + k * 6),
                                              world->planesUncompressed[k], world->numPlanes + k);
                    }
                }
                currentBlockIndex++;
            }
        }
    }

//    for (k = 0; k < 6; k++) {
//        if (realloc(planes[k], numPlanes[k] * sizeof(gePlane)) != NULL) {
//            fprintf(stdout, "Reallocating space of plane %llu to %llu\n", k, numPlanes[k]);
//        }
//    }
    printf("---------------- CULLED MESH ----------------------\n");
    TIME_END("culling and sorting voxel world");
}

void geWorldCompressCulledPlanesWithGreedy(geWorld* world) {
    TIME_START;
    size_t k, l;
    for (k = 0; k < 6; k++) {
        for (l = 0; l < world->numPlanes[k]; l++) {
#ifdef DEBUG_GREEDY
            size_t numIndicesBefore = plane->numIndices, numVerticesBefore = plane->numVertices;
#endif
            gePlaneCompressWithGreedy(&world->planes[k][l]);
#ifdef DEBUG_GREEDY
            fprintf(stdout, "%llu) Before %llu, %llu and after %llu, %llu\n", k, numVerticesBefore, numIndicesBefore, plane->numVertices, plane->numIndices);
#endif
        }
    }
    printf("---------------- GREEDY MESH ----------------------\n");
    TIME_END("greedy");
}

void geWorldShapeFromPlanes(geWorld* world) {
    TIME_START;
    size_t k, l, j, indexOffset = 0;
    for (k = 0; k < 6; k++) {
        for (l = 0; l < world->numPlanes[k]; l++) {
            gePlane* plane = &world->planes[k][l];
            for (j = 0; j < plane->numVertices / 4; j++) {
                plane->indices[j * 6] += indexOffset;
                plane->indices[j * 6 + 1] += indexOffset;
                plane->indices[j * 6 + 2] += indexOffset;
                plane->indices[j * 6 + 3] += indexOffset;
                plane->indices[j * 6 + 4] += indexOffset;
                plane->indices[j * 6 + 5] += indexOffset;
                indexOffset += 4;
            }

            memcpy(world->shape.vertices + indexOffset - plane->numVertices, plane->vertices, plane->numVertices * sizeof(geVertex));
            memcpy(world->shape.indices + indexOffset * 3 / 2 - plane->numIndices, plane->indices, plane->numIndices * sizeof(GLuint));
        }
    }

    world->shape.numVertices = indexOffset;
    world->shape.numIndices = indexOffset * 3 / 2;

//    realloc(world->shape.vertices, world->shape.numVertices * sizeof(geVertex));
//    realloc(world->shape.indices, world->shape.numIndices * sizeof(GLuint));
    TIME_END("converting to shape");
    printf("Total vertices and indices: %llu, %llu\n\n", world->shape.numVertices, world->shape.numIndices);
}

/* EXTERNAL FUNCTIONS */
geWorld geWorldInit(ALGORITHM algorithm, size_t sizeX, size_t sizeY, size_t sizeZ) {
    geWorld world = { 0 };

    world.sizeX = sizeX;
    world.sizeY = sizeY;
    world.sizeZ = sizeZ;
    world.algorithm = algorithm;
    world.numBlocks = 0;

    size_t oX, oZ;
    world.map = calloc(sizeX, sizeof(long long*));
    for (oX = 0; oX < sizeX; oX++) {
        world.map[oX] = calloc(sizeZ, sizeof(long long*));
        for (oZ = 0; oZ < sizeZ; oZ++) {
            world.map[oX][oZ] = calloc(sizeY, sizeof(long long));
        }
    }

    geWorldGenerate(&world, world.sizeY / 2, world.sizeY / 2);
    return world;
}

void geWorldDestroy(geWorld* world) {
    size_t oX, oZ, k, l;
    for (oX = 0; oX < world->sizeX; oX++) {
        for (oZ = 0; oZ < world->sizeZ; oZ++) {
            free(world->map[oX][oZ]);
        }
        free(world->map[oX]);
    }
    free(world->map);

    for (k = 0; k < 6; k++) {
        for (l = 0; l < world->numPlanes[k]; l++) {
            free(world->planes[k][l].vertices);
            free(world->planes[k][l].indices);
            free(world->planesUncompressed[k][l].vertices);
            free(world->planesUncompressed[k][l].indices);
        }
        free(world->planes[k]);
        free(world->planesUncompressed[k]);
        world->numPlanes[k] = 0;
    }
}

void geWorldGenerate(geWorld* world, size_t baseHeight, size_t heightOffsetIntesnsity) {
    size_t oX, oZ, oY;

    for (oX = 0; oX < world->sizeX; oX++) {
        for (oZ = 0; oZ < world->sizeZ; oZ++) {
            int noise = (int)(((1 + sdnoise2(((float) oX) / 32.0f, ((float) oZ) / 32.0f, NULL, NULL)) / 2.0f) * heightOffsetIntesnsity + baseHeight);
            long long up = (long long)((sdnoise2(oX, oZ, NULL, NULL) + 1) * 2.5f) + (long long)(world->sizeY * 0.5f);

            for (oY = 0; oY < noise; oY++) {
                if (up-- < 0) {
                    world->map[oX][oZ][oY] = 3;
                } else {
                    world->map[oX][oZ][oY] = 2;
                }
            }
            world->numBlocks += (size_t) (noise);
        }
    }

    geWorldGenerateShape(world, world->algorithm == GE_ALGORITHM_BASIC);
    if (world->algorithm == GE_ALGORITHM_CULLED) {
        geWorldGenerateCulledPlanes(world);
        geWorldCopyPlanes(world);
        geWorldShapeFromPlanes(world);
    } else if (world->algorithm == GE_ALGORITHM_GREEDY) {
        geWorldGenerateCulledPlanes(world);
        geWorldCopyPlanes(world);
        geWorldCompressCulledPlanesWithGreedy(world);
        geWorldShapeFromPlanes(world);
    }
}

kmVec3 geWorldFind(geWorld* world, kmVec3* v) {
    kmVec3 result;
    size_t x, y, z;
    x = (size_t) floorf(v->x + 0.5f);
    y = (size_t) floorf(v->y + 0.5f);
    z = (size_t) floorf(v->z + 0.5f);
    if (0 <= x && x < world->sizeX && 0 <= y && y < world->sizeY && 0 <= z && z < world->sizeZ && world->map[x][z][y] != 0) {
        result.x = x;
        result.y = y;
        result.z = z;
    } else {
        result.x = result.y = result.z = -1;
    }
    return result;
}

void geWorldCopyPlanes(geWorld* world) {
    size_t k, l;
    for (k = 0; k < 6; k++) {
        if (world->planes[k] == NULL) {
            continue;
        }
        for (l = 0; l < world->numPlanes[k]; l++) {
            if (world->planes[k]->vertices != NULL) {
                free(world->planes[k][l].vertices);
                world->planes[k][l].vertices = NULL;
            }
            if (world->planes[k]->indices != NULL) {
                free(world->planes[k][l].indices);
                world->planes[k][l].indices = NULL;
            }
        }
        free(world->planes[k]);
        world->planes[k] = NULL;
    }

    world->planes[0] = calloc(world->sizeZ, sizeof(gePlane));
    world->planes[1] = calloc(world->sizeZ, sizeof(gePlane));
    world->planes[2] = calloc(world->sizeX, sizeof(gePlane));
    world->planes[3] = calloc(world->sizeX, sizeof(gePlane));
    world->planes[4] = calloc(world->sizeY, sizeof(gePlane));
    world->planes[5] = calloc(world->sizeY, sizeof(gePlane));

    for (k = 0; k < 6; k++) {
        for (l = 0; l < world->numPlanes[k]; l++) {
            gePlane* planeCompressed = &world->planes[k][l];
            gePlane* planeUncompressed = &world->planesUncompressed[k][l];

            planeCompressed->numVertices = planeUncompressed->numVertices;
            planeCompressed->numIndices = planeUncompressed->numIndices;
            planeCompressed->vertices = calloc(8192 * 4, sizeof(geVertex));
            planeCompressed->indices = calloc(8192 * 6, sizeof(GLuint));
            memcpy(planeCompressed->vertices, planeUncompressed->vertices, planeCompressed->numVertices * sizeof(geVertex));
            memcpy(planeCompressed->indices, planeUncompressed->indices, planeCompressed->numIndices * sizeof(GLuint));
        }
    }
}

gePlane* geWorldGetPlane(geWorld* world, size_t side, size_t pos) {
    size_t l;
    gePlane* plane;

    testPlanesIntegrity(world);

    for (l = 0; l < world->numPlanes[side]; l++) {
        plane = &world->planesUncompressed[side][l];
        size_t planePos = (size_t) floorf(planeCoordinate(plane->vertices + 0) + 0.5f);
        if (planePos == pos) {
            return plane;
        }
    }
    return NULL;
}

void geWorldRemoveBlockFromUncompressedPlanes(geWorld* world, size_t x, size_t y, size_t z) {
    size_t k, l, side;
    geVertex* v;

    gePlane* current[6]; // b f l r t d

    current[0] = geWorldGetPlane(world, 0, z + 1);
    current[1] = geWorldGetPlane(world, 1, z);
    current[2] = geWorldGetPlane(world, 2, x);
    current[3] = geWorldGetPlane(world, 3, x + 1);
    current[4] = geWorldGetPlane(world, 4, y + 1);
    current[5] = geWorldGetPlane(world, 5, y);

    float blockType[] = {
            z < world->sizeZ - 1 ? world->map[x][z + 1][y] : 0,
            z != 0 ? world->map[x][z - 1][y] : 0,

            x != 0 ? world->map[x - 1][z][y] : 0,
            x < world->sizeX - 1 ? world->map[x + 1][z][y] : 0,

            y < world->sizeY - 1 ? world->map[x][z][y + 1] : 0,
            y != 0 ? world->map[x][z][y - 1] : 0,
    };

    // REMOVE ALL FACES
    // If not adjacent on a face's direction, remove current face
    for (side = 0; side < 6; side++) {
        if (blockType[side] != 0) {
            continue;
        }
        for (k = 0; k < current[side]->numVertices / 4; k++) {
            v = current[side]->vertices + k * 4 + 2;
            if ((side == 0 || side == 1) && ((size_t) floorf(v->pos.x)) == x && ((size_t) floorf(v->pos.y)) == y ||
                (side == 2 || side == 3) && ((size_t) floorf(v->pos.y)) == y && ((size_t) floorf(v->pos.z)) == z ||
                (side == 4 || side == 5) && ((size_t) floorf(v->pos.x)) == x && ((size_t) floorf(v->pos.z)) == z) {
                removeFace(current[side], k * 4, k * 6, 1);
                break;
            }
        }
    }

    for (side = 0; side < 6; side++) {
        if (blockType[side] != 0) {
            float textureIndex = blockType[side] - 1;
            float fx = (float) x;
            float fy = (float) y;
            float fz = (float) z;
            if (side == 0) {
                geVertex toAddV[] = {
                        {{fx - size / 2, fy - size / 2, fz + size / 2}, {0.0f, 0.0f, -1.0f}, {1, 0, textureIndex}},
                        {{fx + size / 2, fy - size / 2, fz + size / 2}, {0.0f, 0.0f, -1.0f}, {0, 0, textureIndex}},
                        {{fx + size / 2, fy + size / 2, fz + size / 2}, {0.0f, 0.0f, -1.0f}, {0, 1, textureIndex}},
                        {{fx - size / 2, fy + size / 2, fz + size / 2}, {0.0f, 0.0f, -1.0f}, {1, 1, textureIndex}},
                };
                GLuint toAddI[] = {
                        0, 3, 2,
                        2, 1, 0
                };
                addFaceInOrderedPlane(toAddV, toAddI, world->planesUncompressed[1], &world->numPlanes[1]);
            } else if (side == 1) {
                geVertex toAddV[] = {
                        {{fx - size / 2, fy - size / 2, fz - size / 2}, {0.0f, 0.0f, 1.0f}, {1, 0, textureIndex}},
                        {{fx + size / 2, fy - size / 2, fz - size / 2}, {0.0f, 0.0f, 1.0f}, {0, 0, textureIndex}},
                        {{fx + size / 2, fy + size / 2, fz - size / 2}, {0.0f, 0.0f, 1.0f}, {0, 1, textureIndex}},
                        {{fx - size / 2, fy + size / 2, fz - size / 2}, {0.0f, 0.0f, 1.0f}, {1, 1, textureIndex}},
                };
                GLuint toAddI[] = {
                        0, 1, 2,
                        2, 3, 0
                };
                addFaceInOrderedPlane(toAddV, toAddI, world->planesUncompressed[0], &world->numPlanes[0]);
            } else if (side == 2) {
                geVertex toAddV[] = {
                        {{ fx - size / 2, fy - size / 2, fz - size / 2 }, { 1.0f, 0.0f, 0.0f }, {0, 0, textureIndex}},
                        {{ fx - size / 2, fy - size / 2, fz + size / 2 }, { 1.0f, 0.0f, 0.0f }, {1, 0, textureIndex}},
                        {{ fx - size / 2, fy + size / 2, fz + size / 2 }, { 1.0f, 0.0f, 0.0f }, {1, 1, textureIndex}},
                        {{ fx - size / 2, fy + size / 2, fz - size / 2 }, { 1.0f, 0.0f, 0.0f }, {0, 1, textureIndex}},
                };
                GLuint toAddI[] = {
                        0, 3, 2,
                        2, 1, 0
                };
                addFaceInOrderedPlane(toAddV, toAddI, world->planesUncompressed[3], &world->numPlanes[3]);
            } else if (side == 3) {
                geVertex toAddV[] = {
                        {{ fx + size / 2, fy - size / 2, fz - size / 2 }, { -1.0f, 0.0f, 0.0f }, {0, 0, textureIndex}},
                        {{ fx + size / 2, fy - size / 2, fz + size / 2 }, { -1.0f, 0.0f, 0.0f }, {1, 0, textureIndex}},
                        {{ fx + size / 2, fy + size / 2, fz + size / 2 }, { -1.0f, 0.0f, 0.0f }, {1, 1, textureIndex}},
                        {{ fx + size / 2, fy + size / 2, fz - size / 2 }, { -1.0f, 0.0f, 0.0f }, {0, 1, textureIndex}},
                };
                GLuint toAddI[] = {
                        0, 1, 2,
                        2, 3, 0
                };
                addFaceInOrderedPlane(toAddV, toAddI, world->planesUncompressed[2], &world->numPlanes[2]);
            } else if(side == 4) {
                geVertex toAddV[] = {
                        {{ fx - size / 2, fy + size / 2, fz - size / 2 }, { 0.0f, -1.0f, 0.0f }, {0, 0, textureIndex}},
                        {{ fx + size / 2, fy + size / 2, fz - size / 2 }, { 0.0f, -1.0f, 0.0f }, {1, 0, textureIndex}},
                        {{ fx + size / 2, fy + size / 2, fz + size / 2 }, { 0.0f, -1.0f, 0.0f }, {1, 1, textureIndex}},
                        {{ fx - size / 2, fy + size / 2, fz + size / 2 }, { 0.0f, -1.0f, 0.0f }, {0, 1, textureIndex}},
                };
                GLuint toAddI[] = {
                        0, 1, 2,
                        2, 3, 0
                };
                addFaceInOrderedPlane(toAddV, toAddI, world->planesUncompressed[5], &world->numPlanes[5]);
            } else if(side == 5) {
                geVertex toAddV[] = {
                        {{ fx - size / 2, fy - size / 2, fz - size / 2 }, { 0.0f, 1.0f, 0.0f }, {0, 0, textureIndex}},
                        {{ fx + size / 2, fy - size / 2, fz - size / 2 }, { 0.0f, 1.0f, 0.0f }, {1, 0, textureIndex}},
                        {{ fx + size / 2, fy - size / 2, fz + size / 2 }, { 0.0f, 1.0f, 0.0f }, {1, 1, textureIndex}},
                        {{ fx - size / 2, fy - size / 2, fz + size / 2 }, { 0.0f, 1.0f, 0.0f }, {0, 1, textureIndex}},
                };
                GLuint toAddI[] = {
                        0, 3, 2,
                        2, 1, 0
                };
                addFaceInOrderedPlane(toAddV, toAddI, world->planesUncompressed[4], &world->numPlanes[4]);
            }
        }
    }
}

void geWorldRemoveBlock(geWorld* world, kmVec3* v) {
    size_t x, y, z;
    x = (size_t) v->x;
    y = (size_t) v->y;
    z = (size_t) v->z;

    world->map[x][z][y] = 0;

    world->numBlocks--;

    if (world->algorithm == GE_ALGORITHM_GREEDY) {
        geWorldRemoveBlockFromUncompressedPlanes(world, x, y, z);
        geWorldCopyPlanes(world);
        geWorldCompressCulledPlanesWithGreedy(world);
        geWorldShapeFromPlanes(world);
        return;
    }

    geWorldGenerateShape(world, world->algorithm == GE_ALGORITHM_BASIC);
    if (world->algorithm == GE_ALGORITHM_CULLED) {
        geWorldGenerateCulledPlanes(world);
        geWorldShapeFromPlanes(world);
    } else if (world->algorithm == GE_ALGORITHM_GREEDY) {
//        geWorldGenerateCulledPlanes(world);
//        geWorldCompressCulledPlanesWithGreedy(world);
//        geWorldShapeFromPlanes(world);
    }
}