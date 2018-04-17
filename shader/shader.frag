#version 450 core

layout (location = 10) uniform bool exemptFromViewTranslation;
layout (location = 11) uniform bool useCubeMap;
layout (location = 12) uniform bool exemptFromViewProjection;
uniform bool exemptFromView;
uniform bool useAtlas;

layout(location = 100) uniform vec3 _mouseOutColor;
layout(location = 101) uniform vec4 lightAmbient;
layout(location = 102) uniform vec3 viewPosition;
layout(location = 103) uniform vec3 pl[2];
layout(location = 105) uniform vec3 dl;
layout(location = 106) uniform float skyDim;
layout(location = 107) uniform float extraBrightness;

layout(location = 110) uniform sampler2D tex;
layout(location = 111) uniform samplerCube texCube;
layout(location = 112) uniform sampler2D shadow;
uniform sampler2DArray texAtlas;

layout(location = 500) uniform float specularPower;

in vec3 FragNormal;
in vec3 FragTexCoords;
in vec3 FragPos;
in vec3 FragVertexPos;
in vec3 FragShadowPos;

layout(location = 0) out vec4 outColor;

float pointLight(vec3 point) {
    // Diffuse light
    vec3 toLight = normalize(point - FragPos);
    float cos = max(dot(toLight, FragNormal), 0);

    // Specular light
    vec3 toView = normalize(viewPosition - FragPos);
    vec3 reflected = normalize(reflect(-toLight, FragNormal));
    float cos2 = pow(max(dot(toView, reflected), 0), specularPower);
    return cos + cos2;
}

float directionalLight(vec3 direction) {
    return max(dot(-direction, FragNormal), 0);
}

void main(void) {

    /*---------------POINT LIGHT-------------------*/
    float lamount = 0; // Light amount
//    lamount += pointLight(pl[0]);
//    lamount += pointLight(pl[1]);

    lamount += directionalLight(dl);

    float visibility = 1;
//    if (FragShadowPos.z < 0.1f) {
//        visibility = 0.1f;
//    }

    if (texture(shadow, FragShadowPos.xy).z < FragShadowPos.z) {
        visibility = 0.5;
    } else if (texture(shadow, FragShadowPos.xy).z == FragShadowPos.z) {
        visibility = 0.9;
    }

    vec4 texturedFragment;
    if (useCubeMap) {
        texturedFragment = texture(texCube, FragVertexPos);
    } else if (useAtlas) {
        texturedFragment = texture(texAtlas, FragTexCoords);
    } else {
        texturedFragment = texture(tex, vec2(FragTexCoords));
    }

    if (exemptFromViewProjection) {
        outColor = vec4(vec3(texturedFragment), 1) + vec4(_mouseOutColor, 0);
    } else if (exemptFromViewTranslation || exemptFromView) {
        if (useCubeMap) { // If it's the sky
            outColor = texturedFragment * skyDim + vec4(_mouseOutColor, 0);
        } else {
            outColor = texturedFragment + vec4(_mouseOutColor, 0);
        }
    } else {
        outColor = texturedFragment * (lamount * visibility + lightAmbient + extraBrightness) + vec4(_mouseOutColor, 0);
    }
}