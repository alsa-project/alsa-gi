#!/usr/bin/env python3

import sys

import gi
gi.require_version('ALSATimer', '0.0')
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
print('   card:         {0}'.format(client.get_property('card')))
print('   resolution:   {0}'.format(client.get_property('resolution')))
print('   params:       {0}'.format(client.get_property('params')))

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

params = client.get_property('params')
params |= ALSATimer.DeviceParamFlag.AUTO
params |= ALSATimer.DeviceParamFlag.EXCLUSIVE
params &= ~ALSATimer.DeviceParamFlag.EARLY_EVENT
client.set_property('params', params)

client.set_property('ticks', 500)

filter = client.get_property('filter')
filter |= ALSATimer.EventTypeFlag.RESOLUTION
filter |= ALSATimer.EventTypeFlag.TICK
filter |= ALSATimer.EventTypeFlag.START
filter |= ALSATimer.EventTypeFlag.STOP
filter |= ALSATimer.EventTypeFlag.CONTINUE
filter |= ALSATimer.EventTypeFlag.PAUSE
filter |= ALSATimer.EventTypeFlag.EARLY
filter |= ALSATimer.EventTypeFlag.SUSPEND
filter |= ALSATimer.EventTypeFlag.RESUME
filter |= ALSATimer.EventTypeFlag.MSTART
filter |= ALSATimer.EventTypeFlag.MSTOP
filter |= ALSATimer.EventTypeFlag.MCONTINUE
filter |= ALSATimer.EventTypeFlag.MPAUSE
filter |= ALSATimer.EventTypeFlag.MSUSPEND
filter |= ALSATimer.EventTypeFlag.MRESUME
client.set_property('filter', filter)

client.connect('event', handle_event)

print('  Changed parameters:')
params = client.get_property('params')
print('   params:       {0}'.format(params))
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
