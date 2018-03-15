//
// Created by Afterwind on 9/24/2017.
//

#include "camera.h"
#include "utils.h"
#include "shader.h"
#include <kazmath/vec4.h>

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