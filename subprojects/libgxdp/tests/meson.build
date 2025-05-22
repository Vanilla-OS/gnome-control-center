gtk_wayland_dep = dependency('gtk4-wayland')
gtk_x11_dep = dependency('gtk4-x11')

test_wayland_client = executable('test-wayland-client',
  sources: 'test-wayland-client.c',
  dependencies: [
    gtk_dep,
    gtk_wayland_dep,
  ],
)

test_x11_client = executable('test-x11-client',
  sources: 'test-x11-client.c',
  dependencies: [
    gtk_dep,
    gtk_x11_dep,
  ],
)

test_portal = executable('test-portal',
  sources: 'test-portal.c',
  dependencies: [
    libgxdp_dep,
    gtk_wayland_dep,
  ],
)

test_env = environment()
test_env.set('GTK_A11Y', 'none')

dbus_run_session = find_program('dbus-run-session')
mutter = find_program('mutter')
test_stacking = find_program('test-stacking.py')
test('stacking', dbus_run_session,
  args: [
    '--',
    mutter.full_path(),
    '--headless',
    '--virtual-monitor', '800x600',
    '--wayland-display', 'libgxdp-test-display',
    '--',
    test_stacking.full_path(),
    test_wayland_client.full_path(),
    test_x11_client.full_path(),
    test_portal.full_path(),
  ],
  env: test_env,
  is_parallel: false,
)
