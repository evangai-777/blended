/* SPDX-FileCopyrightText: 2025 Blended Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Minimal Epoxy shim for Emscripten/WebGL2 builds.
 *
 * Blender includes <epoxy/gl.h> everywhere for OpenGL function loading.
 * On Emscripten, GL functions are provided directly by the runtime —
 * no function-pointer dispatch is needed. This header redirects to
 * Emscripten's GLES3 headers and provides stubs for epoxy utility
 * functions that Blender's GPU backend queries.
 *
 * Desktop GL constants and function stubs are provided below so that
 * the OpenGL backend compiles on WebGL2. Features behind these stubs
 * are non-functional at runtime but the code paths that use them are
 * typically guarded by capability checks. */

#ifndef EPOXY_GL_H
#define EPOXY_GL_H

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Epoxy utility functions used by Blender's GPU backend. */
int epoxy_gl_version(void);
int epoxy_glsl_version(void);
bool epoxy_has_gl_extension(const char *extension);
bool epoxy_is_desktop_gl(void);

/* Epoxy typedef aliases that Blender may reference. */
typedef void (*GenericFunctionPointer)(void);

/* ── Desktop GL constants not in GLES3/WebGL2 ──────────────────────
 * Each is #ifndef guarded so that if gl2ext.h or a future Emscripten
 * update provides them, we don't conflict. */

/* Texture targets */
#ifndef GL_TEXTURE_1D
#  define GL_TEXTURE_1D 0x0DE0
#endif
#ifndef GL_TEXTURE_1D_ARRAY
#  define GL_TEXTURE_1D_ARRAY 0x8C18
#endif
#ifndef GL_PROXY_TEXTURE_1D
#  define GL_PROXY_TEXTURE_1D 0x8063
#endif
#ifndef GL_PROXY_TEXTURE_1D_ARRAY
#  define GL_PROXY_TEXTURE_1D_ARRAY 0x8C19
#endif
#ifndef GL_PROXY_TEXTURE_2D
#  define GL_PROXY_TEXTURE_2D 0x8064
#endif
#ifndef GL_PROXY_TEXTURE_3D
#  define GL_PROXY_TEXTURE_3D 0x8070
#endif
#ifndef GL_PROXY_TEXTURE_CUBE_MAP
#  define GL_PROXY_TEXTURE_CUBE_MAP 0x851B
#endif
#ifndef GL_TEXTURE_BUFFER
#  define GL_TEXTURE_BUFFER 0x8C2A
#endif
#ifndef GL_TEXTURE_CUBE_MAP_ARRAY
#  define GL_TEXTURE_CUBE_MAP_ARRAY 0x9009
#endif
#ifndef GL_TEXTURE_CUBE_MAP_ARRAY_ARB
#  define GL_TEXTURE_CUBE_MAP_ARRAY_ARB 0x9009
#endif
#ifndef GL_TEXTURE_CUBE_MAP_SEAMLESS
#  define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
#endif
#ifndef GL_TEXTURE_RECTANGLE
#  define GL_TEXTURE_RECTANGLE 0x84F5
#endif

/* Smoothing / antialiasing */
#ifndef GL_POLYGON_SMOOTH
#  define GL_POLYGON_SMOOTH 0x0B41
#endif
#ifndef GL_LINE_SMOOTH
#  define GL_LINE_SMOOTH 0x0B20
#endif
#ifndef GL_POINT_SPRITE
#  define GL_POINT_SPRITE 0x8861
#endif

/* Clipping */
#ifndef GL_CLIP_DISTANCE0
#  define GL_CLIP_DISTANCE0 0x3000
#endif
#ifndef GL_MAX_CLIP_DISTANCES
#  define GL_MAX_CLIP_DISTANCES 0x0D32
#endif
#ifndef GL_LOWER_LEFT
#  define GL_LOWER_LEFT 0x8CA1
#endif
#ifndef GL_UPPER_LEFT
#  define GL_UPPER_LEFT 0x8CA2
#endif
#ifndef GL_ZERO_TO_ONE
#  define GL_ZERO_TO_ONE 0x935F
#endif
#ifndef GL_NEGATIVE_ONE_TO_ONE
#  define GL_NEGATIVE_ONE_TO_ONE 0x935E
#endif

/* Logic operations */
#ifndef GL_COLOR_LOGIC_OP
#  define GL_COLOR_LOGIC_OP 0x0BF2
#endif

/* Provoking vertex */
#ifndef GL_FIRST_VERTEX_CONVENTION
#  define GL_FIRST_VERTEX_CONVENTION 0x8E4D
#endif
#ifndef GL_LAST_VERTEX_CONVENTION
#  define GL_LAST_VERTEX_CONVENTION 0x8E4E
#endif

/* Buffer mapping (GLES3 has glMapBufferRange but not glMapBuffer) */
#ifndef GL_READ_ONLY
#  define GL_READ_ONLY 0x88B8
#endif
#ifndef GL_WRITE_ONLY
#  define GL_WRITE_ONLY 0x88B9
#endif
#ifndef GL_READ_WRITE
#  define GL_READ_WRITE 0x88BA
#endif

/* Shader stages not in base GLES3 */
#ifndef GL_GEOMETRY_SHADER
#  define GL_GEOMETRY_SHADER 0x8DD9
#endif
#ifndef GL_TESS_CONTROL_SHADER
#  define GL_TESS_CONTROL_SHADER 0x8E88
#endif
#ifndef GL_TESS_EVALUATION_SHADER
#  define GL_TESS_EVALUATION_SHADER 0x8E87
#endif

/* Compute shaders (ES 3.1) */
#ifndef GL_COMPUTE_SHADER
#  define GL_COMPUTE_SHADER 0x91B9
#endif
#ifndef GL_MAX_COMPUTE_WORK_GROUP_COUNT
#  define GL_MAX_COMPUTE_WORK_GROUP_COUNT 0x91BE
#endif
#ifndef GL_MAX_COMPUTE_WORK_GROUP_SIZE
#  define GL_MAX_COMPUTE_WORK_GROUP_SIZE 0x91BF
#endif
#ifndef GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS
#  define GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS 0x90DB
#endif

/* Shader storage buffers (ES 3.1) */
#ifndef GL_SHADER_STORAGE_BUFFER
#  define GL_SHADER_STORAGE_BUFFER 0x90D2
#endif
#ifndef GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS
#  define GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS 0x90DD
#endif
#ifndef GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT
#  define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT 0x90DF
#endif

/* Debug output */
#ifndef GL_DEBUG_OUTPUT
#  define GL_DEBUG_OUTPUT 0x92E2
#endif
#ifndef GL_DEBUG_OUTPUT_SYNCHRONOUS
#  define GL_DEBUG_OUTPUT_SYNCHRONOUS 0x8242
#endif

/* Multi-draw indirect */
#ifndef GL_DRAW_INDIRECT_BUFFER
#  define GL_DRAW_INDIRECT_BUFFER 0x8F3F
#endif
#ifndef GL_DISPATCH_INDIRECT_BUFFER
#  define GL_DISPATCH_INDIRECT_BUFFER 0x90EE
#endif
#ifndef GL_COMMAND_BARRIER_BIT
#  define GL_COMMAND_BARRIER_BIT 0x00000040
#endif

/* Texture buffer size query */
#ifndef GL_MAX_TEXTURE_BUFFER_SIZE
#  define GL_MAX_TEXTURE_BUFFER_SIZE 0x8C2B
#endif

/* Proxy textures (cube map array) */
#ifndef GL_PROXY_TEXTURE_CUBE_MAP_ARRAY_ARB
#  define GL_PROXY_TEXTURE_CUBE_MAP_ARRAY_ARB 0x900B
#endif

/* Image textures (ES 3.1) */
#ifndef GL_MAX_IMAGE_UNITS
#  define GL_MAX_IMAGE_UNITS 0x8F38
#endif

/* Primitive restart */
#ifndef GL_PRIMITIVE_RESTART_FIXED_INDEX
#  define GL_PRIMITIVE_RESTART_FIXED_INDEX 0x8D69
#endif

/* Texture width query */
#ifndef GL_TEXTURE_WIDTH
#  define GL_TEXTURE_WIDTH 0x1000
#endif

/* ── Desktop GL function stubs ─────────────────────────────────────
 * Stub declarations for desktop GL functions not in GLES3.
 * Implementations are in epoxy_shim.c — they are no-ops or return
 * safe defaults. */

/* 1D textures */
void glTexImage1D(GLenum target, GLint level, GLint internalformat,
                  GLsizei width, GLint border,
                  GLenum format, GLenum type, const void *data);
void glTexStorage1D(GLenum target, GLsizei levels, GLenum internalformat,
                    GLsizei width);
void glTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                     GLsizei width, GLenum format, GLenum type,
                     const void *data);
void glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer);

/* Buffer mapping (desktop style — GLES3 only has glMapBufferRange) */
void *glMapBuffer(GLenum target, GLenum access);

/* Texture readback (not in GLES3) */
void glGetTexImage(GLenum target, GLint level, GLenum format,
                   GLenum type, void *pixels);
void glGetTexLevelParameteriv(GLenum target, GLint level,
                              GLenum pname, GLint *params);

/* State not in GLES3 */
void glProvokingVertex(GLenum mode);
void glClipControl(GLenum origin, GLenum depth);
void glLogicOp(GLenum opcode);
void glDrawBuffer(GLenum buf);
void glPointSize(GLfloat size);

/* Multi-draw indirect */
void glDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect);
void glDrawArraysIndirect(GLenum mode, const void *indirect);
void glMultiDrawElementsIndirect(GLenum mode, GLenum type,
                                 const void *indirect, GLsizei drawcount,
                                 GLsizei stride);
void glMultiDrawArraysIndirect(GLenum mode, const void *indirect,
                               GLsizei drawcount, GLsizei stride);

/* Image textures */
void glBindImageTexture(GLuint unit, GLuint texture, GLint level,
                        GLboolean layered, GLint layer, GLenum access,
                        GLenum format);
void glBindImageTextures(GLuint first, GLsizei count, const GLuint *textures);

/* Compute dispatch */
void glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y,
                       GLuint num_groups_z);

/* Shader storage */
void glShaderStorageBlockBinding(GLuint program, GLuint storageBlockIndex,
                                 GLuint storageBlockBinding);

/* Buffer storage (GL 4.4 / ES 3.1 extension) */
void glBufferStorage(GLenum target, GLsizeiptr size, const void *data,
                     GLbitfield flags);

/* Clear buffer sub-data */
void glClearBufferSubData(GLenum target, GLenum internalformat,
                          GLintptr offset, GLsizeiptr size,
                          GLenum format, GLenum type, const void *data);

/* Debug */
typedef void (*GLDEBUGPROC)(GLenum source, GLenum type, GLuint id,
                            GLenum severity, GLsizei length,
                            const GLchar *message, const void *userParam);
void glDebugMessageCallback(GLDEBUGPROC callback, const void *userParam);
void glDebugMessageControl(GLenum source, GLenum type, GLenum severity,
                           GLsizei count, const GLuint *ids,
                           GLboolean enabled);
void glObjectLabel(GLenum identifier, GLuint name, GLsizei length,
                   const GLchar *label);

/* Sync objects */
typedef struct __GLsync *GLsync;
#ifndef GL_SYNC_FLUSH_COMMANDS_BIT
#  define GL_SYNC_FLUSH_COMMANDS_BIT 0x00000001
#endif
#ifndef GL_TIMEOUT_IGNORED
#  define GL_TIMEOUT_IGNORED 0xFFFFFFFFFFFFFFFFull
#endif
#ifndef GL_TIMEOUT_EXPIRED
#  define GL_TIMEOUT_EXPIRED 0x911B
#endif
#ifndef GL_ALREADY_SIGNALED
#  define GL_ALREADY_SIGNALED 0x911A
#endif
#ifndef GL_CONDITION_SATISFIED
#  define GL_CONDITION_SATISFIED 0x911C
#endif
#ifndef GL_WAIT_FAILED
#  define GL_WAIT_FAILED 0x911D
#endif
GLsync glFenceSync(GLenum condition, GLbitfield flags);
void glDeleteSync(GLsync sync);
GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
void glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);

/* Memory barrier */
void glMemoryBarrier(GLbitfield barriers);

/* Texture views (GL 4.3) */
void glTextureView(GLuint texture, GLenum target, GLuint origtexture,
                   GLenum internalformat, GLuint minlevel, GLuint numlevels,
                   GLuint minlayer, GLuint numlayers);

/* Copy between textures (GL 4.3 / ES 3.2) */
void glCopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel,
                        GLint srcX, GLint srcY, GLint srcZ,
                        GLuint dstName, GLenum dstTarget, GLint dstLevel,
                        GLint dstX, GLint dstY, GLint dstZ,
                        GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);

/* Vertex attrib binding (GL 4.3 / ES 3.1) */
void glBindVertexBuffer(GLuint bindingindex, GLuint buffer,
                        GLintptr offset, GLsizei stride);
void glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type,
                          GLboolean normalized, GLuint relativeoffset);
void glVertexAttribBinding(GLuint attribindex, GLuint bindingindex);

/* Program uniform (GL 4.1 / ES 3.1) */
void glProgramUniform1i(GLuint program, GLint location, GLint v0);

/* Framebuffer defaults */
#ifndef GL_FRAMEBUFFER_DEFAULT_WIDTH
#  define GL_FRAMEBUFFER_DEFAULT_WIDTH 0x9310
#endif
#ifndef GL_FRAMEBUFFER_DEFAULT_HEIGHT
#  define GL_FRAMEBUFFER_DEFAULT_HEIGHT 0x9311
#endif

/* Buffer mapping flags */
#ifndef GL_MAP_PERSISTENT_BIT
#  define GL_MAP_PERSISTENT_BIT 0x0040
#endif
#ifndef GL_MAP_COHERENT_BIT
#  define GL_MAP_COHERENT_BIT 0x0080
#endif

#ifdef __cplusplus
}
#endif

#endif /* EPOXY_GL_H */
