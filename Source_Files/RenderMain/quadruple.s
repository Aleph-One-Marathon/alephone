# we win

#	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
#	and the "Aleph One" developers.
# 
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	This license is contained in the file "GNU_GeneralPublicLicense.txt",
#	which is included with this source code; it is available online at
#	http://www.gnu.org/licenses/gpl.html

# need to ask eric how to schedule the 16 and 24 bit quadruples

	EXPORT .quadruple_screen			# export the code symbol

kEveryOtherLine:	EQU		0x8000
k16BitMode:			EQU		0x4000
k32BitMode:			EQU		0x2000
kVerticalOnlyMode:	EQU		0x1000

source:			EQU		0
dest:			EQU		4
width:			EQU		8
height:			EQU		10
bytes_per_row:	EQU		12
src_slop:		EQU		14
dst_slop:		EQU		16
flags:			EQU		18

rParamPtr:		EQU		r3
rSrcPtr:		EQU		r4
rDstPtr1:		EQU		r5
rDstPtr2:		EQU		r6
rHeight:		EQU		r7
rWidth:			EQU		r8
rDstSlop:		EQU		r10
rSrcSlop:		EQU		r11


rTemp1:			EQU		r9
rTemp2:			EQU		r12
rTemp3:			EQU		r3		;# HEY YOU, WEÕRE RE-USING THIS!

fpTemp1:		EQU		fp0
fpTemp2:		EQU		fp1

.quadruple_screen:
	lwz			rSrcPtr, source(rParamPtr)			
	subi		rSrcPtr, rSrcPtr, 4					; Adjust for update

	lwz			rDstPtr1, dest(rParamPtr)			
	subi		rDstPtr1, rDstPtr1, 8				; Adjust for update
	lhz			rDstPtr2, bytes_per_row(rParamPtr)
	add			rDstPtr2, rDstPtr2, rDstPtr1

	lhz			rHeight, height(rParamPtr)

	lhz			rDstSlop, dst_slop(rParamPtr)

	lhz			rSrcSlop, src_slop(rParamPtr)

	lhz			rTemp1, flags(rParamPtr)

	cmpi		0, 0, rTemp1, k16BitMode
	beq			@quad16bit
	cmpi		0, 0, rTemp1, k16BitMode|kVerticalOnlyMode
	beq			@duo

	cmpi		0, 0, rTemp1, k32BitMode
	beq			@quad32bit
	cmpi		0, 0, rTemp1, k16BitMode|kVerticalOnlyMode
	beq			@duo

	cmpi		0, 0, rTemp1, kVerticalOnlyMode
	beq			@duo
	
@quad8bit:
	lhz			rWidth, width(rParamPtr)
	subi		rWidth, rWidth, 1

@quad8bit_start:
	lwzu		rTemp1, 4(rSrcPtr)					;load src long
	mr			rTemp2, rTemp1						;save copy in rTemp2
	mr			rTemp3, rTemp1
	inslwi		rTemp2, rTemp1, 16, 8
	insrwi		rTemp3, rTemp1, 16, 8
	rlwimi		rTemp2, rTemp1, 16, 24, 31
	stw			rTemp2, -8(SP)						; store high half into redzone
	rlwimi		rTemp3, rTemp1, 16, 0, 7
	stw			rTemp3, -4(SP)						; store low half into redzone
	mtctr		rWidth
	lfd			fpTemp1, -8(SP)						; load double from redzone

@quad8bit_loop:
	lwzu		rTemp1, 4(rSrcPtr)					;load a long into r10
	stfdu		fpTemp1, 8(rDstPtr1)
	mr			rTemp2, rTemp1  					;put a copy in r0
	mr			rTemp3, rTemp1
	inslwi		rTemp2, rTemp1, 16, 8
	insrwi		rTemp3, rTemp1, 16, 8
	rlwimi		rTemp2, rTemp1, 16, 24, 31
	stw			rTemp2, -8(SP)						; store high half into redzone
	rlwimi		rTemp3, rTemp1, 16, 0, 7
	stw			rTemp3, -4(SP)						; store low half into redzone
	stfdu		fpTemp1, 8(rDstPtr2)
	lfd			fpTemp1, -8(SP)						; load double from redzone
	bdnz		@quad8bit_loop

	stfdu		fpTemp1, 8(rDstPtr1)
	subic.		rHeight, rHeight, 1					;we've done one scanline

	add			rSrcPtr, rSrcPtr, rSrcSlop			;add in "rowBytes"
	add			rDstPtr1, rDstPtr1, rDstSlop		;add in "rowBytes"

	stfdu		fpTemp1, 8(rDstPtr2)
	add			rDstPtr2, rDstPtr2, rDstSlop		;add in "rowBytes"
	bne			@quad8bit_start						;loop for all height

	blr												; outta here
		
@duo
	subi		rSrcPtr, rSrcPtr, 4					; Adjust for update again
	lhz			rWidth, width(rParamPtr)
	srwi		rWidth, rWidth, 2					; we're doing 16 bytes at a time

@duo_start
	mtctr		rWidth

@duo_loop
	lfd			fpTemp1, 8(rSrcPtr)
	lfdu		fpTemp2, 16(rSrcPtr)
	stfd		fpTemp1, 8(rDstPtr1)
	stfd		fpTemp1, 8(rDstPtr2)
	stfdu		fpTemp2, 16(rDstPtr1)
	stfdu		fpTemp2, 16(rDstPtr2)
	bdnz		@duo_loop

	subic.		rHeight, rHeight, 1

	add			rSrcPtr, rSrcPtr, rSrcSlop			;add in "rowBytes"
	add			rDstPtr1, rDstPtr1, rDstSlop		;add in "rowBytes"
	add			rDstPtr2, rDstPtr2, rDstSlop		;add in "rowBytes"

	bne			@duo_start							;loop for all height

	blr												; outta here

@quad16bit:
	lhz			rWidth, width(rParamPtr)
	subi		rWidth, rWidth, 1

@quad16bit_start:
	lwzu		rTemp1, 4(rSrcPtr)					;load src long
	mr			rTemp2, rTemp1						;save copy in rTemp2
	rlwimi		rTemp1, rTemp1, 16, 0, 15
	stw			rTemp1, -4(SP)						; store low half into redzone
	rlwimi		rTemp2, rTemp2, 16, 16, 31
	stw			rTemp2, -8(SP)						; store high half into redzone
	mtctr		rWidth
	lfd			fpTemp1, -8(SP)						; load double from redzone

@quad16bit_loop:
	lwzu		rTemp1, 4(rSrcPtr)					;load a long into r10
	stfdu		fpTemp1, 8(rDstPtr1)
	mr			rTemp2, rTemp1  					;put a copy in r0
	rlwimi		rTemp1, rTemp1, 16, 0, 15
	rlwimi		rTemp2, rTemp2, 16, 16, 31
	stw			rTemp1, -4(SP)						; store low half into redzone
	stw			rTemp2, -8(SP)						; store high half into redzone
	stfdu		fpTemp1, 8(rDstPtr2)
	lfd			fpTemp1, -8(SP)						; load double from redzone
	bdnz		@quad16bit_loop

	stfdu		fpTemp1, 8(rDstPtr1)
	subic.		rHeight, rHeight, 1					;we've done one scanline

	add			rSrcPtr, rSrcPtr, rSrcSlop			;add in "rowBytes"
	add			rDstPtr1, rDstPtr1, rDstSlop		;add in "rowBytes"

	stfdu		fpTemp1, 8(rDstPtr2)
	add			rDstPtr2, rDstPtr2, rDstSlop		;add in "rowBytes"
	bne			@quad16bit_start					;loop for all height

	blr
		
@quad32bit:
	lhz			rWidth, width(rParamPtr)
	subi		rWidth, rWidth, 1

@quad32bit_start:
	lwzu		rTemp1, 4(rSrcPtr)					;load src long
	stw			rTemp1, -4(SP)						; store low half into redzone
	stw			rTemp1, -8(SP)						; store high half into redzone
	mtctr		rWidth
	lfd			fpTemp1, -8(SP)						; load double from redzone

@quad32bit_loop:
	lwzu		rTemp1, 4(rSrcPtr)					;load a long into r10
	stfdu		fpTemp1, 8(rDstPtr1)
	stw			rTemp1, -4(SP)						; store low half into redzone
	stw			rTemp1, -8(SP)						; store high half into redzone
	stfdu		fpTemp1, 8(rDstPtr2)
	lfd			fpTemp1, -8(SP)						; load double from redzone
	bdnz		@quad32bit_loop

	stfdu		fpTemp1, 8(rDstPtr1)
	subic.		rHeight, rHeight, 1					;we've done one scanline

	add			rSrcPtr, rSrcPtr, rSrcSlop			;add in "rowBytes"
	add			rDstPtr1, rDstPtr1, rDstSlop		;add in "rowBytes"

	stfdu		fpTemp1, 8(rDstPtr2)
	add			rDstPtr2, rDstPtr2, rDstSlop		;add in "rowBytes"
	bne			@quad32bit_start					;loop for all height

	blr
		
@exit:
	blr
