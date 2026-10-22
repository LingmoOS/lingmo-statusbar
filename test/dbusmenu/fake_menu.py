#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Lingmo OS Team
#
# SPDX-License-Identifier: GPL-3.0-or-later
"""Fake com.canonical.dbusmenu service used by the headless client test.

Exports a small menu tree on org.lingmo.TestMenu /MenuBar and logs the calls it
receives, so test/dbusmenu/run.sh can check what DbusMenuClient sent.

Run it with the session bus available, e.g.:

    ./fake_menu.py &
    ./lingmo-statusbar-dbusmenu-test
"""

import sys

import dbus
import dbus.mainloop.glib
import dbus.service
from gi.repository import GLib

BUS_NAME = "org.lingmo.TestMenu"
OBJECT_PATH = "/MenuBar"
INTERFACE = "com.canonical.dbusmenu"
REVISION = 7


def make_item(item_id, label, children=(), item_type="standard", enabled=True):
    """One (i a{sv} av) layout entry; children are variants of the same struct."""
    properties = {
        "label": dbus.String(label, variant_level=1),
        "enabled": dbus.Boolean(enabled, variant_level=1),
        "visible": dbus.Boolean(True, variant_level=1),
        "type": dbus.String(item_type, variant_level=1),
    }
    if children:
        properties["children-display"] = dbus.String("submenu", variant_level=1)

    child_array = dbus.Array(
        [dbus.Struct(child, signature="(ia{sv}av)", variant_level=1) for child in children],
        signature="v",
    )
    return dbus.Struct(
        (dbus.Int32(item_id), dbus.Dictionary(properties, signature="sv"), child_array),
        signature="(ia{sv}av)",
    )


def build_tree():
    file_menu = make_item(
        1,
        "_File",
        children=[
            make_item(10, "_New"),
            make_item(11, "_Open"),
            make_item(12, "", item_type="separator"),
            make_item(13, "_Quit"),
        ],
    )
    edit_menu = make_item(2, "_Edit", children=[make_item(20, "Cu_t"), make_item(21, "_Copy")])
    view_menu = make_item(3, "_View")
    return [file_menu, edit_menu, view_menu]


class FakeMenu(dbus.service.Object):
    def __init__(self, bus):
        super().__init__(bus, OBJECT_PATH)
        self.tree = build_tree()

    @dbus.service.method(INTERFACE, in_signature="iias", out_signature="ua(ia{sv}av)")
    def GetLayout(self, parent_id, recursion_depth, property_names):
        print("GET_LAYOUT parent=%d depth=%d" % (int(parent_id), int(recursion_depth)), flush=True)
        return dbus.UInt32(REVISION), dbus.Array(self.tree, signature="(ia{sv}av)")

    @dbus.service.method(INTERFACE, in_signature="i", out_signature="b")
    def AboutToShow(self, item_id):
        print("ABOUT_TO_SHOW %d" % int(item_id), flush=True)
        return True

    @dbus.service.method(INTERFACE, in_signature="isvu")
    def Event(self, item_id, event_id, data, timestamp):
        print("EVENT %d %s" % (int(item_id), str(event_id)), flush=True)
        return None

    @dbus.service.signal(INTERFACE, signature="ui")
    def LayoutUpdated(self, revision, parent_id):
        return None


def main():
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(BUS_NAME, bus)
    menu = FakeMenu(bus)
    print("READY %s" % name.get_name(), flush=True)
    GLib.MainLoop().run()
    return 0


if __name__ == "__main__":
    sys.exit(main())
