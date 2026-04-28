/* SPDX-FileCopyrightText: 2023 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup bke
 *
 * WorkSpace runtime management. IDTypeInfo and Main::workspaces list have been
 * removed (WorkSpace is no longer project data). This file keeps the instance-hook
 * accessors, layout management, and tool helpers that are still needed at runtime.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "DNA_listBase.h"

#include "BLI_listbase.h"
#include "BLI_string.h"
#include "BLI_string_utils.hh"
#include "BLI_utildefines.h"

#include "BKE_asset.hh"
#include "BKE_global.hh"
#include "BKE_idprop.hh"
#include "BKE_lib_id.hh"
#include "BKE_main.hh"
#include "BKE_screen.hh"
#include "BKE_viewer_path.hh"
#include "BKE_workspace.hh"

#include "DNA_scene_types.h"
#include "DNA_screen_types.h"
#include "DNA_windowmanager_types.h"
#include "DNA_workspace_types.h"

#include "MEM_guardedalloc.h"

namespace blender {

/* -------------------------------------------------------------------- */
/** \name Internal helpers
 * \{ */

static void workspace_layout_name_set(WorkSpace *workspace,
                                      WorkSpaceLayout *layout,
                                      const char *new_name)
{
  STRNCPY(layout->name, new_name);
  BLI_uniquename(&workspace->layouts,
                 layout,
                 "Layout",
                 '.',
                 offsetof(WorkSpaceLayout, name),
                 sizeof(layout->name));
}

static WorkSpaceLayout *workspace_layout_find_exec(const WorkSpace *workspace,
                                                   const bScreen *screen)
{
  return static_cast<WorkSpaceLayout *>(
      BLI_findptr(&workspace->layouts, screen, offsetof(WorkSpaceLayout, screen)));
}

static void workspace_relation_add(ListBaseT<WorkSpaceDataRelation> *relation_list,
                                   void *parent,
                                   const int parentid,
                                   void *data)
{
  WorkSpaceDataRelation *relation = MEM_new<WorkSpaceDataRelation>(__func__);
  relation->parent = parent;
  relation->parentid = parentid;
  relation->value = data;
  BLI_addhead(relation_list, relation);
}

static void workspace_relation_remove(ListBaseT<WorkSpaceDataRelation> *relation_list,
                                      WorkSpaceDataRelation *relation)
{
  BLI_remlink(relation_list, relation);
  MEM_delete(relation);
}

static void workspace_relation_ensure_updated(ListBaseT<WorkSpaceDataRelation> *relation_list,
                                              void *parent,
                                              const int parentid,
                                              void *data)
{
  WorkSpaceDataRelation *relation = static_cast<WorkSpaceDataRelation *>(BLI_listbase_bytes_find(
      relation_list, &parentid, sizeof(parentid), offsetof(WorkSpaceDataRelation, parentid)));
  if (relation != nullptr) {
    relation->parent = parent;
    relation->value = data;
    BLI_remlink(relation_list, relation);
    BLI_addhead(relation_list, relation);
  }
  else {
    workspace_relation_add(relation_list, parent, parentid, data);
  }
}

static void *workspace_relation_get_data_matching_parent(
    const ListBaseT<WorkSpaceDataRelation> *relation_list, const void *parent)
{
  WorkSpaceDataRelation *relation = static_cast<WorkSpaceDataRelation *>(
      BLI_findptr(relation_list, parent, offsetof(WorkSpaceDataRelation, parent)));
  return relation ? relation->value : nullptr;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Create, Delete, Init
 * \{ */

WorkSpace *BKE_workspace_add(Main * /*bmain*/, const char *name)
{
  WorkSpace *workspace = MEM_new<WorkSpace>(__func__);
  workspace->runtime = MEM_new<bke::WorkSpaceRuntime>(__func__);
  BLI_strncpy(workspace->id.name + 2, name, sizeof(workspace->id.name) - 2);
  return workspace;
}

void BKE_workspace_remove(Main *bmain, WorkSpace *workspace)
{
  while (WorkSpaceLayout *layout = static_cast<WorkSpaceLayout *>(workspace->layouts.first)) {
    BKE_workspace_layout_remove(bmain, workspace, layout);
  }
  BKE_workspace_relations_free(&workspace->hook_layout_relations);
  MEM_delete(workspace->runtime);
  MEM_delete(workspace);
}

WorkSpaceInstanceHook *BKE_workspace_instance_hook_create(const Main * /*bmain*/,
                                                          const int /*winid*/)
{
  return MEM_new<WorkSpaceInstanceHook>(__func__);
}

void BKE_workspace_instance_hook_free(const Main * /*bmain*/, WorkSpaceInstanceHook *hook)
{
  MEM_delete(hook);
}

WorkSpaceLayout *BKE_workspace_layout_add(Main *bmain,
                                          WorkSpace &workspace,
                                          bScreen &screen,
                                          const char *name)
{
  WorkSpaceLayout *layout = MEM_new<WorkSpaceLayout>(__func__);

#ifdef NDEBUG
  UNUSED_VARS(bmain);
#endif
  layout->screen = &screen;
  id_us_plus(&layout->screen->id);
  workspace_layout_name_set(&workspace, layout, name);
  BLI_addtail(&workspace.layouts, layout);

  return layout;
}

WorkSpaceLayout *BKE_workspace_layout_add_from_layout(Main *bmain,
                                                      WorkSpace &workspace_dst,
                                                      const WorkSpaceLayout &layout_src,
                                                      const int id_copy_flags)
{
  bScreen *screen_src = BKE_workspace_layout_screen_get(&layout_src);
  const char *name = BKE_workspace_layout_name_get(&layout_src);

  if (BKE_screen_is_fullscreen_area(screen_src)) {
    for (ScrArea &area_old : screen_src->areabase) {
      if (area_old.full && area_old.full != screen_src) {
        screen_src = area_old.full;
        break;
      }
    }
  }

  bScreen *screen_dst = id_cast<bScreen *>(
      BKE_id_copy_ex(bmain, &screen_src->id, nullptr, id_copy_flags));

  return BKE_workspace_layout_add(bmain, workspace_dst, *screen_dst, name);
}

void BKE_workspace_layout_remove(Main *bmain, WorkSpace *workspace, WorkSpaceLayout *layout)
{
  if (layout->screen) {
    id_us_min(&layout->screen->id);
    BKE_id_free(bmain, layout->screen);
  }
  BLI_freelinkN(&workspace->layouts, layout);
}

void BKE_workspace_relations_free(ListBaseT<WorkSpaceDataRelation> *relation_list)
{
  for (WorkSpaceDataRelation *relation =
           static_cast<WorkSpaceDataRelation *>(relation_list->first),
                             *relation_next;
       relation;
       relation = relation_next)
  {
    relation_next = relation->next;
    workspace_relation_remove(relation_list, relation);
  }
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name General Utils
 * \{ */

WorkSpaceLayout *BKE_workspace_layout_find(const WorkSpace *workspace, const bScreen *screen)
{
  WorkSpaceLayout *layout = workspace_layout_find_exec(workspace, screen);
  if (layout) {
    return layout;
  }

  printf("%s: Couldn't find layout in workspace '%s' for screen '%s'.\n",
         __func__,
         workspace->id.name + 2,
         screen->id.name + 2);
  return nullptr;
}

WorkSpaceLayout *BKE_workspace_layout_find_global(const Main * /*bmain*/,
                                                  const bScreen * /*screen*/,
                                                  WorkSpace **r_workspace)
{
  /* WorkSpace is no longer stored in Main — cannot search globally. */
  if (r_workspace) {
    *r_workspace = nullptr;
  }
  return nullptr;
}

WorkSpaceLayout *BKE_workspace_layout_iter_circular(const WorkSpace *workspace,
                                                    WorkSpaceLayout *start,
                                                    bool (*callback)(const WorkSpaceLayout *layout,
                                                                     void *arg),
                                                    void *arg,
                                                    const bool iter_backward)
{
  WorkSpaceLayout *iter_layout;

  if (iter_backward) {
    LISTBASE_CIRCULAR_BACKWARD_BEGIN (WorkSpaceLayout *, &workspace->layouts, iter_layout, start) {
      if (!callback(iter_layout, arg)) {
        return iter_layout;
      }
    }
    LISTBASE_CIRCULAR_BACKWARD_END(WorkSpaceLayout *, &workspace->layouts, iter_layout, start);
  }
  else {
    LISTBASE_CIRCULAR_FORWARD_BEGIN (WorkSpaceLayout *, &workspace->layouts, iter_layout, start) {
      if (!callback(iter_layout, arg)) {
        return iter_layout;
      }
    }
    LISTBASE_CIRCULAR_FORWARD_END(WorkSpaceLayout *, &workspace->layouts, iter_layout, start);
  }

  return nullptr;
}

void BKE_workspace_tool_remove(WorkSpace *workspace, bToolRef *tref)
{
  if (tref->runtime) {
    MEM_delete(tref->runtime);
  }
  if (tref->properties) {
    IDP_FreeProperty(tref->properties);
  }
  BLI_remlink(&workspace->tools, tref);
  MEM_delete(tref);
}

void BKE_workspace_tool_id_replace_table(WorkSpace *workspace,
                                         const int space_type,
                                         const int mode,
                                         const char *idname_prefix_skip,
                                         const char *replace_table[][2],
                                         int replace_table_num)
{
  const size_t idname_prefix_len = idname_prefix_skip ? strlen(idname_prefix_skip) : 0;
  const size_t idname_suffix_maxncpy = sizeof(bToolRef::idname) - idname_prefix_len;

  for (bToolRef &tref : workspace->tools) {
    if (!(tref.space_type == space_type && tref.mode == mode)) {
      continue;
    }
    char *idname_suffix = tref.idname;
    if (idname_prefix_skip) {
      if (!STRPREFIX(idname_suffix, idname_prefix_skip)) {
        continue;
      }
      idname_suffix += idname_prefix_len;
    }
    BLI_string_replace_table_exact(
        idname_suffix, idname_suffix_maxncpy, replace_table, replace_table_num);
  }
}

bool BKE_workspace_owner_id_check(const WorkSpace *workspace, const char *owner_id)
{
  if ((*owner_id == '\0') || ((workspace->flags & WORKSPACE_USE_FILTER_BY_ORIGIN) == 0)) {
    return true;
  }
  return BLI_findstring(&workspace->owner_ids, owner_id, offsetof(wmOwnerID, name)) != nullptr;
}

void BKE_workspace_id_tag_all_visible(Main *bmain, int tag)
{
  wmWindowManager *wm = static_cast<wmWindowManager *>(bmain->wm.first);
  if (!wm) {
    return;
  }
  for (wmWindow &win : wm->windows) {
    WorkSpace *workspace = BKE_workspace_active_get(win.workspace_hook);
    if (workspace) {
      workspace->id.tag |= tag;
    }
  }
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Getters/Setters
 * \{ */

WorkSpace *BKE_workspace_active_get(WorkSpaceInstanceHook *hook)
{
  return hook->active;
}

void BKE_workspace_active_set(WorkSpaceInstanceHook *hook, WorkSpace *workspace)
{
  hook->active = workspace;
  if (workspace) {
    WorkSpaceLayout *layout = static_cast<WorkSpaceLayout *>(
        workspace_relation_get_data_matching_parent(&workspace->hook_layout_relations, hook));
    if (layout) {
      hook->act_layout = layout;
    }
  }
}

WorkSpaceLayout *BKE_workspace_active_layout_get(const WorkSpaceInstanceHook *hook)
{
  return hook->act_layout;
}

WorkSpaceLayout *BKE_workspace_active_layout_for_workspace_get(const WorkSpaceInstanceHook *hook,
                                                               const WorkSpace *workspace)
{
  if (hook->active == workspace) {
    return hook->act_layout;
  }
  return static_cast<WorkSpaceLayout *>(
      workspace_relation_get_data_matching_parent(&workspace->hook_layout_relations, hook));
}

void BKE_workspace_active_layout_set(WorkSpaceInstanceHook *hook,
                                     const int winid,
                                     WorkSpace *workspace,
                                     WorkSpaceLayout *layout)
{
  hook->act_layout = layout;
  workspace_relation_ensure_updated(&workspace->hook_layout_relations, hook, winid, layout);
}

bScreen *BKE_workspace_active_screen_get(const WorkSpaceInstanceHook *hook)
{
  return hook->act_layout->screen;
}

void BKE_workspace_active_screen_set(WorkSpaceInstanceHook *hook,
                                     const int winid,
                                     WorkSpace *workspace,
                                     bScreen *screen)
{
  WorkSpaceLayout *layout = BKE_workspace_layout_find(hook->active, screen);
  BKE_workspace_active_layout_set(hook, winid, workspace, layout);
}

const char *BKE_workspace_layout_name_get(const WorkSpaceLayout *layout)
{
  return layout->name;
}

void BKE_workspace_layout_name_set(WorkSpace *workspace,
                                   WorkSpaceLayout *layout,
                                   const char *new_name)
{
  workspace_layout_name_set(workspace, layout, new_name);
}

bScreen *BKE_workspace_layout_screen_get(const WorkSpaceLayout *layout)
{
  return layout->screen;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Status
 * \{ */

void BKE_workspace_status_clear(WorkSpace *workspace)
{
  workspace->runtime->status.clear_and_shrink();
}

/** \} */

}  // namespace blender
