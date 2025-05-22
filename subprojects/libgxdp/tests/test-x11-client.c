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
#include <gdk/x11/gdkx.h>
#include <gtk/gtk.h>

int
main (int    argc,
      char **argv)
{
  g_autoptr (GtkWindow) window = NULL;
  GdkSurface *surface;
  XID xid;

  gdk_set_allowed_backends ("x11");
  gtk_disable_portals ();
  gtk_init ();

  window = GTK_WINDOW (gtk_window_new ());
  gtk_window_set_default_size (window, 100, 100);
  gtk_window_present (window);

  surface = gtk_native_get_surface (GTK_NATIVE (window));
  xid = gdk_x11_surface_get_xid (surface);

  g_print ("%lu\n", xid);

  g_main_context_iteration (NULL, TRUE);

  return EXIT_SUCCESS;
}
