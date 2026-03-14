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

#ifndef EPOXY_STDBOOL_H
#define EPOXY_STDBOOL_H
#include <stdbool.h>
#endif

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
#ifndef GL_XOR
#  define GL_XOR 0x1506
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

/* Texture swizzle (vec4 variant) */
#ifndef GL_TEXTURE_SWIZZLE_RGBA
#  define GL_TEXTURE_SWIZZLE_RGBA 0x8E46
#endif

/* Depth-stencil texture mode (GLES 3.1) */
#ifndef GL_DEPTH_STENCIL_TEXTURE_MODE
#  define GL_DEPTH_STENCIL_TEXTURE_MODE 0x90EA
#endif

/* Texture LOD range */
#ifndef GL_TEXTURE_BASE_LEVEL
#  define GL_TEXTURE_BASE_LEVEL 0x813C
#endif
#ifndef GL_TEXTURE_MAX_LEVEL
#  define GL_TEXTURE_MAX_LEVEL 0x813D
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

/* 16-bit normalized texture formats (not in GLES3) */
#ifndef GL_R16
#  define GL_R16 0x822A
#endif
#ifndef GL_RG16
#  define GL_RG16 0x822C
#endif
#ifndef GL_RGB16
#  define GL_RGB16 0x8054
#endif
#ifndef GL_RGBA16
#  define GL_RGBA16 0x805B
#endif
#ifndef GL_R16_SNORM
#  define GL_R16_SNORM 0x8F98
#endif
#ifndef GL_RG16_SNORM
#  define GL_RG16_SNORM 0x8F99
#endif
#ifndef GL_RGB16_SNORM
#  define GL_RGB16_SNORM 0x8F9A
#endif
#ifndef GL_RGBA16_SNORM
#  define GL_RGBA16_SNORM 0x8F9B
#endif

/* S3TC / DXT compressed texture formats (extension) */
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#  define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#  define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#  define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
#ifndef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
#  define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#endif
#ifndef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT
#  define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#endif
#ifndef GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
#  define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#endif

/* sRGB without alpha (desktop GL) */
#ifndef GL_SRGB8
#  define GL_SRGB8 0x8C41
#endif

/* Proxy textures (not in GLES3) */
#ifndef GL_PROXY_TEXTURE_2D_ARRAY
#  define GL_PROXY_TEXTURE_2D_ARRAY 0x8C1B
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
void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat,
                            GLsizei width, GLint border,
                            GLsizei imageSize, const void *data);
void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                               GLsizei width, GLenum format,
                               GLsizei imageSize, const void *data);
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
void glDebugMessageInsert(GLenum source, GLenum type, GLuint id,
                          GLenum severity, GLsizei length,
                          const GLchar *buf);
void glObjectLabel(GLenum identifier, GLuint name, GLsizei length,
                   const GLchar *label);
void glPushDebugGroup(GLenum source, GLuint id, GLsizei length,
                      const GLchar *message);
void glPopDebugGroup(void);

/* Sync objects — GLsync, glFenceSync, glDeleteSync, glClientWaitSync,
 * glWaitSync are part of GLES 3.0 and provided by <GLES3/gl3.h>.
 * Do NOT redefine or stub them here. */
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

/* Adjacency primitives (geometry shaders) */
#ifndef GL_LINES_ADJACENCY
#  define GL_LINES_ADJACENCY 0x000A
#endif
#ifndef GL_LINE_STRIP_ADJACENCY
#  define GL_LINE_STRIP_ADJACENCY 0x000B
#endif
#ifndef GL_TRIANGLES_ADJACENCY
#  define GL_TRIANGLES_ADJACENCY 0x000C
#endif
#ifndef GL_TRIANGLE_STRIP_ADJACENCY
#  define GL_TRIANGLE_STRIP_ADJACENCY 0x000D
#endif

/* Tessellation */
#ifndef GL_PATCHES
#  define GL_PATCHES 0x000E
#endif
#ifndef GL_PATCH_VERTICES
#  define GL_PATCH_VERTICES 0x8E72
#endif

/* Stencil (desktop GL has GL_STENCIL_INDEX, GLES3 only GL_STENCIL_INDEX8) */
#ifndef GL_STENCIL_INDEX
#  define GL_STENCIL_INDEX 0x1901
#endif

/* Multisampling */
#ifndef GL_MULTISAMPLE
#  define GL_MULTISAMPLE 0x809D
#endif

/* Anisotropic filtering (EXT_texture_filter_anisotropic) */
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#  define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#  define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

/* Dual-source blending (desktop GL / GLES extension) */
#ifndef GL_SRC1_COLOR
#  define GL_SRC1_COLOR 0x88F9
#endif
#ifndef GL_SRC1_ALPHA
#  define GL_SRC1_ALPHA 0x8589
#endif

/* Program point size */
#ifndef GL_PROGRAM_POINT_SIZE
#  define GL_PROGRAM_POINT_SIZE 0x8642
#endif

/* sRGB framebuffer */
#ifndef GL_FRAMEBUFFER_SRGB
#  define GL_FRAMEBUFFER_SRGB 0x8DB9
#endif

/* Depth clamp */
#ifndef GL_DEPTH_CLAMP
#  define GL_DEPTH_CLAMP 0x864F
#endif

/* Timer queries */
#ifndef GL_TIMESTAMP
#  define GL_TIMESTAMP 0x8E28
#endif
#ifndef GL_TIME_ELAPSED
#  define GL_TIME_ELAPSED 0x88BF
#endif

/* Occlusion queries (GLES3 has ANY_SAMPLES_PASSED but not SAMPLES_PASSED) */
#ifndef GL_SAMPLES_PASSED
#  define GL_SAMPLES_PASSED 0x8914
#endif

/* Memory barrier bits */
#ifndef GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT
#  define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 0x00000001
#endif
#ifndef GL_ELEMENT_ARRAY_BARRIER_BIT
#  define GL_ELEMENT_ARRAY_BARRIER_BIT 0x00000002
#endif
#ifndef GL_UNIFORM_BARRIER_BIT
#  define GL_UNIFORM_BARRIER_BIT 0x00000004
#endif
#ifndef GL_TEXTURE_FETCH_BARRIER_BIT
#  define GL_TEXTURE_FETCH_BARRIER_BIT 0x00000008
#endif
#ifndef GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
#  define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#endif
#ifndef GL_TEXTURE_UPDATE_BARRIER_BIT
#  define GL_TEXTURE_UPDATE_BARRIER_BIT 0x00000100
#endif
#ifndef GL_BUFFER_UPDATE_BARRIER_BIT
#  define GL_BUFFER_UPDATE_BARRIER_BIT 0x00000200
#endif
#ifndef GL_FRAMEBUFFER_BARRIER_BIT
#  define GL_FRAMEBUFFER_BARRIER_BIT 0x00000400
#endif
#ifndef GL_SHADER_STORAGE_BARRIER_BIT
#  define GL_SHADER_STORAGE_BARRIER_BIT 0x00002000
#endif
#ifndef GL_ALL_BARRIER_BITS
#  define GL_ALL_BARRIER_BITS 0xFFFFFFFF
#endif

/* SSBO block size query */
#ifndef GL_MAX_SHADER_STORAGE_BLOCK_SIZE
#  define GL_MAX_SHADER_STORAGE_BLOCK_SIZE 0x90DE
#endif

/* Per-stage SSBO limits (ES 3.1) */
#ifndef GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS
#  define GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS 0x90D6
#endif
#ifndef GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS
#  define GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS 0x90DC
#endif

/* Geometry shader texture units */
#ifndef GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS
#  define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS 0x8C29
#endif

/* Desktop GL varying limit (GLES3 has GL_MAX_VARYING_VECTORS instead) */
#ifndef GL_MAX_VARYING_FLOATS
#  define GL_MAX_VARYING_FLOATS 0x8B4B
#endif

/* Stereo / quad-buffer draw buffers (desktop GL) */
#ifndef GL_FRONT_LEFT
#  define GL_FRONT_LEFT 0x0400
#endif
#ifndef GL_FRONT_RIGHT
#  define GL_FRONT_RIGHT 0x0401
#endif
#ifndef GL_BACK_LEFT
#  define GL_BACK_LEFT 0x0402
#endif
#ifndef GL_BACK_RIGHT
#  define GL_BACK_RIGHT 0x0403
#endif
#ifndef GL_STEREO
#  define GL_STEREO 0x0C33
#endif

/* NVX_gpu_memory_info */
#ifndef GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX
#  define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX 0x9048
#endif
#ifndef GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX
#  define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049
#endif

/* ATI_meminfo */
#ifndef GL_TEXTURE_FREE_MEMORY_ATI
#  define GL_TEXTURE_FREE_MEMORY_ATI 0x87FC
#endif

/* KHR_debug object label identifiers (GL 4.3 / ES 3.2) */
#ifndef GL_BUFFER
#  define GL_BUFFER 0x82E0
#endif
#ifndef GL_SAMPLER
#  define GL_SAMPLER 0x82E6
#endif
#ifndef GL_PROGRAM
#  define GL_PROGRAM 0x82E2
#endif
#ifndef GL_SHADER
#  define GL_SHADER 0x82E1
#endif
#ifndef GL_VERTEX_ARRAY
#  define GL_VERTEX_ARRAY 0x8074
#endif
/* GL_TEXTURE (0x1702) and GL_FRAMEBUFFER (0x8D40) are in GLES3.
 * GL_SHADER_STORAGE_BUFFER and GL_UNIFORM_BUFFER are defined above. */

/* EXT_memory_object / device UUID constants */
#ifndef GL_NUM_DEVICE_UUIDS_EXT
#  define GL_NUM_DEVICE_UUIDS_EXT 0x9596
#endif
#ifndef GL_DEVICE_UUID_EXT
#  define GL_DEVICE_UUID_EXT 0x9597
#endif
#ifndef GL_UUID_SIZE_EXT
#  define GL_UUID_SIZE_EXT 16
#endif
#ifndef GL_DEVICE_LUID_EXT
#  define GL_DEVICE_LUID_EXT 0x9599
#endif
#ifndef GL_LUID_SIZE_EXT
#  define GL_LUID_SIZE_EXT 8
#endif
#ifndef GL_DEVICE_NODE_MASK_EXT
#  define GL_DEVICE_NODE_MASK_EXT 0x959A
#endif

/* Polygon modes (not in GLES3) */
#ifndef GL_FILL
#  define GL_FILL 0x1B02
#endif
#ifndef GL_LINE
#  define GL_LINE 0x1B01
#endif
#ifndef GL_POINT
#  define GL_POINT 0x1B00
#endif

/* Quads (not in GLES3/WebGL2) */
#ifndef GL_QUADS
#  define GL_QUADS 0x0007
#endif

/* Desktop-only sampler types (1D, buffer, multisample, cube-map-array) */
#ifndef GL_SAMPLER_1D
#  define GL_SAMPLER_1D 0x8B5D
#endif
#ifndef GL_SAMPLER_1D_SHADOW
#  define GL_SAMPLER_1D_SHADOW 0x8B61
#endif
#ifndef GL_SAMPLER_1D_ARRAY
#  define GL_SAMPLER_1D_ARRAY 0x8DC0
#endif
#ifndef GL_SAMPLER_1D_ARRAY_SHADOW
#  define GL_SAMPLER_1D_ARRAY_SHADOW 0x8DC3
#endif
#ifndef GL_SAMPLER_BUFFER
#  define GL_SAMPLER_BUFFER 0x8DC2
#endif
#ifndef GL_SAMPLER_2D_MULTISAMPLE
#  define GL_SAMPLER_2D_MULTISAMPLE 0x9108
#endif
#ifndef GL_SAMPLER_2D_MULTISAMPLE_ARRAY
#  define GL_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910B
#endif
#ifndef GL_SAMPLER_CUBE_MAP_ARRAY_ARB
#  define GL_SAMPLER_CUBE_MAP_ARRAY_ARB 0x900C
#endif
#ifndef GL_INT_SAMPLER_1D
#  define GL_INT_SAMPLER_1D 0x8DC9
#endif
#ifndef GL_INT_SAMPLER_1D_ARRAY
#  define GL_INT_SAMPLER_1D_ARRAY 0x8DCE
#endif
#ifndef GL_INT_SAMPLER_BUFFER
#  define GL_INT_SAMPLER_BUFFER 0x8DD0
#endif
#ifndef GL_INT_SAMPLER_2D_MULTISAMPLE
#  define GL_INT_SAMPLER_2D_MULTISAMPLE 0x9109
#endif
#ifndef GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY
#  define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910C
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_1D
#  define GL_UNSIGNED_INT_SAMPLER_1D 0x8DD1
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_1D_ARRAY
#  define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY 0x8DD6
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_BUFFER
#  define GL_UNSIGNED_INT_SAMPLER_BUFFER 0x8DD8
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE
#  define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 0x910A
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY
#  define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910D
#endif

/* Desktop-only / GLES 3.1 image types */
#ifndef GL_IMAGE_1D
#  define GL_IMAGE_1D 0x904C
#endif
#ifndef GL_IMAGE_2D
#  define GL_IMAGE_2D 0x904D
#endif
#ifndef GL_IMAGE_3D
#  define GL_IMAGE_3D 0x904E
#endif
#ifndef GL_IMAGE_CUBE
#  define GL_IMAGE_CUBE 0x9050
#endif
#ifndef GL_IMAGE_BUFFER
#  define GL_IMAGE_BUFFER 0x9051
#endif
#ifndef GL_IMAGE_1D_ARRAY
#  define GL_IMAGE_1D_ARRAY 0x9052
#endif
#ifndef GL_IMAGE_2D_ARRAY
#  define GL_IMAGE_2D_ARRAY 0x9053
#endif
#ifndef GL_IMAGE_CUBE_MAP_ARRAY
#  define GL_IMAGE_CUBE_MAP_ARRAY 0x9054
#endif
#ifndef GL_INT_IMAGE_1D
#  define GL_INT_IMAGE_1D 0x9057
#endif
#ifndef GL_INT_IMAGE_2D
#  define GL_INT_IMAGE_2D 0x9058
#endif
#ifndef GL_INT_IMAGE_3D
#  define GL_INT_IMAGE_3D 0x9059
#endif
#ifndef GL_INT_IMAGE_CUBE
#  define GL_INT_IMAGE_CUBE 0x905B
#endif
#ifndef GL_INT_IMAGE_BUFFER
#  define GL_INT_IMAGE_BUFFER 0x905C
#endif
#ifndef GL_INT_IMAGE_1D_ARRAY
#  define GL_INT_IMAGE_1D_ARRAY 0x905D
#endif
#ifndef GL_INT_IMAGE_2D_ARRAY
#  define GL_INT_IMAGE_2D_ARRAY 0x905E
#endif
#ifndef GL_INT_IMAGE_CUBE_MAP_ARRAY
#  define GL_INT_IMAGE_CUBE_MAP_ARRAY 0x905F
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_1D
#  define GL_UNSIGNED_INT_IMAGE_1D 0x9062
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_2D
#  define GL_UNSIGNED_INT_IMAGE_2D 0x9063
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_3D
#  define GL_UNSIGNED_INT_IMAGE_3D 0x9064
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_CUBE
#  define GL_UNSIGNED_INT_IMAGE_CUBE 0x9066
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_BUFFER
#  define GL_UNSIGNED_INT_IMAGE_BUFFER 0x9067
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_1D_ARRAY
#  define GL_UNSIGNED_INT_IMAGE_1D_ARRAY 0x9068
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_2D_ARRAY
#  define GL_UNSIGNED_INT_IMAGE_2D_ARRAY 0x9069
#endif
#ifndef GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY
#  define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY 0x906A
#endif

/* Uniform / UBO introspection constants */
#ifndef GL_UNIFORM_TYPE
#  define GL_UNIFORM_TYPE 0x8A37
#endif
#ifndef GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS
#  define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS 0x8A42
#endif
#ifndef GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES
#  define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES 0x8A43
#endif

/* Program resource interface (GLES 3.1) */
#ifndef GL_SHADER_STORAGE_BLOCK
#  define GL_SHADER_STORAGE_BLOCK 0x92E6
#endif
#ifndef GL_ACTIVE_RESOURCES
#  define GL_ACTIVE_RESOURCES 0x92F5
#endif
#ifndef GL_MAX_NAME_LENGTH
#  define GL_MAX_NAME_LENGTH 0x92F6
#endif
#ifndef GL_BUFFER_BINDING
#  define GL_BUFFER_BINDING 0x9302
#endif

/* ── Additional desktop GL function stubs ──────────────────────── */

/* Framebuffer (desktop glFramebufferTexture vs GLES3 glFramebufferTexture2D) */
void glFramebufferTexture(GLenum target, GLenum attachment,
                          GLuint texture, GLint level);
void glFramebufferParameteri(GLenum target, GLenum pname, GLint param);

/* Base-vertex / base-instance draw calls (GL 4.2) */
void glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count,
                                                   GLenum type, const void *indices,
                                                   GLsizei instancecount,
                                                   GLint basevertex,
                                                   GLuint baseinstance);
void glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count,
                                       GLsizei instancecount, GLuint baseinstance);

/* Compute dispatch indirect */
void glDispatchComputeIndirect(GLintptr indirect);

/* Timer queries */
void glQueryCounter(GLuint id, GLenum target);
void glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params);
void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 *params);

/* Polygon mode (not in GLES3) */
void glPolygonMode(GLenum face, GLenum mode);

/* Tessellation */
void glPatchParameteri(GLenum pname, GLint value);

/* Buffer readback (not in GLES3) */
void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size,
                        void *data);

/* EXT_memory_object functions */
void glGetUnsignedBytei_vEXT(GLenum target, GLuint index, GLubyte *data);
void glGetUnsignedBytevEXT(GLenum pname, GLubyte *data);

/* DSA buffer clearing (GL 4.5) */
void glClearNamedBufferData(GLuint buffer, GLenum internalformat,
                            GLenum format, GLenum type, const void *data);

/* Buffer data clearing (GL 4.3) */
void glClearBufferData(GLenum target, GLenum internalformat,
                       GLenum format, GLenum type, const void *data);

/* DSA buffer operations (GL 4.5) */
GLboolean glUnmapNamedBuffer(GLuint buffer);
void glGetNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size,
                             void *data);
void glCopyNamedBufferSubData(GLuint readBuffer, GLuint writeBuffer,
                              GLintptr readOffset, GLintptr writeOffset,
                              GLsizeiptr size);

/* Texture barrier (GL 4.5) */
void glTextureBarrier(void);

/* DSA texture functions (GL 4.5) */
void glTextureParameteri(GLuint texture, GLenum pname, GLint param);
void glTextureParameteriv(GLuint texture, GLenum pname, const GLint *params);
void glTextureSubImage1D(GLuint texture, GLint level, GLint xoffset,
                         GLsizei width, GLenum format, GLenum type,
                         const void *pixels);
void glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset,
                         GLint yoffset, GLsizei width, GLsizei height,
                         GLenum format, GLenum type, const void *pixels);
void glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset,
                         GLint yoffset, GLint zoffset, GLsizei width,
                         GLsizei height, GLsizei depth, GLenum format,
                         GLenum type, const void *pixels);
void glGetTextureImage(GLuint texture, GLint level, GLenum format,
                       GLenum type, GLsizei bufSize, void *pixels);
void glCompressedTextureSubImage1D(GLuint texture, GLint level, GLint xoffset,
                                   GLsizei width, GLenum format,
                                   GLsizei imageSize, const void *data);
void glCompressedTextureSubImage2D(GLuint texture, GLint level, GLint xoffset,
                                   GLint yoffset, GLsizei width, GLsizei height,
                                   GLenum format, GLsizei imageSize,
                                   const void *data);
void glCompressedTextureSubImage3D(GLuint texture, GLint level, GLint xoffset,
                                   GLint yoffset, GLint zoffset, GLsizei width,
                                   GLsizei height, GLsizei depth, GLenum format,
                                   GLsizei imageSize, const void *data);
void glTextureBuffer(GLuint texture, GLenum internalformat, GLuint buffer);
void glGenerateTextureMipmap(GLuint texture);

/* Multi-bind (GL 4.4 / ARB_multi_bind) */
void glBindTextures(GLuint first, GLsizei count, const GLuint *textures);
void glBindSamplers(GLuint first, GLsizei count, const GLuint *samplers);

/* Program interface query (GLES 3.1) */
void glGetProgramInterfaceiv(GLuint program, GLenum programInterface,
                             GLenum pname, GLint *params);

/* Shader introspection (desktop GL / GLES 3.1) */
void glGetActiveUniformName(GLuint program, GLuint uniformIndex,
                            GLsizei bufSize, GLsizei *length, GLchar *uniformName);
void glGetProgramResourceiv(GLuint program, GLenum programInterface,
                            GLuint index, GLsizei propCount,
                            const GLenum *props, GLsizei count,
                            GLsizei *length, GLint *params);
void glGetProgramResourceName(GLuint program, GLenum programInterface,
                              GLuint index, GLsizei bufSize,
                              GLsizei *length, GLchar *name);

#ifdef __cplusplus
}
#endif

#endif /* EPOXY_GL_H */
