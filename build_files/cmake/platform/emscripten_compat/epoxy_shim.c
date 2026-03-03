/* SPDX-FileCopyrightText: 2025 Blended Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Epoxy utility function stubs for Emscripten/WebGL2.
 *
 * On Emscripten, OpenGL ES 3.0 is provided directly by the runtime.
 * These stubs implement the subset of epoxy functions that Blender's
 * GPU backend calls to query GL capabilities at runtime. */

#include <GLES3/gl3.h>
#include <stdbool.h>
#include <string.h>

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
