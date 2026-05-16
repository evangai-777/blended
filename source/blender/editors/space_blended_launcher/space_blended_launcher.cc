/* SPDX-FileCopyrightText: 2026 CHJ 3 Productions LLC
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup spblendedlauncher
 *
 * Blended launcher — canonical workspace system.
 *
 * The pipeline scroll is the primary surface: "Blending?" heading,
 * Creative/Post sections, mode button cards per BLENDED.md §11.
 *
 * Phase 1 skeleton: SpaceType registration, full pipeline scroll draw,
 * mode button click -> space type switch.
 * Phase 2 applies visual identity (logo, color refinement, type stack).
 */

#include "MEM_guardedalloc.h"

#include "BLI_listbase.h"
#include "BLI_rect.h"
#include "BLI_string_utf8.h"
#include "BLI_utildefines.h"

#include "BLF_api.hh"

#include "DNA_object_types.h"
#include "DNA_space_types.h"
#include "DNA_windowmanager_types.h"

#include "BKE_context.hh"
#include "BKE_main.hh"
#include "BKE_screen.hh"

#include "GPU_immediate.hh"
#include "GPU_state.hh"
#include "GPU_vertex_format.hh"

#include "ED_screen.hh"
#include "ED_space_api.hh"

#include "WM_api.hh"
#include "WM_keymap.hh"
#include "WM_types.hh"

#include "BLO_read_write.hh"

#include "launcher_intern.hh"

namespace blender {

/* -------------------------------------------------------------------- */
/** \name Pipeline layout data (§12 section/mode table)
 * \{ */

struct LauncherMode {
  const char *label;
  int target_space; /* eSpace_Type — Phase 1 skeleton default */
};

struct LauncherSection {
  const char *label;
  LauncherMode modes[8]; /* null label terminates */
};

struct LauncherPipeline {
  const char *group_label; /* e.g. "╌╌  CREATIVE  ╌╌" */
  LauncherSection sections[6]; /* null label terminates */
};

/* §12 Creative sections */
static const LauncherPipeline g_creative = {
    "╌╌  CREATIVE  ╌╌",
    {{
         /* §12.1 — GP canvas lives in VIEW3D; timeline is a secondary area (Phase 2). */
         "Storyboarding",
         {{"Board", SPACE_VIEW3D}, {nullptr, 0}},
     },
     {
         /* §12.2 — All three modes use Grease Pencil, which operates in VIEW3D. */
         "2D Animation",
         {{"Animate", SPACE_VIEW3D}, {"Frame-by-Frame", SPACE_VIEW3D}, {"Paint", SPACE_VIEW3D}, {nullptr, 0}},
     },
     {
         "3D Animation",
         {{"Sculpt", SPACE_VIEW3D},
          {"Model", SPACE_VIEW3D},
          {"Rig", SPACE_VIEW3D},
          {"Environment", SPACE_VIEW3D},
          {"VFX", SPACE_VIEW3D},
          {"Animate", SPACE_VIEW3D},
          {nullptr, 0}},
     },
     {
         "Game",
         {{"Asset", SPACE_VIEW3D}, {"Level", SPACE_VIEW3D}, {"Bake", SPACE_VIEW3D}, {"Export", SPACE_FILE}, {nullptr, 0}},
     },
     {
         "Design",
         {{"Graphic", SPACE_IMAGE}, {"Illustration", SPACE_IMAGE}, {"Concept", SPACE_IMAGE}, {nullptr, 0}},
     },
     {nullptr, {}}}};

/* §12 Post sections */
static const LauncherPipeline g_post = {
    "╌╌  POST  ╌╌",
    {{
         "Finalizing",
         {{"Storyboard", SPACE_SEQ},
          {"2D", SPACE_SEQ},
          {"3D", SPACE_SEQ},
          {"Game", SPACE_SEQ},
          {"Design", SPACE_SEQ},
          {"Mixed", SPACE_SEQ},
          {nullptr, 0}},
     },
     {
         "Compositing",
         {{"Composite", SPACE_NODE}, {"Color", SPACE_NODE}, {"Cleanup", SPACE_NODE}, {nullptr, 0}},
     },
     {
         "Audio",
         {{"Mix", SPACE_SEQ}, {"Score", SPACE_SEQ}, {nullptr, 0}},
     },
     {nullptr, {}}}};

/** \} */

/* -------------------------------------------------------------------- */
/** \name Layout constants (Phase 1 skeleton values)
 * \{ */

static constexpr int HEADING_FONT_SIZE = 32;
static constexpr int GROUP_LABEL_FONT_SIZE = 11;
static constexpr int SECTION_LABEL_FONT_SIZE = 14;
static constexpr int MODE_FONT_SIZE = 12;

static constexpr int HEADING_TOP_PAD = 48;
static constexpr int GROUP_GAP = 32;
static constexpr int SECTION_GAP = 24;
static constexpr int BUTTON_H = 38;
static constexpr int BUTTON_PAD_X = 14;
static constexpr int BUTTON_GAP = 6;
static constexpr int SECTION_LABEL_GAP = 10;
static constexpr int CONTENT_WIDTH = 720;

/* BLENDED.md §11 surface hierarchy (#1D1D1D / #252525 / #2C2C2C) */
static constexpr float COL_BASE[4] = {0x1D / 255.0f, 0x1D / 255.0f, 0x1D / 255.0f, 1.0f};
static constexpr float COL_CARD[4] = {0x2C / 255.0f, 0x2C / 255.0f, 0x2C / 255.0f, 1.0f};
static constexpr float COL_SEPARATOR[4] = {0x50 / 255.0f, 0x50 / 255.0f, 0x50 / 255.0f, 1.0f};
static constexpr float COL_HEADING[4] = {1.0f, 1.0f, 1.0f, 0.92f};
static constexpr float COL_SECTION[4] = {0.85f, 0.85f, 0.85f, 1.0f};
static constexpr float COL_MODE[4] = {0.78f, 0.78f, 0.78f, 1.0f};
static constexpr float COL_GROUP[4] = {0.45f, 0.45f, 0.45f, 1.0f};
/* Slightly brighter card for sections that have project data. */
static constexpr float COL_CARD_ACTIVE[4] = {0x38 / 255.0f, 0x38 / 255.0f, 0x38 / 255.0f, 1.0f};

/** \} */

/* -------------------------------------------------------------------- */
/** \name Button cache (rebuilt each draw; used for click hit testing)
 * \{ */

struct ResolvedButton {
  rcti rect;
  int target_space; /* eSpace_Type */
};

struct ButtonCache {
  ResolvedButton items[64];
  int count = 0;

  void clear() { count = 0; }
  void push(rcti rect, int space)
  {
    if (count < 64) {
      items[count++] = {rect, space};
    }
  }
  int hit(int x, int y) const
  {
    for (int i = 0; i < count; i++) {
      if (BLI_rcti_isect_pt(&items[i].rect, x, y)) {
        return items[i].target_space;
      }
    }
    return SPACE_EMPTY;
  }
};

/* Per-draw cache. Single-threaded draw; rebuilt every frame. */
static ButtonCache g_buttons;

/** \} */

/* -------------------------------------------------------------------- */
/** \name Project state helpers
 * \{ */

static bool section_has_data(const Main *bmain, const char *section_label)
{
  if (!bmain) {
    return false;
  }
  if (STREQ(section_label, "3D Animation") || STREQ(section_label, "Game")) {
    LISTBASE_FOREACH (const Object *, ob, &bmain->objects) {
      if (ELEM(ob->type, OB_MESH, OB_ARMATURE)) {
        return true;
      }
    }
    return false;
  }
  if (STREQ(section_label, "2D Animation") || STREQ(section_label, "Storyboarding")) {
    LISTBASE_FOREACH (const Object *, ob, &bmain->objects) {
      if (ELEM(ob->type, OB_GPENCIL_LEGACY, OB_GREASE_PENCIL)) {
        return true;
      }
    }
    return false;
  }
  if (STREQ(section_label, "Design")) {
    return bmain->images.first != nullptr;
  }
  if (STREQ(section_label, "Compositing")) {
    return bmain->nodetrees.first != nullptr;
  }
  if (STREQ(section_label, "Audio")) {
    return bmain->sounds.first != nullptr;
  }
  if (STREQ(section_label, "Finalizing")) {
    return bmain->objects.first != nullptr;
  }
  return false;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name GPU draw helpers
 * \{ */

static void draw_rect_filled(int x, int y, int w, int h, const float color[4])
{
  GPUVertFormat *fmt = immVertexFormat();
  const uint pos = GPU_vertformat_attr_add(fmt, "pos", gpu::VertAttrType::SFLOAT_32_32);

  immBindBuiltinProgram(GPU_SHADER_3D_UNIFORM_COLOR);
  immUniformColor4fv(color);
  immBegin(GPU_PRIM_TRIS, 6);
  immVertex2f(pos, float(x), float(y));
  immVertex2f(pos, float(x + w), float(y));
  immVertex2f(pos, float(x + w), float(y + h));
  immVertex2f(pos, float(x), float(y));
  immVertex2f(pos, float(x + w), float(y + h));
  immVertex2f(pos, float(x), float(y + h));
  immEnd();
  immUnbindProgram();
}

static void draw_hrule(int x, int y, int w, const float color[4])
{
  draw_rect_filled(x, y, w, 1, color);
}

static void draw_text(int x, int y, float size, const float color[4], const char *text)
{
  const int font = BLF_default();
  BLF_size(font, size);
  BLF_color4fv(font, color);
  BLF_position(font, float(x), float(y), 0.0f);
  BLF_draw(font, text, BLF_DRAW_STR_DUMMY_MAX);
}

static float text_width(float size, const char *text)
{
  const int font = BLF_default();
  BLF_size(font, size);
  return BLF_width(font, text, BLF_DRAW_STR_DUMMY_MAX);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Pipeline scroll renderer
 * \{ */

static void draw_pipeline_scroll(const ARegion *region, float scroll_offset, const Main *bmain)
{
  const int win_w = region->winx;
  const int win_h = region->winy;

  const int content_w = MIN2(CONTENT_WIDTH, win_w - 48);
  const int content_x = (win_w - content_w) / 2;

  /* pen_y tracks the current vertical position (top-down).
   * BLF baseline is at pen_y so we descend by font size before drawing text. */
  int pen_y = win_h - HEADING_TOP_PAD + int(scroll_offset);

  g_buttons.clear();

  /* Heading */
  {
    const char *heading = "Blending?";
    const float fw = text_width(HEADING_FONT_SIZE, heading);
    pen_y -= HEADING_FONT_SIZE;
    draw_text(content_x + int((content_w - fw) * 0.5f),
              pen_y,
              HEADING_FONT_SIZE,
              COL_HEADING,
              heading);
    pen_y -= 28;
  }

  const LauncherPipeline *groups[2] = {&g_creative, &g_post};

  for (const LauncherPipeline *grp : groups) {
    /* Group separator */
    pen_y -= GROUP_GAP;
    {
      const float fw = text_width(GROUP_LABEL_FONT_SIZE, grp->group_label);
      pen_y -= GROUP_LABEL_FONT_SIZE;
      draw_text(content_x + int((content_w - fw) * 0.5f),
                pen_y,
                GROUP_LABEL_FONT_SIZE,
                COL_GROUP,
                grp->group_label);
      pen_y -= 6;
      draw_hrule(content_x, pen_y, content_w, COL_SEPARATOR);
      pen_y -= 18;
    }

    /* Sections */
    for (int si = 0; grp->sections[si].label != nullptr; si++) {
      const LauncherSection &sec = grp->sections[si];

      /* Section label */
      pen_y -= SECTION_LABEL_FONT_SIZE;
      draw_text(content_x, pen_y, SECTION_LABEL_FONT_SIZE, COL_SECTION, sec.label);
      pen_y -= SECTION_LABEL_GAP;

      /* Mode buttons — left-to-right, wrapping */
      int bx = content_x;
      const bool active = section_has_data(bmain, sec.label);

      for (int mi = 0; sec.modes[mi].label != nullptr; mi++) {
        const LauncherMode &mode = sec.modes[mi];
        const int btn_w = int(text_width(MODE_FONT_SIZE, mode.label)) + BUTTON_PAD_X * 2;

        /* Wrap if needed */
        if (bx + btn_w > content_x + content_w && bx > content_x) {
          pen_y -= BUTTON_H + BUTTON_GAP;
          bx = content_x;
        }

        const rcti rect{bx, bx + btn_w, pen_y - BUTTON_H, pen_y};
        g_buttons.push(rect, mode.target_space);
        draw_rect_filled(rect.xmin, rect.ymin, btn_w, BUTTON_H, active ? COL_CARD_ACTIVE : COL_CARD);

        /* Label centered vertically in button */
        const int text_y = rect.ymin + (BUTTON_H - MODE_FONT_SIZE) / 2;
        draw_text(bx + BUTTON_PAD_X, text_y, MODE_FONT_SIZE, COL_MODE, mode.label);

        bx += btn_w + BUTTON_GAP;
      }

      pen_y -= BUTTON_H + SECTION_GAP;
    }
  }
}

void launcher_main_region_draw(const bContext *C, ARegion *region)
{
  const SpaceBlendedLauncher *sl = static_cast<const SpaceBlendedLauncher *>(
      CTX_wm_space_data(C));
  const Main *bmain = CTX_data_main(C);

  const float scroll = sl ? sl->scroll_offset : 0.0f;

  /* Set up 2D pixel-space ortho projection for this region. */
  wmOrtho2_region_pixelspace(region);

  /* Base background fill */
  draw_rect_filled(0, 0, region->winx, region->winy, COL_BASE);

  draw_pipeline_scroll(region, scroll, bmain);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Hit testing
 * \{ */

int launcher_mode_at_cursor(const ARegion * /*region*/, int cursor_x, int cursor_y)
{
  return g_buttons.hit(cursor_x, cursor_y);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Operators
 * \{ */

static wmOperatorStatus activate_mode_invoke(bContext *C,
                                               wmOperator * /*op*/,
                                               const wmEvent *event)
{
  ARegion *region = CTX_wm_region(C);
  ScrArea *area = CTX_wm_area(C);
  if (!region || !area) {
    return OPERATOR_CANCELLED;
  }

  const int cx = event->xy[0] - region->winrct.xmin;
  const int cy = event->xy[1] - region->winrct.ymin;
  const int target = launcher_mode_at_cursor(region, cx, cy);

  if (target == SPACE_EMPTY) {
    return OPERATOR_CANCELLED;
  }

  ED_area_newspace(C, area, target, true);
  return OPERATOR_FINISHED;
}

static bool activate_mode_poll(bContext *C)
{
  const ScrArea *area = CTX_wm_area(C);
  return area && area->spacetype == SPACE_BLENDED_LAUNCHER;
}

static void LAUNCHER_OT_activate_mode(wmOperatorType *ot)
{
  ot->name = "Activate Mode";
  ot->idname = "LAUNCHER_OT_activate_mode";
  ot->description = "Open the selected pipeline mode editor";
  ot->invoke = activate_mode_invoke;
  ot->poll = activate_mode_poll;
  ot->flag = OPTYPE_INTERNAL;
}

/* --- LAUNCHER_OT_open --- */

static wmOperatorStatus open_launcher_exec(bContext *C, wmOperator * /*op*/)
{
  ScrArea *area = CTX_wm_area(C);
  if (!area || area->spacetype == SPACE_BLENDED_LAUNCHER) {
    return OPERATOR_CANCELLED;
  }
  ED_area_newspace(C, area, SPACE_BLENDED_LAUNCHER, true);
  return OPERATOR_FINISHED;
}

static void LAUNCHER_OT_open(wmOperatorType *ot)
{
  ot->name = "Open Launcher";
  ot->idname = "LAUNCHER_OT_open";
  ot->description = "Return to the Blended launcher (canonical workspace system)";
  ot->exec = open_launcher_exec;
  ot->flag = OPTYPE_INTERNAL;
}

void launcher_operatortypes()
{
  WM_operatortype_append(LAUNCHER_OT_activate_mode);
  WM_operatortype_append(LAUNCHER_OT_open);
}

void launcher_keymap(wmKeyConfig *keyconf)
{
  /* Launcher-area keymap: left-click activates a mode button. */
  {
    wmKeyMap *km = WM_keymap_ensure(
        keyconf, "Blended Launcher", SPACE_BLENDED_LAUNCHER, RGN_TYPE_WINDOW);

    KeyMapItem_Params params{};
    params.type = LEFTMOUSE;
    params.value = KM_PRESS;
    params.modifier = 0;
    params.direction = KM_ANY;
    WM_keymap_add_item(km, "LAUNCHER_OT_activate_mode", &params);
  }

  /* Global window keymap: Ctrl+Alt+Home opens the launcher from any area. */
  {
    wmKeyMap *km = WM_keymap_ensure(keyconf, "Window", SPACE_EMPTY, RGN_TYPE_WINDOW);

    KeyMapItem_Params params{};
    params.type = EVT_HOMEKEY;
    params.value = KM_PRESS;
    params.modifier = KM_CTRL | KM_ALT;
    params.direction = KM_ANY;
    WM_keymap_add_item(km, "LAUNCHER_OT_open", &params);
  }
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name SpaceType lifecycle callbacks
 * \{ */

static SpaceLink *launcher_create(const ScrArea * /*area*/, const Scene * /*scene*/)
{
  SpaceBlendedLauncher *sl = MEM_new<SpaceBlendedLauncher>("init blended launcher");
  sl->spacetype = SPACE_BLENDED_LAUNCHER;

  /* Single full-screen window region — no header, no sidebar. */
  ARegion *region = BKE_area_region_new();
  BLI_addtail(&sl->regionbase, region);
  region->regiontype = RGN_TYPE_WINDOW;

  return reinterpret_cast<SpaceLink *>(sl);
}

static void launcher_free(SpaceLink * /*sl*/) {}

static void launcher_init(wmWindowManager * /*wm*/, ScrArea * /*area*/) {}

static SpaceLink *launcher_duplicate(SpaceLink *sl)
{
  return reinterpret_cast<SpaceLink *>(
      MEM_dupalloc(reinterpret_cast<SpaceBlendedLauncher *>(sl)));
}

static void launcher_main_region_init(wmWindowManager *wm, ARegion *region)
{
  wmKeyMap *km = WM_keymap_ensure(
      wm->runtime->defaultconf, "Blended Launcher", SPACE_BLENDED_LAUNCHER, RGN_TYPE_WINDOW);
  WM_event_add_keymap_handler(&region->runtime->handlers, km);
}

static void launcher_main_region_listener(const wmRegionListenerParams *params)
{
  ARegion *region = params->region;
  const wmNotifier *wmn = params->notifier;

  switch (wmn->category) {
    case NC_SCENE:
    case NC_WM:
      ED_region_tag_redraw(region);
      break;
  }
}

static void launcher_space_blend_write(BlendWriter *writer, SpaceLink *sl)
{
  writer->write_struct_cast<SpaceBlendedLauncher>(sl);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Space type registration
 * \{ */

void ED_spacetype_blended_launcher()
{
  std::unique_ptr<SpaceType> st = std::make_unique<SpaceType>();
  ARegionType *art;

  st->spaceid = SPACE_BLENDED_LAUNCHER;
  STRNCPY_UTF8(st->name, "Blended Launcher");

  st->create = launcher_create;
  st->free = launcher_free;
  st->init = launcher_init;
  st->duplicate = launcher_duplicate;
  st->operatortypes = launcher_operatortypes;
  st->keymap = launcher_keymap;
  st->blend_write = launcher_space_blend_write;

  /* Full-screen window region — custom draw, no view2d. */
  art = MEM_new_zeroed<ARegionType>("spacetype blended launcher region");
  art->regionid = RGN_TYPE_WINDOW;
  art->keymapflag = ED_KEYMAP_UI;
  art->init = launcher_main_region_init;
  art->draw = launcher_main_region_draw;
  art->listener = launcher_main_region_listener;
  BLI_addhead(&st->regiontypes, art);

  BKE_spacetype_register(std::move(st));
}

/** \} */

}  // namespace blender
