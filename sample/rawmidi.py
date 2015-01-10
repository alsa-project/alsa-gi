#!/usr/bin/env python3

import sys

#from gi.repository import Gtk
from gi.repository import ALSARawmidi

# Test Client object
try:
    client = ALSARawmidi.Client.new('default')
except Exception as e:
    print(e)
    sys.exit(1)

