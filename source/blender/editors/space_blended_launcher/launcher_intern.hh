/* SPDX-FileCopyrightText: 2026 CHJ 3 Productions LLC
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/** \file
 * \ingroup spblendedlauncher
 */

struct ARegion;
struct wmKeyConfig;
struct wmOperatorType;

namespace blender {

/** Operator registration. */
void launcher_operatortypes();

/** Keymap registration. */
void launcher_keymap(wmKeyConfig *keyconf);

/** Region draw. */
void launcher_main_region_draw(const bContext *C, ARegion *region);

/** Returns the target eSpace_Type for a launcher mode button at cursor position (x, y)
 *  relative to the region window. Returns SPACE_EMPTY if no button was hit.
 *  scroll_offset is taken from the space struct (not the region) so the hit geometry
 *  matches the draw geometry exactly, regardless of which launcher instance drew last. */
int launcher_mode_at_cursor(const ARegion *region, float scroll_offset, int cursor_x, int cursor_y);

}  // namespace blender
