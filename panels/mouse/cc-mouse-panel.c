/*
 * Copyright (C) 2010 Intel, Inc
 * Copyright (C) 2012 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Thomas Wood <thomas.wood@intel.com>
 *          Rodrigo Moya <rodrigo@gnome.org>
 *          Ondrej Holy <oholy@redhat.com>
 *
 */

#include <gdesktop-enums.h>
#include <gtk/gtk.h>

#include "cc-mouse-caps-helper.h"
#include "cc-mouse-panel.h"
#include "cc-mouse-resources.h"
#include "cc-mouse-test.h"
#include "gsd-device-manager.h"
#include "gsd-input-helper.h"

#define TOUCHPAD_PATH_ID "org.gnome.desktop.peripherals.touchpad"
#define TOUCHPAD_CLICK_METHOD_KEY "click-method"

struct _CcMousePanel
{
  CcPanel            parent_instance;

  GtkBox            *edge_scrolling_box;
  GtkCheckButton    *edge_scrolling_radio;
  AdwPreferencesGroup *mouse_group;
  GtkCheckButton    *mouse_natural_scrolling_radio;
  GtkCheckButton    *mouse_traditional_scrolling_radio;
  GtkScale          *mouse_speed_scale;
  CcMouseTest       *mouse_test;
  GtkBox            *primary_button_box;
  GtkToggleButton   *primary_button_left;
  GtkToggleButton   *primary_button_right;
  AdwPreferencesPage*preferences;
  GtkStack          *stack;
  GtkListBoxRow     *tap_to_click_row;
  GtkSwitch         *tap_to_click_switch;
  GtkButton         *test_button;
  AdwPreferencesGroup *touchpad_group;
  GtkListBoxRow     *touchpad_natural_scrolling_row;
  GtkCheckButton    *touchpad_natural_scrolling_radio;
  GtkCheckButton    *touchpad_traditional_scrolling_radio;
  GtkListBoxRow     *touchpad_speed_row;
  GtkScale          *touchpad_speed_scale;
  GtkSwitch         *touchpad_toggle_switch;
  GtkBox           *two_finger_scrolling_box;
  GtkCheckButton    *two_finger_scrolling_radio;

  GSettings         *mouse_settings;
  GSettings         *touchpad_settings;
  GSettings         *interface_settings;

  gboolean           have_mouse;
  gboolean           have_touchpad;
  gboolean           have_touchscreen;

  gboolean           left_handed;
  GtkGesture        *left_gesture;
  GtkGesture        *right_gesture;

  // Revamp mouse panel
  GtkSwitch         *pointer_location_switch;
  GtkSwitch         *disable_while_typing_switch;
  GtkComboBoxText   *mouse_accel_combo;
  GtkComboBoxText   *touchpad_accel_combo;

  GtkCheckButton    *area_click_emulation_radio;
  GtkCheckButton    *default_click_emulation_radio;
};

CC_PANEL_REGISTER (CcMousePanel, cc_mouse_panel)

static void
reload_scrolling_method (CcMousePanel *self)
{
  gboolean edge_scroll_enabled;
  gboolean two_finger_scroll_enabled;

  edge_scroll_enabled = g_settings_get_boolean (self->touchpad_settings, "edge-scrolling-enabled");
  two_finger_scroll_enabled = g_settings_get_boolean (self->touchpad_settings, "two-finger-scrolling-enabled");

  
  if (edge_scroll_enabled && two_finger_scroll_enabled)
    {
      gtk_check_button_set_active (self->edge_scrolling_radio, FALSE);
    }
  else if (edge_scroll_enabled && !two_finger_scroll_enabled)
    {
      gtk_check_button_set_active (self->edge_scrolling_radio, TRUE);
    }
  else if (!edge_scroll_enabled && two_finger_scroll_enabled)
    {
      gtk_check_button_set_active (self->two_finger_scrolling_radio, TRUE);
    }
  else
    {
      gtk_check_button_set_active (self->edge_scrolling_radio, FALSE);
      gtk_check_button_set_active (self->two_finger_scrolling_radio, FALSE);
    }
}

static void
set_scrolling_direction (CcMousePanel   *self,
                         gboolean        edge_scrolling,
                         gboolean        two_finger_scrolling)
{
  gboolean edge_scroll_enabled;
  gboolean two_finger_scroll_enabled;

  edge_scroll_enabled = g_settings_get_boolean (self->touchpad_settings, "edge-scrolling-enabled");
  two_finger_scroll_enabled = g_settings_get_boolean (self->touchpad_settings, "two-finger-scrolling-enabled");

  if (edge_scroll_enabled && edge_scrolling)
    return;

  if (two_finger_scroll_enabled && two_finger_scrolling)
    return;
  
  g_settings_set_boolean (self->touchpad_settings, "edge-scrolling-enabled", edge_scrolling);
  g_settings_set_boolean (self->touchpad_settings, "two-finger-scrolling-enabled", two_finger_scrolling);
}

/* Scrolling Method */ 

static void
on_scrolling_direction_checked_cb (CcMousePanel *self)
{
  if (gtk_check_button_get_active (self->edge_scrolling_radio))
    set_scrolling_direction (self, TRUE, FALSE);
  else if (gtk_check_button_get_active (self->two_finger_scrolling_radio))
    set_scrolling_direction (self, FALSE, TRUE);
}

static void
reload_click_method_buttons (CcMousePanel *self)
{
  GDesktopTouchpadClickMethod clicky;

  clicky = g_settings_get_enum (self->touchpad_settings, TOUCHPAD_CLICK_METHOD_KEY);

  if (clicky == G_DESKTOP_TOUCHPAD_CLICK_METHOD_DEFAULT)
    {
      gtk_check_button_set_active (self->default_click_emulation_radio, TRUE);
    }
  else if (clicky == G_DESKTOP_TOUCHPAD_CLICK_METHOD_AREAS)
    {
      gtk_check_button_set_active (self->area_click_emulation_radio, TRUE);
    }
  else
    {
      gtk_check_button_set_active (self->default_click_emulation_radio, FALSE);
      gtk_check_button_set_active (self->area_click_emulation_radio, FALSE);
    }
}

static void
set_click_method (CcMousePanel   *self,
                  GDesktopTouchpadClickMethod  click_method)
{
  GDesktopTouchpadClickMethod clicky;

  clicky = g_settings_get_enum (self->touchpad_settings,
                                TOUCHPAD_CLICK_METHOD_KEY);

  if (click_method == clicky)
    return;

  g_settings_set_enum (self->touchpad_settings,
                       TOUCHPAD_CLICK_METHOD_KEY,
                       click_method);
}

/* Click Method */ 

static void
on_click_method_checked_cb (CcMousePanel *self)
{
  if (gtk_check_button_get_active (self->default_click_emulation_radio))
    set_click_method (self, G_DESKTOP_TOUCHPAD_CLICK_METHOD_DEFAULT);
    else if (gtk_check_button_get_active (self->area_click_emulation_radio))
    set_click_method (self, G_DESKTOP_TOUCHPAD_CLICK_METHOD_AREAS);
}

static void
setup_touchpad_options (CcMousePanel *self)
{
  gboolean have_two_finger_scrolling;
  gboolean have_edge_scrolling;
  gboolean have_tap_to_click;

  if (!self->have_touchpad) {
    gtk_widget_hide (GTK_WIDGET (self->touchpad_group));
    return;
  }

  cc_touchpad_check_capabilities (&have_two_finger_scrolling, &have_edge_scrolling, &have_tap_to_click);

  gtk_widget_show (GTK_WIDGET (self->touchpad_group));

  gtk_widget_set_visible (GTK_WIDGET (self->two_finger_scrolling_box), have_two_finger_scrolling);
  gtk_widget_set_visible (GTK_WIDGET (self->edge_scrolling_box), have_edge_scrolling);
  gtk_widget_set_visible (GTK_WIDGET (self->tap_to_click_row), have_tap_to_click);
}

static gboolean
get_touchpad_enabled (GSettings *settings)
{
  GDesktopDeviceSendEvents send_events;

  send_events = g_settings_get_enum (settings, "send-events");

  return send_events == G_DESKTOP_DEVICE_SEND_EVENTS_ENABLED;
}

static gboolean
show_touchpad_enabling_switch (CcMousePanel *self)
{
  if (!self->have_touchpad)
    return FALSE;

  g_debug ("Should we show the touchpad disable switch: have_mouse: %s have_touchscreen: %s\n",
     self->have_mouse ? "true" : "false",
     self->have_touchscreen ? "true" : "false");

  /* Let's show the button when a mouse or touchscreen is present */
  if (self->have_mouse || self->have_touchscreen)
    return TRUE;

  /* Let's also show when the touchpad is disabled. */
  if (!get_touchpad_enabled (self->touchpad_settings))
    return TRUE;

  return FALSE;
}

static gboolean
touchpad_enabled_get_mapping (GValue    *value,
                              GVariant  *variant,
                              gpointer   user_data)
{
  gboolean enabled;

  enabled = g_strcmp0 (g_variant_get_string (variant, NULL), "enabled") == 0;
  g_value_set_boolean (value, enabled);

  return TRUE;
}

static GVariant *
touchpad_enabled_set_mapping (const GValue              *value,
                              const GVariantType        *type,
                              gpointer                   user_data)
{
  gboolean enabled;

  enabled = g_value_get_boolean (value);

  return g_variant_new_string (enabled ? "enabled" : "disabled");
}

static void
pressed_cb (GtkButton *button)
{
  g_signal_emit_by_name (button, "activate");
}

static void
handle_secondary_button (CcMousePanel    *self,
                         GtkToggleButton *button,
                         GtkGesture      *gesture)
{
  gtk_gesture_single_set_touch_only (GTK_GESTURE_SINGLE (gesture), FALSE);
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (gesture), TRUE);
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), GDK_BUTTON_SECONDARY);
  g_signal_connect_swapped (gesture, "pressed", G_CALLBACK (pressed_cb), button);
  gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (gesture), GTK_PHASE_BUBBLE);
  gtk_widget_add_controller (GTK_WIDGET (button), GTK_EVENT_CONTROLLER (gesture));
}

/* Set up the property editors in the dialog. */
static void
setup_dialog (CcMousePanel *self)
{
  GtkToggleButton *button;

  gtk_widget_set_direction (GTK_WIDGET (self->primary_button_box), GTK_TEXT_DIR_LTR);

  self->left_handed = g_settings_get_boolean (self->mouse_settings, "left-handed");
  button = self->left_handed ? self->primary_button_right : self->primary_button_left;
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);

  g_settings_bind (self->mouse_settings, "left-handed",
                   self->primary_button_left, "active",
                   G_SETTINGS_BIND_DEFAULT | G_SETTINGS_BIND_INVERT_BOOLEAN);
  g_settings_bind (self->mouse_settings, "left-handed",
                   self->primary_button_right, "active",
                   G_SETTINGS_BIND_DEFAULT);

  /* Allow changing orientation with either button */
  button = self->primary_button_right;
  self->right_gesture = gtk_gesture_click_new ();
  handle_secondary_button (self, button, self->right_gesture);
  button = self->primary_button_left;
  self->left_gesture = gtk_gesture_click_new ();
  handle_secondary_button (self, button, self->left_gesture);

  /* Mouse section */
  gtk_widget_set_visible (GTK_WIDGET (self->mouse_group), self->have_mouse);

  if (g_settings_get_boolean (self->mouse_settings, "natural-scroll"))
    gtk_check_button_set_active (self->mouse_natural_scrolling_radio, TRUE);
  else
    gtk_check_button_set_active (self->mouse_traditional_scrolling_radio, TRUE);

  g_settings_bind (self->mouse_settings, "natural-scroll",
       self->mouse_natural_scrolling_radio, "active",
       G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (self->mouse_settings, "speed",
                   gtk_range_get_adjustment (GTK_RANGE (self->mouse_speed_scale)), "value",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (self->interface_settings, "locate-pointer",
                       self->pointer_location_switch, "active",
                       G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (self->mouse_settings, "accel-profile",
                        self->mouse_accel_combo, "active-id",
                        G_SETTINGS_BIND_DEFAULT);

  /* Touchpad section */
  gtk_widget_set_visible (GTK_WIDGET (self->touchpad_toggle_switch), show_touchpad_enabling_switch (self));

  g_settings_bind (self->touchpad_settings, "accel-profile",
                        self->touchpad_accel_combo, "active-id",
                        G_SETTINGS_BIND_DEFAULT);

  reload_click_method_buttons (self);

  g_signal_connect_object (self->touchpad_settings,
                           "changed::" TOUCHPAD_CLICK_METHOD_KEY,
                           G_CALLBACK (reload_click_method_buttons),
                           self,
                           G_CONNECT_SWAPPED);

  reload_scrolling_method (self);

  g_signal_connect_object (self->touchpad_settings,
                           "changed::" "two-finger-scrolling-enabled",
                           G_CALLBACK (reload_scrolling_method),
                           self,
                           G_CONNECT_SWAPPED);

  g_settings_bind_with_mapping (self->touchpad_settings, "send-events",
                                self->touchpad_toggle_switch, "active",
                                G_SETTINGS_BIND_DEFAULT,
                                touchpad_enabled_get_mapping,
                                touchpad_enabled_set_mapping,
                                NULL, NULL);
  g_settings_bind_with_mapping (self->touchpad_settings, "send-events",
                                self->touchpad_speed_row, "sensitive",
                                G_SETTINGS_BIND_GET,
                                touchpad_enabled_get_mapping,
                                touchpad_enabled_set_mapping,
                                NULL, NULL);
  g_settings_bind_with_mapping (self->touchpad_settings, "send-events",
                                self->tap_to_click_row, "sensitive",
                                G_SETTINGS_BIND_GET,
                                touchpad_enabled_get_mapping,
                                touchpad_enabled_set_mapping,
                                NULL, NULL);
  g_settings_bind_with_mapping (self->touchpad_settings, "send-events",
                                self->two_finger_scrolling_box, "sensitive",
                                G_SETTINGS_BIND_GET,
                                touchpad_enabled_get_mapping,
                                touchpad_enabled_set_mapping,
                                NULL, NULL);
  g_settings_bind_with_mapping (self->touchpad_settings, "send-events",
                                self->edge_scrolling_box, "sensitive",
                                G_SETTINGS_BIND_GET,
                                touchpad_enabled_get_mapping,
                                touchpad_enabled_set_mapping,
                                NULL, NULL);

  if (g_settings_get_boolean (self->touchpad_settings, "natural-scroll"))
    gtk_check_button_set_active (self->touchpad_natural_scrolling_radio, TRUE);
  else
    gtk_check_button_set_active (self->touchpad_traditional_scrolling_radio, TRUE);

  g_settings_bind (self->touchpad_settings, "natural-scroll",
       self->touchpad_natural_scrolling_radio, "active",
       G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (self->touchpad_settings, "speed",
                   gtk_range_get_adjustment (GTK_RANGE (self->touchpad_speed_scale)), "value",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (self->touchpad_settings, "tap-to-click",
                   self->tap_to_click_switch, "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (self->touchpad_settings, "disable-while-typing",
                   self->disable_while_typing_switch, "active",
                   G_SETTINGS_BIND_DEFAULT);

  setup_touchpad_options (self);
}

/* Callback issued when a button is clicked on the dialog */
static void
device_changed (CcMousePanel *self)
{
  self->have_touchpad = touchpad_is_present () || cc_synaptics_check ();
  /*                                              ^^^^^^^^^^^^^^^^^^^^^
   *            Workaround https://gitlab.gnome.org/GNOME/gtk/issues/97
   */

  setup_touchpad_options (self);

  self->have_mouse = mouse_is_present ();
  gtk_widget_set_visible (GTK_WIDGET (self->mouse_group), self->have_mouse);
  gtk_widget_set_visible (GTK_WIDGET (self->touchpad_toggle_switch), show_touchpad_enabling_switch (self));
}

static void
cc_mouse_panel_dispose (GObject *object)
{
  CcMousePanel *self = CC_MOUSE_PANEL (object);

  g_clear_object (&self->mouse_settings);
  g_clear_object (&self->touchpad_settings);
  g_clear_object (&self->interface_settings);

  G_OBJECT_CLASS (cc_mouse_panel_parent_class)->dispose (object);
}

static const char *
cc_mouse_panel_get_help_uri (CcPanel *panel)
{
  return "help:gnome-help/mouse";
}

static void
test_button_toggled_cb (CcMousePanel *self)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (self->test_button)))
    gtk_stack_set_visible_child (self->stack, GTK_WIDGET (self->mouse_test));
  else
    gtk_stack_set_visible_child (self->stack, GTK_WIDGET (self->preferences));
}

static void
cc_mouse_panel_init (CcMousePanel *self)
{
  GsdDeviceManager  *device_manager;

  g_resources_register (cc_mouse_get_resource ());

  cc_mouse_test_get_type ();
  gtk_widget_init_template (GTK_WIDGET (self));

  self->mouse_settings = g_settings_new ("org.gnome.desktop.peripherals.mouse");
  self->touchpad_settings = g_settings_new ("org.gnome.desktop.peripherals.touchpad");

  self->interface_settings = g_settings_new ("org.gnome.desktop.interface");

  device_manager = gsd_device_manager_get ();
  g_signal_connect_object (device_manager, "device-added",
                           G_CALLBACK (device_changed), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (device_manager, "device-removed",
                           G_CALLBACK (device_changed), self, G_CONNECT_SWAPPED);

  self->have_mouse = mouse_is_present ();
  self->have_touchpad = touchpad_is_present () || cc_synaptics_check ();
  /*                                              ^^^^^^^^^^^^^^^^^^^^^
   *            Workaround https://gitlab.gnome.org/GNOME/gtk/issues/97
   */
  self->have_touchscreen = touchscreen_is_present ();

  setup_dialog (self);
}

static void
cc_mouse_panel_class_init (CcMousePanelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  CcPanelClass *panel_class = CC_PANEL_CLASS (klass);

  panel_class->get_help_uri = cc_mouse_panel_get_help_uri;

  object_class->dispose = cc_mouse_panel_dispose;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/control-center/mouse/cc-mouse-panel.ui");

  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, disable_while_typing_switch);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, edge_scrolling_box);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, edge_scrolling_radio);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, mouse_group);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, mouse_natural_scrolling_radio);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, mouse_traditional_scrolling_radio);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, mouse_speed_scale);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, mouse_test);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, pointer_location_switch);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, primary_button_box);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, primary_button_left);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, primary_button_right);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, preferences);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, stack);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, tap_to_click_row);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, tap_to_click_switch);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, test_button);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, touchpad_group);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, touchpad_natural_scrolling_radio);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, touchpad_traditional_scrolling_radio);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, touchpad_speed_row);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, touchpad_speed_scale);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, touchpad_toggle_switch);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, two_finger_scrolling_box);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, two_finger_scrolling_radio);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, default_click_emulation_radio);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, area_click_emulation_radio);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, mouse_accel_combo);
  gtk_widget_class_bind_template_child (widget_class, CcMousePanel, touchpad_accel_combo);

  gtk_widget_class_bind_template_callback (widget_class, test_button_toggled_cb);
  gtk_widget_class_bind_template_callback (widget_class, on_click_method_checked_cb);
  gtk_widget_class_bind_template_callback (widget_class, on_scrolling_direction_checked_cb);

}
