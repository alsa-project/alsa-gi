#!/usr/bin/env python3

import sys

# ALSACtl-1.0 gir
from gi.repository import ALSACtl

# For event loop
from gi.repository import GLib

# For UNIX signal handling
import signal

# To get system time for element uniq name
import time

elements = []

# Create client nad open
client = ALSACtl.Client()
try:
    client.open('/dev/snd/controlC0')
except Exception as e:
    print(e)
    sys.exit()

# Handle GObject signal
def handle_added_event(client, id):
    print('  An element {0} is added.'.format(id))
client.connect('added', handle_added_event)

# Listen asynchronous event from the other processes
print('start listening')
try:
    client.listen()
except Exception as e:
    print(e)
    sys.exit()

# Add my int elements
name = 'int-element-{0}'.format(time.strftime('%S'))
try:
    elems = client.add_int_elems(2, 2, name, 10, 0, 10, 1)
    for elem in elems:
        print(elem.get_min())
        print(elem.get_max())
        print(elem.get_step())
        vals = elem.read()
        print(vals)
        elem.write(vals)
        elem.unlock()
except Exception as e:
    print(e)
    sys.exit()
elements.extend(elems)

# Add my bool elements
name = 'bool-element-{0}'.format(time.strftime('%S'))
try:
    elems = client.add_bool_elems(2, 3, name, 8)
    for elem in elems:
        vals = elem.read()
        print(vals)
        elem.write(vals)
        elem.unlock()
except Exception as e:
    print(e)
    sys.exit()
elements.extend(elems)

# Add my enum elements
name = 'enum-element-{0}'.format(time.strftime('%S'))
items = ('lucid', 'maverick', 'natty', 'oneiric', 'precise')
try:
    elems = client.add_enum_elems(2, 4, name, 6, items)
    for elem in elems:
        elem.unlock()
        print(elem.get_items())
        vals = elem.read()
        print(vals)
        elem.write(vals)
    elements.extend(elems)
except Exception as e:
    print(e)
    sys.exit()

# Add my byte elements
name = 'byte-element-{0}'.format(time.strftime('%S'))
try:
    elems = client.add_byte_elems(2, 5, name, 4)
    for elem in elems:
        vals = elem.read()
        print(vals)
        elem.write(vals)
        elem.unlock()
    elements.extend(elems)
except Exception as e:
    print(e)
    sys.exit()

# Add my iec60958 elements
name = 'iec60958-element-{0}'.format(time.strftime('%S'))
try:
    elems = client.add_iec60958_elems(2, 6, name)
    for elem in elems:
        elem.unlock()
    elements.extend(elems)
except Exception as e:
    print(e)
    sys.exit()

# Get current element list
try:
    element_list = client.get_elem_list()
except Exception as e:
    print(e)
    sys.exit()

# Print element and its properties
properties = ('id', 'type', 'iface', 'device', 'subdevice', 'channels',
              'readable', 'writable', 'volatile', 'inactive', 'locked',
              'is-owned', 'is-user')
for i in element_list:
    try:
        element = client.get_elem(i);
    except Exception as e:
        print(e)
        sys.exit()
    print('\n{0}'.format(element.get_property('name')))
    for n in properties:
        print('    {0}:	{1}'.format(n, element.get_property(n)))
    elements.append(element)

# Assign GObject signal handlers
def handle_changed_event(elem):
    print('  changed: {0}'.format(elem.get_property('id')));
    print(elem.read())
def handle_updated_event(elem, index):
    print('  updated: {0}'.format(elem.get_property('id')));
def handle_removed_event(elem):
    print('  removed: {0}',format(elem.get_property('id')));
for e in elements:
    e.connect('changed', handle_changed_event)
    e.connect('updated', handle_updated_event)
    e.connect('removed', handle_removed_event)

# handle unix signal
def handle_unix_signal(self):
    loop.quit()
GLib.unix_signal_add(GLib.PRIORITY_HIGH, signal.SIGINT, \
                     handle_unix_signal, None)

# Event loop
loop = GLib.MainLoop()
loop.run()

client.unlisten()

print('stop listening')
sys.exit()
