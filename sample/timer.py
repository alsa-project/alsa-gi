#!/usr/bin/env python3

import sys

from gi.repository import ALSATimer

# Test query object
print('Test Query object')
try:
    query = ALSATimer.Query.new()
except Exception as e:
    print(e)
    sys.exit(1)

print('Timers:')
while True:
    print(' Name:       {0}'.format(query.get_property('name')))
    print('  id:        {0}'.format(query.get_property('id')))
    print('  class:     {0}'.format(query.get_property('class')))
    print('  sub-class: {0}'.format(query.get_property('sub-class')))
    print('  card:      {0}'.format(query.get_property('card')))
    print('  device:    {0}'.format(query.get_property('device')))
    print('  sub-device:{0}'.format(query.get_property('sub-device')))

    try:
        query = query.adjoin()
    except Exception as e:
        break

# Test Client object
def handle_event(client, event, sec, nsec, value):
    print('   Event:')
    print('    timestamp:')
    print('     sec:      {0}'.format(sec))
    print('     nsec:     {0}'.format(nsec))
    print('    event:     {0}'.format(event))
    print('    value:     {0}'.format(value))

try:
    query = ALSATimer.Query.new()
except Exception as e:
    print(e)
    sys.exit(1)

print('\nTimer clients:')
while True:
    try:
        client = query.get_client()
    except Exception as e:
        print(e)
        break

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

    print('  Changed parameters:')
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
        break

    print(' Status:')
    print('  timestamp:')
    print('   sec:          {0}'.format(status[0]))
    print('   nsec:         {0}'.format(status[1]))
    print('  losts:         {0}'.format(status[2]))
    print('  overrun:       {0}'.format(status[3]))
    print('  queue:         {0}'.format(status[4]))

    print(' Actions:')
    client.connect('event', handle_event)
    try:
        client.start()
    except Exception as e:
        print(e)
        break
    print('  started')
    try:
        client.stop()
    except Exception as e:
        print(e)
        break
    print('  stopped')
    try:
        client.resume()
    except Exception as e:
        print(e)
        break
    print('  continued')
    try:
        client.stop()
    except Exception as e:
        print(e)
        break
    print('  stopped\n')

    try:
        query = query.adjoin()
    except Exception as e:
        break
