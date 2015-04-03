#!/usr/bin/env python3

import sys

# ALSASeq-1.0 gir
from gi.repository import ALSASeq

# For event loop
from gi.repository import GLib

# For UNIX signal handling
import signal

client = ALSASeq.Client()

# Test Client object
print('Test Client object')
try:
    client.open('/dev/snd/', "sequencer-client")
except Exception as e:
    print(e)
    sys.exit()

print(' Name:           {0}'.format(client.get_property('name')))
print('  id:            {0}'.format(client.get_property('id')))
print('  type:          {0}'.format(client.get_property('type')))
print('  ports:         {0}'.format(client.get_property('ports')))
print('  lost:          {0}'.format(client.get_property('lost')))
print('  output pool:   {0}'.format(client.get_property('output-pool')))
print('  input pool:    {0}'.format(client.get_property('input-pool')))
print('  output room:   {0}'.format(client.get_property('output-room')))
print('  output free:   {0}'.format(client.get_property('output-free')))
print('  input free:    {0}'.format(client.get_property('input-free')))
print('  broadcast:     {0}'.format(client.get_property('broadcast-filter')))
print('  error bounce:  {0}'.format(client.get_property('error-bounce')))


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

    my_ports[i].set_property('type', (1 << 20) | (1 << 17) | (1 << 1))
    my_ports[i].set_property('capabilities', 0x7f)
    my_ports[i].update()

client.update()
print(' {0} ports in this client.'.format(client.get_property('ports')))

def handle_event(port, type, flags, tag, queue, sec, nsec, src_client, src_port):
    print(' Event occur:')

    print('  type:      {0}'.format(type))
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
    print('  id:            {0}'.format(port.get_property('id')))
    print('  type:          0 -> {0}'.format(port.get_property('type')))
    print('  capabilities:  {0}'.format(port.get_property('capabilities')))
    print('  midi channels: {0}'.format(port.get_property('midi-channels')))
    print('  midi voices:   {0}'.format(port.get_property('midi-voices')))
    print('  synth voices:  {0}'.format(port.get_property('synth-voices')))
    print('  port specified:{0}'.format(
                                    port.get_property('port-specified')))
    print('  timestamping:  {0}'.format(port.get_property('timestamping')))
    print('  timestamp queue: {0}'.format(
                                    port.get_property('timestamp-queue')))
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
