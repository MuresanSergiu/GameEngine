//
// Created by Afterwind on 9/24/2017.
//

#ifndef GAMEENGINE_TEXTURE_H
#define GAMEENGINE_TEXTURE_H

#include <GL/glew.h>
#include <SDL2/SDL_image.h>

GLuint tex[50];

SDL_Surface* getTexture(const char* path);
void loadTexture(GLuint* tOut, const char* path);
void loadTextureRaw(GLuint* tOut, void* pixels, int width, int height, GLenum internalFormat);
void loadTextureCubeMap(GLuint* tOut, const char* path);
unsigned char* cubifyTexture(unsigned char * pixels, int width, int height);
void flatifyTexture(unsigned char** pOut, unsigned char * pixels, int width, int height, int pixelSize);

#endif //GAMEENGINE_TEXTURE_H
