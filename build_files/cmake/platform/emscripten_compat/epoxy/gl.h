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
 * functions that Blender's GPU backend queries. */

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

#ifdef __cplusplus
}
#endif

#endif /* EPOXY_GL_H */
