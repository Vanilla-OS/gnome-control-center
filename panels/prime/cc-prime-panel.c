/* cc-prime-panel.c
 *
 * Copyright 2022 Muqtadir <muqtxdir@gmail.com>
 * Copyright 2023 Mirko Brombin <mirko@fabricators.ltd>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "cc-prime-panel.h"
#include "cc-list-row.h"
#include "cc-prime-resources.h"

struct _CcPrimePanel
{
  CcPanel          parent_instance;
};

CC_PANEL_REGISTER (CcPrimePanel, cc_prime_panel)

static void
cc_prime_panel_finalize (GObject *object)
{
  CcPrimePanel *self = (CcPrimePanel *)object;

  G_OBJECT_CLASS (cc_prime_panel_parent_class)->finalize (object);
}

static void
cc_prime_panel_class_init (CcPrimePanelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = cc_prime_panel_finalize;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/control-center/prime/cc-prime-panel.ui");

  //gtk_widget_class_bind_template_child (widget_class, CcWindowsPanel, default_radio);
}

static void
cc_prime_panel_init (CcPrimePanel *self)
{
  g_resources_register (cc_prime_get_resource ());

  gtk_widget_init_template (GTK_WIDGET (self));
}
