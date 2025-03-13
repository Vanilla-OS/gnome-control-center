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

#include "gxdp-external-window.h"

#include <string.h>

#ifdef GXDP_HAVE_GTK_X11
#include <gdk/x11/gdkx.h>
#include "gxdp-external-window-x11.h"
#endif
#ifdef GXDP_HAVE_GTK_WAYLAND
#include <gdk/wayland/gdkwayland.h>
#include "gxdp-external-window-wayland.h"
#endif

G_DEFINE_TYPE (GxdpExternalWindow, gxdp_external_window, G_TYPE_OBJECT)

GxdpExternalWindow *
gxdp_external_window_new_from_handle (const char *handle_str)
{
  if (!handle_str)
    return NULL;

  if (strlen (handle_str) == 0)
    return NULL;

#ifdef GXDP_HAVE_GTK_X11
  if (GDK_IS_X11_DISPLAY (gdk_display_get_default ()))
    {
      GxdpExternalWindowX11 *external_window_x11;

      external_window_x11 = gxdp_external_window_x11_new (handle_str);
      return GXDP_EXTERNAL_WINDOW (external_window_x11);
    }
#endif
#ifdef GXDP_HAVE_GTK_WAYLAND
  if (GDK_IS_WAYLAND_DISPLAY (gdk_display_get_default ()))
    {
      GxdpExternalWindowWayland *external_window_wayland;

      external_window_wayland = gxdp_external_window_wayland_new (handle_str);
      return GXDP_EXTERNAL_WINDOW (external_window_wayland);
    }
#endif

  g_warning ("Unhandled parent window type %s", handle_str);
  return NULL;
}

void
gxdp_external_window_set_parent_of (GxdpExternalWindow *external_window,
                                    GdkSurface     *surface)
{
  GXDP_EXTERNAL_WINDOW_GET_CLASS (external_window)->set_parent_of (external_window,
                                                                   surface);
}

static void
gxdp_external_window_init (GxdpExternalWindow *external_window)
{
}

static void
gxdp_external_window_class_init (GxdpExternalWindowClass *klass)
{
}
