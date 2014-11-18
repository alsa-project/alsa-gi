#!/usr/bin/env python3

from gi.repository import ALSASeq

try:
	client = ALSASeq.Client.new("default")
except Exception as e:
	print(e)
	exit(1)

print("client:")
print("	name:	{0}".format(client.get_name()))
print("	id:	{0}".format(client.get_id()))

print("\nbuffer size:")
print("	output:	{0}".format(client.get_output_buffer_size()))
print("	input:	{0}".format(client.get_input_buffer_size()))

print("\ntrials to set buffer size as 1000 bytes:")
try:
	current = client.set_output_buffer_size(1000)
except Exception as e:
	print(e)
	exit()
print("	output: {0}".format(current))
try:
	current = client.set_input_buffer_size(1000)
except Exception as e:
	print(e)
	exit()
print("	output: {0}".format(current))

print("\nresults")
print("	output:	{0}".format(client.get_output_buffer_size()))
print("	input:	{0}".format(client.get_input_buffer_size()))

try:
	info = client.get_info()
except Exception as e:
	print(e)
	exit()
print("info:")
print("	name:	{0}".format(info.get_name()))
print("	bcast:	{0}".format(info.get_broadcast_filter()))
print("	error:	{0}".format(info.get_error_bounce()))
print("	events:	{0}".format(info.get_event_filter()))
print("	ports:	{0}".format(info.get_num_ports()))
print("	losts:	{0}".format(info.get_event_lost()))

info.set_name("arbitrary name")
print("changed name: {0}".format(info.get_name()))

client.set_info(info)

try:
	info = client.get_info()
except Exception as e:
	print(e)
	exit()
print("new info:")
print("	name:	{0}".format(info.get_name()))

try:
	system = client.get_system_info()
except Exception as e:
	print(e)
	exit()
print("system:")
print("	queues:		{0}".format(system.get_max_queues()))
print("	clients:	{0}".format(system.get_max_clients()))
print("	ports:		{0}".format(system.get_max_ports()))
print("	cur queues:	{0}".format(system.get_cur_queues()))
print("	cur clients:	{0}".format(system.get_cur_clients()))
try:
	port = client.create_port()
except Exception as e:
	print(e)
	exit()
print("port:")
print("	name:		{0}".format(port.get_property("name")));
print("	capabilities:	{0}".format(port.get_property("capabilities")));
print("	type:		{0}".format(port.get_property("type")));
print("	midi channels:	{0}".format(port.get_property("midi_channels")));
print("	midi voices:	{0}".format(port.get_property("midi_voices")));
print("	synth voices:	{0}".format(port.get_property("synth_voices")));
print("	read use:	{0}".format(port.get_property("read_use")));
print("	write use:	{0}".format(port.get_property("write_use")));
print("	port specified	{0}".format(port.get_property("port_specified")))
print("	timestamping	{0}".format(port.get_property("timestamping")))
print("	real timestamp	{0}".format(port.get_property("timestamp_real")))
print("	queue ID for timestamp	{0}".format(port.get_property("timestamp_queue")))
