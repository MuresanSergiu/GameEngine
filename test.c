#include <stdio.h>
#include "test.h"
#include "utils.h"

void testPlanesIntegrity(geWorld* world) {
    size_t k, l;
    geVertex* v;
    for (k = 0; k < 6; k++) {
        for (l = 0; l < world->numPlanes[k]; l++) {
            gePlane* planeU = &world->planesUncompressed[k][l];
            gePlane* planeC = &world->planes[k][l];
            float planeCoord = planeCoordinate(planeU->vertices + 0);
            for (v = planeU->vertices + 4; v < planeU->vertices + planeU->numVertices; v += 4) {
                if (planeCoordinate(v) != planeCoord) {
                    fprintf(stdout, "%f uncompressed plane coord invalid\n", planeCoord);
                }
            }
            for (v = planeC->vertices + 4; v < planeC->vertices + planeC->numVertices; v += 4) {
                if (planeCoordinate(v) != planeCoord) {
                    fprintf(stdout, "%f compressed plane coord invalid\n", planeCoord);
                }
            }
        }
    }
}