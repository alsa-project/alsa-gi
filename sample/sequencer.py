#!/usr/bin/env python3

import sys

#from gi.repository import Gtk
from gi.repository import ALSASeq

# Test Client object
print('Test Client object')
try:
    client = ALSASeq.Client.new('default')
except Exception as e:
    print(e)
    sys.exit(1)

client.set_property('name', 'sequencer-client')

print(' Name:           {0}'.format(client.get_property('name')))
print('  id:            {0}'.format(client.get_property('id')))
print('  type:          {0}'.format(client.get_property('type')))
print('  ports:         {0}'.format(client.get_property('ports')))
print('  lost:          {0}'.format(client.get_property('lost')))
print('  output buffer: {0}'.format(client.get_property('output-buffer')))
print('  input buffer:  {0}'.format(client.get_property('input-buffer')))
print('  output pool:   {0}'.format(client.get_property('output-pool')))
print('  input pool:    {0}'.format(client.get_property('input-pool')))
print('  output room:   {0}'.format(client.get_property('output-room')))
print('  broadcast:     {0}'.format(client.get_property('broadcast-filter')))
print('  error bounce:  {0}'.format(client.get_property('error-bounce')))

try:
    status = client.get_pool_status()
except Exception as e:
    print(e)
    sys.exit(1)

print(' Pool status:')
print('  output free:   {0}'.format(status[0]))
print('  input free:    {0}'.format(status[1]))

ports = [0] * 3

# Test Port object
print('\nTest Port object')
for i in range(len(ports)):
    try:
        ports[i] = ALSASeq.Port.new(client, 'client-port:{0}'.format(i+1))
    except Exception as e:
        print(e)
        sys.exit(1)

    try:
        ports[i].set_property('type', (1 << 20) | (1 << 17) | (1 << 1))
    except Exception as e:
        print(e)
        sys.exit(1)

    print(' Name: {0}'.format(ports[i].get_property('name')))
    print('  id:            {0}'.format(ports[i].get_property('id')))
    print('  type:          0 -> {0}'.format(ports[i].get_property('type')))
    print('  capabilities:  {0}'.format(ports[i].get_property('capabilities')))
    print('  midi channels: {0}'.format(ports[i].get_property('midi-channels')))
    print('  midi voices:   {0}'.format(ports[i].get_property('midi-voices')))
    print('  synth voices:  {0}'.format(ports[i].get_property('synth-voices')))
    print('  port specified:{0}'.format(
                                    ports[i].get_property('port-specified')))
    print('  timestamping:  {0}'.format(ports[i].get_property('timestamping')))
    print('  timestamp queue: {0}'.format(
                                    ports[i].get_property('timestamp-queue')))
    print('  read use:      {0}'.format(ports[i].get_property('read-use')))
    print('  write use:     {0}'.format(ports[i].get_property('write-use')))

    print(' Ports in client:{0}'.format(client.get_property('ports')))
