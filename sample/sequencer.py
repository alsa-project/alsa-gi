#!/usr/bin/env python3

from gi.repository import ALSASeq

try:
	seq = ALSASeq.Seq.new("default")
except Exception as e:
	print(e)
	exit(1)

print("sequencer info:")
print("	name:	{0}".format(seq.get_name()))
print("	client:	{0}".format(seq.get_client_id()))

print("\nbuffer size:")
print("	output:	{0}".format(seq.get_output_buffer_size()))
print("	input:	{0}".format(seq.get_input_buffer_size()))

print("\ntrials to set buffer size as 1000 bytes:")
try:
	current = seq.set_output_buffer_size(1000)
except Exception as e:
	print(e)
	exit()
print("	output: {0}".format(current))
try:
	current = seq.set_input_buffer_size(1000)
except Exception as e:
	print(e)
	exit()
print("	output: {0}".format(current))

print("\nresults")
print("	output:	{0}".format(seq.get_output_buffer_size()))
print("	input:	{0}".format(seq.get_input_buffer_size()))
