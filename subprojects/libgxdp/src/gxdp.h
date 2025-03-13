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

#pragma once

#include <gxdp-external-window.h>

typedef enum _GxdpServiceClientType
{
  GXDP_SERVICE_CLIENT_TYPE_NONE,
  GXDP_SERVICE_CLIENT_TYPE_PORTAL_BACKEND,
  GXDP_SERVICE_CLIENT_TYPE_FILE_CHOOSER,
  GXDP_SERVICE_CLIENT_TYPE_GLOBAL_SHORTCUTS,
} GxdpServiceClientType;

gboolean gxdp_init_gtk (GxdpServiceClientType   service_client_type,
                        GError                **error);
