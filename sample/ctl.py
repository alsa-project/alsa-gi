#!/usr/bin/env python3

import sys

# ALSACtl-1.0 gir
from gi.repository import ALSACtl

# For event loop
from gi.repository import GLib

# For UNIX signal handling
import signal

# To get system time for elemset uniq name
import time

elemsets = []

# Create client nad open
client = ALSACtl.Client()
try:
    client.open('/dev/snd/controlC1')
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

# Add my int element set
name = 'int-elemset-{0}'.format(time.strftime('%S'))
try:
    elemset = client.add_elemset_int(2, name, 10, 0, 10, 1)
    elemset.unlock()
except Exception as e:
    print(e)
    sys.exit()
elemsets.append(elemset)

# Add my bool element set
name = 'bool-elemset-{0}'.format(time.strftime('%S'))
try:
    elemset = client.add_elemset_bool(2, name, 8)
    elemset.unlock()
except Exception as e:
    print(e)
    sys.exit()
elemsets.append(elemset)

# Add my enum element set
name = 'enum-elemset-{0}'.format(time.strftime('%S'))
labels = ('lucid', 'maverick', 'natty', 'oneiric', 'precise')
try:
    elemset = client.add_elemset_enum(2, name, 6, labels)
    elemset.unlock()
except Exception as e:
    print(e)
    sys.exit()
elemsets.append(elemset)

# Add my byte element set
name = 'byte-elemset-{0}'.format(time.strftime('%S'))
try:
    elemset = client.add_elemset_byte(2, name, 4)
    elemset.unlock()
except Exception as e:
    print(e)
    sys.exit()
elemsets.append(elemset)

# Add my iec60958 element set
name = 'iec60958-elemset-{0}'.format(time.strftime('%S'))
try:
    elemset = client.add_elemset_iec60958(2, name)
    elemset.unlock()
except Exception as e:
    print(e)
    sys.exit()
elemsets.append(elemset)

# Get current element set list
try:
    elemset_list = client.get_elemset_list()
except Excepion as e:
    print(e)
    sys.exit()

# Print element set and its properties
properties = ('id', 'type', 'iface', 'device', 'subdevice', 'count',
              'readable', 'writable', 'volatile', 'inactive', 'locked',
              'is-owned', 'is-user')
for i in elemset_list:
    try:
        elemset = client.get_elemset(i);
    except Exception as e:
        print(e)
        sys.exit()
    print('\n{0}'.format(elemset.get_property('name')))
    for n in properties:
        print('    {0}:	{1}'.format(n, elemset.get_property(n)))
    elemsets.append(elemset)

# Assign GObject signal handlers
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
