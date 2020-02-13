#include "StdH.h"
#include <Engine/Graphics/GfxLibrary.h>
#include <Engine/Graphics/Gfx_Shaders.h>
#include <Engine/Math/Matrix.h>
#include <AndroidAdapters/gles_adapter.h>

class GfxProgramPrivate {
public:
  GfxProgramPrivate(const char *vertexShader, const char *fragmentShader)
    : vertexShader(vertexShader),
      fragmentShader(fragmentShader) {};

  GLuint pgmObject = 0;
  const char *vertexShader, *fragmentShader;
  // uniforms
  GLint projMat;
  GLint modelViewMat;
  GLint mainTexture;
  GLint enableTexture;
  GLint enableAlphaTest;
  GLint withReflectionMapping;
  GLint withSpecularMapping;
  GLint isUnlite;
  GLint stretch;
  GLint offset;
  GLint lerpRatio;
  GLint texCorr;
  GLint color;
  GLint lightObj;
  GLint colorAmbient;
  GLint colorLight;
  GLint viewer;
  GLint objectRotation;
  GLint objectToView;
};

GfxShadersUniforms gfxCurrentUniforms;

GfxProgramPrivate *_currentProgram = 0;

GfxProgram gfxMakeShaderProgram(const char *vertexShader, const char *fragmentShader) {
  return (GfxProgram) new GfxProgramPrivate(vertexShader, fragmentShader);
}

GLuint compileShader(GLenum type, const char *name, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  // check status
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char buffer[2001] = "";
    glGetShaderInfoLog(shader, 2000, nullptr, buffer);
    static char error[2500];
    sprintf(error, "Cannot compile %s: %s", name, buffer);
    throw error;
  }

  return shader;
}

void gfxUseProgram(GfxProgram _program) {
  GfxProgramPrivate *pgm = (GfxProgramPrivate *) _program;
  _currentProgram = pgm;
  if (!pgm) {
    glUseProgram(0);
    return;
  }

  if (!pgm->pgmObject) {
    pgm->pgmObject = glCreateProgram();
    glAttachShader(pgm->pgmObject, compileShader(GL_VERTEX_SHADER, "vertex shader", pgm->vertexShader));
    glAttachShader(pgm->pgmObject, compileShader(GL_FRAGMENT_SHADER, "fragment shader", pgm->fragmentShader));
    glLinkProgram(pgm->pgmObject);
    glUseProgram(pgm->pgmObject);

    // get uniforms
    pgm->projMat = glGetUniformLocation(pgm->pgmObject, "projMat");
    pgm->modelViewMat = glGetUniformLocation(pgm->pgmObject, "modelViewMat");
    pgm->mainTexture = glGetUniformLocation(pgm->pgmObject, "mainTexture");
    if (pgm->mainTexture >= 0) glUniform1i(pgm->mainTexture, 0);
    pgm->enableTexture = glGetUniformLocation(pgm->pgmObject, "enableTexture");
    pgm->enableAlphaTest = glGetUniformLocation(pgm->pgmObject, "enableAlphaTest");

    pgm->withReflectionMapping = glGetUniformLocation(pgm->pgmObject, "withReflectionMapping");
    pgm->withSpecularMapping = glGetUniformLocation(pgm->pgmObject, "withSpecularMapping");

    pgm->stretch = glGetUniformLocation(pgm->pgmObject, "stretch");
    pgm->offset = glGetUniformLocation(pgm->pgmObject, "offset");
    pgm->lerpRatio = glGetUniformLocation(pgm->pgmObject, "lerpRatio");
    pgm->texCorr = glGetUniformLocation(pgm->pgmObject, "texCorr");
    pgm->color = glGetUniformLocation(pgm->pgmObject, "color");
    pgm->lightObj = glGetUniformLocation(pgm->pgmObject, "lightObj");
    pgm->colorAmbient = glGetUniformLocation(pgm->pgmObject, "colorAmbient");
    pgm->colorLight = glGetUniformLocation(pgm->pgmObject, "colorLight");
    pgm->viewer = glGetUniformLocation(pgm->pgmObject, "viewer");
    pgm->isUnlite = glGetUniformLocation(pgm->pgmObject, "isUnlite");
    pgm->objectRotation = glGetUniformLocation(pgm->pgmObject, "objectRotation");
    pgm->objectToView = glGetUniformLocation(pgm->pgmObject, "objectToView");

  } else {
    glUseProgram(pgm->pgmObject);
  }

  gles_adapter::syncError();
}

SLONG gfxGetAttribLocation(GfxProgram _program, const char *name) {
  GfxProgramPrivate *pgm = (GfxProgramPrivate *) _program;
  GLint result = glGetAttribLocation(pgm->pgmObject, name);
  gles_adapter::syncError();
  return result;
}

void gfxUniform(const char *uniformName, float f0) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = glGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    glUniform1f(uniformLocation, f0);
  }
  gles_adapter::syncError();
}

void gfxUniform(const char *uniformName, float f0, float f1) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = glGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    glUniform2f(uniformLocation, f0, f1);
  }
  gles_adapter::syncError();
}

void gfxUniform(const char *uniformName, float f0, float f1, float f2) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = glGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    glUniform3f(uniformLocation, f0, f1, f2);
  }
  gles_adapter::syncError();
}

void gfxUniform(const char *uniformName, float f0, float f1, float f2, float f3) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = glGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    glUniform4f(uniformLocation, f0, f1, f2, f3);
  }
  gles_adapter::syncError();
}

void gfxUniform(const char *uniformName, GFXColor &color) {
  gfxUniform(uniformName, color.gfxcol.ub.r / 255.0f, color.gfxcol.ub.g / 255.0f, color.gfxcol.ub.b / 255.0f, color.gfxcol.ub.a / 255.0f);
}

void gfxUniform(const char *uniformName, const FLOATmatrix3D &matrix) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  GLint uniformLocation = glGetUniformLocation(pgm->pgmObject, uniformName);
  if (uniformLocation >= 0) {
    glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, (const float *) &matrix);
  }
  gles_adapter::syncError();
}

inline void gfxSetUniformValue(GLint uniformLocation, bool value) {
  glUniform1f(uniformLocation, value ? 1 : 0);
}

inline void gfxSetUniformValue(GLint uniformLocation, float value) {
  glUniform1f(uniformLocation, value);
}

inline void gfxSetUniformValue(GLint uniformLocation, FLOAT2D value) {
  glUniform2f(uniformLocation, value(1), value(2));
}

inline void gfxSetUniformValue(GLint uniformLocation, const FLOAT3D &value) {
  glUniform3f(uniformLocation, value(1), value(2), value(3));
}

inline void gfxSetUniformValue(GLint uniformLocation, GFXColor value) {
  glUniform4f(uniformLocation, value.gfxcol.ub.r / 255.0f, value.gfxcol.ub.g / 255.0f, value.gfxcol.ub.b / 255.0f, value.gfxcol.ub.a / 255.0f);
}

inline void gfxSetUniformValue(GLint uniformLocation, const FLOATmatrix3D &value) {
  glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, (const float *) &value);
}

inline BOOL operator!=(const GFXColor &v1, const GFXColor &v2) {
  return v1.gfxcol.ul.abgr != v2.gfxcol.ul.abgr;
};

void gfxSyncProgram(struct GfxShadersUniforms &params) {
  GfxProgramPrivate *pgm = _currentProgram;
  ASSERT(pgm);
  glUseProgram(pgm->pgmObject);

  // update buffers
  glUniformMatrix4fv(pgm->projMat, 1, GL_FALSE, gles_adapter::getProjMat());
  glUniformMatrix4fv(pgm->modelViewMat, 1, GL_FALSE, gles_adapter::getModelViewMat());
  params.enableTexture = gles_adapter::isTexture2d();
  params.enableAlphaTest = gles_adapter::isAlphaTest();

  gfxSetUniformValue(pgm->enableTexture, params.enableTexture);
  gfxSetUniformValue(pgm->enableAlphaTest, params.enableAlphaTest);
  gfxSetUniformValue(pgm->withReflectionMapping, params.withReflectionMapping);
  gfxSetUniformValue(pgm->withSpecularMapping, params.withSpecularMapping);
  gfxSetUniformValue(pgm->stretch, params.stretch);
  gfxSetUniformValue(pgm->offset, params.offset);
  gfxSetUniformValue(pgm->lerpRatio, params.lerpRatio);
  gfxSetUniformValue(pgm->texCorr, params.texCorr);
  gfxSetUniformValue(pgm->color, params.color);
  gfxSetUniformValue(pgm->lightObj, params.lightObj);
  gfxSetUniformValue(pgm->colorAmbient, params.colorAmbient);
  gfxSetUniformValue(pgm->colorLight, params.colorLight);
  gfxSetUniformValue(pgm->viewer, params.viewer);
  gfxSetUniformValue(pgm->isUnlite, params.isUnlite);
  gfxSetUniformValue(pgm->objectRotation, params.objectRotation);
  gfxSetUniformValue(pgm->objectToView, params.objectToView);

  gles_adapter::syncError();
}
