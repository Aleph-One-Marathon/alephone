#!/bin/sh

# Turns resource-source files from the CVS archive into MacOS resource files
# Someone ought to create some MacOS 9 counterpart

REZ="/Developer/Tools/Rez -type rsrc -creator RSED"

$REZ CarbonResourceTweaks.r -o "Carbon Resource Tweaks"
$REZ M2Display.r -o "M2 Display"
$REZ MinfDisplay.r -o "Moo Display"
$REZ M2SOCK.r -o "Marathon2_SOCK"
$REZ SemiTpt.r -o "M2,oo Semitpt Shapes"
$REZ TextStrings.r -o "Text Strings"
