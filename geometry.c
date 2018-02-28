//
// Created by Afterwind on 9/25/2017.
//

#include <stdlib.h>
#include <mem.h>
#include <stdio.h>
#include "geometry.h"
#include "utils.h"
#include "simplex_noise.h"

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
    shapes[GE_VERTEX_WORLD_DUMB] = createVoxelWorldDumb(8);
    shapes[GE_VERTEX_WORLD_LESS_DUMB] = createVoxelWorldLessDumb(8);
//    printf("%f %f %f %f\n", 1 / 3.0f, 1536 / 3.0f, (1 / 3.0f) * 1536, (2 * 1 / 3.0f) * 1536);
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

geShape createVoxelWorldDumb(int size) {
    size_t arrayLength = (size_t) (size * size * size);

    geShape shape;

    shape.numVertices = 24 * arrayLength;
    shape.numIndices = 36 * arrayLength;
    shape.vertices = calloc(24 * arrayLength, sizeof(geVertex));
    shape.indices = calloc(36 * arrayLength, sizeof(GLuint));

    geShape blocks[arrayLength];

    size_t i, j;
    for (i = 0; i < arrayLength; i++) {
        blocks[i] = createCube(false);
        for (j = 0; j < blocks[i].numVertices; j++) {
            geVertex* vertexBlock = blocks[i].vertices + j;
            geVertex* vertexWorld = shape.vertices + (i * 24 + j);

            size_t line = (i % (size * size)) / size;
            size_t column = (i % (size * size)) % size;
            size_t depth = i / (size * size);

            vertexWorld->normal.x = vertexBlock->normal.x;
            vertexWorld->normal.y = vertexBlock->normal.y;
            vertexWorld->normal.z = vertexBlock->normal.z;

            vertexWorld->pos.x = vertexBlock->pos.x + line;
            vertexWorld->pos.y = vertexBlock->pos.y + column;
            vertexWorld->pos.z = vertexBlock->pos.z + depth;

            vertexWorld->texCoords.x = vertexBlock->texCoords.x;
            vertexWorld->texCoords.y = vertexBlock->texCoords.y;
            vertexWorld->texCoords.z = vertexBlock->texCoords.z;
        }

        for (j = 0; j < blocks[i].numIndices; j++) {
            shape.indices[i * 36 + j] = (GLuint) (blocks[i].indices[j] + i * 24);
        }

        free(blocks[i].vertices);
        free(blocks[i].indices);
    }
    return shape;
}

geShape createVoxelWorldLessDumb(int worldSize) {
    size_t arrayLength = (size_t) (worldSize * worldSize * worldSize);

    geShape shape;

    int indicesPerCube = 14;

    GLuint indices[] = {
            0, 1, 3, 2,
            6, 1, 7, 0,
            4, 3, 5, 6,
            4, 7
    };

    shape.numVertices = 8 * arrayLength;
    shape.numIndices = indicesPerCube * arrayLength;
    shape.vertices = calloc(shape.numVertices, sizeof(geVertex));
    shape.indices = calloc(shape.numIndices, sizeof(GLuint));

    geVertex vertices[] = {
            // FRONT
            {{ x - size / 2, y - size / 2, z + size / 2 }, { 0.0f, 1.0f,  0.0f }, {0.25f , 1 / 3.0f}},
            {{ x + size / 2, y - size / 2, z + size / 2 }, { 0.0f, 1.0f,  0.0f }, {0.5f, 1 / 3.0f }},
            {{ x + size / 2, y + size / 2, z + size / 2 }, { 0.0f, 1.0f,  0.0f }, {0.5f, 2 / 3.0f }},
            {{ x - size / 2, y + size / 2, z + size / 2 }, { 0.0f, 1.0f,  0.0f }, {0.25f, 2 / 3.0f }},
            // BACK
            {{ x - size / 2, y - size / 2, z - size / 2 }, { 0.0f, 1.0f, 0.0f }, {1, 1 / 3.0f}},
            {{ x - size / 2, y + size / 2, z - size / 2 }, { 0.0f, 1.0f, 0.0f }, {1, 2 / 3.0f}},
            {{ x + size / 2, y + size / 2, z - size / 2 }, { 0.0f, 1.0f, 0.0f }, {0.75f, 2 / 3.0f}},
            {{ x + size / 2, y - size / 2, z - size / 2 }, { 0.0f, 1.0f, 0.0f }, {0.75f, 1 / 3.0f}},
    };


    size_t i, j;
    for (i = 0; i < arrayLength; i++) {
        for (j = 0; j < 8; j++) {
            geVertex* vertexBlock = vertices + j;
            geVertex* vertexWorld = shape.vertices + (i * 8 + j);

            size_t line = (i % (worldSize * worldSize)) / worldSize;
            size_t column = (i % (worldSize * worldSize)) % worldSize;
            size_t depth = i / (worldSize * worldSize);

            vertexWorld->normal.x = vertexBlock->normal.x;
            vertexWorld->normal.y = vertexBlock->normal.y;
            vertexWorld->normal.z = vertexBlock->normal.z;

            vertexWorld->pos.x = vertexBlock->pos.x + line;
            vertexWorld->pos.y = vertexBlock->pos.y + column;
            vertexWorld->pos.z = vertexBlock->pos.z + depth;

            vertexWorld->texCoords.x = vertexBlock->texCoords.x;
            vertexWorld->texCoords.y = vertexBlock->texCoords.y;
            vertexWorld->texCoords.z = vertexBlock->texCoords.z;
        }

        for (j = 0; j < indicesPerCube; j++) {
            shape.indices[i * indicesPerCube + j] = (GLuint) (indices[j] + i * 8);
        }
    }
    return shape;
}
