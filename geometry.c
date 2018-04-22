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
#define PER_CUBE_VERTICES 24
#define PER_CUBE_INDICES 36

const float x = 0;
const float y = 0;
const float z = 0;
const float size = 1.0f;

/* INTERNAL FUNCTIONS */
geShape shapeFromVerticesAndIndices(geVertex* vertices, size_t numVertices, GLuint* indices, size_t numIndices, bool convertIndicesToVertices) {
    geShape shape = { 0 };
    if (convertIndicesToVertices) {
        shape.vertices = calloc(numVertices, sizeof(geVertex));
        memcpy(shape.vertices, vertices, numVertices * sizeof(geVertex));
        if (indices != NULL && numIndices != 0) {
            shape.indices = calloc(numIndices, sizeof(GLuint));
            memcpy(shape.indices, indices, numIndices * sizeof(GLuint));
        }
        shape.numVertices = numVertices;
        shape.numIndices = numIndices;
    } else {
        shape.numVertices = numIndices;
        shape.vertices = calloc(shape.numVertices, sizeof(geVertex));
        for (int i = 0; i < shape.numVertices; i++) {
            memcpy(shape.vertices + i, vertices + indices[i], sizeof(geVertex));
        }
    }
    return shape;
}

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

// Given a shape removes a face
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
    shapes[GE_CUBE_BORDER] = createCubeBorder();
    shapes[GE_SQUARE] = createSquare(true);
    shapes[GE_LINE] = createLine();
    shapes[GE_CIRCLE] = createCircle(25);
    shapes[GE_CYLINDER] = createCylinder(25);
    shapes[GE_TETRAHEDRON] = createTetrahedron();
    shapes[GE_TERRAIN_TRIG] = createTrigTerrain(200);
    shapes[GE_NORMALS] = createLineNormals(shapes + GE_TERRAIN_TRIG);
    shapes[GE_TERRAIN_NOISE] = createNoiseTerrain(40);
//    printf(" \n----- DUMB INIT ----- \n");
//    shapes[GE_VERTEX_WORLD_DUMB] = createVoxelWorldDumb(50, 32);
//    printf("The dumb version draws: %llu vertices and %llu indices\n", shapes[GE_VERTEX_WORLD_DUMB].numVertices, shapes[GE_VERTEX_WORLD_DUMB].numIndices);
//    printf(" \n----- CULLED INIT ----- \n");
//    shapes[GE_VERTEX_WORLD_CULLED] = createVoxelWorldWithCulling(50, 32);
//    printf("The culled version draws: %llu vertices and %llu indices\n", shapes[GE_VERTEX_WORLD_CULLED].numVertices, shapes[GE_VERTEX_WORLD_CULLED].numIndices);
//    printf(" \n----- GREEDY INIT ----- \n");
//    shapes[GE_VERTEX_WORLD_GREEDY] = createVoxelWorldWithGreedy(50, 5);
//    printf("The greedy version draws: %llu vertices and %llu indices\n", shapes[GE_VERTEX_WORLD_GREEDY].numVertices, shapes[GE_VERTEX_WORLD_GREEDY].numIndices);
    shapes[GE_2D_CROSSHAIR] = create2DCrossHair();
    shapes[GE_3D_CROSSHAIR] = create3DCrossHair();


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
        return shapeFromVerticesAndIndices(vertices, sizeof(vertices) / sizeof(*vertices), indices, sizeof(indices) / sizeof(*indices), true);
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
        return shapeFromVerticesAndIndices(vertices, sizeof(vertices) / sizeof(*vertices), indices, sizeof(indices) / sizeof(*indices), true);
    }
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
    return shapeFromVerticesAndIndices(vertices, sizeof(vertices) / sizeof(*vertices), indices, sizeof(indices) / sizeof(*indices), withIndices);
}

geShape createCubeBorder() {
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
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            8, 9, 9, 10, 10, 11, 11, 8,
            12, 13, 13, 14, 14, 15, 15, 12,
            16, 17, 17, 18, 18, 19, 19, 16,
            20, 21, 21, 22, 22, 23, 23, 20,
    };

    return shapeFromVerticesAndIndices(vertices, sizeof(vertices) / sizeof(*vertices), indices, sizeof(indices) / sizeof(*indices), true);
}

geShape createLine() {
    geVertex vertices[] = {
            {{ x, y, z - size / 2}, { 0, 1.0f, 0 } , { 0, 0 }},
            {{ x, y, z + size / 2}, { 0, 1.0f, 0 } , { 0, 1 }}
    };

    GLuint indices[] = { 0, 1 };

    return shapeFromVerticesAndIndices(vertices, sizeof(vertices) / sizeof(*vertices), indices, 2, true);
}

geShape createCircle(unsigned long long tess) {
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

    return shapeFromVerticesAndIndices(vertices, sizeof(vertices) / sizeof(*vertices), indices, sizeof(indices) / sizeof(*indices), true);
}

geShape createCylinder(unsigned long long tess) {
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

    return shapeFromVerticesAndIndices(vertices, sizeof(vertices) / sizeof(*vertices), indices, sizeof(indices) / sizeof(*indices), true);
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

    return shapeFromVerticesAndIndices(vertices, sizeof(vertices) / sizeof(*vertices), indices, sizeof(indices) / sizeof(*indices), true);
}

geShape createTrigTerrain(unsigned long long tess) {
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
        unsigned long long row = sq / (tess - 1);
        unsigned long long column = sq % (tess - 1);
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

geShape createNoiseTerrain(unsigned long long tess) {
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
        unsigned long long row = sq / (tess - 1);
        unsigned long long column = sq % (tess - 1);
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

geShape create2DCrossHair() {
    geVertex vertices[] = {
            {{ x - size / 2, 0, 1 }, { 0, 0, 1 }, { 0, 0 }},
            {{ x + size / 2, 0, 1 }, { 0, 0, 1 }, { 1, 0 }},
            {{ 0, y - size / 2, 1 }, { 0, 0, 1 }, { 0, 0 }},
            {{ 0, y + size / 2, 1 }, { 0, 0, 1 }, { 0, 1 }}
    };

    GLuint indices[] = {
            0, 1, 2, 3
    };

    return shapeFromVerticesAndIndices(vertices, sizeof(vertices) / sizeof(*vertices), indices, 4, true);
}

geShape create3DCrossHair() {
    geVertex vertices[] = {
            {{ 0, 0, 0.0f }, {0, 1.0f, 0}, {0, 0, 0}},
            {{ 1, 0, 0.0f }, {0, 1.0f, 0}, {0, 1, 0}},
            {{ 0, 0, 0.0f }, {0, 1.0f, 0}, {0, 0, 1}},
            {{ 0, 1, 0.0f }, {0, 1.0f, 0}, {0, 1, 1}},
            {{ 0, 0, 0.0f }, {0, 1.0f, 0}, {0, 0, 2}},
            {{ 0, 0, 1.0f }, {0, 1.0f, 0}, {0, 1, 2}},
    };
    GLuint indices[] = {
            0, 1, 2, 3, 4, 5
    };
    return shapeFromVerticesAndIndices(vertices, sizeof(vertices) / sizeof(*vertices), indices, 6, true);
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
            long long up = (long long)((sdnoise2(oX, oZ, NULL, NULL) + 1) * 2.5f) + (long long)(world->sizeY * 0.5f);
            for (oY = 0; oY < world->sizeY; oY++) {
                if (world->map[oX][oZ][oY] == 0) {
                    continue;
                }
                float textureIndex = 1;
                if (up-- < 0) {
                    textureIndex = 2;
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

                    vertexWorld->texCoords.z = textureIndex;
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

void geWorldGenerateCulledPlanes(geWorld* world) {
    TIME_START;
    size_t oX, oY, oZ, k, l;
    size_t currentBlockIndex = 0;

    for (k = 0; k < 6; k++) {
        for (l = 0; l < world->numPlanes[k]; l++) {
            free(world->planes[k][l].vertices);
            free(world->planes[k][l].indices);
        }
        free(world->planes[k]);
        world->numPlanes[k] = 0;
    }

    world->planes[0] = calloc(world->sizeZ, sizeof(gePlane));
    world->planes[1] = calloc(world->sizeZ, sizeof(gePlane));
    world->planes[2] = calloc(world->sizeX, sizeof(gePlane));
    world->planes[3] = calloc(world->sizeX, sizeof(gePlane));
    world->planes[4] = calloc(world->sizeY, sizeof(gePlane));
    world->planes[5] = calloc(world->sizeY, sizeof(gePlane));

    for (oX = 0; oX < world->sizeX; oX++) {
        for (oZ = 0; oZ < world->sizeZ; oZ++) {
            for (oY = 0; oY < world->sizeY; oY++) {
                if (world->map[oX][oZ][oY] == 0) {
                    continue;
                }

                bool isAdjacent[] = {
                        oZ + 1 < world->sizeZ && world->map[oX][oZ + 1][oY] != 0,
                        oZ != 0 && world->map[oX][oZ - 1][oY] != 0,

                        oX != 0 && world->map[oX - 1][oZ][oY] != 0,
                        oX + 1 < world->sizeX && world->map[oX + 1][oZ][oY] != 0,

                        oY + 1 < world->sizeY && world->map[oX][oZ][oY + 1] != 0,
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
                                              world->planes[k], world->numPlanes + k);
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
            gePlane* plane = &world->planes[k][l];
#ifdef DEBUG_GREEDY
            size_t numIndicesBefore = plane->numIndices, numVerticesBefore = plane->numVertices;
#endif
            gePlaneCompressWithGreedy(plane);
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

    realloc(world->shape.vertices, world->shape.numVertices * sizeof(geVertex));
    realloc(world->shape.indices, world->shape.numIndices * sizeof(GLuint));
    TIME_END("converting to shape");
    printf("Total vertices and indices: %llu, %llu\n\n", world->shape.numVertices, world->shape.numIndices);
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
    if (realloc(plane->vertices, newNumVertices * sizeof(geVertex)) == NULL || newNumVertices == 0) {
        fprintf(stderr, "Failed to reallocate vertices\n");
    }
    if (realloc(plane->indices, newNumIndices * sizeof(GLuint)) == NULL || newNumIndices == 0) {
        fprintf(stderr, "Failed to reallocate indices\n");
    }

    plane->numVertices = newNumVertices;
    plane->numIndices = newNumIndices;

    free(hashed);
}