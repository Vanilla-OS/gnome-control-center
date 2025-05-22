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

#include <gdk/wayland/gdkwayland.h>
#include <gxdp.h>

int
main (int    argc,
      char **argv)
{
  GError *error = NULL;
  g_autofree char *wayland_handle = NULL;
  g_autofree char *x11_handle = NULL;
  g_autoptr (GxdpExternalWindow) external_wayland_window = NULL;
  g_autoptr (GxdpExternalWindow) external_x11_window = NULL;
  g_autoptr (GtkWindow) window1 = NULL;
  g_autoptr (GtkWindow) window2 = NULL;
  GdkSurface *surface1;
  GdkSurface *surface2;
  struct wl_display *wl_display;

  g_test_init (&argc, &argv, NULL);

  g_assert_cmpint (argc, ==, 3);

  wayland_handle = g_strdup_printf ("wayland:%s", argv[1]);
  x11_handle = g_strdup_printf ("x11:%s", argv[2]);

  gxdp_init_gtk (GXDP_SERVICE_CLIENT_TYPE_PORTAL_BACKEND, &error);
  g_assert_no_error (error);

  external_wayland_window = gxdp_external_window_new_from_handle (wayland_handle);
  external_x11_window = gxdp_external_window_new_from_handle (x11_handle);

  window1 = GTK_WINDOW (gtk_window_new ());
  gtk_window_set_default_size (window1, 50, 50);
  gtk_window_present (window1);
  surface1 = gtk_native_get_surface (GTK_NATIVE (window1));
  gxdp_external_window_set_parent_of (external_wayland_window, surface1);

  window2 = GTK_WINDOW (gtk_window_new ());
  gtk_window_set_default_size (window2, 50, 50);
  gtk_window_present (window2);
  surface2 = gtk_native_get_surface (GTK_NATIVE (window2));
  gxdp_external_window_set_parent_of (external_x11_window, surface2);

  wl_display =
    gdk_wayland_display_get_wl_display (gdk_display_get_default ());
  wl_display_roundtrip (wl_display);

  while (TRUE)
    {
      if (!g_main_context_iteration (NULL, FALSE))
        break;
    }

  return EXIT_SUCCESS;
}
