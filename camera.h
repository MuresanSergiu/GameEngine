//
// Created by Afterwind on 9/24/2017.
//

#ifndef GAMEENGINE_CAMERA_H
#define GAMEENGINE_CAMERA_H

#include "types.h"

geCamera camera;

void cameraUpdate(geCamera* camera);
kmVec3 cameraRaycast();
//kmVec3 cameraRight();


#endif //GAMEENGINE_CAMERA_H
