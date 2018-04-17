//
// Created by Afterwind on 9/24/2017.
//

#ifndef GAMEENGINE_TEXTURE_H
#define GAMEENGINE_TEXTURE_H

#include <GL/glew.h>
#include <SDL2/SDL_image.h>

typedef enum {
    GE_TEXTURE_GRASS_CUBIC,
    GE_TEXTURE_RED,
    GE_TEXTURE_BLUE,
    GE_TEXTURE_WHITE,
    GE_TEXTURE_YELLOW,
    GE_TEXTURE_SKY,
    GE_TEXTURE_GRASS,
    GE_TEXTURE_GRAY_TRANSPARENT,
    GE_TEXTURE_BUTTON,
    GE_TEXTURE_SHADOW_MAP_512,
    GE_TEXTURE_SHADOW_MAP_16384,
    GE_TEXTURE_COBBLE,
    GE_TEXTURE_COBBLE_2,
    GE_TEXTURE_CONTAINER,
    GE_TEXTURE_ATLAS,

    GE_NUM_TEXTURES
} TEXTURE;

GLuint tex[GE_NUM_TEXTURES];

SDL_Surface* getTexture(const char* path);
void loadTexture(GLuint* tOut, const char* path);
void loadTexture3D(GLuint* tOut, const char* path);
void loadTextureRaw(GLuint* tOut, void* pixels, int width, int height, GLenum internalFormat);
void loadTextureCubeMap(GLuint* tOut, const char* path);
unsigned char* cubifyTexture(unsigned char * pixels, int width, int height);
void flatifyTexture(unsigned char** pOut, unsigned char * pixels, int width, int height, int pixelSize);

#endif //GAMEENGINE_TEXTURE_H
