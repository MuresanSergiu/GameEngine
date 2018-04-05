//
// Created by Afterwind on 9/19/2017.
//

#include <GL/glew.h>
#include "draw.h"
#include "utils.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "world.h"
#include <math.h>
#include <mem.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_image.h>

kmVec3 lightPoint = {0, 200, 0};
geObject* sun;
geObject* sky;
geObject* shadowMap;
geObject* vertexWorldDumb;
geObject* vertexWorldCulled;
geObject* vertexWorldGreedy;
geObject* crosshair;

/* INTERNAL FUNCTIONS */

geObject* initObject() {
    geObject* obj = &objects[numObjects++];
    obj->texture = 0;
    obj->shape = 0;
    obj->pos.x = obj->pos.y = obj->pos.z = 0;
    obj->size.x = obj->size.y = obj->size.z = 1;
    obj->rotation.x = obj->rotation.y = obj->rotation.z = 0;
    obj->exemptFromView = false;
    obj->exemptFromViewProjection = false;
    obj->glTextureId = GL_TEXTURE0;
    obj->glTextureType = GL_TEXTURE_2D;
    obj->extraBrightness = 0.0f;
    return obj;
}

void initObjects() {
    // Initialize all objects
    memset(objects, 0, sizeof(objects));

    sun = initObject();
    sun->shape = shapes + GE_CUBE;
    sun->texture = tex[4];
    sun->size.x = sun->size.y = sun->size.z = 30;
    sun->pos = lightPoint;
    sun->exemptFromView = true;
    sun->extraBrightness = 1;
//    objects[712].shape = shapes + GE_CUBE;
//    objects[712].texture = tex[4];
//    objects[712].size.x = objects[712].size.y = objects[712].size.z = 30;
//    objects[712].pos = lightPoint;

    sky = initObject();
    sky->shape = shapes + GE_CUBE_INVERTED;
    sky->texture = tex[5];
    sky->size.x = sky->size.y = sky->size.z = 500;
    sky->pos.y = -10;
    sky->glTextureType = GL_TEXTURE_CUBE_MAP;
    sky->glTextureId = GL_TEXTURE1;
    sky->exemptFromView = true;
    sky->extraBrightness = 1;

    shadowMap = initObject();
    shadowMap->shape = shapes + GE_SQUARE;
    shadowMap->texture = tex[10];
    shadowMap->size.x = shadowMap->size.y = 4;
    shadowMap->pos.y = 2;
    shadowMap->pos.x = -2;
    shadowMap->pos.z = 5;

//    vertexWorldDumb = initObject();
//    vertexWorldDumb->pos.x = -3;
//    vertexWorldDumb->pos.y = 1;
//    vertexWorldDumb->pos.z = -4;
//    vertexWorldDumb->rotation.y = 60;
//    vertexWorldDumb->texture = tex[12];
//    vertexWorldDumb->shape = shapes + GE_VERTEX_WORLD_DUMB;

//    vertexWorldGreedy = initObject();
//    vertexWorldGreedy->pos.x = -12;
//    vertexWorldGreedy->pos.y = -10;
//    vertexWorldGreedy->pos.z = -10;
//    vertexWorldGreedy->rotation.y = 60;
//    vertexWorldGreedy->texture = tex[12];
//    vertexWorldGreedy->shape = shapes + GE_VERTEX_WORLD_GREEDY;

//    geObject* terrainNoise = initObject();
//    terrainNoise->pos.y = 50;
//    terrainNoise->shape = shapes + GE_TERRAIN_NOISE;
//    terrainNoise->texture = tex[2];
//    terrainNoise->size.x = terrainNoise->size.y = terrainNoise->size.z = 100;


//    crosshair = initObject();
//    crosshair->texture = tex[3];
//    crosshair->shape = shapes + GE_3D_CROSSHAIR;
//    crosshair->exemptFromView = true;
//    crosshair->extraBrightness = 1.0f;
//    crosshair->size.x = crosshair->size.y = crosshair->size.z = 0.2f;
//    crosshair->pos.z = -1;

    initWorld(50, 128, 50);
    bufferShape(&world.shape);

    world.object = initObject();
    world.object->shape = &world.shape;
    world.object->texture = tex[12];

    // <editor-fold> UNUSED USEFUL OBJECTS
//    for (i = 512; i < 612; i++) {
//        objects[i].shape = shapes + GE_LINE;
//        objects[i].texture = tex[1];
//        objects[i].size.x = objects[i].size.y = objects[i].size.z = 100;
//        objects[i].pos.z = i - 562;
//    }
//
//    for (i = 612; i < 712; i++) {
//        objects[i].shape = shapes + GE_LINE;
//        objects[i].texture = tex[1];
//        objects[i].size.x = objects[i].size.y = objects[i].size.z = 100;
//        objects[i].pos.x = i - 662;
//        objects[i].rotation.y = 90;
//    }
//    for (i = 718; i <= 720; i++) {
//        objects[i].shape = shapes + GE_SQUARE;
//        objects[i].texture = tex[8];
//        objects[i].size.x = 0.5f;
//        objects[i].size.y = 0.2f;
//        objects[i].size.z = 1;
//        objects[i].pos.x = 0;
//        objects[i].pos.y = 0.3f * (i - 718);
//        objects[i].pos.z = 0;
//        objects[i].rotation.y = 180;
//    }
//    srand(0);
//    for (i = 722; i < 772; i++) {
//        objects[i].shape = shapes + GE_LINE;
//        objects[i].texture = tex[1];
//        objects[i].size.x = 2;
//        objects[i].size.y = 2;
//        objects[i].size.z = 2;
//
//        float x1 = ((float)rand()/(float)(RAND_MAX)), x2 = ((float)rand()/(float)(RAND_MAX));
//        float v = 1 - fabsf(4 * x1 - 2);
//        float sinT = copysignf(0.5f * (v - sqrtf(2 - v * v)), x1 - 0.5f);
//        float r = fminf(sinT, 0) + x2 * (sqrtf(1 - sinT * sinT) + fabsf(sinT));
//
//        float a = (3 * PI / 2) - asinf(sinT);
//        float sinA = sinf(a);
//        float cosA = cosf(a);
//
//        objects[i].rotation.y = 180.0f * asinf(sinT) / PI;
//        objects[i].pos.x = 0;
//        objects[i].pos.y = 0; //(i - 722) / 50.0f;
//        objects[i].pos.z = r * (cosA * cosA - r * sinA * cosA) / sinA;
//    }

//    objects[772].pos.x = 0;
//    objects[772].pos.y = -0.2f;
//    objects[772].pos.z = 0;
//    objects[772].size.x = 1;
//    objects[772].size.y = 1;
//    objects[772].size.z = 1;
//    objects[772].texture = tex[4];
//    objects[772].shape = shapes + GE_SQUARE;
//    objects[772].rotation.x = 90;
    // </editor-fold>
}

/* EXTERNAL FUNCTIONS */

void bufferShape(geShape* shape) {
    glUseProgram(programs[GE_PROGRAM_MAIN]);
    glGenVertexArrays(1, &shape->vao);

    // Initialize shapes
    shape->offsetBytesVertex = currentOffsetVertex;
    shape->offsetBytesIndex = currentOffsetIndex;

    currentOffsetVertex += shape->numVertices * sizeof(geVertex);
    currentOffsetIndex += shape->numIndices * sizeof(GLuint);

    // Buffer shape
    glBindVertexArray(shape->vao);
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

void addObject(geObject* obj) {
    if (numObjects + 1 > MAX_OBJECTS) {
        fprintf(stderr, "Too many objects to add\n");
        return;
    }
    memcpy(&objects[numObjects++], obj, sizeof(geObject));
}

void addObjects(geObject* obj, size_t num) {
    if (numObjects + num > MAX_OBJECTS) {
        fprintf(stderr, "Too many objects to add\n");
        return;
    }
    memcpy(&objects[numObjects], obj, sizeof(geObject) * num);
    numObjects += num;
}

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

    currentOffsetVertex = shapes[INDEX_NUM - 1].offsetBytesVertex + shapes[INDEX_NUM - 1].numVertices * sizeof(geVertex);
    currentOffsetIndex = shapes[INDEX_NUM - 1].offsetBytesIndex + shapes[INDEX_NUM - 1].numIndices * sizeof(GLuint);

    // Add some extra space for dynamically added shapes
    vertexBufferSize += 100000 * sizeof(geVertex);
    indexBufferSize += 100000 * sizeof(GLuint);

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

    loadTexture(tex + 11, "../res/cobble.png");
    loadTexture(tex + 12, "../res/cobble2.png");

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
    sun->pos = lightPoint;
    sun->rotation.z += 0.004f;
    glUniform3fv(_U(pl[0]), 1, (const GLfloat *) &lightPoint);

    float skyDim = cosf(sun->rotation.z * PI / 180.0f);
    if (skyDim <= 0.1f) {
        skyDim = 0.1f;
    }
    glUniform1f(_U(skyDim), skyDim);

    kmVec3 lightDirection = {0, 0, 0};
    kmVec3Subtract(&lightDirection, &lightDirection, &lightPoint);
    kmVec3Normalize(&lightDirection, &lightDirection);
    glUniform3fv(_U(dl), 1, (const GLfloat *) &lightDirection);

    kmMat4Identity(&rot);
    glUniformMatrix4fv(_U(scaleBias), 1, GL_FALSE, rot.mat);

    // Update crosshair
//    memcpy(&crosshair->rotation, &camera.direction, sizeof(kmVec3));
}

void drawScene() {
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniform4f(_U(lightAmbient), 0.3f, 0.3f, 0.3f, 1);

    size_t i;
    for (i = 0; i < numObjects; i++) {
        geObject* obj = objects + i;

        glUniform1i(_U(exemptFromView), obj->exemptFromView);
        glUniform1i(_U(exemptFromViewProjection), obj->exemptFromViewProjection);
        glUniform1f(_U(extraBrightness), obj->extraBrightness);

        glActiveTexture(obj->glTextureId);
        glBindTexture(obj->glTextureType, obj->texture);
        glUniform1i(_U(useCubeMap), obj->glTextureType == GL_TEXTURE_CUBE_MAP);

        kmMat4 rot, scale, translation;
        if (i == 721) {
            kmMat4Scaling(&scale, 0.5f, 0.5f, 1);
            kmMat4Translation(&translation, 0.5f, 0.5f, 0);
            kmMat4Multiply(&rot, &scale, &translation);
            glUniformMatrix4fv(_U(scaleBias), 1, GL_FALSE, rot.mat);
        } else {
            kmMat4Identity(&rot);
            glUniformMatrix4fv(_U(scaleBias), 1, GL_FALSE, rot.mat);
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
    if (obj->shape == shapes + GE_3D_CROSSHAIR) {
        kmMat4Multiply(&model, &model, &camera.rotY);
        kmMat4Multiply(&model, &model, &camera.rotLeft);
    } else {
        kmMat4Multiply(&model, &model, &rotX);
        kmMat4Multiply(&model, &model, &rotY);
        kmMat4Multiply(&model, &model, &rotZ);
    }

    glUniformMatrix4fv(_U(model), 1, GL_FALSE, model.mat);

    // DRAW THE OBJECT
    GLenum primitive = obj->shape->numVertices == 2 || obj->shape == shapes + GE_3D_CROSSHAIR ? GL_LINES : GL_TRIANGLES;
    glBindVertexArray(obj->shape->vao);
    if (obj->shape->numIndices == 0) {
        glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
        glDrawArrays(primitive, (GLint) obj->shape->offsetBytesVertex / sizeof(geVertex), (GLsizei) (obj->shape->numVertices));
    } else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[1]);

//        if (obj->shape == shapes + GE_VERTEX_WORLD_LESS_DUMB) {
//            glDrawElements(GL_TRIANGLE_STRIP, (GLsizei) obj->shape->numIndices, GL_UNSIGNED_INT, (const void*) obj->shape->offsetBytesIndex);
//        } else {
            glDrawElements(primitive, (GLsizei) obj->shape->numIndices, GL_UNSIGNED_INT, (const void*) obj->shape->offsetBytesIndex);
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