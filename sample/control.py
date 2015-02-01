#!/usr/bin/env python3

import sys

#from gi.repository import Gtk
from gi.repository import ALSACtl

# Gtk+3 gir
from gi.repository import Gtk

elemsets = []

# Create client
def handle_added_event(ctl, id):
    print('  An element {0} is added.'.format(id))
try:
    ctl = ALSACtl.Client.open("hw:0")
    ctl.listen()
    ctl.connect('added', handle_added_event)
except Exception as e:
    print(e)
    sys.exit()

# Add my element set
try:
    elemset = ctl.add_elemset(2, "my-elemset", 10, 0, 10, 1)
    elemset.unlock()
except Exception as e:
    print(e)
    sys.exit()
elemsets.append(elemset)

# Get element set list
try:
    elemset_list = ctl.get_elemset_list()
except Excepion as e:
    print(e)
    sys.exit()
print(elemset_list)

# Print element set
properties = ('id', 'type', 'iface', 'device', 'subdevice', 'count',
              'readable', 'writable', 'volatile', 'inactive', 'locked',
              'is-mine', 'is-user')
for i in elemset_list:
    try:
        elemset = ctl.get_elemset(i);
    except Exception as e:
        print(e)
        sys.exit()
    print('\n{0}'.format(elemset.get_property('name')))
    for n in properties:
        print('    {0}:	{1}'.format(n, elemset.get_property(n)))
    elemsets.append(elemset)

# Assign signal handlers
def handle_changed_event(elem):
    print('  changed: {0}'.format(elem.get_property('name')));
def handle_updated_event(elem, index):
    print('  updated: {0}'.format(elem.get_property('name')));
def handle_removed_event(elem):
    print('  removed: {0}',format(elem.get_property('name')));
for e in elemsets:
    e.connect('changed', handle_changed_event)
    e.connect('updated', handle_updated_event)
    e.connect('removed', handle_removed_event)

# GUI
class Sample(Gtk.Window):

    def __init__(self):
        Gtk.Window.__init__(self, title="Hinawa-1.0 gir sample")
        self.set_border_width(20)

        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        self.add(vbox)

        topbox = Gtk.Box(spacing=10)
        vbox.pack_start(topbox, True, True, 0)

        button = Gtk.Button("transact")
        button.connect("clicked", self.on_click_transact)
        topbox.pack_start(button, True, True, 0)

        button = Gtk.Button("_Close", use_underline=True)
        button.connect("clicked", self.on_click_close)
        topbox.pack_start(button, True, True, 0)

        bottombox = Gtk.Box(spacing=10)
        vbox.pack_start(bottombox, True, True, 0)

        self.entry = Gtk.Entry()
        self.entry.set_text("0xfffff0000980")
        bottombox.pack_start(self.entry, True, True, 0)

        self.label = Gtk.Label("result")
        self.label.set_text("0x00000000")
        bottombox.pack_start(self.label, True, True, 0)

    def on_click_transact(self, button):
        print('clicked')

    def on_click_close(self, button):
        print("Closing application")
        Gtk.main_quit()

# Main logic
win = Sample()
win.connect("delete-event", Gtk.main_quit)
win.show_all()

Gtk.main()

sys.exit()
