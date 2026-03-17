/* SPDX-FileCopyrightText: 2022 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup gpu
 */

#include "BLI_string.h"

#include "GPU_capabilities.hh"
#include "gpu_backend.hh"
#include "gpu_context_private.hh"

#include "gl_backend.hh"
#include "gl_debug.hh"
#include "gl_storage_buffer.hh"
#include "gl_vertex_buffer.hh"

namespace blender::gpu {

/* WebGL2 does not support SSBOs (GL_SHADER_STORAGE_BUFFER). On Emscripten we
 * emulate storage buffers using GL_COPY_READ_BUFFER as a generic buffer target.
 * The data layout is unchanged — only the bind target differs. Shader-level
 * SSBO access needs separate fallback paths (texture buffers or UBOs) handled
 * at the draw/shader level. GL_COPY_READ_BUFFER is used because it does not
 * conflict with GL_ARRAY_BUFFER (used by vertex buffers) or GL_UNIFORM_BUFFER
 * (used by uniform buffers). */
#ifdef __EMSCRIPTEN__
#  define SSBO_TARGET GL_COPY_READ_BUFFER
#else
#  define SSBO_TARGET GL_SHADER_STORAGE_BUFFER
#endif

/* -------------------------------------------------------------------- */
/** \name Creation & Deletion
 * \{ */

GLStorageBuf::GLStorageBuf(size_t size, GPUUsageType usage, const char *name)
    : StorageBuf(size, name)
{
  usage_ = usage;
  /* Do not create SSBO GL buffer here to allow allocation from any thread. */
  BLI_assert(size <= GPU_max_storage_buffer_size());
}

GLStorageBuf::~GLStorageBuf()
{
  if (read_fence_) {
    glDeleteSync(read_fence_);
  }

  if (persistent_ptr_) {
    if (GLContext::direct_state_access_support) {
      glUnmapNamedBuffer(read_ssbo_id_);
    }
    else {
      glBindBuffer(SSBO_TARGET, read_ssbo_id_);
      glUnmapBuffer(SSBO_TARGET);
      glBindBuffer(SSBO_TARGET, 0);
    }
  }

  if (read_ssbo_id_) {
    GLContext::buffer_free(read_ssbo_id_);
  }

  GLContext::buffer_free(ssbo_id_);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Data upload / update
 * \{ */

void GLStorageBuf::init()
{
  BLI_assert(GLContext::get());

  alloc_size_in_bytes_ = ceil_to_multiple_ul(size_in_bytes_, 16);
  glGenBuffers(1, &ssbo_id_);
  glBindBuffer(SSBO_TARGET, ssbo_id_);
  glBufferData(SSBO_TARGET, alloc_size_in_bytes_, nullptr, to_gl(this->usage_));

  debug::object_label(GL_BUFFER, ssbo_id_, name_);
}

void GLStorageBuf::update(const void *data)
{
  if (ssbo_id_ == 0) {
    this->init();
  }

  glBindBuffer(SSBO_TARGET, ssbo_id_);
  glBufferSubData(SSBO_TARGET, 0, size_in_bytes_, data);
  glBindBuffer(SSBO_TARGET, 0);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Usage
 * \{ */

void GLStorageBuf::bind(int slot)
{
  if (slot >= GLContext::max_ssbo_binds) {
    fprintf(
        stderr,
        "Error: Trying to bind \"%s\" ssbo to slot %d which is above the reported limit of %d.\n",
        name_,
        slot,
        GLContext::max_ssbo_binds);
    return;
  }

  if (ssbo_id_ == 0) {
    this->init();
  }

  if (data_ != nullptr) {
    this->update(data_);
    MEM_SAFE_DELETE_VOID(data_);
  }

  slot_ = slot;
#ifdef __EMSCRIPTEN__
  /* WebGL2 has no glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ...).
   * Bind as a UBO slot instead — the data was uploaded to a generic buffer
   * and shaders on Emscripten use UBO or texture fallback paths. */
  glBindBufferBase(GL_UNIFORM_BUFFER, slot_, ssbo_id_);
#else
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot_, ssbo_id_);
#endif

#ifndef NDEBUG
  BLI_assert(slot < 16);
  GLContext::get()->bound_ssbo_slots |= uint16_t(1) << slot;
#endif
}

void GLStorageBuf::bind_as(GLenum target)
{
  BLI_assert_msg(ssbo_id_ != 0,
                 "Trying to use storage buffer as indirect buffer but buffer was never filled.");
  glBindBuffer(target, ssbo_id_);
}

void GLStorageBuf::unbind()
{
#ifndef NDEBUG
  /* NOTE: This only unbinds the last bound slot. */
#  ifdef __EMSCRIPTEN__
  glBindBufferBase(GL_UNIFORM_BUFFER, slot_, 0);
#  else
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot_, 0);
#  endif
  /* Hope that the context did not change. */
  GLContext::get()->bound_ssbo_slots &= uint16_t(~(uint16_t(1) << slot_));
#endif
  slot_ = 0;
}

void GLStorageBuf::clear(uint32_t clear_value)
{
  if (ssbo_id_ == 0) {
    this->init();
  }

#ifdef __EMSCRIPTEN__
  /* WebGL2 has no glClearBufferData / glClearNamedBufferData.
   * Fill on the CPU and re-upload. */
  size_t num_words = alloc_size_in_bytes_ / sizeof(uint32_t);
  uint32_t *tmp = static_cast<uint32_t *>(malloc(alloc_size_in_bytes_));
  for (size_t i = 0; i < num_words; i++) {
    tmp[i] = clear_value;
  }
  glBindBuffer(SSBO_TARGET, ssbo_id_);
  glBufferSubData(SSBO_TARGET, 0, alloc_size_in_bytes_, tmp);
  glBindBuffer(SSBO_TARGET, 0);
  free(tmp);
#else
  if (GLContext::direct_state_access_support) {
    glClearNamedBufferData(ssbo_id_, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &clear_value);
  }
  else {
    /* WATCH(@fclem): This should be ok since we only use clear outside of drawing functions. */
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id_);
    glClearBufferData(
        GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &clear_value);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }
#endif
}

void GLStorageBuf::copy_sub(VertBuf *src_, uint dst_offset, uint src_offset, uint copy_size)
{
  GLVertBuf *src = static_cast<GLVertBuf *>(src_);
  GLStorageBuf *dst = this;

  if (dst->ssbo_id_ == 0) {
    dst->init();
  }
  if (src->vbo_id_ == 0) {
    src->bind();
  }

  if (GLContext::direct_state_access_support) {
    glCopyNamedBufferSubData(src->vbo_id_, dst->ssbo_id_, src_offset, dst_offset, copy_size);
  }
  else {
    /* This binds the buffer to GL_ARRAY_BUFFER and upload the data if any. */
    src->bind();
    glBindBuffer(GL_COPY_WRITE_BUFFER, dst->ssbo_id_);
    glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, src_offset, dst_offset, copy_size);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
  }
}

void GLStorageBuf::async_flush_to_host()
{
  if (ssbo_id_ == 0) {
    this->init();
  }

#ifdef __EMSCRIPTEN__
  /* WebGL2 has no persistent mapping or glBufferStorage.
   * Allocate a read-back buffer and copy synchronously. */
  if (read_ssbo_id_ == 0) {
    glGenBuffers(1, &read_ssbo_id_);
    glBindBuffer(GL_COPY_WRITE_BUFFER, read_ssbo_id_);
    glBufferData(GL_COPY_WRITE_BUFFER, alloc_size_in_bytes_, nullptr, GL_STREAM_READ);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
  }

  glBindBuffer(GL_COPY_READ_BUFFER, ssbo_id_);
  glBindBuffer(GL_COPY_WRITE_BUFFER, read_ssbo_id_);
  glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, alloc_size_in_bytes_);
  glBindBuffer(GL_COPY_READ_BUFFER, 0);
  glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

  if (read_fence_) {
    glDeleteSync(read_fence_);
  }
  read_fence_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
#else
  if (read_ssbo_id_ == 0) {
    glGenBuffers(1, &read_ssbo_id_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, read_ssbo_id_);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER,
                    alloc_size_in_bytes_,
                    nullptr,
                    GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT);
    persistent_ptr_ = glMapBufferRange(GL_SHADER_STORAGE_BUFFER,
                                       0,
                                       alloc_size_in_bytes_,
                                       GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT);
    BLI_assert(persistent_ptr_);
    debug::object_label(GL_SHADER_STORAGE_BUFFER, read_ssbo_id_, name_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  if (GLContext::direct_state_access_support) {
    glCopyNamedBufferSubData(ssbo_id_, read_ssbo_id_, 0, 0, alloc_size_in_bytes_);
  }
  else {
    glBindBuffer(GL_COPY_READ_BUFFER, ssbo_id_);
    glBindBuffer(GL_COPY_WRITE_BUFFER, read_ssbo_id_);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, alloc_size_in_bytes_);
    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
  }

  glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);

  if (read_fence_) {
    glDeleteSync(read_fence_);
  }
  read_fence_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
#endif
}

void GLStorageBuf::read(void *data)
{
  if (data == nullptr) {
    return;
  }

#ifdef __EMSCRIPTEN__
  /* WebGL2 has no glGetBufferSubData or glGetNamedBufferSubData.
   * Use glMapBufferRange for synchronous readback. */
  if (read_fence_) {
    while (glClientWaitSync(read_fence_, GL_SYNC_FLUSH_COMMANDS_BIT, 1000) ==
           GL_TIMEOUT_EXPIRED)
    {
      /* Repeat until the data is ready. */
    }
    glDeleteSync(read_fence_);
    read_fence_ = nullptr;

    /* Read from the read-back buffer. */
    glBindBuffer(SSBO_TARGET, read_ssbo_id_);
    void *mapped = glMapBufferRange(SSBO_TARGET, 0, size_in_bytes_, GL_MAP_READ_BIT);
    if (mapped) {
      memcpy(data, mapped, size_in_bytes_);
      glUnmapBuffer(SSBO_TARGET);
    }
    glBindBuffer(SSBO_TARGET, 0);
  }
  else {
    /* Synchronous path: read directly from the main buffer. */
    glBindBuffer(SSBO_TARGET, ssbo_id_);
    void *mapped = glMapBufferRange(SSBO_TARGET, 0, size_in_bytes_, GL_MAP_READ_BIT);
    if (mapped) {
      memcpy(data, mapped, size_in_bytes_);
      glUnmapBuffer(SSBO_TARGET);
    }
    glBindBuffer(SSBO_TARGET, 0);
  }
#else
  if (!read_fence_) {
    /* Synchronous path. */
    if (GLContext::direct_state_access_support) {
      glGetNamedBufferSubData(ssbo_id_, 0, size_in_bytes_, data);
    }
    else {
      glBindBuffer(GL_COPY_READ_BUFFER, ssbo_id_);
      glGetBufferSubData(GL_COPY_READ_BUFFER, 0, size_in_bytes_, data);
      glBindBuffer(GL_COPY_READ_BUFFER, 0);
    }
    return;
  }

  while (glClientWaitSync(read_fence_, GL_SYNC_FLUSH_COMMANDS_BIT, 1000) == GL_TIMEOUT_EXPIRED) {
    /* Repeat until the data is ready. */
  }
  glDeleteSync(read_fence_);
  read_fence_ = nullptr;

  BLI_assert(persistent_ptr_);
  memcpy(data, persistent_ptr_, size_in_bytes_);
#endif
}

void GLStorageBuf::sync_as_indirect_buffer()
{
  bind_as(GL_DRAW_INDIRECT_BUFFER);
  glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

/** \} */

}  // namespace blender::gpu
