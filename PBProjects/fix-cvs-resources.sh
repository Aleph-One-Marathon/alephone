#!/bin/sh
# fix-cvs-resources.sh - Unarchive resource files from cvs into usable forked files 
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
# To run this script, cd to
#

ROOT_DIR=../
SINGLE2FORKS="perl ${ROOT_DIR}tools/single2forks.pl"
APPLESINGLE_FILES='Cheats/Activate%20Cheats Extras/Marathon2_SOCK Extras/MarathonInf_1.2_SOCK Extras/MarathonInf_1.5_SOCK Extras/demo.resource Extras/demos/demos.resource MML%20Scripts/M2%20Map%20Script%20(test) MML%20Scripts/Marathon%202%20Display MML%20Scripts/Marathon%20oo%20Display MML%20Scripts/Text%20Strings Pfhortran_Release/Quartz%20copy Pfhortran_Release/Ten%20Forty%20Two Resources/MV%20Resources Resources/Old%20Stuff Resources/Pfhormz%20rsrc Resources/marathon2.resource Resources/m2_stripped.rsrc tools/MapChunker.rsrc'

found=0
for i in $APPLESINGLE_FILES;
do
	file=`echo $i | sed -e 's/%20/ /g'`;
	file=$ROOT_DIR$file;
	if test -e "$file"
	then
		found=`expr $found + 1`
		target_file=$file;
		if $SINGLE2FORKS "$file";
		then		
			mv "${file}.data" "$target_file"
			cp "${file}.rsrc" "$target_file"/..namedfork/rsrc
			rm "${file}.rsrc"
			echo $file expanded successfully.
		fi
	fi
done
if [ $found -eq 0 ]
then
	echo fix-cvs-resources.sh was unable to find any of the files it was supposed to fix.\
Please make sure that you are trying to run it from the aleph/PBProjects directory by cd\'ing to it first.
fi