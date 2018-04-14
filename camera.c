//
// Created by Afterwind on 9/24/2017.
//

#include "camera.h"
#include "utils.h"
#include "shader.h"
#include "world.h"
#include <kazmath/vec4.h>
#include <mem.h>

/* EXTERNAL FUNCTIONS */

kmVec3 cameraDefaultDirection = {0, 0, -1};

void geCameraUpdate(geCamera* camera) {
    // Perspective and view matrix construction
    kmMat4 projection;
    kmMat4 view;
    kmVec3 target;
    kmMat4 rotX, rotY, rot;

    memcpy(&camera->direction, &cameraDefaultDirection, sizeof(kmVec3));
    kmMat4RotationY(&rotY, camera->rotation.y * PI / 180.0f);
    kmMat4RotationX(&rotX, camera->rotation.x * PI / 180.0f);
    kmMat4Multiply(&rot, &rotY, &rotX);
    kmVec3MultiplyMat4(&camera->direction, &camera->direction, &rot);
    kmVec3Add(&target, &camera->direction, &camera->pos);

    kmMat4PerspectiveProjection(&projection, 70.0f, camera->aspectRatio, 0.01f, 500);
    kmMat4LookAt(&view, &camera->pos, &target, &camera->up);
    glUniformMatrix4fv(_U(view), 1, GL_FALSE, view.mat);
    glUniformMatrix4fv(_U(projection), 1, GL_FALSE, projection.mat);
    glUniform3fv(_U(viewPosition), 1, (const GLfloat *) &camera->pos);
    glUniform3fv(_U(pl) + 1, 1, (const GLfloat *) &camera->pos);
}

kmVec3 geCameraRaycast(geCamera* camera) {
    size_t i;
    kmVec3 pos = camera->pos;
    kmVec3 dir;
    kmVec3Scale(&dir, &camera->direction, 0.01f);
    kmVec3 block = { -1, -1, -1 };
    for (i = 0; i < 1500; i++) {
        block = geWorldFind(&pos);
        if (block.x != -1 && block.y != -1 && block.z != -1) {
            return block;
        }
        kmVec3Add(&pos, &pos, &dir);
    }
    return block;
}