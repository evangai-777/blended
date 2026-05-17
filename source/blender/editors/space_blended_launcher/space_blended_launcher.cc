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
 * Phase 2: header chrome (wordmark + New/Open/Recent), rounded cards, hover state.
 */

#include "MEM_guardedalloc.h"

#include <algorithm>
#include <climits>
#include <cmath>

#include "BLI_listbase.h"
#include "BLI_rect.h"
#include "BLI_string_utf8.h"
#include "BLI_utildefines.h"

#include "BLF_api.hh"

#include "DNA_object_types.h"
#include "DNA_space_types.h"
#include "DNA_userdef_types.h"
#include "DNA_windowmanager_types.h"

#include "BKE_context.hh"
#include "BKE_main.hh"
#include "BKE_screen.hh"

#include "GPU_immediate.hh"
#include "GPU_state.hh"
#include "GPU_vertex_format.hh"

#include "ED_screen.hh"
#include "ED_space_api.hh"

#include "RNA_access.hh"
#include "RNA_define.hh"

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
/** \name Layout constants
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

/* BLENDED.md §11 surface hierarchy (#1D1D1D / #252525 / #2C2C2C). */
static constexpr float COL_BASE[4] = {0x1D / 255.0f, 0x1D / 255.0f, 0x1D / 255.0f, 1.0f};
static constexpr float COL_CARD[4] = {0x2C / 255.0f, 0x2C / 255.0f, 0x2C / 255.0f, 1.0f};
static constexpr float COL_CARD_HOVER[4] = {0x32 / 255.0f, 0x32 / 255.0f, 0x32 / 255.0f, 1.0f};
static constexpr float COL_CARD_ACTIVE[4] = {0x38 / 255.0f, 0x38 / 255.0f, 0x38 / 255.0f, 1.0f};
static constexpr float COL_CARD_ACTIVE_HOVER[4] = {0x3E / 255.0f, 0x3E / 255.0f, 0x3E / 255.0f, 1.0f};
static constexpr float COL_SEPARATOR[4] = {0x50 / 255.0f, 0x50 / 255.0f, 0x50 / 255.0f, 1.0f};
static constexpr float COL_HEADING[4] = {1.0f, 1.0f, 1.0f, 0.92f};
static constexpr float COL_SECTION[4] = {0.85f, 0.85f, 0.85f, 1.0f};
static constexpr float COL_MODE[4] = {0.78f, 0.78f, 0.78f, 1.0f};
static constexpr float COL_GROUP[4] = {0.45f, 0.45f, 0.45f, 1.0f};

/* Phase 2 placeholder accent — derives from logo render (Blender orange in the interim). */
static constexpr float COL_ACCENT[4] = {0xE8 / 255.0f, 0x7D / 255.0f, 0x0D / 255.0f, 1.0f};

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

/* Rounded rectangle fill using a triangle fan. radius ≤ min(w,h)/2. */
static void draw_rect_rounded(int x, int y, int w, int h, float radius, const float color[4])
{
  /* Clamp radius so it always fits. */
  radius = std::min(radius, std::min(float(w), float(h)) * 0.5f);
  if (radius < 1.0f) {
    draw_rect_filled(x, y, w, h, color);
    return;
  }

  /* Build the rounded quad as a triangle fan from the center.
   * 4 corners × SEGS segments each + closing vertex = 4*SEGS+1 verts + center. */
  constexpr int SEGS = 6; /* segments per corner — enough for 8px radius */
  const int n_corner_verts = 4 * SEGS;
  const int n_verts = 2 + n_corner_verts; /* center + ring + close */

  GPUVertFormat *fmt = immVertexFormat();
  const uint pos = GPU_vertformat_attr_add(fmt, "pos", gpu::VertAttrType::SFLOAT_32_32);

  immBindBuiltinProgram(GPU_SHADER_3D_UNIFORM_COLOR);
  immUniformColor4fv(color);
  immBegin(GPU_PRIM_TRI_FAN, n_verts);

  const float cx = float(x) + float(w) * 0.5f;
  const float cy = float(y) + float(h) * 0.5f;
  immVertex2f(pos, cx, cy);

  /* Corner centers (inner corners of the rounded rect). */
  const float cx0 = float(x) + radius;          /* left */
  const float cx1 = float(x + w) - radius;      /* right */
  const float cy0 = float(y) + radius;           /* bottom */
  const float cy1 = float(y + h) - radius;       /* top */

  /* Starting angle for each corner (going counter-clockwise from bottom-left). */
  const float start_angles[4] = {
      float(M_PI),        /* bottom-left  → 180° */
      float(M_PI) * 1.5f, /* bottom-right → 270° */
      0.0f,               /* top-right    →   0° */
      float(M_PI) * 0.5f, /* top-left     →  90° */
  };
  const float corner_cx[4] = {cx0, cx1, cx1, cx0};
  const float corner_cy[4] = {cy0, cy0, cy1, cy1};

  for (int c = 0; c < 4; c++) {
    for (int s = 0; s < SEGS; s++) {
      const float angle = start_angles[c] + float(s) / float(SEGS) * float(M_PI) * 0.5f;
      immVertex2f(pos, corner_cx[c] + radius * std::cos(angle),
                       corner_cy[c] + radius * std::sin(angle));
    }
  }
  /* Close the fan back to the first perimeter vertex. */
  {
    const float angle = start_angles[0];
    immVertex2f(pos, corner_cx[0] + radius * std::cos(angle),
                     corner_cy[0] + radius * std::sin(angle));
  }

  immEnd();
  immUnbindProgram();
}

/* 2px accent border around a rounded rect (drawn as four thin strips for simplicity). */
static void draw_rect_rounded_border(int x, int y, int w, int h, float radius, const float color[4])
{
  /* Top bar */
  draw_rect_filled(x + int(radius), y + h - 2, w - 2 * int(radius), 2, color);
  /* Bottom bar */
  draw_rect_filled(x + int(radius), y, w - 2 * int(radius), 2, color);
  /* Left bar */
  draw_rect_filled(x, y + int(radius), 2, h - 2 * int(radius), color);
  /* Right bar */
  draw_rect_filled(x + w - 2, y + int(radius), 2, h - 2 * int(radius), color);
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
/** \name Pipeline scroll renderer
 * \{ */

/* Returns the rcti of every mode button in layout order.
 * Used by both draw and hit-testing so geometry is computed once per pass. */
struct ButtonGeom {
  rcti rect;
  int target_space;
  bool active; /* section has project data */
};

/* Maximum mode buttons across all pipelines. */
static constexpr int MAX_BUTTONS = 64;

struct ButtonList {
  ButtonGeom items[MAX_BUTTONS];
  int count = 0;
  void push(rcti r, int sp, bool act)
  {
    if (count < MAX_BUTTONS) {
      items[count++] = {r, sp, act};
    }
  }
};

/* Shared layout traversal: populates `out` with geometry, returns final pen_y. */
static int compute_button_layout(const ARegion *region,
                                 float scroll_offset,
                                 const Main *bmain,
                                 ButtonList *out)
{
  const int win_w = region->winx;
  const int win_h = region->winy;
  const int content_w = MIN2(CONTENT_WIDTH, win_w - 48);
  const int content_x = (win_w - content_w) / 2;

  int pen_y = win_h - HEADING_TOP_PAD + int(scroll_offset);
  pen_y -= HEADING_FONT_SIZE + 28;

  const LauncherPipeline *groups[2] = {&g_creative, &g_post};
  for (const LauncherPipeline *grp : groups) {
    pen_y -= GROUP_GAP + GROUP_LABEL_FONT_SIZE + 6 + 18;

    for (int si = 0; grp->sections[si].label != nullptr; si++) {
      const LauncherSection &sec = grp->sections[si];
      pen_y -= SECTION_LABEL_FONT_SIZE + SECTION_LABEL_GAP;

      const bool active = bmain ? section_has_data(bmain, sec.label) : false;
      int bx = content_x;

      for (int mi = 0; sec.modes[mi].label != nullptr; mi++) {
        const LauncherMode &mode = sec.modes[mi];
        const int btn_w = int(text_width(MODE_FONT_SIZE, mode.label)) + BUTTON_PAD_X * 2;

        if (bx + btn_w > content_x + content_w && bx > content_x) {
          pen_y -= BUTTON_H + BUTTON_GAP;
          bx = content_x;
        }

        const rcti rect{bx, bx + btn_w, pen_y - BUTTON_H, pen_y};
        if (out) {
          out->push(rect, mode.target_space, active);
        }
        bx += btn_w + BUTTON_GAP;
      }
      pen_y -= BUTTON_H + SECTION_GAP;
    }
  }
  return pen_y;
}

static void draw_pipeline_scroll(const ARegion *region,
                                 float scroll_offset,
                                 const Main *bmain,
                                 int hover_x,
                                 int hover_y)
{
  const int win_w = region->winx;
  const int win_h = region->winy;

  const int content_w = MIN2(CONTENT_WIDTH, win_w - 48);
  const int content_x = (win_w - content_w) / 2;

  int pen_y = win_h - HEADING_TOP_PAD + int(scroll_offset);

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
        const bool hovered = BLI_rcti_isect_pt(&rect, hover_x, hover_y);

        const float *fill;
        if (active) {
          fill = hovered ? COL_CARD_ACTIVE_HOVER : COL_CARD_ACTIVE;
        }
        else {
          fill = hovered ? COL_CARD_HOVER : COL_CARD;
        }

        draw_rect_rounded(rect.xmin, rect.ymin, btn_w, BUTTON_H, 8.0f, fill);

        if (hovered) {
          draw_rect_rounded_border(rect.xmin, rect.ymin, btn_w, BUTTON_H, 8.0f, COL_ACCENT);
        }

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
  SpaceBlendedLauncher *sl = static_cast<SpaceBlendedLauncher *>(CTX_wm_space_data(C));
  const Main *bmain = CTX_data_main(C);

  const float scroll = sl ? sl->scroll_offset : 0.0f;

  /* Derive hover position from current event state so stale sl->mouse_x/y
   * values don't highlight cards when the cursor is in another region. */
  int mx = -1, my = -1;
  const wmWindow *win = CTX_wm_window(C);
  if (win && win->eventstate) {
    const int cx = win->eventstate->xy[0] - region->winrct.xmin;
    const int cy = win->eventstate->xy[1] - region->winrct.ymin;
    if (cx >= 0 && cx < region->winx && cy >= 0 && cy < region->winy) {
      mx = cx;
      my = cy;
    }
  }

  wmOrtho2_region_pixelspace(region);

  /* Base background fill */
  draw_rect_filled(0, 0, region->winx, region->winy, COL_BASE);

  draw_pipeline_scroll(region, scroll, bmain, mx, my);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Hit testing
 * \{ */

int launcher_mode_at_cursor(const ARegion *region, float scroll_offset, int cursor_x, int cursor_y)
{
  ButtonList bl;
  compute_button_layout(region, scroll_offset, nullptr, &bl);
  for (int i = 0; i < bl.count; i++) {
    if (BLI_rcti_isect_pt(&bl.items[i].rect, cursor_x, cursor_y)) {
      return bl.items[i].target_space;
    }
  }
  return SPACE_EMPTY;
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

  const SpaceBlendedLauncher *sl = static_cast<const SpaceBlendedLauncher *>(
      CTX_wm_space_data(C));
  const int cx = event->xy[0] - region->winrct.xmin;
  const int cy = event->xy[1] - region->winrct.ymin;
  const float scroll = sl ? sl->scroll_offset : 0.0f;
  const int target = launcher_mode_at_cursor(region, scroll, cx, cy);

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

/* --- LAUNCHER_OT_scroll --- */

static float launcher_max_scroll(const ARegion *region)
{
  /* Section count: 5 Creative + 3 Post = 8. Group count: 2. */
  const int content_h = HEADING_TOP_PAD + HEADING_FONT_SIZE + 28 +
                        2 * (GROUP_GAP + GROUP_LABEL_FONT_SIZE + 6 + 18) +
                        8 * (SECTION_LABEL_FONT_SIZE + SECTION_LABEL_GAP + BUTTON_H + SECTION_GAP) +
                        32;
  return float(MAX2(0, content_h - region->winy));
}

static wmOperatorStatus scroll_exec(bContext *C, wmOperator *op)
{
  ARegion *region = CTX_wm_region(C);
  ScrArea *area = CTX_wm_area(C);
  if (!region || !area) {
    return OPERATOR_CANCELLED;
  }
  SpaceBlendedLauncher *sl = static_cast<SpaceBlendedLauncher *>(area->spacedata.first);
  if (!sl) {
    return OPERATOR_CANCELLED;
  }

  const int delta = RNA_int_get(op->ptr, "delta");
  sl->scroll_offset += float(delta) * 60.0f;
  sl->scroll_offset = std::clamp(sl->scroll_offset, 0.0f, launcher_max_scroll(region));

  ED_region_tag_redraw(region);
  return OPERATOR_FINISHED;
}

static void LAUNCHER_OT_scroll(wmOperatorType *ot)
{
  ot->name = "Scroll Launcher";
  ot->idname = "LAUNCHER_OT_scroll";
  ot->description = "Scroll the launcher pipeline view";
  ot->exec = scroll_exec;
  ot->poll = activate_mode_poll;
  ot->flag = OPTYPE_INTERNAL;

  RNA_def_int(ot->srna, "delta", 0, INT_MIN, INT_MAX, "Delta", "Scroll steps (positive = down)", -100, 100);
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
  WM_operatortype_append(LAUNCHER_OT_scroll);
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

    /* Scroll down: content moves up, revealing lower pipeline sections. */
    {
      KeyMapItem_Params wp{};
      wp.type = WHEELDOWNMOUSE;
      wp.value = KM_PRESS;
      wp.modifier = 0;
      wp.direction = KM_ANY;
      wmKeyMapItem *kmi = WM_keymap_add_item(km, "LAUNCHER_OT_scroll", &wp);
      RNA_int_set(kmi->ptr, "delta", 1);
    }

    /* Scroll up: content moves down, revealing the heading. */
    {
      KeyMapItem_Params wp{};
      wp.type = WHEELUPMOUSE;
      wp.value = KM_PRESS;
      wp.modifier = 0;
      wp.direction = KM_ANY;
      wmKeyMapItem *kmi = WM_keymap_add_item(km, "LAUNCHER_OT_scroll", &wp);
      RNA_int_set(kmi->ptr, "delta", -1);
    }
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

  /* Header region — draws the file management chrome (wordmark, New, Open, Recent). */
  {
    ARegion *region = BKE_area_region_new();
    BLI_addtail(&sl->regionbase, region);
    region->regiontype = RGN_TYPE_HEADER;
    region->alignment = (U.uiflag & USER_HEADER_BOTTOM) ? RGN_ALIGN_BOTTOM : RGN_ALIGN_TOP;
  }

  /* Full-screen window region — pipeline scroll. */
  {
    ARegion *region = BKE_area_region_new();
    BLI_addtail(&sl->regionbase, region);
    region->regiontype = RGN_TYPE_WINDOW;
  }

  return reinterpret_cast<SpaceLink *>(sl);
}

static void launcher_free(SpaceLink * /*sl*/) {}

static void launcher_init(wmWindowManager * /*wm*/, ScrArea * /*area*/) {}

static SpaceLink *launcher_duplicate(SpaceLink *sl)
{
  return reinterpret_cast<SpaceLink *>(
      MEM_dupalloc(reinterpret_cast<SpaceBlendedLauncher *>(sl)));
}

/* ---- Header region ---- */

static void launcher_header_region_init(wmWindowManager * /*wm*/, ARegion *region)
{
  ED_region_header_init(region);
}

static void launcher_header_region_draw(const bContext *C, ARegion *region)
{
  ED_region_header(C, region);
}

static void launcher_header_region_listener(const wmRegionListenerParams *params)
{
  ARegion *region = params->region;
  const wmNotifier *wmn = params->notifier;

  switch (wmn->category) {
    case NC_WM:
    case NC_SCENE:
      ED_region_tag_redraw(region);
      break;
  }
}

/* ---- Main region ---- */

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
    case NC_WINDOW:
      /* Mouse move — update hover state and redraw. */
      if (wmn->action == NA_EDITED) {
        ED_region_tag_redraw(region);
      }
      break;
  }
}

/* Store the mouse position on every MOUSEMOVE event so draw_pipeline_scroll
 * can highlight the button under the cursor without a persistent timer. */
static void launcher_main_region_cursor(wmWindow *win, ScrArea *area, ARegion *region)
{
  /* Called by the region's cursor callback each frame the mouse is inside. */
  SpaceBlendedLauncher *sl = static_cast<SpaceBlendedLauncher *>(area->spacedata.first);
  if (!sl) {
    return;
  }

  const int mx = win->eventstate->xy[0] - region->winrct.xmin;
  const int my = win->eventstate->xy[1] - region->winrct.ymin;

  if (sl->mouse_x != mx || sl->mouse_y != my) {
    sl->mouse_x = mx;
    sl->mouse_y = my;
    ED_region_tag_redraw(region);
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
  art->cursor = launcher_main_region_cursor;
  BLI_addhead(&st->regiontypes, art);

  /* Header region — file management chrome via Python LAUNCHER_HT_header. */
  art = MEM_new_zeroed<ARegionType>("spacetype blended launcher header");
  art->regionid = RGN_TYPE_HEADER;
  art->prefsizey = HEADERY;
  art->keymapflag = ED_KEYMAP_UI | ED_KEYMAP_VIEW2D | ED_KEYMAP_HEADER;
  art->listener = launcher_header_region_listener;
  art->init = launcher_header_region_init;
  art->draw = launcher_header_region_draw;
  BLI_addhead(&st->regiontypes, art);

  BKE_spacetype_register(std::move(st));
}

/** \} */

}  // namespace blender
