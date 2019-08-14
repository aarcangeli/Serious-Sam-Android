#pragma once

#include <Engine/Math/Vector.h>

struct GfxShadersUniforms {
    bool enableTexture;
    bool enableAlphaTest;
    bool withReflectionMapping;
    bool withSpecularMapping;
    bool isUnlite;

    FLOAT3D stretch, offset;
    FLOAT lerpRatio;
    FLOAT2D texCorr;
    GFXColor color;
    FLOAT3D lightObj, colorAmbient, colorLight;
    FLOAT3D viewer;
    FLOATmatrix3D objectRotation;
    FLOATmatrix3D objectToView;
};
