//
// Created by Afterwind on 9/25/2017.
//

#ifndef GAMEENGINE_GEOMETRY_H
#define GAMEENGINE_GEOMETRY_H

#include <stdbool.h>
#include "types.h"

typedef enum {
    GE_CUBE = 0,
    GE_CUBE_BORDER,
    GE_CUBE_INVERTED,
    GE_SQUARE,
    GE_LINE,
    GE_CIRCLE,
    GE_CYLINDER,
    GE_TETRAHEDRON,
    GE_TERRAIN_TRIG,
    GE_NORMALS,
    GE_TERRAIN_NOISE,
    GE_2D_CROSSHAIR,
    GE_3D_CROSSHAIR,

    // Number of shapes
    INDEX_NUM
} SHAPES;

void initShapes();
geShape createCube(bool inverted);
geShape createCubeBorder();
geShape createSquare(bool withIndices);
geShape createLine();
geShape createCircle(unsigned long long tess);
geShape createCylinder(unsigned long long tess);
geShape createTetrahedron();
geShape createTrigTerrain(unsigned long long tess);
geShape createLineNormals(geShape* shape);
geShape createNoiseTerrain(unsigned long long tess);
geShape create3DCrossHair();
geShape create2DCrossHair();

geShape shapes[INDEX_NUM];

#endif //GAMEENGINE_GEOMETRY_H
