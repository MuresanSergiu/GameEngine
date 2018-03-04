//
// Created by Afterwind on 9/24/2017.
//

#include <kazmath/vec3.h>
#include "texture.h"

/* EXTERNAL FUNCTIONS */

SDL_Surface* getTexture(const char* path) {
    SDL_Surface * data = IMG_Load(path);
    if (!data) {
        printf("%s\n%s\n", "Man, that's an error loading the texture!", SDL_GetError());
        return NULL;
    }
    return data;
}

void loadTexture(GLuint* tOut, const char* path) {
    SDL_Surface * data = getTexture(path);
    if (!data) return;
    //printf("%i %i\n", data->h, data->w);
    GLenum format;
    if (data->format->BytesPerPixel == 3) {
        format = GL_RGB;
    } else {
        format = GL_RGBA;
    }
    loadTextureRaw(tOut, data->pixels, data->w, data->h, format);
    SDL_FreeSurface(data);
}

void loadTextureRaw(GLuint* tOut, void* pixels, int width, int height, GLenum internalFormat) {
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, tOut);
    glBindTexture(GL_TEXTURE_2D, *tOut);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, internalFormat, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void loadTextureCubeMap(GLuint* tOut, const char* path) {
    GLenum err;

    err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("Error before: %s\n", gluErrorString(err));
    }

    SDL_Surface * data = getTexture(path);
    if (!data) return;
    //printf("%i %i\n", data->h, data->w);
    GLenum format, sizedFormat;
    if (data->format->BytesPerPixel == 3) {
        format = GL_RGB;
        sizedFormat = GL_RGB8;
    } else {
        format = GL_RGBA;
        sizedFormat = GL_RGBA8;
    }
    unsigned char* pixelsSky[6];
    flatifyTexture(pixelsSky, data->pixels, data->w, data->h, data->format->BytesPerPixel);

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, tOut);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *tOut);
//    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, sizedFormat, (GLsizei) data->w / 4, (GLsizei) data->h / 3);

    // Invalid operation. Need to do glTexSubImage2D
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format, (GLsizei) data->w / 4, (GLsizei) data->h / 3, 0, format, GL_UNSIGNED_BYTE, pixelsSky[0]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format, (GLsizei) data->w / 4, (GLsizei) data->h / 3, 0, format, GL_UNSIGNED_BYTE, pixelsSky[1]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format, (GLsizei) data->w / 4, (GLsizei) data->h / 3, 0, format, GL_UNSIGNED_BYTE, pixelsSky[2]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format, (GLsizei) data->w / 4, (GLsizei) data->h / 3, 0, format, GL_UNSIGNED_BYTE, pixelsSky[3]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format, (GLsizei) data->w / 4, (GLsizei) data->h / 3, 0, format, GL_UNSIGNED_BYTE, pixelsSky[4]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format, (GLsizei) data->w / 4, (GLsizei) data->h / 3, 0, format, GL_UNSIGNED_BYTE, pixelsSky[5]);
    err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("Error after: %s\n", gluErrorString(err));
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
    glUniform1i(111, 1);

    for (int i = 0; i < 6; i++) {
        free(pixelsSky[i]);
    }
    SDL_FreeSurface(data);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

}

unsigned char* cubifyTexture(unsigned char * pixels, int width, int height) {
    unsigned char* pOut = calloc((size_t) ((12 * width * height) * 3), sizeof(*pixels));
    unsigned char* pOutLoc = pOut;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == 1 || ((i == 0 || i == 2) && j == 1)) { // If not inside an empty quadrant
                for (int h = 0; h < height; h++) {
                    memcpy(pOutLoc + 12 * width * h, pixels + 3 * width * h, 3 * width * sizeof(*pixels));
                }
            } else { // Otherwise fill with 0s
                for (int h = 0; h < height; h++) {
                    memset(pOutLoc + 12 * width * h, 0, 3 * width * sizeof(*pixels));
                }
            }
            pOutLoc += 3 * width;
        }
        pOutLoc += 12 * width * (height - 1);
    }
    return pOut;
}

void flatifyTexture(unsigned char** pOut, unsigned char * pixels, int width, int height, int pixelSize) {
    int k = 0;
    unsigned char* pixelsLoc = pixels;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == 1 || ((i == 0 || i == 2) && j == 1)) { // If not inside an empty quadrant
                pOut[k] = calloc((size_t) ((width / 4) * (height / 3) * pixelSize), sizeof(*pixels));
                for (int h = 0; h < height / 3; h++) {
                    memcpy(pOut[k] + (width / 4) * h * pixelSize, pixelsLoc + width * h * pixelSize, sizeof(*pixels) * (width / 4) * pixelSize);
                }
                k++;
            }
            pixelsLoc += (width / 4) * pixelSize;
        }
        pixelsLoc += pixelSize * width * (height / 3 - 1);
    }
}