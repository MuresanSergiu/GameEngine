#ifndef GAMEENGINE_WORLD_H
#define GAMEENGINE_WORLD_H

#include "types.h"

geWorld world;

void initWorld(size_t sizeX, size_t sizeY, size_t sizeZ);
void generateWorld(size_t heightOffsetIntesnsity);
void destroyWorld();
long long findInWorld(kmVec3* v);

#endif //GAMEENGINE_WORLD_H
