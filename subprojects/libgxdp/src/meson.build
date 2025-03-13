gnome = import('gnome')

top_srcdir = meson.project_source_root()
dbus_interfaces = files(
  top_srcdir / 'data' / 'org.gnome.Mutter.ServiceChannel.xml',
)

sources = files(
  'gxdp.c',
  'gxdp-external-window.c',
)

built_sources = []

dependencies = [
  gtk_dep,
]

cdata = configuration_data()

gtk_wayland_dep = dependency('gtk4-wayland', required: false)
if gtk_wayland_dep.found()
  wayland_scanner = find_program('wayland-scanner')

  wayland_protocols_dir = top_srcdir / 'data'

  wayland_protocols = [
    'mutter-x11-interop',
  ]

  foreach protocol : wayland_protocols
    client_header = custom_target('@0@ client header'.format(protocol),
      input: wayland_protocols_dir / '@0@.xml'.format(protocol),
      output: '@0@-client-protocol.h'.format(protocol),
      command: [
        wayland_scanner,
        'client-header',
        '@INPUT@', '@OUTPUT@',
      ]
    )
    protocol_code = custom_target('@0@ source'.format(protocol),
      input: wayland_protocols_dir / '@0@.xml'.format(protocol),
      output: '@0@-protocol.c'.format(protocol),
      command: [
        wayland_scanner,
        'private-code',
        '@INPUT@', '@OUTPUT@',
      ]
    )

    built_sources += gnome.gdbus_codegen(
      'gxdp-dbus',
      sources: dbus_interfaces,
      namespace: 'GxdpDBus',
      interface_prefix: 'org.gnome.',
    )

    built_sources += [client_header, protocol_code]
    dependencies += [gtk_wayland_dep]
  endforeach

  sources += files([
    'gxdp-external-window-wayland.c',
    'gxdp-wayland.c',
  ])
  dependencies += [
    dependency('wayland-client'),
    gtk_wayland_dep,
  ]
  cdata.set('GXDP_HAVE_GTK_WAYLAND', true)
endif

gtk_x11_dep = dependency('gtk4-x11', required: false)
if gtk_x11_dep.found()
  sources += files('gxdp-external-window-x11.c')
  dependencies += [
    dependency('x11'),
    gtk_x11_dep,
  ]
  cdata.set('GXDP_HAVE_GTK_X11', true)
endif

gxdp_config_h = configure_file(
  input: 'gxdp-config.h.meson',
  output: 'gxdp-config.h',
  configuration: cdata
)

built_sources += [gxdp_config_h]

libgxdp_dep = declare_dependency(
  sources: sources + built_sources,
  include_directories: include_directories('.'),
  dependencies: dependencies,
)
meson.override_dependency('libgxdp', libgxdp_dep)
