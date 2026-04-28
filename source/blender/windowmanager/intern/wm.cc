/* SPDX-FileCopyrightText: 2007 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup wm
 *
 * Internal functions for managing UI registerable types (operator, UI and menu types).
 *
 * Also Blender's main event loop (WM_main).
 */

/* Allow using deprecated functionality for .blend file I/O. */
#define DNA_DEPRECATED_ALLOW

#include <cstring>

#include "DNA_ID_enums.h"
#include "DNA_layer_types.h"
#include "DNA_windowmanager_types.h"

#include "MEM_guardedalloc.h"

#include "BLI_ghash.h"
#include "BLI_listbase.h"
#include "BLI_string_utf8.h"
#include "BLI_utildefines.h"

#include "BLT_translation.hh"

#include "BKE_context.hh"
#include "BKE_global.hh"
#include "BKE_idprop.hh"
#include "BKE_idtype.hh"
#include "BKE_lib_id.hh"
#include "BKE_lib_query.hh"
#include "BKE_main.hh"
#include "BKE_report.hh"
#include "BKE_screen.hh"
#include "BKE_workspace.hh"

#include "WM_api.hh"
#include "WM_keymap.hh"
#include "WM_message.hh"
#include "WM_types.hh"
#include "wm.hh"
#include "wm_draw.hh"
#include "wm_event_system.hh"
#include "wm_window.hh"
#ifdef WITH_XR_OPENXR
#  include "wm_xr.hh"
#endif

#include "BKE_undo_system.hh"
#include "ED_screen.hh"

#ifdef WITH_PYTHON
#  include "BPY_extern.hh"
#  include "BPY_extern_run.hh"
#endif

#include "BLO_read_write.hh"

namespace blender {

/* ****************************************************** */


#define MAX_OP_REGISTERED 32

void WM_operator_free(wmOperator *op)
{

#ifdef WITH_PYTHON
  if (op->py_instance) {
    /* Do this first in case there are any __del__ functions or similar that use properties. */
    BPY_DECREF_RNA_INVALIDATE(op->py_instance);
  }
#endif

  if (op->ptr) {
    op->properties = static_cast<IDProperty *>(op->ptr->data);
    MEM_delete(op->ptr);
  }

  if (op->properties) {
    IDP_FreeProperty(op->properties);
  }

  if (op->reports && (op->reports->flag & RPT_FREE)) {
    BKE_reports_free(op->reports);
    MEM_delete(op->reports);
  }

  if (op->macro.first) {
    wmOperator *opm, *opmnext;
    for (opm = static_cast<wmOperator *>(op->macro.first); opm; opm = opmnext) {
      opmnext = opm->next;
      WM_operator_free(opm);
    }
  }

  MEM_delete(op);
}

void WM_operator_free_all_after(wmWindowManager *wm, wmOperator *op)
{
  op = op->next;
  while (op != nullptr) {
    wmOperator *op_next = op->next;
    BLI_remlink(&wm->runtime->operators, op);
    WM_operator_free(op);
    op = op_next;
  }
}

void WM_operator_type_set(wmOperator *op, wmOperatorType *ot)
{
  /* Not supported for Python. */
  BLI_assert(op->py_instance == nullptr);

  op->type = ot;
  op->ptr->type = ot->srna;

  /* Ensure compatible properties. */
  if (op->properties) {
    PointerRNA ptr = WM_operator_properties_create_ptr(ot);

    WM_operator_properties_default(&ptr, false);

    if (ptr.data) {
      IDP_SyncGroupTypes(op->properties, static_cast<const IDProperty *>(ptr.data), true);
    }

    WM_operator_properties_free(&ptr);
  }
}

static void wm_reports_free(wmWindowManager *wm)
{
  WM_event_timer_remove(wm, nullptr, wm->runtime->reports.reporttimer);
}

void wm_operator_register(bContext *C, wmOperator *op)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  int tot = 0;

  BLI_addtail(&wm->runtime->operators, op);

  /* Only count registered operators. */
  while (op) {
    wmOperator *op_prev = op->prev;
    if (op->type->flag & OPTYPE_REGISTER) {
      tot += 1;
    }
    if (tot > MAX_OP_REGISTERED) {
      BLI_remlink(&wm->runtime->operators, op);
      WM_operator_free(op);
    }
    op = op_prev;
  }

  /* So the console is redrawn. */
  WM_event_add_notifier(C, NC_SPACE | ND_SPACE_INFO_REPORT, nullptr);
  WM_event_add_notifier(C, NC_WM | ND_HISTORY, nullptr);
}

void WM_operator_stack_clear(wmWindowManager *wm)
{
  while (wmOperator *op = static_cast<wmOperator *>(BLI_pophead(&wm->runtime->operators))) {
    WM_operator_free(op);
  }

  WM_main_add_notifier(NC_WM | ND_HISTORY, nullptr);
}

void WM_operator_stack_clear(wmWindowManager *wm, const Set<wmOperatorType *> &types)
{
  bool any_removed = false;
  for (wmOperator &op : wm->runtime->operators.items_mutable()) {
    if (types.contains(op.type)) {
      BLI_remlink(&wm->runtime->operators, &op);
      WM_operator_free(&op);
      any_removed = true;
    }
  }

  if (any_removed) {
    WM_main_add_notifier(NC_WM | ND_HISTORY, nullptr);
  }
}

void WM_operator_handlers_clear(wmWindowManager *wm, const Set<wmOperatorType *> &types)
{
  for (wmWindow &win : wm->windows) {
    bScreen *screen = WM_window_get_active_screen(&win);
    for (ScrArea &area : screen->areabase) {
      switch (area.spacetype) {
        case SPACE_FILE: {
          SpaceFile *sfile = static_cast<SpaceFile *>(area.spacedata.first);
          if (sfile->op && types.contains(sfile->op->type)) {
            /* Freed as part of the handler. */
            sfile->op = nullptr;
          }
          break;
        }
      }
    }
  }

  for (wmWindow &win : wm->windows) {
    ListBaseT<wmEventHandler> *lb[2] = {&win.runtime->handlers, &win.runtime->modalhandlers};
    for (int i = 0; i < ARRAY_SIZE(lb); i++) {
      for (wmEventHandler &handler_base : *lb[i]) {
        if (handler_base.type == WM_HANDLER_TYPE_OP) {
          wmEventHandler_Op *handler = reinterpret_cast<wmEventHandler_Op *>(&handler_base);
          if (handler->op && types.contains(handler->op->type)) {
            /* Don't run op->cancel because it needs the context,
             * assume whoever unregisters the operator will cleanup. */
            handler->head.flag |= WM_HANDLER_DO_FREE;
            WM_operator_free(handler->op);
            handler->op = nullptr;
          }
        }
      }
    }
  }
}

void WM_operator_handlers_clear(wmWindowManager *wm, wmOperatorType *ot)
{
  WM_operator_handlers_clear(wm, Set<wmOperatorType *>{ot});
}

/* ****************************************** */

void WM_keyconfig_reload(bContext *C)
{
  if (CTX_py_init_get(C) && !G.background) {
#ifdef WITH_PYTHON
    const char *imports[] = {"bpy", nullptr};
    BPY_run_string_eval(C, imports, "bpy.utils.keyconfig_init()");
#endif
  }
}

void WM_keyconfig_init(bContext *C)
{
  wmWindowManager *wm = CTX_wm_manager(C);

  /* Create standard key configuration. */
  if (wm->runtime->defaultconf == nullptr) {
    /* Keep lowercase to match the preset filename. */
    wm->runtime->defaultconf = WM_keyconfig_new(wm, WM_KEYCONFIG_STR_DEFAULT, false);
  }
  if (wm->runtime->addonconf == nullptr) {
    wm->runtime->addonconf = WM_keyconfig_new(wm, WM_KEYCONFIG_STR_DEFAULT " addon", false);
  }
  if (wm->runtime->userconf == nullptr) {
    wm->runtime->userconf = WM_keyconfig_new(wm, WM_KEYCONFIG_STR_DEFAULT " user", false);
  }

  /* Initialize only after python init is done, for keymaps that use python operators. */
  if (CTX_py_init_get(C) && (wm->init_flag & WM_INIT_FLAG_KEYCONFIG) == 0) {
    /* Create default key config, only initialize once,
     * it's persistent across sessions. */
    if (!(wm->runtime->defaultconf->flag & KEYCONF_INIT_DEFAULT)) {
      wm_window_keymap(wm->runtime->defaultconf);
      ED_spacetypes_keymap(wm->runtime->defaultconf);

      WM_keyconfig_reload(C);

      wm->runtime->defaultconf->flag |= KEYCONF_INIT_DEFAULT;
    }

    /* Harmless, but no need to update in background mode. */
    if (!G.background) {
      WM_keyconfig_update_tag(nullptr, nullptr);
    }
    /* Don't call #WM_keyconfig_update here because add-ons have not yet been registered yet. */

    wm->init_flag |= WM_INIT_FLAG_KEYCONFIG;
  }
}

void WM_check(bContext *C)
{
  Main *bmain = CTX_data_main(C);
  wmWindowManager *wm = CTX_wm_manager(C);

  /* WM context. */
  if (wm == nullptr) {
    wm = static_cast<wmWindowManager *>(bmain->wm.first);
    CTX_wm_manager_set(C, wm);
  }

  if (wm == nullptr || BLI_listbase_is_empty(&wm->windows)) {
    return;
  }

  /* Run before loading the keyconfig. */
  if (wm->runtime->message_bus == nullptr) {
    wm->runtime->message_bus = WM_msgbus_create();
  }

  if (!G.background) {
    /* Case: file-read. */
    if ((wm->init_flag & WM_INIT_FLAG_WINDOW) == 0) {
      WM_keyconfig_init(C);
      WM_file_autosave_init(wm);
    }

    /* Case: no open windows at all, for old file reads. */
    wm_window_ghostwindows_ensure(wm);
  }

  /* Case: file-read. */
  /* NOTE: this runs in background mode to set the screen context cb. */
  if ((wm->init_flag & WM_INIT_FLAG_WINDOW) == 0) {
    ED_screens_init(C, bmain, wm);
    wm->init_flag |= WM_INIT_FLAG_WINDOW;
  }
}

void wm_clear_default_size(bContext *C)
{
  wmWindowManager *wm = CTX_wm_manager(C);

  /* WM context. */
  if (wm == nullptr) {
    wm = static_cast<wmWindowManager *>(CTX_data_main(C)->wm.first);
    CTX_wm_manager_set(C, wm);
  }

  if (wm == nullptr || BLI_listbase_is_empty(&wm->windows)) {
    return;
  }

  for (wmWindow &win : wm->windows) {
    win.sizex = 0;
    win.sizey = 0;
    win.posx = 0;
    win.posy = 0;
  }
}

void wm_add_default(Main *bmain, bContext *C)
{
  wmWindowManager *wm = static_cast<wmWindowManager *>(
      BKE_libblock_alloc(bmain, ID_WM_LEGACY, "WinMan", 0));
  wmWindow *win;
  bScreen *screen = CTX_wm_screen(C); /* XXX: from file read hrmf. */
  WorkSpace *workspace;
  WorkSpaceLayout *layout = BKE_workspace_layout_find_global(bmain, screen, &workspace);

  CTX_wm_manager_set(C, wm);
  win = wm_window_new(bmain, wm, nullptr, false);
  win->scene = CTX_data_scene(C);
  STRNCPY_UTF8(win->view_layer_name, CTX_data_view_layer(C)->name);
  BKE_workspace_active_set(win->workspace_hook, workspace);
  BKE_workspace_active_layout_set(win->workspace_hook, win->winid, workspace, layout);
  screen->winid = win->winid;

  wm->runtime = MEM_new<bke::WindowManagerRuntime>(__func__);
  wm->runtime->winactive = win;
  wm->file_saved = 1;
  wm_window_make_drawable(wm, win);
}

static void wm_xr_data_free(wmWindowManager *wm)
{
  /* NOTE: this also runs when built without `WITH_XR_OPENXR`.
   * It's necessary to prevent leaks when XR data is created or loaded into non XR builds.
   * This can occur when Python reads all properties (see the `bl_rna_paths` test). */

  /* Note that non-runtime data in `wm->xr` is freed as part of freeing the window manager. */
  if (wm->xr.session_settings.shading.prop) {
    IDP_FreeProperty(wm->xr.session_settings.shading.prop);
    wm->xr.session_settings.shading.prop = nullptr;
  }
}

void wm_close_and_free(bContext *C, wmWindowManager *wm)
{
  if (wm->autosavetimer) {
    wm_autosave_timer_end(wm);
  }

#ifdef WITH_XR_OPENXR
  /* May send notifier, so do before freeing notifier queue. */
  wm_xr_exit(wm);
#endif
  wm_xr_data_free(wm);

  while (wmWindow *win = static_cast<wmWindow *>(BLI_pophead(&wm->windows))) {
    /* Prevent draw clear to use screen. */
    BKE_workspace_active_set(win->workspace_hook, nullptr);
    wm_window_free(C, wm, win);
  }

#ifdef WITH_PYTHON
  BPY_callback_wm_free(wm);
#endif

  wm_reports_free(wm);

  if (C && CTX_wm_manager(C) == wm) {
    CTX_wm_manager_set(C, nullptr);
  }

  MEM_delete(wm->runtime);
}

void WM_main(bContext *C)
{
  /* Single refresh before handling events.
   * This ensures we don't run operators before the depsgraph has been evaluated. */
  wm_event_do_refresh_wm_and_depsgraph(C);

  while (true) {

    /* Get events from ghost, handle window events, add to window queues. */
    wm_window_events_process(C);

    /* Per window, all events to the window, screen, area and region handlers. */
    wm_event_do_handlers(C);

    /* Events have left notes about changes, we handle and cache it. */
    wm_event_do_notifiers(C);

    /* Execute cached changes draw. */
    wm_draw_update(C);
  }
}

}  // namespace blender
