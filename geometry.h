//
// Created by Afterwind on 9/25/2017.
//

#ifndef GAMEENGINE_GEOMETRY_H
#define GAMEENGINE_GEOMETRY_H

#include <stdbool.h>
#include "types.h"

typedef enum {
    GE_CUBE = 0,
    GE_CUBE_INVERTED,
    GE_SQUARE,
    GE_LINE,
    GE_CIRCLE,
    GE_CYLINDER,
    GE_TETRAHEDRON,
    GE_TERRAIN_TRIG,
    GE_NORMALS,
    GE_TERRAIN_NOISE,

    // Number of shapes
    INDEX_NUM
} SHAPES;

void initShapes();
geShape createCube(bool withTextureAndNormals);
geShape createSquare(bool withIndices);
geShape createLine();
geShape createCircle(int tess);
geShape createCylinder(int tess);
geShape createTetrahedron();
geShape createTrigTerrain(int tess);
geShape createLineNormals(geShape* shape);
geShape createNoiseTerrain(int tess);

geShape shapes[INDEX_NUM];

#endif //GAMEENGINE_GEOMETRY_H
