# SPDX-FileCopyrightText: 2025 Blended Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Blended UI tier utilities.

Provides helpers for checking the current UI complexity tier.
Import in bl_ui modules to gate panels, menus, and operators by tier.

Usage::

    from blended_utils import tier_at_least, TIER_SIMPLE, TIER_STANDARD, TIER_ADVANCED

    class MY_PT_advanced_panel(Panel):
        @classmethod
        def poll(cls, context):
            return tier_at_least(context, TIER_ADVANCED)
"""

# Tier constants matching eUserPref_UITier in DNA_userdef_enums.h.
TIER_SIMPLE = 0
TIER_STANDARD = 1
TIER_ADVANCED = 2

# Human-readable names for each tier.
TIER_NAMES = {
    TIER_SIMPLE: "Simple",
    TIER_STANDARD: "Standard",
    TIER_ADVANCED: "Advanced",
}

# RNA enum properties return string identifiers, not integers.
# Map from the identifier defined in rna_userdef.cc to our int constants.
_TIER_FROM_ID = {
    'SIMPLE': TIER_SIMPLE,
    'STANDARD': TIER_STANDARD,
    'ADVANCED': TIER_ADVANCED,
}


def get_ui_tier(context):
    """Return the current UI tier as an integer (0/1/2)."""
    raw = context.preferences.view.ui_tier
    # RNA exposes PROP_ENUM as a string identifier; guard against future
    # changes that might return an integer directly.
    if isinstance(raw, str):
        return _TIER_FROM_ID.get(raw, TIER_STANDARD)
    return int(raw)


def tier_at_least(context, tier):
    """Return True if the current UI tier is at least *tier*."""
    return get_ui_tier(context) >= tier


def is_simple(context):
    """Return True if the UI is set to Simple tier."""
    return get_ui_tier(context) == TIER_SIMPLE


def is_standard(context):
    """Return True if the UI is set to Standard tier or higher."""
    return tier_at_least(context, TIER_STANDARD)


def is_advanced(context):
    """Return True if the UI is set to Advanced tier."""
    return tier_at_least(context, TIER_ADVANCED)
