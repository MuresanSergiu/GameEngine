//
// Created by Afterwind on 10/10/2017.
//

#include <GL/glew.h>
#include <kazmath/kazmath.h>
#include <stdio.h>
#include "framebuffer.h"
#include "shader.h"
#include "types.h"
#include "utils.h"
#include "texture.h"
#include "draw.h"
#include "camera.h"

void renderMirror() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbos[0]);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex[9], 0);
    GLenum lst[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, lst);
    GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (err != GL_FRAMEBUFFER_COMPLETE) {
        printf("%s: %s\n", "Error at framebuffering", gluErrorString(err));
    }
    glViewport(0, 0, 512, 512);

    geObject* mirror = objects + 721;
    kmMat4 rot;

//    kmMat4RotationAxisAngle(&rot, &mirror->shape->vertices[0].normal, PI);
    kmVec3Subtract(&cameraMirror.direction, &mirror->pos, &camera.pos);
//    kmMat4RotationAxisAngle(&rot, &mirror->shape->vertices[0].normal, PI);
//    kmVec3MultiplyMat4(&cameraMirror.direction, &cameraMirror.direction, &rot);
    kmVec3Normalize(&cameraMirror.direction, &cameraMirror.direction);
//    kmVec3MultiplyMat4(&cameraMirror.direction, &camera.direction, &rot);
    kmVec3Scale(&cameraMirror.direction, &cameraMirror.direction, -1);

    cameraMirror.aspectRatio = 1;
    cameraMirror.up.x = 0;
    cameraMirror.up.y = 1;
    cameraMirror.up.z = 0;

    cameraMirror.pos = mirror->pos;
    cameraUpdate(&cameraMirror);

//    kmMat4RotationZ(&rot, 0.01f * PI / 180.0f);
//    kmVec3MultiplyMat4(&lightPoint2, &lightPoint2, &rot);
//    objects[712].pos = lightPoint2;
//    objects[712].rotation.z += 0.01f;
//    glUniform3fv(_U(pl), 1, (const GLfloat *) &lightPoint2);

    kmMat4 scale;
    kmMat4 translation;

    kmMat4Identity(&rot);
    kmMat4Scaling(&scale, 1.0f / 2, 1.0f / 2, 0);
    kmMat4Translation(&translation, 1.0f / 2, 1.0f / 2, 0);
    kmMat4Multiply(&rot, &rot, &translation);
    kmMat4Multiply(&rot, &rot, &scale);
    glUniformMatrix4fv(_U(scaleBias), 1, GL_FALSE, rot.mat);

    drawScene();
}

void renderShadowMap() {
    glUseProgram(programs[GE_PROGRAM_SHADOW]);
    glBindFramebuffer(GL_FRAMEBUFFER, fbos[1]);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex[10], 0);
    glDrawBuffer(GL_NONE);
    GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (err != GL_FRAMEBUFFER_COMPLETE) {
        printf("%s: %s\n", "Error at framebuffering", gluErrorString(err));
    }
    glViewport(0, 0, 16384, 16384);

//    kmVec3 eyePos = objects[712].pos;

    if (xOffset == 0) xOffset = 1.0f;
//    kmVec3 eyePos = { sinf(x * PI), cosf(x * PI), 0 };
    xOffset += 0.001f;
//    printf("%f\n", x);
//    kmVec3 eyePos = { 0.000001f, 1.000001f, 0 };
    kmVec3 eyePos;
    kmVec3 center = { 0, 0, 0 };
    kmVec3 up = { 0, 1, 0 };
    kmVec3Subtract(&eyePos, &objects[712].pos, &center);
    kmVec3Normalize(&eyePos, &eyePos);
    kmVec3Scale(&eyePos, &eyePos, 10);

    kmMat4 projection;
    kmMat4 view;
    kmMat4OrthographicProjection(&projection, -120, 120, -120, 120, -10, 30);
    kmMat4LookAt(&view, &eyePos, &center, &up);

    kmMat4 bias, biasScale, biasTranslate;
    kmMat4Scaling(&biasScale, 0.5f, 0.5f, 1);
    kmMat4Translation(&biasTranslate, 0.5f, 0.5f, 0);
    kmMat4Multiply(&bias, &biasTranslate, &biasScale);

    glUseProgram(programs[GE_PROGRAM_MAIN]);
    glUniformMatrix4fv(_U(shadowView), 1, GL_FALSE, view.mat);
    glUniformMatrix4fv(_U(shadowProjection), 1, GL_FALSE, projection.mat);
    glUniformMatrix4fv(_U(shadowScaleBias), 1, GL_FALSE, bias.mat);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex[10]);
    glUniform1i(_U(shadow), 2);

    glUseProgram(programs[GE_PROGRAM_SHADOW]);
    glUniformMatrix4fv(_U(view), 1, GL_FALSE, view.mat);
    glUniformMatrix4fv(_U(projection), 1, GL_FALSE, projection.mat);

    drawScene();
}