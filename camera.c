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

void cameraUpdate(geCamera* camera) {
    // Perspective and view matrix construction
    kmMat4 projection;
    kmMat4 view;
    kmVec3 target;

    kmVec3Add(&target, &camera->pos, &camera->direction);
    kmMat4PerspectiveProjection(&projection, 70.0f, camera->aspectRatio, 0.01f, 500);
    kmMat4LookAt(&view, &camera->pos, &target, &camera->up);
    glUniformMatrix4fv(_U(view), 1, GL_FALSE, view.mat);
    glUniformMatrix4fv(_U(projection), 1, GL_FALSE, projection.mat);
    glUniform3fv(_U(viewPosition), 1, (const GLfloat *) &camera->pos);
    glUniform3fv(_U(pl) + 1, 1, (const GLfloat *) &camera->pos);
}

kmVec3 raycast() {
    size_t i;
    kmVec3 pos = camera.pos;
    kmVec3 dir;
    kmVec3Scale(&dir, &camera.direction, 0.01f);
    long long blockType;
    for (i = 0; i < 1500; i++) {
        blockType = findInWorld(&pos);
        if (blockType != -1 && blockType != 0) {
            return pos;
        }
        kmVec3Add(&pos, &pos, &dir);
    }
    pos.x = -1;
    pos.y = -1;
    pos.z = -1;
    return pos;
}