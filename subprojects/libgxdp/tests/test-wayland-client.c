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
 */

#include <gdk/gdk.h>
#include <gdk/wayland/gdkwayland.h>
#include <gtk/gtk.h>

static void
on_toplevel_exported (GdkToplevel *toplevel,
                      const char  *handle,
                      gpointer     user_data)
{
  char **out_handle = user_data;

  *out_handle = g_strdup (handle);
}

int
main (int    argc,
      char **argv)
{
  g_autoptr (GtkWindow) window = NULL;
  GdkSurface *surface;
  g_autofree char *handle = NULL;

  gdk_set_allowed_backends ("wayland");
  gtk_disable_portals ();
  gtk_init ();

  window = GTK_WINDOW (gtk_window_new ());
  gtk_window_set_default_size (window, 100, 100);
  gtk_window_present (window);

  surface = gtk_native_get_surface (GTK_NATIVE (window));
  gdk_wayland_toplevel_export_handle (GDK_WAYLAND_TOPLEVEL (surface),
                                      on_toplevel_exported, &handle, NULL);
  while (!handle)
    g_main_context_iteration (NULL, TRUE);

  g_print ("%s\n", handle);

  g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
