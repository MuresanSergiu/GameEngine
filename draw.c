//
// Created by Afterwind on 9/19/2017.
//

#include <GL/glew.h>
#include "draw.h"
#include "utils.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include <math.h>
#include <mem.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_image.h>

kmVec3 lightPoint = {0, 200, 0};

/* INTERNAL FUNCTIONS */

void initObjects() {
    // Initialize all objects
    memset(objects, 0, sizeof(objects));

    int i;
    for (i = 0; i < 256; i++) {
        objects[i].shape = shapes + GE_CUBE;
        objects[i].texture = tex[0];
        objects[i].size.x = objects[i].size.y = objects[i].size.z = 0.5f;
        objects[i].pos.x = 1.4f * (i % 16);
        objects[i].pos.y = 1.4f * (i / 16);
        objects[i].pos.z = 10;
        objects[i].rotation.y = 180;
    }

    for (i = 256; i < 512; i++) {
        objects[i].shape = shapes + GE_TERRAIN_TRIG;
        objects[i].texture = tex[2];
        objects[i].size.x = objects[i].size.y = objects[i].size.z = 1;
        objects[i].pos.x = (i - 256) % 16;
        objects[i].pos.y = 0.1f;
        objects[i].pos.z = (i - 256) / 16;
    }

    for (i = 512; i < 612; i++) {
        objects[i].shape = shapes + GE_LINE;
        objects[i].texture = tex[1];
        objects[i].size.x = objects[i].size.y = objects[i].size.z = 100;
        objects[i].pos.z = i - 562;
    }

    for (i = 612; i < 712; i++) {
        objects[i].shape = shapes + GE_LINE;
        objects[i].texture = tex[1];
        objects[i].size.x = objects[i].size.y = objects[i].size.z = 100;
        objects[i].pos.x = i - 662;
        objects[i].rotation.y = 90;
    }

    objects[712].shape = shapes + GE_CUBE;
    objects[712].texture = tex[4];
    objects[712].size.x = objects[712].size.y = objects[712].size.z = 30;
    objects[712].pos = lightPoint;

    objects[713].shape = shapes + GE_TETRAHEDRON;
    objects[713].texture = tex[0];
    objects[713].size.x = objects[713].size.y = objects[713].size.z = 2;
    objects[713].pos.y = 2.5f;
    objects[713].pos.z = 3;

    objects[714].shape = shapes + GE_CYLINDER;
    objects[714].texture = tex[0];
    objects[714].size.x = objects[714].size.y = objects[714].size.z = 2;
    objects[714].pos.x = 3;
    objects[714].pos.y = 2.5f;
    objects[714].pos.z = 3;

    objects[715].shape = shapes + GE_CUBE_INVERTED;
    objects[715].texture = tex[5];
    objects[715].size.x = objects[715].size.y = objects[715].size.z = 500;
    objects[715].pos.x = 0;
    objects[715].pos.y = -10;
    objects[715].pos.z = 0;

    objects[716].shape = shapes + GE_TERRAIN_NOISE;
    objects[716].texture = tex[2];
    objects[716].size.x  = objects[716].size.z = 100;
    objects[716].size.y = 100;
    objects[716].pos.x = 0;
    objects[716].pos.y = -0.3f;
    objects[716].pos.z = 0;

    objects[717].shape = shapes + GE_NORMALS;
    objects[717].texture = tex[3];
    objects[717].size.x = objects[717].size.y = objects[717].size.z = 4;
    objects[717].pos.x = 0;
    objects[717].pos.y = 0;
    objects[717].pos.z = -2;

    for (i = 718; i <= 720; i++) {
        objects[i].shape = shapes + GE_SQUARE;
        objects[i].texture = tex[8];
        objects[i].size.x = 0.5f;
        objects[i].size.y = 0.2f;
        objects[i].size.z = 1;
        objects[i].pos.x = 0;
        objects[i].pos.y = 0.3f * (i - 718);
        objects[i].pos.z = 0;
        objects[i].rotation.y = 180;
    }

    objects[721].shape = shapes + GE_SQUARE;
    objects[721].texture = tex[10];
    objects[721].size.x = objects[721].size.y = 4;
    objects[721].size.z = 1;
    objects[721].pos.y = 2;
    objects[721].pos.x = -2;
    objects[721].pos.z = 5;
    objects[721].rotation.y = 0;
//    objects[721].rotation.z = 180;

    srand(0);
    for (i = 722; i < 772; i++) {
        objects[i].shape = shapes + GE_LINE;
        objects[i].texture = tex[1];
        objects[i].size.x = 2;
        objects[i].size.y = 2;
        objects[i].size.z = 2;

        float x1 = ((float)rand()/(float)(RAND_MAX)), x2 = ((float)rand()/(float)(RAND_MAX));
        float v = 1 - fabs(4 * x1 - 2);
        float sinT = copysign(0.5 * (v - sqrtf(2 - v * v)), x1 - 0.5);
        float r = fmin(sinT, 0) + x2 * (sqrtf(1 - sinT * sinT) + fabs(sinT));

        float a = (3 * PI / 2) - asin(sinT);
        float sinA = sinf(a);
        float cosA = cosf(a);

        objects[i].rotation.y = 180.0f * asinf(sinT) / PI;
        objects[i].pos.x = 0;
        objects[i].pos.y = 0; //(i - 722) / 50.0f;
        objects[i].pos.z = r * (cosA * cosA - r * sinA * cosA) / sinA;
    }

    objects[772].pos.x = 0;
    objects[772].pos.y = -0.2f;
    objects[772].pos.z = 0;
    objects[772].size.x = 1;
    objects[772].size.y = 1;
    objects[772].size.z = 1;
    objects[772].texture = tex[4];
    objects[772].shape = shapes + GE_SQUARE;
    objects[772].rotation.x = 90;

    objects[773].pos.x = -3;
    objects[773].pos.y = 1;
    objects[773].pos.z = -4;
    objects[773].size.x = objects[773].size.y = objects[773].size.z = 1;
    objects[773].rotation.y = 60;
    objects[773].texture = tex[0];
    objects[773].shape = shapes + GE_VERTEX_WORLD_CULLED;
}

/* EXTERNAL FUNCTIONS */

void initScene() {
    glUseProgram(programs[GE_PROGRAM_MAIN]);
    glGenVertexArrays(INDEX_NUM, vaos);
    glGenFramebuffers(2, fbos);
    glGenBuffers(2, vbos);

    // Initialize shapes
    initShapes();
    unsigned long long vertexBufferSize = 0;
    unsigned long long indexBufferSize = 0;
    for (int i = 0; i < INDEX_NUM; i++) {
        vertexBufferSize += shapes[i].numVertices * sizeof(geVertex);
        indexBufferSize += shapes[i].numIndices * sizeof(GLuint);
        if (i == 0) {
            shapes[i].offsetBytesVertex = 0;
            shapes[i].offsetBytesIndex = 0;
        } else {
            shapes[i].offsetBytesVertex = shapes[i - 1].offsetBytesVertex + shapes[i - 1].numVertices * sizeof(geVertex);
            shapes[i].offsetBytesIndex = shapes[i - 1].offsetBytesIndex + shapes[i - 1].numIndices * sizeof(GLuint);
        }
        shapes[i].vao = vaos[i];
    }

    // Initialize buffers
    glBindVertexArray(vaos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, NULL, GL_STATIC_DRAW);

    // Buffer all shapes
    for (int i = 0; i < INDEX_NUM; i++) {
        geShape* shape = shapes + i;
        glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
        glBufferSubData(GL_ARRAY_BUFFER, shape->offsetBytesVertex, sizeof(geVertex) * shape->numVertices, shape->vertices);
        if (shape->numIndices != 0) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[1]);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, shape->offsetBytesIndex, sizeof(GLuint) * shape->numIndices, shape->indices);
        }

        glBindVertexArray(shape->vao);
        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(geVertex), (const void *) shape->offsetBytesVertex);
        // Normal attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(geVertex), (const void *) (shape->offsetBytesVertex + (sizeof(kmVec3))));
        // Texture coords attribute
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(geVertex), (const void *) (shape->offsetBytesVertex + (sizeof(kmVec3) + sizeof(kmVec3))));
    }

    // Initialize textures
    unsigned char pixelsRed[] = { 153, 0, 0 };
    unsigned char pixelsBlue[] = { 0, 0, 140 };
    unsigned char pixelsWhite[] = { 255, 255, 255 };
    unsigned char pixelsYellow[] = { 255, 200, 0 };
    unsigned char pixelsGrayTransparent[] = { 128, 128, 128, 128 };

    SDL_Surface* grass = getTexture("../res/grass.jpg");
    unsigned char* pixelsGrass = cubifyTexture(grass->pixels, grass->w, grass->h);;
    loadTextureRaw(tex + 0, pixelsGrass, grass->w * 4, grass->h * 3, GL_RGB);
    SDL_FreeSurface(grass);
    free(pixelsGrass);

    loadTextureRaw(tex + 1, pixelsRed, 1, 1, GL_RGB);
    loadTextureRaw(tex + 2, pixelsBlue, 1, 1, GL_RGB);
    loadTextureRaw(tex + 3, pixelsWhite, 1, 1, GL_RGB);
    loadTextureRaw(tex + 4, pixelsYellow, 1, 1, GL_RGB);
    loadTextureCubeMap(tex + 5, "../res/sky4.png");
    loadTexture(tex + 6, "../res/grass.jpg");
    loadTextureRaw(tex + 7, pixelsGrayTransparent, 1, 1, GL_RGBA);
    loadTexture(tex + 8, "../res/button.png");

    glGenTextures(1, tex + 9);
    glBindTexture(GL_TEXTURE_2D, tex[9]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, tex + 10);
    glBindTexture(GL_TEXTURE_2D, tex[10]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 16384, 16384, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    initObjects();

//    glGenRenderbuffers(1, rbos);
//    glBindRenderbuffer(GL_RENDERBUFFER, rbos[0]);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, 1, 1);
//    glFramebufferRenderbuffer()

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("%s\n", gluErrorString(err));
    }
}

void update() {
    cameraUpdate(&camera);

    // Update lights
    kmMat4 rot;
    kmMat4RotationZ(&rot, 0.004f * PI / 180.0f);
    kmVec3MultiplyMat4(&lightPoint, &lightPoint, &rot);
    objects[712].pos = lightPoint;
    objects[712].rotation.z += 0.004f;
    glUniform3fv(103, 1, (const GLfloat *) &lightPoint);

    float skyDim = cosf(objects[712].rotation.z * PI / 180.0f);
    if (skyDim <= 0.1f) {
        skyDim = 0.1f;
    }
    glUniform1f(106, skyDim);

    kmVec3 lightDirection = {0, 0, 0};
    kmVec3Subtract(&lightDirection, &lightDirection, &lightPoint);
    kmVec3Normalize(&lightDirection, &lightDirection);
    glUniform3fv(105, 1, (const GLfloat *) &lightDirection);

    kmMat4Identity(&rot);
    glUniformMatrix4fv(3, 1, GL_FALSE, rot.mat);

    for (int i = 0; i < (int) sizeof(objects) / sizeof(geObject); i++) {
        geObject* obj = objects + i;
        if (i < 256 && i != 16 || i == 714) {
            obj->rotation.y += 0.01f;
            obj->rotation.z += 0.02f;
        }
        if (i == 715) {
            obj->rotation.y += 0.001f;
        }
    }
}

void drawScene() {
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int i = 0; i < (int) sizeof(objects) / sizeof(geObject); i++) {
        geObject* obj = objects + i;
        if (i >= 256 && i < 712 || i == 717) {
            continue;
        }

        if (i == 712 || i == 715) {// If it's the light cube or the sky
            glUniform1i(10, true);
            glUniform4f(101, 1, 1, 1, 1);

        } else {
            glUniform1i(10, false);
            glUniform4f(101, 0.1f, 0.1f, 0.1f, 1);
        }

        if (i == 715) { // If it's the sky
            glUniform1i(11, true);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, obj->texture);
        } else {
            glUniform1i(11, false);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, obj->texture);
        }

        if (i >= 512 && i <= 712 || i == 721) { // Set ambient light
            glUniform4f(101, 1, 1, 1, 1);
        } else {
            glUniform4f(101, 0.3f, 0.3f, 0.3f, 1);
        }

        if (i >= 718 && i <= 720) { // If it's the GUI
            if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
                continue;
            }
            glUniform1i(12, true);
            continue;
        } else {
            glUniform1i(12, false);
        }

        kmMat4 rot, scale, translation;
        if (i == 721) {
            kmMat4Scaling(&scale, 0.5f, 0.5f, 1);
            kmMat4Translation(&translation, 0.5f, 0.5f, 0);
            kmMat4Multiply(&rot, &scale, &translation);
            glUniformMatrix4fv(3, 1, GL_FALSE, rot.mat);
        } else {
            kmMat4Identity(&rot);
            glUniformMatrix4fv(3, 1, GL_FALSE, rot.mat);
        }

        drawObject(obj);
    }
}

void drawObject(geObject* obj) {
    // UPDATE MODEL UNIFORM
    kmMat4 rotX, rotY, rotZ;
    kmMat4 scale;
    kmMat4 translation;
    kmMat4 model;

    kmMat4RotationX(&rotX, obj->rotation.x * (PI / 180));
    kmMat4RotationY(&rotY, obj->rotation.y * (PI / 180));
    kmMat4RotationZ(&rotZ, obj->rotation.z * (PI / 180));
    kmMat4Scaling(&scale, obj->size.x, obj->size.y, obj->size.z);
    kmMat4Translation(&translation, obj->pos.x, obj->pos.y, obj->pos.z);

    kmMat4Identity(&model);
    kmMat4Multiply(&model, &model, &translation);
    kmMat4Multiply(&model, &model, &scale);
    kmMat4Multiply(&model, &model, &rotX);
    kmMat4Multiply(&model, &model, &rotY);
    kmMat4Multiply(&model, &model, &rotZ);

    glUniformMatrix4fv(0, 1, GL_FALSE, model.mat);

    // DRAW THE OBJECT
    glBindVertexArray(obj->shape->vao);
    if (obj->shape->numIndices == 0) {
        glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
        glDrawArrays(GL_TRIANGLES, (GLint) obj->shape->offsetBytesVertex / sizeof(geVertex), (GLsizei) (obj->shape->numVertices));
    } else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[1]);
        GLenum shape = obj->shape->numVertices == 2 ? GL_LINES : GL_TRIANGLES;
//        if (obj->shape == shapes + GE_VERTEX_WORLD_LESS_DUMB) {
//            glDrawElements(GL_TRIANGLE_STRIP, (GLsizei) obj->shape->numIndices, GL_UNSIGNED_INT, (const void*) obj->shape->offsetBytesIndex);
//        } else {
            glDrawElements(shape, (GLsizei) obj->shape->numIndices, GL_UNSIGNED_INT, (const void*) obj->shape->offsetBytesIndex);
//        }
    }
    glBindVertexArray(0);
}

void clearScene() {
    for (int i = 0; i < INDEX_NUM; i++) {
        glDeleteFramebuffers(1, fbos);
        glDeleteVertexArrays(INDEX_NUM, vaos);
        glDeleteBuffers(2, vbos);
        glDeleteTextures(sizeof(tex) / sizeof(*tex), tex);
        free(shapes[i].vertices);
        free(shapes[i].indices);
        glDeleteProgram(programs[GE_PROGRAM_MAIN]);
        glDeleteProgram(programs[GE_PROGRAM_TEXTURE]);
    }
}