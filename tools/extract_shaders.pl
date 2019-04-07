#!/usr/bin/perl

# extract_shaders.pl
#
#	Copyright (C) 2009 by Jeremiah Morris and the Aleph One developers
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
# Creates a plugin from the hardcoded shader programs.

use strict;
use warnings 'FATAL' => 'all';
use FindBin;

## Read the default shaders and store them

my %shaders;
my $sourcepath = "$FindBin::Bin/../Source_Files/RenderMain/OGL_Shader.cpp";
my $file;
open($file, '<', $sourcepath) or die "Could not read $sourcepath: $!";

OUTER:
while (my $line = <$file>)
{
  if ($line =~ /^\s*default(Vertex|Fragment)Programs\["(\w+)"\]\s*=\s*""\s*$/)
  {
    my $name = $2;
    my $type = $1;
    my $prog = "";
    while (1)
    {
      $line = <$file>;
      last OUTER unless $line;
      if ($line =~ /^\s*"(.*?)(?:\\n)?"/)
      {
        $prog .= "$1\n";
      }
      last if ($line =~ /;\s*$/);
    }
    $shaders{$name}{$type} = $prog;
  }
  elsif ($line =~ /^\s*default(Vertex|Fragment)Programs\["(\w+)"\]\s*=\s*default(Vertex|Fragment)Programs\["(\w+)"\];\s*$/)
  {
    my $name = $2;
    my $type = $1;
    my $othername = $4;
    my $othertype = $3;
    
    $shaders{$name}{$type} = $shaders{$othername}{$othertype};
  }
}
close($file);


## Create plugin directory and xml/mml files

my $version = `grep '^#define A1_DATE_VERSION' "$FindBin::Bin/../Source_Files/Misc/alephversion.h" | sed -e 's/\\(.*\\"\\)\\(.*\\)\\(\\"\\)/\\2/g' | tr -d '\\n'`;

my $plugin = "Default Shaders";
unless (-d $plugin)
{
  mkdir $plugin or die "Could not create directory ($plugin): $!";
}

open($file, '>', "$plugin/Plugin.xml") or die "Could not write Plugin.xml: $!";
print $file <<END;
<plugin name="$plugin" version="$version" description="Default Aleph One shaders">
  <mml file="Plugin.mml"/>
</plugin>
END
close $file;

open($file, '>', "$plugin/Plugin.mml") or die "Could not write Plugin.mml: $!";
print $file <<END;
<?xml version="1.0"?>
<marathon>
  <opengl>
END
for my $name (sort keys %shaders)
{
  print $file <<END;
    <shader name="$name" vert="$name.vert" frag="$name.frag"/>
END
}
print $file <<END;
  </opengl>
</marathon>
END
close $file;


## Create individual shader files

for my $name (keys %shaders)
{
  for my $type (keys %{ $shaders{$name} })
  {
    my $suffix = lc(substr($type, 0, 4));
    open($file, '>', "$plugin/$name.$suffix") or die "Could not write $name.$suffix: $!";
    print $file $shaders{$name}{$type};
    close $file;
  }
}
