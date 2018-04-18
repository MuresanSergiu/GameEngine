//
// Created by Afterwind on 9/24/2017.
//

#ifndef GAMEENGINE_CAMERA_H
#define GAMEENGINE_CAMERA_H

#include "types.h"

geCamera cameraMain;

void geCameraUpdate(geCamera* camera);
kmVec3 geCameraRaycast(geCamera* camera, geWorld* world);
//kmVec3 cameraRight();


#endif //GAMEENGINE_CAMERA_H
