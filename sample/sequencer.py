#!/usr/bin/env python3

from gi.repository import ALSASeq

seq = ALSASeq.Seq.new("default")
print("sequencer info:")
print("	name:	{0}".format(seq.get_name()))
print("	client:	{0}".format(seq.get_client_id()))

print("\nbuffer size:")
print("	output:	{0}".format(seq.get_output_buffer_size()))
print("	input:	{0}".format(seq.get_input_buffer_size()))

print("\ntrials to set buffer size as 1000 bytes:")
print("	output: {0}".format(seq.set_output_buffer_size(1000)))
print("	input:	{0}".format(seq.set_input_buffer_size(1000)))

print("\nresults")
print("	output:	{0}".format(seq.get_output_buffer_size()))
print("	input:	{0}".format(seq.get_input_buffer_size()))
