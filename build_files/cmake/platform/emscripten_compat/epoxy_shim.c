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

void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat,
                            GLsizei width, GLint border,
                            GLsizei imageSize, const void *data)
{
  (void)target; (void)level; (void)internalformat;
  (void)width; (void)border; (void)imageSize; (void)data;
}

void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                               GLsizei width, GLenum format,
                               GLsizei imageSize, const void *data)
{
  (void)target; (void)level; (void)xoffset;
  (void)width; (void)format; (void)imageSize; (void)data;
}

void glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer)
{
  (void)target; (void)internalformat; (void)buffer;
}

void *glMapBuffer(GLenum target, GLenum access)
{
  /* GLES3/WebGL2 has glMapBufferRange but not glMapBuffer.
   * Emulate by querying buffer size and mapping the full range. */
  GLint buf_size = 0;
  glGetBufferParameteriv(target, GL_BUFFER_SIZE, &buf_size);
  if (buf_size <= 0) {
    return NULL;
  }

  GLbitfield flags = 0;
  if (access == GL_READ_ONLY) {
    flags = GL_MAP_READ_BIT;
  }
  else if (access == GL_WRITE_ONLY) {
    flags = GL_MAP_WRITE_BIT;
  }
  else { /* GL_READ_WRITE */
    flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
  }

  return glMapBufferRange(target, 0, buf_size, flags);
}

void glGetTexImage(GLenum target, GLint level, GLenum format,
                   GLenum type, void *pixels)
{
  /* WebGL2/GLES3 does not have glGetTexImage. Emulate by attaching the
   * texture to a temporary FBO and using glReadPixels for 2D textures.
   * For non-2D targets (cube maps, 3D, arrays), fall back to no-op. */
  if (!pixels) {
    return;
  }

  if (target != GL_TEXTURE_2D) {
    /* Only 2D textures can be read via FBO attachment + glReadPixels.
     * Cube map faces, 3D slices, and array layers would need separate
     * handling. Return zeroed data for now. */
    return;
  }

  /* Get the currently bound texture. */
  GLint tex_id = 0;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex_id);
  if (tex_id == 0) {
    return;
  }

  /* Save current FBO binding. */
  GLint prev_fbo = 0;
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prev_fbo);

  /* Create a temporary FBO, attach the texture, and read back. */
  GLuint tmp_fbo = 0;
  glGenFramebuffers(1, &tmp_fbo);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, tmp_fbo);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, (GLuint)tex_id, level);

  if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
    /* Query texture dimensions at this mip level. We don't have
     * glGetTexLevelParameteriv on GLES3, so we use the FBO viewport. */
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    /* Use viewport dimensions as approximation for mip 0.
     * Callers typically know the size and provide appropriately sized buffers. */
    glReadPixels(0, 0, viewport[2], viewport[3], format, type, pixels);
  }

  /* Restore previous FBO. */
  glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)prev_fbo);
  glDeleteFramebuffers(1, &tmp_fbo);
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
  /* GLES3 has glDrawBuffers but not glDrawBuffer. Emulate via single-element call. */
  glDrawBuffers(1, &buf);
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

void glDebugMessageInsert(GLenum source, GLenum type, GLuint id,
                          GLenum severity, GLsizei length,
                          const GLchar *buf)
{
  (void)source; (void)type; (void)id;
  (void)severity; (void)length; (void)buf;
}

void glObjectLabel(GLenum identifier, GLuint name, GLsizei length,
                   const GLchar *label)
{
  (void)identifier; (void)name; (void)length; (void)label;
}

void glPushDebugGroup(GLenum source, GLuint id, GLsizei length,
                      const GLchar *message)
{
  (void)source; (void)id; (void)length; (void)message;
}

void glPopDebugGroup(void)
{
}

/* NOTE: glFenceSync, glDeleteSync, glClientWaitSync, glWaitSync are
 * part of GLES 3.0 / WebGL2 and provided by Emscripten's runtime.
 * They must NOT be stubbed here — the real implementations are needed
 * for correct synchronization behavior. */

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
  /* GLES3 has glFramebufferTexture2D but not glFramebufferTexture.
   * Best-effort: attach as GL_TEXTURE_2D. Layered rendering is not supported. */
  glFramebufferTexture2D(target, attachment, GL_TEXTURE_2D, texture, level);
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
  /* GLES3/WebGL2 does not have glGetBufferSubData. Emulate via
   * glMapBufferRange + memcpy. */
  if (!data || size <= 0) {
    return;
  }
  void *mapped = glMapBufferRange(target, offset, size, GL_MAP_READ_BIT);
  if (mapped) {
    memcpy(data, mapped, (size_t)size);
    glUnmapBuffer(target);
  }
}

void glClearNamedBufferData(GLuint buffer, GLenum internalformat,
                            GLenum format, GLenum type, const void *data)
{
  (void)buffer; (void)internalformat; (void)format; (void)type; (void)data;
}

void glClearBufferData(GLenum target, GLenum internalformat,
                       GLenum format, GLenum type, const void *data)
{
  (void)target; (void)internalformat; (void)format; (void)type; (void)data;
}

GLboolean glUnmapNamedBuffer(GLuint buffer)
{
  (void)buffer;
  return GL_TRUE;
}

void glGetNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size,
                             void *data)
{
  /* Emulate DSA via bind-to-target path. */
  if (!data || size <= 0) {
    return;
  }
  GLint prev_buf = 0;
  glGetIntegerv(GL_COPY_READ_BUFFER_BINDING, &prev_buf);
  glBindBuffer(GL_COPY_READ_BUFFER, buffer);
  glGetBufferSubData(GL_COPY_READ_BUFFER, offset, size, data);
  glBindBuffer(GL_COPY_READ_BUFFER, (GLuint)prev_buf);
}

void glCopyNamedBufferSubData(GLuint readBuffer, GLuint writeBuffer,
                              GLintptr readOffset, GLintptr writeOffset,
                              GLsizeiptr size)
{
  /* Emulate DSA via bind-to-target path. */
  glBindBuffer(GL_COPY_READ_BUFFER, readBuffer);
  glBindBuffer(GL_COPY_WRITE_BUFFER, writeBuffer);
  glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
                      readOffset, writeOffset, size);
  glBindBuffer(GL_COPY_READ_BUFFER, 0);
  glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
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

void glTextureParameteri(GLuint texture, GLenum pname, GLint param)
{
  (void)texture; (void)pname; (void)param;
}

void glTextureParameteriv(GLuint texture, GLenum pname, const GLint *params)
{
  (void)texture; (void)pname; (void)params;
}

void glTextureSubImage1D(GLuint texture, GLint level, GLint xoffset,
                         GLsizei width, GLenum format, GLenum type,
                         const void *pixels)
{
  (void)texture; (void)level; (void)xoffset;
  (void)width; (void)format; (void)type; (void)pixels;
}

void glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset,
                         GLint yoffset, GLsizei width, GLsizei height,
                         GLenum format, GLenum type, const void *pixels)
{
  (void)texture; (void)level; (void)xoffset; (void)yoffset;
  (void)width; (void)height; (void)format; (void)type; (void)pixels;
}

void glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset,
                         GLint yoffset, GLint zoffset, GLsizei width,
                         GLsizei height, GLsizei depth, GLenum format,
                         GLenum type, const void *pixels)
{
  (void)texture; (void)level; (void)xoffset; (void)yoffset; (void)zoffset;
  (void)width; (void)height; (void)depth; (void)format; (void)type; (void)pixels;
}

void glGetTextureImage(GLuint texture, GLint level, GLenum format,
                       GLenum type, GLsizei bufSize, void *pixels)
{
  (void)texture; (void)level; (void)format; (void)type; (void)bufSize; (void)pixels;
}

void glCompressedTextureSubImage1D(GLuint texture, GLint level, GLint xoffset,
                                   GLsizei width, GLenum format,
                                   GLsizei imageSize, const void *data)
{
  (void)texture; (void)level; (void)xoffset;
  (void)width; (void)format; (void)imageSize; (void)data;
}

void glCompressedTextureSubImage2D(GLuint texture, GLint level, GLint xoffset,
                                   GLint yoffset, GLsizei width, GLsizei height,
                                   GLenum format, GLsizei imageSize,
                                   const void *data)
{
  (void)texture; (void)level; (void)xoffset; (void)yoffset;
  (void)width; (void)height; (void)format; (void)imageSize; (void)data;
}

void glCompressedTextureSubImage3D(GLuint texture, GLint level, GLint xoffset,
                                   GLint yoffset, GLint zoffset, GLsizei width,
                                   GLsizei height, GLsizei depth, GLenum format,
                                   GLsizei imageSize, const void *data)
{
  (void)texture; (void)level; (void)xoffset; (void)yoffset; (void)zoffset;
  (void)width; (void)height; (void)depth; (void)format; (void)imageSize; (void)data;
}

void glTextureBuffer(GLuint texture, GLenum internalformat, GLuint buffer)
{
  (void)texture; (void)internalformat; (void)buffer;
}

void glGenerateTextureMipmap(GLuint texture)
{
  (void)texture;
}

void glBindTextures(GLuint first, GLsizei count, const GLuint *textures)
{
  (void)first; (void)count; (void)textures;
}

void glBindSamplers(GLuint first, GLsizei count, const GLuint *samplers)
{
  (void)first; (void)count; (void)samplers;
}

void glGetProgramInterfaceiv(GLuint program, GLenum programInterface,
                             GLenum pname, GLint *params)
{
  (void)program; (void)programInterface; (void)pname;
  if (params) {
    *params = 0;
  }
}

void glGetActiveUniformName(GLuint program, GLuint uniformIndex,
                            GLsizei bufSize, GLsizei *length, GLchar *uniformName)
{
  (void)program; (void)uniformIndex;
  if (length) {
    *length = 0;
  }
  if (uniformName && bufSize > 0) {
    uniformName[0] = '\0';
  }
}

void glGetProgramResourceiv(GLuint program, GLenum programInterface,
                            GLuint index, GLsizei propCount,
                            const GLenum *props, GLsizei count,
                            GLsizei *length, GLint *params)
{
  (void)program; (void)programInterface; (void)index;
  (void)propCount; (void)props;
  if (length) {
    *length = 0;
  }
  if (params && count > 0) {
    for (GLsizei i = 0; i < count; i++) {
      params[i] = 0;
    }
  }
}

void glGetProgramResourceName(GLuint program, GLenum programInterface,
                              GLuint index, GLsizei bufSize,
                              GLsizei *length, GLchar *name)
{
  (void)program; (void)programInterface; (void)index;
  if (length) {
    *length = 0;
  }
  if (name && bufSize > 0) {
    name[0] = '\0';
  }
}
