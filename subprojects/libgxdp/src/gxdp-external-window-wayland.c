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

#include "gxdp-external-window-wayland.h"

#include <gdk/gdk.h>
#include <gdk/wayland/gdkwayland.h>
#include <wayland-client.h>

#include "gxdp-wayland.h"

#define WAYLAND_HANDLE_PREFIX "wayland:"
#define X11_HANDLE_PREFIX "x11:"

typedef enum _ParentWindowType
{
  PARENT_WINDOW_TYPE_WAYLAND,
  PARENT_WINDOW_TYPE_X11,
} ParentWindowType;

struct _GxdpExternalWindowWayland
{
  GxdpExternalWindow parent;

  ParentWindowType parent_type;
  struct {
    char *xdg_foreign_handle;
  } wayland;
  struct {
    int xid;
    gulong mapped_handler_id;
  } x11;

  GdkSurface *dialog_surface;
};

struct _GxdpExternalWindowWaylandClass
{
  GxdpExternalWindowClass parent_class;
};

G_DEFINE_TYPE (GxdpExternalWindowWayland, gxdp_external_window_wayland,
               GXDP_TYPE_EXTERNAL_WINDOW)

GxdpExternalWindowWayland *
gxdp_external_window_wayland_new (const char *handle_str)
{
  g_autoptr(GxdpExternalWindowWayland) external_window_wayland = NULL;

  external_window_wayland = g_object_new (GXDP_TYPE_EXTERNAL_WINDOW_WAYLAND,
                                          NULL);

  if (g_str_has_prefix (handle_str, WAYLAND_HANDLE_PREFIX))
    {
      external_window_wayland->parent_type = PARENT_WINDOW_TYPE_WAYLAND;
      external_window_wayland->wayland.xdg_foreign_handle =
        g_strdup (handle_str + strlen (WAYLAND_HANDLE_PREFIX));
    }
  else if (g_str_has_prefix (handle_str, X11_HANDLE_PREFIX))
    {
      const char *x11_handle_str = handle_str + strlen (X11_HANDLE_PREFIX);
      int xid;

      errno = 0;
      xid = strtol (x11_handle_str, NULL, 16);
      if (errno != 0)
        {
          g_warning ("Failed to reference external X11 window, invalid XID %s",
                     x11_handle_str);
          return NULL;
        }

      external_window_wayland->parent_type = PARENT_WINDOW_TYPE_X11;
      external_window_wayland->x11.xid = xid;
    }
  else
    {
      g_warning ("Invalid external window handle string '%s'", handle_str);
      return NULL;
    }

  return g_steal_pointer (&external_window_wayland);
}

static void
set_x11_parent (GxdpExternalWindow *external_window)
{
  GxdpExternalWindowWayland *external_window_wayland =
    GXDP_EXTERNAL_WINDOW_WAYLAND (external_window);

  gxdp_wayland_set_x11_parent (external_window_wayland->dialog_surface,
                               external_window_wayland->x11.xid);
}

static void
on_mapped (GdkSurface         *surface,
           GParamSpec         *pspec,
           GxdpExternalWindow *external_window)
{
  if (!gdk_surface_get_mapped (surface))
    return;

  set_x11_parent (external_window);
}

static void
gxdp_external_window_wayland_set_parent_of (GxdpExternalWindow *external_window,
                                            GdkSurface         *surface)
{
  GxdpExternalWindowWayland *external_window_wayland =
    GXDP_EXTERNAL_WINDOW_WAYLAND (external_window);

  g_clear_signal_handler (&external_window_wayland->x11.mapped_handler_id,
                          external_window_wayland->dialog_surface);
  g_clear_object (&external_window_wayland->dialog_surface);

  switch (external_window_wayland->parent_type)
    {
    case PARENT_WINDOW_TYPE_WAYLAND:
      {
        GdkToplevel *toplevel = GDK_TOPLEVEL (surface);
        const char *handle = external_window_wayland->wayland.xdg_foreign_handle;

        if (!gdk_wayland_toplevel_set_transient_for_exported (toplevel, handle))
          g_warning ("Failed to set portal window transient for external parent");
        break;
      }
    case PARENT_WINDOW_TYPE_X11:
      if (gdk_surface_get_mapped (surface))
        {
          set_x11_parent (external_window);
        }
      else
        {
          external_window_wayland->x11.mapped_handler_id =
            g_signal_connect (surface, "notify::mapped",
                              G_CALLBACK (on_mapped), external_window);
        }
      break;
    }

  g_set_object (&external_window_wayland->dialog_surface, surface);
}

static void
gxdp_external_window_wayland_dispose (GObject *object)
{
  GxdpExternalWindowWayland *external_window_wayland =
    GXDP_EXTERNAL_WINDOW_WAYLAND (object);

  g_clear_pointer (&external_window_wayland->wayland.xdg_foreign_handle,
                   g_free);
  g_clear_signal_handler (&external_window_wayland->x11.mapped_handler_id,
                          external_window_wayland->dialog_surface);
  g_clear_object (&external_window_wayland->dialog_surface);

  G_OBJECT_CLASS (gxdp_external_window_wayland_parent_class)->dispose (object);
}

static void
gxdp_external_window_wayland_init (GxdpExternalWindowWayland *external_window_wayland)
{
}

static void
gxdp_external_window_wayland_class_init (GxdpExternalWindowWaylandClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GxdpExternalWindowClass *external_window_class =
    GXDP_EXTERNAL_WINDOW_CLASS (klass);

  object_class->dispose = gxdp_external_window_wayland_dispose;

  external_window_class->set_parent_of = gxdp_external_window_wayland_set_parent_of;
}
