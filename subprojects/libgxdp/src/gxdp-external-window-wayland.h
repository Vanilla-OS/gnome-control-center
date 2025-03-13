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

#pragma once

#include <glib-object.h>

#include "gxdp-external-window.h"

#define GXDP_TYPE_EXTERNAL_WINDOW_WAYLAND (gxdp_external_window_wayland_get_type ())
G_DECLARE_FINAL_TYPE (GxdpExternalWindowWayland, gxdp_external_window_wayland,
                      GXDP, EXTERNAL_WINDOW_WAYLAND, GxdpExternalWindow)

GxdpExternalWindowWayland *gxdp_external_window_wayland_new (const char *handle_str);

GdkDisplay * init_external_window_wayland_display (GError **error);
