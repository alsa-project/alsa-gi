#!/usr/bin/env python3

import sys

# ALSATimer-1.0 gir
from gi.repository import ALSATimer

# For event loop
from gi.repository import GLib

# For UNIX signal handling
import signal

# Create client and open
client = ALSATimer.Client()
try:
    client.open('/dev/snd/timer')
except Exception as e:
    print(e)
    sys.exit()

try:
    timers = client.get_timer_list()
except Exception as e:
    print(e)
    sys.exit()

# Test Client object
print(' Name:   {0}'.format(client.get_property('name')))
print('  Information:')
print('   id:           {0}'.format(client.get_property('id')))
print('   is-slave:     {0}'.format(client.get_property('is-slave')))
print('   card:         {0}'.format(client.get_property('card')))
print('   resolution:   {0}'.format(client.get_property('resolution')))

print('  Parameters:')
print('   auto-start:   {0}'.format(client.get_property('auto-start')))
print('   exclusive:    {0}'.format(client.get_property('exclusive')))
print('   early-event:  {0}'.format(client.get_property('early-event')))
print('   ticks:        {0}'.format(client.get_property('ticks')))
print('   queue-size:   {0}'.format(client.get_property('queue-size')))
print('   filter:       {0}'.format(client.get_property('filter')))

try:
    status = client.get_status()
except Exception as e:
    print(e)
    sys.exit()

print(' Status:')
print('  timestamp:')
print('   sec:          {0}'.format(status[0]))
print('   nsec:         {0}'.format(status[1]))
print('  losts:         {0}'.format(status[2]))
print('  overrun:       {0}'.format(status[3]))
print('  queue:         {0}'.format(status[4]))

# Test Client actions
print(' Actions:')
def handle_event(client, event, sec, nsec, value):
    print('   Event: {0}:'.format(event))
    print('    client:    {0}'.format(client.get_property('name')))
    print('    timestamp:')
    print('     sec:      {0}'.format(sec))
    print('     nsec:     {0}'.format(nsec))
    print('    value:     {0}'.format(value))

client.set_property('auto-start', True)
client.set_property('exclusive', True)
client.set_property('early-event', False)
client.set_property('ticks', 500)
client.set_property('filter', 0xbf)
client.connect('event', handle_event)

print('  Changed parameters:')
print('   auto-start:   {0}'.format(client.get_property('auto-start')))
print('   exclusive:    {0}'.format(client.get_property('exclusive')))
print('   early-event:  {0}'.format(client.get_property('early-event')))
print('   ticks:        {0}'.format(client.get_property('ticks')))
print('   queue-size:   {0}'.format(client.get_property('queue-size')))
print('   filter:       {0}'.format(client.get_property('filter')))

try:
    client.start()
except Exception as e:
    print(e)
    sys.exit()
print('  started')

# Handle unix signal
def handle_unix_signal(self):
    loop.quit()
GLib.unix_signal_add(GLib.PRIORITY_HIGH, signal.SIGINT, \
                     handle_unix_signal, None)

# Event loop
loop = GLib.MainLoop()
loop.run()

client.stop()
print('  stopped')

sys.exit()
