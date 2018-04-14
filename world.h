#ifndef GAMEENGINE_WORLD_H
#define GAMEENGINE_WORLD_H

#include "types.h"

geWorld worldMain;

void geWorldInit(size_t sizeX, size_t sizeY, size_t sizeZ);
void geWorldGenerate(size_t baseHeight, size_t heightOffsetIntesnsity);
void geWorldDestroy();
kmVec3 geWorldFind(kmVec3* v);
void geWorldRemoveBlock(kmVec3* v);

#endif //GAMEENGINE_WORLD_H
