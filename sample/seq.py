#!/usr/bin/env python3

import sys

import gi
gi.require_version('ALSASeq', '0.0')
from gi.repository import ALSASeq

# For event loop
from gi.repository import GLib

# For UNIX signal handling
import signal

client = ALSASeq.Client()

# Test Client object
print('Test Client object')
try:
    client.open('/dev/snd/seq', "sequencer-client")
except Exception as e:
    print(e)
    sys.exit()

print(' Name:           {0}'.format(client.get_property('name')))
print('  type:          {0}'.format(client.get_property('type').value_nick))
print('  ports:         {0}'.format(client.get_property('ports')))
print('  lost:          {0}'.format(client.get_property('lost')))
print('  output pool:   {0}'.format(client.get_property('output-pool')))
print('  input pool:    {0}'.format(client.get_property('input-pool')))
print('  output room:   {0}'.format(client.get_property('output-room')))
print('  output free:   {0}'.format(client.get_property('output-free')))
print('  input free:    {0}'.format(client.get_property('input-free')))

try:
    client.update()
except Exception as e:
    print(e)
    sys.exit()

# Test Port object
print('\nTest Port object')

# Add ports
my_ports = [0] * 3
for i in range(len(my_ports)):
    try:
        my_ports[i] = client.open_port('client-port:{0}'.format(i + 1))
    except Exception as e:
        print(e)
        sys.exit(1)

    types = my_ports[i].get_property('type')
    types |= ALSASeq.PortTypeFlag.MIDI_GENERIC
    types |= ALSASeq.PortTypeFlag.SOFTWARE
    types |= ALSASeq.PortTypeFlag.APPLICATION
    my_ports[i].set_property('type', types)

    caps = my_ports[i].get_property('capabilities')
    caps |= ALSASeq.PortCapFlag.READ
    caps |= ALSASeq.PortCapFlag.WRITE
    caps |= ALSASeq.PortCapFlag.SYNC_READ
    caps |= ALSASeq.PortCapFlag.SYNC_WRITE
    caps |= ALSASeq.PortCapFlag.DUPLEX
    caps |= ALSASeq.PortCapFlag.SUBS_READ
    caps |= ALSASeq.PortCapFlag.SUBS_WRITE
    caps |= ALSASeq.PortCapFlag.NO_EXPORT
    my_ports[i].set_property('capabilities', caps)

    cond_flags = my_ports[i].get_property('cond-flags')
    cond_flags |= ALSASeq.PortCondFlag.GIVEN_PORT
    cond_flags |= ALSASeq.PortCondFlag.TIMESTAMP
    cond_flags |= ALSASeq.PortCondFlag.TIME_REAL
    my_ports[i].set_property('cond-flags', cond_flags)

    my_ports[i].update()

client.update()
print(' {0} ports in this client.'.format(client.get_property('ports')))

def handle_event(port, name, flags, tag, queue, sec, nsec, src_client, src_port):
    print(' Event occur:')

    print('  name:      {0}'.format(name))
    print('  flags:     {0}'.format(flags))
    print('  tag:       {0}'.format(tag))
    print('  queue:     {0}'.format(queue))
    print('  sec:       {0}'.format(sec))
    print('  nsec:      {0}'.format(nsec))
    print('  src client:{0}'.format(src_client))
    print('  src port:  {0}'.format(src_port))
try:
    ports = client.get_ports()
except Exception as e:
    print(e)
    sys.exit()

for port in ports:
    try:
        port.update()
    except Exception as e:
        break

    port.connect('event', handle_event)

    print(' Name: {0}'.format(port.get_property('name')))
    print('  number:        {0}'.format(port.get_property('number')))
    print('  type:          0 -> {0}'.format(port.get_property('type')))
    print('  capabilities:  {0}'.format(port.get_property('capabilities')))
    print('  midi channels: {0}'.format(port.get_property('midi-channels')))
    print('  midi voices:   {0}'.format(port.get_property('midi-voices')))
    print('  synth voices:  {0}'.format(port.get_property('synth-voices')))
    print('  cond-flags:    {0}'.format(port.get_property('cond-flags')))
    print('  timestamp queue: {0}'.format(port.get_property('timestamp-queue')))
    print('  read use:      {0}'.format(port.get_property('read-use')))
    print('  write use:     {0}'.format(port.get_property('write-use')))
    print('')

try:
    client.listen()
except Exception as e:
    print(e)
    sys.exit(1)

# handle unix signal
def handle_unix_signal(loop):
    loop.quit()
loop = GLib.MainLoop()
GLib.unix_signal_add(GLib.PRIORITY_HIGH, signal.SIGINT, \
                     handle_unix_signal, loop)
loop.run()

client.unlisten()
sys.exit()
