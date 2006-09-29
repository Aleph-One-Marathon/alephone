#!/usr/bin/perl

#
#
#	Copyright (C) 1991-2002 and beyond by Bungie Studios, Inc.
#	and the "Aleph One" developers.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	This license is contained in the file "COPYING",
#	which is included with this source code; it is available online at
#	http://www.gnu.org/licenses/gpl.html
#
#

$FORMAT_INT16 = "n"; # 16 bit unsigned short, network (big-endian) order
$FORMAT_INT32 = "N"; # 32 bit unsigned long, network (big-endian) order

$SIZE_INT16 = 2;
$SIZE_INT32 = 4;

$BUFFER_SIZE = 1024; # Max Chunk to read/write data to be nice on memory

# Main routine 
local($source_name, $data_name, $rsrc_name);
local($id, $version);

if($#ARGV < 0 || $#ARGV > 2)
{
	print "Usage: $0 SOURCE [DATA [RESOURCE]]\nExtract data and resource forks from AppleSingle file.\n";
	exit(1);
}

# If the destination file names are not specified, the data fork
# will go to <name>.data and the resource fork to <name>.rsrc

$source_name = shift @ARGV;
$data_name = shift @ARGV;
$rsrc_name = shift @ARGV;

$data_name = $source_name.".data" unless(length($data_name));
$rsrc_name = $source_name.".rsrc" unless(length($rsrc_name));

# Open Source file
open(FIN, $source_name);
binmode FIN;

read(FIN, $id, $SIZE_INT32);
read(FIN, $version, $SIZE_INT32);
$id = unpack($FORMAT_INT32, $id);
$version = unpack($FORMAT_INT32, $version);
if ($id != 0x00051600 || $version != 0x00020000) {
	die "$source_name is not a version 2 AppleSingle file.\n"
}

extract_fork(*FIN, 1, $data_name, "data");
extract_fork(*FIN, 2, $rsrc_name, "resource");

exit(0);

sub extract_fork
{
	local(*FIN, $request_id, $file_name, $fork_name) = @_;

	local($id, $fork_start, $fork_size);
	local($num_entries, $fork_found);
	
	# Look for fork in source file
	seek(FIN, 0x18, SEEK_SET);
	read(FIN, $num_entries, $SIZE_INT16);
	$num_entries = unpack($FORMAT_INT16, $num_entries);
	while($num_entries--)
	{
		local($id, $ofs, $len);
		read(FIN, $id, $SIZE_INT32);
		read(FIN, $ofs, $SIZE_INT32);
		read(FIN, $len, $SIZE_INT32);
		$id = unpack($FORMAT_INT32, $id);
		$ofs = unpack($FORMAT_INT32, $ofs);
		$len = unpack($FORMAT_INT32, $len);
		if ($id == $request_id) {
			$fork_found = 1;
			$fork_start = $ofs;
			$fork_size = $len;
		}
	}
	
	if(!$fork_found)
	{
		print STDERR "Warning: source file doesn't contain a ".$fork_name." fork.\n";
		return;
	}
	
	# Found, open destination file
	open(FOUT, ">$file_name") || die "Can't open destination file $file_name";
	binmode FOUT;
	
	# Copy fork
	seek(FIN, $fork_start, SEEK_SET);
	while ($fork_size) {
		local($length, $buffer);
		if($fork_size > $BUFFER_SIZE) { $length = $BUFFER_SIZE; }
		else { $length = $fork_size; }

		read(FIN, $buffer, $length);
		print FOUT $buffer;

		$fork_size -= $length;
	}
	
	close(FOUT);
}
