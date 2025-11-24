/*
 * Copyright © 2016 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *       Jonas Ådahl <jadahl@redhat.com>
 */

#include "gxdp-config.h"

#include "gxdp-external-window-x11.h"

#include <errno.h>
#include <gdk/x11/gdkx.h>
#include <X11/Xatom.h>
#include <gdk/gdk.h>
#include <stdlib.h>

struct _GxdpExternalWindowX11
{
  GxdpExternalWindow parent;

  Window foreign_xid;
};

struct _GxdpExternalWindowX11Class
{
  GxdpExternalWindowClass parent_class;
};

G_DEFINE_TYPE (GxdpExternalWindowX11, gxdp_external_window_x11,
               GXDP_TYPE_EXTERNAL_WINDOW)

GxdpExternalWindowX11 *
gxdp_external_window_x11_new (const char *handle_str)
{
  GxdpExternalWindowX11 *external_window_x11;
  const char x11_prefix[] = "x11:";
  const char *x11_handle_str;
  int xid;

  if (!g_str_has_prefix (handle_str, x11_prefix))
    {
      g_warning ("Invalid external window handle string '%s'", handle_str);
      return NULL;
    }

  x11_handle_str = handle_str + strlen (x11_prefix);

  errno = 0;
  xid = strtol (x11_handle_str, NULL, 16);
  if (errno != 0)
    {
      g_warning ("Failed to reference external X11 window, invalid XID %s",
                 x11_handle_str);
      return NULL;
    }

  external_window_x11 = g_object_new (GXDP_TYPE_EXTERNAL_WINDOW_X11,
                                      NULL);
  external_window_x11->foreign_xid = xid;

  return external_window_x11;
}

static void
gxdp_external_window_x11_set_parent_of (GxdpExternalWindow *external_window,
                                        GdkSurface         *surface)
{
  GxdpExternalWindowX11 *external_window_x11 =
    GXDP_EXTERNAL_WINDOW_X11 (external_window);
  GdkDisplay *display;
  Display *xdisplay;
  Atom net_wm_window_type_atom;
  Atom net_wm_window_type_dialog_atom;

  G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  display = gdk_display_get_default ();
  xdisplay = gdk_x11_display_get_xdisplay (display);

  XSetTransientForHint (xdisplay,
                        GDK_SURFACE_XID (surface),
                        external_window_x11->foreign_xid);

  net_wm_window_type_atom =
    gdk_x11_get_xatom_by_name_for_display (display, "_NET_WM_WINDOW_TYPE"),
  net_wm_window_type_dialog_atom =
    gdk_x11_get_xatom_by_name_for_display (display, "_NET_WM_WINDOW_TYPE_DIALOG");
  XChangeProperty (xdisplay,
                   GDK_SURFACE_XID (surface),
                   net_wm_window_type_atom,
                   XA_ATOM, 32, PropModeReplace,
                   (guchar *) &net_wm_window_type_dialog_atom, 1);
  G_GNUC_END_IGNORE_DEPRECATIONS
}

static void
gxdp_external_window_x11_init (GxdpExternalWindowX11 *external_window_x11)
{
}

static void
gxdp_external_window_x11_class_init (GxdpExternalWindowX11Class *klass)
{
  GxdpExternalWindowClass *external_window_class = GXDP_EXTERNAL_WINDOW_CLASS (klass);

  external_window_class->set_parent_of = gxdp_external_window_x11_set_parent_of;
}
