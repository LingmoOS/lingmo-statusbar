#!/bin/bash
# SPDX-FileCopyrightText: 2026 Lingmo OS Team
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Headless check of src/appmenu/dbusmenuclient.cpp: a fake
# com.canonical.dbusmenu service (fake_menu.py) is started on the session bus and
# the client is asked to parse it. No display, no DTK, no X11 involved.
#
#   ./test/dbusmenu/run.sh
#
# Needs: python3-dbus, python3-gi, qt6-base-dev (moc), a session bus.

set -u

HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../.." && pwd)
BUILD=${TMPDIR:-/tmp}/lingmo-statusbar-dbusmenu-test
MOC=/usr/lib/qt6/libexec/moc
QTI=/usr/include/x86_64-linux-gnu/qt6

if [ ! -x "$MOC" ]; then
    echo "moc not found at $MOC (qt6-base-dev-tools?)"
    exit 77
fi

rm -rf "$BUILD"
mkdir -p "$BUILD" || exit 77

echo "=== building the test client ==="
"$MOC" -I"$QTI" -I"$QTI/QtCore" -I"$QTI/QtDBus" \
    "$REPO/src/appmenu/dbusmenuclient.h" -o "$BUILD/moc_dbusmenuclient.cpp" || exit 1

g++ -std=c++17 -fPIC -Wall -o "$BUILD/menutest" \
    "$HERE/menutest.cpp" "$REPO/src/appmenu/dbusmenuclient.cpp" "$BUILD/moc_dbusmenuclient.cpp" \
    -I"$QTI" -I"$QTI/QtCore" -I"$QTI/QtDBus" -I"$REPO/src" \
    -lQt6Core -lQt6DBus || exit 1

echo "=== starting the fake menu service ==="
python3 "$HERE/fake_menu.py" > "$BUILD/fake.log" 2>&1 &
FAKE_PID=$!
cleanup() {
    kill "$FAKE_PID" 2>/dev/null
    wait "$FAKE_PID" 2>/dev/null
}
trap cleanup EXIT

for _ in $(seq 1 50); do
    grep -q '^READY' "$BUILD/fake.log" && break
    sleep 0.1
done
if ! grep -q '^READY' "$BUILD/fake.log"; then
    echo "fake menu service did not start:"
    cat "$BUILD/fake.log"
    exit 1
fi

echo "=== running the client ==="
"$BUILD/menutest" > "$BUILD/client.log" 2>&1
CLIENT_EXIT=$?
sleep 0.3

cleanup
trap - EXIT

echo "--- client output ---"
cat "$BUILD/client.log"
echo "--- fake service output ---"
cat "$BUILD/fake.log"

echo "=== assertions ==="
FAIL=0
check() {
    if grep -qF "$1" "$2"; then
        echo "ok   $1"
    else
        echo "FAIL missing in $(basename "$2"): $1"
        FAIL=1
    fi
}

check "AVAILABLE yes" "$BUILD/client.log"
check "TOP 1|File|-|submenu" "$BUILD/client.log"
check "TOP 2|Edit|-|submenu" "$BUILD/client.log"
check "TOP 3|View|-|-" "$BUILD/client.log"
check "CHILD-OF-1 10|New|-|-" "$BUILD/client.log"
check "CHILD-OF-1 11|Open|-|-" "$BUILD/client.log"
check "CHILD-OF-1 12||separator|-" "$BUILD/client.log"
check "CHILD-OF-1 13|Quit|-|-" "$BUILD/client.log"
check "GET_LAYOUT parent=0" "$BUILD/fake.log"
check "ABOUT_TO_SHOW 0" "$BUILD/fake.log"
check "EVENT 10 clicked" "$BUILD/fake.log"
check "CLEARED available=no top=0" "$BUILD/client.log"

if [ "$CLIENT_EXIT" -ne 0 ]; then
    echo "FAIL client exit code $CLIENT_EXIT"
    FAIL=1
fi

echo
if [ "$FAIL" -eq 0 ]; then
    echo "PASS"
else
    echo "FAILED"
fi
exit "$FAIL"
