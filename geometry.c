//
// Created by Afterwind on 9/25/2017.
//

#include <stdlib.h>
#include <mem.h>
#include <stdio.h>
#include <sys/time.h>
#include "geometry.h"
#include "utils.h"
#include "simplex_noise.h"

#define MAX_HEIGHT 128

float x = 0;
float y = 0;
float z = 0;
float size = 1.0f;

/* INTERNAL FUNCTIONS */
void shapeFromVerticesAndIndices(geShape* pOut, geVertex* vertices, unsigned long long numVertices, GLuint* indices, unsigned long long numIndices, bool withIndices) {
    memset(pOut, 0, sizeof(geShape));
    if (withIndices) {
        pOut->vertices = calloc(numVertices, sizeof(geVertex));
        memcpy(pOut->vertices, vertices, numVertices * sizeof(geVertex));
        pOut->indices = calloc(numIndices, sizeof(GLuint));
        memcpy(pOut->indices, indices, numIndices * sizeof(GLuint));
        pOut->numVertices = numVertices;
        pOut->numIndices = numIndices;
    } else {
        pOut->numVertices = numIndices;
        pOut->vertices = calloc(pOut->numVertices, sizeof(geVertex));
        for (int i = 0; i < pOut->numVertices; i++) {
            memcpy(pOut->vertices + i, vertices + indices[i], sizeof(geVertex));
        }
    }
}

void removeFace(geShape* shape, size_t offsetVertex, size_t offsetIndex, size_t numFaces) {
    size_t k;
    memcpy(
            shape->vertices + offsetVertex,
            shape->vertices + (offsetVertex + 4 * numFaces),
            sizeof(geVertex) * (shape->numVertices - (offsetVertex + 4 * numFaces))
    );
    memcpy(
            shape->indices + offsetIndex,
            shape->indices + (offsetIndex + 6 * numFaces),
            sizeof(GLuint) * (shape->numIndices - (offsetIndex + 6 * numFaces))
    );
    for (k = offsetIndex; k < shape->numIndices; k++) {
        shape->indices[k] -= (4 * numFaces);
    }
    shape->numVertices -= (4 * numFaces);
    shape->numIndices -= (6 * numFaces);
}

/* EXTERNAL FUNCTIONS */
void initShapes() {
    shapes[GE_CUBE] = createCube(false);
    shapes[GE_CUBE_INVERTED] = createCube(true);
    shapes[GE_SQUARE] = createSquare(true);
    shapes[GE_LINE] = createLine();
    shapes[GE_CIRCLE] = createCircle(25);
    shapes[GE_CYLINDER] = createCylinder(25);
    shapes[GE_TETRAHEDRON] = createTetrahedron();
    shapes[GE_TERRAIN_TRIG] = createTrigTerrain(200);
    shapes[GE_NORMALS] = createLineNormals(shapes + GE_TERRAIN_TRIG);
    shapes[GE_TERRAIN_NOISE] = createNoiseTerrain(40);
    printf(" \n----- DUMB INIT ----- \n");
    shapes[GE_VERTEX_WORLD_DUMB] = createVoxelWorldDumb(8, 3);
    printf("The dumb version draws: %llu vertices and %llu indices\n", shapes[GE_VERTEX_WORLD_DUMB].numVertices, shapes[GE_VERTEX_WORLD_DUMB].numIndices);
    printf(" \n----- CULLED INIT ----- \n");
    shapes[GE_VERTEX_WORLD_CULLED] = createVoxelWorldWithCulling(8, 3);
    printf("The culled version draws: %llu vertices and %llu indices\n", shapes[GE_VERTEX_WORLD_CULLED].numVertices, shapes[GE_VERTEX_WORLD_CULLED].numIndices);
    printf(" \n----- GREEDY INIT ----- \n");
    shapes[GE_VERTEX_WORLD_GREEDY] = createVoxelWorldWithGreedy(20, 5);
    printf("The greedy version draws: %llu vertices and %llu indices\n", shapes[GE_VERTEX_WORLD_GREEDY].numVertices, shapes[GE_VERTEX_WORLD_GREEDY].numIndices);
}

geShape createCube(bool inverted) {

    // NO NEED FOR DOUBLE CURLY WHEN INITING STRUCT
    geVertex vertices[] = {
            // FRONT
            {{ x - size / 2, y - size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {0.25f , 1 / 3.0f}},
            {{ x + size / 2, y - size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {0.5f, 1 / 3.0f }},
            {{ x + size / 2, y + size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {0.5f, 2 / 3.0f }},
            {{ x - size / 2, y + size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {0.25f, 2 / 3.0f }},
            // BACK
            {{ x - size / 2, y - size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {1, 1 / 3.0f}},
            {{ x - size / 2, y + size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {1, 2 / 3.0f}},
            {{ x + size / 2, y + size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {0.75f, 2 / 3.0f}},
            {{ x + size / 2, y - size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {0.75f, 1 / 3.0f}},
            // LEFT
            {{ x - size / 2, y - size / 2, z - size / 2 }, { -1.0f, 0.0f, 0.0f }, {0, 1 / 3.0f}},
            {{ x - size / 2, y - size / 2, z + size / 2 }, { -1.0f, 0.0f, 0.0f }, {0.25f, 1 / 3.0f}},
            {{ x - size / 2, y + size / 2, z + size / 2 }, { -1.0f, 0.0f, 0.0f }, {0.25f, 2 / 3.0f}},
            {{ x - size / 2, y + size / 2, z - size / 2 }, { -1.0f, 0.0f, 0.0f }, {0, 2 / 3.0f}},
            // RIGHT
            {{ x + size / 2, y - size / 2, z - size / 2 }, { 1.0f, 0.0f, 0.0f }, {0.75f, 1 / 3.0f}},
            {{ x + size / 2, y + size / 2, z - size / 2 }, { 1.0f, 0.0f, 0.0f }, {0.75f, 2 / 3.0f}},
            {{ x + size / 2, y + size / 2, z + size / 2 }, { 1.0f, 0.0f, 0.0f }, {0.5f, 2 / 3.0f}},
            {{ x + size / 2, y - size / 2, z + size / 2 }, { 1.0f, 0.0f, 0.0f }, {0.5f, 1 / 3.0f}},
            // TOP
            {{ x + size / 2, y + size / 2, z - size / 2 }, { 0.0f, 1.0f, 0.0f }, {0.5f, 1}},
            {{ x - size / 2, y + size / 2, z - size / 2 }, { 0.0f, 1.0f, 0.0f }, {0.25f, 1}},
            {{ x - size / 2, y + size / 2, z + size / 2 }, { 0.0f, 1.0f, 0.0f }, {0.25f, 2 / 3.0f}},
            {{ x + size / 2, y + size / 2, z + size / 2 }, { 0.0f, 1.0f, 0.0f }, {0.5f, 2 / 3.0f}},
            // BOTTOM
            {{ x - size / 2, y - size / 2, z - size / 2 }, { 0.0f, -1.0f, 0.0f }, {0.25f, 1 / 3.0f}},
            {{ x - size / 2, y - size / 2, z + size / 2 }, { 0.0f, -1.0f, 0.0f }, {0.25f, 0}},
            {{ x + size / 2, y - size / 2, z + size / 2 }, { 0.0f, -1.0f, 0.0f }, {0.5f, 0}},
            {{ x + size / 2, y - size / 2, z - size / 2 }, { 0.0f, -1.0f, 0.0f }, {0.5f, 1 / 3.0f}},
    };

    geShape shape;
    if (inverted) {
        GLuint indices[] = {
                // FRONT
                0, 3, 2,
                2, 1, 0,
                // BACK
                4, 7, 6,
                6, 5, 4,
                // LEFT
                8, 11 ,10,
                10, 9 ,8,
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
        shapeFromVerticesAndIndices(&shape, vertices, sizeof(vertices) / sizeof(*vertices), indices, sizeof(indices) / sizeof(*indices), true);
    } else {
        GLuint indices[] = {
                // FRONT
                0, 1, 2,
                2, 3, 0,
                // BACK
                4, 5, 6,
                6, 7, 4,
                // LEFT
                8, 9, 10,
                10, 11, 8,
                // RIGHT
                12, 13, 14,
                14, 15, 12,
                // TOP
                16, 17, 18,
                18, 19, 16,
                // BOTTOM
                20, 23, 22,
                22, 21, 20
        };
        shapeFromVerticesAndIndices(&shape, vertices, sizeof(vertices) / sizeof(*vertices), indices, sizeof(indices) / sizeof(*indices), true);
    }
    return shape;
}

geShape createSquare(bool withIndices) {
    geVertex vertices[] = {
            {{ x - size / 2, y - size / 2, z }, { 0, 0, -1.0f }, {0, 0}},
            {{ x + size / 2, y - size / 2, z }, { 0, 0, -1.0f }, {1, 0}},
            {{ x + size / 2, y + size / 2, z }, { 0, 0, -1.0f }, {1, 1}},
            {{ x - size / 2, y + size / 2, z }, { 0, 0, -1.0f }, {0, 1}},
    };

    GLuint indices[] = {
            0, 3, 1,
            2, 1, 3
    };
    geShape shape;
    shapeFromVerticesAndIndices(&shape, vertices, sizeof(vertices) / sizeof(*vertices), indices, sizeof(indices) / sizeof(*indices), withIndices);
    return shape;
}

geShape createLine() {
    geVertex vertices[] = {
            {{ x - size / 2, y, z }, { 0, 1.0f, 0 } , { 0, 0 }},
            {{ x + size / 2, y, z }, { 0, 1.0f, 0 } , { 0, 1 }}
    };

    GLuint indices[] = {
            0, 1
    };
    geShape shape;
    shapeFromVerticesAndIndices(&shape, vertices, sizeof(vertices) / sizeof(*vertices), indices, sizeof(indices) / sizeof(*indices), true);
    return shape;
}

geShape createCircle(int tess) {
    geVertex vertices[1 + tess * 4];
    GLuint indices[tess * 12];

    // Center of the circle
    geVertex center = {{ x, y, z }, { 0, 0, -1 }, { 0.5f, 0.5f}};
    vertices[0] = center;

    // Iterate through slices of the circle (like a pizza)
    for (int i = 1; i < 1 + tess * 4; i++) {
        vertices[i].pos.x = x + (size / 2) * cosf((i - 1) * PI / (tess * 2));
        vertices[i].pos.y = y + (size / 2) * sinf((i - 1) * PI / (tess * 2));
        vertices[i].pos.z = z;
        vertices[i].normal.x = 0;
        vertices[i].normal.y = 0;
        vertices[i].normal.z = -1;
        // TODO: Rotate texture 90 degrees so that it corresponds properly
        vertices[i].texCoords.x = (1 + cosf(i * PI / (tess * 2))) / 2;
        vertices[i].texCoords.y = (1 + sinf(i * PI / (tess * 2))) / 2;

        indices[(i - 1) * 3] = 0;
        if (i == tess * 4) {
            indices[(i - 1) * 3 + 1] = 1;
        } else {
            indices[(i - 1) * 3 + 1] = (GLuint) i + 1;
        }
        indices[(i - 1) * 3 + 2] = (GLuint) i;
    }

    geShape shape;
    shapeFromVerticesAndIndices(&shape, vertices, sizeof(vertices) / sizeof(*vertices), indices, sizeof(indices) / sizeof(*indices), true);
    return shape;
}

geShape createCylinder(int tess) {
    geVertex vertices[4 + tess * 16];
    GLuint indices[tess * 48];

    // Center of the circles
    geVertex center1 = {{ x, y - size / 2.0f, z }, { 0, -1, 0 }, { 0.5f, 0.5f}};
    geVertex center2 = {{ x, y + size / 2.0f, z }, { 0, 1, 0 }, { 0.5f, 0.5f}};
    vertices[0] = center1;
    vertices[1] = center2;

    // Iterate through slices of the circle (like a pizza)
    for (int i = 0; i < tess * 4; i++) {
        geVertex* currCircle1 = vertices + i * 4 + 2;
        geVertex* currCircle2 = vertices + i * 4 + 3;
        geVertex* currSide1 = vertices + i * 4 + 4;
        geVertex* currSide2 = vertices + i * 4 + 5;

        float posX = x + (size / 2.0f) * cosf(i * PI / (tess * 2));
        float posZ = z + (size / 2.0f) * sinf(i * PI / (tess * 2));

        currCircle1->pos.x = currCircle2->pos.x = posX;
        currCircle1->pos.z = - posZ;
        currCircle2->pos.z = posZ;
        currCircle1->pos.y = y - size / 2;
        currCircle2->pos.y = y + size / 2;
        currCircle1->normal.x = currCircle2->normal.x = 0;
        currCircle1->normal.y = -1;
        currCircle2->normal.y = 1;
        currCircle1->normal.z = currCircle2->normal.z = 0;
        // TODO: Rotate texture 90 degrees so that it corresponds properly
        currCircle1->texCoords.x = currCircle2->texCoords.x = (1 + cosf(i * PI / (tess * 2))) / 2;
        currCircle1->texCoords.y = currCircle2->texCoords.y = (1 + sinf(i * PI / (tess * 2))) / 2;

        currSide1->pos.x = currSide2->pos.x = posX;
        currSide1->pos.z = currSide2->pos.z = posZ;
        currSide1->pos.y = y - size / 2;
        currSide2->pos.y = y + size / 2;
        currSide1->normal.x = currSide2->normal.x = cosf(i * PI / (tess * 2));
        currSide1->normal.y = currSide2->normal.y = 0;
        currSide1->normal.z = currSide2->normal.z = sinf(i * PI / (tess * 2));
        currSide1->texCoords.x = currSide2->texCoords.x = i / (float) (tess * 4);
        currSide1->texCoords.y = 0;
        currSide2->texCoords.y = 1;

        // Circle 1
        indices[i * 12] = 0;
        if (i == tess * 4 - 1) {
            // Wrap around
            indices[i * 12 + 1] = 2;
        } else {
            indices[i * 12 + 1] = (GLuint) i * 4 + 6;
        }
        indices[i * 12 + 2] = (GLuint) i * 4 + 2;

        // Circle 2
        indices[i * 12 + 3] = 1;
        if (i == tess * 4 - 1) {
            // Wrap around
            indices[i * 12 + 4] = 3;
        } else {
            indices[i * 12 + 4] = (GLuint) i * 4 + 7;
        }
        indices[i * 12 + 5] = (GLuint) i * 4 + 3;

        // Side 1
        indices[i * 12 + 6] = (GLuint) i * 4 + 4;
        indices[i * 12 + 7] = (GLuint) i * 4 + 5;
        if (i == tess * 4 - 1) {
            // Wrap around
            indices[i * 12 + 8] = (GLuint) (tess * 16 + 2);
        } else {
            indices[i * 12 + 8] = (GLuint) i * 4 + 8;
        }

        // Side 2
        if (i == tess * 4 - 1) {
            // Wrap around
            indices[i * 12 + 9] = (GLuint) (tess * 16 + 2);
            indices[i * 12 + 11] = (GLuint) (tess * 16 + 3);
        } else {
            indices[i * 12 + 9] = (GLuint) i * 4 + 8;
            indices[i * 12 + 11] = (GLuint) i * 4 + 9;
        }
        indices[i * 12 + 10] = (GLuint) i * 4 + 5;
    }

    vertices[tess * 16 + 2] = vertices[4];
    vertices[tess * 16 + 3] = vertices[5];
    geVertex* currSide1 = vertices + tess * 16 + 2;
    geVertex* currSide2 = vertices + tess * 16 + 3;
    currSide1->texCoords.x = currSide2->texCoords.x = 1;

    geShape shape;
    shapeFromVerticesAndIndices(&shape, vertices, sizeof(vertices) / sizeof(*vertices), indices, sizeof(indices) / sizeof(*indices), true);
    return shape;
}

geShape createTetrahedron() {
    kmMat4 rotZ, rotY, rot;
    kmVec3 normal = { 1, 0, 0 };
    kmVec3 normalFace1, normalFace2, normalFace3;

    kmMat4RotationZ(&rotZ, PI / 4);

    kmMat4RotationY(&rotY, PI / 3);
    kmMat4Multiply(&rot, &rotY, &rotZ);
    kmVec3MultiplyMat4(&normalFace1, &normal, &rot);

    kmMat4RotationY(&rotY, PI);
    kmMat4Multiply(&rot, &rotY, &rotZ);
    kmVec3MultiplyMat4(&normalFace2, &normal, &rot);

    kmMat4RotationY(&rotY, 5 * PI / 3);
    kmMat4Multiply(&rot, &rotY, &rotZ);
    kmVec3MultiplyMat4(&normalFace3, &normal, &rot);

    geVertex vertices[] = {
            {{ 1, 0, 0 },                                     {0, -1, 0}, {1, 0}},
            {{ cosf(2 * PI / 3.0f), 0, sinf(2 * PI / 3.0f) }, {0, -1, 0}, {cosf(2 * PI / 3.0f), sinf(2 * PI / 3.0f)}},
            {{ cosf(4 * PI / 3.0f), 0, sinf(4 * PI / 3.0f) }, {0, -1, 0}, {cosf(4 * PI / 3.0f), sinf(4 * PI / 3.0f)}},

            {{ 1, 0, 0 },                                     normalFace3, {1, 0}},
            {{ 0, 1, 0 },                                     normalFace3, {0, 1}},
            {{ cosf(2 * PI / 3.0f), 0, sinf(2 * PI / 3.0f) }, normalFace3, {cosf(2 * PI / 3.0f), sinf(2 * PI / 3.0f)}},

            {{ 1, 0, 0 },                                     normalFace1, {1, 0}},
            {{ cosf(4 * PI / 3.0f), 0, sinf(4 * PI / 3.0f) }, normalFace1, {cosf(4 * PI / 3.0f), sinf(4 * PI / 3.0f)}},
            {{ 0, 1, 0 },                                     normalFace1, {0, 1}},

            {{ 0, 1, 0 },                                     normalFace2, {0, 1}},
            {{ cosf(4 * PI / 3.0f), 0, sinf(4 * PI / 3.0f) }, normalFace2, {cosf(4 * PI / 3.0f), sinf(4 * PI / 3.0f)}},
            {{ cosf(2 * PI / 3.0f), 0, sinf(2 * PI / 3.0f) }, normalFace2, {cosf(2 * PI / 3.0f), sinf(2 * PI / 3.0f)}},
    };

    GLuint indices[] = {
            0, 1, 2,
            3, 4, 5,
            6, 7, 8,
            9, 10, 11
    };

    geShape shape;
    shapeFromVerticesAndIndices(&shape, vertices, sizeof(vertices) / sizeof(*vertices), indices, sizeof(indices) / sizeof(*indices), true);
    return shape;
}

geShape createTrigTerrain(int tess) {
    geVertex* vertices = calloc((size_t) (tess * tess), sizeof(geVertex));
    GLuint* indices = calloc((size_t) ((tess - 1) * (tess - 1) * 6), sizeof(GLuint));

    for (int i = 0; i < tess; i++) {
        for (int j = 0; j < tess; j++) {
            vertices[i * tess + j].pos.x = (i - tess / 2.0f) / (tess - 1);
            vertices[i * tess + j].pos.y = fmaxf(sinf(((float) i / (tess - 1)) * PI) * sinf(((float) j / (tess - 1)) * PI), 0.0f);
            vertices[i * tess + j].pos.z = (j - tess / 2.0f) / (tess - 1);
            vertices[i * tess + j].texCoords.x = (float) i / (tess - 1);
            vertices[i * tess + j].texCoords.y = (float) j / (tess - 1);
            vertices[i * tess + j].texCoords.z = 0;
        }
    }

    for (int i = 0; i < tess - 1; i++) {
        geVertex* v1 = vertices + (tess - 1 - i);
        geVertex* v2 = vertices + tess * (tess - 1) + i;
        geVertex* v3 = vertices + i * tess;
        geVertex* v4 = vertices + (tess - 1 - i) * tess + tess - 1;

        if (i == 0) {
            v3->normal.x = cosf(5 * PI / 4);
            v3->normal.y = 0;
            v3->normal.z = sinf(5 * PI / 4);

            v4->normal.x = cosf(PI / 4);
            v4->normal.y = 0;
            v4->normal.z = sinf(PI / 4);

            v1->normal.x = cosf(3 * PI / 4);
            v1->normal.y = 0;
            v1->normal.z = sinf(3 * PI / 4);

            v2->normal.x = cosf(-PI / 4);
            v2->normal.y = 0;
            v2->normal.z = sinf(-PI / 4);
        } else {
            v1->normal.x = -1;
            v1->normal.y = 0;
            v1->normal.z = 0;

            v2->normal.x = 1;
            v2->normal.y = 0;
            v2->normal.z = 0;

            v3->normal.x = 0;
            v3->normal.y = 0;
            v3->normal.z = -1;

            v4->normal.x = 0;
            v4->normal.y = 0;
            v4->normal.z = 1;
        }
    }

    for (int i = 1; i < tess - 1; i++) {
        for (int j = 1; j < tess - 1; j++) {
            geVertex* current = vertices + i * tess + j;
            geVertex* behindX = vertices + (i - 1) * tess + j;
            geVertex* frontX = vertices + (i + 1) * tess + j;
            geVertex* behindZ = vertices + i * tess + j - 1;
            geVertex* frontZ = vertices + i * tess + j + 1;

            // Vectors for front and behind current point projected on XY and ZY plane
            kmVec3 vecBehindX = {behindX->pos.x, behindX->pos.y, 0};
            kmVec3 vecFrontX =  {frontX->pos.x, frontX->pos.y, 0};
            kmVec3 vecBehindZ = {0, behindZ->pos.y, behindZ->pos.z};
            kmVec3 vecFrontZ = {0, frontZ->pos.y, frontZ->pos.z};

            // Lines from one vector to the other
            kmVec3 resX, resZ;
            kmVec3Subtract(&resX, &vecBehindX, &vecFrontX);
            kmVec3Subtract(&resZ, &vecBehindZ, &vecFrontZ);

            // The cross result of them which should be the normal
            kmVec3 res;
            kmVec3Cross(&res, &resZ, &resX);
            kmVec3Normalize(&res, &res);

            current->normal = res;
        }
    }

    for (int k = 0; k < (tess - 1) * (tess - 1) * 6; k += 6) {
        int sq = k / 6;
        int row = sq / (tess - 1);
        int column = sq % (tess - 1);
        indices[k    ] = (GLuint) (row * tess + column);
        indices[k + 1] = (GLuint) (row * tess + column + 1);
        indices[k + 2] = (GLuint) ((row + 1) * tess + column + 1);
        indices[k + 3] = (GLuint) ((row + 1) * tess + column + 1);
        indices[k + 4] = (GLuint) ((row + 1) * tess + column);
        indices[k + 5] = (GLuint) (row * tess + column);
    }

    geShape shape;
    shape.vertices = vertices;
    shape.indices = indices;
    shape.numVertices = tess * tess;
    shape.numIndices = (tess - 1) * (tess - 1) * 6;
    return shape;
}

geShape createLineNormals(geShape* shape) {
    geVertex* vertices = calloc(shape->numVertices * 2, sizeof(geVertex));
    GLuint* indices = calloc(shape->numVertices * 2, sizeof(GLuint));
    for (int i = 0; i < shape->numVertices; i++) {
        vertices[i * 2].pos.x = shape->vertices[i].pos.x;
        vertices[i * 2 + 1].pos.x = shape->vertices[i].pos.x + shape->vertices[i].normal.x;
        vertices[i * 2].pos.y = shape->vertices[i].pos.y;
        vertices[i * 2 + 1].pos.y = shape->vertices[i].pos.y + shape->vertices[i].normal.y;
        vertices[i * 2].pos.z = shape->vertices[i].pos.z;
        vertices[i * 2 + 1].pos.z = shape->vertices[i].pos.z + shape->vertices[i].normal.z;
        vertices[i * 2].texCoords.x = 1.0f;
        vertices[i * 2 + 1].texCoords.x = 0.0f;
        vertices[i * 2].texCoords.y = 0;
        vertices[i * 2 + 1].texCoords.y = 0;
        vertices[i * 2].texCoords.z = 0;
        vertices[i * 2 + 1].texCoords.z = 0;
        indices[i * 2] = (GLuint) i * 2;
        indices[i * 2 + 1] = (GLuint) i * 2 + 1;
    }
    geShape result;
    result.vertices = vertices;
    result.indices = indices;
    result.numVertices = shape->numVertices * 2;
    result.numIndices = shape->numVertices * 2;
    return result;
}

geShape createNoiseTerrain(int tess) {
    geVertex* vertices = calloc((size_t) (tess * tess), sizeof(geVertex));
    GLuint* indices = calloc((size_t) ((tess - 1) * (tess - 1) * 6), sizeof(GLuint));

    for (int i = 0; i < tess; i++) {
        for (int j = 0; j < tess; j++) {
            geVertex* v = vertices + (i * tess + j);
            v->pos.x = (i - tess / 2.0f) / (tess - 1);
            v->pos.z = (j - tess / 2.0f) / (tess - 1);

            float h = 1     * sdnoise2(v->pos.x , v->pos.z, NULL, NULL)
                    + 0.5f  * sdnoise2(v->pos.x * 2, v->pos.z * 2, NULL, NULL)
                    + 0.25f * sdnoise2(v->pos.x * 4, v->pos.z * 4, NULL, NULL);
            v->pos.y = powf(h, 5.0f);
            v->texCoords.x = (float) i / (tess - 1);
            v->texCoords.y = (float) j / (tess - 1);
            v->texCoords.z = 0;
        }
    }
    for (int i = 1; i < tess - 1; i++) {
        for (int j = 1; j < tess - 1; j++) {
            geVertex* current = vertices + i * tess + j;
            geVertex* behindX = vertices + (i - 1) * tess + j;
            geVertex* frontX = vertices + (i + 1) * tess + j;
            geVertex* behindZ = vertices + i * tess + j - 1;
            geVertex* frontZ = vertices + i * tess + j + 1;

            // Vectors for front and behind current point projected on XY and ZY plane
            kmVec3 vecBehindX = {behindX->pos.x, behindX->pos.y, 0};
            kmVec3 vecFrontX =  {frontX->pos.x, frontX->pos.y, 0};
            kmVec3 vecBehindZ = {0, behindZ->pos.y, behindZ->pos.z};
            kmVec3 vecFrontZ = {0, frontZ->pos.y, frontZ->pos.z};

            // Lines from one vector to the other
            kmVec3 resX, resZ;
            kmVec3Subtract(&resX, &vecBehindX, &vecFrontX);
            kmVec3Subtract(&resZ, &vecBehindZ, &vecFrontZ);

            // The cross result of them which should be the normal
            kmVec3 res;
            kmVec3Cross(&res, &resZ, &resX);
            kmVec3Normalize(&res, &res);

            current->normal = res;
        }
    }
    for (int k = 0; k < (tess - 1) * (tess - 1) * 6; k += 6) {
        int sq = k / 6;
        int row = sq / (tess - 1);
        int column = sq % (tess - 1);
        indices[k    ] = (GLuint) (row * tess + column);
        indices[k + 1] = (GLuint) (row * tess + column + 1);
        indices[k + 2] = (GLuint) ((row + 1) * tess + column + 1);
        indices[k + 3] = (GLuint) ((row + 1) * tess + column + 1);
        indices[k + 4] = (GLuint) ((row + 1) * tess + column);
        indices[k + 5] = (GLuint) (row * tess + column);
    }

    geShape shape;
    shape.vertices = vertices;
    shape.indices = indices;
    shape.numVertices = tess * tess;
    shape.numIndices = (tess - 1) * (tess - 1) * 6;
    return shape;
}

geShape createVoxelWorldDumb(size_t surfaceSize, size_t height) {
    geVertex vertices[] = {
            // FRONT
            {{ x - size / 2, y - size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {0, 0}},
            {{ x + size / 2, y - size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {1, 0}},
            {{ x + size / 2, y + size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {1, 1}},
            {{ x - size / 2, y + size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {0, 1}},
            // BACK
            {{ x - size / 2, y - size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {0, 0}},
            {{ x - size / 2, y + size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {0, 1}},
            {{ x + size / 2, y + size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {1, 1}},
            {{ x + size / 2, y - size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {1, 0}},
            // LEFT
            {{ x - size / 2, y - size / 2, z - size / 2 }, { -1.0f, 0.0f, 0.0f }, {0, 0}},
            {{ x - size / 2, y - size / 2, z + size / 2 }, { -1.0f, 0.0f, 0.0f }, {1, 0}},
            {{ x - size / 2, y + size / 2, z + size / 2 }, { -1.0f, 0.0f, 0.0f }, {1, 1}},
            {{ x - size / 2, y + size / 2, z - size / 2 }, { -1.0f, 0.0f, 0.0f }, {0, 1}},
            // RIGHT
            {{ x + size / 2, y - size / 2, z - size / 2 }, { 1.0f, 0.0f, 0.0f }, {0, 0}},
            {{ x + size / 2, y + size / 2, z - size / 2 }, { 1.0f, 0.0f, 0.0f }, {0, 1}},
            {{ x + size / 2, y + size / 2, z + size / 2 }, { 1.0f, 0.0f, 0.0f }, {1, 1}},
            {{ x + size / 2, y - size / 2, z + size / 2 }, { 1.0f, 0.0f, 0.0f }, {1, 0}},
            // TOP
            {{ x + size / 2, y + size / 2, z - size / 2 }, { 0.0f, 1.0f, 0.0f }, {1, 0}},
            {{ x - size / 2, y + size / 2, z - size / 2 }, { 0.0f, 1.0f, 0.0f }, {0, 0}},
            {{ x - size / 2, y + size / 2, z + size / 2 }, { 0.0f, 1.0f, 0.0f }, {0, 1}},
            {{ x + size / 2, y + size / 2, z + size / 2 }, { 0.0f, 1.0f, 0.0f }, {1, 1}},
            // BOTTOM
            {{ x - size / 2, y - size / 2, z - size / 2 }, { 0.0f, -1.0f, 0.0f }, {0, 0}},
            {{ x - size / 2, y - size / 2, z + size / 2 }, { 0.0f, -1.0f, 0.0f }, {1, 0}},
            {{ x + size / 2, y - size / 2, z + size / 2 }, { 0.0f, -1.0f, 0.0f }, {1, 1}},
            {{ x + size / 2, y - size / 2, z - size / 2 }, { 0.0f, -1.0f, 0.0f }, {0, 1}},
    };

    GLuint indices[] = {
            // FRONT
            0, 1, 2,
            2, 3, 0,
            // BACK
            4, 5, 6,
            6, 7, 4,
            // LEFT
            8, 9, 10,
            10, 11, 8,
            // RIGHT
            12, 13, 14,
            14, 15, 12,
            // TOP
            16, 17, 18,
            18, 19, 16,
            // BOTTOM
            20, 23, 22,
            22, 21, 20
    };

    size_t i, j, k;
    size_t heightMap[surfaceSize * surfaceSize];
    size_t arrayLength = 0;

    for (i = 0; i < surfaceSize; i++) {
        for (j = 0; j < surfaceSize; j++) {
            int noise = (int) floorf(sdnoise2(i, j, NULL, NULL) * 2.0f);
            heightMap[i * surfaceSize + j] = (size_t) (height + noise);
            arrayLength += (size_t) (height + noise);
        }
    }
    printf("Array length for dumb is %llu\n", arrayLength);

    geShape shape;

    size_t numVertices = 24;
    size_t numIndices = 36;

    shape.numVertices = numVertices * arrayLength;
    shape.numIndices = numIndices * arrayLength;
    shape.vertices = calloc(numVertices * arrayLength, sizeof(geVertex));
    shape.indices = calloc(numIndices * arrayLength, sizeof(GLuint));

    size_t currentBlockIndex = 0;

    for (i = 0; i < surfaceSize * surfaceSize; i++) {
        size_t line = i / surfaceSize;
        size_t column = i % surfaceSize;
        for (j = 0; j < heightMap[i]; j++) {
            for (k = 0; k < numVertices; k++) {
                geVertex* vertexBlock = vertices + k;
                geVertex* vertexWorld = shape.vertices + (currentBlockIndex * numVertices + k);

                vertexWorld->normal.x = vertexBlock->normal.x;
                vertexWorld->normal.y = vertexBlock->normal.y;
                vertexWorld->normal.z = vertexBlock->normal.z;

                vertexWorld->pos.x = vertexBlock->pos.x + line;
                vertexWorld->pos.y = vertexBlock->pos.y + j;
                vertexWorld->pos.z = vertexBlock->pos.z + column;

                vertexWorld->texCoords.x = vertexBlock->texCoords.x;
                vertexWorld->texCoords.y = vertexBlock->texCoords.y;
                vertexWorld->texCoords.z = vertexBlock->texCoords.z;
            }
            for (k = 0; k < numIndices; k++) {
                shape.indices[currentBlockIndex * numIndices + k] = (GLuint) (indices[k] + currentBlockIndex * numVertices);
            }
            currentBlockIndex++;
        }
    }
    return shape;
}

geShape createVoxelWorldWithCulling(size_t surfaceSize, size_t height) {
    struct timeval tStart, tEnd;
    gettimeofday(&tStart, NULL);

    // <editor-fold> INIT STAGE
    geVertex vertices[] = {
            // FRONT
            {{ x - size / 2, y - size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {0, 0}},
            {{ x + size / 2, y - size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {1, 0}},
            {{ x + size / 2, y + size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {1, 1}},
            {{ x - size / 2, y + size / 2, z + size / 2 }, { 0.0f, 0.0f,  1.0f }, {0, 1}},
            // BACK
            {{ x - size / 2, y - size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {0, 0}},
            {{ x - size / 2, y + size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {0, 1}},
            {{ x + size / 2, y + size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {1, 1}},
            {{ x + size / 2, y - size / 2, z - size / 2 }, { 0.0f, 0.0f, -1.0f }, {1, 0}},
            // LEFT
            {{ x - size / 2, y - size / 2, z - size / 2 }, { -1.0f, 0.0f, 0.0f }, {0, 0}},
            {{ x - size / 2, y - size / 2, z + size / 2 }, { -1.0f, 0.0f, 0.0f }, {1, 0}},
            {{ x - size / 2, y + size / 2, z + size / 2 }, { -1.0f, 0.0f, 0.0f }, {1, 1}},
            {{ x - size / 2, y + size / 2, z - size / 2 }, { -1.0f, 0.0f, 0.0f }, {0, 1}},
            // RIGHT
            {{ x + size / 2, y - size / 2, z - size / 2 }, { 1.0f, 0.0f, 0.0f }, {0, 0}},
            {{ x + size / 2, y + size / 2, z - size / 2 }, { 1.0f, 0.0f, 0.0f }, {0, 1}},
            {{ x + size / 2, y + size / 2, z + size / 2 }, { 1.0f, 0.0f, 0.0f }, {1, 1}},
            {{ x + size / 2, y - size / 2, z + size / 2 }, { 1.0f, 0.0f, 0.0f }, {1, 0}},
            // TOP
            {{ x + size / 2, y + size / 2, z - size / 2 }, { 0.0f, 1.0f, 0.0f }, {1, 0}},
            {{ x - size / 2, y + size / 2, z - size / 2 }, { 0.0f, 1.0f, 0.0f }, {0, 0}},
            {{ x - size / 2, y + size / 2, z + size / 2 }, { 0.0f, 1.0f, 0.0f }, {0, 1}},
            {{ x + size / 2, y + size / 2, z + size / 2 }, { 0.0f, 1.0f, 0.0f }, {1, 1}},
            // BOTTOM
            {{ x - size / 2, y - size / 2, z - size / 2 }, { 0.0f, -1.0f, 0.0f }, {0, 0}},
            {{ x - size / 2, y - size / 2, z + size / 2 }, { 0.0f, -1.0f, 0.0f }, {1, 0}},
            {{ x + size / 2, y - size / 2, z + size / 2 }, { 0.0f, -1.0f, 0.0f }, {1, 1}},
            {{ x + size / 2, y - size / 2, z - size / 2 }, { 0.0f, -1.0f, 0.0f }, {0, 1}},
    };

    GLuint indices[] = {
            // FRONT
            0, 1, 2,
            2, 3, 0,
            // BACK
            4, 5, 6,
            6, 7, 4,
            // LEFT
            8, 9, 10,
            10, 11, 8,
            // RIGHT
            12, 13, 14,
            14, 15, 12,
            // TOP
            16, 17, 18,
            18, 19, 16,
            // BOTTOM
            20, 23, 22,
            22, 21, 20
    };

    size_t oX, oY, oZ, k;
    size_t arrayLength = 0;
    int map[surfaceSize][surfaceSize][MAX_HEIGHT];
    memset(map, 0, sizeof(int) * surfaceSize * surfaceSize * MAX_HEIGHT);

    for (oX = 0; oX < surfaceSize; oX++) {
        for (oZ = 0; oZ < surfaceSize; oZ++) {
            int noise = (int) floorf(sdnoise2(oX, oZ, NULL, NULL) * 2.0f);
            for (oY = 0; oY < height + noise; oY++) {
                map[oX][oZ][oY] = 1;
            }
            arrayLength += (size_t) (height + noise);
        }
    }
    printf("Array length for culled is %llu\n", arrayLength);

    geShape shape;

    size_t numVertices = 24;
    size_t numIndices = 36;

    shape.numVertices = numVertices * arrayLength;
    shape.numIndices = numIndices * arrayLength;
    shape.vertices = calloc(numVertices * arrayLength, sizeof(geVertex));
    shape.indices = calloc(numIndices * arrayLength, sizeof(GLuint));

    size_t currentBlockIndex = 0;

    for (oX = 0; oX < surfaceSize; oX++) {
        for (oZ = 0; oZ < surfaceSize; oZ++) {
            for (oY = 0; oY < MAX_HEIGHT; oY++) {
                if (map[oX][oZ][oY] == 0) {
                    continue;
                }
                for (k = 0; k < numVertices; k++) {
                    geVertex* vertexBlock = vertices + k;
                    geVertex* vertexWorld = shape.vertices + (currentBlockIndex * numVertices + k);

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
                    shape.indices[currentBlockIndex * numIndices + k] = (GLuint) (indices[k] + currentBlockIndex * numVertices);
                }
                currentBlockIndex++;
            }
        }
    }
    gettimeofday(&tEnd, NULL);
    printf("Time for initializing voxel world: %.2lfms\n", timeDiff(tEnd, tStart));
    gettimeofday(&tStart, NULL);

    // </editor-fold>
    currentBlockIndex = 0;
    size_t indicesJump = 0;
    size_t verticesJump = 0;
    for (oX = 0; oX < surfaceSize; oX++) {
        for (oZ = 0; oZ < surfaceSize; oZ++) {
            for (oY = 0; oY < MAX_HEIGHT; oY++) {
//                struct timeval stop, start;
//                gettimeofday(&start, NULL);

                if (map[oX][oZ][oY] == 0) {
                    continue;
                }

                bool isAdjacent[] = {
                        oZ + 1 < surfaceSize && map[oX][oZ + 1][oY] != 0,
                        oZ != 0 && map[oX][oZ - 1][oY] != 0,

                        oX != 0 && map[oX - 1][oZ][oY] != 0,
                        oX + 1 < surfaceSize && map[oX + 1][oZ][oY] != 0,

                        oY + 1 < MAX_HEIGHT && map[oX][oZ][oY + 1] != 0,
                        oY != 0 && map[oX][oZ][oY - 1] != 0,
                };

                for (k = 0; k < 6; k++) {
                    if (isAdjacent[k]) {
                        removeFace(&shape, currentBlockIndex * numVertices - verticesJump + k * 4, currentBlockIndex * numIndices - indicesJump + k * 6, 1);
                        verticesJump += 4;
                        indicesJump += 6;
                    }
                }
                currentBlockIndex++;

//                gettimeofday(&stop, NULL);
//                printf("took %lfms for one block\n", 1000000.0 / ((stop.tv_usec - start.tv_usec)));
            }
        }
    }
    realloc(shape.vertices, shape.numVertices * sizeof(geVertex));
    realloc(shape.indices, shape.numIndices * sizeof(GLuint));

    gettimeofday(&tEnd, NULL);
    printf("Time for culling voxel world: %.2lfms\n", timeDiff(tEnd, tStart));

    return shape;
}

geShape createVoxelWorldWithGreedy(size_t surfaceSize, size_t height) {
    struct timeval tStart, tEnd;
    size_t l, k;
    geShape shape = createVoxelWorldWithCulling(surfaceSize, height);
    gettimeofday(&tStart, NULL);

    for (k = 0; k < shape.numVertices; k += 4) {
        geVertex* kFace = shape.vertices + k;
        for (l = 0; l < shape.numVertices; l += 4) {
            geVertex* lFace = shape.vertices + l;
            if (k == l) {
                continue;
            }
            size_t i, j, count = 0;

            size_t sameIndices[4];
            for (i = 0; i < 4; i++) {
                for (j = 0; j < 4; j++) {
                    if (memcmp(&kFace[i].pos, &lFace[j].pos, sizeof(kmVec3)) == 0) {
                        sameIndices[count * 2] = i;
                        sameIndices[count * 2 + 1] = j;
                        count++;
                        break;
                    }
                }
            }

            if (count == 2 && memcmp(&kFace[0].normal, &lFace[0].normal, sizeof(kmVec3)) == 0) {

//                printf("Found same indices with same orientation here: %llu with %llu and %llu with %llu\n", sameIndices[0], sameIndices[1], sameIndices[2], sameIndices[3]);
//                printf("And their positions are \n\t");
//                printVec3(&kFace[sameIndices[0]].pos);
//                printf("\n\t");
//                printVec3(&lFace[sameIndices[1]].pos);
//                printf("\n\t");
//                printVec3(&kFace[sameIndices[2]].pos);
//                printf("\n\t");
//                printVec3(&lFace[sameIndices[3]].pos);
//                printf("\n");
//                printFace(kFace);
//                printFace(lFace);

                memcpy(&kFace[sameIndices[0]].pos, &lFace[sameIndices[0]].pos, sizeof(kmVec3));
                memcpy(&kFace[sameIndices[2]].pos, &lFace[sameIndices[2]].pos, sizeof(kmVec3));

                size_t minVertex;
                if (sameIndices[0] == 0 && sameIndices[2] == 3 || sameIndices[0] == 3 && sameIndices[2] == 0) {
                    // Cover the case where 0 and 3 are common
                    minVertex = 3;
                } else {
                    minVertex = __min(sameIndices[0], sameIndices[2]);
                }

                geVertex* kFaceOppositeSideVertex = kFace + (minVertex == 0 ? 3 : minVertex - 1);
                float texCoordDiffX = fabsf(kFaceOppositeSideVertex->texCoords.x - kFace[minVertex].texCoords.x);
                float texCoordDiffY = fabsf(kFaceOppositeSideVertex->texCoords.y - kFace[minVertex].texCoords.y);

//                printf("Got texcoord differences of x: %f and y: %f\n", texCoordDiffX, texCoordDiffY);
                if (texCoordDiffX != 0) {
                    kFace[sameIndices[0]].texCoords.x += 1;
                    kFace[sameIndices[2]].texCoords.x += 1;
                } else if (texCoordDiffY != 0) {
                    kFace[sameIndices[0]].texCoords.y += 1;
                    kFace[sameIndices[2]].texCoords.y += 1;
                } else {
                    printf("-------------- NO DIFFERENCE -----------------------------------------------------------\n");
                }

                removeFace(&shape, l, l / 2 * 3, 1);
                l -= 4;
//                return shape;
            }

        }
    }
    gettimeofday(&tEnd, NULL);
    printf("Time for greedy on voxel world: %.2lfms\n", timeDiff(tEnd, tStart));

    return shape;
}