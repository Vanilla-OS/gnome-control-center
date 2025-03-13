/*
 * Copyright © 2016-2024 Red Hat, Inc
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

#include "gxdp-wayland.h"

#include <gdk/wayland/gdkwayland.h>

#include "mutter-x11-interop-client-protocol.h"
#include "gxdp-dbus.h"

static struct mutter_x11_interop *x11_interop = NULL;

static void
handle_registry_global (void               *user_data,
                        struct wl_registry *registry,
                        uint32_t            id,
                        const char         *interface,
                        uint32_t            version)
{
  struct mutter_x11_interop **x11_interop_ptr = user_data;

  if (strcmp (interface, mutter_x11_interop_interface.name) == 0)
    {
      *x11_interop_ptr = wl_registry_bind (registry, id,
                                           &mutter_x11_interop_interface, 1);
    }
}

static void
handle_registry_global_remove (void               *user_data,
                               struct wl_registry *registry,
                               uint32_t            name)
{
}

static const struct wl_registry_listener registry_listener = {
  handle_registry_global,
  handle_registry_global_remove
};

static void
init_x11_interop (void)
{
  GdkDisplay *display = gdk_display_get_default ();
  struct wl_display *wl_display;
  struct wl_registry *wl_registry;

  g_assert (GDK_IS_WAYLAND_DISPLAY (display));

  wl_display = gdk_wayland_display_get_wl_display (display);
  wl_registry = wl_display_get_registry (wl_display);
  wl_registry_add_listener (wl_registry, &registry_listener, &x11_interop);
  wl_display_roundtrip (wl_display);

  if (!x11_interop)
    {
      g_warning ("Missing X11 interop protocol support, "
                 "portal dialogs may missbehave");
    }
}

static gboolean
init_gtk_wayland_fallback (GError **error)
{
  if (!gtk_init_check ())
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                   "Failed to initialize GTK");
      return FALSE;
    }
  return TRUE;
}

gboolean
gxdp_wayland_init (GxdpServiceClientType   service_client_type,
                   GError                **error)
{
  g_autoptr(GError) local_error = NULL;
  g_autoptr(GxdpDBusMutterServiceChannel) proxy = NULL;
  g_autoptr(GVariant) fd_variant = NULL;
  g_autoptr(GUnixFDList) fd_list = NULL;
  int fd;
  g_autofree char *fd_str = NULL;

  proxy = gxdp_dbus_mutter_service_channel_proxy_new_for_bus_sync (
    G_BUS_TYPE_SESSION,
    (G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START |
     G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
     G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS),
    "org.gnome.Mutter.ServiceChannel",
    "/org/gnome/Mutter/ServiceChannel",
    NULL, &local_error);
  if (!proxy)
    {
      g_warning ("Compositor service channel missing, "
                 "portals dialogs may missbehave (%s)",
                 local_error->message);
      return init_gtk_wayland_fallback (error);
    }

  if (!gxdp_dbus_mutter_service_channel_call_open_wayland_service_connection_sync (
        proxy,
        service_client_type,
        NULL,
        &fd_variant,
        &fd_list,
        NULL, &local_error))
    {
      g_warning ("Failed to open service channel Wayland connection, "
                 "portals dialogs may missbehave (%s).",
                 local_error->message);

      return init_gtk_wayland_fallback (error);
    }

  fd = g_unix_fd_list_get (fd_list,
                           g_variant_get_handle (fd_variant),
                           &local_error);
  if (fd < 0)
    {
      g_warning ("Failed to acquire Wayland connection file descriptor, "
                 "portals dialogs may missbehave (%s).",
                 local_error->message);

      return init_gtk_wayland_fallback (error);
    }

  fd_str = g_strdup_printf ("%d", fd);

  g_setenv ("WAYLAND_SOCKET", fd_str, TRUE);
  gdk_set_allowed_backends ("wayland");
  if (!gtk_init_check ())
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                   "Failed to initialize GTK");
      return FALSE;
    }

  init_x11_interop ();

  return TRUE;
}

void
gxdp_wayland_set_x11_parent (GdkSurface *dialog_surface,
                             int         parent_xid)
{
  struct wl_surface *wl_surface;

  g_return_if_fail (dialog_surface);

  wl_surface = gdk_wayland_surface_get_wl_surface (dialog_surface);
  mutter_x11_interop_set_x11_parent (x11_interop, wl_surface, parent_xid);
}
