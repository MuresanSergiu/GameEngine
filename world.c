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

// Adds a face (4x geVertex) inside a sorted plane
gePlane* addFaceInOrderedPlane(geVertex* vertices, gePlane** planesPointer, size_t* numPlanes) {
    size_t i;
    gePlane* planes = *planesPointer;

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
        (*numPlanes)++;
        *planesPointer = realloc(*planesPointer, (*numPlanes) * sizeof(gePlane));
        destination = (*planesPointer) + (*numPlanes) - 1;
        destination->vertices = calloc(8192 * 40, sizeof(geVertex));
        destination->numVertices = 0;
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
    memcpy(destination->vertices + i * 4, vertices, sizeof(geVertex) * 4);
    destination->numVertices += 4;

    return destination;
}

void geWorldGenerateShape(geWorld* world, bool withFullIndices) {
    TIME_START;
    size_t k;
    size_t oX, oY, oZ;
    size_t currentBlockIndex = 0;

    float s0 = size / 2;

    geVertex vertices[] = {
            // FRONT
            {{ x - s0, y - s0, z + s0 }, { 0.0f, 0.0f,  1.0f }, {0, 0}},
            {{ x + s0, y - s0, z + s0 }, { 0.0f, 0.0f,  1.0f }, {1, 0}},
            {{ x + s0, y + s0, z + s0 }, { 0.0f, 0.0f,  1.0f }, {1, 1}},
            {{ x - s0, y + s0, z + s0 }, { 0.0f, 0.0f,  1.0f }, {0, 1}},
            // BACK
            {{ x - s0, y - s0, z - s0 }, { 0.0f, 0.0f, -1.0f }, {1, 0}},
            {{ x + s0, y - s0, z - s0 }, { 0.0f, 0.0f, -1.0f }, {0, 0}},
            {{ x + s0, y + s0, z - s0 }, { 0.0f, 0.0f, -1.0f }, {0, 1}},
            {{ x - s0, y + s0, z - s0 }, { 0.0f, 0.0f, -1.0f }, {1, 1}},
            // LEFT
            {{ x - s0, y - s0, z - s0 }, { -1.0f, 0.0f, 0.0f }, {0, 0}},
            {{ x - s0, y - s0, z + s0 }, { -1.0f, 0.0f, 0.0f }, {1, 0}},
            {{ x - s0, y + s0, z + s0 }, { -1.0f, 0.0f, 0.0f }, {1, 1}},
            {{ x - s0, y + s0, z - s0 }, { -1.0f, 0.0f, 0.0f }, {0, 1}},
            // RIGHT
            {{ x + s0, y - s0, z - s0 }, { 1.0f, 0.0f, 0.0f }, {1, 0}},
            {{ x + s0, y - s0, z + s0 }, { 1.0f, 0.0f, 0.0f }, {0, 0}},
            {{ x + s0, y + s0, z + s0 }, { 1.0f, 0.0f, 0.0f }, {0, 1}},
            {{ x + s0, y + s0, z - s0 }, { 1.0f, 0.0f, 0.0f }, {1, 1}},
            // TOP
            {{ x - s0, y + s0, z - s0 }, { 0.0f, 1.0f, 0.0f }, {0, 0}},
            {{ x + s0, y + s0, z - s0 }, { 0.0f, 1.0f, 0.0f }, {1, 0}},
            {{ x + s0, y + s0, z + s0 }, { 0.0f, 1.0f, 0.0f }, {1, 1}},
            {{ x - s0, y + s0, z + s0 }, { 0.0f, 1.0f, 0.0f }, {0, 1}},
            // BOTTOM
            {{ x - s0, y - s0, z - s0 }, { 0.0f, -1.0f, 0.0f }, {0, 0}},
            {{ x + s0, y - s0, z - s0 }, { 0.0f, -1.0f, 0.0f }, {1, 0}},
            {{ x + s0, y - s0, z + s0 }, { 0.0f, -1.0f, 0.0f }, {1, 1}},
            {{ x - s0, y - s0, z + s0 }, { 0.0f, -1.0f, 0.0f }, {0, 1}},
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
    printf("Total number of blocks: %llu\n", world->numBlocks);
    printf("World size: (%llu, %llu, %llu)\n", world->sizeX, world->sizeY, world->sizeZ);
    if (withFullIndices) {
        printf("\n");
    }
}

void gePlaneCompressWithGreedy(gePlane* plane) {
    if (plane->numVertices == 0) {
        return;
    }

    size_t i;
    size_t newNumVertices = 0;

    // Take texture coordinate diffs to expand based on the first vertex' coords
    kmVec3 diffFirstOrder, diffSecondOrder, temp;
    kmVec3Subtract(&diffFirstOrder, &plane->vertices[3].texCoords, &plane->vertices[0].texCoords);
    kmVec3Subtract(&diffSecondOrder, &plane->vertices[1].texCoords, &plane->vertices[0].texCoords);

    // Where we mark which faces have already been added to the new plane
    size_t* hashed = calloc(plane->numVertices / 4, sizeof(size_t));
    for (i = 0; i < plane->numVertices / 4; i++) {
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
    }

    // Reallocate to save on memory
    if (newNumVertices == 0) {
        free(plane->vertices);
        plane->vertices = NULL;
        fprintf(stderr, "Failed to reallocate vertices, deallocating\n");
    } else {
        plane->vertices = realloc(plane->vertices, newNumVertices * sizeof(geVertex));
    }

    plane->numVertices = newNumVertices;

    free(hashed);
}

void geWorldGenerateCulledPlanes(geWorld* world) {
    TIME_START;
    size_t oX, oY, oZ, k, l;
    size_t currentBlockIndex = 0;

    for (k = 0; k < 6; k++) {
        for (l = 0; l < world->numPlanes[k]; l++) {
            free(world->planesUncompressed[k][l].vertices);
            world->planesUncompressed[k][l].vertices = NULL;
        }
        free(world->planesUncompressed[k]);
        world->planesUncompressed[k] = NULL;
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
                                              &world->planesUncompressed[k], world->numPlanes + k);
                    }
                }
                currentBlockIndex++;
            }
        }
    }

    for (k = 0; k < 6; k++) {
        world->planesUncompressed[k] = realloc(world->planesUncompressed[k], world->numPlanes[k] * sizeof(gePlane));
        if (world->planesUncompressed[k] == NULL) {
            fprintf(stderr, "Failed to reallocate space of plane %llu to %llu\n", k, world->numPlanes[k]);
        } else {
            fprintf(stdout, "Reallocating space of plane %llu to %llu\n", k, world->numPlanes[k]);
        }
    }
    printf("---------------- CULLED MESH ----------------------\n");
    TIME_END("culling and sorting voxel world");
    size_t numVertices = 0, numPlanes = 0;
    for (k = 0; k < 6; k++) {
        for (l = 0; l < world->numPlanes[k]; l++) {
            numVertices += world->planesUncompressed[k][l].numVertices;
        }
        numPlanes += world->numPlanes[k];
    }
    printf("Total number of vertices: %llu\n", numVertices);
    printf("Total number of planes: %llu\n", numPlanes);
}

void geWorldCompressCulledPlanesWithGreedy(geWorld* world) {
    TIME_START;
    size_t k, l;
    for (k = 0; k < 6; k++) {
        for (l = 0; l < world->numPlanes[k]; l++) {
#ifdef DEBUG_GREEDY
            size_t numIndicesBefore = plane->numIndices, numVerticesBefore = plane->numVertices;
#endif
            if (world->planes[k] != NULL) {
                gePlaneCompressWithGreedy(&world->planes[k][l]);
            }
#ifdef DEBUG_GREEDY
            fprintf(stdout, "%llu) Before %llu, %llu and after %llu, %llu\n", k, numVerticesBefore, numIndicesBefore, plane->numVertices, plane->numIndices);
#endif
        }
    }
    printf("---------------- GREEDY MESH ----------------------\n");
    TIME_END("greedy");
    size_t numVertices = 0, numPlanes = 0;
    for (k = 0; k < 6; k++) {
        for (l = 0; l < world->numPlanes[k]; l++) {
            numVertices += world->planes[k][l].numVertices;
        }
        numPlanes += world->numPlanes[k];
    }
    printf("Total number of vertices: %llu\n", numVertices);
    printf("Total number of planes: %llu\n", numPlanes);
}

void geWorldShapeFromPlanes(geWorld* world) {
    TIME_START;
    size_t k, l, j, indexOffset = 0;
    GLuint indices[6];

    world->shape.numVertices = 0;
    for (k = 0; k < 6; k++) {
        for (l = 0; l < world->numPlanes[k]; l++) {
            world->shape.numVertices += world->planes[k][l].numVertices;
        }
    }
    world->shape.numIndices = world->shape.numVertices * 3 / 2;

    world->shape.vertices = realloc(world->shape.vertices, world->shape.numVertices * sizeof(geVertex));
    world->shape.indices = realloc(world->shape.indices, world->shape.numIndices * sizeof(GLuint));

    for (k = 0; k < 6; k++) {
        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;
        indices[3] = 2;
        indices[4] = 3;
        indices[5] = 0;
        if (k == 1 || k == 3 || k == 4) {
            indices[1] = 3;
            indices[4] = 1;
        }
        for (l = 0; l < world->numPlanes[k]; l++) {
            if (world->planes[k] == NULL) {
                continue;
            }
            gePlane* plane = &world->planes[k][l];
//            for (j = 0; j < plane->numVertices / 4; j++) {
//                plane->indices[j * 6] += indexOffset;
//                plane->indices[j * 6 + 1] += indexOffset;
//                plane->indices[j * 6 + 2] += indexOffset;
//                plane->indices[j * 6 + 3] += indexOffset;
//                plane->indices[j * 6 + 4] += indexOffset;
//                plane->indices[j * 6 + 5] += indexOffset;
//                indexOffset += 4;
//            }

            memcpy(world->shape.vertices + indexOffset * 4, plane->vertices, plane->numVertices * sizeof(geVertex));
            //memcpy(world->shape.indices + indexOffset * 6, plane->indices, plane->numIndices * sizeof(GLuint));
            for (j = 0; j < plane->numVertices * 3 / 2; j++) {
                world->shape.indices[indexOffset * 6 + j] = indices[j % 6];
            }

            indexOffset += (plane->numVertices / 4);
        }
    }

    indexOffset = 0;
    for (l = 0; l < world->shape.numIndices / 6; l++) {
        world->shape.indices[l * 6] += indexOffset;
        world->shape.indices[l * 6 + 1] += indexOffset;
        world->shape.indices[l * 6 + 2] += indexOffset;
        world->shape.indices[l * 6 + 3] += indexOffset;
        world->shape.indices[l * 6 + 4] += indexOffset;
        world->shape.indices[l * 6 + 5] += indexOffset;
        indexOffset += 4;
    }

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

    geWorldGenerate(&world, 2 * world.sizeY / 3, world.sizeY / 3);

//    size_t i;
//    for (i = 0; i < world.numPlanes[4]; i++) {
//        printf("Number of faces on culled top plane with coordinate y = %f is %llu\n", world.planesUncompressed[4][i].vertices[0].pos.y, world.planesUncompressed[4][i].numVertices / 4);
//    }
//    for (i = 0; i < world.numPlanes[4]; i++) {
//        printf("Number of faces on greedied top plane with coordinate y = %f is %llu\n", world.planes[4][i].vertices[0].pos.y, world.planes[4][i].numVertices / 4);
//    }
//    for (i = 0; i < world.numPlanes[0]; i++) {
//        printf("Number of faces on culled front plane with coordinate z = %f is %llu\n", world.planesUncompressed[0][i].vertices[0].pos.z, world.planesUncompressed[0][i].numVertices / 4);
//    }
//    for (i = 0; i < world.numPlanes[0]; i++) {
//        printf("Number of faces on greedied front plane with coordinate z = %f is %llu\n", world.planes[0][i].vertices[0].pos.z, world.planes[0][i].numVertices / 4);
//    }

    return world;
}

geWorld geWorldInitCustom(ALGORITHM algorithm, uint8_t orientation) {
    geWorld world = { 0 };

    world.algorithm = algorithm;

    // Custom L-shape
    world.sizeX = 4;
    world.sizeY = 4;
    world.sizeZ = 1;

    size_t oX, oZ;
    world.map = calloc(world.sizeX, sizeof(long long*));
    for (oX = 0; oX < world.sizeX; oX++) {
        world.map[oX] = calloc(world.sizeZ, sizeof(long long*));
        for (oZ = 0; oZ < world.sizeZ; oZ++) {
            world.map[oX][oZ] = calloc(world.sizeY, sizeof(long long));
        }
    }

    world.map[0][0][0] = 2;
    world.map[1][0][0] = 2;
    world.map[2][0][0] = 2;
    world.map[2][0][1] = 2;
    world.map[0][0][1] = 2;
    world.map[1][0][1] = 2;

    world.map[2][0][1] = 2;
    world.map[2][0][2] = 2;
    world.map[3][0][0] = 2;
    world.map[3][0][1] = 2;
    world.map[3][0][2] = 2;
    world.numBlocks = 11;

    geWorldGenerateShape(&world, world.algorithm == GE_ALGORITHM_BASIC);
    if (world.algorithm == GE_ALGORITHM_CULLED) {
        geWorldGenerateCulledPlanes(&world);
        if (orientation < 6) {
            geWorldCopyPlanesWithOrientation(&world, orientation);
        } else {
            geWorldCopyPlanes(&world);
        }
        geWorldShapeFromPlanes(&world);
    } else if (world.algorithm == GE_ALGORITHM_GREEDY) {
        geWorldGenerateCulledPlanes(&world);
        if (orientation < 6) {
            geWorldCopyPlanesWithOrientation(&world, orientation);
        } else {
            geWorldCopyPlanes(&world);
        }
        geWorldCompressCulledPlanesWithGreedy(&world);
        geWorldShapeFromPlanes(&world);
    }
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
            free(world->planesUncompressed[k][l].vertices);
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
            long long up = (long long)((sdnoise2(((float) oX) / 8.0f, ((float) oZ) / 8.0f, NULL, NULL) + 1) * 2.5f) + (long long)(world->sizeY * 0.5f);

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
            planeCompressed->vertices = calloc(planeCompressed->numVertices, sizeof(geVertex));
            memcpy(planeCompressed->vertices, planeUncompressed->vertices, planeCompressed->numVertices * sizeof(geVertex));
        }
    }
}

void geWorldCopyPlanesWithOrientation(geWorld* world, uint8_t orientation) {
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
        }
        free(world->planes[k]);
        world->planes[k] = NULL;
    }

    world->planes[orientation] = calloc(orientation == 0 || orientation == 1 ? world->sizeZ : (orientation == 2 || orientation == 3 ? world->sizeX : world->sizeY), sizeof(gePlane));


    for (l = 0; l < world->numPlanes[orientation]; l++) {
        gePlane* planeCompressed = &world->planes[orientation][l];
        gePlane* planeUncompressed = &world->planesUncompressed[orientation][l];

        planeCompressed->numVertices = planeUncompressed->numVertices;
        planeCompressed->vertices = calloc(8192 * 4, sizeof(geVertex));
        memcpy(planeCompressed->vertices, planeUncompressed->vertices, planeCompressed->numVertices * sizeof(geVertex));
    }
}

gePlane* geWorldGetPlane(geWorld* world, size_t side, size_t pos) {
    size_t l;
    gePlane* plane;

//    testPlanesIntegrity(world);

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
    float fx = (float) x;
    float fy = (float) y;
    float fz = (float) z;

    gePlane* current[6]; // b f l r t d
    gePlane* toUpdate[12] = { NULL };
    float toUpdatePlaneCoords[12];

    float blockType[] = {
            z < world->sizeZ - 1 ? world->map[x][z + 1][y] : 0,
            z != 0 ? world->map[x][z - 1][y] : 0,

            x != 0 ? world->map[x - 1][z][y] : 0,
            x < world->sizeX - 1 ? world->map[x + 1][z][y] : 0,

            y < world->sizeY - 1 ? world->map[x][z][y + 1] : 0,
            y != 0 ? world->map[x][z][y - 1] : 0,
    };

    // ADD ALL FACES
    // If adjacent on one side add the opposite side for the adjacent cube
    if (blockType[0] != 0) {
        float textureIndex = blockType[0] - 1;
        geVertex toAddV[] = {
                {{fx - size / 2, fy - size / 2, fz + size / 2}, {0.0f, 0.0f, -1.0f}, {1, 0, textureIndex}},
                {{fx + size / 2, fy - size / 2, fz + size / 2}, {0.0f, 0.0f, -1.0f}, {0, 0, textureIndex}},
                {{fx + size / 2, fy + size / 2, fz + size / 2}, {0.0f, 0.0f, -1.0f}, {0, 1, textureIndex}},
                {{fx - size / 2, fy + size / 2, fz + size / 2}, {0.0f, 0.0f, -1.0f}, {1, 1, textureIndex}},
        };
        toUpdate[7] = addFaceInOrderedPlane(toAddV, &world->planesUncompressed[1], &world->numPlanes[1]);
        toUpdatePlaneCoords[7] = planeCoordinate(toUpdate[7]->vertices);
    }
    if (blockType[1] != 0) {
        float textureIndex = blockType[1] - 1;
        geVertex toAddV[] = {
                {{fx - size / 2, fy - size / 2, fz - size / 2}, {0.0f, 0.0f, 1.0f}, {0, 0, textureIndex}},
                {{fx + size / 2, fy - size / 2, fz - size / 2}, {0.0f, 0.0f, 1.0f}, {1, 0, textureIndex}},
                {{fx + size / 2, fy + size / 2, fz - size / 2}, {0.0f, 0.0f, 1.0f}, {1, 1, textureIndex}},
                {{fx - size / 2, fy + size / 2, fz - size / 2}, {0.0f, 0.0f, 1.0f}, {0, 1, textureIndex}},
        };
        toUpdate[6] = addFaceInOrderedPlane(toAddV, &world->planesUncompressed[0], &world->numPlanes[0]);
        toUpdatePlaneCoords[6] = planeCoordinate(toUpdate[6]->vertices);
    }
    if (blockType[2] != 0) {
        float textureIndex = blockType[2] - 1;
        geVertex toAddV[] = {
                {{ fx - size / 2, fy - size / 2, fz - size / 2 }, { 1.0f, 0.0f, 0.0f }, {1, 0, textureIndex}},
                {{ fx - size / 2, fy - size / 2, fz + size / 2 }, { 1.0f, 0.0f, 0.0f }, {0, 0, textureIndex}},
                {{ fx - size / 2, fy + size / 2, fz + size / 2 }, { 1.0f, 0.0f, 0.0f }, {0, 1, textureIndex}},
                {{ fx - size / 2, fy + size / 2, fz - size / 2 }, { 1.0f, 0.0f, 0.0f }, {1, 1, textureIndex}},
        };
        toUpdate[9] = addFaceInOrderedPlane(toAddV, &world->planesUncompressed[3], &world->numPlanes[3]);
        toUpdatePlaneCoords[9] = planeCoordinate(toUpdate[9]->vertices);
    }
    if (blockType[3] != 0) {
        float textureIndex = blockType[3] - 1;
        geVertex toAddV[] = {
                {{ fx + size / 2, fy - size / 2, fz - size / 2 }, { -1.0f, 0.0f, 0.0f }, {0, 0, textureIndex}},
                {{ fx + size / 2, fy - size / 2, fz + size / 2 }, { -1.0f, 0.0f, 0.0f }, {1, 0, textureIndex}},
                {{ fx + size / 2, fy + size / 2, fz + size / 2 }, { -1.0f, 0.0f, 0.0f }, {1, 1, textureIndex}},
                {{ fx + size / 2, fy + size / 2, fz - size / 2 }, { -1.0f, 0.0f, 0.0f }, {0, 1, textureIndex}},
        };
        toUpdate[8] = addFaceInOrderedPlane(toAddV, &world->planesUncompressed[2], &world->numPlanes[2]);
        toUpdatePlaneCoords[8] = planeCoordinate(toUpdate[8]->vertices);
    }
    if(blockType[4] != 0) {
        float textureIndex = blockType[4] - 1;
        geVertex toAddV[] = {
                {{ fx - size / 2, fy + size / 2, fz - size / 2 }, { 0.0f, -1.0f, 0.0f }, {0, 0, textureIndex}},
                {{ fx + size / 2, fy + size / 2, fz - size / 2 }, { 0.0f, -1.0f, 0.0f }, {1, 0, textureIndex}},
                {{ fx + size / 2, fy + size / 2, fz + size / 2 }, { 0.0f, -1.0f, 0.0f }, {1, 1, textureIndex}},
                {{ fx - size / 2, fy + size / 2, fz + size / 2 }, { 0.0f, -1.0f, 0.0f }, {0, 1, textureIndex}},
        };
        toUpdate[11] = addFaceInOrderedPlane(toAddV, &world->planesUncompressed[5], &world->numPlanes[5]);
        toUpdatePlaneCoords[11] = planeCoordinate(toUpdate[11]->vertices);
    }
    if(blockType[5] != 0) {
        float textureIndex = blockType[5] - 1;
        geVertex toAddV[] = {
                {{ fx - size / 2, fy - size / 2, fz - size / 2 }, { 0.0f, 1.0f, 0.0f }, {0, 0, textureIndex}},
                {{ fx + size / 2, fy - size / 2, fz - size / 2 }, { 0.0f, 1.0f, 0.0f }, {1, 0, textureIndex}},
                {{ fx + size / 2, fy - size / 2, fz + size / 2 }, { 0.0f, 1.0f, 0.0f }, {1, 1, textureIndex}},
                {{ fx - size / 2, fy - size / 2, fz + size / 2 }, { 0.0f, 1.0f, 0.0f }, {0, 1, textureIndex}},
        };
        toUpdate[10] = addFaceInOrderedPlane(toAddV, &world->planesUncompressed[4], &world->numPlanes[4]);
        toUpdatePlaneCoords[10] = planeCoordinate(toUpdate[10]->vertices);
    }

    current[0] = geWorldGetPlane(world, 0, z + 1);
    current[1] = geWorldGetPlane(world, 1, z);
    current[2] = geWorldGetPlane(world, 2, x);
    current[3] = geWorldGetPlane(world, 3, x + 1);
    current[4] = geWorldGetPlane(world, 4, y + 1);
    current[5] = geWorldGetPlane(world, 5, y);

    // REMOVE ALL FACES
    // If not adjacent on a face's direction, remove current face
    for (side = 0; side < 6; side++) {
        if (blockType[side] != 0) {
            continue;
        }
        if (current[side] == NULL) {
            fprintf(stderr, "Failed to find plane on side %llu even though there should be one there\n", side);
            continue;
        }
        for (k = 0; k < current[side]->numVertices / 4; k++) {
            v = current[side]->vertices + k * 4 + 2;
            if ((side == 0 || side == 1) && ((size_t) floorf(v->pos.x)) == x && ((size_t) floorf(v->pos.y)) == y ||
                (side == 2 || side == 3) && ((size_t) floorf(v->pos.y)) == y && ((size_t) floorf(v->pos.z)) == z ||
                (side == 4 || side == 5) && ((size_t) floorf(v->pos.x)) == x && ((size_t) floorf(v->pos.z)) == z) {
                toUpdatePlaneCoords[side] = planeCoordinate(current[side]->vertices);
                toUpdate[side] = current[side];
                removeFace(current[side], k * 4, 1);
                break;
            }
        }
    }

    // UPDATE PLANES
    for (k = 0; k < 12; k++) {
        if (toUpdate[k] == NULL) {
            continue;
        }
        gePlane* p = NULL;
        if (k >= 6 && toUpdate[k]->numVertices / 4 == 1) {
            // New plane created therefore make enough memory for compressed planes as well
            world->planes[k % 6] = realloc(world->planes[k % 6], world->numPlanes[k % 6] * sizeof(gePlane));
            l = world->numPlanes[k % 6] - 1;
            p = &world->planes[k % 6][l];
            p->vertices = NULL;
        } else {
            for (l = 0; l < world->numPlanes[k % 6]; l++) {
                p = &world->planes[k % 6][l];
                if (p->numVertices != 0 && planeCoordinate(p->vertices) == toUpdatePlaneCoords[k]) {
                    break;
                }
            }
        }
        if (toUpdate[k]->numVertices == 0) {
            // k here is less than 6
            fprintf(stdout, "Got side %llu with no vertices, deleting plane\n", k);
            memcpy(&world->planesUncompressed[k][l], &world->planesUncompressed[k][l] + 1, (world->numPlanes[k] - l - 1) * sizeof(gePlane));
            memcpy(p, p + 1, (world->numPlanes[k] - l - 1) * sizeof(gePlane));
            world->numPlanes[k]--;

            // Fix plane pointers
            if (toUpdate[k + 6] >= &world->planesUncompressed[k % 6][l]) {
                toUpdate[k + 6]--;
            }
        } else {
            // If plane is found deallocate otherwise, consider it newly created
            free(p->vertices);

            p->vertices = calloc(toUpdate[k]->numVertices, sizeof(geVertex));
            p->numVertices = toUpdate[k]->numVertices;
            memcpy(p->vertices, toUpdate[k]->vertices, sizeof(geVertex) * toUpdate[k]->numVertices);

            if (world->algorithm == GE_ALGORITHM_GREEDY) {
                gePlaneCompressWithGreedy(p);
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

    if (world->algorithm == GE_ALGORITHM_BASIC) {
        geWorldGenerateShape(world, true);
    } else if (world->algorithm == GE_ALGORITHM_CULLED) {
        geWorldRemoveBlockFromUncompressedPlanes(world, x, y, z);
        geWorldCopyPlanes(world);
        geWorldShapeFromPlanes(world);
    } else if (world->algorithm == GE_ALGORITHM_GREEDY) {
        geWorldRemoveBlockFromUncompressedPlanes(world, x, y, z);
        geWorldShapeFromPlanes(world);
    }

}