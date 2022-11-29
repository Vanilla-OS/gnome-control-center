/* cc-windows-panel.c
 *
 * Copyright 2022 Muqtadir <muqtxdir@gmail.com>
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


#include "cc-windows-panel.h"
#include "cc-list-row.h"
#include "cc-windows-resources.h"

#define WM_BUTTON_LAYOUT "button-layout"
#define DEFAULT_BUTTON_LAYOUT "close"
#define EXTENDED_BUTTON_LAYOUT "minimize,maximize,close"

struct _CcWindowsPanel
{
  CcPanel          parent_instance;

  GSettings        *mutter_settings;
  GSettings        *wm_settings;

  GtkCheckButton   *default_radio;
  GtkCheckButton   *extended_radio;
  CcListRow        *attach_modal_dialogs_switch;
  CcListRow        *center_new_windows_switch;
  CcListRow        *resize_with_right_button_switch;

  GtkComboBoxText  *wm_focus_combo;

  gchar           **titlebar_button_layout;
};

CC_PANEL_REGISTER (CcWindowsPanel, cc_windows_panel)

static void
cc_windows_panel_finalize (GObject *object)
{
  CcWindowsPanel *self = (CcWindowsPanel *)object;

  g_clear_object (&self->mutter_settings);
  g_clear_object (&self->wm_settings);
  g_strfreev (self->titlebar_button_layout);

  G_OBJECT_CLASS (cc_windows_panel_parent_class)->finalize (object);
}

static void
reload_titlebar_layout (CcWindowsPanel *self)
{
  gchar *titlebar_buttons=NULL;

  self->titlebar_button_layout = g_strsplit (g_settings_get_string (self->wm_settings, WM_BUTTON_LAYOUT), ":", 2);
  titlebar_buttons = self->titlebar_button_layout[1];

  if ((g_ascii_strcasecmp (titlebar_buttons, DEFAULT_BUTTON_LAYOUT) == 0))
      gtk_check_button_set_active (self->default_radio, TRUE);
  else if ((g_ascii_strcasecmp (titlebar_buttons, EXTENDED_BUTTON_LAYOUT) == 0))
      gtk_check_button_set_active (self->extended_radio, TRUE);
  else
    {
      gtk_check_button_set_active (self->default_radio, FALSE);
      gtk_check_button_set_active (self->extended_radio, FALSE);
    }
}

/*code taken from fork of @Reidond1*/
static GVariant *
set_titlebar_buttons (const GValue        *value,
                      const GVariantType  *expected_type,
                      gpointer             user_data)
{
  gboolean buttons;
  CcWindowsPanel *self = user_data;
  gchar *original_buttons_slice;
  gchar *wm_layout;

  buttons = g_value_get_boolean (value);
  original_buttons_slice = self->titlebar_button_layout[0];

  if (buttons) {
    wm_layout = g_strjoin (":", original_buttons_slice, DEFAULT_BUTTON_LAYOUT, NULL);
    g_settings_set_string (self->wm_settings, WM_BUTTON_LAYOUT, wm_layout);
  } else {
    wm_layout = g_strjoin (":", original_buttons_slice, EXTENDED_BUTTON_LAYOUT, NULL);
    g_settings_set_string (self->wm_settings, WM_BUTTON_LAYOUT, wm_layout);
  }

  return g_variant_new_string (wm_layout);
}

static void
cc_windows_panel_class_init (CcWindowsPanelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = cc_windows_panel_finalize;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/control-center/windows/cc-windows-panel.ui");

  gtk_widget_class_bind_template_child (widget_class, CcWindowsPanel, default_radio);
  gtk_widget_class_bind_template_child (widget_class, CcWindowsPanel, extended_radio);
  gtk_widget_class_bind_template_child (widget_class, CcWindowsPanel, wm_focus_combo);
  gtk_widget_class_bind_template_child (widget_class, CcWindowsPanel, attach_modal_dialogs_switch);
  gtk_widget_class_bind_template_child (widget_class, CcWindowsPanel, center_new_windows_switch);
  gtk_widget_class_bind_template_child (widget_class, CcWindowsPanel, resize_with_right_button_switch);

}

static void
cc_windows_panel_init (CcWindowsPanel *self)
{
  g_resources_register (cc_windows_get_resource ());

  gtk_widget_init_template (GTK_WIDGET (self));

  self->mutter_settings = g_settings_new ("org.gnome.mutter");

  g_settings_bind (self->mutter_settings, "attach-modal-dialogs",
                   self->attach_modal_dialogs_switch, "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (self->mutter_settings, "center-new-windows",
                   self->center_new_windows_switch, "active",
                   G_SETTINGS_BIND_DEFAULT);

  self->wm_settings = g_settings_new ("org.gnome.desktop.wm.preferences");

  reload_titlebar_layout (self);

  g_settings_bind_with_mapping (self->wm_settings,
                                WM_BUTTON_LAYOUT,
                                self->default_radio,
                                "active",
                                G_SETTINGS_BIND_SET,
                                NULL,
                                set_titlebar_buttons,
                                self,
                                NULL);

  g_settings_bind (self->wm_settings, "resize-with-right-button",
                   self->resize_with_right_button_switch, "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (self->wm_settings, "focus-mode",
                        self->wm_focus_combo, "active-id",
                        G_SETTINGS_BIND_DEFAULT);
}