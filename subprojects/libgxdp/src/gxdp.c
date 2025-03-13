/*
 * Copyright © 2024 Red Hat, Inc
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

#include "gxdp.h"

#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#ifdef GXDP_HAVE_GTK_WAYLAND
#include "gxdp-wayland.h"
#endif

gboolean
gxdp_init_gtk (GxdpServiceClientType   service_client_type,
               GError                **error)
{
  const char *forced_gdk_backend;
  const char *session_type;

  if (gtk_is_initialized ())
    {
      g_warning ("GTK was initialized too early, "
                 "portal dialogs may misbehave.");
      return TRUE;
    }

  gtk_disable_portals ();

  if (G_UNLIKELY (!g_setenv ("ADW_DISABLE_PORTAL", "1", TRUE)))
    {
      g_set_error (error, G_IO_ERROR, g_io_error_from_errno (errno),
                   "Failed to set ADW_DISABLE_PORTAL: %s", g_strerror (errno));
      return FALSE;
    }

  forced_gdk_backend = g_getenv ("GDK_BACKEND");

  session_type = getenv ("XDG_SESSION_TYPE");
#ifdef GXDP_HAVE_GTK_WAYLAND
  if (g_strcmp0 (session_type, "wayland") == 0)
    {
      if (forced_gdk_backend && g_strcmp0 (forced_gdk_backend, "wayland") != 0)
        {
          g_warning ("GDK backend forced via env var, "
                     "portal dialogs will not work properly.");
        }
      else
        {
          return gxdp_wayland_init (service_client_type, error);
        }
    }
#endif
#ifdef GXDP_HAVE_GTK_X11
  if (g_strcmp0 (session_type, "x11") == 0)
    {
      if (!gtk_init_check ())
        {
          g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                       "Failed to initialize GTK");
          return FALSE;
        }

      return TRUE;
    }
#endif

  g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
               "Unsupported or missing session type '%s'",
               session_type ? session_type : "");
  return FALSE;
}
