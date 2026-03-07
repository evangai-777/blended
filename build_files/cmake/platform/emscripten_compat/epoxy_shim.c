/* SPDX-FileCopyrightText: 2025 Blended Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Epoxy utility function stubs for Emscripten/WebGL2.
 *
 * On Emscripten, OpenGL ES 3.0 is provided directly by the runtime.
 * These stubs implement the subset of epoxy functions that Blender's
 * GPU backend calls to query GL capabilities at runtime.
 *
 * Desktop GL function stubs are also provided so the OpenGL backend
 * compiles. These are no-ops — features behind them are not functional
 * on WebGL2. */

#include "epoxy/gl.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* ── Epoxy utility functions ───────────────────────────────────── */

int epoxy_gl_version(void)
{
  /* WebGL2 corresponds to OpenGL ES 3.0. Return 30 (major*10 + minor). */
  return 30;
}

int epoxy_glsl_version(void)
{
  /* WebGL2 / OpenGL ES 3.0 supports GLSL ES 3.00 → return 300. */
  return 300;
}

bool epoxy_is_desktop_gl(void)
{
  /* Emscripten provides OpenGL ES, not desktop GL. */
  return false;
}

bool epoxy_has_gl_extension(const char *extension)
{
  const char *extensions = (const char *)glGetString(GL_EXTENSIONS);
  if (!extensions || !extension) {
    return false;
  }

  size_t ext_len = strlen(extension);
  const char *pos = extensions;

  while ((pos = strstr(pos, extension)) != NULL) {
    /* Make sure we matched a full token, not a substring. */
    char before = (pos == extensions) ? ' ' : pos[-1];
    char after = pos[ext_len];
    if ((before == ' ' || before == '\0') && (after == ' ' || after == '\0')) {
      return true;
    }
    pos += ext_len;
  }
  return false;
}

/* ── Desktop GL function stubs ─────────────────────────────────
 * These allow the OpenGL backend to compile on WebGL2.  They are
 * no-ops or return safe defaults.  Code paths that call them are
 * typically guarded by capability checks at runtime. */

void glTexImage1D(GLenum target, GLint level, GLint internalformat,
                  GLsizei width, GLint border,
                  GLenum format, GLenum type, const void *data)
{
  (void)target; (void)level; (void)internalformat;
  (void)width; (void)border; (void)format; (void)type; (void)data;
}

void glTexStorage1D(GLenum target, GLsizei levels, GLenum internalformat,
                    GLsizei width)
{
  (void)target; (void)levels; (void)internalformat; (void)width;
}

void glTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                     GLsizei width, GLenum format, GLenum type,
                     const void *data)
{
  (void)target; (void)level; (void)xoffset;
  (void)width; (void)format; (void)type; (void)data;
}

void glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer)
{
  (void)target; (void)internalformat; (void)buffer;
}

void *glMapBuffer(GLenum target, GLenum access)
{
  (void)target; (void)access;
  return NULL;
}

void glGetTexImage(GLenum target, GLint level, GLenum format,
                   GLenum type, void *pixels)
{
  (void)target; (void)level; (void)format; (void)type; (void)pixels;
}

void glGetTexLevelParameteriv(GLenum target, GLint level,
                              GLenum pname, GLint *params)
{
  (void)target; (void)level; (void)pname;
  if (params) {
    *params = 0;
  }
}

void glProvokingVertex(GLenum mode)
{
  (void)mode;
}

void glClipControl(GLenum origin, GLenum depth)
{
  (void)origin; (void)depth;
}

void glLogicOp(GLenum opcode)
{
  (void)opcode;
}

void glDrawBuffer(GLenum buf)
{
  (void)buf;
}

void glPointSize(GLfloat size)
{
  (void)size;
}

void glDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect)
{
  (void)mode; (void)type; (void)indirect;
}

void glDrawArraysIndirect(GLenum mode, const void *indirect)
{
  (void)mode; (void)indirect;
}

void glMultiDrawElementsIndirect(GLenum mode, GLenum type,
                                 const void *indirect, GLsizei drawcount,
                                 GLsizei stride)
{
  (void)mode; (void)type; (void)indirect; (void)drawcount; (void)stride;
}

void glMultiDrawArraysIndirect(GLenum mode, const void *indirect,
                               GLsizei drawcount, GLsizei stride)
{
  (void)mode; (void)indirect; (void)drawcount; (void)stride;
}

void glBindImageTexture(GLuint unit, GLuint texture, GLint level,
                        GLboolean layered, GLint layer, GLenum access,
                        GLenum format)
{
  (void)unit; (void)texture; (void)level;
  (void)layered; (void)layer; (void)access; (void)format;
}

void glBindImageTextures(GLuint first, GLsizei count, const GLuint *textures)
{
  (void)first; (void)count; (void)textures;
}

void glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y,
                       GLuint num_groups_z)
{
  (void)num_groups_x; (void)num_groups_y; (void)num_groups_z;
}

void glShaderStorageBlockBinding(GLuint program, GLuint storageBlockIndex,
                                 GLuint storageBlockBinding)
{
  (void)program; (void)storageBlockIndex; (void)storageBlockBinding;
}

void glBufferStorage(GLenum target, GLsizeiptr size, const void *data,
                     GLbitfield flags)
{
  (void)target; (void)size; (void)data; (void)flags;
}

void glClearBufferSubData(GLenum target, GLenum internalformat,
                          GLintptr offset, GLsizeiptr size,
                          GLenum format, GLenum type, const void *data)
{
  (void)target; (void)internalformat; (void)offset;
  (void)size; (void)format; (void)type; (void)data;
}

void glDebugMessageCallback(GLDEBUGPROC callback, const void *userParam)
{
  (void)callback; (void)userParam;
}

void glDebugMessageControl(GLenum source, GLenum type, GLenum severity,
                           GLsizei count, const GLuint *ids,
                           GLboolean enabled)
{
  (void)source; (void)type; (void)severity;
  (void)count; (void)ids; (void)enabled;
}

void glObjectLabel(GLenum identifier, GLuint name, GLsizei length,
                   const GLchar *label)
{
  (void)identifier; (void)name; (void)length; (void)label;
}

GLsync glFenceSync(GLenum condition, GLbitfield flags)
{
  (void)condition; (void)flags;
  return NULL;
}

void glDeleteSync(GLsync sync)
{
  (void)sync;
}

GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
  (void)sync; (void)flags; (void)timeout;
  return GL_ALREADY_SIGNALED;
}

void glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
  (void)sync; (void)flags; (void)timeout;
}

void glMemoryBarrier(GLbitfield barriers)
{
  (void)barriers;
}

void glTextureView(GLuint texture, GLenum target, GLuint origtexture,
                   GLenum internalformat, GLuint minlevel, GLuint numlevels,
                   GLuint minlayer, GLuint numlayers)
{
  (void)texture; (void)target; (void)origtexture;
  (void)internalformat; (void)minlevel; (void)numlevels;
  (void)minlayer; (void)numlayers;
}

void glCopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel,
                        GLint srcX, GLint srcY, GLint srcZ,
                        GLuint dstName, GLenum dstTarget, GLint dstLevel,
                        GLint dstX, GLint dstY, GLint dstZ,
                        GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
  (void)srcName; (void)srcTarget; (void)srcLevel;
  (void)srcX; (void)srcY; (void)srcZ;
  (void)dstName; (void)dstTarget; (void)dstLevel;
  (void)dstX; (void)dstY; (void)dstZ;
  (void)srcWidth; (void)srcHeight; (void)srcDepth;
}

void glBindVertexBuffer(GLuint bindingindex, GLuint buffer,
                        GLintptr offset, GLsizei stride)
{
  (void)bindingindex; (void)buffer; (void)offset; (void)stride;
}

void glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type,
                          GLboolean normalized, GLuint relativeoffset)
{
  (void)attribindex; (void)size; (void)type;
  (void)normalized; (void)relativeoffset;
}

void glVertexAttribBinding(GLuint attribindex, GLuint bindingindex)
{
  (void)attribindex; (void)bindingindex;
}

void glProgramUniform1i(GLuint program, GLint location, GLint v0)
{
  (void)program; (void)location; (void)v0;
}

void glFramebufferTexture(GLenum target, GLenum attachment,
                          GLuint texture, GLint level)
{
  (void)target; (void)attachment; (void)texture; (void)level;
}

void glFramebufferParameteri(GLenum target, GLenum pname, GLint param)
{
  (void)target; (void)pname; (void)param;
}

void glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count,
                                                   GLenum type, const void *indices,
                                                   GLsizei instancecount,
                                                   GLint basevertex,
                                                   GLuint baseinstance)
{
  (void)mode; (void)count; (void)type; (void)indices;
  (void)instancecount; (void)basevertex; (void)baseinstance;
}

void glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count,
                                       GLsizei instancecount, GLuint baseinstance)
{
  (void)mode; (void)first; (void)count;
  (void)instancecount; (void)baseinstance;
}

void glDispatchComputeIndirect(GLintptr indirect)
{
  (void)indirect;
}

void glQueryCounter(GLuint id, GLenum target)
{
  (void)id; (void)target;
}

void glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params)
{
  (void)id; (void)pname;
  if (params) {
    *params = 0;
  }
}

void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 *params)
{
  (void)id; (void)pname;
  if (params) {
    *params = 0;
  }
}

void glPolygonMode(GLenum face, GLenum mode)
{
  (void)face; (void)mode;
}

void glPatchParameteri(GLenum pname, GLint value)
{
  (void)pname; (void)value;
}

void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size,
                        void *data)
{
  (void)target; (void)offset; (void)size; (void)data;
}

void glClearNamedBufferData(GLuint buffer, GLenum internalformat,
                            GLenum format, GLenum type, const void *data)
{
  (void)buffer; (void)internalformat; (void)format; (void)type; (void)data;
}

void glGetUnsignedBytei_vEXT(GLenum target, GLuint index, GLubyte *data)
{
  (void)target; (void)index; (void)data;
}

void glGetUnsignedBytevEXT(GLenum pname, GLubyte *data)
{
  (void)pname; (void)data;
}

void glTextureBarrier(void)
{
}
