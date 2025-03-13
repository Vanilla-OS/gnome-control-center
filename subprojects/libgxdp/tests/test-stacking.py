#!/usr/bin/env python3

import os
import subprocess
import sys

[_, wayland_client, x11_client, test_portal] = sys.argv

os.environ['G_DEBUG'] = 'fatal-warnings'

wayland_client_proc = subprocess.Popen(wayland_client, stdout=subprocess.PIPE)
x11_client_proc = subprocess.Popen(x11_client, stdout=subprocess.PIPE)

xdg_foreign_id = wayland_client_proc.stdout.readline().strip()
xid = x11_client_proc.stdout.readline().strip()

test_portal_proc = subprocess.Popen([
  test_portal,
  xdg_foreign_id,
  xid,
])

ret = test_portal_proc.wait()
assert ret == 0

wayland_client_proc.terminate()
wayland_client_proc.wait()

x11_client_proc.terminate()
x11_client_proc.wait()
