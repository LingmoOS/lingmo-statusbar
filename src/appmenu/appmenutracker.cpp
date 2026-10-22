// SPDX-FileCopyrightText: 2026 Lingmo OS Team
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appmenutracker.h"

#include "dbusmenuclient.h"

#include <QDebug>
#include <QSocketNotifier>

#include <cstdlib>

namespace {

constexpr auto ActiveWindowAtomName = "_NET_ACTIVE_WINDOW";
constexpr auto AppMenuServiceNameAtomName = "_KDE_NET_WM_APPMENU_SERVICE_NAME";
constexpr auto AppMenuObjectPathAtomName = "_KDE_NET_WM_APPMENU_OBJECT_PATH";

xcb_atom_t internAtom(xcb_connection_t *connection, const char *name)
{
    const xcb_intern_atom_cookie_t cookie
        = xcb_intern_atom(connection, 0, static_cast<uint16_t>(qstrlen(name)), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, nullptr);
    if (!reply) {
        return XCB_ATOM_NONE;
    }

    const xcb_atom_t atom = reply->atom;
    free(reply);
    return atom;
}

} // namespace

AppMenuTracker::AppMenuTracker(DbusMenuClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    m_connection = xcb_connect(nullptr, nullptr);
    if (xcb_connection_has_error(m_connection)) {
        qInfo() << "[appmenu] no X11 connection, the global menu stays empty";
        xcb_disconnect(m_connection);
        m_connection = nullptr;
        return;
    }

    const xcb_setup_t *setup = xcb_get_setup(m_connection);
    xcb_screen_iterator_t screens = xcb_setup_roots_iterator(setup);
    m_root = screens.data->root;

    m_activeWindowAtom = internAtom(m_connection, ActiveWindowAtomName);
    m_serviceNameAtom = internAtom(m_connection, AppMenuServiceNameAtomName);
    m_objectPathAtom = internAtom(m_connection, AppMenuObjectPathAtomName);
    if (m_activeWindowAtom == XCB_ATOM_NONE || m_serviceNameAtom == XCB_ATOM_NONE
        || m_objectPathAtom == XCB_ATOM_NONE) {
        qWarning() << "[appmenu] could not intern the atoms, the global menu stays empty";
        return;
    }

    // Watch _NET_ACTIVE_WINDOW on the root window; the active window itself is
    // watched in trackActiveWindow() so a menu exported after activation (the Qt
    // platform theme sets the properties when the menubar is created) is picked up.
    const uint32_t rootMask = XCB_EVENT_MASK_PROPERTY_CHANGE;
    xcb_change_window_attributes(m_connection, m_root, XCB_CW_EVENT_MASK, &rootMask);
    xcb_flush(m_connection);

    m_notifier = new QSocketNotifier(xcb_get_file_descriptor(m_connection), QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &AppMenuTracker::processX11Events);

    trackActiveWindow();
}

AppMenuTracker::~AppMenuTracker()
{
    if (m_connection) {
        xcb_disconnect(m_connection);
        m_connection = nullptr;
    }
}

bool AppMenuTracker::isSupported() const
{
    return m_connection != nullptr;
}

void AppMenuTracker::processX11Events()
{
    if (!m_connection) {
        return;
    }

    while (xcb_generic_event_t *event = xcb_poll_for_event(m_connection)) {
        const uint8_t type = event->response_type & ~0x80;
        if (type == XCB_PROPERTY_NOTIFY) {
            const auto *notification = reinterpret_cast<xcb_property_notify_event_t *>(event);
            if (notification->window == m_root && notification->atom == m_activeWindowAtom) {
                trackActiveWindow();
            } else if (notification->window == m_activeWindow
                       && (notification->atom == m_serviceNameAtom
                           || notification->atom == m_objectPathAtom)) {
                refreshMenuProperties();
            }
        }
        free(event);
    }
}

void AppMenuTracker::trackActiveWindow()
{
    if (!m_connection) {
        return;
    }

    xcb_window_t activeWindow = XCB_WINDOW_NONE;
    const xcb_get_property_cookie_t cookie
        = xcb_get_property(m_connection, 0, m_root, m_activeWindowAtom, XCB_ATOM_WINDOW, 0, 1);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(m_connection, cookie, nullptr);
    if (reply) {
        if (reply->format == 32 && xcb_get_property_value_length(reply) >= 4) {
            activeWindow = *static_cast<xcb_window_t *>(xcb_get_property_value(reply));
        }
        free(reply);
    }

    if (activeWindow == m_activeWindow) {
        return;
    }

    m_activeWindow = activeWindow;

    if (m_activeWindow == XCB_WINDOW_NONE) {
        m_client->clear();
        return;
    }

    const uint32_t windowMask = XCB_EVENT_MASK_PROPERTY_CHANGE;
    xcb_change_window_attributes(m_connection, m_activeWindow, XCB_CW_EVENT_MASK, &windowMask);
    xcb_flush(m_connection);

    refreshMenuProperties();
}

void AppMenuTracker::refreshMenuProperties()
{
    if (!m_connection || m_activeWindow == XCB_WINDOW_NONE) {
        m_client->clear();
        return;
    }

    const QByteArray serviceName = readStringProperty(m_activeWindow, m_serviceNameAtom);
    const QByteArray objectPath = readStringProperty(m_activeWindow, m_objectPathAtom);
    if (serviceName.isEmpty() || objectPath.isEmpty()) {
        // The window has no appmenu (most applications do not export one).
        m_client->clear();
        return;
    }

    m_client->setMenu(QString::fromUtf8(serviceName), QString::fromUtf8(objectPath));
}

QByteArray AppMenuTracker::readStringProperty(xcb_window_t window, xcb_atom_t atom) const
{
    if (!m_connection || window == XCB_WINDOW_NONE || atom == XCB_ATOM_NONE) {
        return {};
    }

    const xcb_get_property_cookie_t cookie
        = xcb_get_property(m_connection, 0, window, atom, XCB_GET_PROPERTY_TYPE_ANY, 0, 1024);
    xcb_get_property_reply_t *reply = xcb_get_property_reply(m_connection, cookie, nullptr);
    if (!reply) {
        return {};
    }

    QByteArray value;
    if (reply->format == 8 && reply->type != XCB_ATOM_NONE) {
        const int length = xcb_get_property_value_length(reply);
        if (length > 0) {
            value = QByteArray(static_cast<const char *>(xcb_get_property_value(reply)), length);
            // X11 strings are not always NUL terminated.
            const int terminator = value.indexOf('\0');
            if (terminator >= 0) {
                value.truncate(terminator);
            }
            value = value.trimmed();
        }
    }

    free(reply);
    return value;
}
