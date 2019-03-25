
// glue between opengl 1.2 and opengl es 2.0

namespace gles_adapter {
  // internal api
  float *getModelViewMat();
  float *getProjMat();
  bool isTexture2d();
  bool isAlphaTest();
  void syncError();
  void gles_adp_init();

    /*
     * Miscellaneous
     */
    void gles_adp_glClearIndex(GLfloat c);
    void gles_adp_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
    void gles_adp_glClear(GLbitfield mask);
    void gles_adp_glIndexMask(GLuint mask);
    void gles_adp_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    void gles_adp_glAlphaFunc(GLenum func, GLclampf ref);
    void gles_adp_glBlendFunc(GLenum sfactor, GLenum dfactor);
    void gles_adp_glLogicOp(GLenum opcode);
    void gles_adp_glCullFace(GLenum mode);
    void gles_adp_glFrontFace(GLenum mode);
    void gles_adp_glPointSize(GLfloat size);
    void gles_adp_glLineWidth(GLfloat width);
    void gles_adp_glLineStipple(GLint factor, GLushort pattern);
    void gles_adp_glPolygonMode(GLenum face, GLenum mode);
    void gles_adp_glPolygonOffset(GLfloat factor, GLfloat units);
    void gles_adp_glPolygonStipple(const GLubyte *mask);
    void gles_adp_glGetPolygonStipple(GLubyte *mask);
    void gles_adp_glEdgeFlag(GLboolean flag);
    void gles_adp_glEdgeFlagv(const GLboolean *flag);
    void gles_adp_glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
    void gles_adp_glClipPlane(GLenum plane, const GLdouble *equation);
    void gles_adp_glGetClipPlane(GLenum plane, GLdouble *equation);
    void gles_adp_glDrawBuffer(GLenum mode);
    void gles_adp_glReadBuffer(GLenum mode);
    void gles_adp_glEnable(GLenum cap);
    void gles_adp_glDisable(GLenum cap);
    GLboolean gles_adp_glIsEnabled(GLenum cap);

    void gles_adp_glEnableClientState(GLenum cap);  /* 1.1 */
    void gles_adp_glDisableClientState(GLenum cap);  /* 1.1 */

    void gles_adp_glGetBooleanv(GLenum pname, GLboolean *params);
    void gles_adp_glGetDoublev(GLenum pname, GLdouble *params);
    void gles_adp_glGetFloatv(GLenum pname, GLfloat *params);
    void gles_adp_glGetIntegerv(GLenum pname, GLint *params);

    void gles_adp_glPushAttrib(GLbitfield mask);
    void gles_adp_glPopAttrib(void);

    void gles_adp_glPushClientAttrib(GLbitfield mask);  /* 1.1 */
    void gles_adp_glPopClientAttrib(void);  /* 1.1 */

    GLint gles_adp_glRenderMode(GLenum mode);
    GLenum gles_adp_glGetError(void);
    const GLubyte *gles_adp_glGetString(GLenum name);
    void gles_adp_glFinish(void);
    void gles_adp_glFlush(void);
    void gles_adp_glHint(GLenum target, GLenum mode);

    /*
     * Depth Buffer
     */
    void gles_adp_glClearDepth(GLclampd depth);
    void gles_adp_glDepthFunc(GLenum func);
    void gles_adp_glDepthMask(GLboolean flag);
    void gles_adp_glDepthRange(GLclampd near_val, GLclampd far_val);

    /*
     * Accumulation Buffer
     */
    void gles_adp_glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void gles_adp_glAccum(GLenum op, GLfloat value);

    /*
     * Transformation
     */
    void gles_adp_glMatrixMode(GLenum mode);
    void gles_adp_glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
    void gles_adp_glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);
    void gles_adp_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
    void gles_adp_glPushMatrix(void);
    void gles_adp_glPopMatrix(void);
    void gles_adp_glLoadIdentity(void);
    void gles_adp_glLoadMatrixd(const GLdouble *m);
    void gles_adp_glLoadMatrixf(const GLfloat *m);
    void gles_adp_glMultMatrixd(const GLdouble *m);
    void gles_adp_glMultMatrixf(const GLfloat *m);
    void gles_adp_glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
    void gles_adp_glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    void gles_adp_glScaled(GLdouble x, GLdouble y, GLdouble z);
    void gles_adp_glScalef(GLfloat x, GLfloat y, GLfloat z);
    void gles_adp_glTranslated(GLdouble x, GLdouble y, GLdouble z);
    void gles_adp_glTranslatef(GLfloat x, GLfloat y, GLfloat z);

    /*
     * Display Lists
     */
    GLboolean gles_adp_glIsList(GLuint list);
    void gles_adp_glDeleteLists(GLuint list, GLsizei range);
    GLuint gles_adp_glGenLists(GLsizei range);
    void gles_adp_glNewList(GLuint list, GLenum mode);
    void gles_adp_glEndList(void);
    void gles_adp_glCallList(GLuint list);
    void gles_adp_glCallLists(GLsizei n, GLenum type, const GLvoid *lists);
    void gles_adp_glListBase(GLuint base);

    /*
     * Drawing Functions
     */
    void gles_adp_glBegin(GLenum mode);
    void gles_adp_glEnd(void);

    void gles_adp_glVertex2d(GLdouble x, GLdouble y);
    void gles_adp_glVertex2f(GLfloat x, GLfloat y);
    void gles_adp_glVertex2i(GLint x, GLint y);
    void gles_adp_glVertex2s(GLshort x, GLshort y);
    void gles_adp_glVertex3d(GLdouble x, GLdouble y, GLdouble z);
    void gles_adp_glVertex3f(GLfloat x, GLfloat y, GLfloat z);
    void gles_adp_glVertex3i(GLint x, GLint y, GLint z);
    void gles_adp_glVertex3s(GLshort x, GLshort y, GLshort z);
    void gles_adp_glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void gles_adp_glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void gles_adp_glVertex4i(GLint x, GLint y, GLint z, GLint w);
    void gles_adp_glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w);
    void gles_adp_glVertex2dv(const GLdouble *v);
    void gles_adp_glVertex2fv(const GLfloat *v);
    void gles_adp_glVertex2iv(const GLint *v);
    void gles_adp_glVertex2sv(const GLshort *v);
    void gles_adp_glVertex3dv(const GLdouble *v);
    void gles_adp_glVertex3fv(const GLfloat *v);
    void gles_adp_glVertex3iv(const GLint *v);
    void gles_adp_glVertex3sv(const GLshort *v);
    void gles_adp_glVertex4dv(const GLdouble *v);
    void gles_adp_glVertex4fv(const GLfloat *v);
    void gles_adp_glVertex4iv(const GLint *v);
    void gles_adp_glVertex4sv(const GLshort *v);

    void gles_adp_glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz);
    void gles_adp_glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz);
    void gles_adp_glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
    void gles_adp_glNormal3i(GLint nx, GLint ny, GLint nz);
    void gles_adp_glNormal3s(GLshort nx, GLshort ny, GLshort nz);
    void gles_adp_glNormal3bv(const GLbyte *v);
    void gles_adp_glNormal3dv(const GLdouble *v);
    void gles_adp_glNormal3fv(const GLfloat *v);
    void gles_adp_glNormal3iv(const GLint *v);
    void gles_adp_glNormal3sv(const GLshort *v);

    void gles_adp_glIndexd(GLdouble c);
    void gles_adp_glIndexf(GLfloat c);
    void gles_adp_glIndexi(GLint c);
    void gles_adp_glIndexs(GLshort c);
    void gles_adp_glIndexub(GLubyte c);  /* 1.1 */
    void gles_adp_glIndexdv(const GLdouble *c);
    void gles_adp_glIndexfv(const GLfloat *c);
    void gles_adp_glIndexiv(const GLint *c);
    void gles_adp_glIndexsv(const GLshort *c);
    void gles_adp_glIndexubv(const GLubyte *c);  /* 1.1 */
    void gles_adp_glColor3b(GLbyte red, GLbyte green, GLbyte blue);
    void gles_adp_glColor3d(GLdouble red, GLdouble green, GLdouble blue);
    void gles_adp_glColor3f(GLfloat red, GLfloat green, GLfloat blue);
    void gles_adp_glColor3i(GLint red, GLint green, GLint blue);
    void gles_adp_glColor3s(GLshort red, GLshort green, GLshort blue);
    void gles_adp_glColor3ub(GLubyte red, GLubyte green, GLubyte blue);
    void gles_adp_glColor3ui(GLuint red, GLuint green, GLuint blue);
    void gles_adp_glColor3us(GLushort red, GLushort green, GLushort blue);
    void gles_adp_glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
    void gles_adp_glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
    void gles_adp_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void gles_adp_glColor4i(GLint red, GLint green, GLint blue, GLint alpha);
    void gles_adp_glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha);
    void gles_adp_glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
    void gles_adp_glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha);
    void gles_adp_glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha);

    void gles_adp_glColor3bv(const GLbyte *v);
    void gles_adp_glColor3dv(const GLdouble *v);
    void gles_adp_glColor3fv(const GLfloat *v);
    void gles_adp_glColor3iv(const GLint *v);
    void gles_adp_glColor3sv(const GLshort *v);
    void gles_adp_glColor3ubv(const GLubyte *v);
    void gles_adp_glColor3uiv(const GLuint *v);
    void gles_adp_glColor3usv(const GLushort *v);
    void gles_adp_glColor4bv(const GLbyte *v);
    void gles_adp_glColor4dv(const GLdouble *v);
    void gles_adp_glColor4fv(const GLfloat *v);
    void gles_adp_glColor4iv(const GLint *v);
    void gles_adp_glColor4sv(const GLshort *v);
    void gles_adp_glColor4ubv(const GLubyte *v);
    void gles_adp_glColor4uiv(const GLuint *v);
    void gles_adp_glColor4usv(const GLushort *v);

    void gles_adp_glTexCoord1d(GLdouble s);
    void gles_adp_glTexCoord1f(GLfloat s);
    void gles_adp_glTexCoord1i(GLint s);
    void gles_adp_glTexCoord1s(GLshort s);
    void gles_adp_glTexCoord2d(GLdouble s, GLdouble t);
    void gles_adp_glTexCoord2f(GLfloat s, GLfloat t);
    void gles_adp_glTexCoord2i(GLint s, GLint t);
    void gles_adp_glTexCoord2s(GLshort s, GLshort t);
    void gles_adp_glTexCoord3d(GLdouble s, GLdouble t, GLdouble r);
    void gles_adp_glTexCoord3f(GLfloat s, GLfloat t, GLfloat r);
    void gles_adp_glTexCoord3i(GLint s, GLint t, GLint r);
    void gles_adp_glTexCoord3s(GLshort s, GLshort t, GLshort r);
    void gles_adp_glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
    void gles_adp_glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    void gles_adp_glTexCoord4i(GLint s, GLint t, GLint r, GLint q);
    void gles_adp_glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q);
    void gles_adp_glTexCoord1dv(const GLdouble *v);
    void gles_adp_glTexCoord1fv(const GLfloat *v);
    void gles_adp_glTexCoord1iv(const GLint *v);
    void gles_adp_glTexCoord1sv(const GLshort *v);
    void gles_adp_glTexCoord2dv(const GLdouble *v);
    void gles_adp_glTexCoord2fv(const GLfloat *v);
    void gles_adp_glTexCoord2iv(const GLint *v);
    void gles_adp_glTexCoord2sv(const GLshort *v);
    void gles_adp_glTexCoord3dv(const GLdouble *v);
    void gles_adp_glTexCoord3fv(const GLfloat *v);
    void gles_adp_glTexCoord3iv(const GLint *v);
    void gles_adp_glTexCoord3sv(const GLshort *v);
    void gles_adp_glTexCoord4dv(const GLdouble *v);
    void gles_adp_glTexCoord4fv(const GLfloat *v);
    void gles_adp_glTexCoord4iv(const GLint *v);
    void gles_adp_glTexCoord4sv(const GLshort *v);

    void gles_adp_glRasterPos2d(GLdouble x, GLdouble y);
    void gles_adp_glRasterPos2f(GLfloat x, GLfloat y);
    void gles_adp_glRasterPos2i(GLint x, GLint y);
    void gles_adp_glRasterPos2s(GLshort x, GLshort y);
    void gles_adp_glRasterPos3d(GLdouble x, GLdouble y, GLdouble z);
    void gles_adp_glRasterPos3f(GLfloat x, GLfloat y, GLfloat z);
    void gles_adp_glRasterPos3i(GLint x, GLint y, GLint z);
    void gles_adp_glRasterPos3s(GLshort x, GLshort y, GLshort z);
    void gles_adp_glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void gles_adp_glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void gles_adp_glRasterPos4i(GLint x, GLint y, GLint z, GLint w);
    void gles_adp_glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w);
    void gles_adp_glRasterPos2dv(const GLdouble *v);
    void gles_adp_glRasterPos2fv(const GLfloat *v);
    void gles_adp_glRasterPos2iv(const GLint *v);
    void gles_adp_glRasterPos2sv(const GLshort *v);
    void gles_adp_glRasterPos3dv(const GLdouble *v);
    void gles_adp_glRasterPos3fv(const GLfloat *v);
    void gles_adp_glRasterPos3iv(const GLint *v);
    void gles_adp_glRasterPos3sv(const GLshort *v);
    void gles_adp_glRasterPos4dv(const GLdouble *v);
    void gles_adp_glRasterPos4fv(const GLfloat *v);
    void gles_adp_glRasterPos4iv(const GLint *v);
    void gles_adp_glRasterPos4sv(const GLshort *v);

    void gles_adp_glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
    void gles_adp_glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
    void gles_adp_glRecti(GLint x1, GLint y1, GLint x2, GLint y2);
    void gles_adp_glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2);

    void gles_adp_glRectdv(const GLdouble *v1, const GLdouble *v2);
    void gles_adp_glRectfv(const GLfloat *v1, const GLfloat *v2);
    void gles_adp_glRectiv(const GLint *v1, const GLint *v2);
    void gles_adp_glRectsv(const GLshort *v1, const GLshort *v2);

    /*
     * Vertex Arrays  (1.1)
     */
    void gles_adp_glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
    void gles_adp_glNormalPointer(GLenum type, GLsizei stride, const GLvoid *ptr);
    void gles_adp_glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
    void gles_adp_glIndexPointer(GLenum type, GLsizei stride, const GLvoid *ptr);
    void gles_adp_glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr);
    void gles_adp_glEdgeFlagPointer(GLsizei stride, const GLboolean *ptr);
    void gles_adp_glGetPointerv(GLenum pname, void **params);
    void gles_adp_glArrayElement(GLint i);
    void gles_adp_glDrawArrays(GLenum mode, GLint first, GLsizei count);
    void gles_adp_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
    void gles_adp_glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer);

    /*
     * Lighting
     */
    void gles_adp_glShadeModel(GLenum mode);
    void gles_adp_glLightf(GLenum light, GLenum pname, GLfloat param);
    void gles_adp_glLighti(GLenum light, GLenum pname, GLint param);
    void gles_adp_glLightfv(GLenum light, GLenum pname, const GLfloat *params);
    void gles_adp_glLightiv(GLenum light, GLenum pname, const GLint *params);
    void gles_adp_glGetLightfv(GLenum light, GLenum pname, GLfloat *params);
    void gles_adp_glGetLightiv(GLenum light, GLenum pname, GLint *params);
    void gles_adp_glLightModelf(GLenum pname, GLfloat param);
    void gles_adp_glLightModeli(GLenum pname, GLint param);
    void gles_adp_glLightModelfv(GLenum pname, const GLfloat *params);
    void gles_adp_glLightModeliv(GLenum pname, const GLint *params);
    void gles_adp_glMaterialf(GLenum face, GLenum pname, GLfloat param);
    void gles_adp_glMateriali(GLenum face, GLenum pname, GLint param);
    void gles_adp_glMaterialfv(GLenum face, GLenum pname, const GLfloat *params);
    void gles_adp_glMaterialiv(GLenum face, GLenum pname, const GLint *params);
    void gles_adp_glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params);
    void gles_adp_glGetMaterialiv(GLenum face, GLenum pname, GLint *params);
    void gles_adp_glColorMaterial(GLenum face, GLenum mode);

    /*
     * Raster functions
     */
    void gles_adp_glPixelZoom(GLfloat xfactor, GLfloat yfactor);
    void gles_adp_glPixelStoref(GLenum pname, GLfloat param);
    void gles_adp_glPixelStorei(GLenum pname, GLint param);
    void gles_adp_glPixelTransferf(GLenum pname, GLfloat param);
    void gles_adp_glPixelTransferi(GLenum pname, GLint param);
    void gles_adp_glPixelMapfv(GLenum map, GLint mapsize, const GLfloat *values);
    void gles_adp_glPixelMapuiv(GLenum map, GLint mapsize, const GLuint *values);
    void gles_adp_glPixelMapusv(GLenum map, GLint mapsize, const GLushort *values);
    void gles_adp_glGetPixelMapfv(GLenum map, GLfloat *values);
    void gles_adp_glGetPixelMapuiv(GLenum map, GLuint *values);
    void gles_adp_glGetPixelMapusv(GLenum map, GLushort *values);
    void gles_adp_glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
    void gles_adp_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
    void gles_adp_glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
    void gles_adp_glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);

    /*
     * Stenciling
     */
    void gles_adp_glStencilFunc(GLenum func, GLint ref, GLuint mask);
    void gles_adp_glStencilMask(GLuint mask);
    void gles_adp_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
    void gles_adp_glClearStencil(GLint s);

    /*
     * Texture mapping
     */
    void gles_adp_glTexGend(GLenum coord, GLenum pname, GLdouble param);
    void gles_adp_glTexGenf(GLenum coord, GLenum pname, GLfloat param);
    void gles_adp_glTexGeni(GLenum coord, GLenum pname, GLint param);
    void gles_adp_glTexGendv(GLenum coord, GLenum pname, const GLdouble *params);
    void gles_adp_glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params);
    void gles_adp_glTexGeniv(GLenum coord, GLenum pname, const GLint *params);
    void gles_adp_glGetTexGendv(GLenum coord, GLenum pname, GLdouble *params);
    void gles_adp_glGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params);
    void gles_adp_glGetTexGeniv(GLenum coord, GLenum pname, GLint *params);

    void gles_adp_glTexEnvf(GLenum target, GLenum pname, GLfloat param);
    void gles_adp_glTexEnvi(GLenum target, GLenum pname, GLint param);
    void gles_adp_glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params);
    void gles_adp_glTexEnviv(GLenum target, GLenum pname, const GLint *params);
    void gles_adp_glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params);
    void gles_adp_glGetTexEnviv(GLenum target, GLenum pname, GLint *params);

    void gles_adp_glTexParameterf(GLenum target, GLenum pname, GLfloat param);
    void gles_adp_glTexParameteri(GLenum target, GLenum pname, GLint param);
    void gles_adp_glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params);
    void gles_adp_glTexParameteriv(GLenum target, GLenum pname, const GLint *params);
    void gles_adp_glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params);
    void gles_adp_glGetTexParameteriv(GLenum target, GLenum pname, GLint *params);
    void gles_adp_glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params);
    void gles_adp_glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params);

    void gles_adp_glTexImage1D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    void gles_adp_glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    void gles_adp_glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);

    /* 1.1 functions */
    void gles_adp_glGenTextures(GLsizei n, GLuint *textures);
    void gles_adp_glDeleteTextures(GLsizei n, const GLuint *textures);
    void gles_adp_glBindTexture(GLenum target, GLuint texture);
    void gles_adp_glPrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities);
    GLboolean gles_adp_glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences);
    GLboolean gles_adp_glIsTexture(GLuint texture);

    void gles_adp_glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);

    void gles_adp_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);

    void gles_adp_glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);

    void gles_adp_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);

    void gles_adp_glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);

    void gles_adp_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

    /*
     * Evaluators
     */
    void gles_adp_glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
    void gles_adp_glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
    void gles_adp_glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
    void gles_adp_glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
    void gles_adp_glGetMapdv(GLenum target, GLenum query, GLdouble *v);
    void gles_adp_glGetMapfv(GLenum target, GLenum query, GLfloat *v);
    void gles_adp_glGetMapiv(GLenum target, GLenum query, GLint *v);
    void gles_adp_glEvalCoord1d(GLdouble u);
    void gles_adp_glEvalCoord1f(GLfloat u);
    void gles_adp_glEvalCoord1dv(const GLdouble *u);
    void gles_adp_glEvalCoord1fv(const GLfloat *u);
    void gles_adp_glEvalCoord2d(GLdouble u, GLdouble v);
    void gles_adp_glEvalCoord2f(GLfloat u, GLfloat v);
    void gles_adp_glEvalCoord2dv(const GLdouble *u);
    void gles_adp_glEvalCoord2fv(const GLfloat *u);
    void gles_adp_glMapGrid1d(GLint un, GLdouble u1, GLdouble u2);
    void gles_adp_glMapGrid1f(GLint un, GLfloat u1, GLfloat u2);
    void gles_adp_glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
    void gles_adp_glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
    void gles_adp_glEvalPoint1(GLint i);
    void gles_adp_glEvalPoint2(GLint i, GLint j);
    void gles_adp_glEvalMesh1(GLenum mode, GLint i1, GLint i2);
    void gles_adp_glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);

    /*
     * Fog
     */
    void gles_adp_glFogf(GLenum pname, GLfloat param);
    void gles_adp_glFogi(GLenum pname, GLint param);
    void gles_adp_glFogfv(GLenum pname, const GLfloat *params);
    void gles_adp_glFogiv(GLenum pname, const GLint *params);

    /*
     * Selection and Feedback
     */
    void gles_adp_glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer);
    void gles_adp_glPassThrough(GLfloat token);
    void gles_adp_glSelectBuffer(GLsizei size, GLuint *buffer);
    void gles_adp_glInitNames(void);
    void gles_adp_glLoadName(GLuint name);
    void gles_adp_glPushName(GLuint name);
    void gles_adp_glPopName(void);


    /*
     * 1.0 Extensions
     */
    /* GL_EXT_blend_minmax */
    void gles_adp_glBlendEquationEXT(GLenum mode);

    /* GL_EXT_blend_color */
    void gles_adp_glBlendColorEXT(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

    /* GL_EXT_polygon_offset */
    void gles_adp_glPolygonOffsetEXT(GLfloat factor, GLfloat bias);

    /* GL_EXT_vertex_array */
    void gles_adp_glVertexPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr);
    void gles_adp_glNormalPointerEXT(GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr);
    void gles_adp_glColorPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr);
    void gles_adp_glIndexPointerEXT(GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr);
    void gles_adp_glTexCoordPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *ptr);
    void gles_adp_glEdgeFlagPointerEXT(GLsizei stride, GLsizei count, const GLboolean *ptr);
    void gles_adp_glGetPointervEXT(GLenum pname, void **params);
    void gles_adp_glArrayElementEXT(GLint i);
    void gles_adp_glDrawArraysEXT(GLenum mode, GLint first, GLsizei count);

    /* GL_EXT_texture_object */
    void gles_adp_glGenTexturesEXT(GLsizei n, GLuint *textures);
    void gles_adp_glDeleteTexturesEXT(GLsizei n, const GLuint *textures);
    void gles_adp_glBindTextureEXT(GLenum target, GLuint texture);
    void gles_adp_glPrioritizeTexturesEXT(GLsizei n, const GLuint *textures, const GLclampf *priorities);
    GLboolean gles_adp_glAreTexturesResidentEXT(GLsizei n, const GLuint *textures, GLboolean *residences);
    GLboolean gles_adp_glIsTextureEXT(GLuint texture);

    /* GL_EXT_texture3D */
    void gles_adp_glTexImage3DEXT(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    void gles_adp_glTexSubImage3DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
    void gles_adp_glCopyTexSubImage3DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

    /* GL_EXT_color_table */
    void gles_adp_glColorTableEXT(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
    void gles_adp_glColorSubTableEXT(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
    void gles_adp_glGetColorTableEXT(GLenum target, GLenum format, GLenum type, GLvoid *table);
    void gles_adp_glGetColorTableParameterfvEXT(GLenum target, GLenum pname, GLfloat *params);
    void gles_adp_glGetColorTableParameterivEXT(GLenum target, GLenum pname, GLint *params);

    /* GL_SGIS_multitexture */
    void gles_adp_glMultiTexCoord1dSGIS(GLenum target, GLdouble s);
    void gles_adp_glMultiTexCoord1dvSGIS(GLenum target, const GLdouble *v);
    void gles_adp_glMultiTexCoord1fSGIS(GLenum target, GLfloat s);
    void gles_adp_glMultiTexCoord1fvSGIS(GLenum target, const GLfloat *v);
    void gles_adp_glMultiTexCoord1iSGIS(GLenum target, GLint s);
    void gles_adp_glMultiTexCoord1ivSGIS(GLenum target, const GLint *v);
    void gles_adp_glMultiTexCoord1sSGIS(GLenum target, GLshort s);
    void gles_adp_glMultiTexCoord1svSGIS(GLenum target, const GLshort *v);
    void gles_adp_glMultiTexCoord2dSGIS(GLenum target, GLdouble s, GLdouble t);
    void gles_adp_glMultiTexCoord2dvSGIS(GLenum target, const GLdouble *v);
    void gles_adp_glMultiTexCoord2fSGIS(GLenum target, GLfloat s, GLfloat t);
    void gles_adp_glMultiTexCoord2fvSGIS(GLenum target, const GLfloat *v);
    void gles_adp_glMultiTexCoord2iSGIS(GLenum target, GLint s, GLint t);
    void gles_adp_glMultiTexCoord2ivSGIS(GLenum target, const GLint *v);
    void gles_adp_glMultiTexCoord2sSGIS(GLenum target, GLshort s, GLshort t);
    void gles_adp_glMultiTexCoord2svSGIS(GLenum target, const GLshort *v);
    void gles_adp_glMultiTexCoord3dSGIS(GLenum target, GLdouble s, GLdouble t, GLdouble r);
    void gles_adp_glMultiTexCoord3dvSGIS(GLenum target, const GLdouble *v);
    void gles_adp_glMultiTexCoord3fSGIS(GLenum target, GLfloat s, GLfloat t, GLfloat r);
    void gles_adp_glMultiTexCoord3fvSGIS(GLenum target, const GLfloat *v);
    void gles_adp_glMultiTexCoord3iSGIS(GLenum target, GLint s, GLint t, GLint r);
    void gles_adp_glMultiTexCoord3ivSGIS(GLenum target, const GLint *v);
    void gles_adp_glMultiTexCoord3sSGIS(GLenum target, GLshort s, GLshort t, GLshort r);
    void gles_adp_glMultiTexCoord3svSGIS(GLenum target, const GLshort *v);
    void gles_adp_glMultiTexCoord4dSGIS(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
    void gles_adp_glMultiTexCoord4dvSGIS(GLenum target, const GLdouble *v);
    void gles_adp_glMultiTexCoord4fSGIS(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    void gles_adp_glMultiTexCoord4fvSGIS(GLenum target, const GLfloat *v);
    void gles_adp_glMultiTexCoord4iSGIS(GLenum target, GLint s, GLint t, GLint r, GLint q);
    void gles_adp_glMultiTexCoord4ivSGIS(GLenum target, const GLint *v);
    void gles_adp_glMultiTexCoord4sSGIS(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
    void gles_adp_glMultiTexCoord4svSGIS(GLenum target, const GLshort *v);
    void gles_adp_glMultiTexCoordPointerSGIS(GLenum target, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    void gles_adp_glSelectTextureSGIS(GLenum target);
    void gles_adp_glSelectTextureCoordSetSGIS(GLenum target);

    /* GL_EXT_multitexture */
    void gles_adp_glMultiTexCoord1dEXT(GLenum target, GLdouble s);
    void gles_adp_glMultiTexCoord1dvEXT(GLenum target, const GLdouble *v);
    void gles_adp_glMultiTexCoord1fEXT(GLenum target, GLfloat s);
    void gles_adp_glMultiTexCoord1fvEXT(GLenum target, const GLfloat *v);
    void gles_adp_glMultiTexCoord1iEXT(GLenum target, GLint s);
    void gles_adp_glMultiTexCoord1ivEXT(GLenum target, const GLint *v);
    void gles_adp_glMultiTexCoord1sEXT(GLenum target, GLshort s);
    void gles_adp_glMultiTexCoord1svEXT(GLenum target, const GLshort *v);
    void gles_adp_glMultiTexCoord2dEXT(GLenum target, GLdouble s, GLdouble t);
    void gles_adp_glMultiTexCoord2dvEXT(GLenum target, const GLdouble *v);
    void gles_adp_glMultiTexCoord2fEXT(GLenum target, GLfloat s, GLfloat t);
    void gles_adp_glMultiTexCoord2fvEXT(GLenum target, const GLfloat *v);
    void gles_adp_glMultiTexCoord2iEXT(GLenum target, GLint s, GLint t);
    void gles_adp_glMultiTexCoord2ivEXT(GLenum target, const GLint *v);
    void gles_adp_glMultiTexCoord2sEXT(GLenum target, GLshort s, GLshort t);
    void gles_adp_glMultiTexCoord2svEXT(GLenum target, const GLshort *v);
    void gles_adp_glMultiTexCoord3dEXT(GLenum target, GLdouble s, GLdouble t, GLdouble r);
    void gles_adp_glMultiTexCoord3dvEXT(GLenum target, const GLdouble *v);
    void gles_adp_glMultiTexCoord3fEXT(GLenum target, GLfloat s, GLfloat t, GLfloat r);
    void gles_adp_glMultiTexCoord3fvEXT(GLenum target, const GLfloat *v);
    void gles_adp_glMultiTexCoord3iEXT(GLenum target, GLint s, GLint t, GLint r);
    void gles_adp_glMultiTexCoord3ivEXT(GLenum target, const GLint *v);
    void gles_adp_glMultiTexCoord3sEXT(GLenum target, GLshort s, GLshort t, GLshort r);
    void gles_adp_glMultiTexCoord3svEXT(GLenum target, const GLshort *v);
    void gles_adp_glMultiTexCoord4dEXT(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
    void gles_adp_glMultiTexCoord4dvEXT(GLenum target, const GLdouble *v);
    void gles_adp_glMultiTexCoord4fEXT(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    void gles_adp_glMultiTexCoord4fvEXT(GLenum target, const GLfloat *v);
    void gles_adp_glMultiTexCoord4iEXT(GLenum target, GLint s, GLint t, GLint r, GLint q);
    void gles_adp_glMultiTexCoord4ivEXT(GLenum target, const GLint *v);
    void gles_adp_glMultiTexCoord4sEXT(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
    void gles_adp_glMultiTexCoord4svEXT(GLenum target, const GLshort *v);
    void gles_adp_glInterleavedTextureCoordSetsEXT(GLint factor);
    void gles_adp_glSelectTextureEXT(GLenum target);
    void gles_adp_glSelectTextureCoordSetEXT(GLenum target);
    void gles_adp_glSelectTextureTransformEXT(GLenum target);

    /* GL_EXT_point_parameters */
    void gles_adp_glPointParameterfEXT(GLenum pname, GLfloat param);
    void gles_adp_glPointParameterfvEXT(GLenum pname, const GLfloat *params);

    /* 1.2 functions */
    void gles_adp_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
    void gles_adp_glTexImage3D(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    void gles_adp_glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
    void gles_adp_glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

}

