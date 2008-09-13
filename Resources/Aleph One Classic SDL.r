data '26.A' (0, "Owner resource") {
	$"00"                                                 /* . */
};

data 'BNDL' (128) {
	$"3236 2E41 0000 0001 4652 4546 000C 0000"            /* 26.A....FREF.... */
	$"0080 0001 0081 0002 0082 0003 0083 0004"            /* .€......‚...ƒ.. */
	$"0084 0005 0085 0006 0086 0007 0087 0008"            /* .„...…...†...‡.. */
	$"0088 0009 0089 000A 008A 000B 008B 000C"            /* .ˆ.Æ.‰.Â.Š...‹.. */
	$"008C 4943 4E23 000C 0000 0080 0001 0081"            /* .ŒICN#.....€... */
	$"0002 0082 0003 0086 0004 0083 0005 0085"            /* ...‚...†...ƒ...… */
	$"0006 0087 0007 0084 0008 0088 0009 0089"            /* ...‡...„...ˆ.Æ.‰ */
	$"000A 008A 000B 008B 000C 008C"                      /* .Â.Š...‹...Œ */
};

data 'FREF' (128) {
	$"4150 504C 0000 00"                                  /* APPL... */
};

data 'FREF' (129) {
	$"7363 6541 0001 00"                                  /* sceA... */
};

data 'FREF' (130) {
	$"736E 6441 0002 00"                                  /* sndA... */
};

data 'FREF' (131) {
	$"6669 6C41 0003 00"                                  /* filA... */
};

data 'FREF' (132) {
	$"7368 7041 0004 00"                                  /* shpA... */
};

data 'FREF' (133) {
	$"7367 6141 0005 00"                                  /* sgaA... */
};

data 'FREF' (134) {
	$"7068 7941 0006 00"                                  /* phyA... */
};

data 'FREF' (135) {
	$"6D75 7341 0007 00"                                  /* musA... */
};

data 'FREF' (136) {
	$"696D 6741 0008 00"                                  /* imgA... */
};

data 'FREF' (137) {
	$"4D49 4449 0009 00"                                  /* MIDI.Æ. */
};

data 'FREF' (138) {
	$"4D4D 4C20 000A 00"                                  /* MML .Â. */
};

data 'FREF' (139) {
	$"5445 5854 000B 00"                                  /* TEXT... */
};

data 'FREF' (140) {
	$"3F3F 3F3F 000C 00"                                  /* ????... */
};

data 'FTyp' (128) {
	$"3236 2E41 7363 6541 7367 6141 6669 6C41"            /* 26.AsceAsgaAfilA */
	$"7068 7941 7368 7041 736E 6441 7061 7441"            /* phyAshpAsndApatA */
	$"696D 6741 7072 6566 6D75 7341"                      /* imgAprefmusA */
};

data 'ICN#' (128, "Item Icon") {
	$"000F F000 0078 1E00 01E3 E780 039F F9C0"            /* ..ð..x...ãç€.ŸùÀ */
	$"073F FCE0 0F7F FEF0 1EFF FF78 3CFF FF7C"            /* .?üà..þð.ÿÿx<ÿÿ| */
	$"3DFF FFBC 7DFF FFBE 7DFF FFBE 7DFF FFBE"            /* =ÿÿ¼}ÿÿ¾}ÿÿ¾}ÿÿ¾ */
	$"FDFF FFBF FDFF FFBF FCFF FF3F FEFF FF7F"            /* ýÿÿ¿ýÿÿ¿üÿÿ?þÿÿ. */
	$"FE7F FE7F FF3F FCFF FF9F F9FF 7FC7 E3FF"            /* þ.þ.ÿ?üÿÿŸùÿ.Çãÿ */
	$"7FE0 07FE 7FFE 3FFE 3FFE 7FFE 3FFE 7FFC"            /* .à.þ.þ?þ?þ.þ?þ.ü */
	$"1FFE 7FFC 1FFE 7FF8 0FFE 7FF0 07FE 7FE0"            /* .þ.ü.þ.ø.þ.ð.þ.à */
	$"03FE 7FC0 00FE 7F80 007E 7E00 000E 7000"            /* .þ.À.þ.€.~~...p. */
	$"000F F000 0078 1E00 01E3 E780 039F F9C0"            /* ..ð..x...ãç€.ŸùÀ */
	$"073F FCE0 0F7F FEF0 1EFF FF78 3CFF FF7C"            /* .?üà..þð.ÿÿx<ÿÿ| */
	$"3DFF FFBC 7DFF FFBE 7DFF FFBE 7DFF FFBE"            /* =ÿÿ¼}ÿÿ¾}ÿÿ¾}ÿÿ¾ */
	$"FDFF FFBF FDFF FFBF FCFF FF3F FEFF FF7F"            /* ýÿÿ¿ýÿÿ¿üÿÿ?þÿÿ. */
	$"FE7F FE7F FF3F FCFF FF9F F9FF 7FC7 E3FF"            /* þ.þ.ÿ?üÿÿŸùÿ.Çãÿ */
	$"7FE0 07FE 7FFE 3FFE 3FFE 7FFE 3FFE 7FFC"            /* .à.þ.þ?þ?þ.þ?þ.ü */
	$"1FFE 7FFC 1FFE 7FF8 0FFE 7FF0 07FE 7FE0"            /* .þ.ü.þ.ø.þ.ð.þ.à */
	$"03FE 7FC0 00FE 7F80 007E 7E00 000E 7000"            /* .þ.À.þ.€.~~...p. */
};

data 'ICN#' (129, "Map Icon") {
	$"01FF FFC0 0100 0060 07E0 0070 1BF8 0058"            /* .ÿÿÀ...`.à.p.ø.X */
	$"37EC 004C 69FE 1846 6DF6 187F 6FFF 003F"            /* 7ì.Liþ.Fmö..oÿ.? */
	$"EFFF 0001 EFF7 0001 F7EF 0001 F3DF 0001"            /* ïÿ..ï÷..÷ï..óß.. */
	$"7C3F 0001 7E7E 0001 3A76 0001 364C 0001"            /* |?..~~..:v..6L.. */
	$"0A70 0601 0260 0601 0100 0001 0100 0001"            /* Âp...`.......... */
	$"0100 0001 0103 0001 0103 0001 0100 0001"            /* ................ */
	$"0100 0001 0100 0001 0100 0001 0103 00C1"            /* ...............Á */
	$"0103 00C1 0100 0001 0100 0001 01FF FFFF"            /* ...Á.........ÿÿÿ */
	$"01FF FFC0 01FF FFE0 0FFF FFF0 1FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð.ÿÿø */
	$"3FFF FFFC 7FFF FFFE 7FFF FFFF FFFF FFFF"            /* ?ÿÿü.ÿÿþ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 7FFF FFFF 3FFF FFFF 1FFF FFFF"            /* ÿÿÿÿ.ÿÿÿ?ÿÿÿ.ÿÿÿ */
	$"0FFF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
};

data 'ICN#' (130, "Sounds Icon") {
	$"01FF FFC0 0100 0060 07E0 0070 1BF8 0058"            /* .ÿÿÀ...`.à.p.ø.X */
	$"37EC 004C 69FE 0046 6DF6 007F 6FFF 003F"            /* 7ì.Liþ.Fmö..oÿ.? */
	$"EFFF 0001 EFF7 0001 F7EF 0001 F3DF 0081"            /* ïÿ..ï÷..÷ï..óß. */
	$"7C3F 0081 7E7E 0081 3A76 3081 364C 3081"            /* |?.~~.:v06L0 */
	$"0A38 3081 0240 3081 0102 31C1 0106 35C1"            /* Â80.@0..1Á..5Á */
	$"0106 35D1 0135 5D71 0135 5D79 013D 5F69"            /* ..5Ñ.5]q.5]y.=_i */
	$"0159 5B69 0118 CB65 0100 CB61 0100 C361"            /* .Y[i..Ëe..Ëa..Ãa */
	$"0100 C301 0100 0001 0100 0001 01FF FFFF"            /* ..Ã..........ÿÿÿ */
	$"01FF FFC0 01FF FFE0 0FFF FFF0 1FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð.ÿÿø */
	$"3FFF FFFC 7FFF FFFE 7FFF FFFF FFFF FFFF"            /* ?ÿÿü.ÿÿþ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 7FFF FFFF 3FFF FFFF 1FFF FFFF"            /* ÿÿÿÿ.ÿÿÿ?ÿÿÿ.ÿÿÿ */
	$"0FFF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
};

data 'ICN#' (131, "Shapes Icon") {
	$"01FF FFC0 0100 0060 07E0 0070 1BF8 4058"            /* .ÿÿÀ...`.à.p.ø@X */
	$"35EC 604C 6DF6 7046 6BFE 607F 6FF7 303F"            /* 5ì`LmöpFkþ`.o÷0? */
	$"EFF7 1301 EFF7 0D01 FFEF 0F81 F3CF 0FC1"            /* ï÷..ï÷..ÿï.óÏ.Á */
	$"7C3F 3DE1 7E7E 3FF1 3A36 0FE1 364C 07E1"            /* |?=á~~?ñ:6.á6L.á */
	$"0A78 03E1 0260 03E1 0100 03C1 0100 07C1"            /* Âx.á.`.á...Á...Á */
	$"0100 1FE1 0100 3FE1 0100 3071 0100 6031"            /* ...á..?á..0q..`1 */
	$"0100 4031 0100 2011 0100 2001 0100 2001"            /* ..@1.. ... ... . */
	$"0100 0011 0100 0001 0100 0001 01FF FFFF"            /* .............ÿÿÿ */
	$"01FF FFC0 01FF FFE0 0FFF FFF0 1FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð.ÿÿø */
	$"3FFF FFFC 7FFF FFFE 7FFF FFFF FFFF FFFF"            /* ?ÿÿü.ÿÿþ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 7FFF FFFF 3FFF FFFF 1FFF FFFF"            /* ÿÿÿÿ.ÿÿÿ?ÿÿÿ.ÿÿÿ */
	$"0FFF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
};

data 'ICN#' (132, "Music Icon") {
	$"01FF FFC0 0100 0060 07E0 0070 1BF8 0058"            /* .ÿÿÀ...`.à.p.ø.X */
	$"37EC 004C 69FE 0046 6DF6 007F 6FFF 003F"            /* 7ì.Liþ.Fmö..oÿ.? */
	$"EFFF 0001 EFF7 0001 F7EF 0001 F3DF 0001"            /* ïÿ..ï÷..÷ï..óß.. */
	$"7C3F 0001 7E7E 0001 3A76 0801 364C 0801"            /* |?..~~..:v..6L.. */
	$"0A38 0821 0242 0821 0102 0821 0102 0821"            /* Â8.!.B.!...!...! */
	$"0102 0821 013B EFAD 0102 3821 0102 7821"            /* ...!.;ï­..8!..x! */
	$"015E FBF5 010E 31E1 011E 01E1 013F B6D5"            /* .^ûõ..1á...á.?¶Õ */
	$"010C 0001 0100 0001 0100 0001 01FF FFFF"            /* .............ÿÿÿ */
	$"01FF FFC0 01FF FFE0 0FFF FFF0 1FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð.ÿÿø */
	$"3FFF FFFC 7FFF FFFE 7FFF FFFF FFFF FFFF"            /* ?ÿÿü.ÿÿþ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 7FFF FFFF 3FFF FFFF 1FFF FFFF"            /* ÿÿÿÿ.ÿÿÿ?ÿÿÿ.ÿÿÿ */
	$"0FFF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
};

data 'ICN#' (133, "Saved Game Icon") {
	$"0FFF FE00 0800 0300 0800 0380 0800 02C0"            /* .ÿþ........€...À */
	$"0800 0260 0800 0230 0800 03F8 0800 01F8"            /* ...`...0...ø...ø */
	$"0800 0008 0807 E008 081B F808 0833 EC08"            /* ......à...ø..3ì. */
	$"0865 F608 086D F608 086F FF08 08EF F708"            /* .eö..mö..oÿ..ï÷. */
	$"08EF F708 08F7 EF08 08F3 DF08 087C 3F08"            /* .ï÷..÷ï..óß..|?. */
	$"087E 7E08 083E 6E08 0832 1C08 080E 7808"            /* .~~..>n..2....x. */
	$"0802 6008 0800 0008 0800 0008 0800 0008"            /* ..`............. */
	$"0800 0008 0800 0008 0800 0008 0FFF FFF8"            /* .............ÿÿø */
	$"0FFF FE00 0FFF FF00 0FFF FF80 0FFF FFC0"            /* .ÿþ..ÿÿ..ÿÿ€.ÿÿÀ */
	$"0FFF FFE0 0FFF FFF0 0FFF FFF8 0FFF FFF8"            /* .ÿÿà.ÿÿð.ÿÿø.ÿÿø */
	$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"            /* .ÿÿø.ÿÿø.ÿÿø.ÿÿø */
	$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"            /* .ÿÿø.ÿÿø.ÿÿø.ÿÿø */
	$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"            /* .ÿÿø.ÿÿø.ÿÿø.ÿÿø */
	$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"            /* .ÿÿø.ÿÿø.ÿÿø.ÿÿø */
	$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"            /* .ÿÿø.ÿÿø.ÿÿø.ÿÿø */
	$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"            /* .ÿÿø.ÿÿø.ÿÿø.ÿÿø */
};

data 'ICN#' (134, "Film Icon") {
	$"3FFF FF00 3C00 0180 2400 0140 2400 0120"            /* ?ÿÿ.<..€$..@$..  */
	$"3C00 0110 3C00 0108 2400 01FC 2400 0024"            /* <...<...$..ü$..$ */
	$"3C07 E03C 3C19 F83C 2437 EC24 2469 FE24"            /* <.à<<.ø<$7ì$$iþ$ */
	$"3C6D F63C 3C6F FF3C 24EF F724 24EF F724"            /* <mö<<oÿ<$ï÷$$ï÷$ */
	$"3CF7 EF3C 3CF3 DF3C 247C 3E24 247E 7E24"            /* <÷ï<<óß<$|>$$~~$ */
	$"3C3E 763C 3C32 2C3C 240E 7824 2402 6024"            /* <>v<<2,<$.x$$.`$ */
	$"3C00 003C 3C00 003C 2400 0024 2400 0024"            /* <..<<..<$..$$..$ */
	$"3C00 003C 3C00 003C 2400 0024 3FFF FFFC"            /* <..<<..<$..$?ÿÿü */
	$"3FFF FF00 3FFF FF80 3FFF FFC0 3FFF FFE0"            /* ?ÿÿ.?ÿÿ€?ÿÿÀ?ÿÿà */
	$"3FFF FFF0 3FFF FFF8 3FFF FFFC 3FFF FFFC"            /* ?ÿÿð?ÿÿø?ÿÿü?ÿÿü */
	$"3FFF FFFC 3FFF FFFC 3FFF FFFC 3FFF FFFC"            /* ?ÿÿü?ÿÿü?ÿÿü?ÿÿü */
	$"3FFF FFFC 3FFF FFFC 3FFF FFFC 3FFF FFFC"            /* ?ÿÿü?ÿÿü?ÿÿü?ÿÿü */
	$"3FFF FFFC 3FFF FFFC 3FFF FFFC 3FFF FFFC"            /* ?ÿÿü?ÿÿü?ÿÿü?ÿÿü */
	$"3FFF FFFC 3FFF FFFC 3FFF FFFC 3FFF FFFC"            /* ?ÿÿü?ÿÿü?ÿÿü?ÿÿü */
	$"3FFF FFFC 3FFF FFFC 3FFF FFFC 3FFF FFFC"            /* ?ÿÿü?ÿÿü?ÿÿü?ÿÿü */
	$"3FFF FFFC 3FFF FFFC 3FFF FFFC 3FFF FFFC"            /* ?ÿÿü?ÿÿü?ÿÿü?ÿÿü */
};

data 'ICN#' (135, "Physics") {
	$"7FFF FFF0 8000 0008 8000 0008 8000 0008"            /* .ÿÿð€...€...€... */
	$"8000 0008 8000 0008 8000 0008 8000 0008"            /* €...€...€...€... */
	$"A007 E008 D019 F80A 9037 EC0D 1069 F609"            /*  .à.Ð.øÂ7ì..iöÆ */
	$"106D FE01 106F FF01 10EF FF01 10EF F701"            /* .mþ..oÿ..ïÿ..ï÷. */
	$"10F7 EF01 10F3 DF01 107C 3F01 107E 7E01"            /* .÷ï..óß..|?..~~. */
	$"1036 6E09 902A 5C0D D00E 380A A002 6008"            /* .6nÆ*\.Ð.8Â .`. */
	$"8000 0008 8000 0008 8000 0008 8000 0008"            /* €...€...€...€... */
	$"8000 0008 8000 0008 8000 0008 7FFF FFF0"            /* €...€...€....ÿÿð */
	$"7FFF FFF0 FFFF FFF8 FFFF FFF8 FFFF FFF8"            /* .ÿÿðÿÿÿøÿÿÿøÿÿÿø */
	$"FFFF FFF8 FFFF FFF8 FFFF FFF8 FFFF FFF8"            /* ÿÿÿøÿÿÿøÿÿÿøÿÿÿø */
	$"FFFF FFF8 DFFF FFFA 9FFF FFFF 1FFF FFFF"            /* ÿÿÿøßÿÿúŸÿÿÿ.ÿÿÿ */
	$"1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"1FFF FFFF 9FFF FFFF DFFF FFFA FFFF FFF8"            /* .ÿÿÿŸÿÿÿßÿÿúÿÿÿø */
	$"FFFF FFF8 FFFF FFF8 FFFF FFF8 FFFF FFF8"            /* ÿÿÿøÿÿÿøÿÿÿøÿÿÿø */
	$"FFFF FFF8 FFFF FFF8 FFFF FFF8 7FFF FFF0"            /* ÿÿÿøÿÿÿøÿÿÿø.ÿÿð */
};

data 'ICN#' (136, "Images Icon") {
	$"01FF FFC0 01FF FFE0 07FF FFD0 19FF FFC8"            /* .ÿÿÀ.ÿÿà.ÿÿÐ.ÿÿÈ */
	$"35EF FFC4 6DF7 FFC2 6BFF FFFF 6FFF FFFF"            /* 5ïÿÄm÷ÿÂkÿÿÿoÿÿÿ */
	$"EFF7 FFFF EFF7 FFFF F7EF FFFF F3CF FFFF"            /* ï÷ÿÿï÷ÿÿ÷ïÿÿóÏÿÿ */
	$"7C3F FFFF 7E7F FFFF 3A77 FFFF 364F FFFF"            /* |?ÿÿ~.ÿÿ:wÿÿ6Oÿÿ */
	$"0E3F FFFF 02FF FFFF 01FF FFFF 01FF FFFF"            /* .?ÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFF7 01FF FFEF 01FF FFD7 01FF FFEF"            /* .ÿÿ÷.ÿÿï.ÿÿ×.ÿÿï */
	$"01FF FFB7 01FF FFDF 01FF FFFF 01FF FFFF"            /* .ÿÿ·.ÿÿß.ÿÿÿ.ÿÿÿ */
	$"01FF FFC0 01FF FFE0 0FFF FFF0 1FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð.ÿÿø */
	$"3FFF FFFC 7FFF FFFE 7FFF FFFF FFFF FFFF"            /* ?ÿÿü.ÿÿþ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 7FFF FFFF 3FFF FFFF 1FFF FFFF"            /* ÿÿÿÿ.ÿÿÿ?ÿÿÿ.ÿÿÿ */
	$"0FFF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
};

data 'ICN#' (137, "MIDI Music Icon") {
	$"01FF FFC0 0100 0060 07E0 0070 1BF8 0058"            /* .ÿÿÀ...`.à.p.ø.X */
	$"37EC 004C 69FE 0046 6DF6 007F 6FFF 003F"            /* 7ì.Liþ.Fmö..oÿ.? */
	$"EFFF 0001 EFF7 0001 F7EF 0001 F3DF 0001"            /* ïÿ..ï÷..÷ï..óß.. */
	$"7C3F 0001 7E7E 0001 3A76 0801 364C 0801"            /* |?..~~..:v..6L.. */
	$"0A38 0821 0242 0821 0102 0821 0102 0821"            /* Â8.!.B.!...!...! */
	$"0102 0821 013B EFAD 7FFF F821 7B7F F821"            /* ...!.;ï­.ÿø!{.ø! */
	$"6EA1 BFE9 64AE B1E1 4AAE B1E1 6AA1 BED5"            /* n¡¿éd®±áJ®±áj¡¾Õ */
	$"7FFF F001 7FFF F001 0100 0001 01FF FFFF"            /* .ÿð..ÿð......ÿÿÿ */
	$"01FF FFC0 01FF FFE0 0FFF FFF0 3FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð?ÿÿø */
	$"7FFF FFFC 7FFF FFFE FFFF FFFF FFFF FFFF"            /* .ÿÿü.ÿÿþÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 7FFF FFFF 7FFF FFFF"            /* ÿÿÿÿÿÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"3FFF FFFF 0FFF FFFF 01FF FFFF 01FF FFFF"            /* ?ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 7FFF FFFF 7FFF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"7FFF FFFF 7FFF FFFF 7FFF FFFF 7FFF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"7FFF FFFF 7FFF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
};

data 'ICN#' (138, "MML Script Icon") {
	$"01FF FFC0 0100 0060 07E0 0070 1BF8 3C58"            /* .ÿÿÀ...`.à.p.ø<X */
	$"37EC 424C 69FE 8146 6DF7 077F 6FFF 0F3F"            /* 7ìBLiþFm÷..oÿ.? */
	$"EFFF 1C01 EFF7 3E01 F7EF 7D01 F3DF F881"            /* ïÿ..ï÷>.÷ï}.óßø */
	$"7C3F E041 7E7F E021 366F 0011 3A1C 0011"            /* |?àA~.à!6o..:... */
	$"0E78 0009 0270 0009 0108 0009 0104 0009"            /* .x.Æ.p.Æ...Æ...Æ */
	$"0102 0011 0101 0021 0100 8061 0101 C0C1"            /* .......!..€a..ÀÁ */
	$"0103 C181 0107 2301 0106 2401 0104 2C01"            /* ..Á..#...$...,. */
	$"0102 3001 0101 E001 0100 0001 01FF FFFF"            /* ..0...à......ÿÿÿ */
	$"01FF FFC0 01FF FFE0 0FFF FFF0 3FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð?ÿÿø */
	$"7FFF FFFC 7FFF FFFE FFFF FFFF FFFF FFFF"            /* .ÿÿü.ÿÿþÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 7FFF FFFF 3FFF FFFF"            /* ÿÿÿÿÿÿÿÿ.ÿÿÿ?ÿÿÿ */
	$"3FFF FFFF 0FFF FFFF 01FF FFFF 01FF FFFF"            /* ?ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
};

data 'ICN#' (139, "Text File Icon") {
	$"01FF FFC0 0100 0060 07E0 0070 1BF8 0058"            /* .ÿÿÀ...`.à.p.ø.X */
	$"37EC 004C 69FE 0046 6DF6 007F 6FFF 003F"            /* 7ì.Liþ.Fmö..oÿ.? */
	$"EFFF 0001 EFF7 0001 F7EF DDB1 F3DF 0001"            /* ïÿ..ï÷..÷ïÝ±óß.. */
	$"7C3F 0001 7E7E EF71 3A76 0001 364C 0001"            /* |?..~~ïq:v..6L.. */
	$"0A3B FBB1 0260 0001 0100 0001 0100 0001"            /* Â;û±.`.......... */
	$"0100 0001 0100 0001 0100 0001 0100 0001"            /* ................ */
	$"0100 0001 0100 0001 0100 0001 0100 0001"            /* ................ */
	$"0100 0001 0100 0001 0100 0001 01FF FFFF"            /* .............ÿÿÿ */
	$"01FF FFC0 01FF FFE0 0FFF FFF0 3FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð?ÿÿø */
	$"7FFF FFFC 7FFF FFFE FFFF FFFF FFFF FFFF"            /* .ÿÿü.ÿÿþÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 7FFF FFFF 7FFF FFFF"            /* ÿÿÿÿÿÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"3FFF FFFF 0FFF FFFF 01FF FFFF 01FF FFFF"            /* ?ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
};

data 'ICN#' (140, "Generic File Icon") {
	$"01FF FFC0 0100 0060 07E0 0070 1BF8 0058"            /* .ÿÿÀ...`.à.p.ø.X */
	$"37EC 004C 69FE 0046 6DF6 007F 6FFF 003F"            /* 7ì.Liþ.Fmö..oÿ.? */
	$"EFFF 0001 EFF7 0001 F7EF 0001 F3DF 0001"            /* ïÿ..ï÷..÷ï..óß.. */
	$"7C3F 0001 7E7E 0001 3A76 0001 364C 0001"            /* |?..~~..:v..6L.. */
	$"0A38 0001 0260 0001 0100 0001 0100 0001"            /* Â8...`.......... */
	$"0100 0001 0100 0001 0100 0001 0100 0001"            /* ................ */
	$"0100 0001 0100 0001 0100 0001 0100 0001"            /* ................ */
	$"0100 0001 0100 0001 0100 0001 01FF FFFF"            /* .............ÿÿÿ */
	$"01FF FFC0 01FF FFE0 0FFF FFF0 3FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð?ÿÿø */
	$"7FFF FFFC 7FFF FFFE FFFF FFFF FFFF FFFF"            /* .ÿÿü.ÿÿþÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 7FFF FFFF 7FFF FFFF"            /* ÿÿÿÿÿÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"3FFF FFFF 0FFF FFFF 01FF FFFF 01FF FFFF"            /* ?ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
};

data 'ICN#' (141, "Folder Icon") {
	$"3C00 0000 2F00 0000 E3C0 0000 9CF0 0000"            /* <.../...ãÀ..œð.. */
	$"867D E000 818F 7800 8076 BE00 8038 4E00"            /* †}à.x.€v¾.€8N. */
	$"8007 0700 8001 C380 8000 F380 8000 1D80"            /* €...€.Ã€€.ó€€..€ */
	$"8000 0780 8000 0380 80FC 0380 837F 0780"            /* €..€€..€€ü.€ƒ..€ */
	$"86BF 8380 8DBF C780 8D7E C380 8DFF C780"            /* †¿ƒ€¿Ç€~Ã€ÿÇ€ */
	$"DDFE E3F0 FDFE E7FC 3EFD E3FF 1F7B E3FF"            /* Ýþãðýþçü>ýãÿ.{ãÿ */
	$"0F87 C7FF 0FCF C3FF 074E C7FF 06C1 83FE"            /* .‡Çÿ.ÏÃÿ.NÇÿ.Áƒþ */
	$"014F 83FC 004C F7F8 0000 3FF0 0000 0FE0"            /* .Oƒü.L÷ø..?ð...à */
	$"3C00 0000 3F00 0000 FFC0 0000 FFF0 0000"            /* <...?...ÿÀ..ÿð.. */
	$"FFFD E000 FFFF F800 FFFF FE00 FFFF FE00"            /* ÿýà.ÿÿø.ÿÿþ.ÿÿþ. */
	$"FFFF FF00 FFFF FF80 FFFF FF80 FFFF FF80"            /* ÿÿÿ.ÿÿÿ€ÿÿÿ€ÿÿÿ€ */
	$"FFFF FF80 FFFF FF80 FFFF FF80 FFFF FF80"            /* ÿÿÿ€ÿÿÿ€ÿÿÿ€ÿÿÿ€ */
	$"FFFF FF80 FFFF FF80 FFFF FF80 FFFF FF80"            /* ÿÿÿ€ÿÿÿ€ÿÿÿ€ÿÿÿ€ */
	$"FFFF FFF0 FFFF FFFC 3FFF FFFF 1FFF FFFF"            /* ÿÿÿðÿÿÿü?ÿÿÿ.ÿÿÿ */
	$"1FFF FFFF 1FFF FFFF 0FFF FFFF 07FF FFFE"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿþ */
	$"03FF FFFC 01FE FFF8 0000 3FF0 0000 0FE0"            /* .ÿÿü.þÿø..?ð...à */
};

data 'Mngl' (128) {
	$"0000 1FFE"                                          /* ...þ */
};

data 'icl4' (128, "Item Icon") {
	$"0000 0000 00CD EAFF FFAE DC00 0000 0000"            /* .....Íêÿÿ®Ü..... */
	$"0000 0000 CEFF EDCC CCDE FFEC 0000 0000"            /* ....ÎÿíÌÌÞÿì.... */
	$"0000 000E FFEC CDE9 9EDC CEFF E000 0000"            /* ....ÿìÍéžÜÎÿà... */
	$"0000 0CAF FDC8 8889 9999 ECDF FAC0 0000"            /* ...¯ýÈˆ‰™™ìßúÀ.. */
	$"0000 CFFF CC88 8888 8999 FFCD FFFC 0000"            /* ..ÏÿÌˆˆˆ‰™ÿÍÿü.. */
	$"000C FFFD C888 8888 8999 9FFC EFFF C000"            /* ..ÿýÈˆˆˆ‰™ŸüïÿÀ. */
	$"000E FFAC 8888 8888 8999 99FE CFFF A000"            /* ..ÿ¬ˆˆˆˆ‰™™þÏÿ . */
	$"00BF FFDC 8888 8888 8899 99FF CDFF FE00"            /* .¿ÿÜˆˆˆˆˆ™™ÿÍÿþ. */
	$"0CFF FFCB 8888 8888 8899 99FF ECFF FFC0"            /* .ÿÿËˆˆˆˆˆ™™ÿìÿÿÀ */
	$"0EFF FEC9 8888 8888 8999 99FF ACFF FFE0"            /* .ÿþÉˆˆˆˆ‰™™ÿ¬ÿÿà */
	$"0FFF FEC9 9888 8888 8999 9FFF F09F FFFC"            /* .ÿþÉ˜ˆˆˆ‰™ŸÿðŸÿü */
	$"CF99 FEC9 9888 8888 9999 9FFF F09F 9FFD"            /* Ï™þÉ˜ˆˆˆ™™ŸÿðŸŸý */
	$"D999 FEC9 9999 8899 9999 FFFF F099 99FE"            /* Ù™þÉ™™ˆ™™™ÿÿð™™þ */
	$"E999 9FCE 9999 9999 9999 FFFF EC99 99FE"            /* é™ŸÎ™™™™™™ÿÿì™™þ */
	$"9999 9FCD F999 9999 999F FFFF CD99 999F"            /* ™™ŸÍù™™™™ŸÿÿÍ™™Ÿ */
	$"9999 99E0 9999 9999 9FFF FFFA 0E99 999F"            /* ™™™à™™™™Ÿÿÿú.™™Ÿ */
	$"9999 999C CFF9 999F FFFF FFFC C999 999F"            /* ™™™œÏù™ŸÿÿÿüÉ™™Ÿ */
	$"9999 9999 0DFF FFFF FFFF FFDC 9999 999F"            /* ™™™™.ÿÿÿÿÿÿÜ™™™Ÿ */
	$"E999 9999 E0CF FFFF FFFF AC08 9999 999E"            /* é™™™àÏÿÿÿÿ¬.™™™ž */
	$"D999 9999 99CC DEFF FFED 0C89 9999 99FE"            /* Ù™™™™ÌÞÿÿí.‰™™™þ */
	$"C988 8888 899B C0CC CC0C B998 8888 89FD"            /* Éˆˆˆ‰›ÀÌÌ.¹˜ˆˆ‰ý */
	$"0988 8888 8888 89E0 0B88 8888 8888 89FC"            /* Æˆˆˆˆˆ‰à.ˆˆˆˆˆ‰ü */
	$"0B88 8888 8888 89FC C888 8888 8888 9FE0"            /* .ˆˆˆˆˆ‰üÈˆˆˆˆˆŸà */
	$"0C88 8888 8888 899C C888 8888 8888 9FC0"            /* .ˆˆˆˆˆ‰œÈˆˆˆˆˆŸÀ */
	$"00D8 8888 8888 889C C888 8888 8889 FE00"            /* .ØˆˆˆˆˆœÈˆˆˆˆ‰þ. */
	$"000E 8888 8888 889C C888 8888 8889 E000"            /* ..ˆˆˆˆˆœÈˆˆˆˆ‰à. */
	$"000C 8888 8888 889C C888 8888 889F C000"            /* ..ˆˆˆˆˆœÈˆˆˆˆŸÀ. */
	$"0000 C988 8888 889C 0888 8888 89FC 0000"            /* ..Éˆˆˆˆœ.ˆˆˆ‰ü.. */
	$"0000 0CE8 8888 889C 0888 8889 FEC0 0000"            /* ...èˆˆˆœ.ˆˆ‰þÀ.. */
	$"0000 000D 9888 889C 0888 899F E000 0000"            /* ....˜ˆˆœ.ˆ‰Ÿà... */
	$"0000 0000 CD99 999C C899 FFEC 0000 0000"            /* ....Í™™œÈ™ÿì.... */
	$"0000 0000 00CD DEF0 C9EE DC00 0000 0000"            /* .....ÍÞðÉîÜ..... */
};

data 'icl4' (129, "Map Icon") {
	$"0000 000E DEEE EEEE EEEE EEEE EE00 0000"            /* ....Þîîîîîîîî... */
	$"0000 000D 0000 0000 0000 0000 0EE0 0000"            /* .............à.. */
	$"0000 CDDD DDDC 0000 0000 7000 0EEE 0000"            /* ..ÍÝÝÜ....p..î.. */
	$"000E EDB8 8BDE E000 0000 700C 0ECE E000"            /* ..í¸‹Þà...p..Îà. */
	$"00EF C888 99AD FE00 0000 70C0 CE0C EE00"            /* .ïÈˆ™­þ...pÀÎ.î. */
	$"0DFD 8888 899A DFE0 0003 3077 0E0C CEE0"            /* .ýˆˆ‰šßà..0w..Îà */
	$"CFAD 8888 899F D9FC 0003 3700 0EEE EEEA"            /* Ï­ˆˆ‰ŸÙü..7..îîê */
	$"DFED 9888 99AF DEFE 0000 700C 0DDD DDDE"            /* ßí˜ˆ™¯Þþ..p..ÝÝÞ */
	$"E9ED 9989 9A9F D99E 0000 7000 C0DD DDDA"            /* éí™‰šŸÙž..p.ÀÝÝÚ */
	$"999D 9999 A9FA D99A 0000 700C 0CC0 CCCA"            /* ™™™©úÙš..p..ÀÌÊ */
	$"999E DFFF FFFD 8999 0000 7000 C0CC 0CDA"            /* ™žßÿÿý‰™..p.ÀÌ.Ú */
	$"8999 DDEA FECB 999E 0000 070C 00C0 C0CA"            /* ‰™ÝêþË™ž.....ÀÀÊ */
	$"D888 98DC CD89 889D 0000 0700 CC0C CCCA"            /* Øˆ˜ÜÍ‰ˆ....Ì.ÌÊ */
	$"C988 889D D988 889C 0000 070C 00CC 0CDA"            /* ÉˆˆÙˆˆœ.....Ì.Ú */
	$"0D88 889D D888 89D0 0000 0700 CC0C CCDA"            /* .ˆˆØˆ‰Ð....Ì.ÌÚ */
	$"00B8 888D C888 9B00 0000 007C 00CC CCCF"            /* .¸ˆÈˆ›....|.ÌÌÏ */
	$"000D 888D D88A D000 0000 0330 C0C0 CCD6"            /* ..ˆØŠÐ....0ÀÀÌÖ */
	$"0000 CDED DEDC 0000 0000 0337 7777 70DA"            /* ..ÍíÞÜ.....7wwpÚ */
	$"0000 000E 0000 0000 0000 7770 0CCC CCDF"            /* ..........wp.ÌÌß */
	$"0000 000E 0070 0000 0077 0070 C0C0 CCCF"            /* .....p...w.pÀÀÌÏ */
	$"0000 000E 0007 7000 0770 0007 0C0C CCDF"            /* ......p..p....Ìß */
	$"0000 000E 0000 7733 7000 0007 C0CC C0DF"            /* ......w3p...ÀÌÀß */
	$"0000 000E 0000 0033 0000 00C7 0C0C CCDF"            /* .......3...Ç..Ìß */
	$"0000 000E 0000 0007 0000 0000 70C0 CCCF"            /* ............pÀÌÏ */
	$"0000 000E 0000 0007 0000 00C0 7CCC CCDF"            /* ...........À|ÌÌß */
	$"0000 000A 0000 0007 0000 000C 77C0 CCCF"            /* ...Â........wÀÌÏ */
	$"0000 000E 0000 0007 0000 0000 C70C CCDF"            /* ............Ç.Ìß */
	$"0000 000A 0000 0033 0000 0000 33CC CCCF"            /* ...Â...3....3ÌÌÏ */
	$"0000 000A 0000 0733 7777 7777 3377 7CDF"            /* ...Â...3wwww3w|ß */
	$"0000 000A 0007 7007 0000 000C 07C0 C0DF"            /* ...Â..p......ÀÀß */
	$"0000 000A 0000 0000 0000 0000 C0CC CCDF"            /* ...Â........ÀÌÌß */
	$"0000 000A AAFF FFFF FFFF FFFF FFFF FFFF"            /* ...Âªÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl4' (130, "Sounds Icon") {
	$"0000 000E DEEE EEEE EEEE EEEE EE00 0000"            /* ....Þîîîîîîîî... */
	$"0000 000D 0000 0000 0000 0000 0EE0 0000"            /* .............à.. */
	$"0000 CDDD DDDC 0000 0000 0000 0EEE 0000"            /* ..ÍÝÝÜ.......î.. */
	$"000E EDB8 8BDE E000 0000 000C 0ECE E000"            /* ..í¸‹Þà......Îà. */
	$"00EF C888 99AD FE00 0000 00C0 CE0C EE00"            /* .ïÈˆ™­þ....ÀÎ.î. */
	$"0DFD 8888 899A DFE0 0000 0000 0E0C CEE0"            /* .ýˆˆ‰šßà......Îà */
	$"CFAD 8888 899F D9FC 0000 000C 0EEE EEEA"            /* Ï­ˆˆ‰ŸÙü.....îîê */
	$"DFED 9888 99AF DEFE 0000 00C0 0DDD DDDE"            /* ßí˜ˆ™¯Þþ...À.ÝÝÞ */
	$"E9ED 9989 9A9F D99E 0000 000C 00DD DDDA"            /* éí™‰šŸÙž.....ÝÝÚ */
	$"999D 9999 A9FA D99A 0000 0000 0CC0 CCCA"            /* ™™™©úÙš.....ÀÌÊ */
	$"999E DFFF FFFD 8999 0000 00C0 C0CC 0CDA"            /* ™žßÿÿý‰™...ÀÀÌ.Ú */
	$"8999 DDEA FECB 9999 0000 0000 EC0C C0DA"            /* ‰™ÝêþË™™....ì.ÀÚ */
	$"D888 98DC CD89 889D 0000 0000 F0CC CCDA"            /* Øˆ˜ÜÍ‰ˆ....ðÌÌÚ */
	$"C988 889D D988 889C 00CC 0000 FC0C 0CDA"            /* ÉˆˆÙˆˆœ.Ì..ü..Ú */
	$"0D88 889D D888 89D0 00AE 000C FCC0 CCDA"            /* .ˆˆØˆ‰Ð.®..üÀÌÚ */
	$"00B8 888D C888 9E00 00AE 000C FC0C CCCF"            /* .¸ˆÈˆž..®..ü.ÌÏ */
	$"000D 888D D899 D000 00FE 000C FCC0 CCD6"            /* ..ˆØ™Ð..þ..üÀÌÖ */
	$"0000 CDED DEDC 0000 00FE 000D ED0C C0DA"            /* ..ÍíÞÜ...þ..í.ÀÚ */
	$"0000 000E 0000 0DE0 0CEA 000D ED0C CCDF"            /* .......à.ê..í.Ìß */
	$"0000 000E 0000 0DFC 0CEF CF0D EDC0 CCCF"            /* .......ü.ïÏ.íÀÌÏ */
	$"0000 000E 0000 0AED 0DDF CF0E DEDA 0CDF"            /* ......Âí.ßÏ.ÞÚ.ß */
	$"0000 000E 00DF 0FDD 0DDE DACE DEDF CCDF"            /* .....ß.Ý.ÞÚÎÞßÌß */
	$"0000 000E 00FF CACA 0ECE EEDE CFED E0DF"            /* .....ÿÊÊ.ÎîÞÏíàß */
	$"0000 000E 0CEE DE0F CECE EDDF 0FAC FCCF"            /* .....îÞ.ÎÎíß.¬üÏ */
	$"0000 000E 0DDE ED0E CF0D FCEF 0FF0 DDDF"            /* .....Þí.Ï.üï.ðÝß */
	$"0000 000A 000D AC0D DF0D FCFA 0EFC CEDF"            /* ...Â..¬.ß.üú.üÎß */
	$"0000 000E 0000 DC0C AE0D F0AE 0AE0 CCDF"            /* ......Ü.®.ð®ÂàÌß */
	$"0000 000A 0000 0000 FE00 00EA 0EEC CCCF"            /* ...Â....þ..ê.ìÌÏ */
	$"0000 000A 0000 0000 AD00 00EE 0CCC C0DF"            /* ...Â....­..î.ÌÀß */
	$"0000 000A 0000 0000 C000 00CC C0C0 CCDF"            /* ...Â....À..ÌÀÀÌß */
	$"0000 000A 0000 0000 0000 0000 0CCC 0CDF"            /* ...Â.........Ì.ß */
	$"0000 000A AFAF FFFF FFFF FFFF FFFF FFFF"            /* ...Â¯¯ÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl4' (131, "Shapes Icon") {
	$"0000 000E DEEE EEEE EEEE EEEE EE00 0000"            /* ....Þîîîîîîîî... */
	$"0000 000D 0000 0000 0000 0000 0EE0 0000"            /* .............à.. */
	$"0000 CDDD DDDC 0000 CC00 0000 0EEE 0000"            /* ..ÍÝÝÜ..Ì....î.. */
	$"000E EDB8 EBDA E000 D9C0 000C 0ECE E000"            /* ..í¸ëÚà.ÙÀ...Îà. */
	$"00EF C888 8F9D FE00 CEEC 00C0 CE0C EE00"            /* .ïÈˆþ.Îì.ÀÎ.î. */
	$"0DFD 8888 89F9 DFD0 C99D 0000 0E0C CEE0"            /* .ýˆˆ‰ùßÐÉ....Îà */
	$"CFAD 8888 899A DAFC CEFD 0C0C 0EEE EEEA"            /* Ï­ˆˆ‰šÚüÎý...îîê */
	$"DFED 9888 99AF DEFD 009A 00CC CDDD DDDE"            /* ßí˜ˆ™¯Þý.š.ÌÍÝÝÞ */
	$"E9ED 9989 9A9F D99E 00CE C0DA D0DD DDDA"            /* éí™‰šŸÙž.ÎÀÚÐÝÝÚ */
	$"99AD 99A9 A9FA D99A 000C FDDF DC0C 0CCA"            /* ™­™©©úÙš..ýßÜ..Ê */
	$"999B DFFF FFFD B99A 000C EEFF DCC0 CCDA"            /* ™›ßÿÿý¹š..îÿÜÀÌÚ */
	$"8899 DDEA FECD 999E 00CD FEEE EDCC C0CA"            /* ˆ™ÝêþÍ™ž.ÍþîíÌÀÊ */
	$"D988 98DC CD89 889D 0C9F FADB EF9C CCCA"            /* Ùˆ˜ÜÍ‰ˆ.ŸúÛïœÌÊ */
	$"C888 889D D988 88FC 0CFF 9FAE E9FE CCDA"            /* ÈˆˆÙˆˆü.ÿŸ®éþÌÚ */
	$"0D88 889D D888 89D0 00DD D9AA 99AD C0DF"            /* .ˆˆØˆ‰Ð.ÝÙª™­Àß */
	$"00B8 888D C888 9E00 0000 0DAF F9BC CCDA"            /* .¸ˆÈˆž....¯ù¼ÌÚ */
	$"000D 888D D889 D000 0000 0CFF F99C CCD6"            /* ..ˆØ‰Ð....ÿùœÌÖ */
	$"0000 CDED DEDC 0000 0000 0CAE AFEC CCCA"            /* ..ÍíÞÜ.....®¯ìÌÊ */
	$"0000 000E 0000 0000 0000 0CF9 FD0C 0CDF"            /* ...........ùý..ß */
	$"0000 000E 0000 0000 000C CDFD 9AC0 CCDF"            /* ..........ÍýšÀÌß */
	$"0000 000E 0000 0000 00CE BFF9 EFDC 0CDF"            /* .........Î¿ùïÜ.ß */
	$"0000 000E 0000 0000 0CDE EDEE EAFC CCDF"            /* .........ÞíîêüÌß */
	$"0000 000E 0000 0000 0DAD C0DD 0DFE 0CDF"            /* .........­ÀÝ.þ.ß */
	$"0000 000E 0000 0000 CEED 0000 0CEB CCCF"            /* ........Îí...ëÌÏ */
	$"0000 000E 0000 0000 CECC C0C0 CCEA CCDF"            /* ........ÎÌÀÀÌêÌß */
	$"0000 000A 0000 0000 0DB0 000C 00DD CCCF"            /* ...Â.....°...ÝÌÏ */
	$"0000 000E 0000 0000 00DC 0000 C0DD 0CDF"            /* .........Ü..ÀÝ.ß */
	$"0000 000A 0000 0000 CCDD 000C 0CCD CCDF"            /* ...Â....ÌÝ...ÍÌß */
	$"0000 000A 0000 0000 DCC0 0000 C0DB CCDF"            /* ...Â....ÜÀ..ÀÛÌß */
	$"0000 000A 0000 0000 0000 00C0 0CCD DCCF"            /* ...Â.......À.ÍÜÏ */
	$"0000 000A 0000 0000 0000 000C 00C0 CCDF"            /* ...Â.........ÀÌß */
	$"0000 000A AAFF FFFF FFFF FFFF FFFF FFFF"            /* ...Âªÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl4' (132, "Music Icon") {
	$"0000 000E DEEE EEEE EEEE EEEE EE00 0000"            /* ....Þîîîîîîîî... */
	$"0000 000D 0000 0000 0000 0000 0EE0 0000"            /* .............à.. */
	$"0000 CDDD DDDC 0000 0000 0000 0EEE 0000"            /* ..ÍÝÝÜ.......î.. */
	$"000E EDB8 8BDE E000 0000 000C 0ECE E000"            /* ..í¸‹Þà......Îà. */
	$"00EF C888 99AD FE00 0000 00C0 CE0C EE00"            /* .ïÈˆ™­þ....ÀÎ.î. */
	$"0DFD 8888 899A DFE0 0000 0000 0E0C CEE0"            /* .ýˆˆ‰šßà......Îà */
	$"CFAD 8888 899F D9FC 0000 000C 0EEE EEEA"            /* Ï­ˆˆ‰ŸÙü.....îîê */
	$"DFED 9888 99AF DEFE 0000 00C0 0DDD DDDE"            /* ßí˜ˆ™¯Þþ...À.ÝÝÞ */
	$"E9ED 9989 9A9F D99E 0000 000C 00DD DDDA"            /* éí™‰šŸÙž.....ÝÝÚ */
	$"999D 9999 A9FA D99A 0000 0000 0CC0 CCCA"            /* ™™™©úÙš.....ÀÌÊ */
	$"999E DFFF FFFD 8999 0000 00C0 C0CC 0CDA"            /* ™žßÿÿý‰™...ÀÀÌ.Ú */
	$"8999 DDEA FECB 9999 0000 000C 0CC0 C0CA"            /* ‰™ÝêþË™™.....ÀÀÊ */
	$"D888 98DC CD89 889D 0000 0000 C0CC CCDA"            /* Øˆ˜ÜÍ‰ˆ....ÀÌÌÚ */
	$"C988 889D D988 889C 0000 000C 00C0 CCDA"            /* ÉˆˆÙˆˆœ.....ÀÌÚ */
	$"0D88 889D D888 89D0 0000 F000 CC0C CCDA"            /* .ˆˆØˆ‰Ð..ð.Ì.ÌÚ */
	$"00B8 888D C888 9E00 0000 F00C 00CC 0CCF"            /* .¸ˆÈˆž...ð..Ì.Ï */
	$"000D 889D D899 D000 0000 F000 CCF0 CCD6"            /* ..ˆØ™Ð...ð.ÌðÌÖ */
	$"0000 CDED DEDC 00F0 0000 F00C 00FC C0DA"            /* ..ÍíÞÜ.ð..ð..üÀÚ */
	$"0000 000E 0000 00F0 0000 F000 CCFC CCDF"            /* .......ð..ð.ÌüÌß */
	$"0000 000E 0000 00F0 0000 F00C 00F0 CCCF"            /* .......ð..ð..ðÌÏ */
	$"0000 000E 0000 00F0 0000 F000 CCFC 0CDF"            /* .......ð..ð.Ìü.ß */
	$"0000 000E 0DDD DDFD DDDD FDDD DDFD DDDF"            /* .....ÝÝýÝÝýÝÝýÝß */
	$"0000 000E 0000 00F0 00FF F000 0CF0 C0DF"            /* .......ð.ÿð..ðÀß */
	$"0000 000E 0000 00F0 0FFF FC00 C0FC CCDF"            /* .......ð.ÿü.ÀüÌß */
	$"0000 000E 0DDD DDFD DFFF FDDD FFFD DDCF"            /* .....ÝÝýßÿýÝÿýÝÏ */
	$"0000 000A 0000 FFF0 00FF 000F FFF0 C0DF"            /* ...Â..ÿð.ÿ..ÿðÀß */
	$"0000 000E 000F FFF0 0000 000F FFFC CCDF"            /* ......ÿð....ÿüÌß */
	$"0000 000A 0DDF FFFD DDDD DDDD FFDD DDCF"            /* ...Â.ßÿýÝÝÝÝÿÝÝÏ */
	$"0000 000A 0000 FF00 0000 0000 0C0C C0DF"            /* ...Â..ÿ.......Àß */
	$"0000 000A 0000 0000 0000 0000 C0C0 CCDF"            /* ...Â........ÀÀÌß */
	$"0000 000A 0000 0000 0000 00C0 0CCC 0CDF"            /* ...Â.......À.Ì.ß */
	$"0000 000A AAFF FFFF FFFF FFFF FFFF FFFF"            /* ...Âªÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl4' (133, "Saved Game Icon") {
	$"0000 DEEE EEEE EEEE EEEE EEE0 0000 0000"            /* ..Þîîîîîîîîà.... */
	$"0000 E000 0000 0000 0000 00EE 0000 0000"            /* ..à........î.... */
	$"0000 D000 0000 0C00 0000 00EE E000 0000"            /* ..Ð........îà... */
	$"0000 E000 0000 0C00 00C0 C0EC EE00 0000"            /* ..à......ÀÀìî... */
	$"0000 E000 CCCC CCCC CCCC 0CE0 CEE0 0000"            /* ..à.ÌÌÌÌÌÌ.àÎà.. */
	$"0000 E000 C000 0C00 00C0 00E0 CCEE 0000"            /* ..à.À....À.àÌî.. */
	$"0000 E000 0000 0C00 00C0 C0EE EEEE A000"            /* ..à......ÀÀîîî . */
	$"0000 E000 0000 0C00 00C0 00DD DDDD E000"            /* ..à......À.ÝÝÝà. */
	$"0000 E000 C000 0000 0000 C000 0DDD A000"            /* ..à.À.....À..Ý . */
	$"0000 E00C C000 CDDD DDDC 0CC0 CCCC A000"            /* ..à.À.ÍÝÝÜ.ÀÌÌ . */
	$"0000 E000 000E ADB8 8BDA E00C 0CCD A000"            /* ..à...­¸‹Úà..Í . */
	$"0000 E000 00EF D888 999D FECC 0CCC A000"            /* ..à..ïØˆ™þÌ.Ì . */
	$"0000 E000 0EFD 8888 89AA DFD0 CCCD A000"            /* ..à..ýˆˆ‰ªßÐÌÍ . */
	$"0000 E000 CFAD 8888 899F DAFC 0CCD A000"            /* ..à.Ï­ˆˆ‰ŸÚü.Í . */
	$"0000 E000 DFED 9888 99AF DEFD C0CD A000"            /* ..à.ßí˜ˆ™¯ÞýÀÍ . */
	$"0000 E000 E9ED 9899 99FF D9FE CCCC F000"            /* ..à.éí˜™™ÿÙþÌÌð. */
	$"0000 E000 999D A999 9FFA D99A 0CCD F000"            /* ..à.™©™ŸúÙš.Íð. */
	$"0000 E000 E998 DFF9 FFFD 8999 0CCC F000"            /* ..à.é˜ßùÿý‰™.Ìð. */
	$"0000 E000 8999 DDEA FEDB 999E 0CCD F000"            /* ..à.‰™ÝêþÛ™ž.Íð. */
	$"0000 E000 D988 9BDC CDB9 889D CCCC F000"            /* ..à.Ùˆ›ÜÍ¹ˆÌÌð. */
	$"0000 E000 C888 889D C988 88AC CCCD F000"            /* ..à.ÈˆˆÉˆˆ¬ÌÍð. */
	$"0000 E000 0D88 888D D888 89D0 0C0D F000"            /* ..à..ˆˆØˆ‰Ð..ð. */
	$"0000 E000 00B8 888D D888 9E00 CCCD F000"            /* ..à..¸ˆØˆž.ÌÍð. */
	$"0000 E000 000D 888D D889 D00C 0CCC F000"            /* ..à...ˆØ‰Ð..Ìð. */
	$"0000 E000 0000 CDED DEDC 0C0C 0CCD F000"            /* ..à...ÍíÞÜ...Íð. */
	$"0000 E000 0000 0000 0000 C0C0 0CCC F000"            /* ..à.......ÀÀ.Ìð. */
	$"0000 E000 C000 0C00 00C0 0C0C 0CCD F000"            /* ..à.À....À...Íð. */
	$"0000 E00C C000 CCCC CCC0 00CC 0CCC F000"            /* ..à.À.ÌÌÌÀ.Ì.Ìð. */
	$"0000 E000 C000 0C00 00C0 C00C C0CD F000"            /* ..à.À....ÀÀ.ÀÍð. */
	$"0000 E000 0000 0C00 00CC 00C0 CCCD F000"            /* ..à......Ì.ÀÌÍð. */
	$"0000 E000 0000 0000 0000 0C0C 0CCD F000"            /* ..à..........Íð. */
	$"0000 EEAA AAAA AFFF AFFF FFFF FFFF F000"            /* ..îªªª¯ÿ¯ÿÿÿÿÿð. */
};

data 'icl4' (134, "Film Icon") {
	$"00FF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* .ÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"00FF FF0C 000C 000C 000C 000F F000 0000"            /* .ÿÿ.........ð... */
	$"00F0 0F00 CC00 CC00 CC00 CC0F DF00 0000"            /* .ð..Ì.Ì.Ì.Ì.ß... */
	$"00F0 0F0C 0C0C 0C0C 0C0C 0C0F CDF0 0000"            /* .ð..........Íð.. */
	$"00FF FFC0 C0C0 C0C0 C0C0 C0CF 0CDF 0000"            /* .ÿÿÀÀÀÀÀÀÀÀÏ.ß.. */
	$"00FF FF0C 0C0C 0C0C 0C0C 0C0F C0CD F000"            /* .ÿÿ.........ÀÍð. */
	$"00F0 0F00 C0C0 C0C0 C0C0 C0CF FFFF FF00"            /* .ð..ÀÀÀÀÀÀÀÏÿÿÿ. */
	$"00F0 0F0C 0C0C 000C 0C0C 0C00 00F0 0F00"            /* .ð...........ð.. */
	$"00FF FFC0 C0C0 CDDD DDDC 00CC 0CFF FF00"            /* .ÿÿÀÀÀÍÝÝÜ.Ì.ÿÿ. */
	$"00FF FF0C 0C0E EDD8 8BDE E0C0 C0FF FF00"            /* .ÿÿ...íØ‹ÞàÀÀÿÿ. */
	$"00F0 0F00 C0EA D888 999D FE0C 0CF0 0F00"            /* .ð..ÀêØˆ™þ..ð.. */
	$"00F0 0F0C 0DFD 8888 899A DFE0 C0F0 0F00"            /* .ð...ýˆˆ‰šßàÀð.. */
	$"00FF FFC0 CFAD 8888 899F DAFC 0CFF FF00"            /* .ÿÿÀÏ­ˆˆ‰ŸÚü.ÿÿ. */
	$"00FF FF00 DFED 9888 99AF DEFD C0FF FF00"            /* .ÿÿ.ßí˜ˆ™¯ÞýÀÿÿ. */
	$"00F0 0FC0 EFED 9999 999F D99E 0CF0 0F00"            /* .ð.Àïí™™™ŸÙž.ð.. */
	$"00F0 0F00 999D A999 99FF D99A C0F0 0F00"            /* .ð..™©™™ÿÙšÀð.. */
	$"00FF FF0C 999B DFFF AFFD 899E 0CFF FF00"            /* .ÿÿ.™›ßÿ¯ý‰ž.ÿÿ. */
	$"00FF FFCC B989 DDEA FECB 999E C0FF FF00"            /* .ÿÿÌ¹‰ÝêþË™žÀÿÿ. */
	$"00F0 0F00 D898 88DC CD88 989D 0CF0 0F00"            /* .ð..Ø˜ˆÜÍˆ˜.ð.. */
	$"00F0 0F0C C988 889D D988 88AC C0F0 0F00"            /* .ð..ÉˆˆÙˆˆ¬Àð.. */
	$"00FF FF0C 0D88 888D D888 89D0 0CFF FF00"            /* .ÿÿ..ˆˆØˆ‰Ð.ÿÿ. */
	$"00FF FFC0 C0B8 888D C888 9E00 C0FF FF00"            /* .ÿÿÀÀ¸ˆÈˆž.Àÿÿ. */
	$"00F0 0F0C 0C0D 888D D889 D0C0 C0F0 0F00"            /* .ð....ˆØ‰ÐÀÀð.. */
	$"00F0 0FC0 C0C0 CDED DEDC CC0C 00F0 0F00"            /* .ð.ÀÀÀÍíÞÜÌ..ð.. */
	$"00FF FF0C 0C0C C000 0C00 0C0C C0FF FF00"            /* .ÿÿ...À.....Àÿÿ. */
	$"00FF FFC0 C00C 00C0 C00C 0C0C 00FF FF00"            /* .ÿÿÀÀ..ÀÀ....ÿÿ. */
	$"00F0 0F0C 0C0C 0C0C 0C0C 00C0 CCF0 0F00"            /* .ð.........ÀÌð.. */
	$"00F0 0F00 C0C0 C0C0 C0C0 CC0C 00F0 0F00"            /* .ð..ÀÀÀÀÀÀÌ..ð.. */
	$"00FF FFC0 0C0C 0C0C 0C0C 00C0 C0FF FF00"            /* .ÿÿÀ.......ÀÀÿÿ. */
	$"00FF FF00 C0C0 C0C0 C0C0 0C0C 0CFF FF00"            /* .ÿÿ.ÀÀÀÀÀÀ...ÿÿ. */
	$"00F0 0FC0 C00C 0C0C 0C0C C0C0 C0F0 0F00"            /* .ð.ÀÀ.....ÀÀÀð.. */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
};

data 'icl4' (135, "Physics") {
	$"0EDE EDEE EEEE EEEE EEEE EEEE EEEE 0000"            /* .Þíîîîîîîîîîîî.. */
	$"EDC0 0000 0000 0000 0000 0000 00CD E000"            /* íÀ...........Íà. */
	$"DCC0 0000 0000 0000 0000 0000 CCCD E000"            /* ÜÀ..........ÌÍà. */
	$"E000 0000 0000 0000 0000 00CC 0C0D E000"            /* à..........Ì..à. */
	$"E000 0000 0000 0000 000C 000C 0CCC A000"            /* à............Ì . */
	$"E000 0000 0000 0000 0000 C0C0 0C0D E000"            /* à.........ÀÀ..à. */
	$"E000 0000 0000 0000 0000 0C0C C0CD A000"            /* à...........ÀÍ . */
	$"E000 0000 0000 0000 0000 000C 0C0D E000"            /* à.............à. */
	$"E0E0 0000 0000 CDDD DDDC C0C0 C0CD A000"            /* àà....ÍÝÝÜÀÀÀÍ . */
	$"EE0E 0000 000E EDD8 8EDA E00C 0CCC A0A0"            /* î.....íØŽÚà..Ì   */
	$"E00E 0000 00EF C888 99AD FE00 C0CD AA0A"            /* à....ïÈˆ™­þ.ÀÍªÂ */
	$"000E 0000 0DFD 8888 899A DFEC 0C0D A0CF"            /* .....ýˆˆ‰šßì.. Ï */
	$"000E 0000 CFAD 8888 899F DAFC C0C0 C0DA"            /* ....Ï­ˆˆ‰ŸÚüÀÀÀÚ */
	$"000E 0000 DFED 9888 99AF DEFD 0CCC 0CDF"            /* ....ßí˜ˆ™¯Þý.Ì.ß */
	$"000E 0000 E9ED 9989 999F D99E 0C0C CCDA"            /* ....éí™‰™ŸÙž..ÌÚ */
	$"000E 0000 999D A999 9AFA D99A C0C0 CCCF"            /* ....™©™šúÙšÀÀÌÏ */
	$"000E 0000 9998 DFFF FFFD 899A 0C0C C0DF"            /* ....™˜ßÿÿý‰š..Àß */
	$"000E 0000 8999 DDEA FECB 999E C0CC 0CDF"            /* ....‰™ÝêþË™žÀÌ.ß */
	$"000E 0000 D989 9BDC CD89 889D 0CC0 CCDF"            /* ....Ù‰›ÜÍ‰ˆ.ÀÌß */
	$"000E 0000 C888 889D D988 88AC 0C0C DCCF"            /* ....ÈˆˆÙˆˆ¬..ÜÏ */
	$"000E 0000 0D88 888D C888 89D0 C0CD FDDF"            /* .....ˆˆÈˆ‰ÐÀÍýß */
	$"E00E 0000 00B8 888D D888 9E0C 0C0D FFCF"            /* à....¸ˆØˆž...ÿÏ */
	$"EE0E 0000 000D 888D D899 D0C0 C0CD F0F0"            /* î.....ˆØ™ÐÀÀÍðð */
	$"E0E0 0000 0000 CDED DEDC C00C 0C0D F000"            /* àà....ÍíÞÜÀ...ð. */
	$"E000 0000 0000 0000 0000 00C0 C0CD F000"            /* à..........ÀÀÍð. */
	$"E000 0000 0000 0000 000C 000C 0CCC F000"            /* à............Ìð. */
	$"E000 0000 0000 0000 0000 0C00 C0CD F000"            /* à...........ÀÍð. */
	$"E000 0000 0000 0000 0000 00CC 0CCC F000"            /* à..........Ì.Ìð. */
	$"E000 0000 0000 0000 000C 000C 0C0D F000"            /* à.............ð. */
	$"ECC0 0000 0000 0000 0000 C0C0 C0CD F000"            /* ìÀ........ÀÀÀÍð. */
	$"EDDD DDDD DDDD DDDD DDDD DDDD DDDD F000"            /* íÝÝÝÝÝÝÝÝÝÝÝÝÝð. */
	$"0AEA EAAA AAAF AFAF FFFF FFFF FFFF 0000"            /* Âêêªª¯¯¯ÿÿÿÿÿÿ.. */
};

data 'icl4' (136, "Images Icon") {
	$"0000 000F FFFF FFFF FFFF FFFF FF00 0000"            /* ....ÿÿÿÿÿÿÿÿÿ... */
	$"0000 000F A9FE 9F99 FFEF FEFF 9FF0 0000"            /* ....©þŸ™ÿïþÿŸð.. */
	$"0000 CEED DEDD 9FF8 F9E9 E959 EFDF 0000"            /* ..ÎíÞÝŸøùééYïß.. */
	$"000E EDD8 8BDA E9E9 9F95 FEF9 EFCD F000"            /* ..íØ‹ÚééŸ•þùïÍð. */
	$"00EF D888 99AD FA99 F9EF E95E 9F0C DF00"            /* .ïØˆ™­ú™ùïé^Ÿ.ß. */
	$"0DFD B888 899A DFE9 FF95 FEF9 EF00 CDF0"            /* .ý¸ˆ‰šßéÿ•þùï.Íð */
	$"CFAD 9888 89AF DAF9 F99E 5959 EFFF FFFF"            /* Ï­˜ˆ‰¯ÚùùžYYïÿÿÿ */
	$"DFED 9888 999F DEFE FFEF FEFE EF9E FE9F"            /* ßí˜ˆ™ŸÞþÿïþþïžþŸ */
	$"A9ED 9989 9AFF D99E F9F5 EF5F EF5F EFEF"            /* ©í™‰šÿÙžùõï_ï_ïï */
	$"999D 99A9 A9FA D999 FFFF F5F5 F5F5 F5FF"            /* ™™©©úÙ™ÿÿõõõõõÿ */
	$"999B DF9F FFFD B99A FFFE 65F5 FF5F 5F5F"            /* ™›ßŸÿý¹šÿþeõÿ___ */
	$"8999 DDEA AEDD 999E FFFF AF56 55FF F5FF"            /* ‰™Ýê®Ý™žÿÿ¯VUÿõÿ */
	$"D988 98DC CD88 989A FFA9 A9F5 6F5F 5EEF"            /* Ùˆ˜ÜÍˆ˜šÿ©©õo_^ï */
	$"C888 889D D988 889F FA9F FFFF F5F5 6EEF"            /* ÈˆˆÙˆˆŸúŸÿÿõõnï */
	$"0D88 889D D888 89EF F9AF AFAF FF5F EE5F"            /* .ˆˆØˆ‰ïù¯¯¯ÿ_î_ */
	$"00B8 888D D888 99FF 9A9A FFFF F5F5 6E9F"            /* .¸ˆØˆ™ÿššÿÿõõnŸ */
	$"000D 888D C899 EFFF A9FF FFFF FFFF 5E56"            /* ..ˆÈ™ïÿ©ÿÿÿÿÿ^V */
	$"0000 CDED E9EE FFFF FA9A FFFF FFF5 EE9F"            /* ..ÍíéîÿÿúšÿÿÿõîŸ */
	$"0000 000F FFFF 9FFF FFAF FFFF FF5E 5EEF"            /* ....ÿÿŸÿÿ¯ÿÿÿ^^ï */
	$"0000 000F FFFF 9FFA 9FFF FFFF FFFE E5EF"            /* ....ÿÿŸúŸÿÿÿÿþåï */
	$"0000 000F FF99 9FAF 9FFF FFFF FF56 EEEF"            /* ....ÿ™Ÿ¯ŸÿÿÿÿVîï */
	$"0000 000F F999 9AFF 99FF F9FF FF6E EE9F"            /* ....ù™šÿ™ÿùÿÿnîŸ */
	$"0000 000F 9999 9FFA F999 9FFF FF59 E9EF"            /* ....™™Ÿúù™ŸÿÿYéï */
	$"0000 000F 9899 99AF FAF9 9FFF FF6D 79EF"            /* ....˜™™¯úùŸÿÿmyï */
	$"0000 000F 9999 99FF FF99 AFFF FED7 7DEF"            /* ....™™™ÿÿ™¯ÿþ×}ï */
	$"0000 000F 9999 999A FFA9 FFFF 9E7D 785F"            /* ....™™™šÿ©ÿÿž}x_ */
	$"0000 000F 9999 999F FFF9 FFFF E987 795F"            /* ....™™™Ÿÿùÿÿé‡y_ */
	$"0000 000F 9999 F999 FFF9 FFFF 9DD7 859F"            /* ....™™ù™ÿùÿÿ×…Ÿ */
	$"0000 000F 9999 FFFF 99F9 FFF5 577D D95F"            /* ....™™ÿÿ™ùÿõW}Ù_ */
	$"0000 000F 99FF FFFF F99F FF56 E87D 759F"            /* ....™ÿÿÿùŸÿVè}uŸ */
	$"0000 000F FFFF FFFF 9999 FFFE 9D7D 9E5F"            /* ....ÿÿÿÿ™™ÿþ}ž_ */
	$"0000 000F FFFF FFFF FFFF FFFF FFFF FFFF"            /* ....ÿÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl4' (137, "MIDI Music Icon") {
	$"0000 000E DEEE EEEE EEEE EEEE EE00 0000"            /* ....Þîîîîîîîî... */
	$"0000 000D 0000 0000 0000 0000 0EE0 0000"            /* .............à.. */
	$"0000 CDDD DDDC 0000 0000 0000 0EEE 0000"            /* ..ÍÝÝÜ.......î.. */
	$"000E EDB8 8BDE E000 0000 000C 0ECE E000"            /* ..í¸‹Þà......Îà. */
	$"00EF C888 99AD FE00 0000 00C0 CE0C EE00"            /* .ïÈˆ™­þ....ÀÎ.î. */
	$"0DFD 8888 899A DFE0 0000 0000 0E0C CEE0"            /* .ýˆˆ‰šßà......Îà */
	$"CFAD 8888 899F D9FC 0000 000C 0EEE EEEA"            /* Ï­ˆˆ‰ŸÙü.....îîê */
	$"DFED 9888 99AF DEFE 0000 00C0 0DDD DDDE"            /* ßí˜ˆ™¯Þþ...À.ÝÝÞ */
	$"E9ED 9989 9A9F D99E 0000 000C 00DD DDDA"            /* éí™‰šŸÙž.....ÝÝÚ */
	$"999D 9999 A9FA D99A 0000 0000 0CC0 CCCA"            /* ™™™©úÙš.....ÀÌÊ */
	$"999E DFFF FFFD 8999 0000 00C0 C0CC 0CDA"            /* ™žßÿÿý‰™...ÀÀÌ.Ú */
	$"8999 DDEA FECB 9999 0000 000C 0CC0 C0CA"            /* ‰™ÝêþË™™.....ÀÀÊ */
	$"D888 98DC CD89 889D 0000 0000 C0CC CCDA"            /* Øˆ˜ÜÍ‰ˆ....ÀÌÌÚ */
	$"C988 889D D988 889C 0000 000C 00C0 CCDA"            /* ÉˆˆÙˆˆœ.....ÀÌÚ */
	$"0D88 889D D888 89D0 0000 F000 CC0C CCDA"            /* .ˆˆØˆ‰Ð..ð.Ì.ÌÚ */
	$"00B8 888D C888 9E00 0000 F00C 00CC 0CCF"            /* .¸ˆÈˆž...ð..Ì.Ï */
	$"000D 889D D899 D000 0000 F000 CCF0 CCDF"            /* ..ˆØ™Ð...ð.ÌðÌß */
	$"0000 CDED DEDC 00F0 0000 F00C 00FC C0DF"            /* ..ÍíÞÜ.ð..ð..üÀß */
	$"0000 000E 0000 00F0 0000 F000 CCFC CCDF"            /* .......ð..ð.ÌüÌß */
	$"0000 000E 0000 00F0 0000 F00C 00F0 CCCF"            /* .......ð..ð..ðÌÏ */
	$"0000 000E 0000 00F0 0000 F000 CCFC 0CDF"            /* .......ð..ð.Ìü.ß */
	$"0000 000E 0DDD DDFD DDDD FDDD DDFD DDDF"            /* .....ÝÝýÝÝýÝÝýÝß */
	$"0AAA AAAE AEEE EEEE EEEE F000 0CF0 C0DF"            /* Âªª®®îîîîîð..ðÀß */
	$"0A22 2222 2222 2242 223E F000 C0FC CCDF"            /* Â""""""B">ð.ÀüÌß */
	$"0A40 3330 3030 0003 303E FDDD FFFD DDDF"            /* Â@3000..0>ýÝÿýÝß */
	$"0A20 0300 3030 3330 303E 000F FFFC 0CCF"            /* Â ..00300>..ÿü.Ï */
	$"0A20 3030 3030 3330 303E 000F FFFC CCDF"            /* Â 0000300>..ÿüÌß */
	$"0E20 3030 3030 0003 303E DDDD FFDD DDCF"            /* . 0000..0>ÝÝÿÝÝÏ */
	$"0A33 3333 3333 3333 333E 0000 0C0C C0DF"            /* Â33333333>....Àß */
	$"0EEE EEEE EEEE EEEE EEEE 0000 C0C0 CCDF"            /* .îîîîîîîîî..ÀÀÌß */
	$"0000 000A 0000 0000 0000 00C0 0CCC 0CDF"            /* ...Â.......À.Ì.ß */
	$"0000 000A FAAF FFFF FFFF FFFF FFFF FFFF"            /* ...Âú¯ÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl4' (138, "MML Script Icon") {
	$"0000 000E DEEE EEEE EEEE EEEE EE00 0000"            /* ....Þîîîîîîîî... */
	$"0000 000D 0000 0000 0000 0000 0EE0 0000"            /* .............à.. */
	$"0000 CDDD DDDC 0000 0000 0000 0EEE 0000"            /* ..ÍÝÝÜ.......î.. */
	$"000E EDB8 8BDE E000 00EE EACC 0ECE E000"            /* ..í¸‹Þà..îêÌ.Îà. */
	$"00EF C888 99AD FE00 0E00 CCAC CE0C EE00"            /* .ïÈˆ™­þ...Ì¬Î.î. */
	$"0DFD 8888 899A DFE0 E00C CDDA CE00 CEE0"            /* .ýˆˆ‰šßàà.ÍÚÎ.Îà */
	$"CFAD 8888 899F DAFE 00CC DDEE DEEA EEEA"            /* Ï­ˆˆ‰ŸÚþ.ÌÝîÞêîê */
	$"DFED 9888 99AF DEFD 0CCD DDED DDDD DDDE"            /* ßí˜ˆ™¯Þý.ÍÝíÝÝÝÞ */
	$"E9ED 9989 99FF DE9E CCDD EADD C0DD DDDA"            /* éí™‰™ÿÞžÌÝêÝÀÝÝÚ */
	$"999D 99A9 AFFA D99A CDDE EDAC CC0C 0CCA"            /* ™™©¯úÙšÍÞí¬Ì..Ê */
	$"999E DF9F FFFD 8999 CDDA DDDA CCC0 CCDA"            /* ™žßŸÿý‰™ÍÚÝÚÌÀÌÚ */
	$"8999 DDEA FECB 999E DDAD DDCC ACCC CCCA"            /* ‰™ÝêþË™žÝ­ÝÌ¬ÌÌÊ */
	$"D888 98DC CD89 889D EADD DCCC CACC CCDA"            /* Øˆ˜ÜÍ‰ˆêÝÜÌÊÌÌÚ */
	$"C888 889D D988 889D ADDD CCCC 00AC C0DA"            /* ÈˆˆÙˆˆ­ÝÌÌ.¬ÀÚ */
	$"0D88 888D D888 89DA DDDC CCC0 000F CCDA"            /* .ˆˆØˆ‰ÚÝÜÌÀ..ÌÚ */
	$"00B8 888D C888 9EDD DCCC CC00 000F DCCF"            /* .¸ˆÈˆžÝÜÌÌ...ÜÏ */
	$"000D 888D D889 DDDD CCCC C000 000C FCDF"            /* ..ˆØ‰ÝÝÌÌÀ...üß */
	$"0000 CDED DEDA DDDC CCCC C000 00CC FDDA"            /* ..ÍíÞÚÝÜÌÌÀ..ÌýÚ */
	$"0000 000E 0000 ADCC CCCC 0000 0CCC FDDF"            /* ......­ÌÌÌ...Ìýß */
	$"0000 000E 0000 CACC CCC0 0000 CCCC FDDF"            /* ......ÊÌÌÀ..ÌÌýß */
	$"0000 000E 0000 0CFC CC00 000C CCCF DDDF"            /* .......üÌ...ÌÏÝß */
	$"0000 000E 0000 000F C000 00CC CCFD DCDF"            /* ........À..ÌÌýÜß */
	$"0000 000E 0000 000C F000 0CCC CFDD CCDF"            /* ........ð..ÌÏÝÌß */
	$"0000 000E 0000 000F DF00 0CCC FDDC CCCF"            /* ........ß..ÌýÜÌÏ */
	$"0000 000E 0000 00FD DF00 CCCF DDCC CCDF"            /* .......ýß.ÌÏÝÌÌß */
	$"0000 000A 0000 0FDD DCFC CCFD DCCC CCCF"            /* ...Â...ÝÜüÌýÜÌÌÏ */
	$"0000 000E 0000 0FDD CCFC CFDD CCC0 CCDF"            /* .......ÝÌüÏÝÌÀÌß */
	$"0000 000A 0000 0ADC C0FC FDDC CC0C C0DF"            /* ...Â..ÂÜÀüýÜÌ.Àß */
	$"0000 000A 0000 00FC C0FF DDCC 0CC0 CCDF"            /* ...Â...üÀÿÝÌ.ÀÌß */
	$"0000 000A 0000 00CF FFFD DCC0 C00C CCCF"            /* ...Â...ÏÿýÜÀÀ.ÌÏ */
	$"0000 000A 0000 000C CDDD C00C 0CC0 CCDF"            /* ...Â....ÍÝÀ..ÀÌß */
	$"0000 000A AAFF FFFF FFFF FFFF FFFF FFFF"            /* ...Âªÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl4' (139, "Text File Icon") {
	$"0000 000E DEEE EEEE EEEE EEEE EE00 0000"            /* ....Þîîîîîîîî... */
	$"0000 000D 0000 0000 0000 0000 0EE0 0000"            /* .............à.. */
	$"0000 CDDD DDDC 0000 0000 0000 0EEE 0000"            /* ..ÍÝÝÜ.......î.. */
	$"000E EDB8 8BDE E000 0000 000C 0ECE E000"            /* ..í¸‹Þà......Îà. */
	$"00EF C888 99AD FE00 0000 00C0 CE0C EE00"            /* .ïÈˆ™­þ....ÀÎ.î. */
	$"0DFD 8888 899A DFE0 0000 0000 0E0C CEE0"            /* .ýˆˆ‰šßà......Îà */
	$"CFAD 8888 899F D9FC 0000 000C 0EEE EEEA"            /* Ï­ˆˆ‰ŸÙü.....îîê */
	$"DFED 9888 99AF DEFE 0000 00C0 0DDD DDDE"            /* ßí˜ˆ™¯Þþ...À.ÝÝÞ */
	$"E9ED 9989 9A9F D99E 0000 000C 00DD DDDA"            /* éí™‰šŸÙž.....ÝÝÚ */
	$"999D 9999 A9FA D99A 0000 0000 0CC0 CCCA"            /* ™™™©úÙš.....ÀÌÊ */
	$"999E DFFF FFFD 8999 EE0E EE0A E0AA 0CDA"            /* ™žßÿÿý‰™î.îÂàª.Ú */
	$"8999 DDEA FECB 999E 0000 0000 0C00 CCCA"            /* ‰™ÝêþË™ž......ÌÊ */
	$"D888 98DC CD89 889D 0000 000C 0CCC 0CDA"            /* Øˆ˜ÜÍ‰ˆ.....Ì.Ú */
	$"C988 889D D988 889C EEA0 EEAA 0EAA CCDA"            /* ÉˆˆÙˆˆœî îª.ªÌÚ */
	$"0D88 889D D888 89D0 0000 0000 0C00 CCDA"            /* .ˆˆØˆ‰Ð......ÌÚ */
	$"00B8 888D C888 9B00 0000 0000 0C0C CCCF"            /* .¸ˆÈˆ›.......ÌÏ */
	$"000D 888D D899 D0AE EEAA A0AA A0FA 0CD6"            /* ..ˆØ™Ð®îª ª ú.Ö */
	$"0000 CDED DEDC 0000 0000 0000 0C00 CCDA"            /* ..ÍíÞÜ........ÌÚ */
	$"0000 000E 0000 0000 0000 000C 0CCC C0DF"            /* .............ÌÀß */
	$"0000 000E 0000 0000 0000 0000 C00C 0CDF"            /* ............À..ß */
	$"0000 000E 0000 0000 0000 00C0 0CCC CCDF"            /* ...........À.ÌÌß */
	$"0000 000E 0000 0000 0000 0000 C0C0 CCCF"            /* ............ÀÀÌÏ */
	$"0000 000E 0000 0000 0000 00C0 0C0C CCDF"            /* ...........À..Ìß */
	$"0000 000E 0000 0000 0000 000C 0CC0 CCCF"            /* .............ÀÌÏ */
	$"0000 000E 0000 0000 0000 0000 C0CC C0DF"            /* ............ÀÌÀß */
	$"0000 000A 0000 0000 0000 000C 0C0C 0CDF"            /* ...Â...........ß */
	$"0000 000E 0000 0000 0000 00C0 C0CC CCDF"            /* ...........ÀÀÌÌß */
	$"0000 000A 0000 0000 0000 0000 0CC0 CCCF"            /* ...Â.........ÀÌÏ */
	$"0000 000A 0000 0000 0000 00C0 0C0C CCDF"            /* ...Â.......À..Ìß */
	$"0000 000A 0000 0000 0000 0000 C0CC C0DF"            /* ...Â........ÀÌÀß */
	$"0000 000A 0000 0000 0000 000C 0C0C CCDF"            /* ...Â..........Ìß */
	$"0000 000A AAFF FFFF FFFF FFFF FFFF FFFF"            /* ...Âªÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl4' (140, "Generic File Icon") {
	$"0000 000E DEEE EEEE EEEE EEEE EE00 0000"            /* ....Þîîîîîîîî... */
	$"0000 000D 0000 0000 0000 0000 0EE0 0000"            /* .............à.. */
	$"0000 CDDD DDDC 0000 0000 0000 0EEE 0000"            /* ..ÍÝÝÜ.......î.. */
	$"000E EDB8 8BDE E000 0000 000C 0ECE E000"            /* ..í¸‹Þà......Îà. */
	$"00EF C888 99AD FE00 0000 00C0 CE0C EE00"            /* .ïÈˆ™­þ....ÀÎ.î. */
	$"0DFD 8888 899A DFE0 0000 0000 0E0C CEE0"            /* .ýˆˆ‰šßà......Îà */
	$"CFAD 8888 899F D9FC 0000 000C 0EEE EEEA"            /* Ï­ˆˆ‰ŸÙü.....îîê */
	$"DFED 9888 99AF DEFE 0000 00C0 0DDD DDDE"            /* ßí˜ˆ™¯Þþ...À.ÝÝÞ */
	$"E9ED 9989 9A9F D99E 0000 000C 00DD DDDA"            /* éí™‰šŸÙž.....ÝÝÚ */
	$"999D 9999 A9FA D99A 0000 0000 0CC0 CCCA"            /* ™™™©úÙš.....ÀÌÊ */
	$"999E DFFF FFFD 8999 0000 00C0 C0CC 0CDA"            /* ™žßÿÿý‰™...ÀÀÌ.Ú */
	$"8999 DDEA FECB 9999 0000 000C 0CC0 C0CA"            /* ‰™ÝêþË™™.....ÀÀÊ */
	$"D888 98DC CD89 889D 0000 0000 C0CC CCDA"            /* Øˆ˜ÜÍ‰ˆ....ÀÌÌÚ */
	$"C988 889D D988 889C 0000 000C 00C0 CCDA"            /* ÉˆˆÙˆˆœ.....ÀÌÚ */
	$"0D88 889D D888 89D0 0000 0000 CC0C CCDA"            /* .ˆˆØˆ‰Ð....Ì.ÌÚ */
	$"00B8 888D C888 9B00 0000 000C 00CC 0CCF"            /* .¸ˆÈˆ›......Ì.Ï */
	$"000D 889D D88A D000 0000 00C0 C0CC CCDF"            /* ..ˆØŠÐ....ÀÀÌÌß */
	$"0000 CDED DEDC 0000 0000 0000 0CC0 CCCF"            /* ..ÍíÞÜ.......ÀÌÏ */
	$"0000 000E 0000 0000 0000 00C0 C0CC 0CDF"            /* ...........ÀÀÌ.ß */
	$"0000 000E 0000 0000 0000 000C 0C0C C0DF"            /* ..............Àß */
	$"0000 000E 0000 0000 0000 0000 C0C0 CCDF"            /* ............ÀÀÌß */
	$"0000 000E 0000 0000 0000 000C 0C0C CCDF"            /* ..............Ìß */
	$"0000 000E 0000 0000 0000 0000 C0CC C0DF"            /* ............ÀÌÀß */
	$"0000 000E 0000 0000 0000 000C 0CC0 CCDF"            /* .............ÀÌß */
	$"0000 000E 0000 0000 0000 00C0 00CC 0CDF"            /* ...........À.Ì.ß */
	$"0000 000A 0000 0000 0000 0000 CC0C CCCF"            /* ...Â........Ì.ÌÏ */
	$"0000 000E 0000 0000 0000 000C 00C0 CCDF"            /* .............ÀÌß */
	$"0000 000A 0000 0000 0000 00C0 0CCC CCCF"            /* ...Â.......À.ÌÌÏ */
	$"0000 000A 0000 0000 0000 000C 00C0 CCDF"            /* ...Â.........ÀÌß */
	$"0000 000A 0000 0000 0000 0000 CC0C C0DF"            /* ...Â........Ì.Àß */
	$"0000 000A 0000 0000 0000 00C0 0CCC CCDF"            /* ...Â.......À.ÌÌß */
	$"0000 000A AAFF FFFF FFFF FFFF FFFF FFFF"            /* ...Âªÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl4' (141, "Folder Icon") {
	$"00D5 EE00 0000 0000 0000 0000 0000 0000"            /* .Õî............. */
	$"00DC DDEE 0000 0000 0000 0000 0000 0000"            /* .ÜÝî............ */
	$"DD5C CCDD EE00 0000 0000 0000 0000 0000"            /* Ý\ÌÝî........... */
	$"DCC5 E7CC DDEE 0000 0000 0000 0000 0000"            /* ÜÅçÌÝî.......... */
	$"DCC0 C5EC C7DD EE0E EEE0 0000 0000 0000"            /* ÜÀÅìÇÝî.îà...... */
	$"DCCC C0C5 ECCC DDED CDDE E000 0000 0000"            /* ÜÌÀÅìÌÝíÍÞà..... */
	$"DCCC CCCC C5E7 C7DC DC7D DEE0 0000 0000"            /* ÜÌÌÌÅçÇÜÜ}Þà.... */
	$"DCCC CCCC C07D ECCC C7CC 7DE0 0000 0000"            /* ÜÌÌÌÀ}ìÌÇÌ}à.... */
	$"DCCC CCCC CCCC D5ED CD7D DDD5 0000 0000"            /* ÜÌÌÌÌÌÕíÍ}ÝÕ.... */
	$"5CCC CCCC CCCC C0C5 EDDD 7DDE E000 0000"            /* \ÌÌÌÌÌÀÅíÝ}Þà... */
	$"DCCC CCCC CCCC CCC0 7DED D7DD E000 0000"            /* ÜÌÌÌÌÌÌÀ}í×Ýà... */
	$"5CCC CCCC CCCC CCCC C0D5 EDCD F000 0000"            /* \ÌÌÌÌÌÌÌÀÕíÍð... */
	$"DCCC CCCC CCCC CCCC CCC0 7EDD 9000 0000"            /* ÜÌÌÌÌÌÌÌÌÀ~Ý... */
	$"5CCC CCCC CCCC CCCC CCCC CCED E000 0000"            /* \ÌÌÌÌÌÌÌÌÌÌíà... */
	$"DCCC CCCD EDDD DECC CCCC CCE5 9000 0000"            /* ÜÌÌÍíÝÞÌÌÌÌå... */
	$"5CCC CCEE DBBE BDEE CCCC CDE5 F000 0000"            /* \ÌÌîÛ¾½îÌÌÍåð... */
	$"DCCC CEAD 8888 99DF ECCC CCAD 9000 0000"            /* ÜÌÎ­ˆˆ™ßìÌÌ­... */
	$"5CCC DFD8 8888 99AD FECC CDE5 5000 0000"            /* \ÌßØˆˆ™­þÌÍåP... */
	$"5CCC FAD8 8888 9AFD AFCC CCE5 9000 0000"            /* \ÌúØˆˆšý¯ÌÌå... */
	$"ECCD FED9 8889 99FD EFDC CDE5 E000 0000"            /* ìÍþÙˆ‰™ýïÜÍåà... */
	$"DDCE 9ED9 9899 AFFD E9EC CCE5 EFAA 0000"            /* ÝÎžÙ˜™¯ýéìÌåïª.. */
	$"EE59 99DE 999A 9FAD 99AC CDED AAAA AA00"            /* îY™Þ™šŸ­™¬Ííªªª. */
	$"00E9 99BD FFFF FFD8 999C CD55 5A9F AAAA"            /* .é™½ÿÿÿØ™œÍUZŸªª */
	$"000B 999D DEAF EDD9 89EC CCE5 EFAA AAAA"            /* ..™Þ¯íÙ‰ìÌåïªªª */
	$"000D 8889 8DCC DB89 89DC CDE5 EAAA AAAA"            /* ..ˆ‰ÌÛ‰‰ÜÍåêªªª */
	$"000C 8888 89DD 9888 89CC CCE5 FAAA AAAA"            /* ..ˆˆ‰Ý˜ˆ‰ÌÌåúªªª */
	$"0000 D888 89DD 8888 9DCC C7E5 9AAA AAAA"            /* ..Øˆ‰ÝˆˆÌÇåšªªª */
	$"0000 0B88 88DC 8889 ECCC CDE5 EAAA AAA0"            /* ...ˆˆÜˆ‰ìÌÍåêªª  */
	$"0000 00D8 88DC 889E DD7C CCE5 9FAA AA00"            /* ...ØˆÜˆžÝ|ÌåŸªª. */
	$"0000 000C DECC EDC0 EE5D CDE5 5AAA A000"            /* ....ÞÌíÀî]ÍåZª . */
	$"0000 0000 0000 0000 00EE 5DE5 9AAA 0000"            /* .........î]åšª.. */
	$"0000 0000 0000 0000 0000 EE9E AAA0 0000"            /* ..........îžª .. */
};

data 'icl8' (128, "Application Icon") {
	$"0000 0000 0000 0000 0000 F656 FCFD FEFD"            /* ..........öVüýþý */
	$"FEFE FDFB 56F6 0000 0000 0000 0000 0000"            /* þþýûVö.......... */
	$"0000 0000 0000 0000 F7FB FFFF FB56 F72B"            /* ........÷ûÿÿûV÷+ */
	$"2B2B 56FC FFFF FCF7 0000 0000 0000 0000"            /* ++Vüÿÿü÷........ */
	$"0000 0000 0000 F5FB FFFF 812B 5075 7CA7"            /* ......õûÿÿ+Pu|§ */
	$"A7A6 7BF7 F7FB FFFF FBF5 0000 0000 0000"            /* §¦{÷÷ûÿÿûõ...... */
	$"0000 0000 002B FDFF FF56 257C A1A7 E6A1"            /* .....+ýÿÿV%|¡§æ¡ */
	$"A7CB A7E8 A62B F9FF FFFD F700 0000 0000"            /* §Ë§è¦+ùÿÿý÷..... */
	$"0000 0000 F7EA FFFF F82C E5A1 A1A1 A1A1"            /* ....÷êÿÿø,å¡¡¡¡¡ */
	$"A1A7 E7A7 D1AD F7F9 EAFF FEF7 0000 0000"            /* ¡§ç§Ñ­÷ùêÿþ÷.... */
	$"0000 002B E9FF FFFA 25A1 A19B A19B A1A1"            /* ...+éÿÿú%¡¡›¡›¡¡ */
	$"A7A1 A7E8 A7D1 D12B 81EA EAEA 2B00 0000"            /* §¡§è§ÑÑ+êêê+... */
	$"0000 F5AD EAEA FDF6 A1A1 9B7D 9B9B A1A1"            /* ..õ­êêýö¡¡›}››¡¡ */
	$"A1A7 A7A7 E7AD E9A6 F6E0 E9EA FDF5 0000"            /* ¡§§§ç­é¦öàéêýõ.. */
	$"0000 81EA E9FF F950 A1A1 A19B 9B77 A19B"            /* ..êéÿùP¡¡¡››w¡› */
	$"A1A7 A1E7 A7E8 D1FF F87B E9E9 EAFB 0000"            /* ¡§¡ç§èÑÿø{ééêû.. */
	$"00F7 D1E9 E9E9 2B7C A1A1 A19B 7D9B A1A1"            /* .÷Ñééé+|¡¡¡›}›¡¡ */
	$"A1A1 E7A7 E8AD D1E0 81F7 E9AD E9E9 2B00"            /* ¡¡ç§è­Ñà÷é­éé+. */
	$"00FB E9AD E9AC F6A1 A7A1 77A1 9BA1 A1A1"            /* .ûé­é¬ö¡§¡w¡›¡¡¡ */
	$"A1E6 A7A7 A7E8 ADE9 FDF6 ADD1 D1E9 FC00"            /* ¡æ§§§è­éýö­ÑÑéü. */
	$"F6E7 ADE8 E9A6 F6A7 E5A1 A1A1 A19B A1A1"            /* öç­èé¦ö§å¡¡¡¡›¡¡ */
	$"A7A7 A7E7 E8AD D1EA FEF5 E8AD E8AD E9F6"            /* §§§çè­Ñêþõè­è­éö */
	$"50D1 E8AD D1FC 2BA7 A7A1 A7A1 A1A7 A1A7"            /* PÑè­Ñü+§§¡§¡¡§¡§ */
	$"A1A7 CBA7 ADE8 E9E9 E0F6 A6E8 ADD1 E956"            /* ¡§Ë§­èééàö¦è­ÑéV */
	$"FAE8 ADE7 D1A6 07CB A7A7 A1A7 A1A1 A7A7"            /* úè­çÑ¦.Ë§§¡§¡¡§§ */
	$"CBA7 E7E8 ADD1 E9E0 FDF5 ADE7 E7AD D181"            /* Ë§çè­Ñéàýõ­çç­Ñ */
	$"A6A7 E7A7 D1AD 2582 E7A7 CBA7 CBA7 A7CB"            /* ¦§ç§Ñ­%‚ç§Ë§Ë§§Ë */
	$"A7E7 A7AD D1AD E9FF FBF7 A7E7 ADA7 E8AC"            /* §ç§­Ñ­éÿû÷§ç­§è¬ */
	$"A7E8 A7E8 A7E9 F856 E8E7 A7A7 A7CB A7A7"            /* §è§è§éøVèç§§§Ë§§ */
	$"E8AD E8E8 E9E0 EAFF F850 E8A7 E7E7 ADD1"            /* è­èèéàêÿøPè§çç­Ñ */
	$"A7A7 E7A7 A7E8 8224 ADD1 A7E7 E8A7 E8E7"            /* §§ç§§è‚$­Ñ§çè§èç */
	$"ADE8 D1AD E9E9 FFFD F5A6 A7E7 A7A7 E8AD"            /* ­èÑ­ééÿýõ¦§ç§§è­ */
	$"A7A7 A7E7 A7CB E82C F7E9 D1AD E8AD E8AD"            /* §§§ç§Ëè,÷éÑ­è­è­ */
	$"E8AD E9EA E0EA FF2B 50E7 A7A7 E7A7 ADD1"            /* è­éêàêÿ+Pç§§ç§­Ñ */
	$"A6CB A7A7 CBA7 A7A6 F6F9 EAD1 ADD1 ADD1"            /* ¦Ë§§Ë§§¦öùêÑ­Ñ­Ñ */
	$"E9D1 E0EA FFFF F825 A7A7 A7CB A7A7 CBFE"            /* éÑàêÿÿø%§§§Ë§§Ëþ */
	$"A0A7 A7A7 A1CB A7E7 A001 F8FE EAEA EAE0"            /*  §§§¡Ë§ç .øþêêêà */
	$"E9FF EAFF FDF7 F6A7 E6A7 A7A1 A7E6 ADA6"            /* éÿêÿý÷ö§æ§§¡§æ­¦ */
	$"57CB A1E5 A7A7 A1A7 A7A6 F7F5 F9AC FEE9"            /* WË¡å§§¡§§¦÷õù¬þé */
	$"E0FE ACF9 F52C A1A7 A1A7 A1CB A1A7 D181"            /* àþ¬ùõ,¡§¡§¡Ë¡§Ñ */
	$"50A7 A7A1 A1A7 E5A1 A7E5 A77C 2BF5 F6F6"            /* P§§¡¡§å¡§å§|+õöö */
	$"F6F5 F52C A0A1 CBA1 A7A1 A7A1 A7E6 E956"            /* öõõ, ¡Ë¡§¡§¡§æéV */
	$"F6A1 C5A1 A7A1 A1A7 A1A1 A1A1 E6A1 82F5"            /* ö¡Å¡§¡¡§¡¡¡¡æ¡‚õ */
	$"247C A1E5 A1A7 A1A7 A1E5 A1A7 A1A7 E9F6"            /* $|¡å¡§¡§¡å¡§¡§éö */
	$"007B A1A1 A1A1 A1A1 A1A1 A1A1 A1CB E8F6"            /* .{¡¡¡¡¡¡¡¡¡¡¡Ëèö */
	$"2CA1 A1A1 A1A1 A1A1 A1A1 A1A1 CBD1 FB00"            /* ,¡¡¡¡¡¡¡¡¡¡¡ËÑû. */
	$"002B A7A1 A1A1 A1A1 A1A1 A1A1 A1A1 ADF5"            /* .+§¡¡¡¡¡¡¡¡¡¡¡­õ */
	$"F6A1 A1A1 A1A1 A1A1 A1A1 A1A1 A7E0 2B00"            /* ö¡¡¡¡¡¡¡¡¡¡¡§à+. */
	$"0000 7BA1 A1A1 A1A1 A1A1 A1A1 A1A7 E7F6"            /* ..{¡¡¡¡¡¡¡¡¡¡§çö */
	$"25A1 A1A1 A1A1 A1A1 A1A1 A1A7 D181 0000"            /* %¡¡¡¡¡¡¡¡¡¡§Ñ.. */
	$"0000 01A0 A1A1 9BA1 9BA1 A19B A1A1 ADF6"            /* ... ¡¡›¡›¡¡›¡¡­ö */
	$"25A1 A1A1 9BA1 9BA1 9BA1 A1E8 ADF5 0000"            /* %¡¡¡›¡›¡›¡¡è­õ.. */
	$"0000 002B A1A1 A19B 7D9B A1A1 9BA1 E706"            /* ...+¡¡¡›}›¡¡›¡ç. */
	$"26A1 9B77 A19B 7D9B A1A1 E8FD 2B00 0000"            /* &¡›w¡›}›¡¡èý+... */
	$"0000 0000 2BA1 A1A1 9BA1 9B77 A1A1 AD25"            /* ....+¡¡¡›¡›w¡¡­% */
	$"F6A1 9BA1 9BA1 9BA1 A1E8 FEF7 0000 0000"            /* ö¡›¡›¡›¡¡èþ÷.... */
	$"0000 0000 002B A0A1 A19B 7D9B 9BA1 A72A"            /* .....+ ¡¡›}››¡§* */
	$"269B 7D9B 7D9B A1A7 D1AC 2B00 0000 0000"            /* &›}›}›¡§Ñ¬+..... */
	$"0000 0000 0000 077B A7A1 A19B A1A1 E707"            /* .......{§¡¡›¡¡ç. */
	$"F5A1 9BA1 A1A7 E8E9 81F5 0000 0000 0000"            /* õ¡›¡¡§èéõ...... */
	$"0000 0000 0000 0000 F7FB A7CB A1A7 E8F5"            /* ........÷û§Ë¡§èõ */
	$"2CA1 A7CB E8AD 81F7 0000 0000 0000 0000"            /* ,¡§Ëè­÷........ */
	$"0000 0000 0000 0000 0000 F650 81A6 FDF6"            /* ..........öP¦ýö */
	$"F6D1 A681 F82A 0000 0000 0000 0000 0000"            /* öÑ¦ø*.......... */
};

data 'icl8' (129, "Map Icon") {
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F5F5 00F5 97F5 F5F5 F5AC FBFC 0000 0000"            /* õõ.õ—õõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A082 FAAC FC00 00F5"            /* ..õû¬{|  ‚ú¬ü..õ */
	$"00F5 F5F5 97F5 F5F6 F5FC F7FB AC00 0000"            /* .õõõ—õõöõü÷û¬... */
	$"00F5 ACFE 50A1 A1A1 A7CB AD7A DFAC 0000"            /* .õ¬þP¡¡¡§Ë­zß¬.. */
	$"F500 F5F5 97F5 F6F5 F6AC F5F7 FBAC 0000"            /* õ.õõ—õöõö¬õ÷û¬.. */
	$"0081 FF7A 7D9B 9BA1 A1A7 E7AD FAEA FBF5"            /* .ÿz}››¡¡§ç­úêûõ */
	$"00F5 F523 23F5 9797 F5AC F5F6 F7FB AC00"            /* .õõ##õ——õ¬õö÷û¬. */
	$"2BE9 AD57 C57D 9BA1 A1A7 E8E9 F9E8 E92B"            /* +é­WÅ}›¡¡§èéùèé+ */
	$"F500 F523 2397 F5F5 F5AC ACAC ACAC ACFD"            /* õ.õ##—õõõ¬¬¬¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADEA FAA6 D1FB"            /* {éüu§¡¡¡§Ë­êú¦Ñû */
	$"00F5 F5F5 97F5 F5F6 F556 FAFA FAFA FAAC"            /* .õõõ—õõöõVúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBAD E8FF FAA7 E8A6"            /* ¦è¦ùË§¡§Ë­èÿú§è¦ */
	$"00F5 F5F5 97F5 F5F5 F6F5 5656 5656 56FD"            /* .õõõ—õõõöõVVVVVý */
	$"A7A7 E8F9 A7E7 A7CB ADE8 EAFD 56E7 A7AD"            /* §§èù§ç§Ë­èêýVç§­ */
	$"F5F5 00F5 97F5 F5F6 F5F6 F6F5 F6F6 F8FD"            /* õõ.õ—õõöõööõööøý */
	$"A7A7 A7A6 56E9 E9D1 D1E0 EA56 A0A7 A7D0"            /* §§§¦VééÑÑàêV §§Ð */
	$"00F5 F5F5 97F5 F5F5 F6F5 F6F6 F5F6 56FD"            /* .õõõ—õõõöõööõöVý */
	$"A0A7 E6A7 7B56 FCFD FEFC F87C A7A7 A7A6"            /*  §æ§{Vüýþüø|§§§¦ */
	$"F500 F5F5 F597 F5F6 F5F5 F6F5 2B01 55FD"            /* õ.õõõ—õöõõöõ+.Uý */
	$"57C5 A1A1 A7A0 572C 4F51 A1E6 A1A1 E7FA"            /* WÅ¡¡§ W,OQ¡æ¡¡çú */
	$"00F5 F5F5 F597 F5F5 F6F6 F5F6 F62A F8FD"            /* .õõõõ—õõööõöö*øý */
	$"25A7 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E8F7"            /* %§¡¡¡¡æzQæ¡¡¡¡è÷ */
	$"F500 F5F5 F597 F5F6 F5F5 F6F6 F5F6 56FD"            /* õ.õõõ—õöõõööõöVý */
	$"0075 A1A1 A19B A757 51A1 A1A1 9BE7 FA00"            /* .u¡¡¡›§WQ¡¡¡›çú. */
	$"01F5 F5F5 F597 F5F5 F6F6 F5F6 F6F6 56FD"            /* .õõõõ—õõööõöööVý */
	$"00F5 7CA1 9BA1 A17B 4AA1 779B E782 F500"            /* .õ|¡›¡¡{J¡w›ç‚õ. */
	$"F5F5 F5F5 F5F5 97F6 F5F5 F6F6 F6F6 F8FE"            /* õõõõõõ—öõõööööøþ */
	$"0000 F57B A19B A156 51E3 A1AD 7B00 00F5"            /* ..õ{¡›¡VQã¡­{..õ */
	$"00F5 00F5 F523 23F5 F6F5 F6F5 F6F6 56FD"            /* .õ.õõ##õöõöõööVý */
	$"0000 0000 2B7B A67B 57A6 FA4F 0000 F500"            /* ....+{¦{W¦úO..õ. */
	$"F5F5 F5F5 F523 2397 9797 9797 97F5 56FD"            /* õõõõõ##——————õVý */
	$"0000 0000 0000 00FC 0000 0000 00F5 00F5"            /* .......ü.....õ.õ */
	$"00F5 F5F5 9797 97F5 F5F6 F6F6 F6F6 56E0"            /* .õõõ———õõöööööVà */
	$"0000 0000 0000 00AC 0000 9700 0000 F500"            /* .......¬..—...õ. */
	$"F500 9797 F5F5 97F5 F6F5 F6F5 F6F6 F8FE"            /* õ.——õõ—õöõöõööøþ */
	$"0000 0000 0000 00AC 0000 0097 9700 00F5"            /* .......¬...——..õ */
	$"0097 97F5 F5F5 F597 F5F6 F5F6 F6F6 56FE"            /* .——õõõõ—õöõöööVþ */
	$"0000 0000 0000 00AC 0000 0000 9797 2323"            /* .......¬....——## */
	$"97F5 F5F5 F5F5 F597 F6F5 F6F6 F6F5 56FE"            /* —õõõõõõ—öõöööõVþ */
	$"0000 0000 0000 00AC 0000 0000 00F5 2323"            /* .......¬.....õ## */
	$"F500 F5F5 F5F5 F697 F5F6 F5F6 F6F6 56EA"            /* õ.õõõõö—õöõöööVê */
	$"0000 0000 0000 00AC 0000 0000 F500 0097"            /* .......¬....õ..— */
	$"00F5 F5F5 F5F5 F5F5 97F5 F6F5 F6F6 F8E0"            /* .õõõõõõõ—õöõööøà */
	$"0000 0000 0000 00AC 0000 0000 0000 F597"            /* .......¬......õ— */
	$"F500 F5F5 F5F5 F6F5 97F6 F6F6 F6F6 56EA"            /* õ.õõõõöõ—öööööVê */
	$"0000 0000 0000 00FD 0000 0000 00F5 0097"            /* .......ý.....õ.— */
	$"00F5 F5F5 F5F5 F5F6 9797 F6F5 F6F6 F8F4"            /* .õõõõõõö——öõööøô */
	$"0000 0000 0000 00AC 0000 0000 0000 F597"            /* .......¬......õ— */
	$"F500 F5F5 F5F5 F5F5 F697 F5F6 F6F6 56FF"            /* õ.õõõõõõö—õöööVÿ */
	$"0000 0000 0000 00FD 0000 0000 F500 2323"            /* .......ý....õ.## */
	$"00F5 F5F5 F5F5 F5F5 2323 F6F6 F6F6 F8FF"            /* .õõõõõõõ##ööööøÿ */
	$"0000 0000 0000 00FD 0000 0000 0097 2323"            /* .......ý.....—## */
	$"9797 9797 9797 9797 2323 9797 97F6 56FF"            /* ————————##———öVÿ */
	$"0000 0000 0000 00FD 0000 0097 9700 0097"            /* .......ý...——..— */
	$"F500 F5F5 F5F5 F5F6 F597 F6F5 F6F5 56FF"            /* õ.õõõõõöõ—öõöõVÿ */
	$"0000 0000 0000 00FD 0000 0000 00F5 00F5"            /* .......ý.....õ.õ */
	$"00F5 F5F5 F5F5 F5F5 F6F5 F6F6 F6F6 56FF"            /* .õõõõõõõöõööööVÿ */
	$"0000 0000 0000 00FD FDFD FEFE FEFE EAFE"            /* .......ýýýþþþþêþ */
	$"EAE0 EAFE FFFF FFFF FFFF FFFF FFFF FFFF"            /* êàêþÿÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl8' (130, "Sounds Icon") {
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F5F5 00F5 F5F5 F5F5 F5AC FBFC 0000 0000"            /* õõ.õõõõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A082 FAAC FC00 00F5"            /* ..õû¬{|  ‚ú¬ü..õ */
	$"00F5 F5F5 F5F5 F5F6 F5FC F7FB AC00 0000"            /* .õõõõõõöõü÷û¬... */
	$"00F5 ACFE 50A1 A1A1 A7CB AD7A DFAC 0000"            /* .õ¬þP¡¡¡§Ë­zß¬.. */
	$"F500 F5F5 F5F5 F6F5 F6AC F5F7 FBAC 0000"            /* õ.õõõõöõö¬õ÷û¬.. */
	$"0081 FF7A 7D9B 9BA1 A1A7 E7AD FAEA FBF5"            /* .ÿz}››¡¡§ç­úêûõ */
	$"00F5 F5F5 F5F5 F5F5 F5AC F5F6 F7FB AC00"            /* .õõõõõõõõ¬õö÷û¬. */
	$"2BE9 AD57 C57D 9BA1 A1A7 E8E9 F9E8 E92B"            /* +é­WÅ}›¡¡§èéùèé+ */
	$"F5F5 F5F5 F5F5 F5F6 F5AC ACAC ACAC ACFD"            /* õõõõõõõöõ¬¬¬¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADEA FAA6 D1FB"            /* {éüu§¡¡¡§Ë­êú¦Ñû */
	$"00F5 00F5 F5F5 F6F5 F556 FAFA FAFA FAAC"            /* .õ.õõõöõõVúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBAD E8FF FAA7 E8A6"            /* ¦è¦ùË§¡§Ë­èÿú§è¦ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 5656 5656 56FD"            /* .õõõõõõöõõVVVVVý */
	$"A7A7 E8F9 A7E7 A7CB ADE8 EAFD 56E7 A7AD"            /* §§èù§ç§Ë­èêýVç§­ */
	$"F5F5 F5F5 F5F5 F5F5 F5F6 F6F5 F6F6 F8FD"            /* õõõõõõõõõööõööøý */
	$"A7A7 A7A6 56E9 E9D1 D1E0 EA56 A0A7 A7D0"            /* §§§¦VééÑÑàêV §§Ð */
	$"0000 F5F5 F5F5 F6F5 F6F5 F6F6 F5F6 56FD"            /* ..õõõõöõöõööõöVý */
	$"A0A7 E6A7 7B56 FCFD FEFC F87C A7A7 A7A7"            /*  §æ§{Vüýþüø|§§§§ */
	$"00F5 F5F5 F5F5 F5F5 ACF6 F5F6 F6F5 56FD"            /* .õõõõõõõ¬öõööõVý */
	$"57C5 A1A1 A7A0 572C 4F51 A1E6 A1A1 E7FA"            /* WÅ¡¡§ W,OQ¡æ¡¡çú */
	$"F5F5 F5F5 F5F5 F5F5 FFF5 F6F6 F6F6 56FD"            /* õõõõõõõõÿõööööVý */
	$"25A7 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E8F7"            /* %§¡¡¡¡æzQæ¡¡¡¡è÷ */
	$"F500 F82B F5F5 F5F5 FFF6 F5F6 F5F6 56FD"            /* õ.ø+õõõõÿöõöõöVý */
	$"0075 A1A1 A19B A757 51A1 A1A1 9BE7 FA00"            /* .u¡¡¡›§WQ¡¡¡›çú. */
	$"00F5 FDFB F5F5 F52B FFF6 F6F5 F6F6 56FD"            /* .õýûõõõ+ÿööõööVý */
	$"00F5 7CA1 9B7D C557 4AA1 9B77 CBFB 00F5"            /* .õ|¡›}ÅWJ¡›wËû.õ */
	$"F500 FDFB F5F5 F5F7 FEF7 F5F6 F6F6 F8FE"            /* õ.ýûõõõ÷þ÷õöööøþ */
	$"0000 F57B A19B A17A 519B A7E7 FA00 F500"            /* ..õ{¡›¡zQ›§çú.õ. */
	$"F5F5 FEFB F5F5 F5F8 FEF8 F6F5 F6F6 56FD"            /* õõþûõõõøþøöõööVý */
	$"0000 0000 2B7B A6F9 57A6 7BF7 0000 00F5"            /* ....+{¦ùW¦{÷...õ */
	$"00F5 E0AC F5F5 F556 ACF9 F5F6 F6F5 56FD"            /* .õà¬õõõV¬ùõööõVý */
	$"0000 0000 0000 00AC 0000 0000 0056 FBF5"            /* .......¬.....Vûõ */
	$"F52B ACFD F5F5 F5FA FCFA F5F6 F6F6 56FE"            /* õ+¬ýõõõúüúõöööVþ */
	$"0000 0000 0000 00AC 0000 0000 0081 E0F6"            /* .......¬.....àö */
	$"F5F7 FBFF 2BE0 0081 FB81 F6F5 F6F6 F8FE"            /* õ÷ûÿ+à.ûöõööøþ */
	$"0000 0000 0000 00AC 0000 0000 00FD FC56"            /* .......¬.....ýüV */
	$"00F9 81FE F8FF F5FB FAFB 56FD F5F6 56E0"            /* .ùþøÿõûúûVýõöVà */
	$"0000 0000 0000 00AC 0000 81FE 00FE F981"            /* .......¬..þ.þù */
	$"00FA F9AC FAFD F7AC 56AC FAFE F8F6 56FE"            /* .úù¬úý÷¬V¬úþøöVþ */
	$"0000 0000 0000 00AC 0000 FEFE 2BFD 2BFD"            /* .......¬..þþ+ý+ý */
	$"00FC F8FC FBFC F9AC 2BFE FCFA FBF5 56EA"            /* .üøüûüù¬+þüúûõVê */
	$"0000 0000 0000 00AC 00F7 ACAC FAFB 00FE"            /* .......¬.÷¬¬úû.þ */
	$"F6FC F7FB FC81 81EA F5FE FD2B FE2B F8E0"            /* öü÷ûüêõþý+þ+øà */
	$"0000 0000 0000 00AC 0081 56FB FBF9 00FC"            /* .......¬.Vûûù.ü */
	$"F8FE F5FA FEF8 ACE0 F5FE FEF5 81F9 56F4"            /* øþõúþø¬àõþþõùVô */
	$"0000 0000 0000 00FD 0000 00FA FDF8 00F9"            /* .......ý...úýø.ù */
	$"81EA F5F9 FEF6 F4FD F5AC FEF6 2BFB 56EA"            /* êõùþöôýõ¬þö+ûVê */
	$"0000 0000 0000 00AC 0000 0000 56F6 00F7"            /* .......¬....Vö.÷ */
	$"FDAC F556 FEF5 FDAC F5FD ACF5 F6F6 56FF"            /* ý¬õVþõý¬õý¬õööVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F500"            /* .......ý......õ. */
	$"FEFC F5F5 00F5 FCFD F5FC FCF6 F6F6 F8FF"            /* þüõõ.õüýõüüöööøÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F500"            /* .......ý......õ. */
	$"FD81 F5F5 F5F5 FBAC F52B F6F6 F6F5 56FF"            /* ýõõõõû¬õ+öööõVÿ */
	$"0000 0000 0000 00FD 0000 0000 F500 00F5"            /* .......ý....õ..õ */
	$"F600 F5F5 F5F5 2BF7 F6F5 F6F5 F6F6 56FF"            /* ö.õõõõ+÷öõöõööVÿ */
	$"0000 0000 0000 00FD 0000 0000 00F5 0000"            /* .......ý.....õ.. */
	$"F5F5 F5F5 F5F5 F5F5 F5F6 F6F6 F5F6 56FF"            /* õõõõõõõõõöööõöVÿ */
	$"0000 0000 0000 00FD FDFE FDFE FEFE FEFE"            /* .......ýýþýþþþþþ */
	$"FEFE FEFF FEFF FFFF FFFF FFFF FFFF FFFF"            /* þþþÿþÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl8' (131, "Shapes Icon") {
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F8F6 F5F5 F5F5 F5F5 F5AC FBFC 0000 0000"            /* øöõõõõõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A67C FAFD FBF5 0000"            /* ..õû¬{| ¦|úýûõ.. */
	$"56C9 4FF5 F5F5 F5F6 F5FC F7FB AC00 0000"            /* VÉOõõõõöõü÷û¬... */
	$"00F5 ACFE 50A1 9BA1 A1D1 A7F9 EAAC 00F5"            /* .õ¬þP¡›¡¡Ñ§ùê¬.õ */
	$"F8CF A5F6 F5F5 F6F5 F6AC F5F7 FBAC 0000"            /* øÏ¥öõõöõö¬õ÷û¬.. */
	$"0081 FF7A 7DA1 9BA1 A1A7 D1D0 F9FF 81F5"            /* .ÿz}¡›¡¡§ÑÐùÿõ */
	$"F8CA D080 F5F5 F5F5 F5AC F5F6 F7FB AC00"            /* øÊÐ€õõõõõ¬õö÷û¬. */
	$"2BE9 AD57 E477 A1A1 A1A7 E8AD 81AD E92B"            /* +é­Wäw¡¡¡§è­­é+ */
	$"F6FC EA7A F5F6 F5F6 F5AC ACAC ACAC ACFD"            /* öüêzõöõöõ¬¬¬¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADFF 7BA6 E981"            /* {éüu§¡¡¡§Ë­ÿ{¦é */
	$"F5F5 CAFD F5F5 2B32 F656 FAFA FAFA FAAC"            /* õõÊýõõ+2öVúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBAD E8FF F9A7 E8A6"            /* ¦è¦ùË§¡§Ë­èÿù§è¦ */
	$"00F5 2BA6 F8F5 81DD 57F5 5656 5656 56FD"            /* .õ+¦øõÝWõVVVVVý */
	$"A7CB ADF9 A7CB ADCB ADE8 EAFD F9A7 E7AD"            /* §Ë­ù§Ë­Ë­èêýù§ç­ */
	$"F5F5 F5F8 FE81 F9D1 F9F6 F5F6 F5F6 F8FD"            /* õõõøþùÑùöõöõöøý */
	$"A7A7 CB7C 5CD1 D1D1 D1E0 EAF9 7CE7 A7FD"            /* §§Ë|\ÑÑÑÑàêù|ç§ý */
	$"0000 F52B ACFC D1E9 8150 2BF5 F6F6 56FD"            /* ..õ+¬üÑéP+õööVý */
	$"A0A1 A7CB 7B56 FCFD FEFC F87B E6A7 E6A6"            /*  ¡§Ë{Vüýþüø{æ§æ¦ */
	$"00F5 F67B E9AC FCAC A6FA F8F6 F6F5 55FD"            /* .õö{é¬ü¬¦úøööõUý */
	$"57E6 A1A1 A7A0 572C F775 A1A7 A1A1 A781"            /* Wæ¡¡§ W,÷u¡§¡¡§ */
	$"F550 CAEA EAFD 575D FCD1 CAF6 F62B F8FD"            /* õPÊêêýW]üÑÊöö+øý */
	$"2BA1 A1A1 A1A1 E57A 51A7 A1A1 A1A1 D14F"            /* +¡¡¡¡¡åzQ§¡¡¡¡ÑO */
	$"00F8 FFFF D0D1 FDA6 A5CA EAA5 F6F6 56FD"            /* .øÿÿÐÑý¦¥Êê¥ööVý */
	$"007B A1A1 A19B A757 759B A1A1 9BA7 8100"            /* .{¡¡¡›§Wu›¡¡›§. */
	$"F5F5 F97A 81D0 ADFD CACA FD56 F6F5 56FE"            /* õõùzÐ­ýÊÊýVöõVþ */
	$"00F5 7CA1 9B7D E457 50A1 9B9B A7AC 0000"            /* .õ|¡›}äWP¡››§¬.. */
	$"F500 F501 0081 FDEA D1D0 82F6 F6F6 56FD"            /* õ.õ..ýêÑÐ‚öööVý */
	$"0000 007B A19B A17A 4BA1 A1E7 8100 00F5"            /* ...{¡›¡zK¡¡ç..õ */
	$"F500 F5F5 F5F7 E9FE F3E8 C9F6 F6F6 56FD"            /* õ.õõõ÷éþóèÉöööVý */
	$"0000 0000 F77B A657 57A6 FAF7 0000 00F5"            /* ....÷{¦WW¦ú÷...õ */
	$"00F5 F5F5 F5F6 FDAC FDFE A6F6 07F6 F8FD"            /* .õõõõöý¬ýþ¦ö.öøý */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"00F5 F5F5 F5F8 EACA E981 F5F6 F5F6 56E0"            /* .õõõõøêÊéõöõöVà */
	$"0000 0000 0000 00FC 0000 0000 0000 F500"            /* .......ü......õ. */
	$"F5F5 00F6 F881 FF9F D0FD 2BF5 25F6 56FE"            /* õõ.öøÿŸÐý+õ%öVþ */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"00F5 F7A6 82E0 EACA ACFE FAF6 06F6 56FE"            /* .õ÷¦‚àêÊ¬þúö.öVþ */
	$"0000 0000 0000 00AC 0000 0000 F500 00F5"            /* .......¬....õ..õ */
	$"00F6 FAFB FC81 ACA6 FBAD E0F7 F6F6 56FE"            /* .öúûü¬¦û­à÷ööVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 F500"            /* .......¬......õ. */
	$"F556 AD81 2BF5 5656 F5FA FEFB F5F6 56EA"            /* õV­+õVVõúþûõöVê */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"F788 FB56 F5F5 F5F5 F5F6 FB82 F7F6 F8E0"            /* ÷ˆûVõõõõõöû‚÷öøà */
	$"0000 0000 0000 00AC 0000 0000 0000 F500"            /* .......¬......õ. */
	$"2BA6 F7F7 F6F5 F6F5 F6F7 FBFD 2BF6 56EA"            /* +¦÷÷öõöõö÷ûý+öVê */
	$"0000 0000 0000 00FD 0000 0000 F500 00F5"            /* .......ý....õ..õ */
	$"F556 5DF5 F5F5 F5F6 F5F5 F981 F6F6 F8F4"            /* õV]õõõõöõõùööøô */
	$"0000 0000 0000 00AC 0000 0000 00F5 0000"            /* .......¬.....õ.. */
	$"F5F5 81F6 F5F5 F5F5 F6F5 5657 F5F6 56FF"            /* õõöõõõõöõVWõöVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 00F5"            /* .......ý.......õ */
	$"F6F7 FA56 F5F5 F5F6 F5F6 F856 F6F6 56FF"            /* ö÷úVõõõöõöøVööVÿ */
	$"0000 0000 0000 00FD 0000 0000 F500 F500"            /* .......ý....õ.õ. */
	$"56F8 2CF5 F5F5 F5F5 F6F5 5682 F7F6 56FF"            /* Vø,õõõõõöõV‚÷öVÿ */
	$"0000 0000 0000 00FD 0000 0000 00F5 0000"            /* .......ý.....õ.. */
	$"F5F5 F5F5 F5F5 F6F5 F5F6 0756 F9F6 F8FF"            /* õõõõõõöõõö.Vùöøÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F5F5"            /* .......ý......õõ */
	$"00F5 00F5 F5F5 F5F6 F5F5 25F5 F6F6 56FF"            /* .õ.õõõõöõõ%õööVÿ */
	$"0000 0000 0000 00FD FDFD FEFE FEFE FEFE"            /* .......ýýýþþþþþþ */
	$"E0E0 F4E0 F4FF FFFF FFFF FFFF FFFF FFFF"            /* ààôàôÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl8' (132, "Music Icon") {
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F5F5 00F5 F5F5 F5F5 F5AC FBFC 0000 0000"            /* õõ.õõõõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A082 FAAC FC00 00F5"            /* ..õû¬{|  ‚ú¬ü..õ */
	$"00F5 F5F5 F5F5 F5F6 F5FC F7FB AC00 0000"            /* .õõõõõõöõü÷û¬... */
	$"00F5 ACFE 50A1 A1A1 A7CB AD7A DFAC 0000"            /* .õ¬þP¡¡¡§Ë­zß¬.. */
	$"F500 F5F5 F5F5 F6F5 F6AC F5F7 FBAC 0000"            /* õ.õõõõöõö¬õ÷û¬.. */
	$"0081 FF7A 7D9B 9BA1 A1A7 E7AD FAEA FBF5"            /* .ÿz}››¡¡§ç­úêûõ */
	$"00F5 F5F5 F5F5 F5F5 F5AC F5F6 F7FB AC00"            /* .õõõõõõõõ¬õö÷û¬. */
	$"2BE9 AD57 C57D 9BA1 A1A7 E8E9 F9E8 E92B"            /* +é­WÅ}›¡¡§èéùèé+ */
	$"F5F5 F5F5 F5F5 F5F6 F5AC ACAC ACAC ACFD"            /* õõõõõõõöõ¬¬¬¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADEA FAA6 D1FB"            /* {éüu§¡¡¡§Ë­êú¦Ñû */
	$"00F5 00F5 F5F5 F6F5 F556 FAFA FAFA FAAC"            /* .õ.õõõöõõVúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBAD E8FF FAA7 E8A6"            /* ¦è¦ùË§¡§Ë­èÿú§è¦ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 5656 5656 56FD"            /* .õõõõõõöõõVVVVVý */
	$"A7A7 E8F9 A7E7 A7CB ADE8 EAFD 56E7 A7AD"            /* §§èù§ç§Ë­èêýVç§­ */
	$"F5F5 F5F5 F5F5 F5F5 F5F6 F6F5 F6F6 F8FD"            /* õõõõõõõõõööõööøý */
	$"A7A7 A7A6 56E9 E9D1 D1E0 EA56 A0A7 A7D0"            /* §§§¦VééÑÑàêV §§Ð */
	$"0000 F5F5 F5F5 F6F5 F6F5 F6F6 F5F6 56FD"            /* ..õõõõöõöõööõöVý */
	$"A0A7 E6A7 7B56 FCFD FEFC F87C A7A7 A7A7"            /*  §æ§{Vüýþüø|§§§§ */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F6F5 2B01 55FD"            /* .õõõõõõöõööõ+.Uý */
	$"57C5 A1A1 A7A0 572C 4F51 A1E6 A1A1 E7FA"            /* WÅ¡¡§ W,OQ¡æ¡¡çú */
	$"F500 F5F5 F5F5 F5F5 F6F5 F6F6 F6F6 56FD"            /* õ.õõõõõõöõööööVý */
	$"25A7 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E8F7"            /* %§¡¡¡¡æzQæ¡¡¡¡è÷ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 F6F5 F6F6 56FD"            /* .õõõõõõöõõöõööVý */
	$"0075 A1A1 A19B A757 51A1 A1A1 9BE7 FA00"            /* .u¡¡¡›§WQ¡¡¡›çú. */
	$"F5F5 F5F5 FFF5 F5F5 F6F6 F5F6 F6F6 56FD"            /* õõõõÿõõõööõöööVý */
	$"00F5 7CA1 9BA1 A157 4AA1 9B9B A7A6 00F5"            /* .õ|¡›¡¡WJ¡››§¦.õ */
	$"00F5 00F5 FFF5 F5F6 F5F5 F6F6 F5F6 F8FE"            /* .õ.õÿõõöõõööõöøþ */
	$"0000 F57B A19B E5F9 519B A7E7 8100 00F5"            /* ..õ{¡›åùQ›§ç..õ */
	$"F500 F5F5 FFF5 F5F5 F6F6 FFF5 F6F6 56FD"            /* õ.õõÿõõõööÿõööVý */
	$"0000 0000 2B7B A6F9 57A6 7B2B 0000 FFF5"            /* ....+{¦ùW¦{+..ÿõ */
	$"00F5 F5F5 FFF5 F5F6 F5F5 FFF6 F6F5 56FD"            /* .õõõÿõõöõõÿööõVý */
	$"0000 0000 0000 00AC 0000 0000 0000 FF00"            /* .......¬......ÿ. */
	$"F500 F5F5 FFF5 F5F5 F6F6 FFF6 F6F6 56FE"            /* õ.õõÿõõõööÿöööVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 FFF5"            /* .......¬......ÿõ */
	$"00F5 F5F5 FFF5 F5F6 F5F5 FFF5 F6F6 F8FE"            /* .õõõÿõõöõõÿõööøþ */
	$"0000 0000 0000 00AC 0000 0000 00F5 FF00"            /* .......¬.....õÿ. */
	$"F500 F5F5 FFF5 F5F5 F6F6 FFF6 F5F6 56FE"            /* õ.õõÿõõõööÿöõöVþ */
	$"0000 0000 0000 00AC 00F9 FAFA FAF9 FFFA"            /* .......¬.ùúúúùÿú */
	$"FAFA FAF9 FFFA FAFA FAF9 FFF9 FAFA 56FE"            /* úúúùÿúúúúùÿùúúVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 FF00"            /* .......¬......ÿ. */
	$"0000 FFFF FF00 F5F5 F5F6 FFF5 F6F5 56FF"            /* ..ÿÿÿ.õõõöÿõöõVÿ */
	$"0000 0000 0000 00AC 0000 0000 0000 FFF5"            /* .......¬......ÿõ */
	$"00FF FFFF FFF6 F5F5 F6F5 FFF6 F6F6 56FE"            /* .ÿÿÿÿöõõöõÿöööVþ */
	$"0000 0000 0000 00AC 00FA F9FA FAFA FFF9"            /* .......¬.úùúúúÿù */
	$"FAFF FFFF FFF9 FAFA FFFF FFFA F9FA F8FF"            /* úÿÿÿÿùúúÿÿÿúùúøÿ */
	$"0000 0000 0000 00FD 0000 0000 FFFF FF00"            /* .......ý....ÿÿÿ. */
	$"F500 FFFF F5F5 F5FF FFFF FFF5 F6F5 56FF"            /* õ.ÿÿõõõÿÿÿÿõöõVÿ */
	$"0000 0000 0000 00AC 0000 00FF FFFF FF00"            /* .......¬...ÿÿÿÿ. */
	$"F5F5 00F5 F5F5 F5FF FFFF FFF6 F6F6 56FF"            /* õõ.õõõõÿÿÿÿöööVÿ */
	$"0000 0000 0000 00FD 00F9 FAFF FFFF FFFA"            /* .......ý.ùúÿÿÿÿú */
	$"FAF9 FAFA F9FA FAF9 FFFF F9FA F9FA F8FF"            /* úùúúùúúùÿÿùúùúøÿ */
	$"0000 0000 0000 00FD 0000 0000 FFFF 0000"            /* .......ý....ÿÿ.. */
	$"00F5 00F5 F5F5 F5F5 F5F6 F5F6 F6F5 56FF"            /* .õ.õõõõõõöõööõVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F500"            /* .......ý......õ. */
	$"F5F5 F5F5 F5F5 F5F5 F6F5 F6F5 F6F6 56FF"            /* õõõõõõõõöõöõööVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 00F5"            /* .......ý.......õ */
	$"00F5 F5F5 F5F5 F6F5 F5F6 F6F6 F5F6 56FF"            /* .õõõõõöõõöööõöVÿ */
	$"0000 0000 0000 00FD FDFD FEFE FEFE FEFE"            /* .......ýýýþþþþþþ */
	$"FFFE FEFF FEFF FFFF FFFF FFFF FFFF FFFF"            /* ÿþþÿþÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl8' (133, "Saved Game Icon") {
	$"0000 0000 81FB FBFB FBFB FBFB FCFB FCFC"            /* ....ûûûûûûûüûüü */
	$"FCFB FCFC FCFC FC00 0000 0000 0000 0000"            /* üûüüüüü......... */
	$"0000 0000 FB00 0000 0000 0000 0000 F500"            /* ....û.........õ. */
	$"F5F5 00F5 F5F5 FCFC 0000 0000 0000 0000"            /* õõ.õõõüü........ */
	$"0000 0000 8100 0000 0000 F500 00F7 F5F5"            /* .........õ..÷õõ */
	$"F500 F5F5 F5F5 ACFB FC00 0000 0000 0000"            /* õ.õõõõ¬ûü....... */
	$"0000 0000 FB00 0000 0000 0000 00F7 0000"            /* ....û........÷.. */
	$"0000 F7F5 F6F5 FCF7 FBAC 0000 0000 0000"            /* ..÷õöõü÷û¬...... */
	$"0000 0000 FB00 0000 F7F7 F7F7 F72B F7F7"            /* ....û...÷÷÷÷÷+÷÷ */
	$"F7F7 F7F7 F5F6 ACF5 F7FB AC00 0000 0000"            /* ÷÷÷÷õö¬õ÷û¬..... */
	$"0000 0000 FB00 0000 F700 0000 00F7 F5F5"            /* ....û...÷....÷õõ */
	$"0000 F7F5 F5F5 ACF5 F6F7 FBAC 0000 0000"            /* ..÷õõõ¬õö÷û¬.... */
	$"0000 0000 FB00 0000 0000 F500 00F7 00F5"            /* ....û.....õ..÷.õ */
	$"F500 F7F5 F6F5 ACAC ACAC ACAC FD00 0000"            /* õ.÷õöõ¬¬¬¬¬¬ý... */
	$"0000 0000 FB00 0000 0000 00F5 00F7 F5F5"            /* ....û......õ.÷õõ */
	$"F5F5 F7F5 F5F5 56FA FAFA FAFA AC00 0000"            /* õõ÷õõõVúúúúú¬... */
	$"0000 0000 FB00 0000 F7F5 0000 F500 F5F5"            /* ....û...÷õ..õ.õõ */
	$"F5F5 F5F5 F6F5 F500 0056 5656 FD00 0000"            /* õõõõöõõ..VVVý... */
	$"0000 0000 FB00 00F7 F700 00F5 F781 81FA"            /* ....û..÷÷..õ÷ú */
	$"8181 81F8 F5F6 F6F5 F7F7 F6F8 FD00 0000"            /* øõööõ÷÷öøý... */
	$"0000 0000 FB00 0000 0000 F5FB FD7B 7CA0"            /* ....û.....õûý{|  */
	$"A082 FAFD FBF5 F5F6 00F7 F656 FD00 0000"            /*  ‚úýûõõö.÷öVý... */
	$"0000 0000 FC00 0000 0000 ACFE 569B A1A1"            /* ....ü.....¬þV›¡¡ */
	$"A7E7 E7F9 FEAC F6F6 002B F6F8 FD00 0000"            /* §ççùþ¬öö.+öøý... */
	$"0000 0000 FB00 0000 00FB EA57 9BA1 9BA1"            /* ....û....ûêW›¡›¡ */
	$"A1A7 ADFD 7BFF 81F5 F6F7 F656 FD00 0000"            /* ¡§­ý{ÿõö÷öVý... */
	$"0000 0000 FC00 0000 F7E9 AD7A A1A1 77A1"            /* ....ü...÷é­z¡¡w¡ */
	$"A1A7 E7D1 F9AD EAF7 F5F6 F656 FD00 0000"            /* ¡§çÑù­ê÷õööVý... */
	$"0000 0000 FC00 0000 7BE9 A657 E6A1 A1A1"            /* ....ü...{é¦Wæ¡¡¡ */
	$"A7CB ADEA FAA6 D181 F6F5 F656 FD00 0000"            /* §Ë­êú¦ÑöõöVý... */
	$"0000 0000 FC00 0000 A6E7 AC7B A7A1 A7A7"            /* ....ü...¦ç¬{§¡§§ */
	$"E7A7 D1FF F9A7 D1A6 F6F6 F6F8 FE00 0000"            /* ç§Ñÿù§Ñ¦öööøþ... */
	$"0000 0000 FC00 0000 A7A7 E856 ADE7 A7E7"            /* ....ü...§§èV­ç§ç */
	$"A7E9 E0FD F9CB A7AD 00F6 F656 FE00 0000"            /* §éàýùË§­.ööVþ... */
	$"0000 0000 FC00 0000 A6A7 A7A0 56D1 E9E8"            /* ....ü...¦§§ VÑéè */
	$"E0EA FF56 A0A7 A7E8 F5F7 F6F8 FE00 0000"            /* àêÿV §§èõ÷öøþ... */
	$"0000 0000 FC00 0000 A1A7 A7CB 7B56 FCFD"            /* ....ü...¡§§Ë{Vüý */
	$"FEFB 567C A7E6 A7A6 F5F6 F656 FE00 0000"            /* þûV|§æ§¦õööVþ... */
	$"0000 0000 FC00 0000 57E6 A1A1 E67C 7BF7"            /* ....ü...Wæ¡¡æ|{÷ */
	$"2C75 7CE6 A1A1 CB81 F6F6 F6F8 FE00 0000"            /* ,u|æ¡¡Ëöööøþ... */
	$"0000 0000 FC00 0000 2BA1 A1A1 A1A1 E657"            /* ....ü...+¡¡¡¡¡æW */
	$"50E6 A1A1 A1A1 AD2B F6F6 F656 FE00 0000"            /* Pæ¡¡¡¡­+öööVþ... */
	$"0000 0000 AC00 0000 007B A1A1 A1A1 A17A"            /* ....¬....{¡¡¡¡¡z */
	$"51A1 A19B A1CB 8100 00F6 F556 FE00 0000"            /* Q¡¡›¡Ë..öõVþ... */
	$"0000 0000 FC00 0000 0000 7CA1 9B9B A157"            /* ....ü.....|¡››¡W */
	$"519B 9BA1 A7A6 F5F5 F7F7 F656 FE00 0000"            /* Q››¡§¦õõ÷÷öVþ... */
	$"0000 0000 AC00 0000 0000 007B A1A1 A17A"            /* ....¬......{¡¡¡z */
	$"4BA1 A1A7 81F5 F5F6 00F7 F6F8 FF00 0000"            /* K¡¡§õõö.÷öøÿ... */
	$"0000 0000 FC00 0000 00F5 0000 F77B A657"            /* ....ü....õ..÷{¦W */
	$"56A6 81F7 24F6 F5F6 002B F656 FF00 0000"            /* V¦÷$öõö.+öVÿ... */
	$"0000 0000 AC00 0000 0000 F500 0000 F500"            /* ....¬.....õ...õ. */
	$"F5F5 0024 07F5 F6F5 00F7 F6F8 FF00 0000"            /* õõ.$.õöõ.÷öøÿ... */
	$"0000 0000 AC00 0000 F700 0000 00F7 0000"            /* ....¬...÷....÷.. */
	$"0000 F601 24F6 F5F6 00F7 F656 FF00 0000"            /* ..ö.$öõö.÷öVÿ... */
	$"0000 0000 AC00 00F7 F7F5 00F5 F72B F7F7"            /* ....¬..÷÷õ.õ÷+÷÷ */
	$"F7F7 F7F5 06F5 F6F6 F5F7 F6F8 FF00 0000"            /* ÷÷÷õ.õööõ÷öøÿ... */
	$"0000 0000 AC00 0000 F700 F500 00F7 00F5"            /* ....¬...÷.õ..÷.õ */
	$"F500 F7F5 F6F5 F5F6 F6F5 F656 FF00 0000"            /* õ.÷õöõõööõöVÿ... */
	$"0000 0000 AC00 0000 0000 00F5 00F7 F5F5"            /* ....¬......õ.÷õõ */
	$"F5F5 F7F6 F5F5 F6F5 F6F6 F656 FF00 0000"            /* õõ÷öõõöõöööVÿ... */
	$"0000 0000 AC00 0000 0000 F500 F500 F5F5"            /* ....¬.....õ.õ.õõ */
	$"F5F5 F5F5 F5F6 F5F6 F5F6 F656 FF00 0000"            /* õõõõõöõöõööVÿ... */
	$"0000 0000 ACAC FDFD FDFD FDFD FDFE FEFE"            /* ....¬¬ýýýýýýýþþþ */
	$"FDEA E0FF FFFF FFFF FFFF FFFF FF00 0000"            /* ýêàÿÿÿÿÿÿÿÿÿÿ... */
};

data 'icl8' (134, "Film Icon") {
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿ........ */
	$"0000 FFFF FFFF F5F6 F5F5 F5F6 F5F5 F5F6"            /* ..ÿÿÿÿõöõõõöõõõö */
	$"F5F5 F5F6 F5F5 F5FF FF00 0000 0000 0000"            /* õõõöõõõÿÿ....... */
	$"0000 FF00 00FF F5F5 F6F6 F5F5 F6F6 F5F5"            /* ..ÿ..ÿõõööõõööõõ */
	$"F6F6 F5F5 F6F6 F5FF F9FF 0000 0000 0000"            /* ööõõööõÿùÿ...... */
	$"0000 FF00 00FF F5F6 F5F6 F5F6 F5F6 F5F6"            /* ..ÿ..ÿõöõöõöõöõö */
	$"F5F6 F5F6 F5F6 F5FF F6F9 FF00 0000 0000"            /* õöõöõöõÿöùÿ..... */
	$"0000 FFFF FFFF F6F5 F6F5 F6F5 F6F5 F6F5"            /* ..ÿÿÿÿöõöõöõöõöõ */
	$"F6F5 F6F5 F6F5 F6FF F5F6 F9FF 0000 0000"            /* öõöõöõöÿõöùÿ.... */
	$"0000 FFFF FFFF F5F6 F5F6 F5F6 F5F6 F5F6"            /* ..ÿÿÿÿõöõöõöõöõö */
	$"F5F6 F5F6 F5F6 F5FF F6F5 F6F9 FF00 0000"            /* õöõöõöõÿöõöùÿ... */
	$"0000 FF00 00FF F5F5 F6F5 F6F5 F6F5 F6F5"            /* ..ÿ..ÿõõöõöõöõöõ */
	$"F6F5 F6F5 F6F5 F6FF FFFF FFFF FFFF 0000"            /* öõöõöõöÿÿÿÿÿÿÿ.. */
	$"0000 FF00 00FF F5F6 F5F6 F5F6 F5F5 F5F6"            /* ..ÿ..ÿõöõöõöõõõö */
	$"F5F6 F5F6 F5F6 F5F5 F5F5 FF00 00FF 0000"            /* õöõöõöõõõõÿ..ÿ.. */
	$"0000 FFFF FFFF F6F5 F6F5 F6F5 F881 81FA"            /* ..ÿÿÿÿöõöõöõøú */
	$"8181 81F7 F5F5 F6F6 F5F6 FFFF FFFF 0000"            /* ÷õõööõöÿÿÿÿ.. */
	$"0000 FFFF FFFF F5F6 F5F6 00FC AC7B 7BA1"            /* ..ÿÿÿÿõöõö.ü¬{{¡ */
	$"A082 FAAC FCF5 F6F5 F6F5 FFFF FFFF 0000"            /*  ‚ú¬üõöõöõÿÿÿÿ.. */
	$"0000 FF00 00FF F5F5 F6F5 ACFD 51A1 A1A1"            /* ..ÿ..ÿõõöõ¬ýQ¡¡¡ */
	$"A7CB E8F9 FEAC 00F6 F5F6 FF00 00FF 0000"            /* §Ëèùþ¬.öõöÿ..ÿ.. */
	$"0000 FF00 00FF F5F6 F581 FFF9 A09B 9BA1"            /* ..ÿ..ÿõöõÿù ››¡ */
	$"A1A7 A7FD FAEA FBF5 F6F5 FF00 00FF 0000"            /* ¡§§ýúêûõöõÿ..ÿ.. */
	$"0000 FFFF FFFF F6F5 F7E9 AD7B A1A1 77A1"            /* ..ÿÿÿÿöõ÷é­{¡¡w¡ */
	$"A1E7 E8D1 F9AD E9F7 F5F6 FFFF FFFF 0000"            /* ¡çèÑù­é÷õöÿÿÿÿ.. */
	$"0000 FFFF FFFF F5F5 7BD1 FC51 A7A1 A1A1"            /* ..ÿÿÿÿõõ{ÑüQ§¡¡¡ */
	$"E6A7 ADE0 81A6 D181 F6F5 FFFF FFFF 0000"            /* æ§­à¦Ñöõÿÿÿÿ.. */
	$"0000 FF00 00FF F6F5 A6D1 A67B CBA7 A7A7"            /* ..ÿ..ÿöõ¦Ñ¦{Ë§§§ */
	$"A7E7 E8EA F9A7 E8A6 F5F6 FF00 00FF 0000"            /* §çèêù§è¦õöÿ..ÿ.. */
	$"0000 FF00 00FF F5F5 A7A7 E856 ADA7 CBA7"            /* ..ÿ..ÿõõ§§èV­§Ë§ */
	$"E8E8 E0FE 56E7 A7AD F6F5 FF00 00FF 0000"            /* èèàþVç§­öõÿ..ÿ.. */
	$"0000 FFFF FFFF F5F6 A7A7 CB82 7AE9 D1E9"            /* ..ÿÿÿÿõö§§Ë‚zéÑé */
	$"ADFF EA56 A0A7 E7A6 F5F6 FFFF FFFF 0000"            /* ­ÿêV §ç¦õöÿÿÿÿ.. */
	$"0000 FFFF FFFF F6F6 7CCB A1A7 7B56 FCFD"            /* ..ÿÿÿÿöö|Ë¡§{Vüý */
	$"EAFC F87C A7A7 A7AC F6F5 FFFF FFFF 0000"            /* êüø|§§§¬öõÿÿÿÿ.. */
	$"0000 FF00 00FF F5F5 7BA1 A7A1 A1A0 57F7"            /* ..ÿ..ÿõõ{¡§¡¡ W÷ */
	$"F775 A1A1 E5A1 CB7B 06F6 FF00 00FF 0000"            /* ÷u¡¡å¡Ë{.öÿ..ÿ.. */
	$"0000 FF00 00FF F5F6 25A7 A1A1 A1A1 E67A"            /* ..ÿ..ÿõö%§¡¡¡¡æz */
	$"51A7 A1A1 A1A1 ADF7 F6F5 FF00 00FF 0000"            /* Q§¡¡¡¡­÷öõÿ..ÿ.. */
	$"0000 FFFF FFFF F5F6 F575 A1A1 A1A1 A157"            /* ..ÿÿÿÿõöõu¡¡¡¡¡W */
	$"51E3 A1A1 9BE7 81F5 01F6 FFFF FFFF 0000"            /* Qã¡¡›çõ.öÿÿÿÿ.. */
	$"0000 FFFF FFFF F6F5 F6F5 7CA1 9B9B A17B"            /* ..ÿÿÿÿöõöõ|¡››¡{ */
	$"509B 7D9B A7A6 F5F5 F6F5 FFFF FFFF 0000"            /* P›}›§¦õõöõÿÿÿÿ.. */
	$"0000 FF00 00FF F5F6 F5F6 F57B A1A1 A156"            /* ..ÿ..ÿõöõöõ{¡¡¡V */
	$"4BA1 C5A7 8100 F6F5 F6F5 FF00 00FF 0000"            /* K¡Å§.öõöõÿ..ÿ.. */
	$"0000 FF00 00FF F6F5 F6F5 F6F5 F67B A67A"            /* ..ÿ..ÿöõöõöõö{¦z */
	$"57A6 FAF7 F6F6 F5F6 F5F5 FF00 00FF 0000"            /* W¦ú÷ööõöõõÿ..ÿ.. */
	$"0000 FFFF FFFF F5F6 F5F6 F5F6 F601 F5F5"            /* ..ÿÿÿÿõöõöõöö.õõ */
	$"F5F6 F5F5 F5F6 F5F6 F6F5 FFFF FFFF 0000"            /* õöõõõöõööõÿÿÿÿ.. */
	$"0000 FFFF FFFF F6F5 F6F5 F5F6 F5F5 F6F5"            /* ..ÿÿÿÿöõöõõöõõöõ */
	$"F6F5 F5F6 F5F6 F5F6 F5F5 FFFF FFFF 0000"            /* öõõöõöõöõõÿÿÿÿ.. */
	$"0000 FF00 00FF F5F6 F5F6 F5F6 F5F6 F5F6"            /* ..ÿ..ÿõöõöõöõöõö */
	$"F5F6 F5F6 F5F5 F6F5 F6F6 FF00 00FF 0000"            /* õöõöõõöõööÿ..ÿ.. */
	$"0000 FF00 00FF F5F5 F6F5 F6F5 F6F5 F6F5"            /* ..ÿ..ÿõõöõöõöõöõ */
	$"F6F5 F6F5 F6F6 F5F6 F5F5 FF00 00FF 0000"            /* öõöõööõöõõÿ..ÿ.. */
	$"0000 FFFF FFFF F6F5 F5F6 F5F6 F5F6 F5F6"            /* ..ÿÿÿÿöõõöõöõöõö */
	$"F5F6 F5F6 F5F5 F6F5 F6F5 FFFF FFFF 0000"            /* õöõöõõöõöõÿÿÿÿ.. */
	$"0000 FFFF FFFF F5F5 F6F5 F6F5 F6F5 F6F5"            /* ..ÿÿÿÿõõöõöõöõöõ */
	$"F6F5 F6F5 F5F6 F5F6 F5F6 FFFF FFFF 0000"            /* öõöõõöõöõöÿÿÿÿ.. */
	$"0000 FF00 00FF F6F5 F6F5 F5F6 F5F6 F5F6"            /* ..ÿ..ÿöõöõõöõöõö */
	$"F5F6 F5F6 F6F5 F6F5 F6F5 FF00 00FF 0000"            /* õöõööõöõöõÿ..ÿ.. */
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
};

data 'icl8' (135, "Physics File Icon") {
	$"00FB 81FB FB81 FBFB FBFB FBFB FBFC FBFC"            /* .ûûûûûûûûûûüûü */
	$"FBFC FCFC FCFC FCAC FCAC ACAC 0000 0000"            /* ûüüüüüü¬ü¬¬¬.... */
	$"FBF9 F7F5 0000 0000 0000 0000 0000 0000"            /* ûù÷õ............ */
	$"0000 0000 0000 0000 00F5 2BF9 AC00 0000"            /* .........õ+ù¬... */
	$"81F7 F600 0000 0000 0000 0000 00F5 F500"            /* ÷ö..........õõ. */
	$"F5F5 F5F5 F5F5 F5F5 F6F6 F656 AC00 0000"            /* õõõõõõõõöööV¬... */
	$"FBF5 0000 0000 0000 0000 00F5 F500 F5F5"            /* ûõ.........õõ.õõ */
	$"F5F5 F5F5 F5F5 F6F6 F5F6 F556 AC00 0000"            /* õõõõõõööõöõV¬... */
	$"FB00 0000 0000 0000 0000 0000 F5F5 F5F5"            /* û...........õõõõ */
	$"F5F5 F5F6 F5F5 F5F6 F5F6 F6F8 FD00 0000"            /* õõõöõõõöõööøý... */
	$"FB00 0000 0000 0000 0000 00F5 00F5 00F5"            /* û..........õ.õ.õ */
	$"F5F5 F5F5 F6F5 F6F5 F5F6 F556 AC00 0000"            /* õõõõöõöõõöõV¬... */
	$"FB00 0000 0000 0000 0000 0000 F5F5 F5F5"            /* û...........õõõõ */
	$"F5F5 F5F5 F5F6 F5F6 F6F5 F656 FD00 0000"            /* õõõõõöõööõöVý... */
	$"FB00 0000 0000 0000 0000 00F5 F500 F5F5"            /* û..........õõ.õõ */
	$"F5F5 F5F5 F5F5 F5F6 F5F6 F556 AC00 0000"            /* õõõõõõõöõöõV¬... */
	$"FB00 FB00 0000 0000 0000 0000 F881 81FA"            /* û.û.........øú */
	$"8181 81F7 F6F5 F6F5 F6F5 F656 FD00 0000"            /* ÷öõöõöõöVý... */
	$"FBFB 00FC 0000 0000 0000 F5FC AC7B 7B7D"            /* ûû.ü......õü¬{{} */
	$"A0A6 FAFD FBF5 F5F6 F5F6 F6F8 FD00 FD00"            /*  ¦úýûõõöõööøý.ý. */
	$"FB00 00FB 0000 0000 00F5 ACFE 50A1 A1A1"            /* û..û.....õ¬þP¡¡¡ */
	$"A7CB AD7A E0AC F5F5 F6F5 F656 FDFD 00FD"            /* §Ë­zà¬õõöõöVýý.ý */
	$"0000 00FC 0000 0000 0081 FF7A 7D9B 9BA1"            /* ...ü.....ÿz}››¡ */
	$"A1A7 E7AD F9EA FBF6 F5F6 F556 FD00 F8FE"            /* ¡§ç­ùêûöõöõVý.øþ */
	$"0000 00FB 0000 0000 2BE9 AD57 C5A1 77A1"            /* ...û....+é­WÅ¡w¡ */
	$"A1A7 E8E9 FAAD E92B F6F5 F6F5 F7F5 56FD"            /* ¡§èéú­é+öõöõ÷õVý */
	$"0000 00FC 0000 0000 7BE9 FC75 A7A1 A1A1"            /* ...ü....{éüu§¡¡¡ */
	$"A7CB ADEA FAA6 D181 F5F6 F6F6 F5F6 56FE"            /* §Ë­êú¦ÑõöööõöVþ */
	$"0000 00FC 0000 0000 A6E8 A6F9 CBA7 A1CB"            /* ...ü....¦è¦ùË§¡Ë */
	$"A7E8 E8FF FAA7 E8A6 F5F6 F5F6 F6F6 56FD"            /* §èèÿú§è¦õöõöööVý */
	$"0000 00FC 0000 0000 A7A7 E7F9 ADA7 E8A7"            /* ...ü....§§çù­§è§ */
	$"E7AD E0FD 56E7 A7AD F6F5 F6F5 F6F6 F8FE"            /* ç­àýVç§­öõöõööøþ */
	$"0000 00FC 0000 0000 A7A7 A7A0 56E9 D1E9"            /* ...ü....§§§ VéÑé */
	$"E9EA EA56 A0A7 CBFD F5F6 F5F6 F6F5 56FE"            /* éêêV §ËýõöõööõVþ */
	$"0000 00FC 0000 0000 A0A7 E6A7 7B56 FCFD"            /* ...ü.... §æ§{Vüý */
	$"FEFC F87C A7A7 A7A6 F6F5 F6F6 F5F6 56FE"            /* þüø|§§§¦öõööõöVþ */
	$"0000 00FC 0000 0000 57E5 A1A7 E57C 7B2C"            /* ...ü....Wå¡§å|{, */
	$"F775 A1A7 C5A1 CBFA F5F6 F6F5 F6F6 56FE"            /* ÷u¡§Å¡ËúõööõööVþ */
	$"0000 00FC 0000 0000 2BA1 A1A1 A1A1 A77A"            /* ...ü....+¡¡¡¡¡§z */
	$"51A7 A1A1 A1A1 ADF7 F5F6 F5F6 56F6 F8FE"            /* Q§¡¡¡¡­÷õöõöVöøþ */
	$"0000 00AC 0000 0000 007B A1A1 9BA1 C557"            /* ...¬.....{¡¡›¡ÅW */
	$"50E3 A19B A1CB 8101 F6F5 F656 FE56 56FE"            /* Pã¡›¡Ë.öõöVþVVþ */
	$"FC00 00FC 0000 0000 00F5 7C9B A19B A1F9"            /* ü..ü.....õ|›¡›¡ù */
	$"4BA1 77A1 A7A6 00F6 F5F6 F556 FEFF F8FF"            /* K¡w¡§¦.öõöõVþÿøÿ */
	$"ACFC 00AC 0000 0000 0000 F57B A1A1 A1F9"            /* ¬ü.¬......õ{¡¡¡ù */
	$"519B E5A7 81F5 F624 F6F5 F656 FE00 FF00"            /* Q›å§õö$öõöVþ.ÿ. */
	$"FC00 AC00 0000 0000 0000 0000 2B7B A6F9"            /* ü.¬.........+{¦ù */
	$"51A6 FA2B 2506 2407 F5F6 F556 FE00 0000"            /* Q¦ú+%.$.õöõVþ... */
	$"AC00 0000 0000 0000 0000 0000 F500 F5F5"            /* ¬...........õ.õõ */
	$"F5F5 F5F5 F5F5 F6F5 F6F5 F656 E000 0000"            /* õõõõõõöõöõöVà... */
	$"FC00 0000 0000 0000 0000 00F5 00F5 F500"            /* ü..........õ.õõ. */
	$"F5F5 F5F6 F5F5 F5F6 F5F6 F6F8 EA00 0000"            /* õõõöõõõöõööøê... */
	$"AC00 0000 0000 0000 0000 00F5 00F5 F5F5"            /* ¬..........õ.õõõ */
	$"F5F5 F5F5 F5F6 F5F5 F6F5 F656 E000 0000"            /* õõõõõöõõöõöVà... */
	$"AC00 0000 0000 0000 0000 F500 F500 F5F5"            /* ¬.........õ.õ.õõ */
	$"F5F5 F5F5 F5F5 F6F6 F5F6 F6F8 F400 0000"            /* õõõõõõööõööøô... */
	$"ACF5 0000 0000 0000 0000 00F5 00F5 F5F5"            /* ¬õ.........õ.õõõ */
	$"F5F5 F5F6 F5F5 F5F6 F5F6 F556 FF00 0000"            /* õõõöõõõöõöõVÿ... */
	$"ACF7 F600 0000 0000 0000 0000 F5F5 F5F5"            /* ¬÷ö.........õõõõ */
	$"F5F5 F5F5 F6F5 F6F5 F6F5 F656 FF00 0000"            /* õõõõöõöõöõöVÿ... */
	$"ACF9 5656 5656 5656 5656 5656 5656 5656"            /* ¬ùVVVVVVVVVVVVVV */
	$"5656 5656 5656 5656 5656 56F9 FF00 0000"            /* VVVVVVVVVVVùÿ... */
	$"00FD ACFD ACFD FDFD FDFD FDFE FDFE FDFE"            /* .ý¬ý¬ýýýýýýþýþýþ */
	$"FEFE FEFE FEEA E0EA E0FF FFFF 0000 0000"            /* þþþþþêàêàÿÿÿ.... */
};

data 'icl8' (136, "Images Icon") {
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"0000 0000 0000 00F4 FDCA D6AB D0F3 D0D0"            /* .......ôýÊÖ«ÐóÐÐ */
	$"D6F1 ACF1 F2AC F2F2 D0FF FF00 0000 0000"            /* Öñ¬ñò¬òòÐÿÿ..... */
	$"0000 0000 F7FB FB81 FAFB 8181 A7F3 EAA1"            /* ....÷ûûúû§óê¡ */
	$"D6D0 A6D0 ABCA B1CA A6FF F9FF 0000 0000"            /* ÖÐ¦Ð«Ê±Ê¦ÿùÿ.... */
	$"0000 F5FB AC7B 7BA1 A082 FAFD A6D0 ABCB"            /* ..õû¬{{¡ ‚úý¦Ð«Ë */
	$"D0B2 E5B1 F1AC F1D0 A6FF F6F9 FF00 0000"            /* Ð²å±ñ¬ñÐ¦ÿöùÿ... */
	$"00F5 ACFE 51A1 9BA1 A7CB AD7A FEFD D0A7"            /* .õ¬þQ¡›¡§Ë­zþýÐ§ */
	$"F3D0 A6F1 ABD0 B1FC CBFF F5F6 F9FF 0000"            /* óÐ¦ñ«Ð±üËÿõöùÿ.. */
	$"0081 FF7B 7CA1 9BA1 A1A7 E8AD F9EA ACA7"            /* .ÿ{|¡›¡¡§è­ùê¬§ */
	$"E9D6 E5B1 F1A6 D5D0 A6FF F5F5 F6F9 FF00"            /* éÖå±ñ¦ÕÐ¦ÿõõöùÿ. */
	$"2BE9 FD51 E577 A1A1 A1CB ADE9 FAAD E9E6"            /* +éýQåw¡¡¡Ë­éú­éæ */
	$"FED0 A7CF B1D0 B1CA A6FF FFFF FFFF FFFF"            /* þÐ§Ï±Ð±Ê¦ÿÿÿÿÿÿÿ */
	$"7BE9 A67B A7A1 A1A1 A7A7 E7E9 81A6 E9A6"            /* {é¦{§¡¡¡§§çé¦é¦ */
	$"F3F3 A6F2 F2AB F1CF FCD5 D0AC D5AC D0FF"            /* óó¦òò«ñÏüÕÐ¬Õ¬Ðÿ */
	$"ADE7 FC57 CBA7 A1A7 CBAD D1FF F9A7 E8AC"            /* ­çüWË§¡§Ë­Ñÿù§è¬ */
	$"EAE8 F2B1 A6F1 B1F2 A6F1 B1F1 A6D5 A6FF"            /* êèò±¦ñ±ò¦ñ±ñ¦Õ¦ÿ */
	$"E7A7 E756 A7CB ADCB ADE8 EAFD 56E7 A7E8"            /* ç§çV§Ë­Ë­èêýVç§è */
	$"E0F3 F3F1 D5B1 D5B1 D5B1 F1B1 F1B1 D5FF"            /* àóóñÕ±Õ±Õ±ñ±ñ±Õÿ */
	$"A7A7 E782 7AE9 E8E9 D1E0 FEF9 7CCB A7AD"            /* §§ç‚zéèéÑàþù|Ë§­ */
	$"F4F4 D6AB CEB1 D5B1 D5D5 B1F1 B1F1 B1FF"            /* ôôÖ«Î±Õ±ÕÕ±ñ±ñ±ÿ */
	$"A0A7 A7E6 7B56 FCFD FDFC 567B E6A7 E6AC"            /*  §§æ{VüýýüV{æ§æ¬ */
	$"FFF4 EAD6 FDD6 B1CE B1B1 F1D5 D5B1 D5D6"            /* ÿôêÖýÖ±Î±±ñÕÕ±ÕÖ */
	$"7BE5 A1A1 A7A0 572C 4F51 A1A1 A7A1 A7FD"            /* {å¡¡§ W,OQ¡¡§¡§ý */
	$"EAEA FDD0 FDD0 F4B1 CED5 B1F1 B1CF ABFF"            /* êêýÐýÐô±ÎÕ±ñ±Ï«ÿ */
	$"F6A1 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E8EA"            /* ö¡¡¡¡¡æzQæ¡¡¡¡èê */
	$"EAAD E8D1 E9FE F2E9 F1B1 F1B1 CEAB CFFF"            /* ê­èÑéþòéñ±ñ±Î«Ïÿ */
	$"007B A1A1 A19B A757 51A1 A1A1 9BA7 ACEA"            /* .{¡¡¡›§WQ¡¡¡›§¬ê */
	$"E9E8 ADD1 FDEA FDD6 FED5 B1D5 ABCF AAFF"            /* éè­ÑýêýÖþÕ±Õ«Ïªÿ */
	$"0000 7CA1 9BA1 A156 4BA1 9B9B A7D0 EAE9"            /* ..|¡›¡¡VK¡››§Ðêé */
	$"E8AD E7AD E9FE F3F3 FFB1 F1B1 F0AB C9FF"            /* è­ç­éþóóÿ±ñ±ð«Éÿ */
	$"0000 F575 A1A1 A17B 509B A7CB A6E9 E0D1"            /* ..õu¡¡¡{P›§Ë¦éàÑ */
	$"ADE8 D1D1 FEE9 FED6 E0D6 D5F1 B1CF A4FD"            /* ­èÑÑþéþÖàÖÕñ±Ï¤ý */
	$"0000 0000 F775 A6F9 ACA7 ACA6 D1D1 FED1"            /* ....÷u¦ù¬§¬¦ÑÑþÑ */
	$"D1AD E8FD E9EA F4FE EAFE D5B1 CFAB C9FF"            /* Ñ­èýéêôþêþÕ±Ï«Éÿ */
	$"0000 0000 0000 00FF EAFF EAD1 CAE9 D1D1"            /* .......ÿêÿêÑÊéÑÑ */
	$"D1D1 FDE9 E0EA FFF3 FEF3 B1CF AACF ABFF"            /* ÑÑýéàêÿóþó±ÏªÏ«ÿ */
	$"0000 0000 0000 00FF FFF4 EAD1 E7E9 D1AD"            /* .......ÿÿôêÑçéÑ­ */
	$"E8FE E9E9 E9FF F4FE EAFE F1AB CFA4 CFFF"            /* èþéééÿôþêþñ«Ï¤Ïÿ */
	$"0000 0000 0000 00FF FFFF E8CA E7D1 FDD1"            /* .......ÿÿÿèÊçÑýÑ */
	$"D0D1 E9E0 EAFF EAF3 FEEA B1CE ABCF ABFF"            /* ÐÑéàêÿêóþê±Î«Ï«ÿ */
	$"0000 0000 0000 00FF E9E8 CBCA CAAD E9D1"            /* .......ÿéèËÊÊ­éÑ */
	$"E8E8 E0EA FFD0 EAFE EAFE EFAB CFAB C9FF"            /* èèàêÿÐêþêþï«Ï«Éÿ */
	$"0000 0000 0000 00FF CAE6 CAE6 CAD1 D1FD"            /* .......ÿÊæÊæÊÑÑý */
	$"D1E8 CAE8 D0E9 FEE9 FEFE B1C8 A5C8 CFFF"            /* ÑèÊèÐéþéþþ±È¥ÈÏÿ */
	$"0000 0000 0000 00FF CBC4 E5CA CAE8 ADD1"            /* .......ÿËÄåÊÊè­Ñ */
	$"E9AD D1CA E8E9 E9FE E0D6 CE9F C2C9 A5FF"            /* é­ÑÊèééþàÖÎŸÂÉ¥ÿ */
	$"0000 0000 0000 00FF E6E6 CAE6 CBCA E9E9"            /* .......ÿææÊæËÊéé */
	$"D1FE E8CA ADEA FEE9 FEAB 9FC2 989E CFFF"            /* ÑþèÊ­êþéþ«ŸÂ˜žÏÿ */
	$"0000 0000 0000 00FF CAE6 CACB CACA E8AD"            /* .......ÿÊæÊËÊÊè­ */
	$"E9D1 ADCA E9FE E9FE D0CF C274 C2C3 A4FF"            /* éÑ­ÊéþéþÐÏÂtÂÃ¤ÿ */
	$"0000 0000 0000 00FF E6CA CBCA E7CB CAE9"            /* .......ÿæÊËÊçËÊé */
	$"D1E9 D1CA D1E0 FEFE A5C8 99C2 98C9 A4FF"            /* ÑéÑÊÑàþþ¥È™Â˜É¤ÿ */
	$"0000 0000 0000 00FF CAE6 CAE8 D1CA E8E8"            /* .......ÿÊæÊèÑÊèè */
	$"D1FE D1CB FED1 FEF2 C89F 9E98 C3A4 C9FF"            /* ÑþÑËþÑþòÈŸž˜Ã¤Éÿ */
	$"0000 0000 0000 00FF CBCA E7E8 D1D1 D1D1"            /* .......ÿËÊçèÑÑÑÑ */
	$"CAE8 D1D0 E0EA F2B1 A4BC C29E 74C9 AAEA"            /* ÊèÑÐàêò±¤¼ÂžtÉªê */
	$"0000 0000 0000 00FF D0E8 E9E9 E9EA EAE0"            /* .......ÿÐèéééêêà */
	$"D1CA CAD1 EAF3 B1F0 ABC3 989F C2A4 C9FF"            /* ÑÊÊÑêó±ð«Ã˜ŸÂ¤Éÿ */
	$"0000 0000 0000 00FF D1E9 EAEA EAF4 FFE9"            /* .......ÿÑéêêêôÿé */
	$"E8E8 E8D0 EAF2 D5AB C89E C29E C9CF A4FF"            /* èèèÐêòÕ«ÈžÂžÉÏ¤ÿ */
	$"0000 0000 0000 00FF FFFF FFFF F4FF FFFF"            /* .......ÿÿÿÿÿôÿÿÿ */
	$"FFFF E0FF FFFF FFFF FFFF FFFF E0FF FFFF"            /* ÿÿàÿÿÿÿÿÿÿÿÿàÿÿÿ */
};

data 'icl8' (137, "MIDI Music Icon") {
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F5F5 00F5 F5F5 F5F5 F5AC FBFC 0000 0000"            /* õõ.õõõõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A082 FAAC FC00 00F5"            /* ..õû¬{|  ‚ú¬ü..õ */
	$"00F5 F5F5 F5F5 F5F6 F5FC F7FB AC00 0000"            /* .õõõõõõöõü÷û¬... */
	$"00F5 ACFE 50A1 A1A1 A7CB AD7A DFAC 0000"            /* .õ¬þP¡¡¡§Ë­zß¬.. */
	$"F500 F5F5 F5F5 F6F5 F6AC F5F7 FBAC 0000"            /* õ.õõõõöõö¬õ÷û¬.. */
	$"0081 FF7A 7D9B 9BA1 A1A7 E7AD FAEA FBF5"            /* .ÿz}››¡¡§ç­úêûõ */
	$"00F5 F5F5 F5F5 F5F5 F5AC F5F6 F7FB AC00"            /* .õõõõõõõõ¬õö÷û¬. */
	$"2BE9 AD57 C57D 9BA1 A1A7 E8E9 F9E8 E92B"            /* +é­WÅ}›¡¡§èéùèé+ */
	$"F5F5 F5F5 F5F5 F5F6 F5AC ACAC ACAC ACFD"            /* õõõõõõõöõ¬¬¬¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADEA FAA6 D1FB"            /* {éüu§¡¡¡§Ë­êú¦Ñû */
	$"00F5 00F5 F5F5 F6F5 F556 FAFA FAFA FAAC"            /* .õ.õõõöõõVúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBAD E8FF FAA7 E8A6"            /* ¦è¦ùË§¡§Ë­èÿú§è¦ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 5656 5656 56FD"            /* .õõõõõõöõõVVVVVý */
	$"A7A7 E8F9 A7E7 A7CB ADE8 EAFD 56E7 A7AD"            /* §§èù§ç§Ë­èêýVç§­ */
	$"F5F5 F5F5 F5F5 F5F5 F5F6 F6F5 F6F6 F8FD"            /* õõõõõõõõõööõööøý */
	$"A7A7 A7A6 56E9 E9D1 D1E0 EA56 A0A7 A7D0"            /* §§§¦VééÑÑàêV §§Ð */
	$"0000 F5F5 F5F5 F6F5 F6F5 F6F6 F5F6 56FD"            /* ..õõõõöõöõööõöVý */
	$"A0A7 E6A7 7B56 FCFD FEFC F87C A7A7 A7A7"            /*  §æ§{Vüýþüø|§§§§ */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F6F5 2B01 55FD"            /* .õõõõõõöõööõ+.Uý */
	$"57C5 A1A1 A7A0 572C 4F51 A1E6 A1A1 E7FA"            /* WÅ¡¡§ W,OQ¡æ¡¡çú */
	$"F500 F5F5 F5F5 F5F5 F6F5 F6F6 F6F6 56FD"            /* õ.õõõõõõöõööööVý */
	$"25A7 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E8F7"            /* %§¡¡¡¡æzQæ¡¡¡¡è÷ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 F6F5 F6F6 56FD"            /* .õõõõõõöõõöõööVý */
	$"0075 A1A1 A19B A757 51A1 A1A1 9BE7 FA00"            /* .u¡¡¡›§WQ¡¡¡›çú. */
	$"F5F5 F5F5 FFF5 F5F5 F6F6 F5F6 F6F6 56FD"            /* õõõõÿõõõööõöööVý */
	$"00F5 7CA1 9BA1 A157 4AA1 9B9B A7A6 00F5"            /* .õ|¡›¡¡WJ¡››§¦.õ */
	$"00F5 00F5 FFF5 F5F6 F5F5 F6F6 F5F6 F8FE"            /* .õ.õÿõõöõõööõöøþ */
	$"0000 F57B A19B E5F9 519B A7E7 8100 00F5"            /* ..õ{¡›åùQ›§ç..õ */
	$"F500 F5F5 FFF5 F5F5 F6F6 FFF5 F6F6 56FE"            /* õ.õõÿõõõööÿõööVþ */
	$"0000 0000 2B7B A6F9 57A6 7B2B 0000 FFF5"            /* ....+{¦ùW¦{+..ÿõ */
	$"00F5 F5F5 FFF5 F5F6 F5F5 FFF6 F6F5 56FE"            /* .õõõÿõõöõõÿööõVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 FF00"            /* .......¬......ÿ. */
	$"F500 F5F5 FFF5 F5F5 F6F6 FFF6 F6F6 56FE"            /* õ.õõÿõõõööÿöööVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 FFF5"            /* .......¬......ÿõ */
	$"00F5 F5F5 FFF5 F5F6 F5F5 FFF5 F6F6 F8FE"            /* .õõõÿõõöõõÿõööøþ */
	$"0000 0000 0000 00AC 0000 0000 00F5 FF00"            /* .......¬.....õÿ. */
	$"F500 F5F5 FFF5 F5F5 F6F6 FFF6 F5F6 56FE"            /* õ.õõÿõõõööÿöõöVþ */
	$"0000 0000 0000 00AC 00F9 FAFA FAF9 FFFA"            /* .......¬.ùúúúùÿú */
	$"FAFA FAF9 FFFA FAFA FAF9 FFF9 FAFA 56FE"            /* úúúùÿúúúúùÿùúúVþ */
	$"00FD FDFD FDFD FDAC FDAC ACAC ACAC ACFC"            /* .ýýýýýý¬ý¬¬¬¬¬¬ü */
	$"ACFC FCAC FF00 F5F5 F5F6 FFF5 F6F5 56FE"            /* ¬üü¬ÿ.õõõöÿõöõVþ */
	$"00FD 1C1C 1C16 1C1C 151C 1C1C 1C1C 1B1C"            /* .ý.............. */
	$"1C1C 23FC FFF5 F5F5 F6F5 FFF6 F6F6 56E0"            /* ..#üÿõõõöõÿöööVà */
	$"00FD 1B00 2323 2300 2300 2300 0000 0023"            /* .ý..###.#.#....# */
	$"2300 D9FC FFFA FAFA FFFF FFF9 FAF9 56EA"            /* #.ÙüÿúúúÿÿÿùúùVê */
	$"00FD 1C00 0023 0000 2300 2300 2323 2300"            /* .ý...#..#.#.###. */
	$"2300 47FC F5F5 F5FF FFFF FFF6 F5F6 F8F4"            /* #.Güõõõÿÿÿÿöõöøô */
	$"00FD 1600 2300 2300 2300 2300 2323 2300"            /* .ý..#.#.#.#.###. */
	$"2300 D9FC 00F5 F5FF FFFF FFF6 F6F6 56FF"            /* #.Ùü.õõÿÿÿÿöööVÿ */
	$"00AC 1C00 2300 2300 2300 2300 0000 0023"            /* .¬..#.#.#.#....# */
	$"2300 D9FB FAFA FAF9 FFFF F9FA F9FA F8FF"            /* #.Ùûúúúùÿÿùúùúøÿ */
	$"00FD 23D9 47D9 D947 D9D9 47D9 D947 D9D9"            /* .ý#ÙGÙÙGÙÙGÙÙGÙÙ */
	$"47D9 D9FC 00F5 F5F5 F5F6 F5F6 F6F5 56FF"            /* GÙÙü.õõõõöõööõVÿ */
	$"00AC ACAC ACAC ACAC ACAC FCAC FCFC FCFC"            /* .¬¬¬¬¬¬¬¬¬ü¬üüüü */
	$"FCFC FBFC F5F5 F5F5 F6F5 F6F5 F6F6 56FF"            /* üüûüõõõõöõöõööVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 0000"            /* .......ý........ */
	$"0000 F500 F5F5 F6F5 F5F6 F6F6 F5F6 56FF"            /* ..õ.õõöõõöööõöVÿ */
	$"0000 0000 0000 00FD FEFD FDFE FEFE FEFE"            /* .......ýþýýþþþþþ */
	$"FEFF FEFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* þÿþÿÿÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl8' (138, "MML Script Icon") {
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F5F5 00F5 F5F5 F5F5 F5AC FBFC 0000 0000"            /* õõ.õõõõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A082 FAAC FC00 00F5"            /* ..õû¬{|  ‚ú¬ü..õ */
	$"0000 ACAC ACFD 2BF6 F5FC F7FB AC00 0000"            /* ..¬¬¬ý+öõü÷û¬... */
	$"00F5 ACFE 50A1 A1A1 A7CB AD7A E0AC 00F5"            /* .õ¬þP¡¡¡§Ë­zà¬.õ */
	$"F5AC 00F5 F6F7 FD2B F6AC F5F8 FBAC 0000"            /* õ¬.õö÷ý+ö¬õøû¬.. */
	$"0081 FF7A 7D9B 9BA1 A1A7 E7AD FAEA FB00"            /* .ÿz}››¡¡§ç­úêû. */
	$"AC00 F5F6 F856 F9FD 2BAC F5F5 F7FB AC00"            /* ¬.õöøVùý+¬õõ÷û¬. */
	$"2BE9 AD57 C57D 9BA1 A1A7 E8E9 F9AD E9AC"            /* +é­WÅ}›¡¡§èéù­é¬ */
	$"00F5 2BF7 56FA FBAC 56AC ACFD ACAC ACFD"            /* .õ+÷Vúû¬V¬¬ý¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADEA 81A6 D181"            /* {éüu§¡¡¡§Ë­ê¦Ñ */
	$"F52B F756 FA81 ACFA 56F9 FAFA FAFA FAAC"            /* õ+÷Vú¬úVùúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBA7 D1FF FAA6 E8A6"            /* ¦è¦ùË§¡§Ë§Ñÿú¦è¦ */
	$"2BF7 56FA FBFD F956 2BF5 5656 5656 56FD"            /* +÷VúûýùV+õVVVVVý */
	$"A7A7 E8F9 A7CB ADCB ADD1 EAFD F9A7 E7AD"            /* §§èù§Ë­Ë­Ñêýù§ç­ */
	$"F756 FAFB AC81 FDF8 F6F6 F5F6 F5F6 F8FD"            /* ÷Vúû¬ýøööõöõöøý */
	$"A7A7 A7A6 56E9 E8E9 D1E0 EA56 A0A7 A7E8"            /* §§§¦VéèéÑàêV §§è */
	$"F8FA 81FD 81FA 56FD 2BF6 F6F5 F6F6 56FD"            /* øúýúVý+ööõööVý */
	$"A0A7 E6A7 7B56 FCFD FEFC F87C A7E6 A7A6"            /*  §æ§{Vüýþüø|§æ§¦ */
	$"FA81 FD81 FA56 F72B FDF7 F6F6 F6F6 F8FD"            /* úýúV÷+ý÷ööööøý */
	$"57C5 A1A1 A7A0 572C 4F51 A1A7 A1A1 E7FA"            /* WÅ¡¡§ W,OQ¡§¡¡çú */
	$"FBFD 81F9 56F7 2BF6 F6FD F7F6 F6F6 56FD"            /* ûýùV÷+ööý÷öööVý */
	$"2BA1 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E881"            /* +¡¡¡¡¡æzQæ¡¡¡¡è */
	$"FD81 FA56 F72B 2BF6 F5F5 FDF7 F6F5 56FD"            /* ýúV÷++öõõý÷öõVý */
	$"007B A1A1 9BA1 A157 51A1 A19B A1A7 81FD"            /* .{¡¡›¡¡WQ¡¡›¡§ý */
	$"F956 56F7 2B2B F606 F5F5 F5FE F7F6 56FD"            /* ùVV÷++ö.õõõþ÷öVý */
	$"0000 7CA1 A19B A157 509B 9BA1 CBA6 5656"            /* ..|¡¡›¡WP››¡Ë¦VV */
	$"56F8 F72B 2B2A F5F5 24F5 00FE 562B F8FE"            /* Vø÷++*õõ$õ.þV+øþ */
	$"0000 F575 A1A1 C5F9 4BA1 A1A7 8156 5656"            /* ..õu¡¡ÅùK¡¡§VVV */
	$"F8F7 F72B F6F5 F5F5 06F5 F5F6 FEF7 56FE"            /* ø÷÷+öõõõ.õõöþ÷Vþ */
	$"0000 0000 2B7B A6F9 57A6 FAFD F956 56F8"            /* ....+{¦ùW¦úýùVVø */
	$"F7F7 2B2A F6F5 F5F5 F5F5 2AF6 FE56 F9FD"            /* ÷÷+*öõõõõõ*öþVùý */
	$"0000 0000 0000 00AC 0000 00F5 FD56 F8F7"            /* .......¬...õýVø÷ */
	$"F7F6 F6F6 F5F5 F5F5 F5F6 F62B F4F9 F9FE"            /* ÷öööõõõõõöö+ôùùþ */
	$"0000 0000 0000 00AC 0000 0000 F6FD F731"            /* .......¬....öý÷1 */
	$"2BF6 F6F5 F506 F5F5 F6F6 2BF7 FEF9 56FE"            /* +ööõõ.õõöö+÷þùVþ */
	$"0000 0000 0000 00AC 0000 0000 F5F6 EAF6"            /* .......¬....õöêö */
	$"2BF6 0624 F5F5 F5F6 F62B F7F4 F956 F9FE"            /* +ö.$õõõöö+÷ôùVùþ */
	$"0000 0000 0000 00AC 0000 0000 0024 06EA"            /* .......¬.....$.ê */
	$"F6F5 F5F5 F5F5 F6F6 2BF7 FFF9 F9F7 56FE"            /* öõõõõõöö+÷ÿùù÷Vþ */
	$"0000 0000 0000 00AC 0000 0000 00F5 F5F6"            /* .......¬.....õõö */
	$"FEF5 F5F5 F5F6 F62B F7FE FA56 F7F6 56EA"            /* þõõõõöö+÷þúV÷öVê */
	$"0000 0000 0000 00AC 0000 0000 0000 00FE"            /* .......¬.......þ */
	$"81FE F5F5 F5F6 2A2B FEFA 56F7 F6F6 F8E0"            /* þõõõö*+þúV÷ööøà */
	$"0000 0000 0000 00AC 0000 0000 F500 FE81"            /* .......¬....õ.þ */
	$"FAFE F5F5 2AF6 F7FF FA56 2BF6 F6F6 56EA"            /* úþõõ*ö÷ÿúV+öööVê */
	$"0000 0000 0000 00FD 0000 0000 00FE 81FA"            /* .......ý.....þú */
	$"F9F7 F4F6 F62B E0FA 562B F6F6 F6F6 F8F4"            /* ù÷ôöö+àúV+ööööøô */
	$"0000 0000 0000 00AC 0000 0000 00E0 FA56"            /* .......¬.....àúV */
	$"F8F7 FEF6 F7FF F956 2BF6 F6F5 F6F6 56FF"            /* ø÷þö÷ÿùV+ööõööVÿ */
	$"0000 0000 0000 00FD 0000 0000 00FD F9F8"            /* .......ý.....ýùø */
	$"F7F5 F42B FEFA 562B F6F6 F5F6 F6F5 56FF"            /* ÷õô+þúV+ööõööõVÿ */
	$"0000 0000 0000 00FD 0000 0000 F5F5 EAF7"            /* .......ý....õõê÷ */
	$"F6F5 FEFF F956 2BF6 F5F6 F6F5 F6F6 56FF"            /* öõþÿùV+öõööõööVÿ */
	$"0000 0000 0000 00FD 0000 0000 00F5 F6FE"            /* .......ý.....õöþ */
	$"FEF4 FFF9 562B F6F5 F6F5 F5F6 F6F6 F8FF"            /* þôÿùV+öõöõõöööøÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F52B"            /* .......ý......õ+ */
	$"F856 5656 2BF5 F5F6 F5F6 F6F5 F6F6 56FF"            /* øVVV+õõöõööõööVÿ */
	$"0000 0000 0000 00FD FDFD FEFE FEFE FEFE"            /* .......ýýýþþþþþþ */
	$"E0FE EAE0 EAFF FFFF FFFF FFFF FFFF FFFF"            /* àþêàêÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl8' (139, "Text File Icon") {
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F5F5 00F5 F5F5 F5F5 F5AC FBFC 0000 0000"            /* õõ.õõõõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A082 FAAC FC00 00F5"            /* ..õû¬{|  ‚ú¬ü..õ */
	$"00F5 F5F5 F5F5 F5F6 F5FC F7FB AC00 0000"            /* .õõõõõõöõü÷û¬... */
	$"00F5 ACFE 50A1 A1A1 A7CB AD7A DFAC 0000"            /* .õ¬þP¡¡¡§Ë­zß¬.. */
	$"F500 F5F5 F5F5 F6F5 F6AC F5F7 FBAC 0000"            /* õ.õõõõöõö¬õ÷û¬.. */
	$"0081 FF7A 7D9B 9BA1 A1A7 E7AD FAEA FBF5"            /* .ÿz}››¡¡§ç­úêûõ */
	$"00F5 F5F5 F5F5 F5F5 F5AC F5F6 F7FB AC00"            /* .õõõõõõõõ¬õö÷û¬. */
	$"2BE9 AD57 C57D 9BA1 A1A7 E8E9 F9E8 E92B"            /* +é­WÅ}›¡¡§èéùèé+ */
	$"F5F5 F5F5 F5F5 F5F6 F5AC ACAC ACAC ACFD"            /* õõõõõõõöõ¬¬¬¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADEA FAA6 D1FB"            /* {éüu§¡¡¡§Ë­êú¦Ñû */
	$"00F5 00F5 F5F5 F6F5 F556 FAFA FAFA FAAC"            /* .õ.õõõöõõVúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBAD E8FF FAA7 E8A6"            /* ¦è¦ùË§¡§Ë­èÿú§è¦ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 5656 5656 56FD"            /* .õõõõõõöõõVVVVVý */
	$"A7A7 E8F9 A7E7 A7CB ADE8 EAFD 56E7 A7AD"            /* §§èù§ç§Ë­èêýVç§­ */
	$"F5F5 F5F5 F5F5 F5F5 F5F6 F6F5 F6F6 F8FD"            /* õõõõõõõõõööõööøý */
	$"A7A7 A7A6 56E9 E9D1 D1E0 EA56 A0A7 A7D0"            /* §§§¦VééÑÑàêV §§Ð */
	$"88FC 00AC ACAC F5FD ACF5 FDFD F5F6 56FD"            /* ˆü.¬¬¬õý¬õýýõöVý */
	$"A0A7 E6A7 7B56 FCFD FEFC F87C A7A7 CBA6"            /*  §æ§{Vüýþüø|§§Ë¦ */
	$"0000 F5F5 F5F5 F5F5 F5F6 F5F5 F6F6 F8FD"            /* ..õõõõõõõöõõööøý */
	$"57C5 A1A1 A7A0 572C 4F51 A1E6 A1A1 A781"            /* WÅ¡¡§ W,OQ¡æ¡¡§ */
	$"F5F5 F5F5 F5F5 F5F6 F5F6 F6F6 F5F6 56FD"            /* õõõõõõõöõöööõöVý */
	$"25A7 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E82B"            /* %§¡¡¡¡æzQæ¡¡¡¡è+ */
	$"ACAC FD00 ACAC FDFD F5AC FDFD F6F6 56FD"            /* ¬¬ý.¬¬ýýõ¬ýýööVý */
	$"0075 A1A1 A19B A757 51A1 A1A1 9BE7 8100"            /* .u¡¡¡›§WQ¡¡¡›ç. */
	$"0000 00F5 F5F5 F5F5 F5F6 F5F5 F6F6 56FD"            /* ...õõõõõõöõõööVý */
	$"00F5 7CA1 9B7D C557 4AA1 779B E782 0000"            /* .õ|¡›}ÅWJ¡w›ç‚.. */
	$"F5F5 F5F5 F5F5 F5F5 F5F6 F5F6 F6F6 F8FE"            /* õõõõõõõõõöõöööøþ */
	$"0000 F57B A19B A17A 519B E6A7 8100 FDAC"            /* ..õ{¡›¡zQ›æ§.ý¬ */
	$"ACAC FDFD FDF5 FDFD FDF5 FEFD F5F6 56FD"            /* ¬¬ýýýõýýýõþýõöVý */
	$"0000 0000 2B7B A6F9 57A6 FAF7 0000 0000"            /* ....+{¦ùW¦ú÷.... */
	$"00F5 00F5 00F5 F5F5 F5F6 F5F5 F6F6 56FD"            /* .õ.õ.õõõõöõõööVý */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"F500 F5F5 F5F5 F5F6 F5F6 F6F6 F6F5 56E0"            /* õ.õõõõõöõööööõVà */
	$"0000 0000 0000 00AC 0000 0000 0000 00F5"            /* .......¬.......õ */
	$"00F5 F5F5 F5F5 F5F5 F6F5 F5F6 F5F6 56FE"            /* .õõõõõõõöõõöõöVþ */
	$"0000 0000 0000 00AC 0000 0000 F500 F500"            /* .......¬....õ.õ. */
	$"F500 F5F5 F5F5 F6F5 F5F6 F6F6 F6F6 56FE"            /* õ.õõõõöõõöööööVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 00F5"            /* .......¬.......õ */
	$"00F5 F5F5 F5F5 F5F5 F6F5 F6F5 F6F6 F8FE"            /* .õõõõõõõöõöõööøþ */
	$"0000 0000 0000 00AC 0000 0000 F500 F500"            /* .......¬....õ.õ. */
	$"F5F5 00F5 F5F5 F6F5 F5F6 F5F6 F6F6 56EA"            /* õõ.õõõöõõöõöööVê */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F6F5 F6F6 F8E0"            /* .õõõõõõöõööõööøà */
	$"0000 0000 0000 00AC 0000 0000 0000 F500"            /* .......¬......õ. */
	$"F500 F5F5 F5F5 F5F5 F6F5 F6F6 F6F5 56F4"            /* õ.õõõõõõöõöööõVô */
	$"0000 0000 0000 00FD 0000 0000 F500 00F5"            /* .......ý....õ..õ */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F5F6 F5F6 56FF"            /* .õõõõõõöõöõöõöVÿ */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"F500 F5F5 F5F5 F6F5 F6F5 F6F6 F6F6 56FF"            /* õ.õõõõöõöõööööVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F500"            /* .......ý......õ. */
	$"00F5 F5F5 F5F5 F5F5 F5F6 F6F5 F6F6 F8FF"            /* .õõõõõõõõööõööøÿ */
	$"0000 0000 0000 00FD 0000 0000 F500 00F5"            /* .......ý....õ..õ */
	$"F500 F5F5 F5F5 F6F5 F5F6 F5F6 F6F6 56FF"            /* õ.õõõõöõõöõöööVÿ */
	$"0000 0000 0000 00FD 0000 0000 00F5 00F5"            /* .......ý.....õ.õ */
	$"00F5 F5F5 F5F5 F5F5 F6F5 F6F6 F6F5 56FF"            /* .õõõõõõõöõöööõVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F500"            /* .......ý......õ. */
	$"F5F5 F5F5 F5F5 F5F6 F5F6 F5F6 F6F6 56FF"            /* õõõõõõõöõöõöööVÿ */
	$"0000 0000 0000 00FD FDFD FEFE FEFE FEFE"            /* .......ýýýþþþþþþ */
	$"EAFE FEFE FFFF FFFF FFFF FFFF FFFF FFFF"            /* êþþþÿÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl8' (140, "Generic File Icon") {
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F5F5 00F5 F5F5 F5F5 F5AC FBFC 0000 0000"            /* õõ.õõõõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A082 FAAC FC00 00F5"            /* ..õû¬{|  ‚ú¬ü..õ */
	$"00F5 F5F5 F5F5 F5F6 F5FC F7FB AC00 0000"            /* .õõõõõõöõü÷û¬... */
	$"00F5 ACFE 50A1 A1A1 A7CB AD7A DFAC 0000"            /* .õ¬þP¡¡¡§Ë­zß¬.. */
	$"F500 F5F5 F5F5 F6F5 F6AC F5F7 FBAC 0000"            /* õ.õõõõöõö¬õ÷û¬.. */
	$"0081 FF7A 7D9B 9BA1 A1A7 E7AD FAEA FBF5"            /* .ÿz}››¡¡§ç­úêûõ */
	$"00F5 F5F5 F5F5 F5F5 F5AC F5F6 F7FB AC00"            /* .õõõõõõõõ¬õö÷û¬. */
	$"2BE9 AD57 C57D 9BA1 A1A7 E8E9 F9E8 E92B"            /* +é­WÅ}›¡¡§èéùèé+ */
	$"F5F5 F5F5 F5F5 F5F6 F5AC ACAC ACAC ACFD"            /* õõõõõõõöõ¬¬¬¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADEA FAA6 D1FB"            /* {éüu§¡¡¡§Ë­êú¦Ñû */
	$"00F5 00F5 F5F5 F6F5 F556 FAFA FAFA FAAC"            /* .õ.õõõöõõVúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBAD E8FF FAA7 E8A6"            /* ¦è¦ùË§¡§Ë­èÿú§è¦ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 5656 5656 56FD"            /* .õõõõõõöõõVVVVVý */
	$"A7A7 E8F9 A7E7 A7CB ADE8 EAFD 56E7 A7AD"            /* §§èù§ç§Ë­èêýVç§­ */
	$"F5F5 F5F5 F5F5 F5F5 F5F6 F6F5 F6F6 F8FD"            /* õõõõõõõõõööõööøý */
	$"A7A7 A7A6 56E9 E9D1 D1E0 EA56 A0A7 A7D0"            /* §§§¦VééÑÑàêV §§Ð */
	$"0000 F5F5 F5F5 F6F5 F6F5 F6F6 F5F6 56FD"            /* ..õõõõöõöõööõöVý */
	$"A0A7 E6A7 7B56 FCFD FEFC F87C A7A7 A7A7"            /*  §æ§{Vüýþüø|§§§§ */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F6F5 2B01 55FD"            /* .õõõõõõöõööõ+.Uý */
	$"57C5 A1A1 A7A0 572C 4F51 A1E6 A1A1 E7FA"            /* WÅ¡¡§ W,OQ¡æ¡¡çú */
	$"F500 F5F5 F5F5 F5F5 F6F5 F6F6 F6F6 56FD"            /* õ.õõõõõõöõööööVý */
	$"25A7 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E8F7"            /* %§¡¡¡¡æzQæ¡¡¡¡è÷ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 F6F5 F6F6 56FD"            /* .õõõõõõöõõöõööVý */
	$"0075 A1A1 A19B A757 51A1 A1A1 9BE7 FA00"            /* .u¡¡¡›§WQ¡¡¡›çú. */
	$"F5F5 F5F5 F5F5 F5F5 F6F6 F5F6 F6F6 56FD"            /* õõõõõõõõööõöööVý */
	$"00F5 7CA1 9BA1 A157 4AA1 9B9B E782 00F5"            /* .õ|¡›¡¡WJ¡››ç‚.õ */
	$"F500 F5F5 F5F5 F5F6 F5F5 F6F6 F5F6 F8FE"            /* õ.õõõõõöõõööõöøþ */
	$"0000 F57B A19B E5F9 519B A1AD 8100 0000"            /* ..õ{¡›åùQ›¡­... */
	$"F5F5 00F5 F5F5 F6F5 F6F5 F6F6 F6F6 56FF"            /* õõ.õõõöõöõööööVÿ */
	$"0000 0000 2B7B A6F9 51A6 812B 0000 F5F5"            /* ....+{¦ùQ¦+..õõ */
	$"00F5 F5F5 F5F5 F5F5 F5F6 F6F5 F6F6 F8F4"            /* .õõõõõõõõööõööøô */
	$"0000 0000 0000 00AC 0000 0000 0000 00F5"            /* .......¬.......õ */
	$"F500 F5F5 F5F5 F6F5 F6F5 F6F6 F5F6 56FE"            /* õ.õõõõöõöõööõöVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 F500"            /* .......¬......õ. */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F5F6 F6F5 56FE"            /* .õõõõõõöõöõööõVþ */
	$"0000 0000 0000 00AC 0000 0000 F500 00F5"            /* .......¬....õ..õ */
	$"F500 F5F5 F5F5 F5F5 F6F5 F6F5 F6F6 56FE"            /* õ.õõõõõõöõöõööVþ */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F5F6 F6F6 56FE"            /* .õõõõõõöõöõöööVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 F500"            /* .......¬......õ. */
	$"F5F5 00F5 F5F5 F5F5 F6F5 F6F6 F6F5 56FF"            /* õõ.õõõõõöõöööõVÿ */
	$"0000 0000 0000 00AC 0000 0000 F500 00F5"            /* .......¬....õ..õ */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F6F5 F6F6 56FE"            /* .õõõõõõöõööõööVþ */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"F500 F5F5 F5F5 F6F5 F5F5 F6F6 F5F6 56FF"            /* õ.õõõõöõõõööõöVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F500"            /* .......ý......õ. */
	$"00F5 F5F5 F5F5 F5F5 F6F6 F5F6 F6F6 F8FF"            /* .õõõõõõõööõöööøÿ */
	$"0000 0000 0000 00AC 0000 0000 F500 00F5"            /* .......¬....õ..õ */
	$"F500 F5F5 F5F5 F5F6 F5F5 F6F5 F6F6 56FF"            /* õ.õõõõõöõõöõööVÿ */
	$"0000 0000 0000 00FD 0000 0000 00F5 00F5"            /* .......ý.....õ.õ */
	$"00F5 F5F5 F5F5 F6F5 F5F6 F6F6 F6F6 F8FF"            /* .õõõõõöõõöööööøÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F500"            /* .......ý......õ. */
	$"F5F5 00F5 F5F5 F5F6 F5F5 F6F5 F6F6 56FF"            /* õõ.õõõõöõõöõööVÿ */
	$"0000 0000 0000 00FD 0000 0000 F500 00F5"            /* .......ý....õ..õ */
	$"00F5 F5F5 F5F5 F5F5 F6F6 F5F6 F6F5 56FF"            /* .õõõõõõõööõööõVÿ */
	$"0000 0000 0000 00FD 0000 0000 00F5 00F5"            /* .......ý.....õ.õ */
	$"F500 F5F5 F5F5 F6F5 F5F6 F6F6 F6F6 56FF"            /* õ.õõõõöõõöööööVÿ */
	$"0000 0000 0000 00FD FDFD FEFE FEFE FEFE"            /* .......ýýýþþþþþþ */
	$"FEE0 EAE0 F4FF FFFF FFFF FFFF FFFF FFFF"            /* þàêàôÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icl8' (141, "Folder Icon") {
	$"0000 80AA ABAB 0000 0000 0000 0000 0000"            /* ..€ª««.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 8054 7F80 ABAB 0000 0000 0000 0000"            /* ..€T.€««........ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"7F80 AA55 5454 7F80 ABAB 0000 0000 0000"            /* .€ªUTT.€««...... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"802A 55AA AB7E 5454 7F80 ABAB 0000 0000"            /* €*Uª«~TT.€««.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"7F2A 2AF5 54AA AB54 547E 7F80 ABAB 00AB"            /* .**õTª«TT~.€««.« */
	$"ABAB AB00 0000 0000 0000 0000 0000 0000"            /* «««............. */
	$"802A 2A2A 2AF5 54AA AB55 5454 7F80 AB7F"            /* €****õTª«UTT.€«. */
	$"547F 80AB AB00 0000 0000 0000 0000 0000"            /* T.€««........... */
	$"7F2A 2A2A 2A2A 2A2A 54AA AB7E 547E 8054"            /* .*******Tª«~T~€T */
	$"7F54 7E7F 80AB AB00 0000 0000 0000 0000"            /* .T~.€««......... */
	$"802A 2A2A 2A2A 2A2A 2AF5 7E80 AB55 5455"            /* €********õ~€«UTU */
	$"547E 5555 7E7F AB00 0000 0000 0000 0000"            /* T~UU~.«......... */
	$"802A 2A2A 2A2A 302A 2A2A 2A2A 5AAA AB7F"            /* €*****0*****Zª«. */
	$"545B 785B 797F 80AA 0000 0000 0000 0000"            /* T[x[y.€ª........ */
	$"AA2A 2A2A 2A2A 2AF7 2A54 2A2A 2A24 54AA"            /* ª******÷*T***$Tª */
	$"AB7F 795A 785B 7FAB AB00 0000 0000 0000"            /* «.yZx[.««....... */
	$"802A 2A30 4E2A 2A2A 4E2A 314E 302A 5400"            /* €**0N***N*1N0*T. */
	$"7E80 AB7F 5B78 7F80 AB00 0000 0000 0000"            /* ~€«.[x.€«....... */
	$"AA2A 2A2A 2B30 2A4E 302A 2A30 4EF7 2A54"            /* ª***+0*N0**0N÷*T */
	$"2AF5 5AA4 AB7F 557F B200 0000 0000 0000"            /* *õZ¤«.U.²....... */
	$"802A 2A4E 2A2A 2A30 2A4E 2A2A 304E 2A2A"            /* €**N***0*N**0N** */
	$"554E 30F5 78AB 7F7F D000 0000 0000 0000"            /* UN0õx«..Ð....... */
	$"AA2A 302A 2A2A 304E 2A31 4E30 4E2A 3054"            /* ª*0***0N*1N0N*0T */
	$"2A30 4E54 302A AB7F AB00 0000 0000 0000"            /* *0NT0*«.«....... */
	$"802A 4E30 2A4F 2A56 FB81 8181 81FB F84E"            /* €*N0*O*VûûøN */
	$"304E 302A 5455 AB86 D000 0000 0000 0000"            /* 0N0*TU«†Ð....... */
	$"AA2A 2A2A 4E30 FBAC 7A7C 7CA6 7C81 AC87"            /* ª***N0û¬z||¦|¬‡ */
	$"4F30 4F54 2A7F ABA3 B200 0000 0000 0000"            /* O0OT*.«£²....... */
	$"8054 2B2A 2BAC FD51 A19B A1A1 E8A7 80D1"            /* €T+*+¬ýQ¡›¡¡è§€Ñ */
	$"FC30 542A 5554 FD7F D000 0000 0000 0000"            /* ü0T*UTý.Ð....... */
	$"AA2A 2A2A 81FF 57A0 A19B A1A1 A7E8 ADFA"            /* ª***ÿW ¡›¡¡§è­ú */
	$"F4A6 2A54 2A7F ABA4 B100 0000 0000 0000"            /* ô¦*T*.«¤±....... */
	$"AA2A 3055 D1FD 75A1 9BA1 A1A1 E7AD D1F9"            /* ª*0UÑýu¡›¡¡¡ç­Ñù */
	$"ADE9 5554 5455 AB86 D000 0000 0000 0000"            /* ­éUTTU«†Ð....... */
	$"AB2A 2A7B E9A6 57A7 A1A1 A1A7 A7E7 E980"            /* «**{é¦W§¡¡¡§§çé€ */
	$"A6EA 7B54 2A7F ABAA AC00 0000 0000 0000"            /* ¦ê{T*.«ª¬....... */
	$"807F 54A6 E8AC 7BCB A7A1 A7CB ADD1 E056"            /* €.T¦è¬{Ë§¡§Ë­ÑàV */
	$"A6E8 AC2A 5455 ABAA ABFE FDFD 0000 0000"            /* ¦è¬*TU«ª«þýý.... */
	$"ABAB AAA7 E7A7 56A6 E7A7 E7AD E8E9 FDF9"            /* ««ª§ç§V¦ç§ç­èéýù */
	$"E7A7 AD54 547F AB80 FDFD FDFD FDFD 0000"            /* ç§­TT.«€ýýýýýý.. */
	$"0000 ABA7 A7E7 82F9 FED1 D1D1 E0E0 56A0"            /* ..«§§ç‚ùþÑÑÑààV  */
	$"A7CB A754 3079 B1A4 B1AD D0B2 FDFD FDFD"            /* §Ë§T0y±¤±­Ð²ýýýý */
	$"0000 007C E6A7 A79F F9FC FDFE FBF9 9FA7"            /* ...|æ§§ŸùüýþûùŸ§ */
	$"A1A7 AC54 5455 AB86 ACD6 FDAD FDFD FDFD"            /* ¡§¬TTU«†¬Öý­ýýýý */
	$"0000 007B A1A1 A1A7 A07B F84F 7B7C A1A7"            /* ...{¡¡¡§ {øO{|¡§ */
	$"A1CB 7B54 547F ABA4 ABFD FDFD FDFD FDFD"            /* ¡Ë{TT.«¤«ýýýýýýý */
	$"0000 00F6 A1A1 A1A1 A1E6 F951 E5A1 A1A1"            /* ...ö¡¡¡¡¡æùQå¡¡¡ */
	$"A1E7 5530 5455 ABA4 B2FD FDFD FDFD FDFD"            /* ¡çU0TU«¤²ýýýýýýý */
	$"0000 0000 57E4 A1A1 9BA7 F951 A1A1 A19B"            /* ....Wä¡¡›§ùQ¡¡¡› */
	$"E781 544E 557E AB86 D0FD FDFD FDFD FDFD"            /* çTNU~«†Ðýýýýýýý */
	$"0000 0000 007C A177 A1A1 5150 9B9B 9BA7"            /* .....|¡w¡¡QP›››§ */
	$"FB4F 3054 545B ABA4 ABFD FDFD FDFD FD00"            /* ûO0TT[«¤«ýýýýýý. */
	$"0000 0000 0000 7BA1 9BA1 564A A1A1 A7AC"            /* ......{¡›¡VJ¡¡§¬ */
	$"805B 7855 5455 AB86 D0B2 FDFD FDFD 0000"            /* €[xUTU«†Ð²ýýýý.. */
	$"0000 0000 0000 002B 7BA6 50F8 A681 F700"            /* .......+{¦Pø¦÷. */
	$"ABAB 867F 557F ABA4 B1AD FDFD FD00 0000"            /* ««†.U.«¤±­ýýý... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 CFAB AA7F AB86 D0FD FDFD 0000 0000"            /* ..Ï«ª.«†Ðýýý.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 ABAB D0AC FDFD FD00 0000 0000"            /* ....««Ð¬ýýý..... */
};

data 'icns' (-16455, "Application Icon") {
	$"6963 6E73 0000 140E 4943 4E23 0000 0108"            /* icns....ICN#.... */
	$"000F F000 007C 1E00 01E3 E780 039F F9C0"            /* ..ð..|...ãç€.ŸùÀ */
	$"073F FDE0 0F7F FE70 1EFF FF78 3CFF FF7C"            /* .?ýà..þp.ÿÿx<ÿÿ| */
	$"3DFF FFBC 7DFF FFBE 7DFF FFBE 7DFF FFBE"            /* =ÿÿ¼}ÿÿ¾}ÿÿ¾}ÿÿ¾ */
	$"FDFF FFBF FDFF FFBF FCFF FF3F FEFF FF7F"            /* ýÿÿ¿ýÿÿ¿üÿÿ?þÿÿ. */
	$"FE7F FE7F FF3F FCFF FF9F F9FF 7FC7 E3FF"            /* þ.þ.ÿ?üÿÿŸùÿ.Çãÿ */
	$"7FF0 0FFE 7FFE 7FFE 3FFE 7FFE 3FFE 7FFC"            /* .ð.þ.þ.þ?þ.þ?þ.ü */
	$"1FFE 7FFC 1FFE 7FF8 0FFE 7FF0 07FE 7FE0"            /* .þ.ü.þ.ø.þ.ð.þ.à */
	$"03FE 7FC0 00FE 7F80 007E 7E00 000E 7000"            /* .þ.À.þ.€.~~...p. */
	$"003F FC00 00FF FF00 01FF FF80 07FF FFE0"            /* .?ü..ÿÿ..ÿÿ€.ÿÿà */
	$"0FFF FFF0 1FFF FFF8 1FFF FFF8 3FFF FFFC"            /* .ÿÿð.ÿÿø.ÿÿø?ÿÿü */
	$"7FFF FFFE 7FFF FFFE 7FFF FFBF FFFF FFBF"            /* .ÿÿþ.ÿÿþ.ÿÿ¿ÿÿÿ¿ */
	$"FFFF FFBF FFFF FFFF FFFF FFFF FEFF FF7F"            /* ÿÿÿ¿ÿÿÿÿÿÿÿÿþÿÿ. */
	$"FFFF FFFF FF7F FFFF FFBF FDFF FFFF F7FF"            /* ÿÿÿÿÿ.ÿÿÿ¿ýÿÿÿ÷ÿ */
	$"FFFB DFFF 7FFE 7FFF 7FFF FFFE 7FFF FFFE"            /* ÿûßÿ.þ.ÿ.ÿÿþ.ÿÿþ */
	$"3FFF FFFC 1FFF FFF8 1FFF FFF8 0FFF 7FF0"            /* ?ÿÿü.ÿÿø.ÿÿø.ÿ.ð */
	$"07FF 7FE0 01FF 7F80 00FF FF00 003E FC00"            /* .ÿ.à.ÿ.€.ÿÿ..>ü. */
	$"6963 6C34 0000 0208 0000 0000 00CD EEFA"            /* icl4.........Íîú */
	$"FFEE D000 0000 0000 0000 0000 CEFF DDCC"            /* ÿîÐ.........ÎÿÝÌ */
	$"CCDE FFEC 0000 0000 0000 000E FFDC CD8E"            /* ÌÞÿì........ÿÜÍŽ */
	$"8EDC CEFF E000 0000 0000 0CFF FCC8 9888"            /* ŽÜÎÿà......ÿüÈ˜ˆ */
	$"A899 ECDF FFC0 0000 0000 CFFF CC88 8A88"            /* ¨™ìßÿÀ....ÏÿÌˆŠˆ */
	$"99A8 9ACD FFFC 0000 000C FFFD C88B 88A8"            /* ™¨šÍÿü....ÿýÈ‹ˆ¨ */
	$"B98A 9F9C DF9F C000 0009 FFEC 8888 8188"            /* ¹ŠŸœßŸÀ..Æÿìˆˆˆ */
	$"8A89 A9FE C9FF E000 00EF 9FDC A8B8 8889"            /* Š‰©þÉÿà..ïŸÜ¨¸ˆ‰ */
	$"88A9 9A9F CBF9 FE00 0CAF F9C8 888B 8888"            /* ˆ©šŸËùþ..¯ùÈˆ‹ˆˆ */
	$"B89A 99FF DCFA FFC0 0EF9 FA0A 8A88 188B"            /* ¸š™ÿÜúÿÀ.ùúÂŠˆ.‹ */
	$"8999 9A9F AC99 F9E0 C99A 9EC8 9888 88B8"            /* ‰™šŸ¬™ùàÉšžÈ˜ˆˆ¸ */
	$"9A8A 9FAF F0AF 9AF0 C9A9 FEC8 A8B9 8B98"            /* šŠŸ¯ð¯šðÉ©þÈ¨¹‹˜ */
	$"98A9 9F9F F099 9F9D BF99 A90A 8A89 A88A"            /* ˜©ŸŸð™Ÿ¿™©ÂŠ‰¨Š */
	$"9A99 A9FF ECE9 A9FD 999A 9AC8 F89A 89A8"            /* š™©ÿìé©ý™ššÈøš‰¨ */
	$"A999 FAFF EC9A 99A9 9A99 9FCD 9A98 A999"            /* ©™úÿìš™©š™ŸÍš˜©™ */
	$"9A9A 9FFF CC99 A99F A999 A9E0 A99A 99A9"            /* ššŸÿÌ™©Ÿ©™©à©š™© */
	$"A9AF FFFE C8A9 99FA 8A9A 899C DF9F 9A9A"            /* ©¯ÿþÈ©™úŠš‰œßŸšš */
	$"F9F9 FFFC C99A 8A9F A8A8 99A9 0DFF A9FF"            /* ùùÿüÉšŠŸ¨¨™©.ÿ©ÿ */
	$"9FFF FFD0 99A8 99A9 8A89 A89A 80CF FF9F"            /* ŸÿÿÐ™¨™©Š‰¨š€ÏÿŸ */
	$"FFFF EC09 A899 A89A B899 8A98 A9C0 DEFF"            /* ÿÿìÆ¨™¨š¸™Š˜©ÀÞÿ */
	$"AFEC CC89 898A 88FE C9A8 88A8 899B C0C0"            /* ¯ìÌ‰‰ŠˆþÉ¨ˆ¨‰›ÀÀ */
	$"C0CC 8A8A 8A88 A99C C88A 8A89 8A88 88E0"            /* ÀÌŠŠŠˆ©œÈŠŠ‰Šˆˆà */
	$"CB88 88A8 88A9 89AC 0B88 A888 A8A8 999C"            /* Ëˆˆ¨ˆ©‰¬.ˆ¨ˆ¨¨™œ */
	$"089A 888A 888B 9AE0 0C98 888B 888A 88A0"            /* .šˆŠˆ‹šà.˜ˆ‹ˆŠˆ  */
	$"C888 8A88 8B88 9FC0 00B8 8B88 8888 B8F0"            /* ÈˆŠˆ‹ˆŸÀ.¸‹ˆˆˆ¸ð */
	$"C8B8 88B8 88A8 AE00 0008 A888 8B88 8A9C"            /* È¸ˆ¸ˆ¨®...¨ˆ‹ˆŠœ */
	$"0888 B888 B88F 9000 000C 88B8 88B8 88A0"            /* .ˆ¸ˆ¸...ˆ¸ˆ¸ˆ  */
	$"C888 8888 8A9A C000 0000 CA88 8818 889C"            /* ÈˆˆˆŠšÀ...Êˆˆ.ˆœ */
	$"08B8 8B8B 89AC 0000 0000 0C8B 8888 B890"            /* .¸‹‹‰¬.....‹ˆˆ¸ */
	$"C881 8888 FEC0 0000 0000 000B 8A88 88AC"            /* ÈˆˆþÀ......Šˆˆ¬ */
	$"0888 88AF DC00 0000 0000 0000 CB9A 8990"            /* .ˆˆ¯Ü.......Ëš‰ */
	$"C8A9 AFEC 0000 0000 0000 0000 000C BEAC"            /* È©¯ì..........¾¬ */
	$"CA9D DC00 0000 0000 6963 6C38 0000 0408"            /* ÊÜ.....icl8.... */
	$"0000 0000 0000 0000 0000 F656 FCFD FEFD"            /* ..........öVüýþý */
	$"FEFE FDFB 56F6 0000 0000 0000 0000 0000"            /* þþýûVö.......... */
	$"0000 0000 0000 0000 F7FB FFFF FB56 F72B"            /* ........÷ûÿÿûV÷+ */
	$"2B2B 56FC FFFF FCF7 0000 0000 0000 0000"            /* ++Vüÿÿü÷........ */
	$"0000 0000 0000 F5FB FFFF 812B 5075 7CA7"            /* ......õûÿÿ+Pu|§ */
	$"A7A6 7BF7 F7FB FFFF FBF5 0000 0000 0000"            /* §¦{÷÷ûÿÿûõ...... */
	$"0000 0000 002B FDFF FF56 257C A1A7 E6A1"            /* .....+ýÿÿV%|¡§æ¡ */
	$"A7CB A7E8 A62B F9FF FFFD F700 0000 0000"            /* §Ë§è¦+ùÿÿý÷..... */
	$"0000 0000 F7EA FFFF F82C E5A1 A1A1 A1A1"            /* ....÷êÿÿø,å¡¡¡¡¡ */
	$"A1A7 E7A7 D1AD F7F9 EAFF FEF7 0000 0000"            /* ¡§ç§Ñ­÷ùêÿþ÷.... */
	$"0000 002B E9FF FFFA 25A1 A19B A19B A1A1"            /* ...+éÿÿú%¡¡›¡›¡¡ */
	$"A7A1 A7E8 A7D1 D12B 81EA EAEA 2B00 0000"            /* §¡§è§ÑÑ+êêê+... */
	$"0000 F5AD EAEA FDF6 A1A1 9B7D 9B9B A1A1"            /* ..õ­êêýö¡¡›}››¡¡ */
	$"A1A7 A7A7 E7AD E9A6 F6E0 E9EA FDF5 0000"            /* ¡§§§ç­é¦öàéêýõ.. */
	$"0000 81EA E9FF F950 A1A1 A19B 9B77 A19B"            /* ..êéÿùP¡¡¡››w¡› */
	$"A1A7 A1E7 A7E8 D1FF F87B E9E9 EAFB 0000"            /* ¡§¡ç§èÑÿø{ééêû.. */
	$"00F7 D1E9 E9E9 2B7C A1A1 A19B 7D9B A1A1"            /* .÷Ñééé+|¡¡¡›}›¡¡ */
	$"A1A1 E7A7 E8AD D1E0 81F7 E9AD E9E9 2B00"            /* ¡¡ç§è­Ñà÷é­éé+. */
	$"00FB E9AD E9AC F6A1 A7A1 77A1 9BA1 A1A1"            /* .ûé­é¬ö¡§¡w¡›¡¡¡ */
	$"A1E6 A7A7 A7E8 ADE9 FDF6 ADD1 D1E9 FC00"            /* ¡æ§§§è­éýö­ÑÑéü. */
	$"F6E7 ADE8 E9A6 F6A7 E5A1 A1A1 A19B A1A1"            /* öç­èé¦ö§å¡¡¡¡›¡¡ */
	$"A7A7 A7E7 E8AD D1EA FEF5 E8AD E8AD E9F6"            /* §§§çè­Ñêþõè­è­éö */
	$"50D1 E8AD D1FC 2BA7 A7A1 A7A1 A1A7 A1A7"            /* PÑè­Ñü+§§¡§¡¡§¡§ */
	$"A1A7 CBA7 ADE8 E9E9 E0F6 A6E8 ADD1 E956"            /* ¡§Ë§­èééàö¦è­ÑéV */
	$"FAE8 ADE7 D1A6 07CB A7A7 A1A7 A1A1 A7A7"            /* úè­çÑ¦.Ë§§¡§¡¡§§ */
	$"CBA7 E7E8 ADD1 E9E0 FDF5 ADE7 E7AD D181"            /* Ë§çè­Ñéàýõ­çç­Ñ */
	$"A6A7 E7A7 D1AD 2582 E7A7 CBA7 CBA7 A7CB"            /* ¦§ç§Ñ­%‚ç§Ë§Ë§§Ë */
	$"A7E7 A7AD D1AD E9FF FBF7 A7E7 ADA7 E8AC"            /* §ç§­Ñ­éÿû÷§ç­§è¬ */
	$"A7E8 A7E8 A7E9 F856 E8E7 A7A7 A7CB A7A7"            /* §è§è§éøVèç§§§Ë§§ */
	$"E8AD E8E8 E9E0 EAFF F850 E8A7 E7E7 ADD1"            /* è­èèéàêÿøPè§çç­Ñ */
	$"A7A7 E7A7 A7E8 8224 ADD1 A7E7 E8A7 E8E7"            /* §§ç§§è‚$­Ñ§çè§èç */
	$"ADE8 D1AD E9E9 FFFD F5A6 A7E7 A7A7 E8AD"            /* ­èÑ­ééÿýõ¦§ç§§è­ */
	$"A7A7 A7E7 A7CB E82C F7E9 D1AD E8AD E8AD"            /* §§§ç§Ëè,÷éÑ­è­è­ */
	$"E8AD E9EA E0EA FF2B 50E7 A7A7 E7A7 ADD1"            /* è­éêàêÿ+Pç§§ç§­Ñ */
	$"A6CB A7A7 CBA7 A7A6 F6F9 EAD1 ADD1 ADD1"            /* ¦Ë§§Ë§§¦öùêÑ­Ñ­Ñ */
	$"E9D1 E0EA FFFF F825 A7A7 A7CB A7A7 CBFE"            /* éÑàêÿÿø%§§§Ë§§Ëþ */
	$"A0A7 A7A7 A1CB A7E7 A001 F8FE EAEA EAE0"            /*  §§§¡Ë§ç .øþêêêà */
	$"E9FF EAFF FDF7 F6A7 E6A7 A7A1 A7E6 ADA6"            /* éÿêÿý÷ö§æ§§¡§æ­¦ */
	$"57CB A1E5 A7A7 A1A7 A7A6 F7F5 F9AC FEE9"            /* WË¡å§§¡§§¦÷õù¬þé */
	$"E0FE ACF9 F52C A1A7 A1A7 A1CB A1A7 D181"            /* àþ¬ùõ,¡§¡§¡Ë¡§Ñ */
	$"50A7 A7A1 A1A7 E5A1 A7E5 A77C 2BF5 F6F6"            /* P§§¡¡§å¡§å§|+õöö */
	$"F6F5 F52C A0A1 CBA1 A7A1 A7A1 A7E6 E956"            /* öõõ, ¡Ë¡§¡§¡§æéV */
	$"F6A1 C5A1 A7A1 A1A7 A1A1 A1A1 E6A1 82F5"            /* ö¡Å¡§¡¡§¡¡¡¡æ¡‚õ */
	$"247C A1E5 A1A7 A1A7 A1E5 A1A7 A1A7 E9F6"            /* $|¡å¡§¡§¡å¡§¡§éö */
	$"007B A1A1 A1A1 A1A1 A1A1 A1A1 A1CB E8F6"            /* .{¡¡¡¡¡¡¡¡¡¡¡Ëèö */
	$"2CA1 A1A1 A1A1 A1A1 A1A1 A1A1 CBD1 FB00"            /* ,¡¡¡¡¡¡¡¡¡¡¡ËÑû. */
	$"002B A7A1 A1A1 A1A1 A1A1 A1A1 A1A1 ADF5"            /* .+§¡¡¡¡¡¡¡¡¡¡¡­õ */
	$"F6A1 A1A1 A1A1 A1A1 A1A1 A1A1 A7E0 2B00"            /* ö¡¡¡¡¡¡¡¡¡¡¡§à+. */
	$"0000 7BA1 A1A1 A1A1 A1A1 A1A1 A1A7 E7F6"            /* ..{¡¡¡¡¡¡¡¡¡¡§çö */
	$"25A1 A1A1 A1A1 A1A1 A1A1 A1A7 D181 0000"            /* %¡¡¡¡¡¡¡¡¡¡§Ñ.. */
	$"0000 01A0 A1A1 9BA1 9BA1 A19B A1A1 ADF6"            /* ... ¡¡›¡›¡¡›¡¡­ö */
	$"25A1 A1A1 9BA1 9BA1 9BA1 A1E8 ADF5 0000"            /* %¡¡¡›¡›¡›¡¡è­õ.. */
	$"0000 002B A1A1 A19B 7D9B A1A1 9BA1 E706"            /* ...+¡¡¡›}›¡¡›¡ç. */
	$"26A1 9B77 A19B 7D9B A1A1 E8FD 2B00 0000"            /* &¡›w¡›}›¡¡èý+... */
	$"0000 0000 2BA1 A1A1 9BA1 9B77 A1A1 AD25"            /* ....+¡¡¡›¡›w¡¡­% */
	$"F6A1 9BA1 9BA1 9BA1 A1E8 FEF7 0000 0000"            /* ö¡›¡›¡›¡¡èþ÷.... */
	$"0000 0000 002B A0A1 A19B 7D9B 9BA1 A72A"            /* .....+ ¡¡›}››¡§* */
	$"269B 7D9B 7D9B A1A7 D1AC 2B00 0000 0000"            /* &›}›}›¡§Ñ¬+..... */
	$"0000 0000 0000 077B A7A1 A19B A1A1 E707"            /* .......{§¡¡›¡¡ç. */
	$"F5A1 9BA1 A1A7 E8E9 81F5 0000 0000 0000"            /* õ¡›¡¡§èéõ...... */
	$"0000 0000 0000 0000 F7FB A7CB A1A7 E8F5"            /* ........÷û§Ë¡§èõ */
	$"2CA1 A7CB E8AD 81F7 0000 0000 0000 0000"            /* ,¡§Ëè­÷........ */
	$"0000 0000 0000 0000 0000 F650 81A6 FDF6"            /* ..........öP¦ýö */
	$"F6D1 A681 F82A 0000 0000 0000 0000 0000"            /* öÑ¦ø*.......... */
	$"696C 3332 0000 08E6 87FF 0BD8 944F 2A18"            /* il32...æ‡ÿ.Ø”O*. */
	$"1B1A 1728 5094 D98F FF0F BA50 0003 5597"            /* ...(P”Ùÿ.ºP..U— */
	$"BDC8 C6BA 924C 0000 51BA 8BFF 13E3 5E00"            /* ½ÈÆº’L..Qº‹ÿ.ã^. */
	$"0060 C5AB 7245 3835 406C ACBE 5000 005E"            /* .`Å«rE85@l¬¾P..^ */
	$"E388 FF15 C227 0007 9EC9 5526 262A 2824"            /* ãˆÿ.Â'..žÉU&&*($ */
	$"1E13 1049 CC8A 0000 26C1 86FF 17B7 1200"            /* ...IÌŠ..&Á†ÿ.·.. */
	$"02A4 B430 2D36 3634 302C 2722 1D0C 16B9"            /* .¤´0-6640,'"...¹ */
	$"8D00 0011 B784 FF19 C413 0000 79C7 2E33"            /* ...·„ÿ.Ä...yÇ.3 */
	$"3B3C 3B38 342F 2924 201B 0C12 CC5F 0000"            /* ;<;84/)$ ...Ì_.. */
	$"11C3 82FF 1BE7 2900 0323 D74D 2E3B 3E3F"            /* .Ã‚ÿ.ç)..#×M.;>? */
	$"3F3B 3731 2B26 201C 1703 41CF 1507 0026"            /* ?;71+& ...AÏ...& */
	$"E681 FF1B 6400 0B00 86AA 2337 3C3F 403F"            /* æÿ.d...†ª#7<?@? */
	$"3C37 322B 2620 1C17 0F00 B06F 000C 0060"            /* <72+& ....°o...` */
	$"80FF 09C1 0B0C 090B C55C 2836 3B80 3F3C"            /* €ÿÆÁ..Æ.Å\(6;€?< */
	$"3C37 312B 2520 1C16 1100 5FB1 0A0D 0B06"            /* <71+% ...._±Â... */
	$"BFFF FF5A 0411 0828 DA34 2B34 383B 3C3C"            /* ¿ÿÿZ...(Ú4+48;<< */
	$"3935 2F2A 241F 1B16 1003 25D6 1E0D 1101"            /* 95/*$.....%Ö.... */
	$"53FF DE19 1014 073D D12E 2930 3480 375C"            /* SÿÞ....=Ñ.)04€7\ */
	$"3531 2C27 231E 1914 0F04 14E2 270F 140D"            /* 51,'#......â'... */
	$"10DC A009 1616 0941 D12A 252C 2F31 3231"            /* .Ü Æ..ÆAÑ*%,/121 */
	$"2F2C 2824 201C 1713 0E03 14E3 2C12 1614"            /* /,($ ......ã,... */
	$"019A 690D 1919 0D36 DA28 2127 2A2C 2B2C"            /* .ši....6Ú(!'*,+, */
	$"2A27 2421 1D19 1510 0B00 1FDF 2815 1918"            /* *'$!.......ß(... */
	$"055E 4416 1B1B 141C D845 1822 2480 2612"            /* .^D.....ØE."$€&. */
	$"2423 201D 1A16 120E 0800 52C8 1D19 1B1B"            /* $# .......RÈ.... */
	$"1035 2B80 1D06 1B0B AB92 0A1E 1F81 200B"            /* .5+€....«’Â.. . */
	$"1E1C 1916 120F 0A04 00A4 9910 801D 0217"            /* ......Â..¤™.€... */
	$"192C 8120 0510 4FE2 2411 1B80 1C0C 1B19"            /* ., ..Oâ$..€.... */
	$"1715 120F 0B06 0029 E641 1880 2003 1919"            /* .......)æA.€ ... */
	$"2D22 8021 061F 11B7 AB04 1117 8016 0A14"            /* -"€!...·«...€.Â. */
	$"1310 0E0A 0600 01C2 A216 8121 021A 192F"            /* ...Â...Â¢.!.../ */
	$"8224 131A 2FE2 8500 0810 1110 0F0D 0C08"            /* ‚$../â…......... */
	$"0400 009D D52A 2080 2404 231A 194B 2382"            /* ...Õ* €$.#..K#‚ */
	$"2711 1845 E3A3 1B00 0206 0504 0100 0023"            /* '..Eã£.........# */
	$"B5D9 3C1E 8127 0525 1535 711E 2A81 2911"            /* µÙ<.'.%.5q.*). */
	$"2A1B 3AC0 DE86 330F 0A09 0F38 92E3 B634"            /* *.:ÀÞ†3.ÂÆ.8’ã¶4 */
	$"212A 8129 0525 0A5D A61E 2B82 2C0E 2D24"            /* !*).%Â]¦.+‚,.-$ */
	$"2368 C1E5 DCD4 D3DF E3BB 6023 2783 2C05"            /* #hÁåÜÔÓßã»`#'ƒ,. */
	$"2304 99E0 2F2C 842F 0B2D 2427 3557 F0E0"            /* #.™à/,„/.-$'5Wðà */
	$"613B 2726 2E83 2F06 2C1B 11DB FF6A 2486"            /* a;'&.ƒ/.,..Ûÿj$† */
	$"3207 3021 14DE C928 2C31 8432 0931 2A09"            /* 2.0!.ÞÉ(,1„2Æ1*Æ */
	$"53FF FFC7 2932 3585 3405 281E DFCC 3232"            /* SÿÿÇ)25…4.(.ßÌ22 */
	$"8434 0435 301F 08BF 80FF 0176 2486 3705"            /* „4.50..¿€ÿ.v$†7. */
	$"2A1E DFCD 3535 8437 0335 2904 6081 FF04"            /* *.ßÍ55„7.5).`ÿ. */
	$"E945 2C3A 3A83 3905 2C1F DFCD 3737 8139"            /* éE,::ƒ9.,.ßÍ779 */
	$"063A 3A37 2E0F 28E6 82FF 02CA 322F 843C"            /* .::7..(æ‚ÿ.Ê2/„< */
	$"052E 20DF CE3A 3A81 3C05 3B39 2F13 13C3"            /* .. ßÎ::<.;9/..Ã */
	$"84FF 04BF 302C 3C3E 813D 052F 21DF CE3B"            /* „ÿ.¿0,<>=./!ßÎ; */
	$"3B81 3D04 392E 1113 B786 FF04 C840 2238"            /* ;=.9...·†ÿ.È@"8 */
	$"3E80 3F0D 3021 DFCF 3D3D 3F3E 3C35 2507"            /* >€?.0!ßÏ==?><5%. */
	$"28C1 88FF 13E5 6E23 2435 3C3D 3021 DFCF"            /* (Áˆÿ.ån#$5<=0!ßÏ */
	$"3C3B 3933 2610 0C60 E38B FF0F BF60 251B"            /* <;93&..`ã‹ÿ.¿`%. */
	$"211F 1CDF CC2C 2317 0C16 54BA 8FFF 0BD9"            /* !..ßÌ,#...Tºÿ.Ù */
	$"9C66 3B1F E1C7 1F3A 6098 D887 FF87 FF0B"            /* œf;.áÇ.:`˜Ø‡ÿ‡ÿ. */
	$"D893 4F29 171A 1916 2851 94D8 8FFF 0FBB"            /* Ø“O)....(Q”Øÿ.» */
	$"5000 0251 96C0 CCC9 BC91 4A00 0051 BB8B"            /* P..Q–ÀÌÉ¼‘J..Q»‹ */
	$"FF13 E460 0000 5DC9 CFAD 8B7C 7273 8FBA"            /* ÿ.ä`..]ÉÏ­‹|rsº */
	$"BF50 0002 5FE4 88FF 15C4 2A00 099C E5A6"            /* ¿P.._äˆÿ.Ä*.Æœå¦ */
	$"8A88 867E 7365 5242 66D2 8B04 0029 C286"            /* Šˆ†~seRBfÒ‹..)Â† */
	$"FF17 BB18 0005 A3DD 969A A2A0 998E 8173"            /* ÿ.»...£Ý–š¢ ™Žs */
	$"6557 3B33 C091 0504 16B8 84FF 19C8 1E0A"            /* eW;3À‘...¸„ÿ.È.Â */
	$"027A E494 A2AE B0AE A69A 8A7B 6B5D 4F34"            /* .zä”¢®°®¦šŠ{k]O4 */
	$"28CF 670B 0D18 C582 FF1B E839 110F 2BDF"            /* (Ïg...Å‚ÿ.è9..+ß */
	$"A09B ADB7 BAB8 AFA1 917F 6F60 5242 204C"            /*  ›­·º¸¯¡‘.o`RB L */
	$"D125 1B10 2FE7 81FF 1B73 161F 0A89 D187"            /* Ñ%../çÿ.s..Â‰Ñ‡ */
	$"A2B0 BABE BBB2 A493 8170 6053 432F 0EB2"            /* ¢°º¾»²¤“p`SC/.² */
	$"7C18 240D 6880 FF7F C927 2920 1BC9 9E8A"            /* |.$.h€ÿ.É') .ÉžŠ */
	$"A0AE B8BB B9AF A291 8070 6052 4331 0E63"            /*  ®¸»¹¯¢‘€p`RC1.c */
	$"B826 2B26 16C2 FFFF 7027 3221 38E2 7E87"            /* ¸&+&.Âÿÿp'2!8â~‡ */
	$"99A6 AFB2 AFA7 9B8B 7C6C 5D50 4030 142B"            /* ™¦¯²¯§›‹|l]P@0.+ */
	$"D93D 3032 195D FFE2 3E38 3924 4DDC 747F"            /* Ù=02.]ÿâ>89$MÜt. */
	$"8E9A A1A4 A29B 9082 7466 594B 3C2C 141A"            /* Žš¡¤¢›‚tfYK<,.. */
	$"E449 373A 3022 DCB1 3642 402B 52DC 6B74"            /* äI7:0"Ü±6B@+RÜkt */
	$"818A 9093 918B 8277 6B5F 5345 3727 1018"            /* Š“‘‹‚wk_SE7'.. */
	$"E552 3E42 3D1B 9F87 3F40 4948 344C E060"            /* åR>B=.Ÿ‡?@IH4Là` */
	$"6773 7B7F 8180 7B74 6B61 574B 3D30 2009"            /* gs{.€{tkaWK=0 Æ */
	$"22E2 5546 4947 2768 6D4D 5050 433B DB6E"            /* "âUFIG'hmMPPC;Ûn */
	$"5665 6B6F 7070 6C66 5F57 4C40 3527 1801"            /* Vekopplf_WL@5'.. */
	$"51D5 514E 504F 3844 5D80 571D 5235 B5A4"            /* QÕQNPO8D]€W.R5µ¤ */
	$"3F58 5D60 6061 5D59 534B 4136 2A1D 0E00"            /* ?X]``a]YSKA6*... */
	$"A1B5 4D56 5756 422B 635E 805D 1B46 6BE5"            /* ¡µMVWVB+c^€].Fkå */
	$"4641 4F52 5352 504B 453D 342A 1F12 0029"            /* FAORSRPKE=4*...) */
	$"E976 595D 5D5C 482C 6881 6414 5E43 C3B2"            /* évY]]\H,hd.^CÃ² */
	$"243A 4243 4340 3C37 3028 1D12 0401 BDC0"            /* $:BCC@<70(....½À */
	$"5B80 6403 634B 2D6D 826B 1B5C 5DE7 8D18"            /* [€d.cK-m‚k.\]ç. */
	$"2631 3230 2C27 2018 0F00 0098 E66E 686B"            /* &120,' ....˜ænhk */
	$"6B6C 684C 2C84 7081 7312 725F 75E9 A526"            /* klhL,„ps.r_ué¥& */
	$"0A14 1815 120B 0100 22B1 E980 6D80 7305"            /* Â......."±é€m€s. */
	$"746D 4543 9F72 837B 0F6A 76D3 DE84 3514"            /* tmECŸrƒ{.jvÓÞ„5. */
	$"0F0D 1137 8EE4 D580 7582 7B04 6F35 67C3"            /* ...7ŽäÕ€u‚{.o5gÃ */
	$"7484 820D 7A72 9DD9 E9DC D3D3 DFEC DBA2"            /* t„‚.zrÙéÜÓÓßìÛ¢ */
	$"7C7F 8282 0681 6925 9DE9 7F89 828A 0F8B"            /* |.‚‚.i%é.‰‚Š.‹ */
	$"8B8A 8280 7773 F0F0 A592 8585 8A8B 8B80"            /* ‹Š‚€wsðð¥’……Š‹‹€ */
	$"8A08 8C84 5827 DBFF A186 9484 9307 9493"            /* Š.Œ„X'Ûÿ¡†”„“.”“ */
	$"723E DEE1 8A90 8593 0892 7C35 5CFF FFDA"            /* r>ÞáŠ…“.’|5\ÿÿÚ */
	$"8399 869B 047C 49DF E597 869B 0390 6020"            /* ƒ™†›.|Ißå—†›.`  */
	$"C080 FF01 A88B 86A3 0481 4CDF E79F 85A3"            /* À€ÿ.¨‹†£.LßçŸ…£ */
	$"039D 7A2C 6781 FF04 EE8D 98AA AA83 A904"            /* .z,gÿ.î˜ªªƒ©. */
	$"874E DFE8 A582 A906 AAAA A588 4135 E582"            /* ‡Nßè¥‚©.ªª¥ˆA5å‚ */
	$"FF02 D983 9E84 B004 8C51 DFEA AC83 B004"            /* ÿ.Ùƒž„°.ŒQßê¬ƒ°. */
	$"A98D 4B25 C384 FF17 D07F 9AB3 B6B6 B5B6"            /* ©K%Ã„ÿ.Ð.š³¶¶µ¶ */
	$"B590 53DF EBB0 B5B5 B6B6 B4A9 8847 27B8"            /* µSßë°µµ¶¶´©ˆG'¸ */
	$"86FF 15D5 8087 ABB7 BABC BA95 56DF ECB5"            /* †ÿ.Õ€‡«·º¼º•Vßìµ */
	$"BABA B8B0 9C74 3438 C288 FF13 E994 6F87"            /* ºº¸°œt48Âˆÿ.é”o‡ */
	$"A4B2 B794 56DF ECB4 B4AA 9879 4829 67E3"            /* ¤²·”Vßì´´ª˜yH)gã */
	$"8BFF 0FC9 8463 697B 6C44 DFE2 8576 593D"            /* ‹ÿ.É„ci{lDßâ…vY= */
	$"3360 BC8F FF0B DDAB 8055 2FE1 D03F 506D"            /* 3`¼ÿ.Ý«€U/áÐ?Pm */
	$"9DD8 87FF 87FF 0BD8 944F 2A19 1B1B 1828"            /* Ø‡ÿ‡ÿ.Ø”O*....( */
	$"5094 D98F FF0F BA50 0004 5797 BCC7 C5BA"            /* P”Ùÿ.ºP..W—¼ÇÅº */
	$"924C 0000 51BA 8BFF 13E3 5E00 0062 C19B"            /* ’L..Qº‹ÿ.ã^..bÁ› */
	$"5724 1717 285E A6BD 5000 005E E388 FF06"            /* W$..(^¦½P..^ãˆÿ. */
	$"C026 0007 A0BB 3185 0006 40C8 8A00 0026"            /* À&.. »1…..@ÈŠ..& */
	$"C086 FF06 B510 0001 A59F 0487 0006 0DB6"            /* À†ÿ.µ...¥Ÿ.‡...¶ */
	$"8C00 0010 B584 FF06 C20F 0000 7AB7 0189"            /* Œ...µ„ÿ.Â...z·.‰ */
	$"0006 0CCB 5B00 000F C282 FF06 E622 0000"            /* ...Ë[...Â‚ÿ.æ".. */
	$"1ED3 228B 0006 3ACD 0E00 0022 E681 FF00"            /* .Ó"‹..:Í..."æÿ. */
	$"5C80 0001 8596 8D00 01AE 6880 0000 5C80"            /* \€..…–..®h€..\€ */
	$"FF06 BE01 0000 05C3 3C8D 0001 5DAC 8000"            /* ÿ.¾....Ã<..]¬€. */
	$"0401 BEFF FF4E 8000 0220 D60E 8D00 0222"            /* ..¾ÿÿN€.. Ö..." */
	$"D40E 8000 034E FFDB 0780 0002 35CB 098D"            /* Ô.€..NÿÛ.€..5ËÆ */
	$"0002 12E1 1680 0002 07DB 9881 0002 38CC"            /* ...á.€...Û˜..8Ì */
	$"098D 0002 12E2 1881 0001 985A 8100 022A"            /* Æ...â...˜Z..* */
	$"D60C 8D00 021E DD11 8100 015A 2E81 0002"            /* Ö....Ý...Z... */
	$"0CD6 318D 0002 52C2 0581 0001 2E10 8200"            /* .Ö1..RÂ.....‚. */
	$"01A7 898D 0001 A58B 8200 0110 0F82 0002"            /* .§‰..¥‹‚....‚.. */
	$"40E1 148B 0002 28E5 2782 0001 0F0F 8300"            /* @á.‹..(å'‚....ƒ. */
	$"01B1 A78A 0002 01C4 9283 0001 0F10 8300"            /* .±§Š...Ä’ƒ....ƒ. */
	$"021B DF82 8900 029F CC0D 8300 0110 2E84"            /* ..ß‚‰..ŸÌ.ƒ....„ */
	$"0003 31DF A219 8500 0323 B7D1 1F84 0001"            /* ..1ß¢.…..#·Ñ.„.. */
	$"2E5A 8500 0D21 B7DD 8632 0D07 080F 3994"            /* .Z…..!·Ý†2....9” */
	$"E2A7 1285 0001 5A98 8700 094F B5E3 DCD4"            /* â§.…..Z˜‡.ÆOµãÜÔ */
	$"D4DF E0AC 4087 0002 98DB 0788 0005 1848"            /* Ôßà¬@‡..˜Û.ˆ...H */
	$"F0DA 3F14 8800 0307 DBFF 4E8A 0001 DEBD"            /* ðÚ?.ˆ...ÛÿNŠ..Þ½ */
	$"8A00 044E FFFF BE01 8800 0207 DFC0 8900"            /* Š..Nÿÿ¾.ˆ...ßÀ‰. */
	$"0101 BE80 FF00 5C88 0002 07DF C089 0000"            /* ..¾€ÿ.\ˆ...ßÀ‰.. */
	$"5C81 FF01 E622 8700 0207 DFC0 8800 0122"            /* \ÿ.æ"‡...ßÀˆ.." */
	$"E682 FF01 C20F 8600 0207 DFC0 8700 010F"            /* æ‚ÿ.Â.†...ßÀ‡... */
	$"C284 FF01 B510 8500 0207 DFC0 8600 0110"            /* Â„ÿ.µ.…...ßÀ†... */
	$"B586 FF01 C026 8400 0207 DFC0 8500 0126"            /* µ†ÿ.À&„...ßÀ…..& */
	$"C088 FF02 E35E 0282 0002 07DF C083 0002"            /* Àˆÿ.ã^.‚...ßÀƒ.. */
	$"025E E38B FF02 BA52 0A80 0002 07DF C081"            /* .^ã‹ÿ.ºRÂ€...ßÀ */
	$"0002 0A52 BA8F FF0B D996 5B30 17E1 C311"            /* ..ÂRºÿ.Ù–[0.áÃ. */
	$"305B 96D9 87FF 6C38 6D6B 0000 0408 0000"            /* 0[–Ù‡ÿl8mk...... */
	$"0000 0000 0000 0000 0C66 FFF3 FFFF FFFF"            /* .........fÿóÿÿÿÿ */
	$"F4FF 650C 0000 0000 0000 0000 0000 0000"            /* ôÿe............. */
	$"0000 0000 0000 32FF FFFF FF62 2D1D 2132"            /* ......2ÿÿÿÿb-.!2 */
	$"69FF FFFF BD32 0000 0000 0000 0000 0000"            /* iÿÿÿ½2.......... */
	$"0000 0000 00FF FFFF FF22 275D FFFF FFFF"            /* .....ÿÿÿÿ"']ÿÿÿÿ */
	$"7A3A 2DBE FFFF AB00 0000 0000 0000 0000"            /* z:-¾ÿÿ«......... */
	$"0000 0027 F2FF FF59 08FF FFFF FFFF FFFF"            /* ...'òÿÿY.ÿÿÿÿÿÿÿ */
	$"FFFF AD16 71FF FFF3 2900 0000 0000 0000"            /* ÿÿ­.qÿÿó)....... */
	$"0000 33FF FFFF 5117 FFFF FFFF FFFF FFFF"            /* ..3ÿÿÿQ.ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 2E6A FFFF FF36 0000 0000 0000"            /* ÿÿÿÿ.jÿÿÿ6...... */
	$"0022 FFFF FF87 09FF FFFF FFFF FFFF FFFF"            /* ."ÿÿÿ‡Æÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FB19 A4FF FFFF 2600 0000 0000"            /* ÿÿÿÿû.¤ÿÿÿ&..... */
	$"00FF FFFF F306 FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿó.ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFC9 16FE FFFF EE00 0000 0000"            /* ÿÿÿÿÿÉ.þÿÿî..... */
	$"97FF FFFF 7526 FFFF FFFF FFFF FFFF FFFF"            /* —ÿÿÿu&ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF F4FF 3F8B FFFF FFA3 0000 0023"            /* ÿÿÿÿôÿ?‹ÿÿÿ£...# */
	$"FFFF FFFF 21FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ!ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF F8FF FF3A FFFA FFFF 2A00 009F"            /* ÿÿÿÿøÿÿ:ÿúÿÿ*..Ÿ */
	$"FFFF FFFF 02FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FDFF FF0C FFFF F2FF B100 00FF"            /* ÿÿÿÿýÿÿ.ÿÿòÿ±..ÿ */
	$"FFFF FFFF 0CFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 FFFF FFFF FF06 47FF"            /* ÿÿÿÿÿÿÿ.ÿÿÿÿÿ.Gÿ */
	$"FFFF FFFF 0CFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 FFFF FFFF FF59 83FF"            /* ÿÿÿÿÿÿÿ.ÿÿÿÿÿYƒÿ */
	$"FFFF FFFF 04FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 FFFF FFFF FFA4 A9FF"            /* ÿÿÿÿÿÿÿ.ÿÿÿÿÿ¤©ÿ */
	$"FFFF FFFF 09FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÆÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF16 FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ.ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 3E58 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ>Xÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 5347 FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿSGÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFF2 00FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿò.ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF42 42FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿBBÿÿÿÿÿÿÿÿÿ */
	$"FEFF FFFF FF2D 38FF FFFF FFFF FFFF FFFF"            /* þÿÿÿÿ-8ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0072 FFFF F3F2 F3F8 FEFF"            /* ÿÿÿÿÿÿ.rÿÿóòóøþÿ */
	$"FFFF FFFF 5D01 FFFF FFFF FFFF FFFF 92FF"            /* ÿÿÿÿ].ÿÿÿÿÿÿÿÿ’ÿ */
	$"FFFF FFFF FFFF FF00 50FA FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ.Púÿÿÿÿÿÿ */
	$"FFFF FB3E 00FF FFFF FFFF FFFF FFFF 6AFF"            /* ÿÿû>.ÿÿÿÿÿÿÿÿÿjÿ */
	$"FFFF FFFF FFFF FFFF 1B04 79E3 FFFF FFFF"            /* ÿÿÿÿÿÿÿÿ..yãÿÿÿÿ */
	$"DF6C 001E FFFF FFFF FFFF FFFF FFA4 33FF"            /* ßl..ÿÿÿÿÿÿÿÿÿ¤3ÿ */
	$"FFFF FFFF FFFF FFFF FF70 1500 0612 1202"            /* ÿÿÿÿÿÿÿÿÿp...... */
	$"0016 FFFF FFFF FFFF FFFF FFFF FF5B 00FF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿ[.ÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 00FF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ..ÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF08 006C"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ..l */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF04 0AFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ.Âÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFEF B300 0012"            /* ÿÿÿÿÿÿÿÿÿÿÿï³... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF02 06FF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ..ÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 2B00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ+... */
	$"60FF FFFF FFFF FFFF FFFF FFFF FF02 04FF"            /* `ÿÿÿÿÿÿÿÿÿÿÿÿ..ÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFA4 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿ¤.... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FF02 02FF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿ..ÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF00 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿ..... */
	$"0011 FFFF FFFF FFFF FFFF FFFF FF02 01FF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿ..ÿ */
	$"FFFF FFFF FFFF FFFF FFFE 2700 0000 0000"            /* ÿÿÿÿÿÿÿÿÿþ'..... */
	$"0000 1EFF FFFF FFFF FFFF FFFF FF02 00FF"            /* ...ÿÿÿÿÿÿÿÿÿÿ..ÿ */
	$"FFFF FFFF FFFF FFFF FC36 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿü6...... */
	$"0000 0016 FFFF FFFF FFFF FFFF FF02 00FF"            /* ....ÿÿÿÿÿÿÿÿÿ..ÿ */
	$"FFFF FFFF FFFF FFFF 2900 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿ)....... */
	$"0000 0000 0075 FFFF FFFF FFFF FF02 00FF"            /* .....uÿÿÿÿÿÿÿ..ÿ */
	$"FFFF FFFF FFFB A300 0000 0000 0000 0000"            /* ÿÿÿÿÿû£......... */
	$"0000 0000 0000 25FF FFFF FFFF FF02 09FF"            /* ......%ÿÿÿÿÿÿ.Æÿ */
	$"FFFF FFFF AF32 0000 0000 0000 0000 0000"            /* ÿÿÿÿ¯2.......... */
	$"0000 0000 0000 0000 064D FFFF FF00 1AFF"            /* .........Mÿÿÿ..ÿ */
	$"FFFF 5C0C 0000 0000 0000 0000 0000"                 /* ÿÿ\........... */
};

data 'icns' (128, "Item Icon") {
	$"6963 6E73 0000 8396 4943 4E23 0000 0108"            /* icns..ƒ–ICN#.... */
	$"FFFF FFFF FFFF DFFF FFDF FFFF FF7D 7DFF"            /* ÿÿÿÿÿÿßÿÿßÿÿÿ}}ÿ */
	$"FDF6 DFBF FFED 6FFF F7F6 BBDF FDC9 77FF"            /* ýöß¿ÿíoÿ÷ö»ßýÉwÿ */
	$"EFBE DDEF F7F5 EFF7 DF4E BBFB F7B5 77EF"            /* ï¾Ýï÷õï÷ßN»û÷µwï */
	$"BBDB DEFD EFFD BFF7 FFEE EBDB DDFB FFFF"            /* »ÛÞýïý¿÷ÿîëÛÝûÿÿ */
	$"EFFE BFED FBFF FFB7 BDFF FFFB FFBF FF6F"            /* ïþ¿íûÿÿ·½ÿÿûÿ¿ÿo */
	$"F76F F5DB FFFF FFBF DDDF DDEF FFFD F77F"            /* ÷oõÛÿÿÿ¿ÝßÝïÿý÷. */
	$"FEFF FBDF FBBB FFFF FFEF FF7F FFFF FDFF"            /* þÿûßû»ÿÿÿïÿ.ÿÿýÿ */
	$"FEFF DFFF FFF7 FFFF FFFF FFFF FFFF FFFF"            /* þÿßÿÿ÷ÿÿÿÿÿÿÿÿÿÿ */
	$"001F F800 00FF FF00 01F0 1F80 07CF E7E0"            /* ..ø..ÿÿ..ð.€.Ïçà */
	$"0FBF FBF0 1F7F FDF8 1F7F FEF8 3EFF FF7C"            /* .¿ûð..ýø..þø>ÿÿ| */
	$"7EFF FF7E 7CFF FF3E 7CFF FF3E FCFF FF3F"            /* ~ÿÿ~|ÿÿ>|ÿÿ>üÿÿ? */
	$"FCFF FF3F FCFF FF3F FE7F FE7F FE7F FCFF"            /* üÿÿ?üÿÿ?þ.þ.þ.üÿ */
	$"FF3F FCFF FF8F F1FF FFC0 07FF FFF0 0FFF"            /* ÿ?üÿÿñÿÿÀ.ÿÿð.ÿ */
	$"FFFE 7FFF 7FFE 7FFE 7FFE 7FFE 7FFE 7FFE"            /* ÿþ.ÿ.þ.þ.þ.þ.þ.þ */
	$"3FFE 7FFC 1FFE 7FF8 1FFE 7FF8 0FFE 7FF0"            /* ?þ.ü.þ.ø.þ.ø.þ.ð */
	$"07FE 7FE0 01FE 7F80 00FE 7F00 001E 7800"            /* .þ.à.þ.€.þ....x. */
	$"6963 6C34 0000 0208 FFFF FFFF FFFF F9F9"            /* icl4....ÿÿÿÿÿÿùù */
	$"F9FF FFFF FFFF FFFF FFFF FFFF FF99 9F9F"            /* ùÿÿÿÿÿÿÿÿÿÿÿÿ™ŸŸ */
	$"9F99 99FF FFFF FFFF FFFF FFFF 999F 9FFF"            /* Ÿ™™ÿÿÿÿÿÿÿÿÿ™ŸŸÿ */
	$"FFFF F999 FFFF FFFF FFFF FF99 99FF F999"            /* ÿÿù™ÿÿÿÿÿÿÿ™™ÿù™ */
	$"99F9 FF9F 99FF FFFF FFFF F999 9FF9 9898"            /* ™ùÿŸ™ÿÿÿÿÿù™Ÿù˜˜ */
	$"9999 9FF9 F9FF FFFF FFFF 9999 FF99 8989"            /* ™™Ÿùùÿÿÿÿÿ™™ÿ™‰‰ */
	$"8999 F9FF F999 FFFF FFF9 999F F998 8888"            /* ‰™ùÿù™ÿÿÿù™Ÿù˜ˆˆ */
	$"9899 99FF F99F 9FFF FFF9 999F F988 9889"            /* ˜™™ÿùŸŸÿÿù™Ÿùˆ˜‰ */
	$"8999 999F FFF9 9FFF FF99 99FF 9989 8898"            /* ‰™™ŸÿùŸÿÿ™™ÿ™‰ˆ˜ */
	$"9998 999F F999 99FF FF99 99FF F898 8989"            /* ™˜™Ÿù™™ÿÿ™™ÿø˜‰‰ */
	$"9999 9F99 FF9F 99FF F999 99FF 9988 8998"            /* ™™Ÿ™ÿŸ™ÿù™™ÿ™ˆ‰˜ */
	$"9899 999F FFF9 999F F999 8FFF 9989 9889"            /* ˜™™Ÿÿù™Ÿù™ÿ™‰˜‰ */
	$"9999 999F FF99 9F9F F999 99FF F898 9999"            /* ™™™Ÿÿ™ŸŸù™™ÿø˜™™ */
	$"9999 99F9 FFF9 999F 9F99 9FFF 9F99 9989"            /* ™™™ùÿù™ŸŸ™ŸÿŸ™™‰ */
	$"9999 9F9F FF99 99F9 F999 99FF FF99 9999"            /* ™™ŸŸÿ™™ùù™™ÿÿ™™™ */
	$"9999 99FF FF99 999F F999 F99F FFF9 9999"            /* ™™™ÿÿ™™Ÿù™ùŸÿù™™ */
	$"999F 9FFF F9F9 9999 F999 99FF FFFF 99F9"            /* ™ŸŸÿùù™™ù™™ÿÿÿ™ù */
	$"99F9 FFFF F999 999F 9F9F 9999 FFFF FF99"            /* ™ùÿÿù™™ŸŸŸ™™ÿÿÿ™ */
	$"F9FF FFFF 9999 99F9 F9F9 F999 F9FF FFFF"            /* ùÿÿÿ™™™ùùùù™ùÿÿÿ */
	$"FFFF FFF9 9F99 999F FF9F 999F 99FF FFFF"            /* ÿÿÿùŸ™™ŸÿŸ™Ÿ™ÿÿÿ */
	$"FFFF FF99 9999 999F FF9F 9F99 9F99 FFFF"            /* ÿÿÿ™™™™ŸÿŸŸ™Ÿ™ÿÿ */
	$"FFF9 999F 9999 99F9 FFF9 F9F9 F99F 99FF"            /* ÿù™Ÿ™™™ùÿùùùùŸ™ÿ */
	$"FF9F 9999 9999 9F9F FF9F 9F99 9F99 9FFF"            /* ÿŸ™™™™ŸŸÿŸŸ™Ÿ™Ÿÿ */
	$"FF99 F999 99F9 9FFF FFF9 FF9F F9F9 9F9F"            /* ÿ™ù™™ùŸÿÿùÿŸùùŸŸ */
	$"FFF9 99F9 9F99 F9FF FFFF 9F99 F99F F9FF"            /* ÿù™ùŸ™ùÿÿÿŸ™ùŸùÿ */
	$"FF9F 999F 999F 9FFF FFFF F9FF F9FF 99FF"            /* ÿŸ™Ÿ™ŸŸÿÿÿùÿùÿ™ÿ */
	$"FFF9 F9F9 9FF9 FFFF FFFF FF99 FF99 FFFF"            /* ÿùùùŸùÿÿÿÿÿ™ÿ™ÿÿ */
	$"FF9F 9F9F 9F9F FFFF FFFF F9FF 9F9F FF9F"            /* ÿŸŸŸŸŸÿÿÿÿùÿŸŸÿŸ */
	$"FFFF 9F9F F9FF FFFF FFFF FFF9 F9F9 F9FF"            /* ÿÿŸŸùÿÿÿÿÿÿùùùùÿ */
	$"FF9F F9FF FFFF FFFF FFFF FFFF FFFF 9FFF"            /* ÿŸùÿÿÿÿÿÿÿÿÿÿÿŸÿ */
	$"FFFF FFF9 FFFF FFFF FFFF FFFF FFF9 FFFF"            /* ÿÿÿùÿÿÿÿÿÿÿÿÿùÿÿ */
	$"FFF9 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿùÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 6963 6C38 0000 0408"            /* ÿÿÿÿÿÿÿÿicl8.... */
	$"FFFF FFFF FFFF FFFF FFFF FFEA E9D1 D1D1"            /* ÿÿÿÿÿÿÿÿÿÿÿêéÑÑÑ */
	$"D1D1 D1E9 EAFF FFFF FFFF FFFF FFFF FFFF"            /* ÑÑÑéêÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFE9 E8E7 E7E8 E8E8"            /* ÿÿÿÿÿÿÿÿÿéèççèèè */
	$"D1E8 E8E8 E8E8 D1FF FFFF FFFF FFFF FFFF"            /* ÑèèèèèÑÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFE9 E7E7 E7D1 E9EA FFEA"            /* ÿÿÿÿÿÿÿéçççÑéêÿê */
	$"EAFF EAEA D1E8 E8E8 E9FF FFFF FFFF FFFF"            /* êÿêêÑèèèéÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF E8E7 CBE8 E9FF E9E7 CBCB"            /* ÿÿÿÿÿÿèçËèéÿéçËË */
	$"E7E7 D1E9 FFEA E8E8 E7D1 EAFF FFFF FFFF"            /* ççÑéÿêèèçÑêÿÿÿÿÿ */
	$"FFFF FFFF FFE7 CBCB E8EA FFE7 E6E5 E5E6"            /* ÿÿÿÿÿçËËèêÿçæååæ */
	$"E6CB CBE8 E8FF FFD1 E8E7 D1EA FFFF FFFF"            /* æËËèèÿÿÑèçÑêÿÿÿÿ */
	$"FFFF FFFF E7E6 E6E7 FFFF CBE6 C5C5 C5E5"            /* ÿÿÿÿçææçÿÿËæÅÅÅå */
	$"E5E6 CBCB E8E8 EAFF D1E8 E7E8 FFFF FFFF"            /* åæËËèèêÿÑèçèÿÿÿÿ */
	$"FFFF FFE8 E6E6 CBEA FFE7 E6C5 C5E4 E4C5"            /* ÿÿÿèææËêÿçæÅÅääÅ */
	$"E6E5 E6CB CBE8 D1FF EAD1 E7E8 E8FF FFFF"            /* æåæËËèÑÿêÑçèèÿÿÿ */
	$"FFFF E9CB E6E6 D1FF E9CB E5E4 C5E4 C5E5"            /* ÿÿéËææÑÿéËåäÅäÅå */
	$"E5E6 E6CB CBCB E8EA FFE9 E8E7 E7E9 FFFF"            /* åææËËËèêÿéèççéÿÿ */
	$"FFFF E8E6 E6CB EAFF D1E5 C5C5 E4C5 C5E5"            /* ÿÿèææËêÿÑåÅÅäÅÅå */
	$"E6E6 CBE6 CBE7 E8E9 FFEA E8E7 E7E8 EAFF"            /* ææËæËçèéÿêèççèêÿ */
	$"FFE9 E7E6 CBE7 FFFF E7E5 C5E4 C5E4 E5E5"            /* ÿéçæËçÿÿçåÅäÅäåå */
	$"E6E6 CBCB CBE7 E7D1 FFFF D1E7 E7E7 E9FF"            /* ææËËËççÑÿÿÑçççéÿ */
	$"FFD1 CBE6 CBD1 FFFF E7E5 C5E4 C5E5 E5E6"            /* ÿÑËæËÑÿÿçåÅäÅååæ */
	$"E6E6 CBCB CBCB E8D1 FFFF D1E7 E7E7 E8FF"            /* ææËËËËèÑÿÿÑçççèÿ */
	$"FFE7 CBCB CBE8 FFFF E8E6 C5E5 E5E5 E5E6"            /* ÿçËËËèÿÿèæÅååååæ */
	$"E6CB CBCB CBE7 E7E9 FFFF E8E8 CBE7 E8EA"            /* æËËËËççéÿÿèèËçèê */
	$"E9E8 CBE6 E7E8 FFFF D1CB E6E5 E6E5 E6E6"            /* éèËæçèÿÿÑËæåæåææ */
	$"CBCB CBCB E7E7 E8E9 FFFF D1E7 E7E7 E7E9"            /* ËËËËççèéÿÿÑççççé */
	$"E9E8 CBCB CBE8 FFFF EAE8 CBE6 E6E6 E6CB"            /* éèËËËèÿÿêèËææææË */
	$"CBCB CBE7 CBE8 D1FF FFFF E8E7 E7CB E7D1"            /* ËËËçËèÑÿÿÿèççËçÑ */
	$"E9E7 E7E7 E7E8 EAFF FFE9 E7CB CBCB CBCB"            /* éççççèêÿÿéçËËËËË */
	$"CBCB E7CB E8E8 EAFF FFE9 E8E7 CBE7 E7E8"            /* ËËçËèèêÿÿéèçËççè */
	$"E9E8 E7E7 E7E7 D1FF FFFF D1E8 E7CB CBCB"            /* éèççççÑÿÿÿÑèçËËË */
	$"CBE7 E7E8 E8E9 EAFF FFD1 E7CB E7CB E7E8"            /* ËççèèéêÿÿÑçËçËçè */
	$"E9E8 E7E7 E7E7 E8EA FFFF FFE9 E8E8 E8E7"            /* éèççççèêÿÿÿéèèèç */
	$"E8E7 E8D1 EAFF FFFF E9E7 E7E7 CBCB E7E8"            /* èçèÑêÿÿÿéçççËËçè */
	$"E9D1 E8E7 E7E7 E8E8 EAFF FFFF FFE9 D1D1"            /* éÑèçççèèêÿÿÿÿéÑÑ */
	$"D1D1 EAEA FFFF FFEA E8E7 E7CB CBE7 E7D1"            /* ÑÑêêÿÿÿêèççËËççÑ */
	$"EAD1 D1E8 E8E7 E7E8 D1E9 FFFF FFFF FFFF"            /* êÑÑèèççèÑéÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF E9E8 E7E7 CBE7 CBCB E7D1"            /* ÿÿÿÿÿÿéèççËçËËçÑ */
	$"EAE9 D1D1 E8E8 E7E8 E7D1 E9EA FFFF FFFF"            /* êéÑÑèèçèçÑéêÿÿÿÿ */
	$"FFFF FFFF EAD1 E8E7 E7CB E7CB CBE7 E8E9"            /* ÿÿÿÿêÑèççËçËËçèé */
	$"FFE9 D1D1 D1E8 E8E7 E8E7 E8D1 D1EA FFFF"            /* ÿéÑÑÑèèçèçèÑÑêÿÿ */
	$"FFFF E9D1 E8E8 E7E7 E7E7 CBE7 E7E7 E8EA"            /* ÿÿéÑèèççççËçççèê */
	$"FFEA E9D1 D1D1 E8E8 E8E8 E8E8 E8D1 E9FF"            /* ÿêéÑÑÑèèèèèèèÑéÿ */
	$"FFEA D1E8 E7E7 E7E7 E7CB E7E7 E7E8 D1FF"            /* ÿêÑèçççççËçççèÑÿ */
	$"FFEA E9E9 D1D1 D1E8 E8E8 E8E7 E8D1 EAFF"            /* ÿêééÑÑÑèèèèçèÑêÿ */
	$"FFEA D1E8 E7E7 E7E7 E7E7 E7E7 E8D1 E9FF"            /* ÿêÑèççççççççèÑéÿ */
	$"FFFF EAE9 E9D1 D1D1 D1D1 E8E8 E8D1 E9FF"            /* ÿÿêééÑÑÑÑÑèèèÑéÿ */
	$"FFEA D1E7 E8E7 E7E7 E7E8 E7E8 E8D1 FFFF"            /* ÿêÑçèççççèçèèÑÿÿ */
	$"FFFF EAE9 D1E9 D1D1 D1D1 D1E8 D1D1 EAFF"            /* ÿÿêéÑéÑÑÑÑÑèÑÑêÿ */
	$"FFFF D1E8 E8E8 E8E8 E8E8 E8D1 D1EA FFFF"            /* ÿÿÑèèèèèèèèÑÑêÿÿ */
	$"FFFF FFEA E9E9 D1E9 D1D1 D1D1 D1D1 EAFF"            /* ÿÿÿêééÑéÑÑÑÑÑÑêÿ */
	$"FFFF D1D1 E8E8 E8E8 E8D1 D1D1 EAFF FFFF"            /* ÿÿÑÑèèèèèÑÑÑêÿÿÿ */
	$"FFFF FFFF EAE9 E9D1 E9D1 D1E9 D1E9 EAFF"            /* ÿÿÿÿêééÑéÑÑéÑéêÿ */
	$"FFEA E9D1 D1D1 D1D1 D1D1 E9EA EAFF FFFF"            /* ÿêéÑÑÑÑÑÑÑéêêÿÿÿ */
	$"FFFF FFFF FFEA E9E9 D1E9 D1D1 E9E9 EAFF"            /* ÿÿÿÿÿêééÑéÑÑééêÿ */
	$"FFFF E9E9 D1D1 D1E9 E9E9 EAFF FFFF FFFF"            /* ÿÿééÑÑÑéééêÿÿÿÿÿ */
	$"FFFF FFFF FFFF EAE9 E9E9 E9D1 E9E9 FFFF"            /* ÿÿÿÿÿÿêééééÑééÿÿ */
	$"FFFF EAE9 E9E9 EAE9 EAEA FFFF FFFF FFFF"            /* ÿÿêéééêéêêÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF EAE9 E9E9 E9EA EAFF"            /* ÿÿÿÿÿÿÿÿêééééêêÿ */
	$"FFFF EAEA EAEA EAEA EAFF FFFF FFFF FFFF"            /* ÿÿêêêêêêêÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF EAE9 E9EA EAFF"            /* ÿÿÿÿÿÿÿÿÿÿêééêêÿ */
	$"FFFF FFEA FFEA FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿêÿêÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFEA FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿêÿÿ */
	$"FFFF EAFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿêÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"696C 3332 0000 03BD FF00 C400 0102 01FF"            /* il32...½ÿ.Ä....ÿ */
	$"00FF 00FF 00FF 00FF 00FF 00A6 0087 0003"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.¦.‡.. */
	$"0314 2735 8139 0333 2616 058F 0009 0728"            /* ..'59.3&...Æ.( */
	$"454E 4F4B 4138 383F 8043 0241 2A0C 8B00"            /* ENOKA88?€C.A*.‹. */
	$"1302 234D 5755 4021 0B08 0E0E 070B 1C36"            /* ..#MWU@!.......6 */
	$"4549 4326 0588 0015 083E 5D60 4D1A 0428"            /* EIC&.ˆ...>]`M..( */
	$"5162 5E55 4C39 1C03 183B 4648 390D 8600"            /* Qb^UL9...;FH9.†. */
	$"170C 4D69 6645 080F 5071 8289 7F72 675D"            /* ..MifE..Pq‚‰.rg] */
	$"4E35 0909 3045 4B3F 1184 0019 084E 7071"            /* N5ÆÆ0EK?.„...Npq */
	$"4D06 0D58 7B90 9B9D 9081 786E 6252 3908"            /* M..X{›xnbR9. */
	$"0731 484E 3E0D 8200 1B01 3F72 7D63 1102"            /* .1HN>.‚...?r}c.. */
	$"487A 949F A3A1 9385 7B73 6C60 4F2E 0110"            /* Hz”Ÿ£¡“…{sl`O... */
	$"3B4E 4E37 0581 001B 2063 7F75 3900 1D6B"            /* ;NN7... c.u9..k */
	$"8E9F A3A4 9E90 8379 736C 6559 4411 0027"            /* ŽŸ£¤žƒysleYD..' */
	$"4653 4C27 8000 7F05 496F 7B63 1300 3D7C"            /* FSL'€...Io{c..=| */
	$"99A2 A4A3 9789 7E75 6E69 655D 4B24 0011"            /* ™¢¤£—‰~unie]K$.. */
	$"3F54 5743 0E00 0020 5972 704E 0201 5185"            /* ?TWC... YrpN..Q… */
	$"9DA4 A39C 8F83 7971 6B67 645D 4D2D 0006"            /* ¤£œƒyqkgd]M-.. */
	$"3952 5A4E 2900 013B 6272 683D 0003 5687"            /* 9RZN)..;brh=..V‡ */
	$"9DA0 9A91 877D 756E 6966 635C 4C2F 0101"            /*  š‘‡}unifc\L/.. */
	$"3651 5A53 3C08 0D48 6670 6339 0001 4778"            /* 6QZS<..Hfpc9..Gx */
	$"8D90 8C87 8078 716B 6764 615A 4A2A 0001"            /* Œ‡€xqkgdaZJ*.. */
	$"3651 5A57 4518 1B7F 4D66 6D62 3D00 002D"            /* 6QZWE...Mfmb=..- */
	$"627A 8180 7E79 736C 6764 625E 5441 1D00"            /* bz€~yslgdb^TA.. */
	$"0339 515A 594C 2824 4C60 665F 4405 0011"            /* .9QZYL($L`f_D... */
	$"4A67 7578 7673 6D67 6362 605A 4B34 0B00"            /* Jguxvsmgcb`ZK4.. */
	$"0A3E 535B 5C52 3628 4859 5E5B 4715 0001"            /* Â>S[\R6(HY^[G... */
	$"2A50 646D 6D6B 6763 6160 5C50 3B1C 0000"            /* *Pdmmkgca`\P;... */
	$"1C46 575E 5F58 3F29 4554 5957 4B2E 0100"            /* .FW^_X?)ETYWK... */
	$"052F 4B59 6060 5E5D 5C57 4D3D 2303 0004"            /* ./KY``^]\WM=#... */
	$"3651 5D60 615A 4226 2740 4F55 5550 4219"            /* 6Q]`aZB&'@OUUPB. */
	$"0000 0424 3C48 4E50 4F4A 4234 1D03 0000"            /* ...$<HNPOJB4.... */
	$"2248 595F 605F 5841 1F38 484F 5253 4E3C"            /* "HY_`_XA.8HORSN< */
	$"1180 0007 0D20 2F34 322B 1C0B 8000 1319"            /* .€... /42+..€... */
	$"4154 5D5F 5F5D 543D 162F 3D44 4A50 5048"            /* AT]__]T=./=DJPPH */
	$"3715 0181 0001 0303 8100 0402 1E40 505A"            /* 7..........@PZ */
	$"805F 0F5D 5136 0E26 323A 4248 4D4C 463C"            /* €_.]Q6.&2:BHMLF< */
	$"260C 0183 007F 0110 2F44 5158 5C5E 5F5F"            /* &..ƒ..../DQX\^__ */
	$"5A49 2606 1F2A 3239 4146 4B4C 4942 3A2C"            /* ZI&..*29AFKLIB:, */
	$"1908 0000 0518 2F40 4B54 585B 5D5C 5C5B"            /* ....../@KTX[]\\[ */
	$"513C 1501 1826 2E33 393F 4549 4A49 4743"            /* Q<...&.39?EIJIGC */
	$"3516 0000 0D31 4550 5659 5958 5A59 5854"            /* 5....1EPVYYXZYXT */
	$"462E 0600 0C20 2A2F 3337 3E41 4345 4847"            /* F.... *.37>ACEHG */
	$"3A19 0000 0F38 4C53 5556 5654 5556 544A"            /* :....8LSUVVTUVTJ */
	$"381C 0000 0218 242C 2F32 3538 3A3F 4443"            /* 8.....$,/258:?DC */
	$"3617 0000 0E36 0B49 4F4F 5050 4E4E 4F4A"            /* 6....6.IOOPPNNOJ */
	$"3D29 0980 001B 0B1D 272C 2E30 3133 373D"            /* =)Æ€....',.0137= */
	$"3C31 1400 000C 3041 4849 4947 4545 433B"            /* <1....0AHIIGEEC; */
	$"2C15 8100 1B01 1220 292C 2E2F 2F32 3534"            /* ,..... ),.//254 */
	$"2911 0000 0A29 3B41 4241 3F3D 3A33 291A"            /* )...Â);ABA?=:3). */
	$"0382 0005 0316 2128 2C2E 802F 102D 230F"            /* .‚....!(,.€/.-#. */
	$"0000 0924 3339 3936 3432 2D23 1706 8400"            /* ..Æ$399642-#..„. */
	$"1704 161F 2529 2C2D 2E2A 200D 0000 071D"            /* ....%),-.* ..... */
	$"292E 2D2B 2926 1F14 0686 0015 0312 1B21"            /* ).-+)&...†.....! */
	$"2528 2A27 1F0C 0000 0515 1C20 201D 1A17"            /* %(*'.......  ... */
	$"1105 8800 1301 0A15 1A1F 2424 1C0B 0000"            /* ..ˆ...Â...$$.... */
	$"030C 1012 1310 0D08 028B 000F 020A 1218"            /* .........‹...Â.. */
	$"1A15 0800 0002 0709 0B0B 0702 8F00 0B01"            /* .......Æ....... */
	$"060B 0B05 0000 0205 0504 0187 00FF 00FF"            /* ...........‡.ÿ.ÿ */
	$"00FF 00FF 00FF 00FF 00FF 00EF 006C 386D"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ï.l8m */
	$"6B00 0004 0800 0000 0000 0000 0000 0022"            /* k.............." */
	$"FFFF FFFF FFFF FFFF FFFF 2800 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ(..... */
	$"0000 0000 0000 0000 0000 0000 00AD FFFF"            /* .............­ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF B600 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ¶... */
	$"0000 0000 0000 0000 0000 001A FFFF FFFF"            /* ............ÿÿÿÿ */
	$"FF50 5050 5050 5050 BEFF FFFF FFFF 2100"            /* ÿPPPPPPP¾ÿÿÿÿÿ!. */
	$"0000 0000 0000 0000 0000 D3FF FFFF FF50"            /* ..........ÓÿÿÿÿP */
	$"50FF FFFF FFFF FFFF 5050 BEFF FFFF FFDE"            /* PÿÿÿÿÿÿÿPP¾ÿÿÿÿÞ */
	$"0000 0000 0000 0000 00FE FFFF FFFF 50BE"            /* .........þÿÿÿÿP¾ */
	$"FFFF FFFF FFFF FFFF FFFF 50BE FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿP¾ÿÿÿÿ */
	$"FF00 0000 0000 0000 D9FF FFFF FF50 BEFF"            /* ÿ.......ÙÿÿÿÿP¾ÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF50 BEFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿP¾ÿÿÿ */
	$"FFE6 0000 0000 001E FFFF FFFF BE50 FFFF"            /* ÿæ......ÿÿÿÿ¾Pÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 50BE FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿP¾ÿÿ */
	$"FFFF 2B00 0000 00FF FFFF FFDF 50BE FFFF"            /* ÿÿ+....ÿÿÿÿßP¾ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF BE50 FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ¾Pÿÿ */
	$"FFFF FF00 0000 BEFF FFFF FFB6 00FF FFFF"            /* ÿÿÿ...¾ÿÿÿÿ¶.ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ.ÿÿ */
	$"FFFF FFD1 0000 FFFF FFFF FF8E 14FF FFFF"            /* ÿÿÿÑ..ÿÿÿÿÿŽ.ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ.ÿÿ */
	$"FFFF FFFF 0037 FFFF FFFF FF80 25FF FFFF"            /* ÿÿÿÿ.7ÿÿÿÿÿ€%ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 AFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ.¯ÿ */
	$"FFFF FFFF 47FF FFFF FFFF FF80 12FF FFFF"            /* ÿÿÿÿGÿÿÿÿÿÿ€.ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 79FF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ.yÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF80 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿ€.ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 E2FF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ.âÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF78 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿx.ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF F700 FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ÷.ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFB6 0037 FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿ¶.7ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ..ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 2300 FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ#.ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF7C 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿ|.ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 00FE"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ..þ */
	$"FFFF FFFF FFFF FFFF FFFF 8F00 50FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿ.Pÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"1FFF FFFF FFFF FFFF FF00 0024 FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿ..$ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF09"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÆ */
	$"0000 000E 3939 0300 0000 B8FF FFFF FFFF"            /* ....99....¸ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF08 0000 0000 0000 6DFF FFFF FFFF FFFF"            /* ÿ.......mÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ..ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF50 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿPÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ..ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 6300 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿc.ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ..ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 DEFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ..Þÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ..ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFEB 0000 00FF FFFF FFFF FFFF FFFF"            /* ÿÿÿë...ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ..ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF00 0000 0047 FFFF FFFF FFFF FFFF"            /* ÿÿÿ....Gÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ..ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 5800 0000 0000 F7FF FFFF FFFF FFFF"            /* ÿÿX.....÷ÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ..ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFD 0000 0000 0000 00FF FFFF FFFF FFFF"            /* ÿý.......ÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ..ÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 0000 FAFF FFFF FFFF"            /* ÿ.........úÿÿÿÿÿ */
	$"FFFF FFFF 0000 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ..ÿÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 0054 FFFF FFFF"            /* ...........Tÿÿÿÿ */
	$"FFFF FFFF 0000 FFFF FFFF FFFF FFFF 6000"            /* ÿÿÿÿ..ÿÿÿÿÿÿÿÿ`. */
	$"0000 0000 0000 0000 0000 0000 00E9 FFFF"            /* .............éÿÿ */
	$"FFFF FFFF 0000 FFFF FFFF FFFF EF00 0000"            /* ÿÿÿÿ..ÿÿÿÿÿÿï... */
	$"0000 0000 0000 0000 0000 0000 0000 006C"            /* ...............l */
	$"FFFF FFFF 0000 FFFF FFFF 7300 0000 0000"            /* ÿÿÿÿ..ÿÿÿÿs..... */
	$"0000 0000 0069 6373 2300 0000 48FF FFFF"            /* .....ics#...Hÿÿÿ */
	$"FFFA BFDD 7BF6 DFBD BDDF EFFA BFFF FADF"            /* ÿú¿Ý{öß½½ßïú¿ÿúß */
	$"FFFD EDF7 DFFF F7FF FFFB FFFF FF07 E01F"            /* ÿýí÷ßÿ÷ÿÿûÿÿÿ.à. */
	$"F83F FC7F FE7F FEEF EFEF EFEF EFF7 DFF8"            /* ø?ü.þ.þïïïïïï÷ßø */
	$"3FFE 7F7E 7E7E 7E3E 7C1E 7806 6069 6373"            /* ?þ.~~~~>|.x.`ics */
	$"3400 0000 88FF FFFF 9F9F 9FFF FFFF FF99"            /* 4...ˆÿÿÿŸŸŸÿÿÿÿ™ */
	$"F9F9 FF9F FFFF 99F9 9899 9FF9 FFF9 9F99"            /* ùùÿŸÿÿ™ù˜™ŸùÿùŸ™ */
	$"8989 99F9 9FF9 9F98 8899 99FF 9FF9 9F99"            /* ‰‰™ùŸùŸ˜ˆ™™ÿŸùŸ™ */
	$"8999 99FF 9999 9F99 9999 99FF 99F9 9FF9"            /* ‰™™ÿ™™Ÿ™™™™ÿ™ùŸù */
	$"9999 9FF9 999F 99FF F99F FFF9 99F9 9F9F"            /* ™™Ÿù™Ÿ™ÿùŸÿù™ùŸŸ */
	$"FFFF FF99 99FF 99F9 9FFF 9999 9FFF 9F9F"            /* ÿÿÿ™™ÿ™ùŸÿ™™ŸÿŸŸ */
	$"9FF9 99F9 9FFF F9F9 FFFF 999F FFFF FF9F"            /* Ÿù™ùŸÿùùÿÿ™ŸÿÿÿŸ */
	$"9FFF 9FF9 FFFF FFF9 FFFF F9FF FFFF FFFF"            /* ŸÿŸùÿÿÿùÿÿùÿÿÿÿÿ */
	$"FFFF FFFF FF69 6373 3800 0001 08FF FFFF"            /* ÿÿÿÿÿics8....ÿÿÿ */
	$"FFFF D1D1 E8D1 D1D1 EAFF FFFF FFFF FFFF"            /* ÿÿÑÑèÑÑÑêÿÿÿÿÿÿÿ */
	$"D1E7 E9D1 D1D1 E9E9 D1E8 EAFF FFFF FFE8"            /* ÑçéÑÑÑééÑèêÿÿÿÿè */
	$"CBEA E7E5 C5E6 E6E8 EAD1 E8EA FFFF D1E6"            /* ËêçåÅææèêÑèêÿÿÑæ */
	$"D1D1 E5E4 C5E5 E6CB E7FF E8E7 FFFF CBCB"            /* ÑÑåäÅåæËçÿèçÿÿËË */
	$"FFCB E4C5 E5E6 E6CB E7EA E9E7 D1E9 CBE7"            /* ÿËäÅåææËçêéçÑéËç */
	$"FFCB C5E5 E5E6 CBCB E7E9 EAE7 E7D1 CBE7"            /* ÿËÅååæËËçéêççÑËç */
	$"FFE8 E6E6 E6CB CBE7 E8FF E9E7 E7D1 CBE7"            /* ÿèæææËËçèÿéççÑËç */
	$"EAFF E8CB CBCB E7E7 E9FF E8CB E7D1 E8E7"            /* êÿèËËËççéÿèËçÑèç */
	$"E8FF EAE9 E8E8 D1EA FFE9 E7CB E7E9 D1E7"            /* èÿêéèèÑêÿéçËçéÑç */
	$"E8D1 EAFF FFFF FFEA E9E8 CBE7 E7EA D1D1"            /* èÑêÿÿÿÿêéèËççêÑÑ */
	$"E8E7 E8D1 FFFF EAE8 E7CB E7E7 E8FF EAD1"            /* èçèÑÿÿêèçËççèÿêÑ */
	$"E8D1 E8E8 EAFF E9E7 E7E8 E7E7 E9FF EAE9"            /* èÑèèêÿéççèççéÿêé */
	$"D1D1 D1D1 FFFF E9E8 E8E8 D1E9 FFFF FFEA"            /* ÑÑÑÑÿÿéèèèÑéÿÿÿê */
	$"E9D1 D1E9 FFFF EAD1 D1D1 E9EA FFFF FFFF"            /* éÑÑéÿÿêÑÑÑéêÿÿÿÿ */
	$"EAEA E9E9 EAFF FFEA E9EA FFFF FFFF FFFF"            /* êêééêÿÿêéêÿÿÿÿÿÿ */
	$"FFFF EAEA FFFF FFEA FFFF FFFF FF69 7333"            /* ÿÿêêÿÿÿêÿÿÿÿÿis3 */
	$"3200 0001 0FB1 0001 0101 FF00 C500 8100"            /* 2....±....ÿ.Å.. */
	$"080C 2B3E 3B39 3C31 1C03 8200 0B02 3055"            /* ..+>;9<1..‚...0U */
	$"2D2A 3632 261D 393E 1380 007F 0246 6419"            /* -*62&.9>.€...Fd. */
	$"4D88 8E7A 6B4E 152C 4918 0000 3179 2C35"            /* MˆŽzkN.,I...1y,5 */
	$"8FA3 9984 7768 4709 3E4A 0B0A 6168 0664"            /* £™„whGÆ>J.Âah.d */
	$"9FA2 8D7C 7067 5515 2456 3325 6B51 0168"            /* Ÿ¢|pgU.$V3%kQ.h */
	$"9790 8073 6B64 5317 1C56 4B37 6751 023B"            /* —€skdS..VK7gQ.; */
	$"767B 736A 645F 460A 2257 5538 5A51 120D"            /* v{sjd_FÂ"WU8ZQ.. */
	$"4C65 6560 5E4E 2001 3B5D 5D30 4F53 3A05"            /* Lee`^N .;]]0OS:. */
	$"0A2D 4141 3518 0120 5560 5A1F 3C49 4D34"            /* Â-AA5.. U`Z.<IM4 */
	$"0D01 0102 0005 254F 5D5F 5510 3D2C 3A46"            /* ......%O]_U.=,:F */
	$"4B44 3008 0017 4154 5A5C 5941 0422 3038"            /* KD0...ATZ\YA."08 */
	$"3E44 3F0C 0023 4E53 5253 4A22 000F 282E"            /* >D?..#NSRSJ"..(. */
	$"3137 330A 001C 4246 423E 2B06 0001 1527"            /* 173Â..BFB>+....' */
	$"2D2F 2707 0015 3132 2E21 0980 000B 010F"            /* -/'...12.!Æ€.... */
	$"1E26 2206 000B 1819 1207 8300 0803 0D12"            /* .&".......ƒ..... */
	$"0400 0408 0501 8000 FF00 FB00 7338 6D6B"            /* ......€.ÿ.û.s8mk */
	$"0000 0108 0000 0000 6BC7 FFFF FFFF C96D"            /* ........kÇÿÿÿÿÉm */
	$"0000 0000 0000 34C5 FFA7 A7A7 A797 C3FF"            /* ......4Åÿ§§§§—Ãÿ */
	$"C737 0000 0036 FEFF 97EE FFFF FFFF D3B2"            /* Ç7...6þÿ—îÿÿÿÿÓ² */
	$"FFFF 3900 00C6 FFBA C2FF FFFF FFFF FFC2"            /* ÿÿ9..ÆÿºÂÿÿÿÿÿÿÂ */
	$"C2FF CA00 6FFF FF56 FFFF FFFF FFFF FFFF"            /* ÂÿÊ.oÿÿVÿÿÿÿÿÿÿÿ */
	$"7FFF FF73 CDFF FF4D FFFF FFFF FFFF FFFF"            /* .ÿÿsÍÿÿMÿÿÿÿÿÿÿÿ */
	$"49FF FFD1 FFFF FF3E FFFF FFFF FFFF FFFD"            /* IÿÿÑÿÿÿ>ÿÿÿÿÿÿÿý */
	$"78FF FFFF FFFF FF6A 8DFF FFFF FFFF FF5E"            /* xÿÿÿÿÿÿjÿÿÿÿÿÿ^ */
	$"BFFF FFFF FFFF FFFF 3F86 FFFF FFFF 635C"            /* ¿ÿÿÿÿÿÿÿ?†ÿÿÿÿc\ */
	$"FFFF FFFF FFFF FFFF FF81 0211 0F1B ADFF"            /* ÿÿÿÿÿÿÿÿÿ....­ÿ */
	$"FFFF FFFF D3FF FFFF FFFF FF7F 7FFF FFFF"            /* ÿÿÿÿÓÿÿÿÿÿÿ..ÿÿÿ */
	$"FFFF FFD8 77FF FFFF FFFF FF7F 7FFF FFFF"            /* ÿÿÿØwÿÿÿÿÿÿ..ÿÿÿ */
	$"FFFF FF7A 00D1 FFFF FFFF FF7F 7FFF FFFF"            /* ÿÿÿz.Ñÿÿÿÿÿ..ÿÿÿ */
	$"FFFF D500 003D FFFF FFFF FF7F 7FFF FFFF"            /* ÿÿÕ..=ÿÿÿÿÿ..ÿÿÿ */
	$"FFFF 3F00 0000 3ED4 FFFF FF7F 7FFF FFFF"            /* ÿÿ?...>Ôÿÿÿ..ÿÿÿ */
	$"D73F 0000 0000 0000 7ADA FF7F 7FFF DC7B"            /* ×?......zÚÿ..ÿÜ{ */
	$"0000 0000 6963 6D38 0000 00C8 0000 0000"            /* ....icm8...È.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 6974 3332"            /* ............it32 */
	$"0000 2FF2 0000 0000 FF00 FF00 FF00 FF00"            /* ../ò....ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 F800 8101 F800 0001 8002 0001"            /* ÿ.ÿ.ø..ø...€... */
	$"F700 0501 0203 0302 01F7 0005 0203 0302"            /* ÷........÷...... */
	$"0101 F600 0401 0203 0201 F800 0301 0201"            /* ..ö.......ø..... */
	$"01FF 00FF 00FF 00FF 00FF 00FF 00FF 00FF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
	$"00FF 00FF 00FF 00FF 00FF 00FF 00FF 00FF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
	$"00FF 00FF 00FF 00FF 00FF 00FF 00FF 00FF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
	$"00FF 00FF 00FF 00FF 00FF 00FF 00FF 00FF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
	$"00FF 00FF 00FF 00FF 00FF 00FF 00FF 00FF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
	$"00FF 00FF 00FF 00FF 00FF 00FF 00FF 00FF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
	$"00FF 00FF 00FF 00FF 00FF 00FF 00FF 00FF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
	$"00FF 00FF 00FF 00FF 00FF 00FF 00FF 00FF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
	$"00FF 00FF 00FF 00FF 00FF 00FF 00FF 00FF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
	$"00FF 00FF 00FF 00FF 00FF 00FF 00FF 00FF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
	$"00FF 00FF 00FF 00FF 00FF 00FF 00FF 00FF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
	$"00FF 00FF 00FF 00FF 00FF 00FF 00FF 00FF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
	$"00FF 00FF 0091 00B1 000A 030B 1219 1F22"            /* .ÿ.ÿ.‘.±.Â....." */
	$"2629 2B2C 2C80 2E0A 2D2C 2B27 2421 1C16"            /* &)+,,€.Â-,+'$!.. */
	$"1008 02DE 000F 0101 050F 1C2A 353A 3D3E"            /* ...Þ.......*5:=> */
	$"3E3C 3B3B 3A3A 8139 803A 803B 0A3C 3B38"            /* ><;;::9€:€;Â<;8 */
	$"342E 2319 0D03 0101 D500 0701 010A 1929"            /* 4.#.....Õ....Â.) */
	$"363D 4181 4304 4141 403F 3E83 3D82 3E00"            /* 6=AC.AA@?>ƒ=‚>. */
	$"3F80 3E0A 3D3C 3B3A 362F 2316 0801 01CF"            /* ?€>Â=<;:6/#....Ï */
	$"0016 0102 091B 2E3C 4244 4445 4648 4949"            /* ....Æ..<BDDEFHII */
	$"4847 4544 4442 4040 3F80 4002 4141 4281"            /* HGEDDB@@?€@.AAB */
	$"430F 4242 4141 3F3E 3D3D 3C3A 3529 1707"            /* C.BBAA?>==<:5).. */
	$"0201 CA00 0B01 0514 2A3B 4445 4547 494B"            /* ..Ê.....*;DEEGIK */
	$"4B80 4D07 4C4A 4948 4544 4342 8241 0242"            /* K€M.LJIHEDCB‚A.B */
	$"4243 8044 0945 4444 4544 4343 4140 3F80"            /* BC€DÆEDDEDCCA@?€ */
	$"3D05 3C34 2511 0301 C600 1A02 091D 3543"            /* =.<4%...Æ...Æ.5C */
	$"4647 484A 4C4D 4E4F 504F 4F4E 4D4B 4947"            /* FGHJLMNOPOONMKIG */
	$"4544 4442 4141 8140 0141 4180 4300 4480"            /* EDDBAA@.AA€C.D€ */
	$"4500 4483 4309 4242 4140 403D 2F19 0702"            /* E.DƒCÆBBA@@=/... */
	$"C200 0A02 0921 3B46 4849 4B4D 4E50 8251"            /* Â.Â.Æ!;FHIKMNP‚Q */
	$"0B50 4F4D 4C4A 4847 4545 4442 4184 4003"            /* .POMLJHGEEDBA„@. */
	$"4141 4242 8043 0142 4280 430D 4545 4645"            /* AABB€C.BB€C.EEFE */
	$"4544 4241 413F 331D 0702 BE00 3302 0921"            /* EDBAA?3...¾.3.Æ! */
	$"3C48 4A4B 4D50 5152 5453 5352 5251 5050"            /* <HJKMPQRTSSRRQPP */
	$"4E4D 4C48 433D 362F 2822 1E1C 1C1B 1C1F"            /* NMLHC=6/("...... */
	$"2429 3037 3A3F 4141 4040 4141 4243 4445"            /* $)07:?AA@@AABCDE */
	$"4781 4808 4645 4240 3F33 1B07 01BA 0010"            /* GH.FEB@?3...º.. */
	$"0106 1B3B 494B 4D50 5254 5556 5554 5453"            /* ...;IKMPRTUVUTTS */
	$"5280 5006 4E48 3F2F 2012 078A 001C 030D"            /* R€P.NH?/ ..Š.... */
	$"1723 2F38 3D3E 3E3F 4041 4445 4748 4949"            /* .#/8=>>?@ADEGHII */
	$"4848 4643 403F 3E2F 1504 01B7 0008 0312"            /* HHFC@?>/...·.... */
	$"3349 4C4D 5154 5681 570A 5654 5251 504E"            /* 3ILMQTVWÂVTRQPN */
	$"493A 2612 0293 000C 0918 2733 3A3D 3E40"            /* I:&..“..Æ.'3:=>@ */
	$"4244 4748 4980 4A08 4846 4341 403B 290E"            /* BDGHI€J.HFCA@;). */
	$"03B4 0015 0107 2243 4E4E 5155 5859 5A5A"            /* .´...."CNNQUXYZZ */
	$"5957 5553 5251 4E43 2D13 8600 0402 0304"            /* YWUSRQNC-.†..... */
	$"0506 8105 0204 0301 8500 0A07 1A2D 383C"            /* .......….Â..-8< */
	$"3E40 4244 4749 804B 094A 4845 433F 3E37"            /* >@BDGI€KÆJHEC?>7 */
	$"1D07 01B1 0014 0316 394C 4E52 5659 5B5D"            /* ...±....9LNRVY[] */
	$"5D5B 5A57 5554 524E 3F23 0784 0013 0307"            /* ][ZWUTRN?#.„.... */
	$"0B15 202A 3034 3736 3534 302B 2319 1008"            /* .. *0476540+#... */
	$"0502 8400 1512 2838 3D3F 4143 4648 494A"            /* ..„...(8=?ACFHIJ */
	$"4A49 4846 443F 3E3D 2D0F 02AF 0007 0723"            /* JIHFD?>=-..¯...# */
	$"454F 5056 5A5D 805F 085D 5B57 5453 5043"            /* EOPVZ]€_.][WTSPC */
	$"2405 8300 1902 0711 263A 4B56 5758 5855"            /* $.ƒ.....&:KVWXXU */
	$"5250 4F4D 4C4C 4A46 4238 2A1A 0B05 0183"            /* RPOMLLJFB8*....ƒ */
	$"0014 1129 393D 3E41 4345 4849 494A 4848"            /* ...)9=>ACEHIIJHH */
	$"4542 3F3E 361A 04AC 0013 010E 334D 5154"            /* EB?>6..¬....3MQT */
	$"595D 6062 6260 5E5B 5755 5249 2C09 8300"            /* Y]`bb`^[WURI,Æƒ. */
	$"1303 0C24 4057 6162 615C 5D5B 5958 5654"            /* ...$@Waba\][YXVT */
	$"5250 4D4C 4A81 4805 453D 2C17 0802 8300"            /* RPMLJH.E=,...ƒ. */
	$"1316 2E3A 3D3F 4144 4648 494A 4A49 4745"            /* ...:=?ADFHIJJIGE */
	$"403E 3B25 0AAA 0012 0215 3C51 5257 5D61"            /* @>;%Âª....<QRW]a */
	$"6364 6462 5F5B 5755 503C 1683 0021 030C"            /* cddb_[WUP<.ƒ.!.. */
	$"294B 5E63 6263 6567 6667 6663 615F 5C5A"            /* )K^cbcegfgfca_\Z */
	$"5856 5452 4F4E 4B49 4746 4642 321A 0701"            /* XVTRONKIGFFB2... */
	$"8200 1406 2135 3A3B 3E41 4447 4A4B 4B4A"            /* ‚...!5:;>ADGJKKJ */
	$"4845 423E 3E2D 0E01 A700 1204 1E45 5254"            /* HEB>>-..§....ERT */
	$"5B60 6466 6766 6460 5C57 564D 2C05 8300"            /* [`dfgfd`\WVM,.ƒ. */
	$"230A 254B 5F61 6164 696D 7172 7372 706F"            /* #Â%K_aadimqrsrpo */
	$"6C69 6663 625F 5C5A 5854 5250 4E4A 4644"            /* lifcb_\ZXTRPNJFD */
	$"4442 3217 0683 0008 122C 3839 3B3F 4346"            /* DB2..ƒ...,89;?CF */
	$"4880 4B07 4A47 443F 3D33 1502 A500 1105"            /* H€K.JGD?=3..¥... */
	$"244A 5456 5D63 6769 6968 6562 5D57 5647"            /* $JTV]cgiiheb]WVG */
	$"1E83 000B 0317 3E5B 5F60 656A 6F73 777A"            /* .ƒ....>[_`ejoswz */
	$"807C 187A 7975 726F 6B69 6664 615F 5D59"            /* €|.zyurokifda_]Y */
	$"5755 514E 4A46 4342 3E27 0D01 8200 130A"            /* WUQNJFCB>'..‚..Â */
	$"2534 3739 3E42 4648 4B4C 4C4A 4844 3F3D"            /* %479>BFHKLLJHD?= */
	$"3618 03A3 0011 0729 4E54 5960 6669 6B6C"            /* 6..£...)NTY`fikl */
	$"6A67 635E 5956 4014 8300 2908 2650 5E5F"            /* jgc^YV@.ƒ.).&P^_ */
	$"6269 6F74 797D 8183 8686 8583 817D 7A77"            /* bioty}ƒ††…ƒ}zw */
	$"7370 6D6B 6766 6460 5E5C 5956 514D 4944"            /* spmkgfd`^\YVQMID */
	$"4140 3517 0482 0013 041E 3235 383D 4145"            /* A@5..‚....258=AE */
	$"484B 4C4D 4B49 4540 3D38 1C04 A100 1107"            /* HKLMKIE@=8..¡... */
	$"2C4F 545A 6268 6C6E 6F6D 6965 5F59 563D"            /* ,OTZbhlnomie_YV= */
	$"0F83 002B 0A34 575D 5F67 6D73 787D 8286"            /* .ƒ.+Â4W]_gmsx}‚† */
	$"898C 8E8F 8E8A 8784 807E 7976 7371 6E6C"            /* ‰ŒŽŽŠ‡„€~yvsqnl */
	$"6B67 6462 5F5C 5753 4F4A 4541 3F3A 2005"            /* kgdb_\WSOJEA?: . */
	$"8200 1302 1A30 3438 3C41 4549 4B4D 4E4C"            /* ‚....048<AEIKMNL */
	$"4946 403C 391D 049F 0011 062A 5055 5C64"            /* IF@<9..Ÿ...*PU\d */
	$"696E 7171 6F6B 6760 5A56 3B0D 8300 2D0E"            /* inqqokg`ZV;.ƒ.-. */
	$"3C59 5D61 6971 767D 8286 8A8D 9093 9596"            /* <Y]aiqv}‚†Š“•– */
	$"9590 8D8B 8683 7F7A 7876 7371 6F6D 6966"            /* •‹†ƒ.zxvsqomif */
	$"6562 5D5A 5651 4C47 423E 3B25 0782 0013"            /* eb]ZVQLGB>;%.‚.. */
	$"0118 2E34 373C 4146 494D 4E4E 4D49 4640"            /* ...47<AFIMNNMIF@ */
	$"3C38 1D04 9D00 1104 2850 565D 656B 7073"            /* <8.....(PV]ekps */
	$"7472 6E69 625B 583F 0E83 002F 0E3F 595C"            /* trnib[X?.ƒ./.?Y\ */
	$"626B 7279 7F84 898E 9193 9698 9A9B 9995"            /* bkry.„‰Ž‘“–˜š›™• */
	$"928E 8A86 827E 7C7A 7776 7270 6D6A 6966"            /* ’ŽŠ†‚~|zwvrpmjif */
	$"625E 5B57 524D 4742 3E3B 2707 8200 1302"            /* b^[WRMGB>;'.‚... */
	$"182F 3337 3D42 464A 4E4E 4F4D 4946 403C"            /* ./37=BFJNNOMIF@< */
	$"3719 029B 0011 0223 4F56 5D66 6C71 7576"            /* 7..›...#OV]flquv */
	$"7471 6B64 5D59 4311 8300 310D 3E59 5C64"            /* tqkd]YC.ƒ.1.>Y\d */
	$"6D74 7B82 888C 9193 9698 9A9C 9E9E 9C99"            /* mt{‚ˆŒ‘“–˜šœžžœ™ */
	$"9591 8D88 8581 7F7C 7A78 7574 716E 6C69"            /* •‘ˆ….|zxutqnli */
	$"6563 5F5B 5752 4D47 413C 3A26 0782 0013"            /* ec_[WRMGA<:&.‚.. */
	$"031B 2F33 383E 4347 4B4E 4F4F 4C4A 4540"            /* ../38>CGKNOOLJE@ */
	$"3B35 1701 9900 1101 1C4B 575C 666D 7376"            /* ;5..™....KW\fmsv */
	$"7877 746F 6860 5B4B 1883 0033 0838 575C"            /* xwtoh`[K.ƒ.3.8W\ */
	$"636D 757D 8388 8E92 9698 9A9C 9E9F 9FA0"            /* cmu}ƒˆŽ’–˜šœžŸŸ  */
	$"9E9C 9793 8E8A 8783 807E 7C7A 7676 7371"            /* žœ—“ŽŠ‡ƒ€~|zvvsq */
	$"6F6C 6967 645F 5B57 524D 4641 3C39 2204"            /* oligd_[WRMFA<9". */
	$"8200 1205 2030 333A 4045 494C 4F4F 4E4E"            /* ‚... 03:@EILOONN */
	$"4A44 3F3B 3311 9900 1014 4657 5C66 6D74"            /* JD?;3.™...FW\fmt */
	$"787A 7A78 726B 625D 5225 8300 3505 2E54"            /* xzzxrkb]R%ƒ.5..T */
	$"5B63 6C75 7D84 8A8F 9397 9A9C 9E9F A0A1"            /* [clu}„Š“—šœžŸ ¡ */
	$"A1A2 A09C 9894 908C 8884 817F 7D7B 7977"            /* ¡¢ œ˜”Œˆ„.}{yw */
	$"7572 726E 6B69 6763 5F5B 5751 4C46 403C"            /* urrnkigc_[WQLF@< */
	$"361A 0282 0012 0926 3134 3C42 454A 4D50"            /* 6..‚..Æ&14<BEJMP */
	$"504F 4D49 443C 3A2E 0C97 0011 0C3B 565B"            /* POMID<:..—...;V[ */
	$"656D 7479 7C7C 7A76 6F67 5E59 3406 8300"            /* emty||zvog^Y4.ƒ. */
	$"1120 4E59 606B 747D 848A 8F94 989A 9D9E"            /* . NY`kt}„Š”˜šž */
	$"9FA1 A180 A320 A09D 9994 908C 8984 817F"            /* Ÿ¡¡€£  ™”Œ‰„. */
	$"7E7B 7979 7673 726F 6D6B 6865 615E 5A56"            /* ~{yyvsromkhea^ZV */
	$"514C 453F 3B31 1183 0012 112B 3237 3D43"            /* QLE?;1.ƒ...+27=C */
	$"474C 4E50 504F 4C48 423B 3927 0795 0011"            /* GLNPPOLHB;9'.•.. */
	$"042F 5358 636D 747A 7D7F 7D79 726B 625C"            /* ./SXcmtz}.}yrkb\ */
	$"450F 8300 100E 4258 5E69 737B 838A 8F95"            /* E.ƒ...BX^is{ƒŠ• */
	$"999B 9D9F A1A1 80A3 23A4 A4A1 9E9A 9591"            /* ™›Ÿ¡¡€£#¤¤¡žš•‘ */
	$"8D8A 8682 807E 7C7A 7975 7474 706F 6C6B"            /* Š†‚€~|zyuttpolk */
	$"6764 615E 5A55 4F4A 433D 3929 0782 0013"            /* gda^ZUOJC=9).‚.. */
	$"0219 3034 393F 464B 4E50 5150 4F4C 4740"            /* ..049?FKNPQPOLG@ */
	$"3B38 1E03 9300 1101 1E4D 5560 6973 7A7F"            /* ;8..“....MU`isz. */
	$"8180 7D78 7066 5E53 2483 0012 042F 545B"            /* €}xpf^S$ƒ.../T[ */
	$"6570 7A82 898F 9598 9C9E 9FA1 A1A3 A381"            /* epz‚‰•˜œžŸ¡¡££ */
	$"A422 A09D 9995 918D 8A86 8281 7E7C 7A78"            /* ¤" ™•‘Š†‚~|zx */
	$"7575 7370 6F6E 6B68 6563 615C 5854 4E48"            /* uusponkheca\XTNH */
	$"413C 371C 0182 0012 0725 3235 3B43 484C"            /* A<7..‚...%25;CHL */
	$"4F51 5150 4E4C 4640 3A35 1493 0011 1143"            /* OQQPNLF@:5.“...C */
	$"515B 6670 797E 8383 807B 746B 615B 3907"            /* Q[fpy~ƒƒ€{tka[9. */
	$"8300 1118 4959 606C 767F 888F 9498 9C9E"            /* ƒ...IY`lv.ˆ”˜œž */
	$"9FA1 A2A3 A381 A423 A3A0 9C98 9591 8D8A"            /* Ÿ¡¢££¤#£ œ˜•‘Š */
	$"8682 817E 7C79 7876 7472 716F 6E6C 6967"            /* †‚~|yxvtrqonlig */
	$"6562 5F5B 5752 4C46 3E3B 2E0D 8300 1214"            /* eb_[WRLF>;..ƒ... */
	$"2D33 373E 454A 4F51 5252 504F 4B44 3D3A"            /* -37>EJOQRRPOKD=: */
	$"2F0A 9100 1106 324F 5461 6C75 7C81 8483"            /* /Â‘...2OTalu|„ƒ */
	$"7F79 7067 5F4F 1C83 000E 0533 555C 6772"            /* .ypg_O.ƒ...3U\gr */
	$"7C85 8C92 979A 9E9F A180 A382 A424 A29E"            /* |…Œ’—šžŸ¡€£‚¤$¢ž */
	$"9B97 9490 8C89 8582 807E 7B79 7775 7572"            /* ›—”Œ‰…‚€~{ywuur */
	$"7170 6E6C 6968 6664 615D 5A55 4F4A 423C"            /* qpnlihfda]ZUOJB< */
	$"371F 0282 0013 0520 3135 3B42 484C 4F51"            /* 7..‚... 15;BHLOQ */
	$"5353 504D 4943 3C39 2304 8F00 1201 1F49"            /* SSPMIC<9#.....I */
	$"515C 6671 797F 8283 817D 756B 615A 3705"            /* Q\fqy.‚ƒ}ukaZ7. */
	$"8300 0E19 4958 626D 7881 8A91 969A 9D9F"            /* ƒ...IXbmxŠ‘–šŸ */
	$"A1A2 80A3 82A4 24A0 9C99 9692 8E8B 8884"            /* ¡¢€£‚¤$ œ™–’Ž‹ˆ„ */
	$"827E 7D7A 7975 7573 7271 6F6D 6C6A 6866"            /* ‚~}zyuusrqomljhf */
	$"6461 5E5C 5752 4D46 3F3A 2E0D 8300 1212"            /* da^\WRMF?:..ƒ... */
	$"2B33 383F 454A 4F52 5454 5350 4D48 423B"            /* +38?EJORTTSPMHB; */
	$"3615 8F00 110D 3D4C 5561 6C74 7A7F 8381"            /* 6....=LUaltz.ƒ */
	$"7E78 7066 5D4D 1D83 000E 032E 545B 6873"            /* ~xpf]M.ƒ....T[hs */
	$"7E86 8E94 999D 9FA1 A280 A382 A426 A39E"            /* ~†Ž”™Ÿ¡¢€£‚¤&£ž */
	$"9C98 9591 8C8A 8683 817E 7C79 7775 7571"            /* œ˜•‘ŒŠ†ƒ~|ywuuq */
	$"7170 6E6B 6A6B 6865 6361 5E5C 5854 504A"            /* qpnkjkheca^\XTPJ */
	$"433B 361A 0182 0013 0521 3136 3B42 494E"            /* C;6..‚...!16;BIN */
	$"5255 5454 5350 4D46 3F3A 2E08 8D00 1203"            /* RUTTSPMF?:..... */
	$"2A49 4F5B 666E 757C 8081 807B 746B 6159"            /* *IO[fnu|€€{tkaY */
	$"3907 8300 0F10 4157 606D 7882 8B91 969B"            /* 9.ƒ...AW`mx‚‹‘–› */
	$"9EA0 A1A3 A383 A427 A3A1 9E9A 9693 8F8C"            /* ž ¡££ƒ¤'£¡žš–“Œ */
	$"8984 8280 7D7B 7975 7573 7171 6F6D 6B6A"            /* ‰„‚€}{yuusqqomkj */
	$"6A67 6564 615F 5C59 5550 4B45 3D38 2608"            /* jgeda_\YUPKE=8&. */
	$"8300 1314 2D33 3940 474C 5053 5557 5654"            /* ƒ...-39@GLPSUWVT */
	$"4F4B 443D 391E 018B 0012 0113 414A 535F"            /* OKD=9..‹....AJS_ */
	$"6971 787E 807F 7C77 7066 5C50 2484 000E"            /* iqx~€.|wpf\P$„.. */
	$"204D 5965 727D 868D 9499 9D9F A1A2 A384"            /*  MYer}†”™Ÿ¡¢£„ */
	$"A427 A29F 9D98 9590 8D8A 8782 817F 7B7A"            /* ¤'¢Ÿ˜•Š‡‚.{z */
	$"7775 7471 716F 6E6C 6B6A 6967 6564 6260"            /* wutqqonlkjigedb` */
	$"5E5B 5752 4D47 3F39 3012 8300 1309 2432"            /* ^[WRMG?90.ƒ..Æ$2 */
	$"373E 444B 5053 5557 5756 534F 4941 3B32"            /* 7>DKPSUWWVSOIA;2 */
	$"0D8B 0012 042E 464D 5963 6C73 797D 7D7C"            /* .‹....FMYclsy}}| */
	$"7873 6A60 5944 1283 000F 0431 545D 6976"            /* xsj`YD.ƒ...1T]iv */
	$"818A 9096 9B9E A0A2 A3A3 84A4 28A1 9D9A"            /* Š–›ž ¢££„¤(¡š */
	$"9693 8E8B 8884 827F 7D7A 7976 7572 7170"            /* –“Ž‹ˆ„‚.}zyvurqp */
	$"6D6C 6A6B 6A67 6564 6463 615F 5C59 544E"            /* mljkjgeddca_\YTN */
	$"4841 3934 1C01 8200 1401 1A2F 353B 424A"            /* HA94..‚..../5;BJ */
	$"4F53 5657 5858 5551 4C45 3E38 2402 8900"            /* OSVWXXUQLE>8$.‰. */
	$"1301 1640 4853 5D66 6E74 787C 7B79 756E"            /* ...@HS]fntx|{yun */
	$"655B 5334 0483 000E 0E3E 5760 6D79 838C"            /* e[S4.ƒ...>W`myƒŒ */
	$"9398 9C9F A1A3 A383 A42A A3A3 A09C 9896"            /* “˜œŸ¡££ƒ¤*££ œ˜– */
	$"908D 8B87 8281 7F7B 7977 7673 7171 6D6C"            /* ‹‡‚.{ywvsqqml */
	$"6B6A 6968 6665 6563 6362 5F5C 5955 5049"            /* kjihfeeccb_\YUPI */
	$"423A 3523 0683 0013 122B 343A 4148 4E53"            /* B:5#.ƒ...+4:AHNS */
	$"5658 585A 5754 4F48 413A 340F 8900 1204"            /* VXXZWTOHA:4.‰... */
	$"3044 4C56 5F68 7074 797A 7A77 7169 6057"            /* 0DLV_hptyzzwqi`W */
	$"4C24 8400 0E17 4759 6370 7C86 8E95 9A9E"            /* L$„...GYcp|†Ž•šž */
	$"A0A1 A3A3 83A4 2AA3 A19E 9996 938E 8B89"            /*  ¡££ƒ¤*£¡ž™–“Ž‹‰ */
	$"8482 7F7D 7A79 7574 7170 706C 6B6A 6A69"            /* „‚.}zyutqpplkjji */
	$"6766 6565 6463 625E 5C59 5550 4942 3A36"            /* gfeedcb^\YUPIB:6 */
	$"280B 8300 1409 2532 373F 464D 5256 5859"            /* (.ƒ..Æ%27?FMRVXY */
	$"5A58 5652 4C46 3D38 2503 8800 1215 3F46"            /* ZXVRLF=8%.ˆ...?F */
	$"505A 6269 7074 7879 7874 6D65 5B54 4316"            /* PZbiptxyxtme[TC. */
	$"8400 0E20 4E59 6673 7F88 9096 9B9E A1A1"            /* „.. NYfs.ˆ–›ž¡¡ */
	$"A3A3 83A4 1AA2 9E9A 9794 918D 8A86 8281"            /* ££ƒ¤.¢žš—”‘Š†‚ */
	$"7F7B 7977 7574 7170 6E6C 6B6A 6A69 6766"            /* .{ywutqpnlkjjigf */
	$"8065 0C64 605F 5D59 554F 4943 3B35 2C0F"            /* €e.d`_]YUOIC;5,. */
	$"8300 1402 1F31 363D 464C 5156 5859 5A5A"            /* ƒ....16=FLQVXYZZ */
	$"5652 4E48 4039 330F 8700 1303 2C42 4954"            /* VRNH@93.‡...,BIT */
	$"5D65 6B71 7377 7675 716A 6157 5038 0B83"            /* ]ekqswvuqjaWP8.ƒ */
	$"000E 0129 525B 6976 818A 9197 9C9F A1A3"            /* ...)R[ivŠ‘—œŸ¡£ */
	$"A383 A41A A29E 9B98 9592 8F8B 8884 827F"            /* £ƒ¤.¢ž›˜•’‹ˆ„‚. */
	$"7D7A 7976 7572 706F 6C6C 6A69 6867 6680"            /* }zyvurpolljihgf€ */
	$"650D 6462 605F 5C59 5651 4B45 3C35 2E14"            /* e.db`_\YVQKE<5.. */
	$"8400 1419 2F35 3C44 4B50 5558 5A59 5958"            /* „.../5<DKPUXZYYX */
	$"5450 4A44 3C36 2301 8600 1310 3B43 4C57"            /* TPJD<6#.†...;CLW */
	$"5F67 6D72 7476 7571 6D65 5E53 4C2F 0283"            /* _gmrtvuqme^SL/.ƒ */
	$"000E 052E 545D 6B77 838C 9398 9C9F A1A3"            /* ....T]kwƒŒ“˜œŸ¡£ */
	$"A382 A41B A1A0 9C99 9793 908C 8986 8281"            /* £‚¤.¡ œ™—“Œ‰†‚ */
	$"7F7B 7977 7674 7170 6D6B 6A6A 6968 6766"            /* .{ywvtqpmkjjihgf */
	$"8065 0D64 625F 5E5C 5A56 514B 453C 362F"            /* €e.db_^\ZVQKE<6/ */
	$"1784 0014 122C 353C 454A 5053 565B 5A59"            /* .„...,5<EJPSV[ZY */
	$"5957 524C 463E 3730 0B85 0013 0126 4046"            /* YWRLF>70.…...&@F */
	$"505A 6168 6E71 7475 736F 6A62 5A50 4726"            /* PZahnqtusojbZPG& */
	$"8400 0E08 3355 5F6C 7884 8C93 999D A0A1"            /* „...3U_lx„Œ“™ ¡ */
	$"A3A3 80A4 1DA3 A19F 9D99 9794 908E 8A88"            /* ££€¤.£¡Ÿ™—”ŽŠˆ */
	$"8481 7F7D 7A79 7574 7270 6F6D 6B6A 6A69"            /* „.}zyutrpomkjji */
	$"6767 6680 650E 6461 5F5E 5C59 5550 4A44"            /* ggf€e.da_^\YUPJD */
	$"3D35 2F1A 0283 0014 0D29 363C 4349 4F53"            /* =5/..ƒ...)6<CIOS */
	$"5759 5A5A 5857 524F 4841 3934 1C85 0013"            /* WYZZXWROHA94.….. */
	$"0835 4049 535B 636A 6F72 7373 726E 6861"            /* .5@IS[cjorssrnha */
	$"574E 431E 8400 2E0A 3756 5F6D 7984 8D94"            /* WNC.„..Â7V_my„” */
	$"9A9D A0A1 A3A3 A4A4 A2A0 9E9C 9896 9490"            /* š ¡££¤¤¢ žœ˜–” */
	$"8E8C 8985 8280 7E7B 7977 7573 7170 6E6C"            /* ŽŒ‰…‚€~{ywusqpnl */
	$"6B6A 6A68 6767 8165 0E63 625F 5D5C 5954"            /* kjjhgge.cb_]\YT */
	$"504A 433C 352F 1B04 8300 1509 2736 3C43"            /* PJC<5/..ƒ..Æ'6<C */
	$"494F 5356 595A 5A58 5654 504A 443C 352A"            /* IOSVYZZXVTPJD<5* */
	$"0684 0013 193C 424D 565E 646B 6F72 7373"            /* .„...<BMV^dkorss */
	$"706C 655E 554D 3F17 8400 2E0A 3757 606D"            /* ple^UM?.„..Â7W`m */
	$"7A84 8D94 9A9D A0A1 A3A2 A2A1 9F9D 9B98"            /* z„”š ¡£¢¢¡Ÿ›˜ */
	$"9694 918E 8D8A 8683 817F 7C7A 7876 7572"            /* –”‘ŽŠ†ƒ.|zxvur */
	$"706F 6D6B 6A6A 6968 6766 8065 0F64 6262"            /* pomkjjihgf€e.dbb */
	$"5F5E 5C59 5450 4A43 3D35 301B 0483 0015"            /* _^\YTPJC=50..ƒ.. */
	$"0624 363C 4249 4F53 5658 5A5B 5A57 5450"            /* .$6<BIOSVXZ[ZWTP */
	$"4C46 4037 3214 8300 1403 2A3D 454F 5861"            /* LF@72.ƒ...*=EOXa */
	$"666C 6F72 7373 706B 645C 534B 3C14 8400"            /* florsspkd\SK<.„. */
	$"2E0A 3756 5F6D 7984 8C94 999C 9E9F A09F"            /* .Â7V_my„Œ”™œžŸ Ÿ */
	$"9F9D 9C99 9895 9491 8E8C 8A87 8482 807E"            /* Ÿœ™˜•”‘ŽŒŠ‡„‚€~ */
	$"7B79 7775 7371 706E 6C6B 6A6A 6968 6766"            /* {ywusqpnlkjjihgf */
	$"8065 0F62 6161 5F5D 5B58 544F 4A43 3C34"            /* €e.baa_][XTOJC<4 */
	$"2F1C 0383 0016 0422 363C 4349 4F54 5658"            /* /..ƒ..."6<CIOTVX */
	$"5B5B 5A59 5552 4E48 413A 3323 0182 0014"            /* [[ZYURNHA:3#.‚.. */
	$"0D35 3E48 5259 6168 6D70 7171 706E 6A63"            /* .5>HRYahmpqqpnjc */
	$"5B51 4A39 1184 002D 0833 535C 6976 8189"            /* [QJ9.„.-.3S\iv‰ */
	$"9095 989A 9B9C 9B9B 9997 9593 9290 8E8C"            /* •˜š›œ››™—•“’ŽŒ */
	$"8A88 8482 817F 7C7A 7875 7471 706F 6D6B"            /* Šˆ„‚.|zxutqpomk */
	$"6A6A 6968 6766 8065 1064 6161 605F 5D5B"            /* jjihgf€e.daa`_][ */
	$"5753 4E4A 433B 342E 1A03 8300 1602 2135"            /* WSNJC;4...ƒ...!5 */
	$"3C44 4A4F 5456 585B 5A5B 5A57 5350 4A44"            /* <DJOTVX[Z[ZWSPJD */
	$"3D34 2D09 8200 141C 393F 4A54 5C63 676C"            /* =4-Æ‚...9?JT\cgl */
	$"7172 7270 6C67 615A 514A 3810 8400 0D06"            /* qrrplgaZQJ8.„... */
	$"2F4F 5966 727D 858B 9193 9798 9780 9615"            /* /OYfr}…‹‘“—˜—€–. */
	$"9392 918E 8D8B 8A87 8582 817F 7D7B 7977"            /* “’‘Ž‹Š‡…‚.}{yw */
	$"7573 716F 6E6C 806A 0268 6766 8165 1063"            /* usqonl€j.hgfe.c */
	$"6261 5F5E 5D5A 5653 4E49 433B 342D 1801"            /* ba_^]ZVSNIC;4-.. */
	$"8300 1602 2136 3C43 494F 5456 595A 595B"            /* ƒ...!6<CIOTVYZY[ */
	$"5958 5551 4C46 3F37 3116 8100 1502 293A"            /* YXUQLF?71....): */
	$"424C 555C 6469 6C6F 7271 706C 6761 5950"            /* BLU\dilorqplgaYP */
	$"4838 1084 002B 0228 4B55 616D 7880 868C"            /* H8.„.+.(KUamx€†Œ */
	$"8E91 9393 9292 918F 8E8D 8B8A 8987 8582"            /* Ž‘““’’‘Ž‹Š‰‡…‚ */
	$"8180 7D7C 7977 7574 7270 6E6D 6B6A 6A69"            /* €}|ywutrpnmkjji */
	$"6766 8065 1164 6362 6161 5F5D 5C59 5652"            /* gf€e.dcbaa_]\YVR */
	$"4E49 413A 332C 1584 000A 0221 363C 444A"            /* NIA:3,.„.Â.!6<DJ */
	$"4F54 5759 5A80 5B09 5856 524D 4841 3933"            /* OTWYZ€[ÆXVRMHA93 */
	$"2201 8000 150A 323A 454E 565D 646A 6D6F"            /* ".€..Â2:ENV]djmo */
	$"7070 6F6C 6761 594F 4837 1085 000A 2047"            /* ppolgaYOH7.….Â G */
	$"515C 6772 7A82 878B 8C80 8D80 8C18 8B8A"            /* Q\grz‚‡‹Œ€€Œ.‹Š */
	$"8987 8584 8281 807F 7C7A 7876 7472 716F"            /* ‰‡…„‚€.|zxvtrqo */
	$"6D6C 6A6A 6968 6680 6512 6462 6261 6060"            /* mljjihf€e.dbba`` */
	$"5F5D 5B58 5451 4C46 4039 312A 1184 0017"            /* _][XTQLF@91*.„.. */
	$"0221 363D 444B 4F54 5758 595C 5B5A 5956"            /* .!6=DKOTWXY\[ZYV */
	$"544F 4A43 3B34 2A08 8000 1515 363C 464F"            /* TOJC;4*.€...6<FO */
	$"575F 646A 6E6F 706F 6D6C 6760 5950 4838"            /* W_djnopomlg`YPH8 */
	$"1285 000D 1740 4C56 616C 747C 8185 8789"            /* .…...@LValt|…‡‰ */
	$"8A8A 8089 1787 8685 8482 8181 807D 7D7B"            /* ŠŠ€‰.‡†…„‚€}}{ */
	$"7977 7573 7170 6E6C 6A6A 6968 6681 6580"            /* ywusqpnljjihfe€ */
	$"620F 6060 5F5E 5C58 5754 504A 453E 3730"            /* b.``_^\XWTPJE>70 */
	$"260C 8400 1704 2336 3D44 4B50 5456 585A"            /* &.„...#6=DKPTVXZ */
	$"5B5B 5A5A 5755 514C 453E 3630 1380 0015"            /* [[ZZWUQLE>60.€.. */
	$"2037 3E47 5059 6065 696D 6F70 6F6D 6A66"            /*  7>GPY`eimopomjf */
	$"6059 5149 3A15 8500 0D0F 3749 525C 676F"            /* `YQI:.…...7IR\go */
	$"767C 8083 8486 8680 8516 8483 8282 8180"            /* v|€ƒ„††€….„ƒ‚‚€ */
	$"7F7D 7D7A 7977 7574 7270 6F6D 6A6A 6968"            /* .}}zywutrpomjjih */
	$"6681 6513 6362 6261 6060 5E5D 5A58 5653"            /* fe.cbba``^]ZXVS */
	$"4F48 433C 342F 2208 8400 3005 2537 3D45"            /* OHC<4/".„.0.%7=E */
	$"4B50 5456 585A 5A5B 5B5A 5856 524D 4740"            /* KPTVXZZ[[ZXVRMG@ */
	$"3832 1D00 0004 2A37 4049 5059 5E64 696C"            /* 82....*7@IPY^dil */
	$"6D6E 6F6D 6A66 5F58 5048 3D1A 8500 1207"            /* mnomjf_XPH=.…... */
	$"2B44 4C56 6069 7075 7A7D 7F81 8182 8183"            /* +DLV`ipuz}.‚ƒ */
	$"8281 8080 107F 7D7C 7B79 7876 7472 716F"            /* ‚€€..}|{yxvtrqo */
	$"6D6B 6A69 6866 8165 1463 6262 6161 6060"            /* mkjihfe.cbbaa`` */
	$"5E5B 5957 5551 4C45 4039 322C 1A02 8400"            /* ^[YWUQLE@92,..„. */
	$"3009 2838 3D45 4A4F 5356 5859 5A5B 5A59"            /* 0Æ(8=EJOSVXYZ[ZY */
	$"5857 534E 4942 3A33 2602 000B 2E38 414A"            /* XWSNIB:3&....8AJ */
	$"5158 5E64 686B 6C6D 6D6C 6A65 6059 5047"            /* QX^dhklmmlje`YPG */
	$"3F20 8500 0E01 1F3F 4751 5B64 6B72 767A"            /* ? …....?GQ[dkrvz */
	$"7D7E 7E7F 8080 127F 7F7D 7E7D 7C7B 7978"            /* }~~.€€...}~}|{yx */
	$"7675 7371 6F6E 6B6A 6968 8065 1664 6363"            /* vusqonkjih€e.dcc */
	$"6262 6161 6060 5E5E 5A58 5652 4E49 433D"            /* bbaa``^^ZXVRNIC= */
	$"362F 2912 8500 0A0C 2B38 3E45 4A4F 5456"            /* 6/).….Â.+8>EJOTV */
	$"5859 805B 235A 5857 5450 4A44 3C35 2C08"            /* XY€[#ZXWTPJD<5,. */
	$"0013 3038 414A 5158 5E64 6668 6C6D 6B6A"            /* ..08AJQX^dfhlmkj */
	$"6864 6059 5148 4025 0185 000D 1234 424B"            /* hd`YQH@%.…...4BK */
	$"545D 666D 7276 797B 7B7C 817D 107C 7C7B"            /* T]fmrvy{{|}.||{ */
	$"7A79 7877 7575 7471 706E 6C6B 6968 8065"            /* zyxwuutqpnlkih€e */
	$"0064 8262 1161 6160 5F5E 5C5A 5754 4F4A"            /* .d‚b.aa`_^\ZWTOJ */
	$"4540 3A33 2E22 0A85 0031 112E 383F 454A"            /* E@:3."Â….1..8?EJ */
	$"4F53 5657 595B 5C5B 5A59 5855 514B 463F"            /* OSVWY[\[ZYXUQKF? */
	$"3731 1100 1A31 3841 4A50 585E 6265 6869"            /* 71...18AJPX^behi */
	$"6A69 6868 635F 5852 4941 2B06 8500 2106"            /* jihhc_XRIA+.….!. */
	$"273E 454F 5860 686C 7276 7779 7A7B 7C7B"            /* '>EOX`hlrvwyz{|{ */
	$"7B7A 7A79 7978 7675 7574 7270 6F6C 6B6A"            /* {zzyyxvuutrpolkj */
	$"6880 6500 6483 6211 6161 605F 5E5A 5856"            /* h€e.dƒb.aa`_^ZXV */
	$"524D 4844 3D37 2F2B 1902 8500 0A18 3138"            /* RMHD=7/+..….Â.18 */
	$"3F46 4B50 5356 585A 825B 2158 5652 4D47"            /* ?FKPSVXZ‚[!XVRMG */
	$"4139 3318 0122 3038 4049 5057 5C60 6366"            /* A93.."08@IPW\`cf */
	$"6668 6767 6562 5F58 5149 4233 0E86 000D"            /* fhggeb_XQIB3.†.. */
	$"1737 3F48 525A 6267 6D71 7376 7878 8179"            /* .7?HRZbgmqsvxxy */
	$"1178 7777 7575 7473 7170 6F6C 6B6A 6866"            /* .xwwuutsqpolkjhf */
	$"6565 6381 6282 610E 5F5E 5B59 5754 4E4A"            /* eecb‚a._^[YWTNJ */
	$"453F 3932 2D25 0E85 0032 011F 3339 4047"            /* E?92-%.….2..39@G */
	$"4C50 5456 585A 5B5B 5C5C 5B5A 5854 4F49"            /* LPTVXZ[[\\[ZXTOI */
	$"433B 3421 0325 3039 4148 4F54 5A5E 6063"            /* C;4!.%09AHOTZ^`c */
	$"6567 6665 6361 5D57 5149 4237 1786 0024"            /* egfeca]WQIB7.†.$ */
	$"0A2B 3C43 4C54 5C62 686B 7072 7476 7677"            /* Â+<CLT\bhkprtvvw */
	$"7876 7675 7574 7372 7170 6F6C 6B6A 6966"            /* xvvuutsrqpolkjif */
	$"6565 6362 6284 610F 605E 5E5B 5956 514B"            /* eecbb„a.`^^[YVQK */
	$"4841 3B34 2E2A 1C04 8500 3208 2635 3A42"            /* HA;4.*..….2.&5:B */
	$"484D 5255 5859 5A5C 5B5D 5D5C 5A58 5551"            /* HMRUXYZ\[]]\ZXUQ */
	$"4C45 3D36 2808 272F 3940 474F 5457 5B5E"            /* LE=6(.'/9@GOTW[^ */
	$"6062 6363 6262 605D 5852 4A42 3A22 8700"            /* `bccbb`]XRJB:"‡. */
	$"2219 353D 464E 565D 6267 6B6E 7172 7374"            /* ".5=FNV]bgknqrst */
	$"7575 7473 7372 7170 706E 6C6B 6A69 6765"            /* uutssrqppnlkjige */
	$"6564 6262 8361 1060 605F 5E5D 5A56 524D"            /* edbbƒa.``_^]ZVRM */
	$"4943 3D36 302A 230F 8600 3310 2D36 3C43"            /* IC=60*#.†.3.-6<C */
	$"4A4E 5356 5859 5A5C 5C5E 5E5D 5B59 5654"            /* JNSVXYZ\\^^][YVT */
	$"4E47 3F37 2B0C 282F 3740 474E 5356 5A5D"            /* NG?7+.(/7@GNSVZ] */
	$"5E5E 5F61 6061 5F5C 5852 4B43 3D2C 0986"            /* ^^_a`a_\XRKC=,Æ† */
	$"0021 0827 393F 4850 575E 6267 6B6E 7070"            /* .!.'9?HPW^bgknpp */
	$"7172 7372 7171 706F 6E6E 6B6B 6A69 6765"            /* qrsrqqponnkkjige */
	$"6564 6262 8261 1262 6060 5F5E 5D5A 5753"            /* edbb‚a.b``_^]ZWS */
	$"4F4A 4540 3833 2C28 1903 8600 0D1A 3138"            /* OJE@83,(..†...18 */
	$"3F45 4B50 5357 585A 5B5D 5D80 5E15 5C5A"            /* ?EKPSWXZ[]]€^.\Z */
	$"5955 5049 413A 2D0F 2930 383E 454C 5256"            /* YUPIA:-.)08>ELRV */
	$"595A 5D5E 805F 095E 5C5A 5651 4A44 3C32"            /* YZ]^€_Æ^\ZVQJD<2 */
	$"1487 000F 1331 3B41 4951 565D 6165 6A6C"            /* .‡...1;AIQV]aejl */
	$"6E6E 6F70 806F 026E 6D6B 806A 0668 6665"            /* nnop€o.nmk€j.hfe */
	$"6564 6262 8261 1262 6060 5F5E 5D5B 5855"            /* edbb‚a.b``_^][XU */
	$"514B 4640 3A34 2D28 200B 8600 0F08 2535"            /* QKF@:4-( .†...%5 */
	$"3A41 474C 5154 575A 5B5D 5E5E 5F80 5E22"            /* :AGLQTWZ[]^^_€^" */
	$"5D5A 5650 4B43 3B2E 1229 2F36 3E45 4A4F"            /* ]ZVPKC;..)/6>EJO */
	$"5457 595C 5C5D 5E5D 5C5B 5955 514B 443D"            /* TWY\\]^]\[YUQKD= */
	$"3721 0286 0010 011E 353B 434A 5057 5C60"            /* 7!.†....5;CJPW\` */
	$"6467 6A6B 6C6D 6D80 6B0E 6A6A 6968 6666"            /* dgjklmm€k.jjihff */
	$"6564 6362 6161 6060 6180 6011 5E5F 5D5C"            /* edcbaa``a€`.^_]\ */
	$"5B58 5451 4C47 413C 352F 2923 1301 8600"            /* [XTQLGA<5/)#..†. */
	$"2514 2E37 3D44 4A4F 5356 595B 5C5E 5F5F"            /* %..7=DJOSVY[\^__ */
	$"6060 5E5F 5E5A 5652 4D45 3D2F 1429 2F36"            /* ``^_^ZVRME=/.)/6 */
	$"3D45 4A4E 5155 5880 5B80 5C09 5B58 5551"            /* =EJNQUX€[€\Æ[XUQ */
	$"4B45 3E38 2C0E 8700 1B08 2635 3B42 4A50"            /* KE>8,.‡...&5;BJP */
	$"555B 5F62 6567 6868 6969 6868 6967 6766"            /* U[_beghhiihhiggf */
	$"6565 6462 6280 6100 5F80 6012 5E5E 5D5D"            /* eedbb€a._€`.^^]] */
	$"5B59 5755 524D 4742 3D37 3029 2618 0486"            /* [YWURMGB=70)&..† */
	$"000E 0422 353A 4147 4D51 5558 5B5D 5E5F"            /* ..."5:AGMQUX[]^_ */
	$"5F82 6022 5E5B 5853 4D45 3D31 1628 2E36"            /* _‚`"^[XSME=1.(.6 */
	$"3D43 494E 5154 5658 5A5B 5B5A 5A59 5855"            /* =CINQTVXZ[[ZZYXU */
	$"524D 4740 3934 1D88 000E 0E2B 363B 4248"            /* RMG@94.ˆ...+6;BH */
	$"4E54 585C 5F62 6364 6580 6600 6581 6304"            /* NTX\_bcde€f.ec. */
	$"6261 6060 5F80 6014 5F5E 5E5D 5D5C 5B58"            /* ba``_€`._^^]]\[X */
	$"5754 514D 4743 3E38 312B 271D 0887 0029"            /* WTQMGC>81+'..‡.) */
	$"102D 373D 444A 5054 585A 5C5D 5F5F 6061"            /* .-7=DJPTXZ\]__`a */
	$"6060 6160 5F5B 5954 4E46 3E31 1828 2E35"            /* ``a`_[YTNF>1.(.5 */
	$"3D43 484D 5154 5556 5859 805A 0A58 5755"            /* =CHMQTUVXY€ZÂXWU */
	$"524D 4842 3C36 2B0D 8800 1412 2C35 3A41"            /* RMHB<6+.ˆ...,5:A */
	$"474C 5155 585C 5E60 6161 6362 6162 6261"            /* GLQUX\^`aacbabba */
	$"805F 835E 135D 5D5C 5C5A 5857 5452 4F4A"            /* €_ƒ^.]]\\ZXWTROJ */
	$"4643 3E38 322C 271F 0B87 000F 0320 353A"            /* FC>82,'..‡... 5: */
	$"4248 4E52 5659 5B5D 5E5F 6060 8161 1660"            /* BHNRVY[]^_``a.` */
	$"5F5B 5854 4F47 3F32 1927 2E35 3B42 474B"            /* _[XTOG?2.'.5;BGK */
	$"5053 5456 5657 8058 0B57 5655 524E 4A44"            /* PSTVVW€X.WVURNJD */
	$"3F37 331D 0188 000E 132C 3338 3F44 494F"            /* ?73..ˆ...,38?DIO */
	$"5255 585A 5C5D 5E81 5F80 5E83 5D13 5C5C"            /* RUXZ\]^_€^ƒ].\\ */
	$"5A5A 5857 5453 504D 4946 423D 3732 2C27"            /* ZZXWTSPMIFB=72,' */
	$"210D 8800 0F12 2E38 3E45 4B50 5458 5A5C"            /* !.ˆ....8>EKPTXZ\ */
	$"5E5F 5F60 6080 6118 6060 5F5B 5854 4F47"            /* ^__``€a.``_[XTOG */
	$"3F32 1C26 2D34 3A41 474B 4E52 5354 5657"            /* ?2.&-4:AGKNRSTVW */
	$"5680 570A 5654 5250 4C47 423B 362E 1189"            /* V€WÂVTRPLGB;6..‰ */
	$"000D 142A 3236 3C41 464A 4E51 5456 5759"            /* ...*26<AFJNQTVWY */
	$"825B 025C 5B5B 815A 8059 1057 5554 5250"            /* ‚[.\[[Z€Y.WUTRP */
	$"4D4B 4643 3F3B 3631 2B27 200D 8800 0F06"            /* MKFC?;61+' .ˆ... */
	$"2436 3B42 494E 5357 5A5C 5E5E 6061 6081"            /* $6;BINSWZ\^^`a` */
	$"6127 6060 5F5C 5954 4F47 4032 1C25 2C34"            /* a'``_\YTOG@2.%,4 */
	$"3A40 454B 4D50 5254 5656 5757 5655 5554"            /* :@EKMPRTVVWWVUUT */
	$"5250 4D4A 453F 3935 2406 8900 1010 2830"            /* RPMJE?95$.‰...(0 */
	$"3438 3E42 474A 4D50 5153 5455 5657 8058"            /* 48>BGJMPQSTUVW€X */
	$"0057 8256 1155 5453 514F 4D4A 4844 403D"            /* .W‚V.UTSQOMJHD@= */
	$"3934 302B 271F 0B89 000C 1732 383F 464C"            /* 940+'..‰...28?FL */
	$"5155 585B 5D5D 5F84 6015 5F5E 5E5C 5852"            /* QUX[]]_„`._^^\XR */
	$"4E46 4032 1925 2B32 383F 4448 4C4F 5054"            /* NF@2.%+28?DHLOPT */
	$"8055 0E57 5656 5554 5251 4F4C 4842 3D38"            /* €U.WVVUTRQOLHB=8 */
	$"321A 8A00 290B 212C 3135 3A3E 4245 484B"            /* 2.Š.).!,15:>BEHK */
	$"4D4F 4F52 5253 5453 5453 5252 5150 4F4E"            /* MOORRSTSTSRRQPON */
	$"4E4C 4A46 4440 3D3A 3631 2D29 251A 0789"            /* NLJFD@=:61-)%..‰ */
	$"000E 0F2C 373C 4349 4E53 575A 5C5E 5F60"            /* ...,7<CINSWZ\^_` */
	$"6181 602A 5F60 5F5E 5B5B 5852 4E46 3F31"            /* a`*_`_^[[XRNF?1 */
	$"1624 2A31 383D 4347 4A4E 5052 5354 5556"            /* .$*18=CGJNPRSTUV */
	$"5756 5554 5351 504F 4A46 413C 372E 118A"            /* WVUTSQPOJFA<7..Š */
	$"000F 051A 292E 3236 3A3D 4144 4648 4A4B"            /* ....).26:=ADFHJK */
	$"4D4D 804E 004D 814C 0F4A 4948 4442 3F3C"            /* MM€N.ML.JIHDB?< */
	$"3A36 322F 2A27 2214 0389 000D 0623 3639"            /* :62/*'"..‰...#69 */
	$"4047 4D51 5559 5B5D 5E5F 8360 805F 285D"            /* @GMQUY[]^_ƒ`€_(] */
	$"5B59 5752 4C45 3E31 1423 282F 363B 4146"            /* [YWRLE>1.#(/6;AF */
	$"494C 4F50 5152 5355 5655 5655 5352 5151"            /* ILOPQRSUVUVUSRQQ */
	$"4E49 4540 3937 290B 8B00 0D0D 212A 2E32"            /* NIE@97).‹...!*.2 */
	$"3538 3C3E 4143 4545 4683 470F 4644 4341"            /* 58<>ACEEFƒG.FDCA */
	$"3F3D 3B38 3532 2E2B 2724 1B0A 8A00 0F03"            /* ?=;852.+'$.ÂŠ... */
	$"1D33 383E 454B 4F54 575B 5C5E 5F5F 6083"            /* .38>EKOTW[\^__`ƒ */
	$"5F1D 5E5E 5C5B 5855 504B 443D 3111 2227"            /* _.^^\[XUPKD=1."' */
	$"2D33 393E 4447 4A4C 4F50 5152 5254 5655"            /* -39>DGJLOPQRRTVU */
	$"8053 0A52 5150 4C49 453F 3935 2307 8B00"            /* €SÂRQPLIE?95#.‹. */
	$"0D03 1323 2A2E 3134 3639 3B3D 3E3F 3F80"            /* ...#*.1469;=>??€ */
	$"4010 3F3E 3D3C 3B39 3836 3331 2D2B 2825"            /* @.?>=<;98631-+(% */
	$"1D0E 028A 000E 0117 3137 3C43 494E 5256"            /* ...Š....17<CINRV */
	$"595C 5E5F 5E84 5F80 5E2A 5C5A 5754 4F4A"            /* Y\^_^„_€^*\ZWTOJ */
	$"433C 300F 2126 2B31 373C 4145 484A 4D4E"            /* C<0.!&+17<AEHJMN */
	$"5050 5252 5353 5453 5352 5151 4E4B 4743"            /* PPRRSSTSSRQQNKGC */
	$"3E38 3420 058C 000A 0312 2028 2C2F 3133"            /* >84 .Œ.Â.. (,/13 */
	$"3436 3780 380F 3737 3635 3433 3130 2E2C"            /* 467€8.77654310., */
	$"2927 221B 0D02 8B00 1001 142F 373B 4147"            /* )'"...‹..../7;AG */
	$"4C51 5558 5A5C 5E5E 5F60 835F 805E 2B5C"            /* LQUXZ\^^_`ƒ_€^+\ */
	$"5957 534E 4942 3A2E 0C1F 232A 3035 393F"            /* YWSNIB:...#*059? */
	$"4245 484A 4C4E 4E4F 5051 5251 5353 5252"            /* BEHJLNNOPQRQSSRR */
	$"5050 4E4A 4641 3B36 331D 048D 0019 010D"            /* PPNJFA;63...... */
	$"1922 282C 2D2E 2F30 3131 2F2F 2E2E 2D2B"            /* ."(,-./011//..-+ */
	$"2A29 2723 1D15 0A01 8C00 1001 122E 373A"            /* *)'#..Â.Œ.....7: */
	$"4046 4B4F 5357 5A5B 5D5E 5E60 815F 825E"            /* @FKOSWZ[]^^`_‚^ */
	$"2D5C 5C59 5753 4E49 4139 2D09 1D22 282D"            /* -\\YWSNIA9-Æ."(- */
	$"3338 3C3F 4345 4849 4B4B 4D4E 4E50 5251"            /* 38<?CEHIKKMNNPRQ */
	$"5153 5252 5150 4D49 453F 3A36 321D 058F"            /* QSRRQPMIE?:62.. */
	$"0013 020B 131B 2023 2527 2827 2726 2422"            /* ...... #%'(''&$" */
	$"201C 170F 0801 8E00 1102 132F 3839 3F45"            /*  .....Ž..../89?E */
	$"494E 5255 595B 5D5E 5F5F 5E82 5F32 5E5F"            /* INRUY[]^__^‚_2^_ */
	$"5E5E 5D5B 5856 524C 4740 382C 061A 2026"            /* ^^][XVRLG@8,.. & */
	$"2B30 363A 3D40 4344 4649 494B 4C4D 4E50"            /* +06:=@CDFIIKLMNP */
	$"5052 5354 5351 504E 4B47 433E 3934 311E"            /* PRSTSQPNKGC>941. */
	$"0792 000A 0105 090C 0D0E 0D0D 0B08 0592"            /* .’.Â..Æ........’ */
	$"0013 0216 3037 3A40 4449 4D50 5558 5A5C"            /* ....07:@DIMPUXZ\ */
	$"5D5E 5F60 5F60 835F 325E 5E5D 5B58 5651"            /* ]^_`_`ƒ_2^^][XVQ */
	$"4C46 3F36 2A03 181D 2329 2E32 373A 3C3F"            /* LF?6*...#).27:<? */
	$"4242 4445 4849 4A4C 4E50 5051 5052 5150"            /* BBDEHIJLNPPQPRQP */
	$"4E4C 4A46 403C 3833 3123 0A01 AF00 1105"            /* NLJF@<831#Â.¯... */
	$"1B33 383A 4046 494D 5054 5658 5B5C 5E5E"            /* .38:@FIMPTVX[\^^ */
	$"5F80 6083 5F33 5E5D 5D5B 5854 4F4A 443C"            /* _€`ƒ_3^]][XTOJD< */
	$"3428 0015 1C21 262B 3033 383A 3D3F 4041"            /* 4(...!&+038:=?@A */
	$"4345 4648 4A4C 4E4F 504F 5152 514F 4D4B"            /* CEFHJLNOPOQRQOMK */
	$"4743 403C 3833 3127 1002 AC00 1101 0923"            /* GC@<831'..¬...Æ# */
	$"3637 3B40 454A 4D50 5355 5759 5C5D 5E87"            /* 67;@EJMPSUWY\]^‡ */
	$"5F80 5E33 5D5A 5753 4E49 4139 3323 0010"            /* _€^3]ZWSNIA93#.. */
	$"1B1F 2428 2C30 3537 3A3B 3D3F 4042 4346"            /* ..$(,057:;=?@BCF */
	$"484A 4B4D 4E4F 4F50 4F4E 4D4B 4947 4440"            /* HJKMNOOPONMKIGD@ */
	$"3C38 3331 2D19 0701 A900 1204 132C 3739"            /* <831-...©....,79 */
	$"3C42 464A 4D50 5355 5759 5A5D 5E5E 865F"            /* <BFJMPSUWYZ]^^†_ */
	$"3860 5F5E 5E5C 5A57 524D 473F 3831 1B00"            /* 8`_^^\ZWRMG?81.. */
	$"0C1B 1E22 262A 2E30 3336 383B 3C3D 4142"            /* ..."&*.0368;<=AB */
	$"4346 4848 4A4C 4E4E 4D4F 4E4D 4C4B 4846"            /* CFHHJLNNMONMLKHF */
	$"4441 3D39 3531 3025 0F03 A600 1402 0A1F"            /* DA=9510%..¦...Â. */
	$"3337 393E 4347 4B4E 5153 5557 585A 5C5D"            /* 379>CGKNQSUWXZ\] */
	$"5E5E 865F 3A60 5F5E 5D5B 5855 504B 453D"            /* ^^†_:`_^][XUPKE= */
	$"352F 1200 0818 1D21 2629 2C2F 3132 3539"            /* 5/.....!&),/1259 */
	$"3A3B 3E40 4243 4546 4849 4C4D 4D4C 4D4D"            /* :;>@BCEFHILMMLMM */
	$"4C4B 4A48 4543 413E 3B37 3332 2E1F 0B03"            /* LKJHECA>;732.... */
	$"A200 1602 0818 2D36 383B 4044 484C 4F52"            /* ¢.....-68;@DHLOR */
	$"5355 5759 5A5B 5C5D 5D5E 875F 255E 5E5C"            /* SUWYZ[\]]^‡_%^^\ */
	$"5A56 534F 4842 3A33 2B0A 0003 161B 1F24"            /* ZVSOHB:3+Â.....$ */
	$"272A 2D31 3233 3437 393A 3D40 4243 4447"            /* '*-123479:=@BCDG */
	$"4849 4B80 4C12 4D4C 4B4B 4A48 4645 4340"            /* HIK€L.MLKKJHFEC@ */
	$"3D39 3532 332C 1C0B 049E 0015 0208 1629"            /* =9523,...ž.....) */
	$"3537 383D 4246 494B 4F51 5354 5758 595B"            /* 578=BFIKOQSTWXY[ */
	$"5C5C 805D 825E 815F 2B5E 5E5D 5C58 5551"            /* \\€]‚^_+^^]\XUQ */
	$"4B45 3E37 3025 0300 0111 191D 2124 272C"            /* KE>70%......!$', */
	$"2E31 3334 3437 393B 3E40 4243 4446 4749"            /* .134479;>@BCDFGI */
	$"4A4B 4C4B 4C81 4B0F 4946 4644 413F 3B39"            /* JKLKLK.IFFDA?;9 */
	$"3433 322C 1F0F 0602 9800 1501 050B 192A"            /* 432,....˜......* */
	$"3437 393C 4044 484B 4D4F 5254 5657 5859"            /* 479<@DHKMORTVWXY */
	$"5B80 5C85 5D10 5E5F 5E5F 5E5D 5A59 5753"            /* [€\…].^_^_^]ZYWS */
	$"4D48 413B 332C 1C80 0032 0B19 1B1F 2326"            /* MHA;3,.€.2....#& */
	$"292B 2D30 3234 3436 393C 3E3F 4243 4445"            /* )+-024469<>?BCDE */
	$"4748 494A 4C4C 4B4C 4B4C 4B49 4847 4542"            /* GHIJLLKLKLKIHGEB */
	$"413E 3B37 3433 332F 2418 0C06 0392 0018"            /* A>;7433/$....’.. */
	$"0204 0812 202D 3537 383B 3F43 4649 4C4E"            /* .... -578;?CFILN */
	$"5153 5556 5758 595A 5B80 5C88 5D80 5C0A"            /* QSUVWXYZ[€\ˆ]€\Â */
	$"5A58 544F 4A45 3E37 302A 1280 0012 0616"            /* ZXTOJE>70*.€.... */
	$"1A1D 2124 272A 2C2D 3032 3435 3639 3C3E"            /* ..!$'*,-024569<> */
	$"3F80 4207 4446 4648 494B 4B4C 804D 164C"            /* ?€B.DFFHIKKL€M.L */
	$"4A4A 4846 4541 3F3D 3B38 3533 3332 2C23"            /* JJHFEA?=;85332,# */
	$"1A10 0905 0302 8900 1A01 0306 0B13 1E27"            /* ..Æ...‰........' */
	$"3034 3638 3B3F 4246 484B 4D4F 5254 5556"            /* 0468;?BFHKMORTUV */
	$"5758 595A 805B 015C 5C80 5D81 5C01 5D5D"            /* WXYZ€[.\\€]\.]] */
	$"805C 0C5B 5A58 5550 4C46 413B 332C 2609"            /* €\.[ZXUPLFA;3,&Æ */
	$"8000 3802 1319 1C20 2326 292B 2C2E 3033"            /* €.8.... #&)+,.03 */
	$"3535 3639 3C3D 3F40 4243 4345 4647 4A4B"            /* 5569<=?@BCCEFGJK */
	$"4B4C 4D4D 4C4C 4B4A 4846 4543 413F 3D3B"            /* KLMMLLKJHFECA?=; */
	$"3836 3433 3330 2A25 1C15 0F05 8800 1E07"            /* 864330*%....ˆ... */
	$"141A 2228 2E31 3337 3B3E 4245 484A 4C4E"            /* .."(.137;>BEHJLN */
	$"5152 5456 5758 5859 5B5B 5A5C 5B5B 895C"            /* QRTVWXXY[[Z\[[‰\ */
	$"0D5B 5B58 5451 4E49 453D 3730 291E 0281"            /* .[[XTQNIE=70).. */
	$"001B 0E17 1A1E 2224 272A 2B2D 2E31 3335"            /* ......"$'*+-.135 */
	$"3536 393C 3D3E 4042 4344 4446 4849 804B"            /* 569<=>@BCDDFHI€K */
	$"184D 4C4A 4C4A 4A48 4846 4543 4240 3E3D"            /* .MLJLJJHHFECB@>= */
	$"3A38 3532 2E2B 2722 1E0C 8700 1601 0C1E"            /* :852.+'"..‡..... */
	$"2026 2B30 3539 3E41 4447 4A4C 4E51 5253"            /*  &+059>ADGJLNQRS */
	$"5556 5758 8059 045A 595A 5B5B 825C 865B"            /* UVWX€Y.ZYZ[[‚\†[ */
	$"0B58 5553 504C 4641 3A33 2C27 1482 0021"            /* .XUSPLFA:3,'.‚.! */
	$"0816 191D 2024 2628 2B2C 2D2F 3234 3535"            /* .... $&(+,-/2455 */
	$"3639 3C3C 3D3F 4143 4344 4547 4A4B 4B4D"            /* 69<<=?ACCDEGJKKM */
	$"4C4B 804A 1249 4948 4746 4442 4141 3F3D"            /* LK€J.IIHGFDBAA?= */
	$"3937 332F 2A23 200C 8700 1E01 0C20 2329"            /* 973/*# .‡.... #) */
	$"2E35 3B3F 4346 494B 4E50 5253 5556 5758"            /* .5;?CFIKNPRSUVWX */
	$"5959 5A5A 5959 5858 5A5B 815C 015B 5A82"            /* YYZZYYXXZ[\.[Z‚ */
	$"590E 5A58 5856 5451 4D49 433D 362F 2823"            /* Y.ZXXVTQMIC=6/(# */
	$"0982 0025 0213 181B 1F23 2528 2A2C 2D2E"            /* Æ‚.%.....#%(*,-. */
	$"3133 3335 3536 383A 3C3E 3F41 4343 4446"            /* 1335568:<>?ACCDF */
	$"4849 4A4B 4B4C 4A49 4B4A 8148 0D47 4745"            /* HIJKKLJIKJH.GGE */
	$"4443 413E 3A37 322D 2722 0E87 0012 010E"            /* DCA>:72-'".‡.... */
	$"2227 2D33 383E 4347 4A4D 4F51 5254 5656"            /* "'-38>CGJMOQRTVV */
	$"5780 5981 5A81 5800 5980 5B01 5A59 8358"            /* W€YZX.Y€[.ZYƒX */
	$"0E59 5756 5551 4E4A 4540 3933 2C26 1C02"            /* .YWVUQNJE@93,&.. */
	$"8300 360C 1719 1D20 2426 292B 2D2D 2F30"            /* ƒ.6.... $&)+--/0 */
	$"3233 3434 3537 3A3C 3D3E 4142 4344 4647"            /* 234457:<=>ABCDFG */
	$"484A 494A 4949 4A49 4A49 4847 4848 4746"            /* HJIJIIJIJIHGHHGF */
	$"4443 413D 3A34 2F29 240E 8700 1A01 0F25"            /* DCA=:4/)$.‡....% */
	$"2931 373C 4045 494C 4F51 5253 5556 5758"            /* )17<@EILOQRSUVWX */
	$"5958 595A 5959 5757 8058 0057 8159 8458"            /* YXYZYYWW€X.WY„X */
	$"0D57 5655 534E 4B47 423C 362F 2823 1284"            /* .WVUSNKGB<6/(#.„ */
	$"001F 0515 181C 1F22 2528 292B 2D2E 2E31"            /* ......."%()+-..1 */
	$"3033 3435 3537 393C 3D3E 4042 4344 4546"            /* 0345579<=>@BCDEF */
	$"4748 8047 8148 1049 4948 4847 4748 4543"            /* GH€GH.IIHHGGHEC */
	$"3F3C 3731 2A26 0F01 8600 1701 1027 2C33"            /* ?<71*&..†....',3 */
	$"393F 4448 4B4D 4F51 5253 5555 5657 5857"            /* 9?DHKMOQRSUUVWXW */
	$"5859 5887 5780 5881 570E 5655 5553 504D"            /* XYX‡W€XW.VUUSPM */
	$"4843 3F39 322B 251F 0784 001E 0110 171A"            /* HC?92+%..„...... */
	$"1E21 2426 292A 2D2D 2E30 3031 3334 3535"            /* .!$&)*--.0013455 */
	$"3639 3C3D 3E40 4243 4244 4481 4500 4680"            /* 69<=>@BCBDDE.F€ */
	$"4702 4849 4A80 490A 4847 4441 3C38 322B"            /* G.HIJ€IÂHGDA<82+ */
	$"270F 0186 000D 0111 292C 343B 4146 4A4D"            /* '..†....),4;AFJM */
	$"4F51 5354 8055 0456 5657 5858 8257 0156"            /* OQST€U.VVWXX‚W.V */
	$"5687 5710 5656 5553 5250 4D49 4540 3B35"            /* V‡W.VVUSRPMIE@;5 */
	$"2E28 2315 0185 001C 0816 181B 1F21 2527"            /* .(#..….......!%' */
	$"292A 2D2E 2F30 2F31 3334 3535 3638 3B3C"            /* )*-./0/1345568;< */
	$"3E3F 4141 4281 4302 4443 4581 470E 494A"            /* >?AABC.DCEG.IJ */
	$"4949 4746 4442 3C38 322C 270F 0186 0012"            /* IIGFDB<82,'..†.. */
	$"0110 292D 353C 4146 4B4E 5053 5454 5556"            /* ..)-5<AFKNPSTTUV */
	$"5555 5683 5780 5682 5500 5681 5710 5655"            /* UUVƒW€V‚U.VW.VU */
	$"5654 5150 4E4B 4642 3D37 312A 2420 0A86"            /* VTQPNKFB=71*$ Â† */
	$"001C 0212 171A 1E20 2226 2829 2B2C 2E2D"            /* ....... "&()+,.- */
	$"2F2F 3132 3435 3536 383A 3C3D 3E3F 4080"            /* //1245568:<=>?@€ */
	$"4100 4281 4303 4546 4747 8049 0A48 4544"            /* A.BC.EFGG€IÂHED */
	$"403D 3831 2B26 0F01 8600 0D01 1129 2E35"            /* @=81+&..†....).5 */
	$"3B41 464B 4E50 5253 5380 5402 5355 5582"            /* ;AFKNPRSS€T.SUU‚ */
	$"5602 5555 5482 5200 5481 5510 5455 5452"            /* V.UUT‚R.TU.TUTR */
	$"504E 4B48 433E 3933 2D27 2219 0287 000C"            /* PNKHC>93-'"..‡.. */
	$"0A16 181C 1E21 2427 292A 2C2C 2D80 2F0B"            /* Â....!$')*,,-€/. */
	$"3032 3435 3435 3638 3A3C 3D3D 803E 1640"            /* 02454568:<==€>.@ */
	$"4141 4242 4344 4647 4748 4748 4644 403C"            /* AABBCDFGGHGHFD@< */
	$"3731 2B26 0F01 8600 0901 1129 2E35 3C42"            /* 71+&..†.Æ..).5<B */
	$"464A 4C80 5002 5253 5380 5200 5382 5402"            /* FJL€P.RSS€R.S‚T. */
	$"5252 5382 5201 5352 8053 0F52 5352 4F4E"            /* RRS‚R.SR€S.RSRON */
	$"4C48 4440 3B34 2F29 241F 0C88 0014 0312"            /* LHD@;4/)$..ˆ.... */
	$"1619 1D20 2125 2729 2B2B 2D2E 2E2F 2F30"            /* ... !%')++-..//0 */
	$"3133 3480 3506 3639 3A3B 3C3C 3D80 3E01"            /* 134€5.69:;<<=€>. */
	$"4041 8043 0E46 4747 4646 4542 3F3B 3630"            /* @A€C.FGGFFEB?;60 */
	$"2A25 0F01 8600 0D01 1129 2E35 3B41 464A"            /* *%..†....).5;AFJ */
	$"4C4E 4F50 5183 5282 5302 5252 5181 5001"            /* LNOPQƒR‚S.RRQP. */
	$"5151 8252 0E51 4E4D 4B49 4541 3C37 302B"            /* QQ‚R.QNMKIEA<70+ */
	$"2520 1803 8900 3409 1618 1B1E 2022 2628"            /* % ..‰.4Æ.... "&( */
	$"2A2B 2D2D 2E2F 2F30 3031 3335 3534 3536"            /* *+--.//001355456 */
	$"3637 3839 3A3A 3B3D 3D3F 4142 4243 4547"            /* 6789::;==?ABBCEG */
	$"4645 4442 3E39 342F 2923 0E01 8600 0F01"            /* FEDB>94/)#..†... */
	$"1028 2C34 393F 4448 4B4E 4F4F 5150 4F81"            /* .(,49?DHKNOOQPO */
	$"5082 5102 5050 4F81 4E01 4F4E 8150 0E4F"            /* P‚Q.PPON.ONP.O */
	$"4E4B 4B48 4642 3D37 322D 2822 1E0C 8A00"            /* NKKHFB=72-("..Š. */
	$"0D03 1116 191C 1F21 2427 292A 2C2D 2E80"            /* .......!$')*,-.€ */
	$"2F05 3030 3133 3434 8135 8036 1638 393B"            /* /.0013445€6.89; */
	$"3C3E 3E41 4243 4345 4544 4340 3D38 342E"            /* <>>ABCCEEDC@=84. */
	$"2722 0E01 8600 0E01 1027 2B33 383F 4347"            /* '"..†....'+38?CG */
	$"494C 4D4D 4F4E 814D 004E 824F 034D 4D4E"            /* ILMMONM.N‚O.MMN */
	$"4D82 4C12 4E4D 4D4E 4C4B 4A48 4441 3D3A"            /* M‚L.NMMNLKJHDA=: */
	$"342E 2A24 1F17 038B 0033 0915 161A 1D20"            /* 4.*$...‹.3Æ....  */
	$"2226 282A 2B2C 2D2E 2F2E 2F30 2F30 3233"            /* "&(*+,-././0/023 */
	$"3434 3535 3435 3536 3637 3A3C 3D3E 4041"            /* 4455455667:<=>@A */
	$"4142 4341 413E 3B37 322C 2621 0D01 8600"            /* ABCAA>;72,&!..†. */
	$"0E01 1025 2A31 373D 4245 484A 4B4C 4D4C"            /* ...%*17=BEHJKLML */
	$"824D 814E 814C 004B 804A 804B 104C 4C4A"            /* ‚MNL.K€J€K.LLJ */
	$"4A48 4644 413D 3935 2F2B 2620 1B0A 8C00"            /* JHFDA=95/+& .ÂŒ. */
	$"0F02 0F15 171B 1E21 2326 282A 2C2D 2D2E"            /* .......!#&(*,--. */
	$"2E82 2F03 3032 3133 8034 8135 1336 383B"            /* .‚/.0213€45.68; */
	$"3D3D 3F3F 4240 3F3F 3C39 3632 2B25 210D"            /* ==??B@??<962+%!. */
	$"0186 000E 010F 2428 2F36 3A3F 4346 4849"            /* .†....$(/6:?CFHI */
	$"4B4B 4C81 4B82 4C00 4A80 4982 4880 490F"            /* KKLK‚L.J€I‚H€I. */
	$"4848 4744 433F 3C39 3632 2C27 201D 1302"            /* HHGDC?<962,' ... */
	$"8D00 0E06 1416 191C 1F21 2527 292A 2B2D"            /* ........!%')*+- */
	$"2E2E 842F 0130 3181 3200 3480 3511 3638"            /* ..„/.012.4€5.68 */
	$"3B3D 3D3E 403F 3E3D 3C38 3530 2A24 1F0C"            /* ;==>@?>=<850*$.. */
	$"8700 0E01 0E23 272D 3339 3D42 4446 4748"            /* ‡....#'-39=BDFGH */
	$"494A 8749 0147 4884 4610 4546 4444 4241"            /* IJ‡I.GH„F.EFDDBA */
	$"3F3C 3936 312C 2721 1D19 068E 000E 010C"            /* ?<961,'!...Ž.... */
	$"1517 1A1D 2022 2527 292B 2B2C 2D81 2E80"            /* .... "%')++,-.€ */
	$"2F83 3001 3234 8035 1036 373A 3B3C 3D3E"            /* /ƒ0.24€5.67:;<=> */
	$"3D3B 3A36 322E 2823 1E0C 8700 0D01 0E21"            /* =;:62.(#..‡....! */
	$"262C 3136 3B3F 4244 4547 4784 4880 4702"            /* &,16;?BDEGG„H€G. */
	$"4646 4584 4410 4342 4240 3F3E 3B39 3631"            /* FFE„D.CBB@?>;961 */
	$"2D27 221D 1B0E 018F 000E 0310 1618 1B1E"            /* -'"............ */
	$"2123 2628 2A2B 2B2C 2C82 2D84 2F03 3032"            /* !#&(*++,,‚-„/.02 */
	$"3233 8035 0237 3839 803B 0839 3735 302C"            /* 23€5.789€;.9750, */
	$"2621 1D0C 8700 0E01 0D20 242A 3035 393D"            /* &!..‡.... $*059= */
	$"4043 4445 4647 8346 0045 8144 8142 1343"            /* @CDEFGƒF.EDB.C */
	$"4341 4140 3F3F 3D3D 3A36 3430 2C27 231E"            /* CAA@??==:640,'#. */
	$"1B13 0491 000B 0714 1619 1C1F 2124 2628"            /* ...‘........!$&( */
	$"2A2B 802C 042D 2E2D 2D2E 842F 0330 3031"            /* *+€,.-.--.„/.001 */
	$"3380 3501 3637 8038 0836 3431 2E2A 2420"            /* 3€5.67€8.641.*$  */
	$"1C0B 8700 0C01 0D1E 2229 2E33 383C 3E41"            /* ..‡.....").38<>A */
	$"4143 8644 0242 4243 8341 1140 3F3E 3D3C"            /* AC†D.BBCƒA.@?>=< */
	$"3B39 3735 322F 2B27 231E 1B17 0792 000F"            /* ;9752/+'#....’.. */
	$"010C 1517 191D 1F22 2527 292A 2B2B 2C2C"            /* ......."%')*++,, */
	$"812D 852F 0430 3031 3334 8135 0A36 3534"            /* -…/.001345Â654 */
	$"322F 2C28 231F 1B0B 8700 0C01 0C1C 2027"            /* 2/,(#...‡..... ' */
	$"2D32 363A 3D40 4041 8342 0143 4380 4105"            /* -26:=@@AƒB.CC€A. */
	$"403F 3F3E 3F3E 803D 0F3B 3B38 3634 322F"            /* @??>?>€=.;;8642/ */
	$"2C29 2622 1F1A 180C 0193 000C 0310 1617"            /* ,)&".....“...... */
	$"1B1D 2023 2527 292A 2B80 2C02 2E2D 2D86"            /* .. #%')*+€,..--† */
	$"2F12 3030 3131 3334 3535 3433 3230 2E2B"            /* /.001134554320.+ */
	$"2722 1D1A 0A87 000B 010B 1B1F 252B 3035"            /* '"..Â‡......%+05 */
	$"383B 3E3F 8041 2042 4142 4140 403F 3F3E"            /* 8;>?€A BABA@@??> */
	$"3E3D 3D3C 3D3B 3C3B 3937 3633 3130 2E2B"            /* >==<=;<;976310.+ */
	$"2825 211E 1A18 1003 9500 0E06 1316 181B"            /* (%!.....•....... */
	$"1E20 2326 2729 2A2B 2B2D 812E 862F 8030"            /* . #&')*++-.†/€0 */
	$"0E31 3233 3332 3331 2E2C 2825 201C 190A"            /* .1233231.,(% ..Â */
	$"8700 0C01 0B1B 1F23 292E 3337 393C 3D3E"            /* ‡......#).379<=> */
	$"833F 053E 3D3D 3C3C 3B81 3A11 3837 3636"            /* ƒ?.>==<<;:.8766 */
	$"3433 322F 2C2A 2623 1F1C 1917 1305 9700"            /* 432/,*&#......—. */
	$"1208 1416 191C 1E21 2326 2729 2A2B 2C2C"            /* .......!#&')*+,, */
	$"2D2F 2F2E 872F 8030 8031 0930 2E2C 2A27"            /* -//.‡/€0€1Æ0.,*' */
	$"241F 1B18 0987 0017 010A 191E 2327 2C30"            /* $...Æ‡...Â..#',0 */
	$"3638 3B3C 3C3D 3E3D 3D3C 3C3B 3A3A 3839"            /* 68;<<=>==<<;::89 */
	$"8237 1036 3534 3333 302E 2A27 2522 1E1C"            /* ‚7.654330.*'%".. */
	$"1816 1307 9800 1401 0B15 1719 1C1E 2124"            /* ....˜.........!$ */
	$"2627 282A 2B2B 2C2D 2E2D 2E2E 882F 0C30"            /* &'(*++,-.-..ˆ/.0 */
	$"2F2F 2E2C 2A29 2522 1E1A 1709 8700 1201"            /* //.,*)%"...Æ‡... */
	$"0A18 1C22 272B 2E32 3538 3A3B 3C3B 3B3A"            /* Â.."'+.258:;<;;: */
	$"3938 8037 8036 8035 8034 0E32 2F2F 2C29"            /* 98€7€6€5€4.2//,) */
	$"2522 201D 1A17 1413 0901 9900 1502 0D16"            /* %" .....Æ.™..... */
	$"171A 1C1F 2123 2627 282A 2B2B 2C2C 2D2E"            /* ....!#&'(*++,,-. */
	$"2D2E 2E87 2F0B 2E2E 2C2B 2927 2421 1C19"            /* -..‡/...,+)'$!.. */
	$"1609 8800 0C09 181B 2125 2B2E 3034 3536"            /* .Æˆ..Æ..!%+.0456 */
	$"3738 8137 0236 3535 8034 1432 3333 3231"            /* 787.655€4.23321 */
	$"3130 2E2D 2A27 2522 1F1D 1916 1311 0A01"            /* 10.-*'%"......Â. */
	$"9B00 1203 0E16 171A 1C1F 2123 2527 2829"            /* ›.........!#%'() */
	$"2B2A 2B2C 2C2D 802E 852F 802E 092C 2A29"            /* +*+,,-€.…/€.Æ,*) */
	$"2623 1F1C 1815 0988 000C 0916 1A1F 2428"            /* &#....Æˆ..Æ...$( */
	$"2D30 3233 3435 3681 3503 3433 3332 8030"            /* -0234565.4332€0 */
	$"122F 302F 2E2D 2C2B 2826 2421 1F1C 1915"            /* ./0/.-,+(&$!.... */
	$"1211 0A02 9D00 1404 0F16 171A 1C1E 2023"            /* ..Â.......... # */
	$"2426 2628 2829 2B2B 2C2C 2E2D 832E 002F"            /* $&&(()++,,.-ƒ../ */
	$"802E 0A2D 2B2A 2825 221E 1B17 1408 8800"            /* €.Â-+*(%".....ˆ. */
	$"2909 1518 1D22 2529 2D2F 3132 3334 3333"            /* )Æ..."%)-/123433 */
	$"3232 3130 2F2F 2E2F 2E2C 2C2D 2C29 2928"            /* 2210//./.,,-,))( */
	$"2423 201E 1B17 1311 100A 029F 0009 0410"            /* $# ......Â.Ÿ.Æ.. */
	$"1617 191C 1E20 2223 8025 0627 2928 292B"            /* ..... "#€%.')()+ */
	$"2C2C 832D 0E2E 2D2E 2D2C 2B29 2724 211E"            /* ,,ƒ-..-.-,+)'$!. */
	$"1B17 1408 8800 0C09 1417 1C20 2427 2A2C"            /* ....ˆ..Æ... $'*, */
	$"2D2F 3030 802F 0B2E 2D2C 2C2B 2B2A 2A29"            /* -/00€/..-,,++**) */
	$"2829 2780 2509 221F 1E1B 1713 1110 0A02"            /* ()'€%Æ".......Â. */
	$"A100 1704 0F16 1619 1B1D 1F20 2223 2425"            /* ¡.......... "#$% */
	$"2628 2829 2B2B 2C2D 2E2C 2C81 2D0B 2C2C"            /* &(()++,-.,,-.,, */
	$"2A28 2623 211E 1A17 1408 8800 0A08 1417"            /* *(&#!.....ˆ.Â... */
	$"1B1F 2124 2728 2A2B 812C 002B 812A 0029"            /* ..!$'(*+,.+*.) */
	$"8028 8026 0C24 2322 211F 1C1A 1714 1110"            /* €(€&.$#"!....... */
	$"0A03 A300 1503 0E15 1618 1A1C 1E20 2123"            /* Â.£.......... !# */
	$"2425 2626 2828 292A 2B2C 2B80 2C0D 2D2D"            /* $%&&(()*+,+€,.-- */
	$"2C2B 2A28 2523 211E 1A16 1408 8800 1207"            /* ,+*(%#!.....ˆ... */
	$"1215 191C 2023 2425 2828 292A 2A29 2928"            /* .... #$%(()**))( */
	$"2928 8026 0025 8024 0C22 2121 201E 1B1A"            /* )(€&.%€$."!! ... */
	$"1713 110F 0902 A500 1103 0C14 1517 191B"            /* ....Æ.¥......... */
	$"1D1F 2022 2324 2526 2827 2780 2910 2A2C"            /* .. "#$%&(''€).*, */
	$"2B2C 2C2B 2B29 2725 2321 1E1A 1614 0788"            /* +,,++)'%#!.....ˆ */
	$"000B 0711 1316 191C 1E21 2324 2425 8027"            /* .........!#$$%€' */
	$"1626 2524 2322 2221 2120 201F 1E1D 1C1B"            /* .&%$#""!!  ..... */
	$"1918 1512 110F 0802 A700 1202 0912 1516"            /* ........§...Æ... */
	$"1819 1C1C 1F21 2224 2525 2627 2728 8029"            /* .....!"$%%&''(€) */
	$"0E2A 2B2A 292A 2827 2523 211E 1A16 1307"            /* .*+*)*('%#!..... */
	$"8800 1507 1011 1517 191C 1D1F 1F20 2021"            /* ˆ............  ! */
	$"2221 2020 1F20 1F1E 1E80 1C0B 1A1A 1918"            /* "!  . ...€...... */
	$"1615 1311 110E 0601 A900 1001 0710 1314"            /* ........©....... */
	$"1617 1A1C 1E1F 2022 2324 2526 8027 8028"            /* ...... "#$%&€'€( */
	$"0C29 2928 2827 2523 211D 1A16 1307 8800"            /* .))(('%#!.....ˆ. */
	$"0906 0E10 1315 1719 1A1C 1C80 1D81 1E04"            /* Æ..........€... */
	$"1D1C 1C1B 1B80 190A 1617 1615 1312 100F"            /* .....€.Â........ */
	$"0C05 01AC 0010 040C 1113 1416 1819 1C1E"            /* ...¬............ */
	$"2021 2222 2424 2580 260D 2729 2828 2726"            /*  !""$$%€&.')(('& */
	$"2423 201D 1A16 1307 8800 0B05 0C0E 1012"            /* $# .....ˆ....... */
	$"1315 1617 1919 1A81 1B11 191A 1A18 1817"            /* ............... */
	$"1715 1414 1312 1110 0F0E 0903 AF00 2002"            /* ..........Æ.¯. . */
	$"080F 1213 1517 181B 1C1E 1F20 2121 2324"            /* ........... !!#$ */
	$"2525 2626 2727 2625 2422 201C 1915 1207"            /* %%&&''&%$" ..... */
	$"8800 0C05 0A0B 0E0F 1112 1314 1415 1616"            /* ˆ...Â........... */
	$"8117 0F15 1616 1313 1211 100F 0F0E 0D0D"            /* ............... */
	$"0B06 01B1 001F 0105 0C10 1113 1517 191A"            /* ...±............ */
	$"1C1D 1D1F 2021 2223 2525 2626 2525 2321"            /* .... !"#%%&&%%#! */
	$"1E1B 1814 1207 8800 0904 0A0B 0C0D 0E0F"            /* ......ˆ.Æ.Â..... */
	$"1011 1183 1308 1213 1211 100F 0F0E 0C81"            /* ...ƒ........... */
	$"0B01 0803 B500 1102 080E 1112 1315 1617"            /* ....µ........... */
	$"181A 1B1C 1E1F 2121 2380 2408 2321 201D"            /* ......!!#€$.#! . */
	$"1A17 1311 0688 000C 0408 090A 0B0C 0D0E"            /* .....ˆ....ÆÂ.... */
	$"0E0F 0F11 1282 1102 100F 0F80 0C05 0A0A"            /* .....‚.....€..ÂÂ */
	$"0907 0401 B700 1001 030A 0F11 1112 1415"            /* Æ...·....Â...... */
	$"1618 191A 1C1D 1F20 8121 0720 1E1B 1916"            /* ....... !. .... */
	$"1210 0688 000A 0307 0809 0A0A 0C0C 0D0D"            /* ...ˆ.Â...ÆÂÂ.... */
	$"0E83 0F0A 0E0E 0D0B 0A0A 0908 0705 02BB"            /* .ƒ.Â....ÂÂÆ....» */
	$"001A 0105 0B10 1111 1213 1415 1619 1A1C"            /* ................ */
	$"1D1F 1E1F 1F1E 1C1A 1714 110F 0688 0009"            /* .............ˆ.Æ */
	$"0307 0708 0909 0A0A 0B0B 840D 080C 0C0B"            /* ....ÆÆÂÂ..„..... */
	$"0A08 0807 0402 BF00 1801 050B 0E0F 1011"            /* Â.....¿......... */
	$"1213 1417 191A 1B1C 1B1D 1B1A 1715 1310"            /* ................ */
	$"0E05 8800 0502 0606 0708 0880 0901 0B0B"            /* ..ˆ........€Æ... */
	$"820C 070B 0B0A 0908 0704 02C3 000C 0105"            /* ‚....ÂÆ....Ã.... */
	$"0A0D 0F10 1112 1416 1718 1980 1806 1615"            /* Â..........€.... */
	$"1310 0E0C 0588 0004 0205 0606 0781 0804"            /* .....ˆ......... */
	$"0A0A 0B0A 0B80 0A04 0909 0705 02C7 0014"            /* ÂÂ.Â.€Â.ÆÆ...Ç.. */
	$"0103 080C 0F10 1112 1315 1516 1515 1413"            /* ................ */
	$"100F 0D0B 0488 0003 0205 0506 8107 0008"            /* .....ˆ......... */
	$"8509 0207 0502 CC00 0602 050A 0E10 1111"            /* …Æ....Ì....Â.... */
	$"8113 0611 100E 0D0B 0A04 8800 0402 0505"            /* .......Â.ˆ..... */
	$"0606 8007 8408 0206 0301 D100 0E02 060A"            /* ..€.„.....Ñ....Â */
	$"0D0F 1010 0F0E 0D0D 0B0A 0803 8800 0102"            /* .........Â..ˆ... */
	$"0480 0501 0606 8007 0406 0605 0301 D700"            /* .€....€.......×. */
	$"0B02 0508 0A0B 0C0B 0A09 0807 0388 0002"            /* ....Â...ÂÆ...ˆ.. */
	$"0204 0483 0502 0402 01DE 0007 0103 0407"            /* ...ƒ.....Þ...... */
	$"0706 0502 8800 0101 0380 0401 0201 B000"            /* ....ˆ....€....°. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 FF00 FF00"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ. */
	$"FF00 FF00 FF00 FF00 FF00 FF00 8100 7438"            /* ÿ.ÿ.ÿ.ÿ.ÿ.ÿ..t8 */
	$"6D6B 0000 4008 0000 0000 0000 0000 0000"            /* mk..@........... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0717 2B3E"            /* ..............+> */
	$"4E5B 6874 838C 8C82 7366 5A4C 3D2D 1506"            /* N[htƒŒŒ‚sfZL=-.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0003 2154 85AE D1E8 F5FD"            /* ........!T…®Ñèõý */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FCF6 E7CF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿüöçÏ */
	$"AB80 5023 0600 0000 0000 0000 0000 0000"            /* «€P#............ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 1B54 90C6 EFFF FFFF FFFF FFFF"            /* .....TÆïÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFF1 CD97 5A1F 0000 0000 0000 0000"            /* ÿÿÿñÍ—Z......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0022 6AB1 E9FF FFFF FFFF FFFF FFFF FFFF"            /* ."j±éÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFED B771 2800 0000 0000"            /* ÿÿÿÿÿÿÿí·q(..... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 145C"            /* ...............\ */
	$"ADED FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ­íÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF F1B5 6419 0000"            /* ÿÿÿÿÿÿÿÿÿÿñµd... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 2C87 DEFF"            /* ............,‡Þÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFE3 9034"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿã4 */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 3EA7 F2FF FFFF"            /* ..........>§òÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFF7"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ÷ */
	$"AE47 0000 0000 0000 0000 0000 0000 0000"            /* ®G.............. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 3CAD FAFF FFFF FFFF"            /* ........<­úÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFF4"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿô */
	$"E5D2 C1B4 AFAE B1B9 C8DB ECF9 FFFF FFFF"            /* åÒÁ´¯®±¹ÈÛìùÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFD B646 0000 0000 0000 0000 0000 0000"            /* ÿý¶F............ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 2EA0 F8FF FFFF FFFF FFFF"            /* ....... øÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF EFC6 956A 4527"            /* ÿÿÿÿÿÿÿÿÿÿïÆ•jE' */
	$"1207 0100 0000 0000 040B 1A33 557D AADA"            /* ...........3U}ªÚ */
	$"FAFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* úÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFC AB37 0000 0000 0000 0000 0000"            /* ÿÿÿü«7.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 1383 EFFF FFFF FFFF FFFF FFFF"            /* .....ƒïÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFE0 9E5B 2103 0000 0000"            /* ÿÿÿÿÿÿÿàž[!..... */
	$"0000 0000 0000 0000 0000 0000 0000 000C"            /* ................ */
	$"3878 BDF3 FFFF FFFF FFFF FFFF FFFF FFFF"            /* 8x½óÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFF4 8F1B 0000 0000 0000 0000"            /* ÿÿÿÿÿô......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 004F D3FF FFFF FFFF FFFF FFFF FFFF"            /* ...OÓÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF F2A8 5312 0000 0000 0000 0000"            /* ÿÿÿÿò¨S......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 002A 78CF FFFF FFFF FFFF FFFF FFFF"            /* ...*xÏÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFDD 5F00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÝ_....... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"001A 9CFD FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..œýÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF E689 2B00 0000 0000 0000 0000 0005"            /* ÿÿæ‰+........... */
	$"1833 4B5D 6768 675E 4C35 1A06 0000 0000"            /* .3K]ghg^L5...... */
	$"0000 0000 000A 51B6 FEFF FFFF FFFF FFFF"            /* .....ÂQ¶þÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFA9 2100 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ©!..... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"4BD6 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* KÖÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"E984 1F00 0000 0000 0000 0006 326C A0CC"            /* é„..........2l Ì */
	$"E9F9 FFFF FFFF FFFF FFF9 EBCE A370 3609"            /* éùÿÿÿÿÿÿÿùëÎ£p6Æ */
	$"0000 0000 0000 0000 47B7 FFFF FFFF FFFF"            /* ........G·ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF DF58 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿßX.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 067F"            /* ................ */
	$"FAFF FFFF FFFF FFFF FFFF FFFF FFFF FCA1"            /* úÿÿÿÿÿÿÿÿÿÿÿÿÿü¡ */
	$"2600 0000 0000 0000 002E 7FCA F8FF FFFF"            /* &..........Êøÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF F9D0"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿùÐ */
	$"8333 0000 0000 0000 0002 58D2 FFFF FFFF"            /* ƒ3........XÒÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 8F0C 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 001D B3FF"            /* ..............³ÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFD1 4A00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÑJ. */
	$"0000 0000 0000 014A B0F4 FFFF FFFF FFFF"            /* .......J°ôÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFF7 B652 0400 0000 0000 0012 88F8 FFFF"            /* ÿ÷¶R........ˆøÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFBF 2600"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ¿&. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 35D3 FFFF"            /* ............5Óÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 9A15 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿš... */
	$"0000 0000 0043 B7FF FFFF FFFF FFFF FFFF"            /* .....C·ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF BE4A 0000 0000 0000 0047 D7FF"            /* ÿÿÿÿ¾J.......G×ÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF DE43"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÞC */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 004D E9FF FFFF"            /* ...........Méÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF F066 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿðf.... */
	$"0000 001E 9BFA FFFF FFFF FFFF FFFF FFFF"            /* ....›úÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFD A326 0000 0000 0000 1CAC"            /* ÿÿÿÿÿý£&.......¬ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFF1"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿñ */
	$"5D00 0000 0000 0000 0000 0000 0000 0000"            /* ]............... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 60F5 FFFF FFFF"            /* ..........`õÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFDD 3E00 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÝ>..... */
	$"0000 54DD FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..TÝÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFE5 5E00 0000 0000 0004"            /* ÿÿÿÿÿÿÿå^....... */
	$"89FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ‰ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FC71 0000 0000 0000 0000 0000 0000 0000"            /* üq.............. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 006F FCFF FFFF FFFF"            /* .........oüÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF CF28 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÏ(...... */
	$"068B FDFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .‹ýÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF95 0B00 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ•...... */
	$"0070 FCFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .püÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 7F00 0000 0000 0000 0000 0000 0000"            /* ÿÿ.............. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 6FFE FFFF FFFF FFFF"            /* ........oþÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFCC 1F00 0000 0000 0011"            /* ÿÿÿÿÿÿÿÌ........ */
	$"ADFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ­ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF B719 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ·..... */
	$"0000 66FB FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..fûÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF81 0000 0000 0000 0000 0000 0000"            /* ÿÿÿ............ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0061 FCFF FFFF FFFF FFFF"            /* .......aüÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF D221 0000 0000 0000 17BC"            /* ÿÿÿÿÿÿÒ!.......¼ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFC9 1F00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÉ.... */
	$"0000 006A FDFF FFFF FFFF FFFF FFFF FFFF"            /* ...jýÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 7500 0000 0000 0000 0000 0000"            /* ÿÿÿÿu........... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 50F7 FFFF FFFF FFFF FFFF"            /* ......P÷ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFE0 2900 0000 0000 0014 C0FF"            /* ÿÿÿÿÿà).......Àÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF CB1C 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿË... */
	$"0000 0000 7DFF FFFF FFFF FFFF FFFF FFFF"            /* ....}ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FD62 0000 0000 0000 0000 0000"            /* ÿÿÿÿýb.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0039 EDFF FFFF FFFF FFFF FFFF"            /* .....9íÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF F340 0000 0000 0000 09B3 FFFF"            /* ÿÿÿÿó@......Æ³ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFBF 0E00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ¿.. */
	$"0000 0000 009C FFFF FFFF FFFF FFFF FFFF"            /* .....œÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFF4 4800 0000 0000 0000 0000"            /* ÿÿÿÿÿôH......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 1DD9 FFFF FFFF FFFF FFFF FFFF"            /* .....Ùÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 6900 0000 0000 0000 91FF FFFF"            /* ÿÿÿÿi.......‘ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 9F00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿŸ. */
	$"0000 0000 000E C8FF FFFF FFFF FFFF FFFF"            /* ......Èÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF E62B 0000 0000 0000 0000"            /* ÿÿÿÿÿÿæ+........ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0008 B9FF FFFF FFFF FFFF FFFF FFFF"            /* ....¹ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFA5 0000 0000 0000 005B FEFF FFFF"            /* ÿÿÿ¥.......[þÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF6D"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿm */
	$"0000 0000 0000 2FED FFFF FFFF FFFF FFFF"            /* ....../íÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFCA 1100 0000 0000 0000"            /* ÿÿÿÿÿÿÿÊ........ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0087 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...‡ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF DE19 0000 0000 0000 23E8 FFFF FFFF"            /* ÿÿÞ.......#èÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFF0"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿð */
	$"2E00 0000 0000 006B FFFF FFFF FFFF FFFF"            /* .......kÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 9D00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿ....... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 52FC FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..Rüÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFE 5800 0000 0000 0001 ACFF FFFF FFFF"            /* ÿþX.......¬ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"BA05 0000 0000 0004 BDFF FFFF FFFF FFFF"            /* º.......½ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF65 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿe...... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"001E E2FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..âÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFB4 0100 0000 0000 004D FEFF FFFF FFFF"            /* ÿ´.......Mþÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF5B 0000 0000 0000 38F6 FFFF FFFF FFFF"            /* ÿ[......8öÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFEB 2B00 0000 0000"            /* ÿÿÿÿÿÿÿÿÿë+..... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"01A9 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .©ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"F838 0000 0000 0000 06C5 FFFF FFFF FFFF"            /* ø8.......Åÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFD1 0B00 0000 0000 009D FFFF FFFF FFFF"            /* ÿÑ.......ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF BC06 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ¼..... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"5CFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* \ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"A500 0000 0000 0000 53FF FFFF FFFF FFFF"            /* ¥.......Sÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 6100 0000 0000 002A F3FF FFFF FFFF"            /* ÿÿa......*óÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF71 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿq.... */
	$"0000 0000 0000 0000 0000 0000 0000 0018"            /* ................ */
	$"E1FF FFFF FFFF FFFF FFFF FFFF FFFF FFF9"            /* áÿÿÿÿÿÿÿÿÿÿÿÿÿÿù */
	$"3700 0000 0000 0001 BBFF FFFF FFFF FFFF"            /* 7.......»ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF C805 0000 0000 0000 9DFF FFFF FFFF"            /* ÿÿÈ.......ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFEC 2600 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿì&... */
	$"0000 0000 0000 0000 0000 0000 0000 0091"            /* ...............‘ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFBA"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿº */
	$"0100 0000 0000 0030 F8FF FFFF FFFF FFFF"            /* .......0øÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FC40 0000 0000 0000 39FA FFFF FFFF"            /* ÿÿü@......9úÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF A700 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ§... */
	$"0000 0000 0000 0000 0000 0000 0000 36F6"            /* ..............6ö */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF5C"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ\ */
	$"0000 0000 0000 0080 FFFF FFFF FFFF FFFF"            /* .......€ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF94 0000 0000 0000 02BF FFFF FFFF"            /* ÿÿÿ”.......¿ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FC49 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿüI.. */
	$"0000 0000 0000 0000 0000 0000 0002 B4FF"            /* ..............´ÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF E312"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿã. */
	$"0000 0000 0000 05CA FFFF FFFF FFFF FFFF"            /* .......Êÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFD9 0A00 0000 0000 0069 FFFF FFFF"            /* ÿÿÿÙÂ......iÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFC5 0600"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÅ.. */
	$"0000 0000 0000 0000 0000 0000 004B FEFF"            /* .............Kþÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 9E00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿž. */
	$"0000 0000 0000 2BF7 FFFF FFFF FFFF FFFF"            /* ......+÷ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFB 3900 0000 0000 0022 F1FF FFFF"            /* ÿÿÿû9......"ñÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 5D00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ]. */
	$"0000 0000 0000 0000 0000 0000 03BE FFFF"            /* .............¾ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 5800"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿX. */
	$"0000 0000 0000 5FFF FFFF FFFF FFFF FFFF"            /* ......_ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 6E00 0000 0000 0001 BEFF FFFF"            /* ÿÿÿÿn.......¾ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF D10A"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÑÂ */
	$"0000 0000 0000 0000 0000 0000 4BFE FFFF"            /* ............Kþÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFEF 1D00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿï.. */
	$"0000 0000 0000 8EFF FFFF FFFF FFFF FFFF"            /* ......Žÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 9D00 0000 0000 0000 81FF FFFF"            /* ÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF61"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿa */
	$"0000 0000 0000 0000 0000 0002 B8FF FFFF"            /* ............¸ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFC4 0200"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÄ.. */
	$"0000 0000 0000 B7FF FFFF FFFF FFFF FFFF"            /* ......·ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF C503 0000 0000 0000 49FE FFFF"            /* ÿÿÿÿÅ.......Iþÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFCC"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÌ */
	$"0700 0000 0000 0000 0000 003A FBFF FFFF"            /* ...........:ûÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF95 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ•.. */
	$"0000 0000 0008 D6FF FFFF FFFF FFFF FFFF"            /* ......Öÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF E20F 0000 0000 0000 1DEE FFFF"            /* ÿÿÿÿâ........îÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"4E00 0000 0000 0000 0000 009C FFFF FFFF"            /* N..........œÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF6E 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿn.. */
	$"0000 0000 0014 E8FF FFFF FFFF FFFF FFFF"            /* ......èÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF EF1F 0000 0000 0000 05D0 FFFF"            /* ÿÿÿÿï........Ðÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"B200 0000 0000 0000 0000 1EED FFFF FFFF"            /* ²..........íÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF4D 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿM.. */
	$"0000 0000 0020 F0FF FFFF FFFF FFFF FFFF"            /* ..... ðÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF F62C 0000 0000 0000 00AF FFFF"            /* ÿÿÿÿö,.......¯ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"F72F 0000 0000 0000 0000 70FF FFFF FFFF"            /* ÷/........pÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF F933 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿù3.. */
	$"0000 0000 0025 F2FF FFFF FFFF FFFF FFFF"            /* .....%òÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF F831 0000 0000 0000 0095 FFFF"            /* ÿÿÿÿø1.......•ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF86 0000 0000 0000 0005 C8FF FFFF FFFF"            /* ÿ†........Èÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF F021 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿð!.. */
	$"0000 0000 0024 F2FF FFFF FFFF FFFF FFFF"            /* .....$òÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF F830 0000 0000 0000 0080 FFFF"            /* ÿÿÿÿø0.......€ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFD6 0A00 0000 0000 0037 FAFF FFFF FFFF"            /* ÿÖÂ......7úÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF E917 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿé... */
	$"0000 0000 001E EFFF FFFF FFFF FFFF FFFF"            /* ......ïÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF F52A 0000 0000 0000 0072 FFFF"            /* ÿÿÿÿõ*.......rÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFE 4700 0000 0000 0082 FFFF FFFF FFFF"            /* ÿþG......‚ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF E512 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿå... */
	$"0000 0000 0012 E5FF FFFF FFFF FFFF FFFF"            /* ......åÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF EE1C 0000 0000 0000 006C FFFF"            /* ÿÿÿÿî........lÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 9700 0000 0000 03C8 FFFF FFFF FFFF"            /* ÿÿ—......Èÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF E511 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿå... */
	$"0000 0000 0005 D1FF FFFF FFFF FFFF FFFF"            /* ......Ñÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF DE0C 0000 0000 0000 006B FFFF"            /* ÿÿÿÿÞ........kÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF DB0B 0000 0000 2DF7 FFFF FFFF FFFF"            /* ÿÿÛ.....-÷ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF E714 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿç... */
	$"0000 0000 0000 AFFF FFFF FFFF FFFF FFFF"            /* ......¯ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF BE01 0000 0000 0000 006F FFFF"            /* ÿÿÿÿ¾........oÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FD42 0000 0000 6DFF FFFF FFFF FFFF"            /* ÿÿýB....mÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF EC1B 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿì... */
	$"0000 0000 0000 86FF FFFF FFFF FFFF FFFF"            /* ......†ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 9400 0000 0000 0000 0079 FFFF"            /* ÿÿÿÿ”........yÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF83 0000 0000 ABFF FFFF FFFF FFFF"            /* ÿÿÿƒ....«ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF F52A 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿõ*.. */
	$"0000 0000 0000 55FF FFFF FFFF FFFF FFFF"            /* ......Uÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 6400 0000 0000 0000 008B FFFF"            /* ÿÿÿÿd........‹ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFC0 0100 000D DFFF FFFF FFFF FFFF"            /* ÿÿÿÀ....ßÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FD40 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿý@.. */
	$"0000 0000 0000 22F1 FFFF FFFF FFFF FFFF"            /* ......"ñÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFF8 2E00 0000 0000 0000 00A2 FFFF"            /* ÿÿÿø.........¢ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFED 1B00 0035 FAFF FFFF FFFF FFFF"            /* ÿÿÿí...5úÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF5E 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ^.. */
	$"0000 0000 0000 01BD FFFF FFFF FFFF FFFF"            /* .......½ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFCE 0500 0000 0000 0000 01C1 FFFF"            /* ÿÿÿÎ.........Áÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFE 4900 0064 FFFF FFFF FFFF FFFF"            /* ÿÿÿþI..dÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF83 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿƒ.. */
	$"0000 0000 0000 0070 FFFF FFFF FFFF FFFF"            /* .......pÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF84 0000 0000 0000 0000 10E2 FFFF"            /* ÿÿÿ„.........âÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 7A00 0092 FFFF FFFF FFFF FFFF"            /* ÿÿÿÿz..’ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFAE 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ®.. */
	$"0000 0000 0000 0022 F1FF FFFF FFFF FFFF"            /* ......."ñÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF F730 0000 0000 0000 0000 33F9 FFFF"            /* ÿÿ÷0........3ùÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF A800 01BD FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ¨..½ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFDD 0D00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÝ.. */
	$"0000 0000 0000 0000 A7FF FFFF FFFF FFFF"            /* ........§ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF B600 0000 0000 0000 0000 66FF FFFF"            /* ÿÿ¶.........fÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF D307 0FE2 FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÓ..âÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFC 3B00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿü;. */
	$"0000 0000 0000 0000 3DFB FFFF FFFF FFFF"            /* ........=ûÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFE 4B00 0000 0000 0000 0000 A1FF FFFF"            /* ÿþK.........¡ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF EE1E 29F5 FFFF FFFF FFFF FFFF"            /* ÿÿÿÿî.)õÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 7D00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ}. */
	$"0000 0000 0000 0000 00B0 FFFF FFFF FFFF"            /* .........°ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFBE 0300 0000 0000 0000 000D DDFF FFFF"            /* ÿ¾..........Ýÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FC3C 45FE FFFF FFFF FFFF FFFF"            /* ÿÿÿÿü<Eþÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF C603"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÆ. */
	$"0000 0000 0000 0000 0037 F7FF FFFF FFFF"            /* .........7÷ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FB43 0000 0000 0000 0000 0046 FDFF FFFF"            /* ûC.........Fýÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF5B 60FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿ[`ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF F934"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿù4 */
	$"0000 0000 0000 0000 0000 8FFF FFFF FFFF"            /* ..........ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"9D00 0000 0000 0000 0000 0097 FFFF FFFF"            /* ..........—ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF75 78FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿuxÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF8F"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 12D3 FFFF FFFF"            /* ...........Óÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFDF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿß */
	$"1A00 0000 0000 0000 0000 17E7 FFFF FFFF"            /* ...........çÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF8D 8DFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFE4"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿä */
	$"1600 0000 0000 0000 0000 003D F5FF FFFF"            /* ...........=õÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FA4E"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿúN */
	$"0000 0000 0000 0000 0000 6EFF FFFF FFFF"            /* ..........nÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFA1 9FFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿ¡Ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"7100 0000 0000 0000 0000 0000 6EFF FFFF"            /* q...........nÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 7C00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ|. */
	$"0000 0000 0000 0000 000C D4FF FFFF FFFF"            /* ..........Ôÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFB2 ACFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿ²¬ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"DB12 0000 0000 0000 0000 0000 0090 FFFF"            /* Û............ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF9D 0200"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 0000 0000 0000 0064 FFFF FFFF FFFF"            /* .........dÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFBE B9FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿ¾¹ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF7A 0000 0000 0000 0000 0000 0005 9CFF"            /* ÿz............œÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF A80A 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ¨Â.. */
	$"0000 0000 0000 0000 10D8 FFFF FFFF FFFF"            /* .........Øÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFCA D9FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÊÙÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFE9 2300 0000 0000 0000 0000 0000 0697"            /* ÿé#............— */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFA5 0A00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿ¥Â... */
	$"0000 0000 0000 0000 81FF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFE2 DFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿâßÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF A800 0000 0000 0000 0000 0000 0001"            /* ÿÿ¨............. */
	$"84FE FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* „þÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 8F05 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ..... */
	$"0000 0000 0000 002E F1FF FFFF FFFF FFFF"            /* ........ñÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFE6 C4FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿæÄÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FE5D 0000 0000 0000 0000 0000 0000"            /* ÿÿþ]............ */
	$"0060 EBFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .`ëÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF F06A 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿðj...... */
	$"0000 0000 0000 07BC FFFF FFFF FFFF FFFF"            /* .......¼ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFD3 AFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÓ¯ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFE8 2700 0000 0000 0000 0000 0000"            /* ÿÿÿè'........... */
	$"0000 2FBB FFFF FFFF FFFF FFFF FFFF FFFF"            /* ../»ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFC4 3700 0000 0000 0000"            /* ÿÿÿÿÿÿÿÄ7....... */
	$"0000 0000 0000 81FF FFFF FFFF FFFF FFFF"            /* ......ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFC2 A4FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÂ¤ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF C20E 0000 0000 0000 0000 0000"            /* ÿÿÿÿÂ........... */
	$"0000 0007 6FE2 FFFF FFFF FFFF FFFF FFFF"            /* ....oâÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFE7 780C 0000 0000 0000 0000"            /* ÿÿÿÿÿçx......... */
	$"0000 0000 0050 F9FF FFFF FFFF FFFF FFFF"            /* .....Pùÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFB7 93FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿ·“ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF9E 0100 0000 0000 0000 0000"            /* ÿÿÿÿÿž.......... */
	$"0000 0000 001F 8BE9 FFFF FFFF FFFF FFFF"            /* ......‹éÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFEE 9324 0000 0000 0000 0000 0000"            /* ÿÿÿî“$.......... */
	$"0000 0000 30E7 FFFF FFFF FFFF FFFF FFFF"            /* ....0çÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFA7 80FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿ§€ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 8400 0000 0000 0000 0000"            /* ÿÿÿÿÿÿ„......... */
	$"0000 0000 0000 0023 83D9 FFFF FFFF FFFF"            /* .......#ƒÙÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFDD 8929 0000 0000 0000 0000 0000 0000"            /* ÿÝ‰)............ */
	$"0000 0020 D6FF FFFF FFFF FFFF FFFF FFFF"            /* ... Öÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF94 69FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿ”iÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF7C 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÿ|........ */
	$"0000 0000 0000 0000 0010 519D DFFE FFFF"            /* ..........Qßþÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF E2A3"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿâ£ */
	$"5613 0000 0000 0000 0000 0000 0000 0000"            /* V............... */
	$"0000 1DCB FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...Ëÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF7E 4FFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿ~Oÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 7E00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿ~....... */
	$"0000 0000 0000 0000 0000 0000 1140 73A0"            /* .............@s  */
	$"C5E1 EFF7 FBFB FBF7 F0E2 C7A2 7643 1300"            /* Åáï÷ûûû÷ðâÇ¢vC.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0024 CBFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .$Ëÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF64 33F9 FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿd3ùÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF8F 0500 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ...... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"030E 1F2F 3839 3930 200F 0300 0000 0000"            /* .../8990 ....... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"32D6 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* 2Öÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FE47 17EA FFFF FFFF FFFF FFFF"            /* ÿÿÿÿþG.êÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF AA18 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿª..... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 004E"            /* ...............N */
	$"E6FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* æÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF F529 04CB FFFF FFFF FFFF FFFF"            /* ÿÿÿÿõ).Ëÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFD1 3C00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÑ<... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0007 7FF9"            /* ...............ù */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF DF0D 00A1 FFFF FFFF FFFF FFFF"            /* ÿÿÿÿß..¡ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF F278 0900"            /* ÿÿÿÿÿÿÿÿÿÿÿÿòxÆ. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 2FB8 FFFF"            /* ............/¸ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF B800 0073 FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ¸..sÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF C03E"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÀ> */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0D75 ECFF FFFF"            /* ...........uìÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 8A00 0045 FEFF FFFF FFFF FFFF"            /* ÿÿÿÿŠ..Eþÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFF7"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ÷ */
	$"9626 0000 0000 0000 0000 0000 0000 0000"            /* –&.............. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0352 CAFF FFFF FFFF"            /* .........RÊÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 5A00 0018 ECFF FFFF FFFF FFFF"            /* ÿÿÿÿZ...ìÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFEB 8D29 0000 0000 0000 0000 0000 0000"            /* ÿë)............ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0751 BCFF FFFF FFFF FFFF"            /* .......Q¼ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFF5 2900 0001 BEFF FFFF FFFF FFFF"            /* ÿÿÿõ)...¾ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFEE A047 0800 0000 0000 0000 0000"            /* ÿÿÿî G.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 1E6D C8FF FFFF FFFF FFFF FFFF"            /* .....mÈÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFD2 0600 0000 82FF FFFF FFFF FFFF"            /* ÿÿÿÒ....‚ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF CD80 3809 0000 0000 0000"            /* ÿÿÿÿÿÿÍ€8Æ...... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"001A 56A2 E9FF FFFF FFFF FFFF FFFF FFFF"            /* ..V¢éÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF98 0000 0000 42FD FFFF FFFF FFFF"            /* ÿÿÿ˜....Býÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FAD2 955F 2F0F 0100"            /* ÿÿÿÿÿÿÿÿúÒ•_/... */
	$"0000 0000 0000 0000 0000 0000 051B 4375"            /* ..............Cu */
	$"B0E8 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* °èÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF58 0000 0000 0CDC FFFF FFFF FFFF"            /* ÿÿÿX.....Üÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF F7DF BE91"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ÷ß¾‘ */
	$"0D00 0000 0000 0000 0000 0007 B4EC FEFF"            /* ............´ìþÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF EB1A 0000 0000 009A FFFF FFFF FFFF"            /* ÿÿë......šÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFF7"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ÷ */
	$"1A00 0000 0000 0000 0000 000A E5FF FFFF"            /* ...........Âåÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF B000 0000 0000 0050 FFFF FFFF FFFF"            /* ÿÿ°......Pÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 6300 0000 0000 0010 DFFF FFFF FFFF"            /* ÿÿc.......ßÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFEA 1A00 0000 0000 0000 8FFF FFFF FFFF"            /* ÿê........ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFA3 0000 0000 0000 0000 37FA FFFF FFFF"            /* ÿ£........7úÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF4B 0000 0000 0000 0000 02BD FFFF FFFF"            /* ÿK.........½ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"D109 0000 0000 0000 0000 005A FFFF FFFF"            /* ÑÆ.........Zÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"7000 0000 0000 0000 0000 000C D7FF FFFF"            /* p...........×ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFE6"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿæ */
	$"1800 0000 0000 0000 0000 0000 71FF FFFF"            /* ............qÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF88"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿˆ */
	$"0000 0000 0000 0000 0000 0000 12DE FFFF"            /* .............Þÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF EB1F"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿë. */
	$"0000 0000 0000 0000 0000 0000 0073 FFFF"            /* .............sÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 8800"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿˆ. */
	$"0000 0000 0000 0000 0000 0000 0010 D9FF"            /* ..............Ùÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFE4 1900"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿä.. */
	$"0000 0000 0000 0000 0000 0000 0000 5FFF"            /* .............._ÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF74 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿt.. */
	$"0000 0000 0000 0000 0000 0000 0000 05BE"            /* ...............¾ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF D10D 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÑ... */
	$"0000 0000 0000 0000 0000 0000 0000 0039"            /* ...............9 */
	$"F8FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* øÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFD 4C00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿýL... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"8EFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* Žÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFA4 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿ¤.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"12D4 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .Ôÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF E31E 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿã..... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0047 FAFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .Gúÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFE 5800 0000 0000"            /* ÿÿÿÿÿÿÿÿÿþX..... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 88FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ˆÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF9C 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿœ...... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0ABE FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..Â¾ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF CF13 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÏ....... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0025 E4FF FFFF FFFF FFFF FFFF FFFF"            /* ...%äÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFEF 3400 0000 0000 0000"            /* ÿÿÿÿÿÿÿï4....... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 4BF7 FFFF FFFF FFFF FFFF FFFF"            /* ....K÷ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FD5F 0000 0000 0000 0000"            /* ÿÿÿÿÿÿý_........ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0072 FFFF FFFF FFFF FFFF FFFF"            /* .....rÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 8400 0000 0000 0000 0000"            /* ÿÿÿÿÿÿ„......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 8EFF FFFF FFFF FFFF FFFF"            /* ......Žÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF9F 0200 0000 0000 0000 0000"            /* ÿÿÿÿÿŸ.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 04A1 FFFF FFFF FFFF FFFF"            /* .......¡ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF B30C 0000 0000 0000 0000 0000"            /* ÿÿÿÿ³........... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0009 AEFF FFFF FFFF FFFF"            /* .......Æ®ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFC0 1200 0000 0000 0000 0000 0000"            /* ÿÿÿÀ............ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0EB1 FFFF FFFF FFFF"            /* .........±ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF C018 0000 0000 0000 0000 0000 0000"            /* ÿÿÀ............. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 000C A5FF FFFF FFFF"            /* ..........¥ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFB4 1300 0000 0000 0000 0000 0000 0000"            /* ÿ´.............. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0690 FFFF FFFF"            /* ...........ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"A10D 0000 0000 0000 0000 0000 0000 0000"            /* ¡............... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 77FA FFFF"            /* ............wúÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF87"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ‡ */
	$"0200 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0055 E9FF"            /* .............Uéÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFF0 6300"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿðc. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 2FC4"            /* ............../Ä */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF D03B 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÐ;.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 000F"            /* ................ */
	$"92FD FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ’ýÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFA0 1700 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿ .... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0054 DCFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .TÜÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFE4 6000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿä`..... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 1D9D FDFF FFFF FFFF FFFF FFFF FFFF"            /* ...ýÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF AA26 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿª&...... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 4BCA FFFF FFFF FFFF FFFF FFFF"            /* ....KÊÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF D457 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÔW........ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 000D 73E0 FFFF FFFF FFFF FFFF"            /* ......sàÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF E77E 1300 0000 0000 0000 0000"            /* ÿÿÿÿç~.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 001B 87E9 FFFF FFFF FFFF"            /* ........‡éÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF EF92 2300 0000 0000 0000 0000 0000"            /* ÿÿï’#........... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0023 8AE4 FFFF FFFF"            /* .........#Šäÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"EA93 2B00 0000 0000 0000 0000 0000 0000"            /* ê“+............. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 001B 74D2 FFFF"            /* ............tÒÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF D97D"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÙ} */
	$"2200 0000 0000 0000 0000 0000 0000 0000"            /* "............... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 000C 52A7"            /* ..............R§ */
	$"ECFF FFFF FFFF FFFF FFFF FFFF FFFF FFEB"            /* ìÿÿÿÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFF0 AF59 1000"            /* ÿÿÿÿÿÿÿÿÿÿÿð¯Y.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"216C B7EF FFFF FFFF FFFF FFFF FFFF FFEB"            /* !l·ïÿÿÿÿÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FFFF FFFF F2BD 7327 0000 0000"            /* ÿÿÿÿÿÿÿÿò½s'.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0022 63A3 DBF9 FFFF FFFF FFFF FFEB"            /* ..."c£Ûùÿÿÿÿÿÿÿë */
	$"1900 0000 0000 0000 0000 000A DEFF FFFF"            /* ...........ÂÞÿÿÿ */
	$"FFFF FFFF FBDE A86A 2701 0000 0000 0000"            /* ÿÿÿÿûÞ¨j'....... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0D34 6697 C6E9 FAFF FFF2"            /* .......4f—Æéúÿÿò */
	$"1A00 0000 0000 0000 0000 000A E5FF FFFB"            /* ...........Âåÿÿû */
	$"EBCA 9C6B 3910 0000 0000 0000 0000 0000"            /* ëÊœk9........... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0317 3657 8CA2"            /* ............6WŒ¢ */
	$"1100 0000 0000 0000 0000 0007 9A93 5939"            /* ............š“Y9 */
	$"1904 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000"                                     /* ...... */
};

data 'icns' (129, "Map Icon") {
	$"6963 6E73 0000 15F9 4943 4E23 0000 0108"            /* icns...ùICN#.... */
	$"016D BFC0 0000 0020 07A0 0870 1958 0048"            /* .m¿À... . .p.X.H */
	$"36EC 104C 75B6 0B22 5AFE 187F EAB3 2055"            /* 6ì.Lu¶."Zþ..ê³ U */
	$"B5DD 0829 6F77 0103 B7EE 1001 D5DD 0411"            /* µÝ.)ow..·î..ÕÝ.. */
	$"6A26 0443 5D6D 2001 2A96 0415 1538 0901"            /* j&.C]m .*–...8Æ. */
	$"1AD0 0603 0520 8569 0100 0A13 0120 1081"            /* .Ð... …i..Â.. . */
	$"0110 610B 010F 8101 0102 0113 0101 0481"            /* ..a........... */
	$"0100 0043 0101 20C1 0100 0213 0103 10C1"            /* ...C.. Á.......Á */
	$"0105 A56B 0108 0001 0100 0413 01FF FFFF"            /* ..¥k.........ÿÿÿ */
	$"01FF FFC0 01FF FFE0 07FF FFF0 1FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð.ÿÿø */
	$"3FFF FFFC 7FFF FFFE 7FFF FFFF FFFF FFFF"            /* ?ÿÿü.ÿÿþ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"7FFF FFFF 7FFF FFFF 3FFF FFFF 1FFF FFFF"            /* .ÿÿÿ.ÿÿÿ?ÿÿÿ.ÿÿÿ */
	$"0FFF FFFF 03FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"6963 6C34 0000 0208 0000 000E DEED EEDE"            /* icl4........ÞíîÞ */
	$"EDEE EEEE EE00 0000 0000 000D 0000 0000"            /* íîîîî........... */
	$"0000 0000 0EE0 0000 0000 CDED EDEC 0000"            /* .....à....Íííì.. */
	$"C00C 700C 0EEE 0000 000E ED8E 8EDE E000"            /* À.p..î....íŽŽÞà. */
	$"0C00 70C0 0ECE E000 00AF C888 A8FD FE00"            /* ..pÀ.Îà..¯Èˆ¨ýþ. */
	$"000C CC0C CE0C EE00 0EFD 8888 899A DFEC"            /* ..Ì.Î.î..ýˆˆ‰šßì */
	$"0003 3077 0EC0 CEE0 C9AD 8B8B 8A9F D9F0"            /* ..0w.ÀÎàÉ­‹‹ŠŸÙð */
	$"C003 3700 0AEF EEAF DF9D A888 99AF DE9E"            /* À.7.Âïî¯ß¨ˆ™¯Þž */
	$"00C0 700C 0DDD DDDE E9ED 89B9 9A9F D99E"            /* .Àp..ÝÝÞéí‰¹šŸÙž */
	$"000C CC00 C0DD DDCF 899D A999 A9FE D99A"            /* ..Ì.ÀÝÝÏ‰©™©þÙš */
	$"0C00 700C 0C00 00DE A8A8 DAF9 FFFD 8AA9"            /* ..p....Þ¨¨ÚùÿýŠ© */
	$"000C 70C0 C0C0 C0DF 8A89 DDEE FECB A989"            /* ..pÀÀÀÀßŠ‰ÝîþË©‰ */
	$"00C0 070C 000C 0CCE D888 98DC CB88 88AD"            /* .À.....ÎØˆ˜ÜËˆˆ­ */
	$"000C 0D00 CC0C C0DF 0998 A88D D888 8A9C"            /* ....Ì.ÀßÆ˜¨ØˆŠœ */
	$"00C0 070C 00C0 00DE 0B88 88AB 1889 88D0"            /* .À...À.Þ.ˆˆ«.‰ˆÐ */
	$"C00C 0700 CC0C CCCF 008B 888D C8B8 9BC0"            /* À...Ì.ÌÏ.‹ˆÈ¸›À */
	$"0C00 C07C 00C0 C0CF 0008 888D D88A D000"            /* ..À|.ÀÀÏ..ˆØŠÐ. */
	$"0000 0330 C00C 0CD6 0000 0B8D D8DC 0000"            /* ...0À..Ö...ØÜ.. */
	$"C00C 0327 C77C 70DE 0000 000E 0000 0C00"            /* À..'Ç|pÞ........ */
	$"00C0 7C70 C0CC 0CCF 0000 000E 0070 0000"            /* .À|pÀÌ.Ï.....p.. */
	$"C0C7 0C70 C0C0 0CCF 0000 0009 0007 7000"            /* ÀÇ.pÀÀ.Ï...Æ..p. */
	$"0770 C007 0C0C C0DF 0000 000E 0000 D733"            /* .pÀ...Àß......×3 */
	$"700C 0C0D C00C 00DA 0000 000A 0000 0033"            /* p...À..Ú...Â...3 */
	$"00C0 00C7 0C0C C0DF 0000 000F 0000 0007"            /* .À.Ç..Àß........ */
	$"0C00 0C00 70C0 0CCF 0000 000E 0000 000D"            /* ....pÀ.Ï........ */
	$"000C 00C0 DC0C C0DF 0000 000F 0000 0C07"            /* ...ÀÜ.Àß........ */
	$"00C0 0C00 77C0 0CCF 0000 000E 0000 0007"            /* .À..wÀ.Ï........ */
	$"0000 C0C0 070C C0DF 0000 000F 0000 0033"            /* ..ÀÀ..Àß.......3 */
	$"0C0C 0000 32C0 0CCF 0000 000E 0000 0733"            /* ....2À.Ï.......3 */
	$"7C77 C77D 2377 CCDF 0000 000F 0007 7007"            /* |wÇ}#wÌß......p. */
	$"0000 C000 C700 C0CF 0000 000A 0000 0C00"            /* ..À.Ç.ÀÏ...Â.... */
	$"0C0C 0C0C 00CC 0CDF 0000 000E FAFF F9FF"            /* .....Ì.ß....úÿùÿ */
	$"FFFA FFFF FFFF FFFF 6963 6C38 0000 0408"            /* ÿúÿÿÿÿÿÿicl8.... */
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F5F5 00F5 97F5 F5F5 F5AC FBFC 0000 0000"            /* õõ.õ—õõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A082 FAAC FC00 00F5"            /* ..õû¬{|  ‚ú¬ü..õ */
	$"00F5 F5F5 97F5 F5F6 F5FC F7FB AC00 0000"            /* .õõõ—õõöõü÷û¬... */
	$"00F5 ACFE 50A1 A1A1 A7CB AD7A DFAC 0000"            /* .õ¬þP¡¡¡§Ë­zß¬.. */
	$"F500 F5F5 97F5 F6F5 F6AC F5F7 FBAC 0000"            /* õ.õõ—õöõö¬õ÷û¬.. */
	$"0081 FF7A 7D9B 9BA1 A1A7 E7AD FAEA FBF5"            /* .ÿz}››¡¡§ç­úêûõ */
	$"00F5 F523 23F5 9797 F5AC F5F6 F7FB AC00"            /* .õõ##õ——õ¬õö÷û¬. */
	$"2BE9 AD57 C57D 9BA1 A1A7 E8E9 F9E8 E92B"            /* +é­WÅ}›¡¡§èéùèé+ */
	$"F500 F523 2397 F5F5 F5AC ACAC ACAC ACFD"            /* õ.õ##—õõõ¬¬¬¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADEA FAA6 D1FB"            /* {éüu§¡¡¡§Ë­êú¦Ñû */
	$"00F5 F5F5 97F5 F5F6 F556 FAFA FAFA FAAC"            /* .õõõ—õõöõVúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBAD E8FF FAA7 E8A6"            /* ¦è¦ùË§¡§Ë­èÿú§è¦ */
	$"00F5 F5F5 97F5 F5F5 F6F5 5656 5656 56FD"            /* .õõõ—õõõöõVVVVVý */
	$"A7A7 E8F9 A7E7 A7CB ADE8 EAFD 56E7 A7AD"            /* §§èù§ç§Ë­èêýVç§­ */
	$"F5F5 00F5 97F5 F5F6 F5F6 F6F5 F6F6 F8FD"            /* õõ.õ—õõöõööõööøý */
	$"A7A7 A7A6 56E9 E9D1 D1E0 EA56 A0A7 A7D0"            /* §§§¦VééÑÑàêV §§Ð */
	$"00F5 F5F5 97F5 F5F5 F6F5 F6F6 F5F6 56FD"            /* .õõõ—õõõöõööõöVý */
	$"A0A7 E6A7 7B56 FCFD FEFC F87C A7A7 A7A6"            /*  §æ§{Vüýþüø|§§§¦ */
	$"F500 F5F5 F597 F5F6 F5F5 F6F5 2B01 55FD"            /* õ.õõõ—õöõõöõ+.Uý */
	$"57C5 A1A1 A7A0 572C 4F51 A1E6 A1A1 E7FA"            /* WÅ¡¡§ W,OQ¡æ¡¡çú */
	$"00F5 F5F5 F597 F5F5 F6F6 F5F6 F62A F8FD"            /* .õõõõ—õõööõöö*øý */
	$"25A7 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E8F7"            /* %§¡¡¡¡æzQæ¡¡¡¡è÷ */
	$"F500 F5F5 F597 F5F6 F5F5 F6F6 F5F6 56FD"            /* õ.õõõ—õöõõööõöVý */
	$"0075 A1A1 A19B A757 51A1 A1A1 9BE7 FA00"            /* .u¡¡¡›§WQ¡¡¡›çú. */
	$"01F5 F5F5 F597 F5F5 F6F6 F5F6 F6F6 56FD"            /* .õõõõ—õõööõöööVý */
	$"00F5 7CA1 9BA1 A17B 4AA1 779B E782 F500"            /* .õ|¡›¡¡{J¡w›ç‚õ. */
	$"F5F5 F5F5 F5F5 97F6 F5F5 F6F6 F6F6 F8FE"            /* õõõõõõ—öõõööööøþ */
	$"0000 F57B A19B A156 51E3 A1AD 7B00 00F5"            /* ..õ{¡›¡VQã¡­{..õ */
	$"00F5 00F5 F523 23F5 F6F5 F6F5 F6F6 56D2"            /* .õ.õõ##õöõöõööVÒ */
	$"0000 0000 2B7B A67B 57A6 FA4F 0000 F500"            /* ....+{¦{W¦úO..õ. */
	$"F5F5 F5F5 F523 2397 9797 9797 97F5 56FD"            /* õõõõõ##——————õVý */
	$"0000 0000 0000 00FC 0000 0000 00F5 00F5"            /* .......ü.....õ.õ */
	$"00F5 F5F5 9797 97F5 F5F6 F6F6 F6F6 56E0"            /* .õõõ———õõöööööVà */
	$"0000 0000 0000 00AC 0000 9700 0000 F500"            /* .......¬..—...õ. */
	$"F500 9797 F5F5 97F5 F6F5 F6F5 F6F6 F8FE"            /* õ.——õõ—õöõöõööøþ */
	$"0000 0000 0000 00AC 0000 0097 9700 00F5"            /* .......¬...——..õ */
	$"0097 97F5 F5F5 F597 F5F6 F5F6 F6F6 56FE"            /* .——õõõõ—õöõöööVþ */
	$"0000 0000 0000 00AC 0000 0000 9797 2323"            /* .......¬....——## */
	$"97F5 F5F5 F5F5 F597 F6F5 F6F6 F6F5 56FE"            /* —õõõõõõ—öõöööõVþ */
	$"0000 0000 0000 00AC 0000 0000 00F5 2323"            /* .......¬.....õ## */
	$"F500 F5F5 F5F5 F697 F5F6 F5F6 F6F6 56EA"            /* õ.õõõõö—õöõöööVê */
	$"0000 0000 0000 00AC 0000 0000 F500 0097"            /* .......¬....õ..— */
	$"00F5 F5F5 F5F5 F5F5 97F5 F6F5 F6F6 F8E0"            /* .õõõõõõõ—õöõööøà */
	$"0000 0000 0000 00AC 0000 0000 0000 F597"            /* .......¬......õ— */
	$"F500 F5F5 F5F5 F6F5 97F6 F6F6 F6F6 56EA"            /* õ.õõõõöõ—öööööVê */
	$"0000 0000 0000 00FD 0000 0000 00F5 0097"            /* .......ý.....õ.— */
	$"00F5 F5F5 F5F5 F5F6 9797 F6F5 F6F6 F8F4"            /* .õõõõõõö——öõööøô */
	$"0000 0000 0000 00AC 0000 0000 0000 F597"            /* .......¬......õ— */
	$"F500 F5F5 F5F5 F5F5 F697 F5F6 F6F6 56FF"            /* õ.õõõõõõö—õöööVÿ */
	$"0000 0000 0000 00FD 0000 0000 F500 2323"            /* .......ý....õ.## */
	$"00F5 F5F5 F5F5 F5F5 2323 F6F6 F6F6 F8FF"            /* .õõõõõõõ##ööööøÿ */
	$"0000 0000 0000 00FD 0000 0000 0097 2323"            /* .......ý.....—## */
	$"9797 9797 9797 9797 2323 9797 97F6 56FF"            /* ————————##———öVÿ */
	$"0000 0000 0000 00FD 0000 0097 9700 0097"            /* .......ý...——..— */
	$"F500 F5F5 F5F5 F5F6 F597 F6F5 F6F5 56FF"            /* õ.õõõõõöõ—öõöõVÿ */
	$"0000 0000 0000 00FD 0000 0000 00F5 00F5"            /* .......ý.....õ.õ */
	$"00F5 F5F5 F5F5 F5F5 F6F5 F6F6 F6F6 56FF"            /* .õõõõõõõöõööööVÿ */
	$"0000 0000 0000 00FD FDFD FEFE FEFE EAFE"            /* .......ýýýþþþþêþ */
	$"EAE0 EAFE FFFF FFFF FFFF FFFF FFFF FFFF"            /* êàêþÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"696C 3332 0000 0AD1 84FF 125E 5D5C 5B5A"            /* il32..ÂÑ„ÿ.^]\[Z */
	$"5958 5756 5453 5150 4E4D 4B49 4846 8AFF"            /* YXWVTSQPNMKIHFŠÿ */
	$"135D FFFF FEFD FBFA F8F6 F5F3 F1EF EEEC"            /* .]ÿÿþýûúøöõóñïîì */
	$"EAE8 E744 4286 FF17 B76E 6968 6E66 6DB6"            /* êèçDB†ÿ.·nihnfm¶ */
	$"FBFA F8F6 F5F3 F1EF 33EC EAE8 E742 5942"            /* ûúøöõóñï3ìêèçBYB */
	$"83FF 1AEC 5432 775E 4741 526F 2E54 FAF8"            /* ƒÿ.ìT2w^GARo.Túø */
	$"F6F5 F3F1 EF33 ECEA E8E7 40B3 5933 81FF"            /* öõóñï3ìêèç@³Y3ÿ */
	$"1CED 391A 8B3D 2F30 2615 1E7E 1639 F8F6"            /* .í9.‹=/0&..~.9øö */
	$"F5F3 F1EF 33EC EAE8 E73D DEB3 5933 80FF"            /* õóñï3ìêèç=Þ³Y3€ÿ */
	$"5861 007F 4937 413A 2F24 102B 7600 5FF6"            /* Xa..I7A:/$.+v._ö */
	$"F5F3 F1FF FFEC 3333 E73B EEDE B359 33FF"            /* õóñÿÿì33ç;îÞ³Y3ÿ */
	$"C503 2481 293C 3E39 2E23 1904 7623 02C1"            /* Å.$)<>9.#..v#.Á */
	$"F5F3 F1FF FF33 EAE8 E739 3736 3432 312F"            /* õóñÿÿ3êèç976421/ */
	$"7000 3B78 2133 3531 281F 1600 6E3A 0068"            /* p.;x!351(...n:.h */
	$"F5F3 F1EF 33EC EAE8 E79C 8275 1A2D 3E0F"            /* õóñï3ìêèçœ‚u.->. */
	$"357D 1828 2927 211A 1100 7934 1034 F5F3"            /* 5}.()'!...y4.4õó */
	$"F1EF 33EC EAE8 E7E5 829C 7F2B 311D 1A8C"            /* ñï3ìêèçå‚œ.+1..Œ */
	$"2617 1E1D 1913 031D 8C1B 1D24 F5F3 F1EF"            /* &.......Œ..$õóñï */
	$"33EC EAE8 E7E5 E3E2 E0DF 9C29 3523 1953"            /* 3ìêèçåãâàßœ)5#.S */
	$"880A 090E 0B00 078C 501A 2326 F5F3 F1EF"            /* ˆÂÆ....ŒP.#&õóñï */
	$"33EC EAE8 E7E5 E3E2 E0DF 9C27 4823 281C"            /* 3ìêèçåãâàßœ'H#(. */
	$"6890 4517 1548 9466 1D29 2236 F5F3 F1EF"            /* hE..H”f.)"6õóñï */
	$"EE33 EAE8 E7E5 E3E2 E0DF 9C25 7C20 2E2E"            /* î3êèçåãâàßœ%| .. */
	$"2248 7AB4 B57C 4623 2D2F 1769 F5F3 F1EF"            /* "Hz´µ|F#-/.iõóñï */
	$"EE33 EAE8 E7E5 E3E2 E0DF 9C7F 23C9 2832"            /* î3êèçåãâàßœ.#É(2 */
	$"3333 2F13 888D 1C2E 3334 3112 C0F5 F3F1"            /* 33/.ˆ..341.Àõóñ */
	$"EFEE 33EA E8E7 E5E3 E2E0 DF9C 21FF 7726"            /* ïî3êèçåãâàßœ!ÿw& */
	$"3938 3924 888F 2C38 393A 1562 F6F5 F3F1"            /* 989$ˆ,89:.böõóñ */
	$"EFEE 33EA E8E7 E5E3 E2E0 DF9C 1FFF EE5B"            /* ïî3êèçåãâàßœ.ÿî[ */
	$"283C 3F28 8993 323F 3B18 43F8 F6F5 F3F1"            /* (<?(‰“2?;.Cøöõóñ */
	$"EFEE EC33 E8E7 E5E3 E2E0 DF9C 1CFF FFED"            /* ïîì3èçåãâàßœ.ÿÿí */
	$"722A 2F25 8994 2D26 1960 FAF8 F6F5 F3F1"            /* r*.%‰”-&.`úøöõóñ */
	$"EFEE FFFF E8E7 E5E3 E2E0 DF9C 0018 81FF"            /* ïîÿÿèçåãâàßœ..ÿ */
	$"12C3 6F2F 878C 346B BDFB FAF8 F6F5 F3F1"            /* .Ão/‡Œ4k½ûúøöõóñ */
	$"EFEE FFFF 8333 02DF 9C18 84FF 0C3D FFFF"            /* ïîÿÿƒ3.ßœ.„ÿ.=ÿÿ */
	$"FEFD FBFA F8F6 F5F3 F1EF 8033 08E8 E7E5"            /* þýûúøöõóñï€3.èçå */
	$"E3E2 E0DF 9C16 84FF 183B FFFF 33FD FBFA"            /* ãâàßœ.„ÿ.;ÿÿ3ýûú */
	$"F8F6 F5F3 3333 EEEC 33E8 E7E5 E3E2 E0DF"            /* øöõó33îì3èçåãâàß */
	$"9C14 84FF 1839 FFFF FE33 33FA F8F6 F533"            /* œ.„ÿ.9ÿÿþ33úøöõ3 */
	$"33EF EEEC EA33 E7E5 E3E2 E0DF 9C12 84FF"            /* 3ïîìê3çåãâàßœ.„ÿ */
	$"1837 FFFF FEFD 3333 FFFF 33F3 F1EF EEEC"            /* .7ÿÿþý33ÿÿ3óñïîì */
	$"EA33 E7E5 E3E2 E0DF 9C10 84FF 1835 FFFF"            /* ê3çåãâàßœ.„ÿ.5ÿÿ */
	$"FEFD FBFA FFFF F5F3 F1EF EEEC EA33 E7E5"            /* þýûúÿÿõóñïîìê3çå */
	$"E3E2 E0DF 9C0E 84FF 1833 FFFF FEFD FBFA"            /* ãâàßœ.„ÿ.3ÿÿþýûú */
	$"F833 F5F3 F1EF EEEC EAE8 33E5 E3E2 E0DF"            /* ø3õóñïîìêè3åãâàß */
	$"9C0C 84FF 1831 FFFF FEFD FBFA F833 F5F3"            /* œ.„ÿ.1ÿÿþýûúø3õó */
	$"F1EF EEEC EAE8 33E5 E3E2 E0DF 9C0A 84FF"            /* ñïîìêè3åãâàßœÂ„ÿ */
	$"182F FFFF FEFD FBFA F833 F5F3 F1EF EEEC"            /* ./ÿÿþýûúø3õóñïîì */
	$"EAE8 3333 E3E2 E0DF 9C08 84FF 182D FFFF"            /* êè33ãâàßœ.„ÿ.-ÿÿ */
	$"FEFD FBFA F833 F5F3 F1EF EEEC EAE8 E733"            /* þýûúø3õóñïîìêèç3 */
	$"E3E2 E0DF 9C06 84FF 182B FFFF FEFD FBFA"            /* ãâàßœ.„ÿ.+ÿÿþýûú */
	$"FFFF F5F3 F1EF EEEC EAE8 FFFF E3E2 E0DF"            /* ÿÿõóñïîìêèÿÿãâàß */
	$"9C05 84FF 0829 FFFF FEFD FB33 FFFF 8533"            /* œ.„ÿ.)ÿÿþýû3ÿÿ…3 */
	$"01FF FF80 3302 DF9C 0384 FF18 26FF FFFE"            /* .ÿÿ€3.ßœ.„ÿ.&ÿÿþ */
	$"3333 FAF8 33F5 F3F1 EFEE ECEA E8E7 33E3"            /* 33úø3õóñïîìêèç3ã */
	$"E2E0 DF9C 0284 FF18 24FF FFFE FDFB FAF8"            /* âàßœ.„ÿ.$ÿÿþýûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"0184 FF18 2220 1F1D 1B1A 1816 1513 1110"            /* .„ÿ." .......... */
	$"0E0D 0B0A 0807 0605 0403 0201 0084 FF12"            /* ...Â.........„ÿ. */
	$"5E5D 5C5B 5A59 5857 5654 5351 504E 4D4B"            /* ^]\[ZYXWVTSQPNMK */
	$"4948 468A FF13 5DFF FFFE FDFB FAF8 F6F5"            /* IHFŠÿ.]ÿÿþýûúøöõ */
	$"F3F1 EFEE ECEA E8E7 4442 86FF 17B6 6A67"            /* óñïîìêèçDB†ÿ.¶jg */
	$"6C71 656C B7FB FAF8 F6F5 F3F1 EFCC ECEA"            /* lqel·ûúøöõóñïÌìê */
	$"E8E7 4259 4283 FF1A ED55 2F94 A693 8079"            /* èçBYBƒÿ.íU/”¦“€y */
	$"782D 55FA F8F6 F5F3 F1EF CCEC EAE8 E740"            /* x-UúøöõóñïÌìêèç@ */
	$"B359 3381 FF1C EE3E 19B6 ADA5 987C 5848"            /* ³Y3ÿ.î>.¶­¥˜|XH */
	$"861B 3DF8 F6F5 F3F1 EFCC ECEA E8E7 3DDE"            /* †.=øöõóñïÌìêèç=Þ */
	$"B359 3380 FF58 6F00 90AB B2BE AC8A 6942"            /* ³Y3€ÿXo.«²¾¬ŠiB */
	$"3E7E 0667 F6F5 F3F1 0000 ECCC CCE7 3BEE"            /* >~.göõóñ..ìÌÌç;î */
	$"DEB3 5933 FFCC 2233 A891 B1B8 A787 674A"            /* Þ³Y3ÿÌ"3¨‘±¸§‡gJ */
	$"1C7B 4219 C4F5 F3F1 0000 CCEA E8E7 3937"            /* .{B.Äõóñ..Ìêèç97 */
	$"3634 3231 2F88 2A4F 9F7B 979C 8F76 5D42"            /* 6421/ˆ*OŸ{—œv]B */
	$"1273 5F28 75F5 F3F1 EFCC ECEA E8E7 9C82"            /* .s_(uõóñïÌìêèçœ‚ */
	$"751A 2D67 4453 985F 7679 7161 4C33 087E"            /* u.-gDS˜_vyqaL3.~ */
	$"6445 4CF5 F3F1 EFCC ECEA E8E7 E582 9C7F"            /* dELõóñïÌìêèçå‚œ. */
	$"2B66 5A4E 9C52 535B 5549 3616 1BA0 5A5A"            /* +fZNœRS[UI6.. ZZ */
	$"44F5 F3F1 EFCC ECEA E8E7 E5E3 E2E0 DF9C"            /* DõóñïÌìêèçåãâàßœ */
	$"2971 685F 7C92 2A2E 3227 0F03 8D8B 6268"            /* )qh_|’*.2'..‹bh */
	$"46F5 F3F1 EFCC ECEA E8E7 E5E3 E2E0 DF9C"            /* FõóñïÌìêèçåãâàßœ */
	$"2787 7478 6B97 9745 2119 419C A170 786F"            /* '‡txk——E!.Aœ¡pxo */
	$"53F5 F3F1 EFEE CCEA E8E7 E5E3 E2E0 DF9C"            /* SõóñïîÌêèçåãâàßœ */
	$"25AD 7E86 877E 96A6 B8C8 B499 8086 8B64"            /* %­~†‡~–¦¸È´™€†‹d */
	$"77F5 F3F1 EFEE CCEA E8E7 E5E3 E2E0 DF9C"            /* wõóñïîÌêèçåãâàßœ */
	$"7F23 DD8B 9897 9898 739B C88B 9497 9997"            /* .#Ý‹˜—˜˜s›È‹”—™— */
	$"48C0 F5F3 F1EF EECC EAE8 E7E5 E3E2 E0DF"            /* HÀõóñïîÌêèçåãâàß */
	$"9C21 FEAF 99A9 A6A9 879C CEA1 A6A8 AC66"            /* œ!þ¯™©¦©‡œÎ¡¦¨¬f */
	$"6FF6 F5F3 F1EF EECC EAE8 E7E5 E3E2 E0DF"            /* oöõóñïîÌêèçåãâàß */
	$"9C1F FFF2 A1A1 B7BA 949F D6B1 B9B4 705A"            /* œ.ÿò¡¡·º”ŸÖ±¹´pZ */
	$"F8F6 F5F3 F1EF EEEC CCE8 E7E5 E3E2 E0DF"            /* øöõóñïîìÌèçåãâàß */
	$"9C1C FFFF EFA6 91AC 97A1 DBAF 915B 71FA"            /* œ.ÿÿï¦‘¬—¡Û¯‘[qú */
	$"F8F6 F5F3 F1EF EE00 00E8 E7E5 E3E2 E0DF"            /* øöõóñïî..èçåãâàß */
	$"9C00 1880 FF13 FECF 9865 93AF 6680 BEFB"            /* œ..€ÿ.þÏ˜e“¯f€¾û */
	$"FAF8 F6F5 F3F1 EFEE 0000 83CC 02DF 9C18"            /* úøöõóñïî..ƒÌ.ßœ. */
	$"84FF 0C3D FFFF FEFD FBFA F8F6 F5F3 F1EF"            /* „ÿ.=ÿÿþýûúøöõóñï */
	$"80CC 08E8 E7E5 E3E2 E0DF 9C16 84FF 183B"            /* €Ì.èçåãâàßœ.„ÿ.; */
	$"FFFF CCFD FBFA F8F6 F5F3 CCCC EEEC CCE8"            /* ÿÿÌýûúøöõóÌÌîìÌè */
	$"E7E5 E3E2 E0DF 9C14 84FF 1839 FFFF FECC"            /* çåãâàßœ.„ÿ.9ÿÿþÌ */
	$"CCFA F8F6 F5CC CCEF EEEC EACC E7E5 E3E2"            /* ÌúøöõÌÌïîìêÌçåãâ */
	$"E0DF 9C12 84FF 1837 FFFF FEFD CCCC 0000"            /* àßœ.„ÿ.7ÿÿþýÌÌ.. */
	$"CCF3 F1EF EEEC EACC E7E5 E3E2 E0DF 9C10"            /* ÌóñïîìêÌçåãâàßœ. */
	$"84FF 1835 FFFF FEFD FBFA 0000 F5F3 F1EF"            /* „ÿ.5ÿÿþýûú..õóñï */
	$"EEEC EACC E7E5 E3E2 E0DF 9C0E 84FF 1833"            /* îìêÌçåãâàßœ.„ÿ.3 */
	$"FFFF FEFD FBFA F8CC F5F3 F1EF EEEC EAE8"            /* ÿÿþýûúøÌõóñïîìêè */
	$"CCE5 E3E2 E0DF 9C0C 84FF 1831 FFFF FEFD"            /* Ìåãâàßœ.„ÿ.1ÿÿþý */
	$"FBFA F8CC F5F3 F1EF EEEC EAE8 CCE5 E3E2"            /* ûúøÌõóñïîìêèÌåãâ */
	$"E0DF 9C0A 84FF 182F FFFF FEFD FBFA F8CC"            /* àßœÂ„ÿ./ÿÿþýûúøÌ */
	$"F5F3 F1EF EEEC EAE8 CCCC E3E2 E0DF 9C08"            /* õóñïîìêèÌÌãâàßœ. */
	$"84FF 182D FFFF FEFD FBFA F8CC F5F3 F1EF"            /* „ÿ.-ÿÿþýûúøÌõóñï */
	$"EEEC EAE8 E7CC E3E2 E0DF 9C06 84FF 182B"            /* îìêèçÌãâàßœ.„ÿ.+ */
	$"FFFF FEFD FBFA 0000 F5F3 F1EF EEEC EAE8"            /* ÿÿþýûú..õóñïîìêè */
	$"0000 E3E2 E0DF 9C05 84FF 0829 FFFF FEFD"            /* ..ãâàßœ.„ÿ.)ÿÿþý */
	$"FBCC 0000 85CC 0100 0080 CC02 DF9C 0384"            /* ûÌ..…Ì...€Ì.ßœ.„ */
	$"FF18 26FF FFFE CCCC FAF8 CCF5 F3F1 EFEE"            /* ÿ.&ÿÿþÌÌúøÌõóñïî */
	$"ECEA E8E7 CCE3 E2E0 DF9C 0284 FF18 24FF"            /* ìêèçÌãâàßœ.„ÿ.$ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* ÿþýûúøöõóñïîìêèç */
	$"E5E3 E2E0 DF9C 0184 FF18 2220 1F1D 1B1A"            /* åãâàßœ.„ÿ." .... */
	$"1816 1513 1110 0E0D 0B0A 0807 0605 0403"            /* .........Â...... */
	$"0201 0084 FF12 5E5D 5C5B 5A59 5857 5654"            /* ...„ÿ.^]\[ZYXWVT */
	$"5351 504E 4D4B 4948 468A FF13 5DFF FFFE"            /* SQPNMKIHFŠÿ.]ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 4442"            /* ýûúøöõóñïîìêèçDB */
	$"86FF 17B7 7069 666D 666E B6FB FAF8 F6F5"            /* †ÿ.·pifmfn¶ûúøöõ */
	$"F3F1 EFCC ECEA E8E7 4259 4283 FF1A EC54"            /* óñïÌìêèçBYBƒÿ.ìT */
	$"3469 4423 2447 6B2E 54FA F8F6 F5F3 F1EF"            /* 4iD#$Gk.Túøöõóñï */
	$"CCEC EAE8 E740 B359 3381 FF04 EC37 1C78"            /* Ììêèç@³Y3ÿ.ì7.x */
	$"0F81 0013 147B 1539 F8F6 F5F3 F1EF CCEC"            /* ....{.9øöõóñïÌì */
	$"EAE8 E73D DEB3 5933 80FF 035C 0078 1B83"            /* êèç=Þ³Y3€ÿ.\.x.ƒ */
	$"0018 2472 005C F6F5 F3F1 0000 ECCC CCE7"            /* ..$r.\öõóñ..ìÌÌç */
	$"3BEE DEB3 5933 FFC1 001E 6E85 0017 7316"            /* ;îÞ³Y3ÿÁ..n…..s. */
	$"00C0 F5F3 F100 00CC EAE8 E739 3736 3432"            /* .Àõóñ..Ìêèç97642 */
	$"312F 6700 3463 8500 0D6D 2800 66F5 F3F1"            /* 1/g.4c…..m(.fõóñ */
	$"EFCC ECEA E8E7 9C82 7504 2D2D 0027 7185"            /* ïÌìêèçœ‚u.--.'q… */
	$"000D 771D 002C F5F3 F1EF CCEC EAE8 E7E5"            /* ..w..,õóñïÌìêèçå */
	$"829C 052B 1600 0787 1583 001A 1D84 0400"            /* ‚œ.+...‡.ƒ...„.. */
	$"15F5 F3F1 EFCC ECEA E8E7 E5E3 E2E0 DF9C"            /* .õóñïÌìêèçåãâàßœ */
	$"2916 0000 4386 0581 0016 0A8A 3900 0015"            /* )...C†...ÂŠ9... */
	$"F5F3 F1EF CCEC EAE8 E7E5 E3E2 E0DF 9C27"            /* õóñïÌìêèçåãâàßœ' */
	$"2D80 0007 578D 4714 154B 8F4E 8000 112C"            /* -€..WG..KN€.., */
	$"F5F3 F1EF EECC EAE8 E7E5 E3E2 E0DF 9C48"            /* õóñïîÌêèçåãâàßœH */
	$"6781 0005 2768 B1AC 6622 8100 1166 F5F3"            /* g..'h±¬f"..fõó */
	$"F1EF EECC EAE8 E7E5 E3E2 E0DF 9C23 C183"            /* ñïîÌêèçåãâàßœ#Áƒ */
	$"0001 7E72 8300 12C0 F5F3 F1EF EECC EAE8"            /* ..~rƒ..ÀõóñïîÌêè */
	$"E7E5 E3E2 E0DF 9C21 FF5D 8200 017E 7082"            /* çåãâàßœ!ÿ]‚..~p‚ */
	$"0014 5DF6 F5F3 F1EF EECC EAE8 E7E5 E3E2"            /* ..]öõóñïîÌêèçåãâ */
	$"E0DF 9C1F FFEC 3D81 0001 7E71 8100 163D"            /* àßœ.ÿì=..~q..= */
	$"F8F6 F5F3 F1EF EEEC CCE8 E7E5 E3E2 E0DF"            /* øöõóñïîìÌèçåãâàß */
	$"9C1C FFFF EB5F 8000 017E 7180 0013 5FFA"            /* œ.ÿÿë_€..~q€.._ú */
	$"F8F6 F5F3 F1EF EE00 00E8 E7E5 E3E2 E0DF"            /* øöõóñïî..èçåãâàß */
	$"9C18 81FF 12BF 6021 807B 2766 BEFB FAF8"            /* œ.ÿ.¿`!€{'f¾ûúø */
	$"F6F5 F3F1 EFEE 0000 83CC 02DF 9C18 84FF"            /* öõóñïî..ƒÌ.ßœ.„ÿ */
	$"0C3D FFFF FEFD FBFA F8F6 F5F3 F1EF 80CC"            /* .=ÿÿþýûúøöõóñï€Ì */
	$"08E8 E7E5 E3E2 E0DF 9C16 84FF 183B FFFF"            /* .èçåãâàßœ.„ÿ.;ÿÿ */
	$"CCFD FBFA F8F6 F5F3 CCCC EEEC CCE8 E7E5"            /* ÌýûúøöõóÌÌîìÌèçå */
	$"E3E2 E0DF 9C14 84FF 1839 FFFF FECC CCFA"            /* ãâàßœ.„ÿ.9ÿÿþÌÌú */
	$"F8F6 F5CC CCEF EEEC EACC E7E5 E3E2 E0DF"            /* øöõÌÌïîìêÌçåãâàß */
	$"9C12 84FF 1837 FFFF FEFD CCCC 0000 CCF3"            /* œ.„ÿ.7ÿÿþýÌÌ..Ìó */
	$"F1EF EEEC EACC E7E5 E3E2 E0DF 9C10 84FF"            /* ñïîìêÌçåãâàßœ.„ÿ */
	$"1835 FFFF FEFD FBFA 0000 F5F3 F1EF EEEC"            /* .5ÿÿþýûú..õóñïîì */
	$"EACC E7E5 E3E2 E0DF 9C0E 84FF 1833 FFFF"            /* êÌçåãâàßœ.„ÿ.3ÿÿ */
	$"FEFD FBFA F8CC F5F3 F1EF EEEC EAE8 CCE5"            /* þýûúøÌõóñïîìêèÌå */
	$"E3E2 E0DF 9C0C 84FF 1831 FFFF FEFD FBFA"            /* ãâàßœ.„ÿ.1ÿÿþýûú */
	$"F8CC F5F3 F1EF EEEC EAE8 CCE5 E3E2 E0DF"            /* øÌõóñïîìêèÌåãâàß */
	$"9C0A 84FF 182F FFFF FEFD FBFA F8CC F5F3"            /* œÂ„ÿ./ÿÿþýûúøÌõó */
	$"F1EF EEEC EAE8 CCCC E3E2 E0DF 9C08 84FF"            /* ñïîìêèÌÌãâàßœ.„ÿ */
	$"182D FFFF FEFD FBFA F8CC F5F3 F1EF EEEC"            /* .-ÿÿþýûúøÌõóñïîì */
	$"EAE8 E7CC E3E2 E0DF 9C06 84FF 182B FFFF"            /* êèçÌãâàßœ.„ÿ.+ÿÿ */
	$"FEFD FBFA 0000 F5F3 F1EF EEEC EAE8 0000"            /* þýûú..õóñïîìêè.. */
	$"E3E2 E0DF 9C05 84FF 0829 FFFF FEFD FBCC"            /* ãâàßœ.„ÿ.)ÿÿþýûÌ */
	$"0000 85CC 0100 0080 CC02 DF9C 0384 FF18"            /* ..…Ì...€Ì.ßœ.„ÿ. */
	$"26FF FFFE CCCC FAF8 CCF5 F3F1 EFEE ECEA"            /* &ÿÿþÌÌúøÌõóñïîìê */
	$"E8E7 CCE3 E2E0 DF9C 0284 FF18 24FF FFFE"            /* èçÌãâàßœ.„ÿ.$ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 0184 FF18 2220 1F1D 1B1A 1816"            /* âàßœ.„ÿ." ...... */
	$"1513 1110 0E0D 0B0A 0807 0605 0403 0201"            /* .......Â........ */
	$"006C 386D 6B00 0004 0800 0000 0000 0000"            /* .l8mk........... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF00 0000 0000 0000 0000 0000 0000"            /* ÿÿÿ............. */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 0000 0000 0000 0000 FFFF"            /* ÿÿÿÿ..........ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 0000 0000 FFFF FFFF"            /* ÿÿÿÿÿ.......ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 00FF FFFF FFFF"            /* ÿÿÿÿÿÿ.....ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿ..ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿ.ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿ.ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 00FF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿ..ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿ...ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿ....ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 00FF"            /* ÿÿÿÿÿÿÿÿÿ......ÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF"                             /* ÿÿÿÿÿÿÿÿÿ */
};

data 'icns' (130, "Sounds Icon") {
	$"6963 6E73 0000 1BF2 4943 4E23 0000 0108"            /* icns...òICN#.... */
	$"016D BBC0 0100 0020 0550 0070 1AB8 0048"            /* .m»À... .P.p.¸.H */
	$"35CC 0254 6B76 0046 6ABE 207B 76FB 0015"            /* 5Ì.Tkv.Fj¾ {vû.. */
	$"DB76 044B 6DF7 0001 B7EB 0102 6BCD 1089"            /* Ûv.Km÷..·ë..kÍ.‰ */
	$"B436 0083 6B5B 20A1 34AC 3185 0B54 3081"            /* ´6.ƒk[ ¡4¬1….T0 */
	$"1578 2193 0A40 38C1 0102 3143 0106 3489"            /* .x!“Â@8Á..1C..4‰ */
	$"0106 55D3 0135 5D31 0135 2D6B 0135 5B69"            /* ..UÓ.5]1.5-k.5[i */
	$"0159 5B6B 0108 CB65 0104 CB63 0100 C321"            /* .Y[k..Ëe..Ëc..Ã! */
	$"0100 8283 0100 0109 0100 2003 01FF FFFF"            /* ..‚ƒ...Æ.. ..ÿÿÿ */
	$"01FF FFC0 01FF FFE0 07FF FFF0 1FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð.ÿÿø */
	$"3FFF FFFC 7FFF FFFE 7FFF FFFF FFFF FFFF"            /* ?ÿÿü.ÿÿþ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 7FFF FFFF 7FFF FFFF 3FFF FFFF"            /* ÿÿÿÿ.ÿÿÿ.ÿÿÿ?ÿÿÿ */
	$"1FFF FFFF 07FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"6963 6C34 0000 0208 0000 000E DEDE EDEE"            /* icl4........ÞÞíî */
	$"DEEE EEEE EE00 0000 0000 000D 0000 0000"            /* Þîîîî........... */
	$"C000 0000 0EE0 0000 0000 CDED EDEC 0000"            /* À....à....Íííì.. */
	$"000C 00C0 CEDE 0000 000E AD8B 8EDE E000"            /* ...ÀÎÞ....­‹ŽÞà. */
	$"00C0 0C0C 0ECE A000 00EF D888 999B FE0C"            /* .À...Î ..ïØˆ™›þ. */
	$"0000 C000 CECC EE00 0EFC 888A 89AE DFE0"            /* ..À.ÎÌî..üˆŠ‰®ßà */
	$"000C 000C 0E00 CEE0 CF9D 8818 8A9F D9FC"            /* ......ÎàÏˆ.ŠŸÙü */
	$"0000 C0C0 CEEF EEEA D9ED A898 A99F DE9B"            /* ..ÀÀÎïîêÙí¨˜©ŸÞ› */
	$"00C0 0C00 0DDD DDDF 9A9B 898A 8AFF D9A9"            /* .À...ÝÝßš›‰ŠŠÿÙ© */
	$"000C 00C0 C0DD CDDE A98D A9A8 F9FE D99F"            /* ...ÀÀÝÍÞ©©¨ùþÙŸ */
	$"00C0 000C 0C00 C0DE 8A9E D9FF 9FFD 8A8E"            /* .À....ÀÞŠžÙÿŸýŠŽ */
	$"000C 0C00 00C0 C0DE E898 DDE9 FECB 98A9"            /* .....ÀÀÞè˜ÝéþË˜© */
	$"00C0 000C EC0C 0CCF D8A8 8BDC CD88 A89B"            /* .À..ì..ÏØ¨‹ÜÍˆ¨› */
	$"000C 00C0 F0C0 C0DE 0989 888D B888 889C"            /* ...ÀðÀÀÞÆ‰ˆ¸ˆˆœ */
	$"0CCC 0C00 FC0C 0CCF 0B88 A88D C8A8 8AE0"            /* .Ì..ü..Ï.ˆ¨È¨Šà */
	$"00FD 00CC F0C0 C0DE 00B8 888D 1888 9E00"            /* .ý.ÌðÀÀÞ.¸ˆ.ˆž. */
	$"00EE C00C FC0C 0CCF 000B 888D D88A D000"            /* .îÀ.ü..Ï..ˆØŠÐ. */
	$"C0FD 0C0C FCC0 C0D6 0000 CB9D D8BC 0000"            /* Àý..üÀÀÖ..ËØ¼.. */
	$"00FE C00D ED0C 00DE 0000 000E 0000 0DE0"            /* .þÀ.í..Þ.......à */
	$"0CEF 00CD ED0C 0CCF 0000 000E 0000 0EAC"            /* .ï.Íí..Ï.......¬ */
	$"0CEF CF0D EDC0 C0DF 0000 000E 0000 0EED"            /* .ïÏ.íÀÀß.......í */
	$"0DDF CF0E DECF 0CCF 0000 000E 00DF 0FDD"            /* .ßÏ.ÞÏ.Ï.....ß.Ý */
	$"0DDE DFCE CEEA CCCF 0000 000A 00FF CECF"            /* .ÞßÎÎêÌÏ...Â.ÿÎÏ */
	$"0ECE EEDA CFDD E0DF 0000 000F 0CEE DE0E"            /* .ÎîÚÏÝàß.....îÞ. */
	$"CECE EDDF CEFC FCCF 0000 000E 0EDD ED0E"            /* ÎÎíßÎüüÏ.....Ýí. */
	$"CF0D FDEF 0FF0 DDDF 0000 000E 000E EC0D"            /* Ï.ýï.ðÝß......ì. */
	$"DF0D F0FF 0EF0 CECF 0000 000F 0000 DC0C"            /* ß.ðÿ.ðÎÏ......Ü. */
	$"FE0D E0FE 0FEC 00DF 0000 000E 0000 0000"            /* þ.àþ.ì.ß........ */
	$"FE00 C0EE 0EE0 CCCF 0000 000A 0000 0000"            /* þ.Àî.àÌÏ...Â.... */
	$"EE00 0CEE C0C0 0CDF 0000 000F 0000 0000"            /* î..îÀÀ.ß........ */
	$"C00C 00CC 0C0C C0CF 0000 000E 0000 0000"            /* À..Ì..ÀÏ........ */
	$"00C0 0C00 C0C0 CCDF 0000 000E FFAF FFFF"            /* .À..ÀÀÌß....ÿ¯ÿÿ */
	$"FFAF FFFF FFFF FFFF 6963 6C38 0000 0408"            /* ÿ¯ÿÿÿÿÿÿicl8.... */
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F5F5 00F5 F5F5 F5F5 F5AC FBFC 0000 0000"            /* õõ.õõõõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A082 FAAC FC00 00F5"            /* ..õû¬{|  ‚ú¬ü..õ */
	$"00F5 F5F5 F5F5 F5F6 F5FC F7FB AC00 0000"            /* .õõõõõõöõü÷û¬... */
	$"00F5 ACFE 50A1 A1A1 A7CB AD7A DFAC 0000"            /* .õ¬þP¡¡¡§Ë­zß¬.. */
	$"F500 F5F5 F5F5 F6F5 F6AC F5F7 FBAC 0000"            /* õ.õõõõöõö¬õ÷û¬.. */
	$"0081 FF7A 7D9B 9BA1 A1A7 E7AD FAEA FBF5"            /* .ÿz}››¡¡§ç­úêûõ */
	$"00F5 F5F5 F5F5 F5F5 F5AC F5F6 F7FB AC00"            /* .õõõõõõõõ¬õö÷û¬. */
	$"2BE9 AD57 C57D 9BA1 A1A7 E8E9 F9E8 E92B"            /* +é­WÅ}›¡¡§èéùèé+ */
	$"F5F5 F5F5 F5F5 F5F6 F5AC ACAC ACAC ACFD"            /* õõõõõõõöõ¬¬¬¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADEA FAA6 D1FB"            /* {éüu§¡¡¡§Ë­êú¦Ñû */
	$"00F5 00F5 F5F5 F6F5 F556 FAFA FAFA FAAC"            /* .õ.õõõöõõVúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBAD E8FF FAA7 E8A6"            /* ¦è¦ùË§¡§Ë­èÿú§è¦ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 5656 5656 56FD"            /* .õõõõõõöõõVVVVVý */
	$"A7A7 E8F9 A7E7 A7CB ADE8 EAFD 56E7 A7AD"            /* §§èù§ç§Ë­èêýVç§­ */
	$"F5F5 F5F5 F5F5 F5F5 F5F6 F6F5 F6F6 F8FD"            /* õõõõõõõõõööõööøý */
	$"A7A7 A7A6 56E9 E9D1 D1E0 EA56 A0A7 A7D0"            /* §§§¦VééÑÑàêV §§Ð */
	$"0000 F5F5 F5F5 F6F5 F6F5 F6F6 F5F6 56FD"            /* ..õõõõöõöõööõöVý */
	$"A0A7 E6A7 7B56 FCFD FEFC F87C A7A7 A7A7"            /*  §æ§{Vüýþüø|§§§§ */
	$"00F5 F5F5 F5F5 F5F5 ACF6 F5F6 F6F5 56FD"            /* .õõõõõõõ¬öõööõVý */
	$"57C5 A1A1 A7A0 572C 4F51 A1E6 A1A1 E7FA"            /* WÅ¡¡§ W,OQ¡æ¡¡çú */
	$"F5F5 F5F5 F5F5 F5F5 FFF5 F6F6 F6F6 56FD"            /* õõõõõõõõÿõööööVý */
	$"25A7 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E8F7"            /* %§¡¡¡¡æzQæ¡¡¡¡è÷ */
	$"F500 F82B F5F5 F5F5 FFF6 F5F6 F5F6 56FD"            /* õ.ø+õõõõÿöõöõöVý */
	$"0075 A1A1 A19B A757 51A1 A1A1 9BE7 FA00"            /* .u¡¡¡›§WQ¡¡¡›çú. */
	$"00F5 FDFB F5F5 F52B FFF6 F6F5 F6F6 56FD"            /* .õýûõõõ+ÿööõööVý */
	$"00F5 7CA1 9B7D C557 4AA1 9B77 CBFB 00F5"            /* .õ|¡›}ÅWJ¡›wËû.õ */
	$"F500 FDFB F5F5 F5F7 FEF7 F5F6 F6F6 F8FE"            /* õ.ýûõõõ÷þ÷õöööøþ */
	$"0000 F57B A19B A17A 519B A7E7 FA00 F500"            /* ..õ{¡›¡zQ›§çú.õ. */
	$"F5F5 FEFB F5F5 F5F8 FEF8 F6F5 F6F6 56D2"            /* õõþûõõõøþøöõööVÒ */
	$"0000 0000 2B7B A6F9 57A6 7BF7 0000 00F5"            /* ....+{¦ùW¦{÷...õ */
	$"00F5 E0AC F5F5 F556 ACF9 F5F6 F6F5 56FD"            /* .õà¬õõõV¬ùõööõVý */
	$"0000 0000 0000 00AC 0000 0000 0056 FBF5"            /* .......¬.....Vûõ */
	$"F52B ACFD F5F5 F5FA FCFA F5F6 F6F6 56FE"            /* õ+¬ýõõõúüúõöööVþ */
	$"0000 0000 0000 00AC 0000 0000 0081 E0F6"            /* .......¬.....àö */
	$"F5F7 FBFF 2BE0 0081 FB81 F6F5 F6F6 F8FE"            /* õ÷ûÿ+à.ûöõööøþ */
	$"0000 0000 0000 00AC 0000 0000 00FD FC56"            /* .......¬.....ýüV */
	$"00F9 81FE F8FF F5FB FAFB 56FD F5F6 56E0"            /* .ùþøÿõûúûVýõöVà */
	$"0000 0000 0000 00AC 0000 81FE 00FE F981"            /* .......¬..þ.þù */
	$"00FA F9AC FAFD F7AC 56AC FAFE F8F6 56FE"            /* .úù¬úý÷¬V¬úþøöVþ */
	$"0000 0000 0000 00AC 0000 FEFE 2BFD 2BFD"            /* .......¬..þþ+ý+ý */
	$"00FC F8FC FBFC F9AC 2BFE FCFA FBF5 56EA"            /* .üøüûüù¬+þüúûõVê */
	$"0000 0000 0000 00AC 00F7 ACAC FAFB 00FE"            /* .......¬.÷¬¬úû.þ */
	$"F6FC F7FB FC81 81EA F5FE FD2B FE2B F8E0"            /* öü÷ûüêõþý+þ+øà */
	$"0000 0000 0000 00AC 0081 56FB FBF9 00FC"            /* .......¬.Vûûù.ü */
	$"F8FE F5FA FEF8 ACE0 F5FE FEF5 81F9 56F4"            /* øþõúþø¬àõþþõùVô */
	$"0000 0000 0000 00FD 0000 00FA FDF8 00F9"            /* .......ý...úýø.ù */
	$"81EA F5F9 FEF6 F4FD F5AC FEF6 2BFB 56EA"            /* êõùþöôýõ¬þö+ûVê */
	$"0000 0000 0000 00AC 0000 0000 56F6 00F7"            /* .......¬....Vö.÷ */
	$"FDAC F556 FEF5 FDAC F5FD ACF5 F6F6 56FF"            /* ý¬õVþõý¬õý¬õööVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F500"            /* .......ý......õ. */
	$"FEFC F5F5 00F5 FCFD F5FC FCF6 F6F6 F8FF"            /* þüõõ.õüýõüüöööøÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F500"            /* .......ý......õ. */
	$"FD81 F5F5 F5F5 FBAC F52B F6F6 F6F5 56FF"            /* ýõõõõû¬õ+öööõVÿ */
	$"0000 0000 0000 00FD 0000 0000 F500 00F5"            /* .......ý....õ..õ */
	$"F600 F5F5 F5F5 2BF7 F6F5 F6F5 F6F6 56FF"            /* ö.õõõõ+÷öõöõööVÿ */
	$"0000 0000 0000 00FD 0000 0000 00F5 0000"            /* .......ý.....õ.. */
	$"F5F5 F5F5 F5F5 F5F5 F5F6 F6F6 F5F6 56FF"            /* õõõõõõõõõöööõöVÿ */
	$"0000 0000 0000 00FD FDFE FDFE FEFE FEFE"            /* .......ýýþýþþþþþ */
	$"FEFE FEFF FEFF FFFF FFFF FFFF FFFF FFFF"            /* þþþÿþÿÿÿÿÿÿÿÿÿÿÿ */
	$"696C 3332 0000 0ADF 84FF 125E 5D5C 5B5A"            /* il32..Âß„ÿ.^]\[Z */
	$"5958 5756 5453 5150 4E4D 4B49 4846 8AFF"            /* YXWVTSQPNMKIHFŠÿ */
	$"135D FFFF FEFD FBFA F8F6 F5F3 F1EF EEEC"            /* .]ÿÿþýûúøöõóñïîì */
	$"EAE8 E744 4286 FF17 B76E 6968 6E66 6DB6"            /* êèçDB†ÿ.·nihnfm¶ */
	$"FBFA F8F6 F5F3 F1EF EEEC EAE8 E742 5942"            /* ûúøöõóñïîìêèçBYB */
	$"83FF 1AEC 5432 775E 4741 526F 2E54 FAF8"            /* ƒÿ.ìT2w^GARo.Túø */
	$"F6F5 F3F1 EFEE ECEA E8E7 40B3 5933 81FF"            /* öõóñïîìêèç@³Y3ÿ */
	$"1CED 391A 8B3D 2F30 2615 1E7E 1639 F8F6"            /* .í9.‹=/0&..~.9øö */
	$"F5F3 F1EF EEEC EAE8 E73D DEB3 5933 80FF"            /* õóñïîìêèç=Þ³Y3€ÿ */
	$"5861 007F 4937 413A 2F24 102B 7600 5FF6"            /* Xa..I7A:/$.+v._ö */
	$"F5F3 F1EF EEEC EAE8 E73B EEDE B359 33FF"            /* õóñïîìêèç;îÞ³Y3ÿ */
	$"C503 2481 293C 3E39 2E23 1904 7623 02C1"            /* Å.$)<>9.#..v#.Á */
	$"F5F3 F1EF EEEC EAE8 E739 3736 3432 312F"            /* õóñïîìêèç976421/ */
	$"7000 3B78 2133 3531 281F 1600 6E3A 0068"            /* p.;x!351(...n:.h */
	$"F5F3 F1EF EEEC EAE8 E79C 8275 1A2D 3E0F"            /* õóñïîìêèçœ‚u.->. */
	$"357D 1828 2927 211A 1100 7934 1034 F5F3"            /* 5}.()'!...y4.4õó */
	$"F1EF EEEC EAE8 E7E5 829C 522B 311D 1A8C"            /* ñïîìêèçå‚œR+1..Œ */
	$"2617 1E1D 1913 031D 8C1B 1D24 F5F3 F1EF"            /* &.......Œ..$õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C29 3523 1953"            /* îìêèçåãâàßœ)5#.S */
	$"880A 090E 0B00 078C 501A 2326 F5F3 F1EF"            /* ˆÂÆ....ŒP.#&õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C27 4823 281C"            /* îìêèçåãâàßœ'H#(. */
	$"6890 4517 1548 9466 1D29 2236 F5F3 82EE"            /* hE..H”f.)"6õó‚î */
	$"1CE8 3AE6 E3E2 E0DF 9C25 7C20 2E2E 2248"            /* .è:æãâàßœ%| .."H */
	$"7AB4 B57C 4623 2D2F 1769 F5F3 F1EF 80EE"            /* z´µ|F#-/.iõóñï€î */
	$"1CE8 01E5 E3E2 E0DF 9C23 C928 3233 332F"            /* .è.åãâàßœ#É(233/ */
	$"1388 8D1C 2E33 3431 12C0 F5F3 ACCB 80EE"            /* .ˆ..341.Àõó¬Ë€î */
	$"1CE8 01E5 E3E2 E0DF 9C21 FF77 2639 3839"            /* .è.åãâàßœ!ÿw&989 */
	$"2488 8F2C 3839 3A15 62F6 F5F3 2C59 80EE"            /* $ˆ,89:.böõó,Y€î */
	$"1CD4 07D9 E3E2 E0DF 9C1F FFEE 5B28 3C3F"            /* .Ô.Ùãâàßœ.ÿî[(<? */
	$"2889 9332 3F3B 1843 F8F6 F5F3 2355 80EE"            /* (‰“2?;.Cøöõó#U€î */
	$"1CBE 10BE E3E2 E0DF 9C1C FFFF ED72 2A2F"            /* .¾.¾ãâàßœ.ÿÿír*. */
	$"2589 942D 2619 60FA F8F6 F5EE 154E 80EE"            /* %‰”-&.`úøöõî.N€î */
	$"08A9 1CA9 E3E2 E0DF 9C18 81FF 0FC3 6F2F"            /* .©.©ãâàßœ.ÿ.Ão/ */
	$"878C 346B BDFB FAF8 F6F5 E90A 3D80 EE08"            /* ‡Œ4k½ûúøöõéÂ=€î. */
	$"932D 93E3 E2E0 DF9C 1884 FF0C 3DFF FFFE"            /* “-“ãâàßœ.„ÿ.=ÿÿþ */
	$"FDFB AA5A EEF5 D02B 2780 EE08 7D42 7DE3"            /* ýûªZîõÐ+'€î.}B}ã */
	$"E2E0 DF9C 1684 FF18 3BFF FFFE FDFB 5F11"            /* âàßœ.„ÿ.;ÿÿþýû_. */
	$"DCF5 B14A 0DC0 18EE 685B 68E3 E2E0 DF9C"            /* Üõ±J.À.îh[hãâàßœ */
	$"1484 FF18 39FF FFFE FDFB 3443 9CF5 8F68"            /* .„ÿ.9ÿÿþýû4Cœõh */
	$"15A3 06E6 5278 52A0 2DE0 DF9C 1284 FF18"            /* .£.æRxR -àßœ.„ÿ. */
	$"37FF FF66 22FB 0D87 68F5 7586 2B7D 1DBA"            /* 7ÿÿf"û.‡hõu†+}.º */
	$"3D9C 386C 1AB1 DF9C 1084 FF18 35FF FF1A"            /* =œ8l.±ßœ.„ÿ.5ÿÿ. */
	$"0BC5 2BC7 30F5 57A4 455B 3C8F 2BC1 234A"            /* .Å+Ç0õW¤E[<+Á#J */
	$"6E57 DF9C 0E84 FF18 33FF AD34 3484 57F8"            /* nWßœ.„ÿ.3ÿ­44„Wø */
	$"12DC 38BE 5B3D 6864 15E7 0D2B C412 D99C"            /* .Ü8¾[=hd.ç.+Ä.Ùœ */
	$"0C84 FF18 31FF 718A 574F 82F8 45A9 1EE1"            /* .„ÿ.1ÿqŠWO‚øE©.á */
	$"7123 9C34 0DE7 150D E26C 829C 0A84 FF18"            /* q#œ4.ç..âl‚œÂ„ÿ. */
	$"2FFF FFFE 792C ADF8 866F 0DEE 8611 D80C"            /* /ÿÿþy,­ø†o.î†.Ø. */
	$"23E7 281A E2C7 579C 0884 FF18 2DFF FFFE"            /* #ç(.âÇWœ.„ÿ.-ÿÿþ */
	$"FD97 DCF8 BE2A 30EE 9C18 EC23 30E7 332F"            /* ý—Üø¾*0îœ.ì#0ç3/ */
	$"E2E0 DF9C 0684 FF18 2BFF FFFE FDFB FAF8"            /* âàßœ.„ÿ.+ÿÿþýûúø */
	$"F60A 47EE EFEE EC3F 34E7 3D42 E2E0 DF9C"            /* öÂGîïîì?4ç=Bâàßœ */
	$"0584 FF0A 29FF FFFE FDFB FAF8 F631 6080"            /* .„ÿÂ)ÿÿþýûúøö1`€ */
	$"EE0A EC53 31E7 D4D0 E2E0 DF9C 0384 FF0A"            /* îÂìS1çÔÐâàßœ.„ÿÂ */
	$"26FF FFFE FDFB FAF8 F6E1 F380 EE0A ECD0"            /* &ÿÿþýûúøöáó€îÂìÐ */
	$"B6E7 E5E3 E2E0 DF9C 0284 FF18 24FF FFFE"            /* ¶çåãâàßœ.„ÿ.$ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 0184 FF18 2220 1F1D 1B1A 1816"            /* âàßœ.„ÿ." ...... */
	$"1513 1110 0E0D 0B0A 0807 0605 0403 0201"            /* .......Â........ */
	$"0084 FF12 5E5D 5C5B 5A59 5857 5654 5351"            /* .„ÿ.^]\[ZYXWVTSQ */
	$"504E 4D4B 4948 468A FF13 5DFF FFFE FDFB"            /* PNMKIHFŠÿ.]ÿÿþýû */
	$"FAF8 F6F5 F3F1 EFEE ECEA E8E7 4442 86FF"            /* úøöõóñïîìêèçDB†ÿ */
	$"17B6 6A67 6C71 656C B7FB FAF8 F6F5 F3F1"            /* .¶jglqel·ûúøöõóñ */
	$"EFEE ECEA E8E7 4259 4283 FF1A ED55 2F94"            /* ïîìêèçBYBƒÿ.íU/” */
	$"A693 8079 782D 55FA F8F6 F5F3 F1EF EEEC"            /* ¦“€yx-Uúøöõóñïîì */
	$"EAE8 E740 B359 3381 FF1C EE3E 19B6 ADA5"            /* êèç@³Y3ÿ.î>.¶­¥ */
	$"987C 5848 861B 3DF8 F6F5 F3F1 EFEE ECEA"            /* ˜|XH†.=øöõóñïîìê */
	$"E8E7 3DDE B359 3380 FF58 6F00 90AB B2BE"            /* èç=Þ³Y3€ÿXo.«²¾ */
	$"AC8A 6942 3E7E 0667 F6F5 F3F1 EFEE ECEA"            /* ¬ŠiB>~.göõóñïîìê */
	$"E8E7 3BEE DEB3 5933 FFCC 2233 A891 B1B8"            /* èç;îÞ³Y3ÿÌ"3¨‘±¸ */
	$"A787 674A 1C7B 4219 C4F5 F3F1 EFEE ECEA"            /* §‡gJ.{B.Äõóñïîìê */
	$"E8E7 3937 3634 3231 2F88 2A4F 9F7B 979C"            /* èç976421/ˆ*OŸ{—œ */
	$"8F76 5D42 1273 5F28 75F5 F3F1 EFEE ECEA"            /* v]B.s_(uõóñïîìê */
	$"E8E7 9C82 751A 2D67 4453 985F 7679 7161"            /* èçœ‚u.-gDS˜_vyqa */
	$"4C33 087E 6445 4CF5 F3F1 EFEE ECEA E8E7"            /* L3.~dELõóñïîìêèç */
	$"E582 9C52 2B66 5A4E 9C52 535B 5549 3616"            /* å‚œR+fZNœRS[UI6. */
	$"1BA0 5A5A 44F5 F3F1 EFEE ECEA E8E7 E5E3"            /* . ZZDõóñïîìêèçåã */
	$"E2E0 DF9C 2971 685F 7C92 2A2E 3227 0F03"            /* âàßœ)qh_|’*.2'.. */
	$"8D8B 6268 46F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ‹bhFõóñïîìêèçåã */
	$"E2E0 DF9C 2787 7478 6B97 9745 2119 419C"            /* âàßœ'‡txk——E!.Aœ */
	$"A170 786F 53F5 F382 EE1C E83A E6E3 E2E0"            /* ¡pxoSõó‚î.è:æãâà */
	$"DF9C 25AD 7E86 877E 96A6 B8C8 B499 8086"            /* ßœ%­~†‡~–¦¸È´™€† */
	$"8B64 77F5 F3F1 EF80 EE1C E801 E5E3 E2E0"            /* ‹dwõóñï€î.è.åãâà */
	$"DF9C 23DD 8B98 9798 9873 9BC8 8B94 9799"            /* ßœ#Ý‹˜—˜˜s›È‹”—™ */
	$"9748 C0F5 F3AC CB80 EE1C E801 E5E3 E2E0"            /* —HÀõó¬Ë€î.è.åãâà */
	$"DF9C 21FE AF99 A9A6 A987 9CCE A1A6 A8AC"            /* ßœ!þ¯™©¦©‡œÎ¡¦¨¬ */
	$"666F F6F5 F32C 5980 EE1C D407 D9E3 E2E0"            /* foöõó,Y€î.Ô.Ùãâà */
	$"DF9C 1FFF F2A1 A1B7 BA94 9FD6 B1B9 B470"            /* ßœ.ÿò¡¡·º”ŸÖ±¹´p */
	$"5AF8 F6F5 F323 5580 EE1C BE10 BEE3 E2E0"            /* Zøöõó#U€î.¾.¾ãâà */
	$"DF9C 1CFF FFEF A691 AC97 A1DB AF91 5B71"            /* ßœ.ÿÿï¦‘¬—¡Û¯‘[q */
	$"FAF8 F6F5 EE15 4E80 EE08 A91C A9E3 E2E0"            /* úøöõî.N€î.©.©ãâà */
	$"DF9C 1880 FF10 FECF 9865 93AF 6680 BEFB"            /* ßœ.€ÿ.þÏ˜e“¯f€¾û */
	$"FAF8 F6F5 E90A 3D80 EE08 932D 93E3 E2E0"            /* úøöõéÂ=€î.“-“ãâà */
	$"DF9C 1884 FF0C 3DFF FFFE FDFB AA5A EEF5"            /* ßœ.„ÿ.=ÿÿþýûªZîõ */
	$"D02B 2780 EE08 7D42 7DE3 E2E0 DF9C 1684"            /* Ð+'€î.}B}ãâàßœ.„ */
	$"FF18 3BFF FFFE FDFB 5F11 DCF5 B14A 0DC0"            /* ÿ.;ÿÿþýû_.Üõ±J.À */
	$"18EE 685B 68E3 E2E0 DF9C 1484 FF18 39FF"            /* .îh[hãâàßœ.„ÿ.9ÿ */
	$"FFFE FDFB 3443 9CF5 8F68 15A3 06E6 5278"            /* ÿþýû4Cœõh.£.æRx */
	$"52A0 2DE0 DF9C 1284 FF18 37FF FF66 22FB"            /* R -àßœ.„ÿ.7ÿÿf"û */
	$"0D87 68F5 7586 2B7D 1DBA 3D9C 386C 1AB1"            /* .‡hõu†+}.º=œ8l.± */
	$"DF9C 1084 FF18 35FF FF1A 0BC5 2BC7 30F5"            /* ßœ.„ÿ.5ÿÿ..Å+Ç0õ */
	$"57A4 455B 3C8F 2BC1 234A 6E57 DF9C 0E84"            /* W¤E[<+Á#JnWßœ.„ */
	$"FF18 33FF AD34 3484 57F8 12DC 38BE 5B3D"            /* ÿ.3ÿ­44„Wø.Ü8¾[= */
	$"6864 15E7 0D2B C412 D99C 0C84 FF18 31FF"            /* hd.ç.+Ä.Ùœ.„ÿ.1ÿ */
	$"718A 574F 82F8 45A9 1EE1 7123 9C34 0DE7"            /* qŠWO‚øE©.áq#œ4.ç */
	$"150D E26C 829C 0A84 FF18 2FFF FFFE 792C"            /* ..âl‚œÂ„ÿ./ÿÿþy, */
	$"ADF8 866F 0DEE 8611 D80C 23E7 281A E2C7"            /* ­ø†o.î†.Ø.#ç(.âÇ */
	$"579C 0884 FF18 2DFF FFFE FD97 DCF8 BE2A"            /* Wœ.„ÿ.-ÿÿþý—Üø¾* */
	$"30EE 9C18 EC23 30E7 332F E2E0 DF9C 0684"            /* 0îœ.ì#0ç3/âàßœ.„ */
	$"FF18 2BFF FFFE FDFB FAF8 F60A 47EE EFEE"            /* ÿ.+ÿÿþýûúøöÂGîïî */
	$"EC3F 34E7 3D42 E2E0 DF9C 0584 FF0A 29FF"            /* ì?4ç=Bâàßœ.„ÿÂ)ÿ */
	$"FFFE FDFB FAF8 F631 6080 EE0A EC53 31E7"            /* ÿþýûúøö1`€îÂìS1ç */
	$"D4D0 E2E0 DF9C 0384 FF0A 26FF FFFE FDFB"            /* ÔÐâàßœ.„ÿÂ&ÿÿþýû */
	$"FAF8 F6E1 F380 EE0A ECD0 B6E7 E5E3 E2E0"            /* úøöáó€îÂìÐ¶çåãâà */
	$"DF9C 0284 FF18 24FF FFFE FDFB FAF8 F6F5"            /* ßœ.„ÿ.$ÿÿþýûúøöõ */
	$"F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C 0184"            /* óñïîìêèçåãâàßœ.„ */
	$"FF18 2220 1F1D 1B1A 1816 1513 1110 0E0D"            /* ÿ." ............ */
	$"0B0A 0807 0605 0403 0201 0084 FF12 5E5D"            /* .Â.........„ÿ.^] */
	$"5C5B 5A59 5857 5654 5351 504E 4D4B 4948"            /* \[ZYXWVTSQPNMKIH */
	$"468A FF13 5DFF FFFE FDFB FAF8 F6F5 F3F1"            /* FŠÿ.]ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 4442 86FF 17B7 7069 666D"            /* ïîìêèçDB†ÿ.·pifm */
	$"666E B6FB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* fn¶ûúøöõóñïîìêèç */
	$"4259 4283 FF1A EC54 3469 4423 2447 6B2E"            /* BYBƒÿ.ìT4iD#$Gk. */
	$"54FA F8F6 F5F3 F1EF EEEC EAE8 E740 B359"            /* Túøöõóñïîìêèç@³Y */
	$"3381 FF04 EC37 1C78 0F81 0013 147B 1539"            /* 3ÿ.ì7.x....{.9 */
	$"F8F6 F5F3 F1EF EEEC EAE8 E73D DEB3 5933"            /* øöõóñïîìêèç=Þ³Y3 */
	$"80FF 035C 0078 1B83 0018 2472 005C F6F5"            /* €ÿ.\.x.ƒ..$r.\öõ */
	$"F3F1 EFEE ECEA E8E7 3BEE DEB3 5933 FFC1"            /* óñïîìêèç;îÞ³Y3ÿÁ */
	$"001E 6E85 0017 7316 00C0 F5F3 F1EF EEEC"            /* ..n…..s..Àõóñïîì */
	$"EAE8 E739 3736 3432 312F 6700 3463 8500"            /* êèç976421/g.4c…. */
	$"0D6D 2800 66F5 F3F1 EFEE ECEA E8E7 9C82"            /* .m(.fõóñïîìêèçœ‚ */
	$"7504 2D2D 0027 7185 000D 771D 002C F5F3"            /* u.--.'q…..w..,õó */
	$"F1EF EEEC EAE8 E7E5 829C 052B 1600 0787"            /* ñïîìêèçå‚œ.+...‡ */
	$"1583 001A 1D84 0400 15F5 F3F1 EFEE ECEA"            /* .ƒ...„...õóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 2916 0000 4386 0581"            /* èçåãâàßœ)...C†. */
	$"0016 0A8A 3900 0015 F5F3 F1EF EEEC EAE8"            /* ..ÂŠ9...õóñïîìêè */
	$"E7E5 E3E2 E0DF 9C27 2D80 0007 578D 4714"            /* çåãâàßœ'-€..WG. */
	$"154B 8F4E 8000 022C F5F3 82EE 09E8 3AE6"            /* .KN€..,õó‚îÆè:æ */
	$"E3E2 E0DF 9C25 6781 0005 2768 B1AC 6622"            /* ãâàßœ%g..'h±¬f" */
	$"8100 0466 F5F3 F1EF 80EE 09E8 01E5 E3E2"            /* ..fõóñï€îÆè.åãâ */
	$"E0DF 9C23 C183 0001 7E72 8300 04C0 F5F3"            /* àßœ#Áƒ..~rƒ..Àõó */
	$"ACCB 80EE 0AE8 01E5 E3E2 E0DF 9C21 FF5D"            /* ¬Ë€îÂè.åãâàßœ!ÿ] */
	$"8200 017E 7082 0005 5DF6 F5F3 2C59 80EE"            /* ‚..~p‚..]öõó,Y€î */
	$"0BD4 07D9 E3E2 E0DF 9C1F FFEC 3D81 0001"            /* .Ô.Ùãâàßœ.ÿì=.. */
	$"7E71 8100 063D F8F6 F5F3 2355 80EE 0CBE"            /* ~q..=øöõó#U€î.¾ */
	$"10BE E3E2 E0DF 9C1C FFFF EB5F 8000 017E"            /* .¾ãâàßœ.ÿÿë_€..~ */
	$"7180 0007 5FFA F8F6 F5EE 154E 80EE 08A9"            /* q€.._úøöõî.N€î.© */
	$"1CA9 E3E2 E0DF 9C18 81FF 0FBF 6021 807B"            /* .©ãâàßœ.ÿ.¿`!€{ */
	$"2766 BEFB FAF8 F6F5 E90A 3D80 EE08 932D"            /* 'f¾ûúøöõéÂ=€î.“- */
	$"93E3 E2E0 DF9C 1884 FF0C 3DFF FFFE FDFB"            /* “ãâàßœ.„ÿ.=ÿÿþýû */
	$"AA5A EEF5 D02B 2780 EE08 7D42 7DE3 E2E0"            /* ªZîõÐ+'€î.}B}ãâà */
	$"DF9C 1684 FF18 3BFF FFFE FDFB 5F11 DCF5"            /* ßœ.„ÿ.;ÿÿþýû_.Üõ */
	$"B14A 0DC0 18EE 685B 68E3 E2E0 DF9C 1484"            /* ±J.À.îh[hãâàßœ.„ */
	$"FF18 39FF FFFE FDFB 3443 9CF5 8F68 15A3"            /* ÿ.9ÿÿþýû4Cœõh.£ */
	$"06E6 5278 52A0 2DE0 DF9C 1284 FF18 37FF"            /* .æRxR -àßœ.„ÿ.7ÿ */
	$"FF66 22FB 0D87 68F5 7586 2B7D 1DBA 3D9C"            /* ÿf"û.‡hõu†+}.º=œ */
	$"386C 1AB1 DF9C 1084 FF18 35FF FF1A 0BC5"            /* 8l.±ßœ.„ÿ.5ÿÿ..Å */
	$"2BC7 30F5 57A4 455B 3C8F 2BC1 234A 6E57"            /* +Ç0õW¤E[<+Á#JnW */
	$"DF9C 0E84 FF18 33FF AD34 3484 57F8 12DC"            /* ßœ.„ÿ.3ÿ­44„Wø.Ü */
	$"38BE 5B3D 6864 15E7 0D2B C412 D99C 0C84"            /* 8¾[=hd.ç.+Ä.Ùœ.„ */
	$"FF18 31FF 718A 574F 82F8 45A9 1EE1 7123"            /* ÿ.1ÿqŠWO‚øE©.áq# */
	$"9C34 0DE7 150D E26C 829C 0A84 FF18 2FFF"            /* œ4.ç..âl‚œÂ„ÿ./ÿ */
	$"FFFE 792C ADF8 866F 0DEE 8611 D80C 23E7"            /* ÿþy,­ø†o.î†.Ø.#ç */
	$"281A E2C7 579C 0884 FF18 2DFF FFFE FD97"            /* (.âÇWœ.„ÿ.-ÿÿþý— */
	$"DCF8 BE2A 30EE 9C18 EC23 30E7 332F E2E0"            /* Üø¾*0îœ.ì#0ç3/âà */
	$"DF9C 0684 FF18 2BFF FFFE FDFB FAF8 F60A"            /* ßœ.„ÿ.+ÿÿþýûúøöÂ */
	$"47EE EFEE EC3F 34E7 3D42 E2E0 DF9C 0584"            /* Gîïîì?4ç=Bâàßœ.„ */
	$"FF0A 29FF FFFE FDFB FAF8 F631 6080 EE0A"            /* ÿÂ)ÿÿþýûúøö1`€îÂ */
	$"EC53 31E7 D4D0 E2E0 DF9C 0384 FF0A 26FF"            /* ìS1çÔÐâàßœ.„ÿÂ&ÿ */
	$"FFFE FDFB FAF8 F6E1 F380 EE0A ECD0 B6E7"            /* ÿþýûúøöáó€îÂìÐ¶ç */
	$"E5E3 E2E0 DF9C 0284 FF18 24FF FFFE FDFB"            /* åãâàßœ.„ÿ.$ÿÿþýû */
	$"FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0"            /* úøöõóñïîìêèçåãâà */
	$"DF9C 0184 FF18 2220 1F1D 1B1A 1816 1513"            /* ßœ.„ÿ." ........ */
	$"1110 0E0D 0B0A 0807 0605 0403 0201 006C"            /* .....Â.........l */
	$"386D 6B00 0004 0800 0000 0000 0000 FFFF"            /* 8mk...........ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 0000 0000 0000 FFFF"            /* ÿ.............ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 0000 0000 0000 0000 0000 FFFF FFFF"            /* ÿÿ..........ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF00 0000 0000 0000 FFFF FFFF FFFF"            /* ÿÿÿ.......ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿ.....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿ...ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿ..ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ.ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ.ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ..ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 FFFF FFFF"            /* ÿÿÿÿÿÿÿ.....ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿ.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿ.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿ.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿ.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿ.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿ.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿ.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿ.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿ.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿ.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿ.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿ.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿ.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿ.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF69 6373 2300 0000 4809"            /* ÿÿÿÿÿÿÿics#...HÆ */
	$"2814 046E 0ADB 05BD 02DB 0966 012A 6814"            /* (..nÂÛ.½.ÛÆf.*h. */
	$"4911 2902 D415 2D0A D500 9910 000A B71F"            /* I.).Ô.-ÂÕ.™..Â·. */
	$"FC7F FEFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ü.þÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF9F FF9F FF9F FF9F FF9F FF9F FF9F FF69"            /* ÿŸÿŸÿŸÿŸÿŸÿŸÿŸÿi */
	$"6373 3400 0000 8800 0CDC CDCD CDDC 000C"            /* cs4...ˆ..ÜÍÍÍÜ.. */
	$"DEDD C000 00CE C0CE 888A EC00 C0DC ECB9"            /* ÞÝÀ..ÎÀÎˆŠì.ÀÜì¹ */
	$"B899 EE00 00CE DE9B 99AF E8C0 0C0C CB99"            /* ¸™î..ÎÞ›™¯èÀ..Ë™ */
	$"DAEE 8900 0CC0 0EB8 8DD8 9B0C 0CD0 CDC8"            /* Úî‰..À.¸Ø›..ÐÍÈ */
	$"8B88 9C0E 0CDC 0D0C 8DBB 00CE 0CE0 CD00"            /* ‹ˆœ..Ü..».Î.àÍ. */
	$"0D00 CDCE CDDC 0D00 0CCC DDCE EDDE CD00"            /* ..ÍÎÍÜ...ÌÝÎíÞÍ. */
	$"0CCE DDDD DEDE DD00 0DCD DCED EDED CE00"            /* .ÎÝÝÞÞÝ..ÍÜíííÎ. */
	$"0C00 0CEC CEDD CD00 0D00 00C0 0CC0 0E00"            /* ...ìÎÝÍ....À.À.. */
	$"0DDD DDDD DDDD DE69 6373 3800 0001 0800"            /* .ÝÝÝÝÝÞics8..... */
	$"0000 F856 F8F7 F8F8 5656 F881 F700 0000"            /* ..øVø÷øøVVø÷... */
	$"2BFA 7C81 F92B 0000 0000 0056 FB2B 002B"            /* +ú|ù+.....Vû+.+ */
	$"ACA0 9BA7 FDA6 2B00 F5F5 0056 F8FB 2BFB"            /* ¬ ›§ý¦+.õõ.Vøû+û */
	$"A69B A1A7 E7AC FC00 F5F5 F5F8 FBFB FCE7"            /* ¦›¡§ç¬ü.õõõøûûüç */
	$"82A6 A7E8 FD82 CB06 F5F5 F5F5 2B2B 83A7"            /* ‚¦§èý‚Ë.õõõõ++ƒ§ */
	$"A082 FDFD FBA1 A6F6 00F5 F6F7 F5F5 827C"            /*  ‚ýýû¡¦ö.õö÷õõ‚| */
	$"E5A0 7B51 A1E6 7C00 2B00 2BFA F5F6 8124"            /* å {Q¡æ|.+.+úõö$ */
	$"A1A1 7C76 A1A6 2BF5 FBF6 2BFA F5F6 5500"            /* ¡¡|v¡¦+õûö+úõöU. */
	$"F676 7B7B A0F6 00F6 AC00 F781 F5F6 7E00"            /* öv{{ ö.ö¬.÷õö~. */
	$"00F5 F8F6 00F7 56F6 FCF8 56FB F6F5 8100"            /* .õøö.÷VöüøVûöõ. */
	$"0000 F8F5 F8FA 81F7 FBFB F981 FB2B 8100"            /* ..øõøú÷ûûùû+. */
	$"0000 F8F7 ACFA F956 FAFC FB81 FCF9 FA00"            /* ..ø÷¬úùVúüûüùú. */
	$"0000 F82B F9FA F7FB F9FB FB81 81F8 FB00"            /* ..ø+ùú÷ûùûûøû. */
	$"0000 56F5 00F6 F6AC 2BF7 FCFA F9F5 8100"            /* ..Võ.öö¬+÷üúùõ. */
	$"0000 F8F5 0000 00F8 0000 F8F5 F5F5 8100"            /* ..øõ...ø..øõõõ. */
	$"0000 56FA F9F9 F9F9 F9FA FAFA FAFA AC69"            /* ..Vúùùùùùúúúúú¬i */
	$"7333 3200 0003 0B0D FFFF FEB0 98AC B2A6"            /* s32.....ÿÿþ°˜¬²¦ */
	$"A4A1 9E9E 6DBD 80FF 7EC8 795F 6780 D5FF"            /* ¤¡žžm½€ÿ~Èy_g€Õÿ */
	$"FFFB F8FF 9E5A C0FF C73A 4A35 1F2D 38C6"            /* ÿûøÿžZÀÿÇ:J5.-8Æ */
	$"FCF0 EDF3 9BAB 5CC8 4B40 3A38 2616 374E"            /* üðíó›«\ÈK@:8&.7N */
	$"F8F2 EDF2 AA5A 584F 1F4E 2C1B 111A 4A27"            /* øòíòªZXO.N,...J' */
	$"E8F4 EDEB EAC1 D44F 2A32 5A26 245A 302E"            /* èôíëêÁÔO*2Z&$Z0. */
	$"E9F8 EFDE C0EE EA4F 631E 3676 7938 1C5A"            /* éøïÞÀîêOc.6vy8.Z */
	$"F3D4 F1CD 7EF1 E24F D43B 225B 6220 30CF"            /* óÔñÍ~ñâOÔ;"[b 0Ï */
	$"EA58 EBC5 76ED E14F FFD2 6063 6C58 CEFF"            /* êXëÅvíáOÿÒ`clXÎÿ */
	$"DD40 F4BA 67E8 E24F 80FF 0CA4 DEFF BB94"            /* Ý@ôºgèâO€ÿ.¤Þÿ»” */
	$"DB3D ADA1 63DB E44F 80FF 0CAA E5B4 806C"            /* Û=­¡cÛäO€ÿ.ªå´€l */
	$"BD52 5080 655D C64F 80FF 0CAC BD2F 7786"            /* ½RP€e]ÆO€ÿ.¬½/w† */
	$"8F71 4E61 6247 9571 80FF 0CA9 C68A 78B5"            /* qNabG•q€ÿ.©ÆŠxµ */
	$"5A8B 594A 646D A15C 80FF 0CA2 EAFC DEDA"            /* Z‹YJdm¡\€ÿ.¢êüÞÚ */
	$"3AC5 AD50 798B EA62 80FF 0CA1 F1FF FFFE"            /* :Å­Py‹êb€ÿ.¡ñÿÿþ */
	$"B2FF FFA2 E2EC F469 80FF 0C97 798E 8A88"            /* ²ÿÿ¢âìôi€ÿ.—yŽŠˆ */
	$"8D80 7D7F 7977 7139 0DFF FFFB AA94 ACB2"            /* €}.ywq9.ÿÿûª”¬² */
	$"A6A4 A19E 9E6D BD80 FF7F C581 8783 83D4"            /* ¦¤¡žžm½€ÿ.Å‡ƒƒÔ */
	$"FFFF FBF8 FF9E 5AC0 FFC9 41A5 B172 493C"            /* ÿÿûøÿžZÀÿÉA¥±rI< */
	$"C7FC F0ED F39B AB5C C860 61A3 A773 3649"            /* Çüðíó›«\È`a£§s6I */
	$"63F7 F2ED F2AA 5A58 4F53 7065 6142 236C"            /* c÷òíòªZXOSpeaB#l */
	$"53E8 F3ED EBEA C1E5 4F72 7474 392B 6D7C"            /* SèóíëêÁåOrtt9+m| */
	$"63E7 F7EF DEC0 EEEA 4FA5 8893 99AD 9983"            /* cç÷ïÞÀîêO¥ˆ“™­™ƒ */
	$"80EF D4F1 CD7E F1E2 4FE5 A1A6 9FBE A27A"            /* €ïÔñÍ~ñâOå¡¦Ÿ¾¢z */
	$"D2E8 58EB C576 EDE1 4FFF E3AB 97AC 90D1"            /* ÒèXëÅvíáOÿã«—¬Ñ */
	$"FFDD 40F4 BA67 E8E2 4FFF 0EFF FBA4 DCFF"            /* ÿÝ@ôºgèâOÿ.ÿû¤Üÿ */
	$"BA95 DB3D ADA1 63DB E44F 80FF 0CA9 E5B3"            /* º•Û=­¡cÛäO€ÿ.©å³ */
	$"806C BD52 5080 655D C64F 80FF 0CAC BD2F"            /* €l½RP€e]ÆO€ÿ.¬½/ */
	$"7786 8F71 4E61 6247 9571 80FF 0CA9 C68A"            /* w†qNabG•q€ÿ.©ÆŠ */
	$"78B5 5A8B 594A 646D A15C 80FF 0CA2 EAFC"            /* xµZ‹YJdm¡\€ÿ.¢êü */
	$"DEDA 3AC5 AD50 798B EA62 80FF 0CA1 F1FF"            /* ÞÚ:Å­Py‹êb€ÿ.¡ñÿ */
	$"FFFE B2FF FFA2 E2EC F469 80FF 0C97 798E"            /* ÿþ²ÿÿ¢âìôi€ÿ.—yŽ */
	$"8A88 8D80 7D7F 7977 7139 80FF 0AB2 99AD"            /* Šˆ€}.ywq9€ÿÂ²™­ */
	$"B2A6 A4A1 9E9E 6DBD 80FF 7EC9 764F 5D80"            /* ²¦¤¡žžm½€ÿ~ÉvO]€ */
	$"D5FF FFFB F8FF 9E5A C0FF C737 2200 0024"            /* ÕÿÿûøÿžZÀÿÇ7"..$ */
	$"38C6 FCF0 EDF3 9BAB 5CC7 4834 0800 000A"            /* 8Æüðíó›«\ÇH4...Â */
	$"314B F8F2 EDF2 AA5A 584B 0942 1500 0017"            /* 1KøòíòªZXKÆB.... */
	$"3C15 E8F4 EDEB EAC1 CC4B 0918 5324 2553"            /* <.èôíëêÁÌKÆ.S$%S */
	$"1416 EAF8 EFDE C0EE EA4B 4900 1269 6710"            /* ..êøïÞÀîêKI..ig. */
	$"004E F5D4 F1CD 7EF1 E24B CC15 0040 3A00"            /* .NõÔñÍ~ñâKÌ..@:. */
	$"17CF EA58 EBC5 76ED E14B FFCB 4C50 534D"            /* .ÏêXëÅvíáKÿËLPSM */
	$"CDFF DC40 F4BA 67E8 E24B 80FF 0CA5 DFFF"            /* ÍÿÜ@ôºgèâK€ÿ.¥ßÿ */
	$"BC94 DB3D ADA1 63DB E44B 80FF 0CAA E6B4"            /* ¼”Û=­¡cÛäK€ÿ.ªæ´ */
	$"806C BD52 5080 655D C64B 80FF 0CAC BD2F"            /* €l½RP€e]ÆK€ÿ.¬½/ */
	$"7786 8F71 4E61 6247 9571 80FF 0CA9 C68A"            /* w†qNabG•q€ÿ.©ÆŠ */
	$"78B5 5A8B 594A 646D A15C 80FF 0CA2 EAFC"            /* xµZ‹YJdm¡\€ÿ.¢êü */
	$"DEDA 3AC5 AD50 798B EA62 80FF 0CA1 F1FF"            /* ÞÚ:Å­Py‹êb€ÿ.¡ñÿ */
	$"FFFE B2FF FFA2 E2EC F469 80FF 0C97 798E"            /* ÿþ²ÿÿ¢âìôi€ÿ.—yŽ */
	$"8A88 8D80 7D7F 7977 7139 7338 6D6B 0000"            /* Šˆ€}.ywq9s8mk.. */
	$"0108 0000 0087 FFFF FFFF FFFF FFFF F74D"            /* .....‡ÿÿÿÿÿÿÿÿ÷M */
	$"0000 0045 B7F7 FFFF FFFF FFFF FFFF FFF9"            /* ...E·÷ÿÿÿÿÿÿÿÿÿù */
	$"4C00 38FA FFFF FFFF FFFF FFFF FFFF FFFF"            /* L.8úÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"F94E CBFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ùNËÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF D3FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÓÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 54F7 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿT÷ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 0943 B7F7 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÆC·÷ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 1A00 0087 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿ...‡ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 1900 007D FFFF FFFF FFFF FFFF FFFF"            /* ÿÿ...}ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 1A00 007F FFFF FFFF FFFF FFFF FFFF"            /* ÿÿ....ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 1900 007F FFFF FFFF FFFF FFFF FFFF"            /* ÿÿ....ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 1B00 007F FFFF FFFF FFFF FFFF FFFF"            /* ÿÿ....ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 1C00 007F FFFF FFFF FFFF FFFF FFFF"            /* ÿÿ....ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 1500 007F FFFF FFFF FFFF FFFF FFFF"            /* ÿÿ....ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF"                                               /* ÿÿ */
};

data 'icns' (131, "Shapes Icon") {
	$"6963 6E73 0000 1611 4943 4E23 0000 0108"            /* icns....ICN#.... */
	$"016D BFC0 0000 0020 07A0 8070 1D58 4048"            /* .m¿À... . €p.X@H */
	$"32EC 624C 2D76 7042 6ABE A47F ED7B 3055"            /* 2ìbL-vpBj¾¤.í{0U */
	$"77D6 13A9 EB77 0D03 B7ED 0F81 6ADB 1BA9"            /* wÖ.©ëw..·í.jÛ.© */
	$"BC56 7CE3 65AA 3F71 3AB4 8BAB 126C 07E1"            /* ¼V|ãeª?q:´‹«.l.á */
	$"0D50 03D1 02A0 0363 0100 4681 0100 0BE3"            /* .P.Ñ. .c..F...ã */
	$"0104 16C1 0100 3B73 0100 5131 0100 6055"            /* ...Á..;s..Q1..`U */
	$"0100 482B 0100 2011 0100 2223 0100 5015"            /* ..H+.. ..."#..P. */
	$"0100 80A1 0104 0015 0100 0803 01FF FFFF"            /* ..€¡.........ÿÿÿ */
	$"01FF FFC0 01FF FFE0 07FF FFF0 1FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð.ÿÿø */
	$"3FFF FFFC 7FFF FFFE 7FFF FFFF FFFF FFFF"            /* ?ÿÿü.ÿÿþ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"7FFF FFFF 7FFF FFFF 3FFF FFFF 1FFF FFFF"            /* .ÿÿÿ.ÿÿÿ?ÿÿÿ.ÿÿÿ */
	$"0FFF FFFF 03FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"6963 6C34 0000 0208 0000 000E DEED EEDE"            /* icl4........ÞíîÞ */
	$"EDEE EEEE EE00 0000 0000 000D 0000 0000"            /* íîîîî........... */
	$"0000 0000 0EE0 0000 0000 CDED EDEC 0000"            /* .....à....Íííì.. */
	$"DC0C 00C0 0AEE 0000 000E EBD9 8BDA E000"            /* Ü..ÀÂî....ëÙ‹Úà. */
	$"C9C0 C00C 0ECE E000 00F9 D888 999D FE0C"            /* ÉÀÀ..Îà..ùØˆ™þ. */
	$"C68C 00C0 CE0C EE00 0DFD 888B 89FE DFE0"            /* ÆŒ.ÀÎ.î..ýˆ‹‰þßà */
	$"C9FD 0C00 0EC0 CEE0 CFEB 8188 A89A EEFC"            /* Éý...ÀÎàÏëˆ¨šîü */
	$"CEF7 0C0C 0EEF EEAF DFBD 9989 89AF D8FD"            /* Î÷...ïî¯ß½™‰‰¯Øý */
	$"009A C0CC CDDD DDDE 899D 8A8A 9A9F D99E"            /* .šÀÌÍÝÝÞ‰ŠŠšŸÙž */
	$"00CE C0EA D0DD DDCF A9AD 99A9 A9FA D9A9"            /* .ÎÀêÐÝÝÏ©­™©©úÙ© */
	$"0C0C AEDA DC00 00DE 999B D9F9 FFFD B99A"            /* ..®ÚÜ..Þ™›Ùùÿý¹š */
	$"000C EE9F DCCC 0CCF 8B98 DDE9 FECD 989E"            /* ..îŸÜÌ.Ï‹˜ÝéþÍ˜ž */
	$"00CD FEEE 9DD0 C0DA D889 A8CC CD88 89AD"            /* .ÍþîÐÀÚØ‰¨ÌÍˆ‰­ */
	$"0D8F F9DB E990 CCCF C8A8 888D B888 A89C"            /* .ùÛéÌÏÈ¨ˆ¸ˆ¨œ */
	$"0CFF 9FAE D9FE 00DE 0D88 A89B D88A 8AD0"            /* .ÿŸ®Ùþ.Þ.ˆ¨›ØŠŠÐ */
	$"C0DD DEFE 99ED C0DF 0089 188C C888 A900"            /* ÀÝÞþ™íÀß.‰.ŒÈˆ©. */
	$"0000 0EEF FEE0 C0DE 000D 888D B888 B00C"            /* ...ïþàÀÞ..ˆ¸ˆ°. */
	$"00C0 0CF9 F99C 0CC6 0000 CDEB DBBC 0000"            /* .À.ùùœ.Æ..ÍëÛ¼.. */
	$"0000 C0FE EFE0 C0DE 0000 0009 0000 0000"            /* ..ÀþïàÀÞ...Æ.... */
	$"0C00 0D99 FD0C 0CCF 0000 000E 0000 0000"            /* ...™ý..Ï........ */
	$"C00C DDFD EFC0 C0DF 0000 000E 0000 0C00"            /* À.ÝýïÀÀß........ */
	$"00CE BFF9 EFD0 0CCF 0000 000A 0000 0000"            /* .Î¿ùïÐ.Ï...Â.... */
	$"0CDE EDEE EEFC C0DF 0000 000E 0000 0000"            /* .ÞíîîüÀß........ */
	$"0D9D CCCD 0DFE 0CCF 0000 000F 0000 000C"            /* .ÌÍ.þ.Ï........ */
	$"CEED 00C0 0CBE CCCF 0000 000E 0000 0000"            /* Îí.À.¾ÌÏ........ */
	$"CECC C00C 0CEE C0DF 0000 000F 0000 000C"            /* ÎÌÀ..îÀß........ */
	$"0CB0 0C00 C0DE 0CCF 0000 000E 0000 0C00"            /* .°..ÀÞ.Ï........ */
	$"0CDC 00C0 00DD 00DF 0000 000F 0000 0000"            /* .Ü.À.Ý.ß........ */
	$"CCDD 000C 0CCD 0CDF 0000 000E 0000 0000"            /* ÌÝ...Í.ß........ */
	$"DCC0 C000 C0DB CCCF 0000 000F 0000 0C00"            /* ÜÀÀ.ÀÛÌÏ........ */
	$"00C0 00CC 00CC DCCF 0000 000A 0000 000C"            /* .À.Ì.ÌÜÏ...Â.... */
	$"0000 C000 C0C0 C0DF 0000 000E FAFF FFFF"            /* ..À.ÀÀÀß....úÿÿÿ */
	$"FFFF FFFF FFFF FFFF 6963 6C38 0000 0408"            /* ÿÿÿÿÿÿÿÿicl8.... */
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F8F6 F5F5 F5F5 F5F5 F5AC FBFC 0000 0000"            /* øöõõõõõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A67C FAFD FBF5 0000"            /* ..õû¬{| ¦|úýûõ.. */
	$"56C9 4FF5 F5F5 F5F6 F5FC F7FB AC00 0000"            /* VÉOõõõõöõü÷û¬... */
	$"00F5 ACFE 50A1 9BA1 A1D1 A7F9 EAAC 00F5"            /* .õ¬þP¡›¡¡Ñ§ùê¬.õ */
	$"F8CF A5F6 F5F5 F6F5 F6AC F5F7 FBAC 0000"            /* øÏ¥öõõöõö¬õ÷û¬.. */
	$"0081 FF7A 7DA1 9BA1 A1A7 D1D0 F9FF 81F5"            /* .ÿz}¡›¡¡§ÑÐùÿõ */
	$"F8CA D080 F5F5 F5F5 F5AC F5F6 F7FB AC00"            /* øÊÐ€õõõõõ¬õö÷û¬. */
	$"2BE9 AD57 E477 A1A1 A1A7 E8AD 81AD E92B"            /* +é­Wäw¡¡¡§è­­é+ */
	$"F6FC EA7A F5F6 F5F6 F5AC ACAC ACAC ACFD"            /* öüêzõöõöõ¬¬¬¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADFF 7BA6 E981"            /* {éüu§¡¡¡§Ë­ÿ{¦é */
	$"F5F5 CAFD F5F5 2B32 F656 FAFA FAFA FAAC"            /* õõÊýõõ+2öVúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBAD E8FF F9A7 E8A6"            /* ¦è¦ùË§¡§Ë­èÿù§è¦ */
	$"00F5 2BA6 F8F5 81DD 57F5 5656 5656 56FD"            /* .õ+¦øõÝWõVVVVVý */
	$"A7CB ADF9 A7CB ADCB ADE8 EAFD F9A7 E7AD"            /* §Ë­ù§Ë­Ë­èêýù§ç­ */
	$"F5F5 F5F8 FE81 F9D1 F9F6 F5F6 F5F6 F8FD"            /* õõõøþùÑùöõöõöøý */
	$"A7A7 CB7C 5CD1 D1D1 D1E0 EAF9 7CE7 A7FD"            /* §§Ë|\ÑÑÑÑàêù|ç§ý */
	$"0000 F52B ACFC D1E9 8150 2BF5 F6F6 56FD"            /* ..õ+¬üÑéP+õööVý */
	$"A0A1 A7CB 7B56 FCFD FEFC F87B E6A7 E6A6"            /*  ¡§Ë{Vüýþüø{æ§æ¦ */
	$"00F5 F67B E9AC FCAC A6FA F8F6 F6F5 55FD"            /* .õö{é¬ü¬¦úøööõUý */
	$"57E6 A1A1 A7A0 572C F775 A1A7 A1A1 A781"            /* Wæ¡¡§ W,÷u¡§¡¡§ */
	$"F550 CAEA EAFD 575D FCD1 CAF6 F62B F8FD"            /* õPÊêêýW]üÑÊöö+øý */
	$"2BA1 A1A1 A1A1 E57A 51A7 A1A1 A1A1 D14F"            /* +¡¡¡¡¡åzQ§¡¡¡¡ÑO */
	$"00F8 FFFF D0D1 FDA6 A5CA EAA5 F6F6 56FD"            /* .øÿÿÐÑý¦¥Êê¥ööVý */
	$"007B A1A1 A19B A757 759B A1A1 9BA7 8100"            /* .{¡¡¡›§Wu›¡¡›§. */
	$"F5F5 F97A 81D0 ADFD CACA FD56 F6F5 56FE"            /* õõùzÐ­ýÊÊýVöõVþ */
	$"00F5 7CA1 9B7D E457 50A1 9B9B A7AC 0000"            /* .õ|¡›}äWP¡››§¬.. */
	$"F500 F501 0081 FDEA D1D0 82F6 F6F6 56FD"            /* õ.õ..ýêÑÐ‚öööVý */
	$"0000 007B A19B A17A 4BA1 A1E7 8100 00F5"            /* ...{¡›¡zK¡¡ç..õ */
	$"F500 F5F5 F5F7 E9FE F3E8 C9F6 F6F6 56D2"            /* õ.õõõ÷éþóèÉöööVÒ */
	$"0000 0000 F77B A657 57A6 FAF7 0000 00F5"            /* ....÷{¦WW¦ú÷...õ */
	$"00F5 F5F5 F5F6 FDAC FDFE A6F6 07F6 F8FD"            /* .õõõõöý¬ýþ¦ö.öøý */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"00F5 F5F5 F5F8 EACA E981 F5F6 F5F6 56E0"            /* .õõõõøêÊéõöõöVà */
	$"0000 0000 0000 00FC 0000 0000 0000 F500"            /* .......ü......õ. */
	$"F5F5 00F6 F881 FF9F D0FD 2BF5 25F6 56FE"            /* õõ.öøÿŸÐý+õ%öVþ */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"00F5 F7A6 82E0 EACA ACFE FAF6 06F6 56FE"            /* .õ÷¦‚àêÊ¬þúö.öVþ */
	$"0000 0000 0000 00AC 0000 0000 F500 00F5"            /* .......¬....õ..õ */
	$"00F6 FAFB FC81 ACA6 FBAD E0F7 F6F6 56FE"            /* .öúûü¬¦û­à÷ööVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 F500"            /* .......¬......õ. */
	$"F556 AD81 2BF5 5656 F5FA FEFB F5F6 56EA"            /* õV­+õVVõúþûõöVê */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"F788 FB56 F5F5 F5F5 F5F6 FB82 F7F6 F8E0"            /* ÷ˆûVõõõõõöû‚÷öøà */
	$"0000 0000 0000 00AC 0000 0000 0000 F500"            /* .......¬......õ. */
	$"2BA6 F7F7 F6F5 F6F5 F6F7 FBFD 2BF6 56EA"            /* +¦÷÷öõöõö÷ûý+öVê */
	$"0000 0000 0000 00FD 0000 0000 F500 00F5"            /* .......ý....õ..õ */
	$"F556 5DF5 F5F5 F5F6 F5F5 F981 F6F6 F8F4"            /* õV]õõõõöõõùööøô */
	$"0000 0000 0000 00AC 0000 0000 00F5 0000"            /* .......¬.....õ.. */
	$"F5F5 81F6 F5F5 F5F5 F6F5 5657 F5F6 56FF"            /* õõöõõõõöõVWõöVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 00F5"            /* .......ý.......õ */
	$"F6F7 FA56 F5F5 F5F6 F5F6 F856 F6F6 56FF"            /* ö÷úVõõõöõöøVööVÿ */
	$"0000 0000 0000 00FD 0000 0000 F500 F500"            /* .......ý....õ.õ. */
	$"56F8 2CF5 F5F5 F5F5 F6F5 5682 F7F6 56FF"            /* Vø,õõõõõöõV‚÷öVÿ */
	$"0000 0000 0000 00FD 0000 0000 00F5 0000"            /* .......ý.....õ.. */
	$"F5F5 F5F5 F5F5 F6F5 F5F6 0756 F9F6 F8FF"            /* õõõõõõöõõö.Vùöøÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F5F5"            /* .......ý......õõ */
	$"00F5 00F5 F5F5 F5F6 F5F5 25F5 F6F6 56FF"            /* .õ.õõõõöõõ%õööVÿ */
	$"0000 0000 0000 00FD FDFD FEFE FEFE FEFE"            /* .......ýýýþþþþþþ */
	$"E0E0 F4E0 F4FF FFFF FFFF FFFF FFFF FFFF"            /* ààôàôÿÿÿÿÿÿÿÿÿÿÿ */
	$"696C 3332 0000 0AE9 84FF 125E 5D5C 5B5A"            /* il32..Âé„ÿ.^]\[Z */
	$"5958 5756 5453 5150 4E4D 4B49 4846 8AFF"            /* YXWVTSQPNMKIHFŠÿ */
	$"135D FFFF FEFD FBFA F8F6 F5F3 F1EF EEEC"            /* .]ÿÿþýûúøöõóñïîì */
	$"EAE8 E744 4286 FF17 B76E 6968 6E66 6DB6"            /* êèçDB†ÿ.·nihnfm¶ */
	$"FBFA F8F6 8CC9 F1EE EEEC EAE8 E742 5942"            /* ûúøöŒÉñîîìêèçBYB */
	$"83FF 1AEC 5432 775E 4741 526F 2E54 FAF8"            /* ƒÿ.ìT2w^GARo.Túø */
	$"F68D 26AB EFEE ECEA E8E7 40B3 5933 81FF"            /* ö&«ïîìêèç@³Y3ÿ */
	$"1CED 391A 8B3D 2F30 2615 1E7E 1639 F8F6"            /* .í9.‹=/0&..~.9øö */
	$"A419 1EBE EEEC EAE8 E73D DEB3 5933 80FF"            /* ¤..¾îìêèç=Þ³Y3€ÿ */
	$"5861 007F 4937 413A 2F24 102B 7600 5FF6"            /* Xa..I7A:/$.+v._ö */
	$"9F12 007B EEEC EAE8 E73B EEDE B359 33FF"            /* Ÿ..{îìêèç;îÞ³Y3ÿ */
	$"C503 2481 293C 3E39 2E23 1904 7623 02C1"            /* Å.$)<>9.#..v#.Á */
	$"DE35 006D EEEC EAE8 E739 3736 3432 312F"            /* Þ5.mîìêèç976421/ */
	$"7000 3B78 2133 3531 281F 1600 6E3A 0068"            /* p.;x!351(...n:.h */
	$"F5D6 1722 EEEC CCBD E09C 8275 1A2D 3E0F"            /* õÖ."îìÌ½àœ‚u.->. */
	$"357D 1828 2927 211A 1100 7934 1034 F5F3"            /* 5}.()'!...y4.4õó */
	$"AE4A A5EC 7D5E 9BE5 829C 7F2B 311D 1A8C"            /* ®J¥ì}^›å‚œ.+1..Œ */
	$"2617 1E1D 1913 031D 8C1B 1D24 F5F3 F1A4"            /* &.......Œ..$õóñ¤ */
	$"2569 7900 71E5 E3E2 E0DF 9C29 3523 1953"            /* %iy.qåãâàßœ)5#.S */
	$"880A 090E 0B00 078C 501A 2326 F5F3 F1C6"            /* ˆÂÆ....ŒP.#&õóñÆ */
	$"3743 2000 56A5 D2E2 E0DF 9C27 4823 281C"            /* 7C .V¥Òâàßœ'H#(. */
	$"6890 4517 1548 9466 1D29 2236 F5F3 C757"            /* hE..H”f.)"6õóÇW */
	$"0535 4C2E 5C5C 97E2 E0DF 9C25 7C20 2E2E"            /* .5L.\\—âàßœ%| .. */
	$"2248 7AB4 B57C 4623 2D2F 1769 F59F 0900"            /* "Hz´µ|F#-/.iõŸÆ. */
	$"0014 8C94 4A00 18B8 E0DF 9C7F 23C9 2832"            /* ..Œ”J..¸àßœ.#É(2 */
	$"3333 2F13 888D 1C2E 3334 3112 C0F5 9000"            /* 33/.ˆ..341.Àõ. */
	$"0001 072C 372F 0E00 53E0 DF9C 21FF 7726"            /* ...,7/..Sàßœ!ÿw& */
	$"3938 3924 888F 2C38 393A 1562 F6F5 F386"            /* 989$ˆ,89:.böõó† */
	$"8063 0B22 0C07 0F1A 8EE0 DF9C 1FFF EE5B"            /* €c."....Žàßœ.ÿî[ */
	$"283C 3F28 8993 323F 3B18 43F8 F6F5 F3F1"            /* (<?(‰“2?;.Cøöõóñ */
	$"EFF0 6129 0904 0154 D9E0 DF9C 1CFF FFED"            /* ïða)Æ..TÙàßœ.ÿÿí */
	$"722A 2F25 8994 2D26 1960 FAF8 F6F5 F3F1"            /* r*.%‰”-&.`úøöõóñ */
	$"EFEE BD06 0704 021C CEE0 DF9C 001C 81FF"            /* ïî½.....Îàßœ..ÿ */
	$"1BC3 6F2F 878C 346B BDFB FAF8 F6F5 F3F1"            /* .Ão/‡Œ4k½ûúøöõóñ */
	$"EFEE CE1A 4110 0340 E2E0 DF9C 1884 FF18"            /* ïîÎ.A..@âàßœ.„ÿ. */
	$"3DFF FFFE FDFB FAF8 F6F5 F3F1 EFEE A700"            /* =ÿÿþýûúøöõóñïî§. */
	$"0E05 67E1 E2E0 DF9C 1684 FF18 3BFF FFFE"            /* ..gáâàßœ.„ÿ.;ÿÿþ */
	$"FDFB FAF8 F6F5 F3F8 D1AB 6601 130E 24D4"            /* ýûúøöõóøÑ«f...$Ô */
	$"E2E0 DF9C 1484 FF18 39FF FFFE FDFB FAF8"            /* âàßœ.„ÿ.9ÿÿþýûúø */
	$"F6F5 F6BA 5A61 1900 0D2E 1A7D E2E0 DF9C"            /* öõöºZa.....}âàßœ */
	$"1284 FF18 37FF FFFE FDFB FAF8 F6F5 E26C"            /* .„ÿ.7ÿÿþýûúøöõâl */
	$"5C46 6830 305A 1911 B4E0 DF9C 1084 FF18"            /* \Fh00Z..´àßœ.„ÿ. */
	$"35FF FFFE FDFB FAF8 F6F5 9C43 5DBE EC86"            /* 5ÿÿþýûúøöõœC]¾ì† */
	$"9CE7 8513 5FE4 DF9C 0E84 FF18 33FF FFFE"            /* œç…._äßœ.„ÿ.3ÿÿþ */
	$"FDFB FAF8 F6BC 5152 9AEE ECEA E8E7 DE58"            /* ýûúøö¼QRšîìêèçÞX */
	$"59BE DF9C 0C84 FF18 31FF FFFE FDFB FAF8"            /* Y¾ßœ.„ÿ.1ÿÿþýûúø */
	$"F6CA 55B2 C2D6 ECEA E8E7 BD5A 32C4 DF9C"            /* öÊU²ÂÖìêèç½Z2Äßœ */
	$"0A84 FF18 2FFF FFFE FDFB FAF8 F6F5 9281"            /* Â„ÿ./ÿÿþýûúøöõ’ */
	$"EFEE EEEA E8E7 E587 69E0 DF9C 0884 FF18"            /* ïîîêèçå‡iàßœ.„ÿ. */
	$"2DFF FFFE FDFB FAF8 F6F5 E66E D8EE ECEE"            /* -ÿÿþýûúøöõænØîìî */
	$"E8E7 E5A4 95E0 DF9C 0684 FF18 2BFF FFFE"            /* èçå¤•àßœ.„ÿ.+ÿÿþ */
	$"FDFB FAF8 F6D5 C872 A7EE ECEE E8E7 E5A7"            /* ýûúøöÕÈr§îìîèçå§ */
	$"96E0 DF9C 0584 FF18 29FF FFFE FDFB FAF8"            /* –àßœ.„ÿ.)ÿÿþýûúø */
	$"F69E A8CC E9EE ECEA E8E7 E5A5 70B4 DF9C"            /* öž¨Ìéîìêèçå¥p´ßœ */
	$"0384 FF18 26FF FFFE FDFB FAF8 F6ED F4F4"            /* .„ÿ.&ÿÿþýûúøöíôô */
	$"EFEE ECEA E8E7 E5D9 9397 DF9C 0284 FF18"            /* ïîìêèçåÙ“—ßœ.„ÿ. */
	$"24FF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* $ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 0184 FF18 2220 1F1D"            /* èçåãâàßœ.„ÿ." .. */
	$"1B1A 1816 1513 1110 0E0D 0B0A 0807 0605"            /* ...........Â.... */
	$"0403 0201 0084 FF12 5E5D 5C5B 5A59 5857"            /* .....„ÿ.^]\[ZYXW */
	$"5654 5351 504E 4D4B 4948 468A FF13 5DFF"            /* VTSQPNMKIHFŠÿ.]ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* ÿþýûúøöõóñïîìêèç */
	$"4442 86FF 17B6 6A67 6C71 656C B7FB FAF8"            /* DB†ÿ.¶jglqel·ûúø */
	$"F6AA DAF1 EEEE ECEA E8E7 4259 4283 FF1A"            /* öªÚñîîìêèçBYBƒÿ. */
	$"ED55 2F94 A693 8079 782D 55FA F8F6 A173"            /* íU/”¦“€yx-Uúøö¡s */
	$"BEEF EEEC EAE8 E740 B359 3381 FF1C EE3E"            /* ¾ïîìêèç@³Y3ÿ.î> */
	$"19B6 ADA5 987C 5848 861B 3DF8 F6A8 505F"            /* .¶­¥˜|XH†.=øö¨P_ */
	$"D4EE ECEA E8E7 3DDE B359 3380 FF58 6F00"            /* Ôîìêèç=Þ³Y3€ÿXo. */
	$"90AB B2BE AC8A 6942 3E7E 0667 F6A4 482D"            /* «²¾¬ŠiB>~.gö¤H- */
	$"89EE ECEA E8E7 3BEE DEB3 5933 FFCC 2233"            /* ‰îìêèç;îÞ³Y3ÿÌ"3 */
	$"A891 B1B8 A787 674A 1C7B 4219 C4E0 451B"            /* ¨‘±¸§‡gJ.{B.ÄàE. */
	$"8CEE ECEA E8E7 3937 3634 3231 2F88 2A4F"            /* Œîìêèç976421/ˆ*O */
	$"9F7B 979C 8F76 5D42 1273 5F28 75F5 D965"            /* Ÿ{—œv]B.s_(uõÙe */
	$"4BEE ECCA ABDD 9C82 751A 2D67 4453 985F"            /* KîìÊ«Ýœ‚u.-gDS˜_ */
	$"7679 7161 4C33 087E 6445 4CF5 F3C0 4DA1"            /* vyqaL3.~dELõóÀM¡ */
	$"EC64 1289 E582 9C7F 2B66 5A4E 9C52 535B"            /* ìd.‰å‚œ.+fZNœRS[ */
	$"5549 3616 1BA0 5A5A 44F5 F3F1 A123 6F80"            /* UI6.. ZZDõóñ¡#o€ */
	$"2C92 E5E3 E2E0 DF9C 2971 685F 7C92 2A2E"            /* ,’åãâàßœ)qh_|’*. */
	$"3227 0F03 8D8B 6268 46F5 F3F1 C638 452F"            /* 2'..‹bhFõóñÆ8E/ */
	$"2275 AAD2 E2E0 DF9C 2787 7478 6B97 9745"            /* "uªÒâàßœ'‡txk——E */
	$"2119 419C A170 786F 53F5 F1DC 9222 2F49"            /* !.Aœ¡pxoSõñÜ’"/I */
	$"2F57 89B2 E2E0 DF9C 25AD 7E86 877E 96A6"            /* /W‰²âàßœ%­~†‡~–¦ */
	$"B8C8 B499 8086 8B64 77F5 C55D 221B 2381"            /* ¸È´™€†‹dwõÅ]".# */
	$"8644 3775 D5E0 DF9C 7F23 DD8B 9897 9898"            /* †D7uÕàßœ.#Ý‹˜—˜˜ */
	$"739B C88B 9497 9997 48C0 F5AD 1405 2335"            /* s›È‹”—™—HÀõ­..#5 */
	$"2C52 6E69 1268 E0DF 9C21 FEAF 99A9 A6A9"            /* ,Rni.hàßœ!þ¯™©¦© */
	$"879C CEA1 A6A8 AC66 6FF6 F5F3 8F8B 6D38"            /* ‡œÎ¡¦¨¬foöõó‹m8 */
	$"2430 5E5D 1B91 E0DF 9C1F FFF2 A1A1 B7BA"            /* $0^].‘àßœ.ÿò¡¡·º */
	$"949F D6B1 B9B4 705A F8F6 F5F3 F1EF F165"            /* ”ŸÖ±¹´pZøöõóñïñe */
	$"3014 3832 5AD6 E0DF 9C1C FFFF EFA6 91AC"            /* 0.82ZÖàßœ.ÿÿï¦‘¬ */
	$"97A1 DBAF 915B 71FA F8F6 F5F3 F1EF EEBD"            /* —¡Û¯‘[qúøöõóñïî½ */
	$"1811 1840 70D7 E0DF 9C00 1C80 FF1C FECF"            /* ...@p×àßœ..€ÿ.þÏ */
	$"9865 93AF 6680 BEFB FAF8 F6F5 F3F1 EFEE"            /* ˜e“¯f€¾ûúøöõóñïî */
	$"D01F 4214 1564 E2E0 DF9C 1884 FF18 3DFF"            /* Ð.B..dâàßœ.„ÿ.=ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE A81C 6025"            /* ÿþýûúøöõóñïî¨.`% */
	$"64DF E2E0 DF9C 1684 FF18 3BFF FFFE FDFB"            /* dßâàßœ.„ÿ.;ÿÿþýû */
	$"FAF8 F6F5 F3F6 D4AB 6312 8041 1CD3 E2E0"            /* úøöõóöÔ«c.€A.Óâà */
	$"DF9C 1484 FF18 39FF FFFE FDFB FAF8 F6F5"            /* ßœ.„ÿ.9ÿÿþýûúøöõ */
	$"F5C5 6559 120C 7237 167C E2E0 DF9C 1284"            /* õÅeY..r7.|âàßœ.„ */
	$"FF18 37FF FFFE FDFB FAF8 F6F5 E272 5741"            /* ÿ.7ÿÿþýûúøöõârWA */
	$"6441 5755 1811 B1E0 DF9C 1084 FF18 35FF"            /* dAWU..±àßœ.„ÿ.5ÿ */
	$"FFFE FDFB FAF8 F6F6 9A3D 5ABE EC93 A6E7"            /* ÿþýûúøööš=Z¾ì“¦ç */
	$"8513 5BE3 DF9C 0E84 FF18 33FF FFFE FDFB"            /* ….[ãßœ.„ÿ.3ÿÿþýû */
	$"FAF8 F6BA 4C50 98EE ECEA E8E7 DD55 53BB"            /* úøöºLP˜îìêèçÝUS» */
	$"DF9C 0C84 FF18 31FF FFFE FDFB FAF8 F6C8"            /* ßœ.„ÿ.1ÿÿþýûúøöÈ */
	$"51B1 C3D6 ECEA E8E7 BC56 33C3 DF9C 0A84"            /* Q±ÃÖìêèç¼V3ÃßœÂ„ */
	$"FF18 2FFF FFFE FDFB FAF8 F6F5 907F EFEE"            /* ÿ./ÿÿþýûúøöõ.ïî */
	$"EEEA E8E7 E583 69E0 DF9C 0884 FF18 2DFF"            /* îêèçåƒiàßœ.„ÿ.-ÿ */
	$"FFFE FDFB FAF8 F6F5 E66A D7EE ECEE E8E7"            /* ÿþýûúøöõæj×îìîèç */
	$"E5A2 93E0 DF9C 0684 FF18 2BFF FFFE FDFB"            /* å¢“àßœ.„ÿ.+ÿÿþýû */
	$"FAF8 F6D4 C56E A6EE ECEE E8E7 E5A6 93E0"            /* úøöÔÅn¦îìîèçå¦“à */
	$"DF9C 0584 FF18 29FF FFFE FDFB FAF8 F69C"            /* ßœ.„ÿ.)ÿÿþýûúøöœ */
	$"A2CA E9EE ECEA E8E7 E5A3 67B0 DF9C 0384"            /* ¢Êéîìêèçå£g°ßœ.„ */
	$"FF18 26FF FFFE FDFB FAF8 F6EC F4F4 EFEE"            /* ÿ.&ÿÿþýûúøöìôôïî */
	$"ECEA E8E7 E5D8 9093 DF9C 0284 FF18 24FF"            /* ìêèçåØ“ßœ.„ÿ.$ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* ÿþýûúøöõóñïîìêèç */
	$"E5E3 E2E0 DF9C 0184 FF18 2220 1F1D 1B1A"            /* åãâàßœ.„ÿ." .... */
	$"1816 1513 1110 0E0D 0B0A 0807 0605 0403"            /* .........Â...... */
	$"0201 0084 FF12 5E5D 5C5B 5A59 5857 5654"            /* ...„ÿ.^]\[ZYXWVT */
	$"5351 504E 4D4B 4948 468A FF13 5DFF FFFE"            /* SQPNMKIHFŠÿ.]ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 4442"            /* ýûúøöõóñïîìêèçDB */
	$"86FF 17B7 7069 666D 666E B6FB FAF8 F6AE"            /* †ÿ.·pifmfn¶ûúøö® */
	$"DCF1 EEEE ECEA E8E7 4259 4283 FF1A EC54"            /* ÜñîîìêèçBYBƒÿ.ìT */
	$"3469 4423 2447 6B2E 54FA F8F6 A27D C0EF"            /* 4iD#$Gk.Túøö¢}Àï */
	$"EEEC EAE8 E740 B359 3381 FF04 EC37 1C78"            /* îìêèç@³Y3ÿ.ì7.x */
	$"0F81 0013 147B 1539 F8F6 A657 68D7 EEEC"            /* ....{.9øö¦Wh×îì */
	$"EAE8 E73D DEB3 5933 80FF 035C 0078 1B83"            /* êèç=Þ³Y3€ÿ.\.x.ƒ */
	$"0018 2472 005C F6A3 4F31 8BEE ECEA E8E7"            /* ..$r.\ö£O1‹îìêèç */
	$"3BEE DEB3 5933 FFC1 001E 6E85 0017 7316"            /* ;îÞ³Y3ÿÁ..n…..s. */
	$"00C0 DF43 1B83 EEEC EAE8 E739 3736 3432"            /* .ÀßC.ƒîìêèç97642 */
	$"312F 6700 3463 8500 0D6D 2800 66F5 D740"            /* 1/g.4c…..m(.fõ×@ */
	$"35EE ECCA A1CF 9C82 7504 2D2D 0027 7185"            /* 5îìÊ¡Ïœ‚u.--.'q… */
	$"000D 771D 002C F5F3 B649 9EEC 6308 78E5"            /* ..w..,õó¶Ižìc.xå */
	$"829C 052B 1600 0787 1583 001A 1D84 0400"            /* ‚œ.+...‡.ƒ...„.. */
	$"15F5 F3F1 9B1A 6C7B 0B7F E5E3 E2E0 DF9C"            /* .õóñ›.l{..åãâàßœ */
	$"2916 0000 4386 0581 0016 0A8A 3900 0015"            /* )...C†...ÂŠ9... */
	$"F5F3 F1C1 2A3D 270B 5FA7 D2E2 E0DF 9C27"            /* õóñÁ*='._§Òâàßœ' */
	$"2D80 0007 578D 4714 154B 8F4E 8000 112C"            /* -€..WG..KN€.., */
	$"F5F2 D174 1229 3A24 4770 A3E2 E0DF 9C48"            /* õòÑt.):$Gp£âàßœH */
	$"6781 0005 2768 B1AC 6622 8100 1166 F5B1"            /* g..'h±¬f"..fõ± */
	$"310E 0E17 656B 3617 44C6 E0DF 9C23 C183"            /* 1...ek6.DÆàßœ#Áƒ */
	$"0001 7E72 8300 12C0 F59D 0000 101D 2339"            /* ..~rƒ..Àõ....#9 */
	$"453A 015B E0DF 9C21 FF5D 8200 017E 7082"            /* E:.[àßœ!ÿ]‚..~p‚ */
	$"0014 5DF6 F5F3 8B86 681D 1C1E 3235 178F"            /* ..]öõó‹†h...25. */
	$"E0DF 9C1F FFEC 3D81 0001 7E71 8100 163D"            /* àßœ.ÿì=..~q..= */
	$"F8F6 F5F3 F1EF F15D 220D 1C24 48D3 E0DF"            /* øöõóñïñ]"..$HÓàß */
	$"9C1C FFFF EB5F 8000 017E 7180 0013 5FFA"            /* œ.ÿÿë_€..~q€.._ú */
	$"F8F6 F5F3 F1EF EEBD 0F0B 0E21 42D1 E0DF"            /* øöõóñïî½...!BÑàß */
	$"9C1C 81FF 1BBF 6021 807B 2766 BEFB FAF8"            /* œ.ÿ.¿`!€{'f¾ûúø */
	$"F6F5 F3F1 EFEE CF19 301C 1151 E2E0 DF9C"            /* öõóñïîÏ.0..Qâàßœ */
	$"1884 FF18 3DFF FFFE FDFB FAF8 F6F5 F3F1"            /* .„ÿ.=ÿÿþýûúøöõóñ */
	$"EFEE A80C 3416 61E0 E2E0 DF9C 1684 FF18"            /* ïî¨.4.aàâàßœ.„ÿ. */
	$"3BFF FFFE FDFB FAF8 F6F5 F3F7 D4A8 6109"            /* ;ÿÿþýûúøöõó÷Ô¨aÆ */
	$"4824 1BD3 E2E0 DF9C 1484 FF18 39FF FFFE"            /* H$.Óâàßœ.„ÿ.9ÿÿþ */
	$"FDFB FAF8 F6F5 F6C1 5245 0C06 3E27 1072"            /* ýûúøöõöÁRE..>'.r */
	$"E2E0 DF9C 1284 FF18 37FF FFFE FDFB FAF8"            /* âàßœ.„ÿ.7ÿÿþýûúø */
	$"F6F5 E35F 3E36 6138 434C 0F0A ADE0 DF9C"            /* öõã_>6a8CL.Â­àßœ */
	$"1084 FF18 35FF FFFE FDFB FAF8 F6F6 942D"            /* .„ÿ.5ÿÿþýûúøöö”- */
	$"57BE EC8C A1E7 8311 51E4 DF9C 0E84 FF18"            /* W¾ìŒ¡çƒ.Qäßœ.„ÿ. */
	$"33FF FFFE FDFB FAF8 F6B6 3A4E 97EE ECEA"            /* 3ÿÿþýûúøö¶:N—îìê */
	$"E8E7 DD4C 3DB6 DF9C 0C84 FF18 31FF FFFE"            /* èçÝL=¶ßœ.„ÿ.1ÿÿþ */
	$"FDFB FAF8 F6C6 41AD C2D5 ECEA E8E7 BB4C"            /* ýûúøöÆA­ÂÕìêèç»L */
	$"28BF DF9C 0A84 FF18 2FFF FFFE FDFB FAF8"            /* (¿ßœÂ„ÿ./ÿÿþýûúø */
	$"F6F5 8A78 EFEE EEEA E8E7 E57F 5EE0 DF9C"            /* öõŠxïîîêèçå.^àßœ */
	$"0884 FF18 2DFF FFFE FDFB FAF8 F6F5 E661"            /* .„ÿ.-ÿÿþýûúøöõæa */
	$"D6EE ECEE E8E7 E59E 88E0 DF9C 0684 FF18"            /* Öîìîèçåžˆàßœ.„ÿ. */
	$"2BFF FFFE FDFB FAF8 F6D4 C05E A3EE ECEE"            /* +ÿÿþýûúøöÔÀ^£îìî */
	$"E8E7 E5A0 87E0 DF9C 0584 FF18 29FF FFFE"            /* èçå ‡àßœ.„ÿ.)ÿÿþ */
	$"FDFB FAF8 F695 96C7 E9EE ECEA E8E7 E59F"            /* ýûúøö•–ÇéîìêèçåŸ */
	$"50A9 DF9C 0384 FF18 26FF FFFE FDFB FAF8"            /* P©ßœ.„ÿ.&ÿÿþýûúø */
	$"F6ED F5F4 EFEE ECEA E8E7 E5D9 8389 DF9C"            /* öíõôïîìêèçåÙƒ‰ßœ */
	$"0284 FF18 24FF FFFE FDFB FAF8 F6F5 F3F1"            /* .„ÿ.$ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 0184 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"2220 1F1D 1B1A 1816 1513 1110 0E0D 0B0A"            /* " .............Â */
	$"0807 0605 0403 0201 006C 386D 6B00 0004"            /* .........l8mk... */
	$"0800 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF00 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿ..... */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 0000 0000 FFFF FFFF FFFF FFFF FFFF"            /* ......ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ....ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ.ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ.ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ..ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ...ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿ....ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* ÿ......ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF"                                                 /* ÿ */
};

data 'icns' (132, "Music Icon") {
	$"6963 6E73 0000 15C6 4943 4E23 0000 0108"            /* icns...ÆICN#.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"01FF FFC0 01FF FFE0 07FF FFF0 1FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð.ÿÿø */
	$"3FFF FFFC 7FFF FFFE 7FFF FFFF FFFF FFFF"            /* ?ÿÿü.ÿÿþ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"7FFF FFFF 7FFF FFFF 3FFF FFFF 1FFF FFFF"            /* .ÿÿÿ.ÿÿÿ?ÿÿÿ.ÿÿÿ */
	$"0FFF FFFF 03FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"6963 6C34 0000 0208 0000 000E DEED EEDE"            /* icl4........ÞíîÞ */
	$"EDEE EEEE EE00 0000 0000 000D 0000 0000"            /* íîîîî........... */
	$"0000 0000 0EE0 0000 0000 CDED EDEC 0000"            /* .....à....Íííì.. */
	$"C00C 00C0 0AEE 0000 000E ED8E 8EDE E000"            /* À..ÀÂî....íŽŽÞà. */
	$"0C00 C00C 0ECE E000 00AF C888 A8FD FE00"            /* ..À..Îà..¯Èˆ¨ýþ. */
	$"000C 00C0 CE0C EE00 0EFD 8888 899A DFEC"            /* ...ÀÎ.î..ýˆˆ‰šßì */
	$"00C0 0C00 0EC0 CEE0 C9AD 8B8B 8A9F D9F0"            /* .À...ÀÎàÉ­‹‹ŠŸÙð */
	$"C000 C00C 0FEE EEAF DF9D A888 99AF DE9E"            /* À.À..îî¯ß¨ˆ™¯Þž */
	$"000C 00C0 CCDD EDDE E9ED 89B9 9A9F D99E"            /* ...ÀÌÝíÞéí‰¹šŸÙž */
	$"0000 0C0C 00DD CDDE 899D A999 A9FE D99A"            /* .....ÝÍÞ‰©™©þÙš */
	$"00C0 C000 0C00 C0CF A8A8 DAF9 FFFD 8A99"            /* .ÀÀ...ÀÏ¨¨ÚùÿýŠ™ */
	$"0000 0C0C 00CC 0CDE 8A89 DDEE FECB A98A"            /* .....Ì.ÞŠ‰ÝîþË©Š */
	$"000C 00C0 0CC0 C0CF D888 98DC CB88 88AD"            /* ...À.ÀÀÏØˆ˜ÜËˆˆ­ */
	$"C000 C00C 000C 0CCF 0998 A88D D888 8A9C"            /* À.À....ÏÆ˜¨ØˆŠœ */
	$"00C0 0C00 C0C0 C0DE 0B88 88AB 1889 88D0"            /* .À..ÀÀÀÞ.ˆˆ«.‰ˆÐ */
	$"0C00 F0C0 0C0C 0CDE 008B 888D C888 AE0C"            /* ..ðÀ...Þ.‹ˆÈˆ®. */
	$"000C F000 C0C0 0CCF 000B 888D D889 D000"            /* ..ð.ÀÀ.Ï..ˆØ‰Ð. */
	$"0000 F0C0 0CF0 C0D6 0000 0DED BEBC 00F0"            /* ..ðÀ.ðÀÖ...í¾¼.ð */
	$"00C0 FC00 C0FC C0DE 0000 0009 0000 00F0"            /* .Àü.ÀüÀÞ...Æ...ð */
	$"C000 F00C 0CF0 0CCF 0000 000E 0000 00F0"            /* À.ð..ð.Ï.......ð */
	$"00C0 F00C 00F0 CCCF 0000 000A 0000 00F0"            /* .Àð..ðÌÏ...Â...ð */
	$"000C F0C0 C0FC 00DF 0000 000E 0DDD DDFD"            /* ..ðÀÀü.ß.....ÝÝý */
	$"DDDD FDDD DDFD DECF 0000 000F 0000 00F0"            /* ÝÝýÝÝýÞÏ.......ð */
	$"00FF F00C 0CF0 00DF 0000 000E 0000 00F0"            /* .ÿð..ð.ß.......ð */
	$"0FFF FC00 C0F0 CCCF 0000 000E 0DDD DDFD"            /* .ÿü.ÀðÌÏ.....ÝÝý */
	$"DFFF FDDD FFFD DDDF 0000 000F 0000 FFF0"            /* ßÿýÝÿýÝß......ÿð */
	$"00FF 000F FFF0 C0CF 0000 000E 000F FFF0"            /* .ÿ..ÿðÀÏ......ÿð */
	$"C000 C0CF FFFC 0CDF 0000 000F 0DDF FFFD"            /* À.ÀÏÿü.ß.....ßÿý */
	$"DDDD DDDD FFDD DDCF 0000 000E 0000 FF00"            /* ÝÝÝÝÿÝÝÏ......ÿ. */
	$"0000 0C00 0C0C 0CCF 0000 000F 0000 0000"            /* .......Ï........ */
	$"00C0 C000 C0C0 C0DF 0000 000A 0000 000C"            /* .ÀÀ.ÀÀÀß...Â.... */
	$"000C 00C0 0C0C 0CCF 0000 000E FAFF FFFA"            /* ...À...Ï....úÿÿú */
	$"FFFF FFFF FFFF FFFF 6963 6C38 0000 0408"            /* ÿÿÿÿÿÿÿÿicl8.... */
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F5F5 00F5 F5F5 F5F5 F5AC FBFC 0000 0000"            /* õõ.õõõõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A082 FAAC FC00 00F5"            /* ..õû¬{|  ‚ú¬ü..õ */
	$"00F5 F5F5 F5F5 F5F6 F5FC F7FB AC00 0000"            /* .õõõõõõöõü÷û¬... */
	$"00F5 ACFE 50A1 A1A1 A7CB AD7A DFAC 0000"            /* .õ¬þP¡¡¡§Ë­zß¬.. */
	$"F500 F5F5 F5F5 F6F5 F6AC F5F7 FBAC 0000"            /* õ.õõõõöõö¬õ÷û¬.. */
	$"0081 FF7A 7D9B 9BA1 A1A7 E7AD FAEA FBF5"            /* .ÿz}››¡¡§ç­úêûõ */
	$"00F5 F5F5 F5F5 F5F5 F5AC F5F6 F7FB AC00"            /* .õõõõõõõõ¬õö÷û¬. */
	$"2BE9 AD57 C57D 9BA1 A1A7 E8E9 F9E8 E92B"            /* +é­WÅ}›¡¡§èéùèé+ */
	$"F5F5 F5F5 F5F5 F5F6 F5AC ACAC ACAC ACFD"            /* õõõõõõõöõ¬¬¬¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADEA FAA6 D1FB"            /* {éüu§¡¡¡§Ë­êú¦Ñû */
	$"00F5 00F5 F5F5 F6F5 F556 FAFA FAFA FAAC"            /* .õ.õõõöõõVúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBAD E8FF FAA7 E8A6"            /* ¦è¦ùË§¡§Ë­èÿú§è¦ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 5656 5656 56FD"            /* .õõõõõõöõõVVVVVý */
	$"A7A7 E8F9 A7E7 A7CB ADE8 EAFD 56E7 A7AD"            /* §§èù§ç§Ë­èêýVç§­ */
	$"F5F5 F5F5 F5F5 F5F5 F5F6 F6F5 F6F6 F8FD"            /* õõõõõõõõõööõööøý */
	$"A7A7 A7A6 56E9 E9D1 D1E0 EA56 A0A7 A7D0"            /* §§§¦VééÑÑàêV §§Ð */
	$"0000 F5F5 F5F5 F6F5 F6F5 F6F6 F5F6 56FD"            /* ..õõõõöõöõööõöVý */
	$"A0A7 E6A7 7B56 FCFD FEFC F87C A7A7 A7A7"            /*  §æ§{Vüýþüø|§§§§ */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F6F5 2B01 55FD"            /* .õõõõõõöõööõ+.Uý */
	$"57C5 A1A1 A7A0 572C 4F51 A1E6 A1A1 E7FA"            /* WÅ¡¡§ W,OQ¡æ¡¡çú */
	$"F500 F5F5 F5F5 F5F5 F6F5 F6F6 F6F6 56FD"            /* õ.õõõõõõöõööööVý */
	$"25A7 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E8F7"            /* %§¡¡¡¡æzQæ¡¡¡¡è÷ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 F6F5 F6F6 56FD"            /* .õõõõõõöõõöõööVý */
	$"0075 A1A1 A19B A757 51A1 A1A1 9BE7 FA00"            /* .u¡¡¡›§WQ¡¡¡›çú. */
	$"F5F5 F5F5 FFF5 F5F5 F6F6 F5F6 F6F6 56FD"            /* õõõõÿõõõööõöööVý */
	$"00F5 7CA1 9BA1 A157 4AA1 9B9B A7A6 00F5"            /* .õ|¡›¡¡WJ¡››§¦.õ */
	$"00F5 00F5 FFF5 F5F6 F5F5 F6F6 F5F6 F8FE"            /* .õ.õÿõõöõõööõöøþ */
	$"0000 F57B A19B E5F9 519B A7E7 8100 00F5"            /* ..õ{¡›åùQ›§ç..õ */
	$"F500 F5F5 FFF5 F5F5 F6F6 FFF5 F6F6 56D2"            /* õ.õõÿõõõööÿõööVÒ */
	$"0000 0000 2B7B A6F9 57A6 7B2B 0000 FFF5"            /* ....+{¦ùW¦{+..ÿõ */
	$"00F5 F5F5 FFF5 F5F6 F5F5 FFF6 F6F5 56FD"            /* .õõõÿõõöõõÿööõVý */
	$"0000 0000 0000 00AC 0000 0000 0000 FF00"            /* .......¬......ÿ. */
	$"F500 F5F5 FFF5 F5F5 F6F6 FFF6 F6F6 56FE"            /* õ.õõÿõõõööÿöööVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 FFF5"            /* .......¬......ÿõ */
	$"00F5 F5F5 FFF5 F5F6 F5F5 FFF5 F6F6 F8FE"            /* .õõõÿõõöõõÿõööøþ */
	$"0000 0000 0000 00AC 0000 0000 00F5 FF00"            /* .......¬.....õÿ. */
	$"F500 F5F5 FFF5 F5F5 F6F6 FFF6 F5F6 56FE"            /* õ.õõÿõõõööÿöõöVþ */
	$"0000 0000 0000 00AC 00F9 FAFA FAF9 FFFA"            /* .......¬.ùúúúùÿú */
	$"FAFA FAF9 FFFA FAFA FAF9 FFF9 FAFA 56FE"            /* úúúùÿúúúúùÿùúúVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 FF00"            /* .......¬......ÿ. */
	$"0000 FFFF FF00 F5F5 F5F6 FFF5 F6F5 56FF"            /* ..ÿÿÿ.õõõöÿõöõVÿ */
	$"0000 0000 0000 00AC 0000 0000 0000 FFF5"            /* .......¬......ÿõ */
	$"00FF FFFF FFF6 F5F5 F6F5 FFF6 F6F6 56FE"            /* .ÿÿÿÿöõõöõÿöööVþ */
	$"0000 0000 0000 00AC 00FA F9FA FAFA FFF9"            /* .......¬.úùúúúÿù */
	$"FAFF FFFF FFF9 FAFA FFFF FFFA F9FA F8FF"            /* úÿÿÿÿùúúÿÿÿúùúøÿ */
	$"0000 0000 0000 00FD 0000 0000 FFFF FF00"            /* .......ý....ÿÿÿ. */
	$"F500 FFFF F5F5 F5FF FFFF FFF5 F6F5 56FF"            /* õ.ÿÿõõõÿÿÿÿõöõVÿ */
	$"0000 0000 0000 00AC 0000 00FF FFFF FF00"            /* .......¬...ÿÿÿÿ. */
	$"F5F5 00F5 F5F5 F5FF FFFF FFF6 F6F6 56FF"            /* õõ.õõõõÿÿÿÿöööVÿ */
	$"0000 0000 0000 00FD 00F9 FAFF FFFF FFFA"            /* .......ý.ùúÿÿÿÿú */
	$"FAF9 FAFA F9FA FAF9 FFFF F9FA F9FA F8FF"            /* úùúúùúúùÿÿùúùúøÿ */
	$"0000 0000 0000 00FD 0000 0000 FFFF 0000"            /* .......ý....ÿÿ.. */
	$"00F5 00F5 F5F5 F5F5 F5F6 F5F6 F6F5 56FF"            /* .õ.õõõõõõöõööõVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F500"            /* .......ý......õ. */
	$"F5F5 F5F5 F5F5 F5F5 F6F5 F6F5 F6F6 56FF"            /* õõõõõõõõöõöõööVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 00F5"            /* .......ý.......õ */
	$"00F5 F5F5 F5F5 F6F5 F5F6 F6F6 F5F6 56FF"            /* .õõõõõöõõöööõöVÿ */
	$"0000 0000 0000 00FD FDFD FEFE FEFE FEFE"            /* .......ýýýþþþþþþ */
	$"FFFE FEFF FEFF FFFF FFFF FFFF FFFF FFFF"            /* ÿþþÿþÿÿÿÿÿÿÿÿÿÿÿ */
	$"696C 3332 0000 0A9E 84FF 125E 5D5C 5B5A"            /* il32..Âž„ÿ.^]\[Z */
	$"5958 5756 5453 5150 4E4D 4B49 4846 8AFF"            /* YXWVTSQPNMKIHFŠÿ */
	$"135D FFFF FEFD FBFA F8F6 F5F3 F1EF EEEC"            /* .]ÿÿþýûúøöõóñïîì */
	$"EAE8 E744 4286 FF17 B76E 6968 6E66 6DB6"            /* êèçDB†ÿ.·nihnfm¶ */
	$"FBFA F8F6 F5F3 F1EF EEEC EAE8 E742 5942"            /* ûúøöõóñïîìêèçBYB */
	$"83FF 1AEC 5432 775E 4741 526F 2E54 FAF8"            /* ƒÿ.ìT2w^GARo.Túø */
	$"F6F5 F3F1 EFEE ECEA E8E7 40B3 5933 81FF"            /* öõóñïîìêèç@³Y3ÿ */
	$"1CED 391A 8B3D 2F30 2615 1E7E 1639 F8F6"            /* .í9.‹=/0&..~.9øö */
	$"F5F3 F1EF EEEC EAE8 E73D DEB3 5933 80FF"            /* õóñïîìêèç=Þ³Y3€ÿ */
	$"5861 007F 4937 413A 2F24 102B 7600 5FF6"            /* Xa..I7A:/$.+v._ö */
	$"F5F3 F1EF EEEC EAE8 E73B EEDE B359 33FF"            /* õóñïîìêèç;îÞ³Y3ÿ */
	$"C503 2481 293C 3E39 2E23 1904 7623 02C1"            /* Å.$)<>9.#..v#.Á */
	$"F5F3 F1EF EEEC EAE8 E739 3736 3432 312F"            /* õóñïîìêèç976421/ */
	$"7000 3B78 2133 3531 281F 1600 6E3A 0068"            /* p.;x!351(...n:.h */
	$"F5F3 F1EF EEEC EAE8 E79C 8275 1A2D 3E0F"            /* õóñïîìêèçœ‚u.->. */
	$"357D 1828 2927 211A 1100 7934 1034 F5F3"            /* 5}.()'!...y4.4õó */
	$"F1EF EEEC EAE8 E7E5 829C 7F2B 311D 1A8C"            /* ñïîìêèçå‚œ.+1..Œ */
	$"2617 1E1D 1913 031D 8C1B 1D24 F5F3 F1EF"            /* &.......Œ..$õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C29 3523 1953"            /* îìêèçåãâàßœ)5#.S */
	$"880A 090E 0B00 078C 501A 2326 F5F3 F1EF"            /* ˆÂÆ....ŒP.#&õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C27 4823 281C"            /* îìêèçåãâàßœ'H#(. */
	$"6890 4517 1548 9466 1D29 2236 F5F3 F1EF"            /* hE..H”f.)"6õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C25 7C20 2E2E"            /* îìêèçåãâàßœ%| .. */
	$"2248 7AB4 B57C 4623 2D2F 1769 F5F3 F1EF"            /* "Hz´µ|F#-/.iõóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C7F 23C9 2832"            /* îìêèçåãâàßœ.#É(2 */
	$"3333 2F13 888D 1C2E 3334 3112 C0F5 F3F1"            /* 33/.ˆ..341.Àõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 21FF 7726"            /* ïîìêèçåãâàßœ!ÿw& */
	$"3938 3924 888F 2C38 393A 1562 F6F5 F3F1"            /* 989$ˆ,89:.böõóñ */
	$"EF00 ECEA E8E7 E5E3 E2E0 DF9C 1FFF EE5B"            /* ï.ìêèçåãâàßœ.ÿî[ */
	$"283C 3F28 8993 323F 3B18 43F8 F6F5 F3F1"            /* (<?(‰“2?;.Cøöõóñ */
	$"EF00 ECEA E8E7 E5E3 E2E0 DF9C 1CFF FFED"            /* ï.ìêèçåãâàßœ.ÿÿí */
	$"722A 2F25 8994 2D26 1960 FAF8 F6F5 F3F1"            /* r*.%‰”-&.`úøöõóñ */
	$"EF00 ECEA E8E7 E500 E2E0 DF9C 001C 81FF"            /* ï.ìêèçå.âàßœ..ÿ */
	$"1BC3 6F2F 878C 346B BDFB FA00 F6F5 F3F1"            /* .Ão/‡Œ4k½ûú.öõóñ */
	$"EF00 ECEA E8E7 E500 E2E0 DF9C 1884 FF18"            /* ï.ìêèçå.âàßœ.„ÿ. */
	$"3DFF FFFE FDFB FA00 F6F5 F3F1 EF00 ECEA"            /* =ÿÿþýûú.öõóñï.ìê */
	$"E8E7 E500 E2E0 DF9C 1684 FF18 3BFF FFFE"            /* èçå.âàßœ.„ÿ.;ÿÿþ */
	$"FDFB FA00 F6F5 F3F1 EF00 ECEA E8E7 E500"            /* ýûú.öõóñï.ìêèçå. */
	$"E2E0 DF9C 1484 FF18 39FF FFFE FDFB FA00"            /* âàßœ.„ÿ.9ÿÿþýûú. */
	$"F6F5 F3F1 EF00 ECEA E8E7 E500 E2E0 DF9C"            /* öõóñï.ìêèçå.âàßœ */
	$"1284 FF01 37FF 8280 0000 8280 0000 8280"            /* .„ÿ.7ÿ‚€..‚€..‚€ */
	$"0000 8080 019C 1084 FF0A 35FF FFFE FDFB"            /* ..€€.œ.„ÿÂ5ÿÿþýû */
	$"FA00 F6F5 F380 000A ECEA E8E7 E500 E2E0"            /* ú.öõó€.Âìêèçå.âà */
	$"DF9C 0E84 FF09 33FF FFFE FDFB FA00 F6F5"            /* ßœ.„ÿÆ3ÿÿþýûú.öõ */
	$"8100 0AEC EAE8 E7E5 00E2 E0DF 9C0C 84FF"            /* .Âìêèçå.âàßœ.„ÿ */
	$"0131 FF82 8002 0080 8081 0080 8080 0080"            /* .1ÿ‚€..€€.€€€.€ */
	$"8001 9C0A 84FF 042F FFFF FEFD 8000 07F6"            /* €.œÂ„ÿ./ÿÿþý€..ö */
	$"F5F3 0000 EEEC EA81 0004 E2E0 DF9C 0884"            /* õó..îìê..âàßœ.„ */
	$"FF03 2DFF FFFE 8100 07F6 F5F3 F1EF EEEC"            /* ÿ.-ÿÿþ..öõóñïîì */
	$"EA81 0004 E2E0 DF9C 0684 FF03 2BFF 8080"            /* ê..âàßœ.„ÿ.+ÿ€€ */
	$"8100 8680 0100 0081 8001 9C05 84FF 1829"            /* .†€...€.œ.„ÿ.) */
	$"FFFF FEFD 0000 F8F6 F5F3 F1EF EEEC EAE8"            /* ÿÿþý..øöõóñïîìêè */
	$"E7E5 E3E2 E0DF 9C03 84FF 1826 FFFF FEFD"            /* çåãâàßœ.„ÿ.&ÿÿþý */
	$"FBFA F8F6 F5F3 F1EF EEEC EAE8 E7E5 E3E2"            /* ûúøöõóñïîìêèçåãâ */
	$"E0DF 9C02 84FF 1824 FFFF FEFD FBFA F8F6"            /* àßœ.„ÿ.$ÿÿþýûúøö */
	$"F5F3 F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C01"            /* õóñïîìêèçåãâàßœ. */
	$"84FF 1822 201F 1D1B 1A18 1615 1311 100E"            /* „ÿ." ........... */
	$"0D0B 0A08 0706 0504 0302 0100 84FF 125E"            /* ..Â.........„ÿ.^ */
	$"5D5C 5B5A 5958 5756 5453 5150 4E4D 4B49"            /* ]\[ZYXWVTSQPNMKI */
	$"4846 8AFF 135D FFFF FEFD FBFA F8F6 F5F3"            /* HFŠÿ.]ÿÿþýûúøöõó */
	$"F1EF EEEC EAE8 E744 4286 FF17 B66A 676C"            /* ñïîìêèçDB†ÿ.¶jgl */
	$"7165 6CB7 FBFA F8F6 F5F3 F1EF EEEC EAE8"            /* qel·ûúøöõóñïîìêè */
	$"E742 5942 83FF 1AED 552F 94A6 9380 7978"            /* çBYBƒÿ.íU/”¦“€yx */
	$"2D55 FAF8 F6F5 F3F1 EFEE ECEA E8E7 40B3"            /* -Uúøöõóñïîìêèç@³ */
	$"5933 81FF 1CEE 3E19 B6AD A598 7C58 4886"            /* Y3ÿ.î>.¶­¥˜|XH† */
	$"1B3D F8F6 F5F3 F1EF EEEC EAE8 E73D DEB3"            /* .=øöõóñïîìêèç=Þ³ */
	$"5933 80FF 586F 0090 ABB2 BEAC 8A69 423E"            /* Y3€ÿXo.«²¾¬ŠiB> */
	$"7E06 67F6 F5F3 F1EF EEEC EAE8 E73B EEDE"            /* ~.göõóñïîìêèç;îÞ */
	$"B359 33FF CC22 33A8 91B1 B8A7 8767 4A1C"            /* ³Y3ÿÌ"3¨‘±¸§‡gJ. */
	$"7B42 19C4 F5F3 F1EF EEEC EAE8 E739 3736"            /* {B.Äõóñïîìêèç976 */
	$"3432 312F 882A 4F9F 7B97 9C8F 765D 4212"            /* 421/ˆ*OŸ{—œv]B. */
	$"735F 2875 F5F3 F1EF EEEC EAE8 E79C 8275"            /* s_(uõóñïîìêèçœ‚u */
	$"1A2D 6744 5398 5F76 7971 614C 3308 7E64"            /* .-gDS˜_vyqaL3.~d */
	$"454C F5F3 F1EF EEEC EAE8 E7E5 829C 7F2B"            /* ELõóñïîìêèçå‚œ.+ */
	$"665A 4E9C 5253 5B55 4936 161B A05A 5A44"            /* fZNœRS[UI6.. ZZD */
	$"F5F3 F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C29"            /* õóñïîìêèçåãâàßœ) */
	$"7168 5F7C 922A 2E32 270F 038D 8B62 6846"            /* qh_|’*.2'..‹bhF */
	$"F5F3 F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C27"            /* õóñïîìêèçåãâàßœ' */
	$"8774 786B 9797 4521 1941 9CA1 7078 6F53"            /* ‡txk——E!.Aœ¡pxoS */
	$"F5F3 F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C25"            /* õóñïîìêèçåãâàßœ% */
	$"AD7E 8687 7E96 A6B8 C8B4 9980 868B 6477"            /* ­~†‡~–¦¸È´™€†‹dw */
	$"F5F3 F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C7F"            /* õóñïîìêèçåãâàßœ. */
	$"23DD 8B98 9798 9873 9BC8 8B94 9799 9748"            /* #Ý‹˜—˜˜s›È‹”—™—H */
	$"C0F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* Àõóñïîìêèçåãâàßœ */
	$"21FE AF99 A9A6 A987 9CCE A1A6 A8AC 666F"            /* !þ¯™©¦©‡œÎ¡¦¨¬fo */
	$"F6F5 F3F1 EF00 ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñï.ìêèçåãâàßœ */
	$"1FFF F2A1 A1B7 BA94 9FD6 B1B9 B470 5AF8"            /* .ÿò¡¡·º”ŸÖ±¹´pZø */
	$"F6F5 F3F1 EF00 ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñï.ìêèçåãâàßœ */
	$"1CFF FFEF A691 AC97 A1DB AF91 5B71 FAF8"            /* .ÿÿï¦‘¬—¡Û¯‘[qúø */
	$"F6F5 F3F1 EF00 ECEA E8E7 E500 E2E0 DF9C"            /* öõóñï.ìêèçå.âàßœ */
	$"001C 80FF 1CFE CF98 6593 AF66 80BE FBFA"            /* ..€ÿ.þÏ˜e“¯f€¾ûú */
	$"00F6 F5F3 F1EF 00EC EAE8 E7E5 00E2 E0DF"            /* .öõóñï.ìêèçå.âàß */
	$"9C18 84FF 183D FFFF FEFD FBFA 00F6 F5F3"            /* œ.„ÿ.=ÿÿþýûú.öõó */
	$"F1EF 00EC EAE8 E7E5 00E2 E0DF 9C16 84FF"            /* ñï.ìêèçå.âàßœ.„ÿ */
	$"183B FFFF FEFD FBFA 00F6 F5F3 F1EF 00EC"            /* .;ÿÿþýûú.öõóñï.ì */
	$"EAE8 E7E5 00E2 E0DF 9C14 84FF 1839 FFFF"            /* êèçå.âàßœ.„ÿ.9ÿÿ */
	$"FEFD FBFA 00F6 F5F3 F1EF 00EC EAE8 E7E5"            /* þýûú.öõóñï.ìêèçå */
	$"00E2 E0DF 9C12 84FF 0137 FF82 8000 0082"            /* .âàßœ.„ÿ.7ÿ‚€..‚ */
	$"8000 0082 8000 0080 8001 9C10 84FF 0A35"            /* €..‚€..€€.œ.„ÿÂ5 */
	$"FFFF FEFD FBFA 00F6 F5F3 8000 0AEC EAE8"            /* ÿÿþýûú.öõó€.Âìêè */
	$"E7E5 00E2 E0DF 9C0E 84FF 0933 FFFF FEFD"            /* çå.âàßœ.„ÿÆ3ÿÿþý */
	$"FBFA 00F6 F581 000A ECEA E8E7 E500 E2E0"            /* ûú.öõ.Âìêèçå.âà */
	$"DF9C 0C84 FF01 31FF 8280 0200 8080 8100"            /* ßœ.„ÿ.1ÿ‚€..€€. */
	$"8080 8000 8080 019C 0A84 FF04 2FFF FFFE"            /* €€€.€€.œÂ„ÿ./ÿÿþ */
	$"FD80 0007 F6F5 F300 00EE ECEA 8100 04E2"            /* ý€..öõó..îìê..â */
	$"E0DF 9C08 84FF 032D FFFF FE81 0007 F6F5"            /* àßœ.„ÿ.-ÿÿþ..öõ */
	$"F3F1 EFEE ECEA 8100 04E2 E0DF 9C06 84FF"            /* óñïîìê..âàßœ.„ÿ */
	$"032B FF80 8081 0086 8001 0000 8180 019C"            /* .+ÿ€€.†€...€.œ */
	$"0584 FF18 29FF FFFE FD00 00F8 F6F5 F3F1"            /* .„ÿ.)ÿÿþý..øöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 0384 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"26FF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* &ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 0284 FF18 24FF FFFE"            /* èçåãâàßœ.„ÿ.$ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 0184 FF18 2220 1F1D 1B1A 1816"            /* âàßœ.„ÿ." ...... */
	$"1513 1110 0E0D 0B0A 0807 0605 0403 0201"            /* .......Â........ */
	$"0084 FF12 5E5D 5C5B 5A59 5857 5654 5351"            /* .„ÿ.^]\[ZYXWVTSQ */
	$"504E 4D4B 4948 468A FF13 5DFF FFFE FDFB"            /* PNMKIHFŠÿ.]ÿÿþýû */
	$"FAF8 F6F5 F3F1 EFEE ECEA E8E7 4442 86FF"            /* úøöõóñïîìêèçDB†ÿ */
	$"17B7 7069 666D 666E B6FB FAF8 F6F5 F3F1"            /* .·pifmfn¶ûúøöõóñ */
	$"EFEE ECEA E8E7 4259 4283 FF1A EC54 3469"            /* ïîìêèçBYBƒÿ.ìT4i */
	$"4423 2447 6B2E 54FA F8F6 F5F3 F1EF EEEC"            /* D#$Gk.Túøöõóñïîì */
	$"EAE8 E740 B359 3381 FF04 EC37 1C78 0F81"            /* êèç@³Y3ÿ.ì7.x. */
	$"0013 147B 1539 F8F6 F5F3 F1EF EEEC EAE8"            /* ...{.9øöõóñïîìêè */
	$"E73D DEB3 5933 80FF 035C 0078 1B83 0018"            /* ç=Þ³Y3€ÿ.\.x.ƒ.. */
	$"2472 005C F6F5 F3F1 EFEE ECEA E8E7 3BEE"            /* $r.\öõóñïîìêèç;î */
	$"DEB3 5933 FFC1 001E 6E85 0017 7316 00C0"            /* Þ³Y3ÿÁ..n…..s..À */
	$"F5F3 F1EF EEEC EAE8 E739 3736 3432 312F"            /* õóñïîìêèç976421/ */
	$"6700 3463 8500 0D6D 2800 66F5 F3F1 EFEE"            /* g.4c…..m(.fõóñïî */
	$"ECEA E8E7 9C82 7504 2D2D 0027 7185 000D"            /* ìêèçœ‚u.--.'q….. */
	$"771D 002C F5F3 F1EF EEEC EAE8 E7E5 829C"            /* w..,õóñïîìêèçå‚œ */
	$"052B 1600 0787 1583 001A 1D84 0400 15F5"            /* .+...‡.ƒ...„...õ */
	$"F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C 2916"            /* óñïîìêèçåãâàßœ). */
	$"0000 4386 0581 0016 0A8A 3900 0015 F5F3"            /* ..C†...ÂŠ9...õó */
	$"F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C27 2D80"            /* ñïîìêèçåãâàßœ'-€ */
	$"0007 578D 4714 154B 8F4E 8000 112C F5F3"            /* ..WG..KN€..,õó */
	$"F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C48 6781"            /* ñïîìêèçåãâàßœHg */
	$"0005 2768 B1AC 6622 8100 1166 F5F3 F1EF"            /* ..'h±¬f"..fõóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C23 C183 0001"            /* îìêèçåãâàßœ#Áƒ.. */
	$"7E72 8300 12C0 F5F3 F1EF EEEC EAE8 E7E5"            /* ~rƒ..Àõóñïîìêèçå */
	$"E3E2 E0DF 9C21 FF5D 8200 017E 7082 0014"            /* ãâàßœ!ÿ]‚..~p‚.. */
	$"5DF6 F5F3 F1EF 00EC EAE8 E7E5 E3E2 E0DF"            /* ]öõóñï.ìêèçåãâàß */
	$"9C1F FFEC 3D81 0001 7E71 8100 163D F8F6"            /* œ.ÿì=..~q..=øö */
	$"F5F3 F1EF 00EC EAE8 E7E5 E3E2 E0DF 9C1C"            /* õóñï.ìêèçåãâàßœ. */
	$"FFFF EB5F 8000 017E 7180 0013 5FFA F8F6"            /* ÿÿë_€..~q€.._úøö */
	$"F5F3 F1EF 00EC EAE8 E7E5 00E2 E0DF 9C1C"            /* õóñï.ìêèçå.âàßœ. */
	$"81FF 1BBF 6021 807B 2766 BEFB FA00 F6F5"            /* ÿ.¿`!€{'f¾ûú.öõ */
	$"F3F1 EF00 ECEA E8E7 E500 E2E0 DF9C 1884"            /* óñï.ìêèçå.âàßœ.„ */
	$"FF18 3DFF FFFE FDFB FA00 F6F5 F3F1 EF00"            /* ÿ.=ÿÿþýûú.öõóñï. */
	$"ECEA E8E7 E500 E2E0 DF9C 1684 FF18 3BFF"            /* ìêèçå.âàßœ.„ÿ.;ÿ */
	$"FFFE FDFB FA00 F6F5 F3F1 EF00 ECEA E8E7"            /* ÿþýûú.öõóñï.ìêèç */
	$"E500 E2E0 DF9C 1484 FF18 39FF FFFE FDFB"            /* å.âàßœ.„ÿ.9ÿÿþýû */
	$"FA00 F6F5 F3F1 EF00 ECEA E8E7 E500 E2E0"            /* ú.öõóñï.ìêèçå.âà */
	$"DF9C 1284 FF01 37FF 8280 0000 8280 0000"            /* ßœ.„ÿ.7ÿ‚€..‚€.. */
	$"8280 0000 8080 019C 1084 FF0A 35FF FFFE"            /* ‚€..€€.œ.„ÿÂ5ÿÿþ */
	$"FDFB FA00 F6F5 F380 000A ECEA E8E7 E500"            /* ýûú.öõó€.Âìêèçå. */
	$"E2E0 DF9C 0E84 FF09 33FF FFFE FDFB FA00"            /* âàßœ.„ÿÆ3ÿÿþýûú. */
	$"F6F5 8100 0AEC EAE8 E7E5 00E2 E0DF 9C0C"            /* öõ.Âìêèçå.âàßœ. */
	$"84FF 0131 FF82 8002 0080 8081 0080 8080"            /* „ÿ.1ÿ‚€..€€.€€€ */
	$"0080 8001 9C0A 84FF 042F FFFF FEFD 8000"            /* .€€.œÂ„ÿ./ÿÿþý€. */
	$"07F6 F5F3 0000 EEEC EA81 0004 E2E0 DF9C"            /* .öõó..îìê..âàßœ */
	$"0884 FF03 2DFF FFFE 8100 07F6 F5F3 F1EF"            /* .„ÿ.-ÿÿþ..öõóñï */
	$"EEEC EA81 0004 E2E0 DF9C 0684 FF03 2BFF"            /* îìê..âàßœ.„ÿ.+ÿ */
	$"8080 8100 8680 0100 0081 8001 9C05 84FF"            /* €€.†€...€.œ.„ÿ */
	$"1829 FFFF FEFD 0000 F8F6 F5F3 F1EF EEEC"            /* .)ÿÿþý..øöõóñïîì */
	$"EAE8 E7E5 E3E2 E0DF 9C03 84FF 1826 FFFF"            /* êèçåãâàßœ.„ÿ.&ÿÿ */
	$"FEFD FBFA F8F6 F5F3 F1EF EEEC EAE8 E7E5"            /* þýûúøöõóñïîìêèçå */
	$"E3E2 E0DF 9C02 84FF 1824 FFFF FEFD FBFA"            /* ãâàßœ.„ÿ.$ÿÿþýûú */
	$"F8F6 F5F3 F1EF EEEC EAE8 E7E5 E3E2 E0DF"            /* øöõóñïîìêèçåãâàß */
	$"9C01 84FF 1822 201F 1D1B 1A18 1615 1311"            /* œ.„ÿ." ......... */
	$"100E 0D0B 0A08 0706 0504 0302 0100 6C38"            /* ....Â.........l8 */
	$"6D6B 0000 0408 0000 0000 0000 00FF FFFF"            /* mk...........ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 0000 00FF FFFF"            /* .............ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 0000 00FF FFFF FFFF"            /* ÿ..........ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 0000 0000 0000 00FF FFFF FFFF FFFF"            /* ÿÿ.......ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF00 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿÿÿ.....ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 00FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ...ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 00FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿ..ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 00FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿ.ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 00FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿ.ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿ..ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿ...ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿ....ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 FFFF FFFF"            /* ÿÿÿÿÿÿ......ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 00FF FFFF"            /* ÿÿÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 00FF FFFF"            /* ÿÿÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 00FF FFFF"            /* ÿÿÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 00FF FFFF"            /* ÿÿÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 00FF FFFF"            /* ÿÿÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 00FF FFFF"            /* ÿÿÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 00FF FFFF"            /* ÿÿÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 00FF FFFF"            /* ÿÿÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 00FF FFFF"            /* ÿÿÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 00FF FFFF"            /* ÿÿÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 00FF FFFF"            /* ÿÿÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 00FF FFFF"            /* ÿÿÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 00FF FFFF"            /* ÿÿÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 00FF FFFF"            /* ÿÿÿÿÿÿ.......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF"                                     /* ÿÿÿÿÿÿ */
};

data 'icns' (133, "Saved Game Icon") {
	$"6963 6E73 0000 15D5 4943 4E23 0000 0108"            /* icns...ÕICN#.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0FFF FE00 0FFF FF00 0FFF FF80 0FFF FFC0"            /* .ÿþ..ÿÿ..ÿÿ€.ÿÿÀ */
	$"0FFF FFE0 0FFF FFF0 0FFF FFF8 0FFF FFF8"            /* .ÿÿà.ÿÿð.ÿÿø.ÿÿø */
	$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"            /* .ÿÿø.ÿÿø.ÿÿø.ÿÿø */
	$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"            /* .ÿÿø.ÿÿø.ÿÿø.ÿÿø */
	$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"            /* .ÿÿø.ÿÿø.ÿÿø.ÿÿø */
	$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"            /* .ÿÿø.ÿÿø.ÿÿø.ÿÿø */
	$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"            /* .ÿÿø.ÿÿø.ÿÿø.ÿÿø */
	$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"            /* .ÿÿø.ÿÿø.ÿÿø.ÿÿø */
	$"6963 6C34 0000 0208 0000 DEED EEDE EEEE"            /* icl4......ÞíîÞîî */
	$"EEEE EEE0 0000 0000 0000 E000 0000 0000"            /* îîîà......à..... */
	$"0000 00EE 0000 0000 0000 D000 0000 0C0C"            /* ...î......Ð..... */
	$"000C 00AE E000 0000 0000 E000 0000 0C00"            /* ...®à.....à..... */
	$"00C0 C0EC EE00 0000 0000 E000 CCCC CCCC"            /* .ÀÀìî.....à.ÌÌÌÌ */
	$"CCCC 0CE0 CEE0 0000 0000 E000 C000 0C0C"            /* ÌÌ.àÎà....à.À... */
	$"00C0 C0EC 0CEE 0000 0000 D000 00C0 0C00"            /* .ÀÀì.î....Ð..À.. */
	$"00CC 00FE EEEA F000 0000 E000 0000 0CC0"            /* .Ì.þîêð...à....À */
	$"0CC0 0CCD DEDD E000 0000 E000 C000 000C"            /* .À.ÍÞÝà...à.À... */
	$"00C0 C000 0DDD E000 0000 E00C C00C CDED"            /* .ÀÀ..Ýà...à.À.Íí */
	$"DEDC 0CC0 CC0C F000 0000 D000 000E FD8E"            /* ÞÜ.ÀÌ.ð...Ð...ýŽ */
	$"8BDF E00C 0CCC F000 0000 A000 00EF C8B8"            /* ‹ßà..Ìð... ..ïÈ¸ */
	$"999D FEC0 0C0D E000 0000 E000 0EFD 8888"            /* ™þÀ..à...à..ýˆˆ */
	$"8AA9 DFDC 0CCC F000 0000 E000 CFAD 8818"            /* Š©ßÜ.Ìð...à.Ï­ˆ. */
	$"B99F DAFC 0C0D A000 0000 E000 D99D 8988"            /* ¹ŸÚü.. ...à.Ù‰ˆ */
	$"99AF D9FD C0CC F000 0000 E000 99ED A89A"            /* ™¯ÙýÀÌð...à.™í¨š */
	$"9A9F D999 C0CC F000 0000 E000 9A9D 9A99"            /* šŸÙ™ÀÌð...à.šš™ */
	$"99FE D9AE 0C0D F000 0000 E000 8A98 CF9F"            /* ™þÙ®..ð...à.Š˜ÏŸ */
	$"AFFD 8A99 0CCC F000 0000 E000 89A8 BDEE"            /* ¯ýŠ™.Ìð...à.‰¨½î */
	$"FECB 88A9 00CD E000 0000 E000 B988 98DC"            /* þËˆ©.Íà...à.¹ˆ˜Ü */
	$"CDB8 989D CC0C F000 0000 E000 C8A8 889B"            /* Í¸˜Ì.ð...à.È¨ˆ› */
	$"C889 8A9C 0CCC F000 0000 E000 0B88 8A8C"            /* È‰Šœ.Ìð...à..ˆŠŒ */
	$"B888 89B0 000D F000 0000 E000 00B8 888B"            /* ¸ˆ‰°..ð...à..¸ˆ‹ */
	$"D888 AE00 CCCC F000 0000 E000 000B 8887"            /* Øˆ®.ÌÌð...à...ˆ‡ */
	$"1889 B00C 0CCC F000 0000 E000 0000 CBBB"            /* .‰°..Ìð...à...Ë» */
	$"CEBC 0C0C 0C0D F000 0000 A000 0000 0000"            /* Î¼....ð... ..... */
	$"C000 C0C0 0CCC F000 0000 E000 C000 0C00"            /* À.ÀÀ.Ìð...à.À... */
	$"00C0 0C0C 0C0D F000 0000 F00C CC00 CCCC"            /* .À....ð...ð.Ì.ÌÌ */
	$"CCCC 00C0 0CCC F000 0000 E000 C000 0C00"            /* ÌÌ.À.Ìð...à.À... */
	$"C0C0 C00C C0CD F000 0000 E000 000C 0C0C"            /* ÀÀÀ.ÀÍð...à..... */
	$"00CC 00C0 C0CC F000 0000 A000 0000 00C0"            /* .Ì.ÀÀÌð... ....À */
	$"0C00 0C0C 0C0D F000 0000 EFEF FAFE FFFF"            /* ......ð...ïïúþÿÿ */
	$"AFFF FFFF FFFF F000 6963 6C38 0000 0408"            /* ¯ÿÿÿÿÿð.icl8.... */
	$"0000 0000 81FB FBFB FBFB FBFB FCFB FCFC"            /* ....ûûûûûûûüûüü */
	$"FCFB FCFC FCFC FC00 0000 0000 0000 0000"            /* üûüüüüü......... */
	$"0000 0000 FB00 0000 0000 0000 0000 F500"            /* ....û.........õ. */
	$"F5F5 00F5 F5F5 FCFC 0000 0000 0000 0000"            /* õõ.õõõüü........ */
	$"0000 0000 8100 0000 0000 F500 00F7 F5F5"            /* .........õ..÷õõ */
	$"F500 F5F5 F5F5 ACFB FC00 0000 0000 0000"            /* õ.õõõõ¬ûü....... */
	$"0000 0000 FB00 0000 0000 0000 00F7 0000"            /* ....û........÷.. */
	$"0000 F7F5 F6F5 FCF7 FBAC 0000 0000 0000"            /* ..÷õöõü÷û¬...... */
	$"0000 0000 FB00 0000 F7F7 F7F7 F72B F7F7"            /* ....û...÷÷÷÷÷+÷÷ */
	$"F7F7 F7F7 F5F6 ACF5 F7FB AC00 0000 0000"            /* ÷÷÷÷õö¬õ÷û¬..... */
	$"0000 0000 FB00 0000 F700 0000 00F7 F5F5"            /* ....û...÷....÷õõ */
	$"0000 F7F5 F5F5 ACF5 F6F7 FBAC 0000 0000"            /* ..÷õõõ¬õö÷û¬.... */
	$"0000 0000 FB00 0000 0000 F500 00F7 00F5"            /* ....û.....õ..÷.õ */
	$"F500 F7F5 F6F5 ACAC ACAC ACAC FD00 0000"            /* õ.÷õöõ¬¬¬¬¬¬ý... */
	$"0000 0000 FB00 0000 0000 00F5 00F7 F5F5"            /* ....û......õ.÷õõ */
	$"F5F5 F7F5 F5F5 56FA FAFA FAFA AC00 0000"            /* õõ÷õõõVúúúúú¬... */
	$"0000 0000 FB00 0000 F7F5 0000 F500 F5F5"            /* ....û...÷õ..õ.õõ */
	$"F5F5 F5F5 F6F5 F500 0056 5656 FD00 0000"            /* õõõõöõõ..VVVý... */
	$"0000 0000 FB00 00F7 F700 00F5 F781 81FA"            /* ....û..÷÷..õ÷ú */
	$"8181 81F8 F5F6 F6F5 F7F7 F6F8 FD00 0000"            /* øõööõ÷÷öøý... */
	$"0000 0000 FB00 0000 0000 F5FB FD7B 7CA0"            /* ....û.....õûý{|  */
	$"A082 FAFD FBF5 F5F6 00F7 F656 FD00 0000"            /*  ‚úýûõõö.÷öVý... */
	$"0000 0000 FC00 0000 0000 ACFE 569B A1A1"            /* ....ü.....¬þV›¡¡ */
	$"A7E7 E7F9 FEAC F6F6 002B F6F8 FD00 0000"            /* §ççùþ¬öö.+öøý... */
	$"0000 0000 FB00 0000 00FB EA57 9BA1 9BA1"            /* ....û....ûêW›¡›¡ */
	$"A1A7 ADFD 7BFF 81F5 F6F7 F656 FD00 0000"            /* ¡§­ý{ÿõö÷öVý... */
	$"0000 0000 FC00 0000 F7E9 AD7A A1A1 77A1"            /* ....ü...÷é­z¡¡w¡ */
	$"A1A7 E7D1 F9AD EAF7 F5F6 F656 FD00 0000"            /* ¡§çÑù­ê÷õööVý... */
	$"0000 0000 FC00 0000 7BE9 A657 E6A1 A1A1"            /* ....ü...{é¦Wæ¡¡¡ */
	$"A7CB ADEA FAA6 D181 F6F5 F656 FD00 0000"            /* §Ë­êú¦ÑöõöVý... */
	$"0000 0000 FC00 0000 A6E7 AC7B A7A1 A7A7"            /* ....ü...¦ç¬{§¡§§ */
	$"E7A7 D1FF F9A7 D1A6 F6F6 F6F8 FE00 0000"            /* ç§Ñÿù§Ñ¦öööøþ... */
	$"0000 0000 FC00 0000 A7A7 E856 ADE7 A7E7"            /* ....ü...§§èV­ç§ç */
	$"A7E9 E0FD F9CB A7AD 00F6 F656 FE00 0000"            /* §éàýùË§­.ööVþ... */
	$"0000 0000 FC00 0000 A6A7 A7A0 56D1 E9E8"            /* ....ü...¦§§ VÑéè */
	$"E0EA FF56 A0A7 A7E8 F5F7 F6F8 FE00 0000"            /* àêÿV §§èõ÷öøþ... */
	$"0000 0000 FC00 0000 A1A7 A7CB 7B56 FCFD"            /* ....ü...¡§§Ë{Vüý */
	$"FEFB 567C A7E6 A7A6 F5F6 F656 FE00 0000"            /* þûV|§æ§¦õööVþ... */
	$"0000 0000 FC00 0000 57E6 A1A1 E67C 7BF7"            /* ....ü...Wæ¡¡æ|{÷ */
	$"2C75 7CE6 A1A1 CB81 F6F6 F6F8 FE00 0000"            /* ,u|æ¡¡Ëöööøþ... */
	$"0000 0000 FC00 0000 2BA1 A1A1 A1A1 E657"            /* ....ü...+¡¡¡¡¡æW */
	$"50E6 A1A1 A1A1 AD2B F6F6 F656 FE00 0000"            /* Pæ¡¡¡¡­+öööVþ... */
	$"0000 0000 AC00 0000 007B A1A1 A1A1 A17A"            /* ....¬....{¡¡¡¡¡z */
	$"51A1 A19B A1CB 8100 00F6 F556 FE00 0000"            /* Q¡¡›¡Ë..öõVþ... */
	$"0000 0000 FC00 0000 0000 7CA1 9B9B A157"            /* ....ü.....|¡››¡W */
	$"519B 9BA1 A7A6 F5F5 F7F7 F656 FE00 0000"            /* Q››¡§¦õõ÷÷öVþ... */
	$"0000 0000 AC00 0000 0000 007B A1A1 A17A"            /* ....¬......{¡¡¡z */
	$"4BA1 A1A7 81F5 F5F6 00F7 F6F8 FF00 0000"            /* K¡¡§õõö.÷öøÿ... */
	$"0000 0000 FC00 0000 00F5 0000 F77B A657"            /* ....ü....õ..÷{¦W */
	$"56A6 81F7 24F6 F5F6 002B F656 FF00 0000"            /* V¦÷$öõö.+öVÿ... */
	$"0000 0000 AC00 0000 0000 F500 0000 F500"            /* ....¬.....õ...õ. */
	$"F5F5 0024 07F5 F6F5 00F7 F6F8 FF00 0000"            /* õõ.$.õöõ.÷öøÿ... */
	$"0000 0000 AC00 0000 F700 0000 00F7 0000"            /* ....¬...÷....÷.. */
	$"0000 F601 24F6 F5F6 00F7 F656 FF00 0000"            /* ..ö.$öõö.÷öVÿ... */
	$"0000 0000 AC00 00F7 F7F5 00F5 F72B F7F7"            /* ....¬..÷÷õ.õ÷+÷÷ */
	$"F7F7 F7F5 06F5 F6F6 F5F7 F6F8 FF00 0000"            /* ÷÷÷õ.õööõ÷öøÿ... */
	$"0000 0000 AC00 0000 F700 F500 00F7 00F5"            /* ....¬...÷.õ..÷.õ */
	$"F500 F7F5 F6F5 F5F6 F6F5 F656 FF00 0000"            /* õ.÷õöõõööõöVÿ... */
	$"0000 0000 AC00 0000 0000 00F5 00F7 F5F5"            /* ....¬......õ.÷õõ */
	$"F5F5 F7F6 F5F5 F6F5 F6F6 F656 FF00 0000"            /* õõ÷öõõöõöööVÿ... */
	$"0000 0000 AC00 0000 0000 F500 F500 F5F5"            /* ....¬.....õ.õ.õõ */
	$"F5F5 F5F5 F5F6 F5F6 F5F6 F656 FF00 0000"            /* õõõõõöõöõööVÿ... */
	$"0000 0000 ACAC FDFD FDFD FDFD FDFE FEFE"            /* ....¬¬ýýýýýýýþþþ */
	$"FDEA E0FF FFFF FFFF FFFF FFFF FF00 0000"            /* ýêàÿÿÿÿÿÿÿÿÿÿ... */
	$"696C 3332 0000 09E5 81FF 1260 5E5D 5A58"            /* il32..Æåÿ.`^]ZX */
	$"5653 514F 4D4B 4846 4E4D 4B49 4846 8AFF"            /* VSQOMKHFNMKIHFŠÿ */
	$"135F FFFF FDFC FAF9 F7FF F3F3 F0EF EEEC"            /* ._ÿÿýüúù÷ÿóóðïîì */
	$"EAE8 E744 4289 FF14 5DFF FFFD FCFA F8F7"            /* êèçDB‰ÿ.]ÿÿýüúø÷ */
	$"FFBF F2F0 EFFF ECEA E8E7 4259 4288 FF00"            /* ÿ¿òðïÿìêèçBYBˆÿ. */
	$"5B85 FF00 BF81 FF07 BFEA E8E7 40B3 5933"            /* […ÿ.¿ÿ.¿êèç@³Y3 */
	$"87FF 0059 80FF 89BF 06E8 E73D DEB3 5933"            /* ‡ÿ.Y€ÿ‰¿.èç=Þ³Y3 */
	$"86FF 1758 FFFF FEBF FAF8 F7FF BFF2 F1EF"            /* †ÿ.Xÿÿþ¿úø÷ÿ¿òñï */
	$"FFBF EAE8 E73B EEDE B359 3385 FF18 57FF"            /* ÿ¿êèç;îÞ³Y3…ÿ.Wÿ */
	$"FFFD FCFB F8F7 FFBF F3F1 EFFF BFEA E8E7"            /* ÿýüûø÷ÿ¿óñïÿ¿êèç */
	$"3937 3634 3231 2F84 FF00 5580 FF0E FBFB"            /* 976421/„ÿ.U€ÿ.ûû */
	$"F9F7 F5BF F2F1 EFEE BFEA E8E7 9C82 7500"            /* ù÷õ¿òñïî¿êèçœ‚u. */
	$"2D84 FF00 5480 FF10 BFFA F9F7 F6F3 F2F0"            /* -„ÿ.T€ÿ.¿úù÷öóòð */
	$"EFEE ECEA E8E7 E5FF FF80 9C00 2B84 FF18"            /* ïîìêèçåÿÿ€œ.+„ÿ. */
	$"52FF FFBF BFFA F9F7 B66D 6870 6E66 6DB6"            /* Rÿÿ¿¿úù÷¶mhpnfm¶ */
	$"E8E7 E5E3 BFBF DF9C 2984 FF18 50FF FFFE"            /* èçåã¿¿ßœ)„ÿ.Pÿÿþ */
	$"FCFB F854 3277 5E47 4152 6F2E 54E7 E5E3"            /* üûøT2w^GARo.Tçåã */
	$"FFBF DF9C 2784 FF18 4FFF FFFD FCFA 391A"            /* ÿ¿ßœ'„ÿ.Oÿÿýüú9. */
	$"8B3D 2F30 2615 1E7E 1639 E5E3 FFBF DF9C"            /* ‹=/0&..~.9åãÿ¿ßœ */
	$"2584 FF18 4DFF FFFE FC61 007F 4937 413A"            /* %„ÿ.Mÿÿþüa..I7A: */
	$"2F24 102B 7600 5FE3 E2BF DF9C 2384 FF18"            /* /$.+v._ãâ¿ßœ#„ÿ. */
	$"4CFF FFFE C403 2481 293C 3E39 2E23 1904"            /* LÿÿþÄ.$)<>9.#.. */
	$"7623 02C1 E2E0 DF9C 2184 FF18 4BFF FFFD"            /* v#.Áâàßœ!„ÿ.Kÿÿý */
	$"6F00 3B78 2133 3531 281F 1600 6E3A 0068"            /* o.;x!351(...n:.h */
	$"E2E0 DF9C 1F84 FF18 49FF FFFE 3D0F 357D"            /* âàßœ.„ÿ.Iÿÿþ=.5} */
	$"1828 2927 211A 1100 7934 1034 E2E0 DF9C"            /* .()'!...y4.4âàßœ */
	$"1C84 FF18 47FF FFFD 301D 1A8C 2617 1E1D"            /* .„ÿ.Gÿÿý0..Œ&... */
	$"1913 031D 8C1B 1D24 FFE0 DF9C 1A84 FF18"            /* ....Œ..$ÿàßœ.„ÿ. */
	$"46FF FFFD 3423 1953 880A 090E 0B00 078C"            /* Fÿÿý4#.SˆÂÆ....Œ */
	$"501A 2326 E2BF DF9C 1884 FF18 44FF FFFD"            /* P.#&â¿ßœ.„ÿ.Dÿÿý */
	$"4723 281C 6890 4517 1548 9466 1D29 2236"            /* G#(.hE..H”f.)"6 */
	$"E2E0 DF9C 1684 FF18 43FF FFFD 7B20 2E2E"            /* âàßœ.„ÿ.Cÿÿý{ .. */
	$"2248 7AB4 B57C 4623 2D2F 1769 E2E0 DF9C"            /* "Hz´µ|F#-/.iâàßœ */
	$"1484 FF18 41FF FFFD C828 3233 332F 1388"            /* .„ÿ.AÿÿýÈ(233/.ˆ */
	$"8D1C 2E33 3431 12C0 E2E0 DF9C 1284 FF18"            /* ..341.Àâàßœ.„ÿ. */
	$"40FF FFFE FC77 2639 3839 2488 8F2C 3839"            /* @ÿÿþüw&989$ˆ,89 */
	$"3A15 62FF FFE0 DF9C 1084 FF18 3EFF FFFD"            /* :.bÿÿàßœ.„ÿ.>ÿÿý */
	$"FCFA 5B28 3C3F 2889 9332 3F3B 1843 E5E3"            /* üú[(<?(‰“2?;.Cåã */
	$"BFBF DF9C 0E84 FF18 3CFF FFFE FCFA F972"            /* ¿¿ßœ.„ÿ.<ÿÿþüúùr */
	$"2A2F 2589 942D 2619 60E7 E5E3 FFBF DF9C"            /* *.%‰”-&.`çåãÿ¿ßœ */
	$"0C84 FF18 3BFF FFFD FCFA F9F7 C275 358E"            /* .„ÿ.;ÿÿýüúù÷Âu5Ž */
	$"8C34 6BBD E8E7 E5E3 FFBF DF9C 0A84 FF00"            /* Œ4k½èçåãÿ¿ßœÂ„ÿ. */
	$"3980 FF14 FBFA F8F7 FFF4 F2F1 EFEE ECEA"            /* 9€ÿ.ûúø÷ÿôòñïîìê */
	$"E8E7 E5E3 FFBF DF9C 0884 FF00 3880 FF05"            /* èçåãÿ¿ßœ.„ÿ.8€ÿ. */
	$"BFFA F9FF FFBF 81FF 0AEC EAE8 E7E5 E3FF"            /* ¿úùÿÿ¿ÿÂìêèçåãÿ */
	$"BFDF 9C06 84FF 0737 FFFF BFBF FBF8 F684"            /* ¿ßœ.„ÿ.7ÿÿ¿¿ûøö„ */
	$"BF09 EAE8 E7E5 E3E2 BFDF 9C05 84FF 1836"            /* ¿Æêèçåãâ¿ßœ.„ÿ.6 */
	$"FFFF FEBF FAF8 F7FF BFF2 F0EF FFBF EAE8"            /* ÿÿþ¿úø÷ÿ¿òðïÿ¿êè */
	$"E7E5 E3E2 E0DF 9C03 84FF 1834 FFFF FDFC"            /* çåãâàßœ.„ÿ.4ÿÿýü */
	$"FAF9 F7F6 BFF2 F1EF EEBF EAE8 E7E5 E3E2"            /* úù÷ö¿òñïî¿êèçåãâ */
	$"E0DF 9C02 84FF 1832 FFFF FEFC FAF9 F7F5"            /* àßœ.„ÿ.2ÿÿþüúù÷õ */
	$"F3F2 F0EE EEEC EAE8 E7E5 E3E2 E0DF 9C01"            /* óòðîîìêèçåãâàßœ. */
	$"84FF 1830 2E2D 2A29 2725 2321 1F1D 1C19"            /* „ÿ.0.-*)'%#!.... */
	$"0D0B 0A08 0706 0504 0302 0100 80FF 81FF"            /* ..Â.........€ÿÿ */
	$"1261 5F5C 5A58 5553 514F 4D4B 4847 4E4D"            /* .a_\ZXUSQOMKHGNM */
	$"4B49 4846 8AFF 135F FFFF FDFC FAF9 F6FF"            /* KIHFŠÿ._ÿÿýüúùöÿ */
	$"F3F3 F0EF EEEC EAE8 E744 4289 FF14 5DFF"            /* óóðïîìêèçDB‰ÿ.]ÿ */
	$"FFFE FCFA F8F7 FFBF F2F0 EFFF ECEA E8E7"            /* ÿþüúø÷ÿ¿òðïÿìêèç */
	$"4259 4288 FF00 5C85 FF00 BF81 FF07 BFEA"            /* BYBˆÿ.\…ÿ.¿ÿ.¿ê */
	$"E8E7 40B3 5933 87FF 005A 80FF 89BF 06E8"            /* èç@³Y3‡ÿ.Z€ÿ‰¿.è */
	$"E73D DEB3 5933 86FF 1759 FFFF FDBF FAF9"            /* ç=Þ³Y3†ÿ.Yÿÿý¿úù */
	$"F7FF BFF2 F1EF FFBF EAE8 E73B EEDE B359"            /* ÷ÿ¿òñïÿ¿êèç;îÞ³Y */
	$"3385 FF18 57FF FFFD FBFA F9F7 FFBF F2F0"            /* 3…ÿ.Wÿÿýûúù÷ÿ¿òð */
	$"EFFF BFEA E8E7 3937 3634 3231 2F84 FF00"            /* ïÿ¿êèç976421/„ÿ. */
	$"5680 FF0E FCFB F8F7 F6BF F2F0 EFEE BFEA"            /* V€ÿ.üûø÷ö¿òðïî¿ê */
	$"E8E7 9C82 7500 2D84 FF00 5580 FF10 BFFA"            /* èçœ‚u.-„ÿ.U€ÿ.¿ú */
	$"F9F7 F5F4 F2F0 EFEE ECEA E8E7 E5FF FF80"            /* ù÷õôòðïîìêèçåÿÿ€ */
	$"9C00 2B84 FF18 53FF FFBF BFFA F9F7 B569"            /* œ.+„ÿ.Sÿÿ¿¿úù÷µi */
	$"6674 7165 6CB6 E8E7 E5E3 BFBF DF9C 2984"            /* ftqel¶èçåã¿¿ßœ)„ */
	$"FF18 52FF FFFE FCFA F955 2F94 A693 8079"            /* ÿ.RÿÿþüúùU/”¦“€y */
	$"782D 55E7 E5E3 FFBF DF9C 2784 FF18 50FF"            /* x-Uçåãÿ¿ßœ'„ÿ.Pÿ */
	$"FFFD FCFA 3E19 B6AD A598 7C58 4886 1B3D"            /* ÿýüú>.¶­¥˜|XH†.= */
	$"E5E3 FFBF DF9C 2584 FF18 4EFF FFFD FC6F"            /* åãÿ¿ßœ%„ÿ.Nÿÿýüo */
	$"0090 ABB2 BEAC 8A69 423E 7E06 67E3 E2BF"            /* .«²¾¬ŠiB>~.gãâ¿ */
	$"DF9C 2384 FF18 4DFF FFFD CB22 33A8 91B1"            /* ßœ#„ÿ.MÿÿýË"3¨‘± */
	$"B8A7 8767 4A1C 7B42 19C4 E2E0 DF9C 2184"            /* ¸§‡gJ.{B.Äâàßœ!„ */
	$"FF18 4BFF FFFE 872A 4F9F 7B97 9C8F 765D"            /* ÿ.Kÿÿþ‡*OŸ{—œv] */
	$"4212 735F 2875 E2E0 DF9C 1F84 FF18 49FF"            /* B.s_(uâàßœ.„ÿ.Iÿ */
	$"FFFD 6644 5398 5F76 7971 614C 3308 7E64"            /* ÿýfDS˜_vyqaL3.~d */
	$"454C E2E0 DF9C 1C84 FF18 48FF FFFD 655A"            /* ELâàßœ.„ÿ.HÿÿýeZ */
	$"4E9C 5253 5B55 4936 161B A05A 5A44 FFE0"            /* NœRS[UI6.. ZZDÿà */
	$"DF9C 1A84 FF18 47FF FFFD 7068 5F7C 922A"            /* ßœ.„ÿ.Gÿÿýph_|’* */
	$"2E32 270F 038D 8B62 6846 E2BF DF9C 1884"            /* .2'..‹bhFâ¿ßœ.„ */
	$"FF18 45FF FFFE 8674 786B 9797 4521 1941"            /* ÿ.Eÿÿþ†txk——E!.A */
	$"9CA1 7078 6F53 E2E0 DF9C 1684 FF18 43FF"            /* œ¡pxoSâàßœ.„ÿ.Cÿ */
	$"FFFD AC7E 8687 7E96 A6B8 C8B4 9980 868B"            /* ÿý¬~†‡~–¦¸È´™€†‹ */
	$"6477 E2E0 DF9C 1484 FF18 42FF FFFD DC8B"            /* dwâàßœ.„ÿ.BÿÿýÜ‹ */
	$"9897 9898 739B C88B 9497 9997 48C0 E2E0"            /* ˜—˜˜s›È‹”—™—HÀâà */
	$"DF9C 1284 FF18 40FF FFFD FCAF 99A9 A6A9"            /* ßœ.„ÿ.@ÿÿýü¯™©¦© */
	$"879C CEA1 A6A8 AC66 6FFF FFE0 DF9C 1084"            /* ‡œÎ¡¦¨¬foÿÿàßœ.„ */
	$"FF18 3FFF FFFD FCFA A1A1 B7BA 949F D6B1"            /* ÿ.?ÿÿýüú¡¡·º”ŸÖ± */
	$"B9B4 705A E5E3 BFBF DF9C 0E84 FF18 3DFF"            /* ¹´pZåã¿¿ßœ.„ÿ.=ÿ */
	$"FFFD FCFB F9A6 91AC 97A1 DBAF 915B 71E7"            /* ÿýüûù¦‘¬—¡Û¯‘[qç */
	$"E5E3 FFBF DF9C 0C84 FF18 3BFF FFFD FCFA"            /* åãÿ¿ßœ.„ÿ.;ÿÿýüú */
	$"F8F7 CE9E 6B9A AF66 80BE E8E7 E5E3 FFBF"            /* ø÷Îžkš¯f€¾èçåãÿ¿ */
	$"DF9C 0A84 FF00 3A80 FF14 FCFA F8F7 FFF3"            /* ßœÂ„ÿ.:€ÿ.üúø÷ÿó */
	$"F2F0 EEEE ECEA E8E7 E5E3 FFBF DF9C 0884"            /* òðîîìêèçåãÿ¿ßœ.„ */
	$"FF00 3880 FF05 BFFB F9FF FFBF 81FF 0AEC"            /* ÿ.8€ÿ.¿ûùÿÿ¿ÿÂì */
	$"EAE8 E7E5 E3FF BFDF 9C06 84FF 0737 FFFF"            /* êèçåãÿ¿ßœ.„ÿ.7ÿÿ */
	$"BFBF FAF9 F684 BF09 EAE8 E7E5 E3E2 BFDF"            /* ¿¿úùö„¿Æêèçåãâ¿ß */
	$"9C05 84FF 1835 FFFF FDBF FAF8 F7FF BFF2"            /* œ.„ÿ.5ÿÿý¿úø÷ÿ¿ò */
	$"F0EF FFBF EAE8 E7E5 E3E2 E0DF 9C03 84FF"            /* ðïÿ¿êèçåãâàßœ.„ÿ */
	$"1834 FFFF FDFC FAF8 F7F5 BFF2 F1EE EEBF"            /* .4ÿÿýüúø÷õ¿òñîî¿ */
	$"EAE8 E7E5 E3E2 E0DF 9C02 84FF 1832 FFFF"            /* êèçåãâàßœ.„ÿ.2ÿÿ */
	$"FDFC FAF8 F7F5 F3F2 F1EF EEEC EAE8 E7E5"            /* ýüúø÷õóòñïîìêèçå */
	$"E3E2 E0DF 9C01 84FF 1831 302D 2B2A 2826"            /* ãâàßœ.„ÿ.10-+*(& */
	$"2422 201E 1D1B 0D0B 0A08 0706 0504 0302"            /* $" .....Â....... */
	$"0100 80FF 81FF 1261 5F5C 5A58 5653 524F"            /* ..€ÿÿ.a_\ZXVSRO */
	$"4D4B 4847 4E4D 4B49 4846 8AFF 135F FFFF"            /* MKHGNMKIHFŠÿ._ÿÿ */
	$"FDFC FBF9 F6FF F4F2 F0EF EEEC EAE8 E744"            /* ýüûùöÿôòðïîìêèçD */
	$"4289 FF14 5EFF FFFD FCFA F9F7 FFBF F2F0"            /* B‰ÿ.^ÿÿýüúù÷ÿ¿òð */
	$"EFFF ECEA E8E7 4259 4288 FF00 5D85 FF00"            /* ïÿìêèçBYBˆÿ.]…ÿ. */
	$"BF81 FF07 BFEA E8E7 40B3 5933 87FF 005B"            /* ¿ÿ.¿êèç@³Y3‡ÿ.[ */
	$"80FF 89BF 06E8 E73D DEB3 5933 86FF 1759"            /* €ÿ‰¿.èç=Þ³Y3†ÿ.Y */
	$"FFFF FEBF FAF8 F7FF BFF2 F1EF FFBF EAE8"            /* ÿÿþ¿úø÷ÿ¿òñïÿ¿êè */
	$"E73B EEDE B359 3385 FF18 58FF FFFD FCFA"            /* ç;îÞ³Y3…ÿ.Xÿÿýüú */
	$"F9F7 FFBF F3F0 EFFF BFEA E8E7 3937 3634"            /* ù÷ÿ¿óðïÿ¿êèç9764 */
	$"3231 2F84 FF00 5680 FF0E FCFA F9F7 F6BF"            /* 21/„ÿ.V€ÿ.üúù÷ö¿ */
	$"F2F1 EEEE BFEA E8E7 9C82 7500 2D84 FF00"            /* òñîî¿êèçœ‚u.-„ÿ. */
	$"5480 FF10 BFFB F8F7 F6F4 F2F0 EFEE ECEA"            /* T€ÿ.¿ûø÷öôòðïîìê */
	$"E8E7 E5FF FF80 9C00 2B84 FF18 53FF FFBF"            /* èçåÿÿ€œ.+„ÿ.Sÿÿ¿ */
	$"BFFA F9F7 B66F 686E 6D66 6EB6 E8E7 E5E3"            /* ¿úù÷¶ohnmfn¶èçåã */
	$"BFBF DF9C 2984 FF18 52FF FFFD FBFB F854"            /* ¿¿ßœ)„ÿ.RÿÿýûûøT */
	$"3469 4423 2447 6B2E 54E7 E5E3 FFBF DF9C"            /* 4iD#$Gk.Tçåãÿ¿ßœ */
	$"2784 FF09 50FF FFFD FCFA 371C 780F 8100"            /* '„ÿÆPÿÿýüú7.x.. */
	$"0A14 7B15 39E5 E3FF BFDF 9C25 84FF 084E"            /* Â.{.9åãÿ¿ßœ%„ÿ.N */
	$"FFFF FDFC 5C00 781B 8300 0924 7200 5CE3"            /* ÿÿýü\.x.ƒ.Æ$r.\ã */
	$"E2BF DF9C 2384 FF07 4DFF FFFD C000 1E6E"            /* â¿ßœ#„ÿ.MÿÿýÀ..n */
	$"8500 0873 1600 C0E2 E0DF 9C21 84FF 074B"            /* …..s..Àâàßœ!„ÿ.K */
	$"FFFF FD66 0034 6385 0008 6D28 0066 E2E0"            /* ÿÿýf.4c…..m(.fâà */
	$"DF9C 1F84 FF07 49FF FFFD 2C00 2771 8500"            /* ßœ.„ÿ.Iÿÿý,.'q…. */
	$"0877 1D00 2CE2 E0DF 9C1C 84FF 0848 FFFF"            /* .w..,âàßœ.„ÿ.Hÿÿ */
	$"FE15 0007 8715 8300 091D 8404 0015 FFE0"            /* þ...‡.ƒ.Æ.„...ÿà */
	$"DF9C 1A84 FF09 46FF FFFD 1500 0043 8605"            /* ßœ.„ÿÆFÿÿý...C†. */
	$"8100 0A0A 8A39 0000 15E2 BFDF 9C18 84FF"            /* .ÂÂŠ9...â¿ßœ.„ÿ */
	$"0445 FFFF FE2C 8000 0757 8D47 1415 4B8F"            /* .Eÿÿþ,€..WG..K */
	$"4E80 0005 2CE2 E0DF 9C16 84FF 0443 FFFF"            /* N€..,âàßœ.„ÿ.Cÿÿ */
	$"FE66 8100 0527 68B1 AC66 2281 0005 66E2"            /* þf..'h±¬f"..fâ */
	$"E0DF 9C14 84FF 0442 FFFF FEC0 8300 017E"            /* àßœ.„ÿ.BÿÿþÀƒ..~ */
	$"7283 0005 C0E2 E0DF 9C12 84FF 0540 FFFF"            /* rƒ..Àâàßœ.„ÿ.@ÿÿ */
	$"FDFC 5D82 0001 7E70 8200 065D FFFF E0DF"            /* ýü]‚..~p‚..]ÿÿàß */
	$"9C10 84FF 063F FFFF FEFC FA3D 8100 017E"            /* œ.„ÿ.?ÿÿþüú=..~ */
	$"7181 0007 3DE5 E3BF BFDF 9C0E 84FF 073D"            /* q..=åã¿¿ßœ.„ÿ.= */
	$"FFFF FEFC FAF9 5F80 0001 7E71 8000 085F"            /* ÿÿþüúù_€..~q€.._ */
	$"E7E5 E3FF BFDF 9C0C 84FF 183B FFFF FDFC"            /* çåãÿ¿ßœ.„ÿ.;ÿÿýü */
	$"FAF8 F7BE 6627 877B 2766 BEE8 E7E5 E3FF"            /* úø÷¾f'‡{'f¾èçåãÿ */
	$"BFDF 9C0A 84FF 0039 80FF 14FC FAF8 F7FF"            /* ¿ßœÂ„ÿ.9€ÿ.üúø÷ÿ */
	$"F4F2 F1EE EEEC EAE8 E7E5 E3FF BFDF 9C08"            /* ôòñîîìêèçåãÿ¿ßœ. */
	$"84FF 0038 80FF 05BF FAF9 FFFF BF81 FF0A"            /* „ÿ.8€ÿ.¿úùÿÿ¿ÿÂ */
	$"ECEA E8E7 E5E3 FFBF DF9C 0684 FF07 37FF"            /* ìêèçåãÿ¿ßœ.„ÿ.7ÿ */
	$"FFBF BFFA F8F7 84BF 09EA E8E7 E5E3 E2BF"            /* ÿ¿¿úø÷„¿Æêèçåãâ¿ */
	$"DF9C 0584 FF18 35FF FFFD BFFA F9F7 FFBF"            /* ßœ.„ÿ.5ÿÿý¿úù÷ÿ¿ */
	$"F2F1 EEFF BFEA E8E7 E5E3 E2E0 DF9C 0384"            /* òñîÿ¿êèçåãâàßœ.„ */
	$"FF18 34FF FFFE FCFA F9F7 F5BF F2F1 EFEE"            /* ÿ.4ÿÿþüúù÷õ¿òñïî */
	$"BFEA E8E7 E5E3 E2E0 DF9C 0284 FF18 32FF"            /* ¿êèçåãâàßœ.„ÿ.2ÿ */
	$"FFFE FBFA F9F7 F5F4 F2F1 EFEE ECEA E8E7"            /* ÿþûúù÷õôòñïîìêèç */
	$"E5E3 E2E0 DF9C 0184 FF18 302F 2C2B 2827"            /* åãâàßœ.„ÿ.0/,+(' */
	$"2523 2120 1E1B 1A0D 0B0A 0807 0605 0403"            /* %#! .....Â...... */
	$"0201 0080 FF6C 386D 6B00 0004 0800 0000"            /* ...€ÿl8mk....... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 0000 0000 0000 0000 0000"            /* ÿÿÿÿ............ */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿ........... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿÿ.......... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÿ......... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿ........ */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0069 636D"            /* ÿÿÿÿÿÿÿÿÿÿ...icm */
	$"3800 0000 C800 0000 0000 0000 0000 0000"            /* 8...È........... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 00"                                       /* ..... */
};

data 'icns' (134, "Film Icon") {
	$"6963 6E73 0000 1237 4943 4E23 0000 0108"            /* icns...7ICN#.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"3FFF FF00 3FFF FF80 3FFF FFC0 3FFF FFE0"            /* ?ÿÿ.?ÿÿ€?ÿÿÀ?ÿÿà */
	$"3FFF FFF0 3FFF FFF8 3FFF FFFC 3FFF FFFC"            /* ?ÿÿð?ÿÿø?ÿÿü?ÿÿü */
	$"3FFF FFFC 3FFF FFFC 3FFF FFFC 3FFF FFFC"            /* ?ÿÿü?ÿÿü?ÿÿü?ÿÿü */
	$"3FFF FFFC 3FFF FFFC 3FFF FFFC 3FFF FFFC"            /* ?ÿÿü?ÿÿü?ÿÿü?ÿÿü */
	$"3FFF FFFC 3FFF FFFC 3FFF FFFC 3FFF FFFC"            /* ?ÿÿü?ÿÿü?ÿÿü?ÿÿü */
	$"3FFF FFFC 3FFF FFFC 3FFF FFFC 3FFF FFFC"            /* ?ÿÿü?ÿÿü?ÿÿü?ÿÿü */
	$"3FFF FFFC 3FFF FFFC 3FFF FFFC 3FFF FFFC"            /* ?ÿÿü?ÿÿü?ÿÿü?ÿÿü */
	$"3FFF FFFC 3FFF FFFC 3FFF FFFC 3FFF FFFC"            /* ?ÿÿü?ÿÿü?ÿÿü?ÿÿü */
	$"6963 6C34 0000 0208 00FF FFFF FFFF FFFF"            /* icl4.....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 0000 00FF FF0C 000C 000C"            /* ÿÿÿÿ.....ÿÿ..... */
	$"000C 000F F000 0000 00F0 0F00 CC00 CC00"            /* ....ð....ð..Ì.Ì. */
	$"CC00 CC0F DF00 0000 00F0 0F0C 000C 0C0C"            /* Ì.Ì.ß....ð...... */
	$"000C 0C0F 0DF0 0000 00FF FFC0 C0C0 C0C0"            /* .....ð...ÿÿÀÀÀÀÀ */
	$"C0C0 C0CF 0CDF 0000 00FF FF0C 0C0C 0C0C"            /* ÀÀÀÏ.ß...ÿÿ..... */
	$"0C0C 0C0F C00D F000 00F0 0F00 C0C0 C0C0"            /* ....À.ð..ð..ÀÀÀÀ */
	$"C0C0 C0CF FFFF FF00 00F0 0F0C 0C0C 000C"            /* ÀÀÀÏÿÿÿ..ð...... */
	$"0C0C 0C00 00F0 0F00 00FF FFC0 C0C0 CEDD"            /* .....ð...ÿÿÀÀÀÎÝ */
	$"DDEC 00CC 0CFF FF00 00FF FF0C 0C0E 9DB8"            /* Ýì.Ì.ÿÿ..ÿÿ...¸ */
	$"8EDE EC00 C0FF FF00 00F0 0F00 C0EF C889"            /* ŽÞì.Àÿÿ..ð..ÀïÈ‰ */
	$"A8FD FE0C 0CF0 0F00 00F0 0F0C 0DFD 8888"            /* ¨ýþ..ð...ð...ýˆˆ */
	$"8A8E DFE0 C0F0 0F00 00FF FFC0 CFAD 88B8"            /* ŠŽßàÀð...ÿÿÀÏ­ˆ¸ */
	$"89F9 DAFC 0CFF FF00 00FF FF00 D9EB 8A88"            /* ‰ùÚü.ÿÿ..ÿÿ.ÙëŠˆ */
	$"99AF D99D C0FF FF00 00F0 0FC0 9F8D 98AA"            /* ™¯ÙÀÿÿ..ð.ÀŸ˜ª */
	$"8A9F D9A9 0CF0 0F00 00F0 0F00 A99D AA98"            /* ŠŸÙ©.ð...ð..©ª˜ */
	$"F9FA D99E C0F0 0F00 00FF FF0C 8A9B D9FF"            /* ùúÙžÀð...ÿÿ.Š›Ùÿ */
	$"AFFD 89A8 0CFF FF00 00FF FFCC B988 DDEE"            /* ¯ý‰¨.ÿÿ..ÿÿÌ¹ˆÝî */
	$"FECB 9B9E C0FF FF00 00F0 0F00 D8A8 88DC"            /* þË›žÀÿÿ..ð..Ø¨ˆÜ */
	$"CD88 889D 0CF0 0F00 00F0 0FC0 CA88 988D"            /* Íˆˆ.ð...ð.ÀÊˆ˜ */
	$"B888 A9AC C0F0 0F00 00FF FF0C 088B 8A8D"            /* ¸ˆ©¬Àð...ÿÿ..‹Š */
	$"1888 89B0 0CFF FF00 00FF FFC0 C0B8 8888"            /* .ˆ‰°.ÿÿ..ÿÿÀÀ¸ˆˆ */
	$"C8B8 A900 C0FF FF00 00F0 0F0C 0C0D 88AC"            /* È¸©.Àÿÿ..ð....ˆ¬ */
	$"D888 B0C0 C0F0 0F00 00F0 0FC0 C00C 0D8C"            /* Øˆ°ÀÀð...ð.ÀÀ..Œ */
	$"B9DC 0C0C 00F0 0F00 00FF FF0C 0C0C C00C"            /* ¹Ü...ð...ÿÿ...À. */
	$"00C0 C00C 0CFF FF00 00FF FFC0 C0C0 00C0"            /* .ÀÀ..ÿÿ..ÿÿÀÀÀ.À */
	$"C00C 0C0C 00FF FF00 00F0 0F0C 0C0C 0C0C"            /* À....ÿÿ..ð...... */
	$"0C00 C0C0 C0F0 0F00 00F0 0F00 C0C0 C0C0"            /* ..ÀÀÀð...ð..ÀÀÀÀ */
	$"C00C 0C0C 00F0 0F00 00FF FFC0 0C0C 0C0C"            /* À....ð...ÿÿÀ.... */
	$"0C00 C0C0 C0FF FF00 00FF FF00 C0C0 C0C0"            /* ..ÀÀÀÿÿ..ÿÿ.ÀÀÀÀ */
	$"C0C0 0C0C 0CFF FF00 00F0 0FC0 000C 0C0C"            /* ÀÀ...ÿÿ..ð.À.... */
	$"0C0C 00C0 C0F0 0F00 00FF FFFF FFFF FFFF"            /* ...ÀÀð...ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 6963 6C38 0000 0408"            /* ÿÿÿÿÿÿÿ.icl8.... */
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿ........ */
	$"0000 FFFF FFFF F5F6 F5F5 F5F6 F5F5 F5F6"            /* ..ÿÿÿÿõöõõõöõõõö */
	$"F5F5 F5F6 F5F5 F5FF FF00 0000 0000 0000"            /* õõõöõõõÿÿ....... */
	$"0000 FF00 00FF F5F5 F6F6 F5F5 F6F6 F5F5"            /* ..ÿ..ÿõõööõõööõõ */
	$"F6F6 F5F5 F6F6 F5FF F9FF 0000 0000 0000"            /* ööõõööõÿùÿ...... */
	$"0000 FF00 00FF F5F6 F5F6 F5F6 F5F6 F5F6"            /* ..ÿ..ÿõöõöõöõöõö */
	$"F5F6 F5F6 F5F6 F5FF F6F9 FF00 0000 0000"            /* õöõöõöõÿöùÿ..... */
	$"0000 FFFF FFFF F6F5 F6F5 F6F5 F6F5 F6F5"            /* ..ÿÿÿÿöõöõöõöõöõ */
	$"F6F5 F6F5 F6F5 F6FF F5F6 F9FF 0000 0000"            /* öõöõöõöÿõöùÿ.... */
	$"0000 FFFF FFFF F5F6 F5F6 F5F6 F5F6 F5F6"            /* ..ÿÿÿÿõöõöõöõöõö */
	$"F5F6 F5F6 F5F6 F5FF F6F5 F6F9 FF00 0000"            /* õöõöõöõÿöõöùÿ... */
	$"0000 FF00 00FF F5F5 F6F5 F6F5 F6F5 F6F5"            /* ..ÿ..ÿõõöõöõöõöõ */
	$"F6F5 F6F5 F6F5 F6FF FFFF FFFF FFFF 0000"            /* öõöõöõöÿÿÿÿÿÿÿ.. */
	$"0000 FF00 00FF F5F6 F5F6 F5F6 F5F5 F5F6"            /* ..ÿ..ÿõöõöõöõõõö */
	$"F5F6 F5F6 F5F6 F5F5 F5F5 FF00 00FF 0000"            /* õöõöõöõõõõÿ..ÿ.. */
	$"0000 FFFF FFFF F6F5 F6F5 F6F5 F881 81FA"            /* ..ÿÿÿÿöõöõöõøú */
	$"8181 81F7 F5F5 F6F6 F5F6 FFFF FFFF 0000"            /* ÷õõööõöÿÿÿÿ.. */
	$"0000 FFFF FFFF F5F6 F5F6 00FC AC7B 7BA1"            /* ..ÿÿÿÿõöõö.ü¬{{¡ */
	$"A082 FAAC FCF5 F6F5 F6F5 FFFF FFFF 0000"            /*  ‚ú¬üõöõöõÿÿÿÿ.. */
	$"0000 FF00 00FF F5F5 F6F5 ACFD 51A1 A1A1"            /* ..ÿ..ÿõõöõ¬ýQ¡¡¡ */
	$"A7CB E8F9 FEAC 00F6 F5F6 FF00 00FF 0000"            /* §Ëèùþ¬.öõöÿ..ÿ.. */
	$"0000 FF00 00FF F5F6 F581 FFF9 A09B 9BA1"            /* ..ÿ..ÿõöõÿù ››¡ */
	$"A1A7 A7FD FAEA FBF5 F6F5 FF00 00FF 0000"            /* ¡§§ýúêûõöõÿ..ÿ.. */
	$"0000 FFFF FFFF F6F5 F7E9 AD7B A1A1 77A1"            /* ..ÿÿÿÿöõ÷é­{¡¡w¡ */
	$"A1E7 E8D1 F9AD E9F7 F5F6 FFFF FFFF 0000"            /* ¡çèÑù­é÷õöÿÿÿÿ.. */
	$"0000 FFFF FFFF F5F5 7BD1 FC51 A7A1 A1A1"            /* ..ÿÿÿÿõõ{ÑüQ§¡¡¡ */
	$"E6A7 ADE0 81A6 D181 F6F5 FFFF FFFF 0000"            /* æ§­à¦Ñöõÿÿÿÿ.. */
	$"0000 FF00 00FF F6F5 A6D1 A67B CBA7 A7A7"            /* ..ÿ..ÿöõ¦Ñ¦{Ë§§§ */
	$"A7E7 E8EA F9A7 E8A6 F5F6 FF00 00FF 0000"            /* §çèêù§è¦õöÿ..ÿ.. */
	$"0000 FF00 00FF F5F5 A7A7 E856 ADA7 CBA7"            /* ..ÿ..ÿõõ§§èV­§Ë§ */
	$"E8E8 E0FE 56E7 A7AD F6F5 FF00 00FF 0000"            /* èèàþVç§­öõÿ..ÿ.. */
	$"0000 FFFF FFFF F5F6 A7A7 CB82 7AE9 D1E9"            /* ..ÿÿÿÿõö§§Ë‚zéÑé */
	$"ADFF EA56 A0A7 E7A6 F5F6 FFFF FFFF 0000"            /* ­ÿêV §ç¦õöÿÿÿÿ.. */
	$"0000 FFFF FFFF F6F6 7CCB A1A7 7B56 FCFD"            /* ..ÿÿÿÿöö|Ë¡§{Vüý */
	$"EAFC F87C A7A7 A7AC F6F5 FFFF FFFF 0000"            /* êüø|§§§¬öõÿÿÿÿ.. */
	$"0000 FF00 00FF F5F5 7BA1 A7A1 A1A0 57F7"            /* ..ÿ..ÿõõ{¡§¡¡ W÷ */
	$"F775 A1A1 E5A1 CB7B 06F6 FF00 00FF 0000"            /* ÷u¡¡å¡Ë{.öÿ..ÿ.. */
	$"0000 FF00 00FF F5F6 25A7 A1A1 A1A1 E67A"            /* ..ÿ..ÿõö%§¡¡¡¡æz */
	$"51A7 A1A1 A1A1 ADF7 F6F5 FF00 00FF 0000"            /* Q§¡¡¡¡­÷öõÿ..ÿ.. */
	$"0000 FFFF FFFF F5F6 F575 A1A1 A1A1 A157"            /* ..ÿÿÿÿõöõu¡¡¡¡¡W */
	$"51E3 A1A1 9BE7 81F5 01F6 FFFF FFFF 0000"            /* Qã¡¡›çõ.öÿÿÿÿ.. */
	$"0000 FFFF FFFF F6F5 F6F5 7CA1 9B9B A17B"            /* ..ÿÿÿÿöõöõ|¡››¡{ */
	$"509B 7D9B A7A6 F5F5 F6F5 FFFF FFFF 0000"            /* P›}›§¦õõöõÿÿÿÿ.. */
	$"0000 FF00 00FF F5F6 F5F6 F57B A1A1 A156"            /* ..ÿ..ÿõöõöõ{¡¡¡V */
	$"4BA1 C5A7 8100 F6F5 F6F5 FF00 00FF 0000"            /* K¡Å§.öõöõÿ..ÿ.. */
	$"0000 FF00 00FF F6F5 F6F5 F6F5 F67B A67A"            /* ..ÿ..ÿöõöõöõö{¦z */
	$"57A6 FAF7 F6F6 F5F6 F5F5 FF00 00FF 0000"            /* W¦ú÷ööõöõõÿ..ÿ.. */
	$"0000 FFFF FFFF F5F6 F5F6 F5F6 F601 F5F5"            /* ..ÿÿÿÿõöõöõöö.õõ */
	$"F5F6 F5F5 F5F6 F5F6 F6F5 FFFF FFFF 0000"            /* õöõõõöõööõÿÿÿÿ.. */
	$"0000 FFFF FFFF F6F5 F6F5 F5F6 F5F5 F6F5"            /* ..ÿÿÿÿöõöõõöõõöõ */
	$"F6F5 F5F6 F5F6 F5F6 F5F5 FFFF FFFF 0000"            /* öõõöõöõöõõÿÿÿÿ.. */
	$"0000 FF00 00FF F5F6 F5F6 F5F6 F5F6 F5F6"            /* ..ÿ..ÿõöõöõöõöõö */
	$"F5F6 F5F6 F5F5 F6F5 F6F6 FF00 00FF 0000"            /* õöõöõõöõööÿ..ÿ.. */
	$"0000 FF00 00FF F5F5 F6F5 F6F5 F6F5 F6F5"            /* ..ÿ..ÿõõöõöõöõöõ */
	$"F6F5 F6F5 F6F6 F5F6 F5F5 FF00 00FF 0000"            /* öõöõööõöõõÿ..ÿ.. */
	$"0000 FFFF FFFF F6F5 F5F6 F5F6 F5F6 F5F6"            /* ..ÿÿÿÿöõõöõöõöõö */
	$"F5F6 F5F6 F5F5 F6F5 F6F5 FFFF FFFF 0000"            /* õöõöõõöõöõÿÿÿÿ.. */
	$"0000 FFFF FFFF F5F5 F6F5 F6F5 F6F5 F6F5"            /* ..ÿÿÿÿõõöõöõöõöõ */
	$"F6F5 F6F5 F5F6 F5F6 F5F6 FFFF FFFF 0000"            /* öõöõõöõöõöÿÿÿÿ.. */
	$"0000 FF00 00FF F6F5 F6F5 F5F6 F5F6 F5F6"            /* ..ÿ..ÿöõöõõöõöõö */
	$"F5F6 F5F6 F6F5 F6F5 F6F5 FF00 00FF 0000"            /* õöõööõöõöõÿ..ÿ.. */
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"696C 3332 0000 070F 01FF FF93 0087 FF81"            /* il32.....ÿÿ“.‡ÿ */
	$"008E E601 0000 86FF 0300 FFFF 008E E602"            /* .Žæ...†ÿ..ÿÿ.Žæ. */
	$"0080 0085 FF03 00FF FF00 8EE6 0300 E680"            /* .€.…ÿ..ÿÿ.Žæ..æ€ */
	$"0084 FF81 008E E604 00E6 E680 0083 FF81"            /* .„ÿ.Žæ..ææ€.ƒÿ */
	$"008E E600 0080 E601 8000 82FF 0300 FFFF"            /* .Žæ..€æ.€.‚ÿ..ÿÿ */
	$"008E E684 0081 FF03 00FF FF00 91E6 0300"            /* .Žæ„.ÿ..ÿÿ.‘æ.. */
	$"FFFF 0081 FF81 0083 E607 B76E 6971 6F67"            /* ÿÿ.ÿ.ƒæ.·niqog */
	$"6EB7 83E6 8100 81FF 8100 81E6 0BEC 5432"            /* n·ƒæ.ÿ.æ.ìT2 */
	$"775E 4741 526F 2E54 EC81 E681 0081 FF03"            /* w^GARo.Tìæ.ÿ. */
	$"00FF FF00 80E6 0DED 391A 8B3D 2F30 2615"            /* .ÿÿ.€æ.í9.‹=/0&. */
	$"1E7E 1639 EC80 E603 00FF FF00 81FF 0300"            /* .~.9ì€æ..ÿÿ.ÿ.. */
	$"FFFF 0080 E60D 6100 7F49 3741 3A2F 2410"            /* ÿÿ.€æ.a..I7A:/$. */
	$"2B76 005F 80E6 0300 FFFF 0081 FF81 0013"            /* +v._€æ..ÿÿ.ÿ.. */
	$"E6E6 C503 2481 293C 3E39 2E23 1904 7623"            /* ææÅ.$)<>9.#..v# */
	$"02C2 E6E6 8100 81FF 8100 13E6 E670 003B"            /* .Âææ.ÿ..ææp.; */
	$"7821 3335 3128 1F16 006E 3A00 69E6 E681"            /* x!351(...n:.iææ */
	$"0081 FF1B 00FF FF00 E6E6 3E0F 357D 1828"            /* .ÿ..ÿÿ.ææ>.5}.( */
	$"2927 211A 1100 7934 1035 E6E6 00FF FF00"            /* )'!...y4.5ææ.ÿÿ. */
	$"81FF 1B00 FFFF 00E6 E631 1D1A 8C26 171E"            /* ÿ..ÿÿ.ææ1..Œ&.. */
	$"1D19 1303 1D8C 1B1D 25E6 E600 FFFF 0081"            /* .....Œ..%ææ.ÿÿ. */
	$"FF81 0013 E6E6 3523 1953 880A 090E 0B00"            /* ÿ..ææ5#.SˆÂÆ... */
	$"078C 501A 2327 E6E6 8100 81FF 8100 13E6"            /* .ŒP.#'ææ.ÿ..æ */
	$"E648 2328 1C68 9045 1715 4894 661D 2922"            /* æH#(.hE..H”f.)" */
	$"37E6 E681 0081 FF1B 00FF FF00 E6E6 7C20"            /* 7ææ.ÿ..ÿÿ.ææ|  */
	$"2E2E 2248 7AB4 B57C 4623 2D2F 176A E6E6"            /* .."Hz´µ|F#-/.jææ */
	$"00FF FF00 81FF 1B00 FFFF 00E6 E6C9 2832"            /* .ÿÿ.ÿ..ÿÿ.ææÉ(2 */
	$"3333 2F13 888D 1C2E 3334 3112 C1E6 E600"            /* 33/.ˆ..341.Áææ. */
	$"FFFF 0081 FF81 0080 E60D 7726 3938 3924"            /* ÿÿ.ÿ.€æ.w&989$ */
	$"888F 2C38 393A 1562 80E6 8100 81FF 8100"            /* ˆ,89:.b€æ.ÿ. */
	$"80E6 0DEE 5B28 3C3F 2889 9332 3F3B 1843"            /* €æ.î[(<?(‰“2?;.C */
	$"EB80 E681 0081 FF03 00FF FF00 81E6 0BED"            /* ë€æ.ÿ..ÿÿ.æ.í */
	$"722A 2F25 8994 2D26 1960 EB81 E603 00FF"            /* r*.%‰”-&.`ëæ..ÿ */
	$"FF00 81FF 0300 FFFF 0083 E607 C376 368F"            /* ÿ.ÿ..ÿÿ.ƒæ.Ãv6 */
	$"8D35 6CBE 83E6 0300 FFFF 0081 FF81 0091"            /* 5l¾ƒæ..ÿÿ.ÿ.‘ */
	$"E681 0081 FF81 0091 E681 0081 FF03 00FF"            /* æ.ÿ.‘æ.ÿ..ÿ */
	$"FF00 91E6 0300 FFFF 0081 FF03 00FF FF00"            /* ÿ.‘æ..ÿÿ.ÿ..ÿÿ. */
	$"91E6 0300 FFFF 0081 FF81 0091 E681 0081"            /* ‘æ..ÿÿ.ÿ.‘æ. */
	$"FF81 0091 E681 0081 FF03 00FF FF00 91E6"            /* ÿ.‘æ.ÿ..ÿÿ.‘æ */
	$"0300 FFFF 0081 FF99 0001 FFFF 01FF FF93"            /* ..ÿÿ.ÿ™..ÿÿ.ÿÿ“ */
	$"0087 FF81 008E E601 0000 86FF 0300 FFFF"            /* .‡ÿ.Žæ...†ÿ..ÿÿ */
	$"008E E602 0080 0085 FF03 00FF FF00 8EE6"            /* .Žæ..€.…ÿ..ÿÿ.Žæ */
	$"0300 E680 0084 FF81 008E E604 00E6 E680"            /* ..æ€.„ÿ.Žæ..ææ€ */
	$"0083 FF81 008E E600 0080 E601 8000 82FF"            /* .ƒÿ.Žæ..€æ.€.‚ÿ */
	$"0300 FFFF 008E E684 0081 FF03 00FF FF00"            /* ..ÿÿ.Žæ„.ÿ..ÿÿ. */
	$"91E6 0300 FFFF 0081 FF81 0083 E607 B66A"            /* ‘æ..ÿÿ.ÿ.ƒæ.¶j */
	$"6775 7266 6DB7 83E6 8100 81FF 8100 81E6"            /* gurfm·ƒæ.ÿ.æ */
	$"0BED 552F 94A6 9380 7978 2D55 EC81 E681"            /* .íU/”¦“€yx-Uìæ */
	$"0081 FF03 00FF FF00 80E6 0DEE 3E19 B6AD"            /* .ÿ..ÿÿ.€æ.î>.¶­ */
	$"A598 7C58 4886 1B3D EC80 E603 00FF FF00"            /* ¥˜|XH†.=ì€æ..ÿÿ. */
	$"81FF 0300 FFFF 0080 E60D 6F00 90AB B2BE"            /* ÿ..ÿÿ.€æ.o.«²¾ */
	$"AC8A 6942 3E7E 0667 80E6 0300 FFFF 0081"            /* ¬ŠiB>~.g€æ..ÿÿ. */
	$"FF81 0013 E6E6 CC22 33A8 91B1 B8A7 8767"            /* ÿ..ææÌ"3¨‘±¸§‡g */
	$"4A1C 7B42 19C5 E6E6 8100 81FF 8100 13E6"            /* J.{B.Åææ.ÿ..æ */
	$"E688 2A4F 9F7B 979C 8F76 5D42 1273 5F28"            /* æˆ*OŸ{—œv]B.s_( */
	$"76E6 E681 0081 FF1B 00FF FF00 E6E6 6744"            /* vææ.ÿ..ÿÿ.æægD */
	$"5398 5F76 7971 614C 3308 7E64 454D E6E6"            /* S˜_vyqaL3.~dEMææ */
	$"00FF FF00 81FF 1B00 FFFF 00E6 E666 5A4E"            /* .ÿÿ.ÿ..ÿÿ.ææfZN */
	$"9C52 535B 5549 3616 1BA0 5A5A 45E6 E600"            /* œRS[UI6.. ZZEææ. */
	$"FFFF 0081 FF81 0013 E6E6 7168 5F7C 922A"            /* ÿÿ.ÿ..ææqh_|’* */
	$"2E32 270F 038D 8B62 6847 E6E6 8100 81FF"            /* .2'..‹bhGææ.ÿ */
	$"8100 13E6 E687 7478 6B97 9745 2119 419C"            /* ..ææ‡txk——E!.Aœ */
	$"A170 786F 54E6 E681 0081 FF1B 00FF FF00"            /* ¡pxoTææ.ÿ..ÿÿ. */
	$"E6E6 AD7E 8687 7E96 A6B8 C8B4 9980 868B"            /* ææ­~†‡~–¦¸È´™€†‹ */
	$"6478 E6E6 00FF FF00 81FF 1B00 FFFF 00E6"            /* dxææ.ÿÿ.ÿ..ÿÿ.æ */
	$"E6DD 8B98 9798 9873 9BC8 8B94 9799 9748"            /* æÝ‹˜—˜˜s›È‹”—™—H */
	$"C1E6 E600 FFFF 0081 FF81 0080 E60D AF99"            /* Áææ.ÿÿ.ÿ.€æ.¯™ */
	$"A9A6 A987 9CCE A1A6 A8AC 666F 80E6 8100"            /* ©¦©‡œÎ¡¦¨¬fo€æ. */
	$"81FF 8100 80E6 0DF2 A1A1 B7BA 949F D6B1"            /* ÿ.€æ.ò¡¡·º”ŸÖ± */
	$"B9B4 705A E980 E681 0081 FF03 00FF FF00"            /* ¹´pZé€æ.ÿ..ÿÿ. */
	$"81E6 0BEF A691 AC97 A1DB AF91 5B71 E881"            /* æ.ï¦‘¬—¡Û¯‘[qè */
	$"E603 00FF FF00 81FF 0300 FFFF 0083 E607"            /* æ..ÿÿ.ÿ..ÿÿ.ƒæ. */
	$"CF9F 6C9B B067 81BF 83E6 0300 FFFF 0081"            /* ÏŸl›°g¿ƒæ..ÿÿ. */
	$"FF81 0091 E681 0081 FF81 0091 E681 0081"            /* ÿ.‘æ.ÿ.‘æ. */
	$"FF03 00FF FF00 91E6 0300 FFFF 0081 FF03"            /* ÿ..ÿÿ.‘æ..ÿÿ.ÿ. */
	$"00FF FF00 91E6 0300 FFFF 0081 FF81 0091"            /* .ÿÿ.‘æ..ÿÿ.ÿ.‘ */
	$"E681 0081 FF81 0091 E681 0081 FF03 00FF"            /* æ.ÿ.‘æ.ÿ..ÿ */
	$"FF00 91E6 0300 FFFF 0081 FF99 0001 FFFF"            /* ÿ.‘æ..ÿÿ.ÿ™..ÿÿ */
	$"01FF FF93 0087 FF81 008E E601 0000 86FF"            /* .ÿÿ“.‡ÿ.Žæ...†ÿ */
	$"0300 FFFF 008E E602 0080 0085 FF03 00FF"            /* ..ÿÿ.Žæ..€.…ÿ..ÿ */
	$"FF00 8EE6 0300 E680 0084 FF81 008E E604"            /* ÿ.Žæ..æ€.„ÿ.Žæ. */
	$"00E6 E680 0083 FF81 008E E600 0080 E601"            /* .ææ€.ƒÿ.Žæ..€æ. */
	$"8000 82FF 0300 FFFF 008E E684 0081 FF03"            /* €.‚ÿ..ÿÿ.Žæ„.ÿ. */
	$"00FF FF00 91E6 0300 FFFF 0081 FF81 0083"            /* .ÿÿ.‘æ..ÿÿ.ÿ.ƒ */
	$"E607 B770 696F 6E67 6FB7 83E6 8100 81FF"            /* æ.·piongo·ƒæ.ÿ */
	$"8100 81E6 0BEC 5434 6944 2324 476B 2E54"            /* .æ.ìT4iD#$Gk.T */
	$"EC81 E681 0081 FF03 00FF FF00 80E6 04EC"            /* ìæ.ÿ..ÿÿ.€æ.ì */
	$"371C 780F 8100 0414 7B15 39EC 80E6 0300"            /* 7.x....{.9ì€æ.. */
	$"FFFF 0081 FF03 00FF FF00 80E6 035C 0078"            /* ÿÿ.ÿ..ÿÿ.€æ.\.x */
	$"1B83 0003 2472 005C 80E6 0300 FFFF 0081"            /* .ƒ..$r.\€æ..ÿÿ. */
	$"FF81 0005 E6E6 C100 1E6E 8500 0573 1600"            /* ÿ..ææÁ..n…..s.. */
	$"C1E6 E681 0081 FF81 0005 E6E6 6700 3463"            /* Áææ.ÿ..ææg.4c */
	$"8500 056D 2800 67E6 E681 0081 FF09 00FF"            /* …..m(.gææ.ÿÆ.ÿ */
	$"FF00 E6E6 2D00 2771 8500 0977 1D00 2DE6"            /* ÿ.ææ-.'q….Æw..-æ */
	$"E600 FFFF 0081 FF0A 00FF FF00 E6E6 1600"            /* æ.ÿÿ.ÿÂ.ÿÿ.ææ.. */
	$"0787 1583 000A 1D84 0400 16E6 E600 FFFF"            /* .‡.ƒ.Â.„...ææ.ÿÿ */
	$"0081 FF81 0007 E6E6 1600 0043 8605 8100"            /* .ÿ..ææ...C†.. */
	$"070A 8A39 0000 16E6 E681 0081 FF81 0002"            /* .ÂŠ9...ææ.ÿ.. */
	$"E6E6 2D80 0007 578D 4714 154B 8F4E 8000"            /* ææ-€..WG..KN€. */
	$"022D E6E6 8100 81FF 0600 FFFF 00E6 E667"            /* .-ææ.ÿ..ÿÿ.ææg */
	$"8100 0527 68B1 AC66 2281 0006 67E6 E600"            /* ..'h±¬f"..gææ. */
	$"FFFF 0081 FF06 00FF FF00 E6E6 C183 0001"            /* ÿÿ.ÿ..ÿÿ.ææÁƒ.. */
	$"7E72 8300 06C1 E6E6 00FF FF00 81FF 8100"            /* ~rƒ..Áææ.ÿÿ.ÿ. */
	$"80E6 005D 8200 017E 7082 0000 5D80 E681"            /* €æ.]‚..~p‚..]€æ */
	$"0081 FF81 0080 E601 EC3D 8100 017E 7181"            /* .ÿ.€æ.ì=..~q */
	$"0001 3DEC 80E6 8100 81FF 0300 FFFF 0081"            /* ..=ì€æ.ÿ..ÿÿ. */
	$"E601 EB5F 8000 017E 7180 0001 5FEB 81E6"            /* æ.ë_€..~q€.._ëæ */
	$"0300 FFFF 0081 FF03 00FF FF00 83E6 07BF"            /* ..ÿÿ.ÿ..ÿÿ.ƒæ.¿ */
	$"6728 887C 2867 BF83 E603 00FF FF00 81FF"            /* g(ˆ|(g¿ƒæ..ÿÿ.ÿ */
	$"8100 91E6 8100 81FF 8100 91E6 8100 81FF"            /* .‘æ.ÿ.‘æ.ÿ */
	$"0300 FFFF 0091 E603 00FF FF00 81FF 0300"            /* ..ÿÿ.‘æ..ÿÿ.ÿ.. */
	$"FFFF 0091 E603 00FF FF00 81FF 8100 91E6"            /* ÿÿ.‘æ..ÿÿ.ÿ.‘æ */
	$"8100 81FF 8100 91E6 8100 81FF 0300 FFFF"            /* .ÿ.‘æ.ÿ..ÿÿ */
	$"0091 E603 00FF FF00 81FF 9900 01FF FF6C"            /* .‘æ..ÿÿ.ÿ™..ÿÿl */
	$"386D 6B00 0004 0800 00FF FFFF FFFF FFFF"            /* 8mk......ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 0000 0000 0000 00FF FFFF FFFF FFFF"            /* .........ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 00FF FFFF FFFF FFFF"            /* .........ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 00FF FFFF FFFF FFFF"            /* ÿ........ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 0000 0000 0000 00FF FFFF FFFF FFFF"            /* ÿÿ.......ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF00 0000 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿ......ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿ.....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿ....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF00 00"                                  /* ÿÿÿÿÿ.. */
};

data 'icns' (135, "Phisics Icon") {
	$"6963 6E73 0000 159A 4943 4E23 0000 0108"            /* icns...šICN#.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"7FFF FFF0 FFFF FFF8 FFFF FFF8 FFFF FFF8"            /* .ÿÿðÿÿÿøÿÿÿøÿÿÿø */
	$"FFFF FFF8 FFFF FFF8 FFFF FFF8 FFFF FFF8"            /* ÿÿÿøÿÿÿøÿÿÿøÿÿÿø */
	$"FFFF FFF8 DFFF FFFA 9FFF FFFF 1FFF FFFF"            /* ÿÿÿøßÿÿúŸÿÿÿ.ÿÿÿ */
	$"1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"1FFF FFFF 9FFF FFFF DFFF FFFA FFFF FFF8"            /* .ÿÿÿŸÿÿÿßÿÿúÿÿÿø */
	$"FFFF FFF8 FFFF FFF8 FFFF FFF8 FFFF FFF8"            /* ÿÿÿøÿÿÿøÿÿÿøÿÿÿø */
	$"FFFF FFF8 FFFF FFF8 FFFF FFF8 7FFF FFF0"            /* ÿÿÿøÿÿÿøÿÿÿø.ÿÿð */
	$"6963 6C34 0000 0208 0EDE EDEE DEED EEEE"            /* icl4.....ÞíîÞíîî */
	$"EEEE EEEE EEAE 0000 EDC0 0000 0000 0000"            /* îîîîî®..íÀ...... */
	$"0000 0000 00CD E000 DCC0 0000 0000 0000"            /* .....Íà.ÜÀ...... */
	$"00C0 000C 0C0D E000 E000 0000 0000 00C0"            /* .À....à.à......À */
	$"0C00 C0C0 C0CC F000 E000 0000 0000 C00C"            /* ..ÀÀÀÌð.à.....À. */
	$"000C 000C 0C0D E000 D000 0000 0000 0000"            /* ......à.Ð....... */
	$"0C00 C0C0 000D E000 E000 0000 0000 0C00"            /* ..ÀÀ..à.à....... */
	$"C0C0 0C0C C0CC F000 E000 0000 000C 000C"            /* ÀÀ..ÀÌð.à....... */
	$"0000 00C0 0C0D E000 D0E0 0000 0000 DDED"            /* ...À..à.Ðà....Ýí */
	$"DEDD 0C0C 00CC F000 EE0E 0000 000E EDD8"            /* ÞÝ...Ìð.î.....íØ */
	$"E9DE E00C 0C0C F0A0 E00E 0000 00FF C888"            /* éÞà...ð à....ÿÈˆ */
	$"A8FD FE00 C0CD EF0F 000E 0000 0DFD 888A"            /* ¨ýþ.ÀÍï......ýˆŠ */
	$"899E DFEC 0C0C F0CF 000E 0000 CFAD 8818"            /* ‰žßì..ðÏ....Ï­ˆ. */
	$"8A9F DAF0 C0C0 C0DE 000E 0000 DF8D A888"            /* ŠŸÚðÀÀÀÞ....ß¨ˆ */
	$"99AF D99E 0C0C 0CCF 000E 0000 99ED 8A89"            /* ™¯Ùž...Ï....™íŠ‰ */
	$"A99F D99E 0C00 C0DA 000E 0000 A99D A9A9"            /* ©ŸÙž..ÀÚ....©©© */
	$"9AFE D9A9 C00C 0CCF 000E 0000 8A88 DF9F"            /* šþÙ©À..Ï....ŠˆßŸ */
	$"FFFD 8A8F 0C0C 00DF 000E 0000 B999 DCEE"            /* ÿýŠ...ß....¹™Üî */
	$"FECB 8A9E C00C 0CCF 000E 0000 D88A 88DC"            /* þËŠžÀ..Ï....ØŠˆÜ */
	$"CD88 989D 0CC0 C0DF 000E 0000 C8A8 898D"            /* Íˆ˜.ÀÀß....È¨‰ */
	$"B88A 88AC 000C CCCF 000E 0000 0D88 8B8D"            /* ¸Šˆ¬..ÌÏ.....ˆ‹ */
	$"C888 89B0 C0CC FDCF E00E 0000 00B8 888D"            /* Èˆ‰°ÀÌýÏà....¸ˆ */
	$"1888 AE0C 0C0D FFCF EE0E 0000 000B 888D"            /* .ˆ®...ÿÏî.....ˆ */
	$"D88A D0C0 C0CC F0F0 E0A0 0000 0000 CBED"            /* ØŠÐÀÀÌððà ....Ëí */
	$"C8DC 0C0C 0C0D F000 E000 0000 0000 0000"            /* ÈÜ....ð.à....... */
	$"C000 C0C0 C0CC F000 E000 0000 0000 00C0"            /* À.ÀÀÀÌð.à......À */
	$"0C0C 000C 0C0C F000 E000 0000 000C 0000"            /* ......ð.à....... */
	$"C000 CC00 C0CD F000 F000 0000 0000 000C"            /* À.Ì.ÀÍð.ð....... */
	$"000C 000C 0C0C F000 E000 0000 0000 0C00"            /* ......ð.à....... */
	$"0C00 C0C0 0C0D F000 ECC0 0000 0000 C000"            /* ..ÀÀ..ð.ìÀ....À. */
	$"C00C 0C00 C0CC F000 ADDD DCDD DDDC DDDD"            /* À...ÀÌð.­ÝÜÝÝÜÝÝ */
	$"CDDC DCDD CDCD F000 0FEE EFFE AEEF EFAE"            /* ÍÜÜÝÍÍð..îïþ®ïï® */
	$"FFEF FFFF FFFF 0000 6963 6C38 0000 0408"            /* ÿïÿÿÿÿ..icl8.... */
	$"00FB 81FB FB81 FBFB FBFB FBFB FBFC FBFC"            /* .ûûûûûûûûûûüûü */
	$"FBFC FCFC FCFC FCAC FCAC ACAC 0000 0000"            /* ûüüüüüü¬ü¬¬¬.... */
	$"FBF9 F7F5 0000 0000 0000 0000 0000 0000"            /* ûù÷õ............ */
	$"0000 0000 0000 0000 00F5 2BF9 AC00 0000"            /* .........õ+ù¬... */
	$"81F7 F600 0000 0000 0000 0000 00F5 F500"            /* ÷ö..........õõ. */
	$"F5F5 F5F5 F5F5 F5F5 F6F6 F656 AC00 0000"            /* õõõõõõõõöööV¬... */
	$"FBF5 0000 0000 0000 0000 00F5 F500 F5F5"            /* ûõ.........õõ.õõ */
	$"F5F5 F5F5 F5F5 F6F6 F5F6 F556 AC00 0000"            /* õõõõõõööõöõV¬... */
	$"FB00 0000 0000 0000 0000 0000 F5F5 F5F5"            /* û...........õõõõ */
	$"F5F5 F5F6 F5F5 F5F6 F5F6 F6F8 FD00 0000"            /* õõõöõõõöõööøý... */
	$"FB00 0000 0000 0000 0000 00F5 00F5 00F5"            /* û..........õ.õ.õ */
	$"F5F5 F5F5 F6F5 F6F5 F5F6 F556 AC00 0000"            /* õõõõöõöõõöõV¬... */
	$"FB00 0000 0000 0000 0000 0000 F5F5 F5F5"            /* û...........õõõõ */
	$"F5F5 F5F5 F5F6 F5F6 F6F5 F656 FD00 0000"            /* õõõõõöõööõöVý... */
	$"FB00 0000 0000 0000 0000 00F5 F500 F5F5"            /* û..........õõ.õõ */
	$"F5F5 F5F5 F5F5 F5F6 F5F6 F556 AC00 0000"            /* õõõõõõõöõöõV¬... */
	$"FB00 FB00 0000 0000 0000 0000 F881 81FA"            /* û.û.........øú */
	$"8181 81F7 F6F5 F6F5 F6F5 F656 FD00 0000"            /* ÷öõöõöõöVý... */
	$"FBFB 00FC 0000 0000 0000 F5FC AC7B 7B7D"            /* ûû.ü......õü¬{{} */
	$"A0A6 FAFD FBF5 F5F6 F5F6 F6F8 FD00 FD00"            /*  ¦úýûõõöõööøý.ý. */
	$"FB00 00FB 0000 0000 00F5 ACFE 50A1 A1A1"            /* û..û.....õ¬þP¡¡¡ */
	$"A7CB AD7A E0AC F5F5 F6F5 F656 FDFD 00FD"            /* §Ë­zà¬õõöõöVýý.ý */
	$"0000 00FC 0000 0000 0081 FF7A 7D9B 9BA1"            /* ...ü.....ÿz}››¡ */
	$"A1A7 E7AD F9EA FBF6 F5F6 F556 FD00 F8FE"            /* ¡§ç­ùêûöõöõVý.øþ */
	$"0000 00FB 0000 0000 2BE9 AD57 C5A1 77A1"            /* ...û....+é­WÅ¡w¡ */
	$"A1A7 E8E9 FAAD E92B F6F5 F6F5 F7F5 56FD"            /* ¡§èéú­é+öõöõ÷õVý */
	$"0000 00FC 0000 0000 7BE9 FC75 A7A1 A1A1"            /* ...ü....{éüu§¡¡¡ */
	$"A7CB ADEA FAA6 D181 F5F6 F6F6 F5F6 56FE"            /* §Ë­êú¦ÑõöööõöVþ */
	$"0000 00FC 0000 0000 A6E8 A6F9 CBA7 A1CB"            /* ...ü....¦è¦ùË§¡Ë */
	$"A7E8 E8FF FAA7 E8A6 F5F6 F5F6 F6F6 56FD"            /* §èèÿú§è¦õöõöööVý */
	$"0000 00FC 0000 0000 A7A7 E7F9 ADA7 E8A7"            /* ...ü....§§çù­§è§ */
	$"E7AD E0FD 56E7 A7AD F6F5 F6F5 F6F6 F8FE"            /* ç­àýVç§­öõöõööøþ */
	$"0000 00FC 0000 0000 A7A7 A7A0 56E9 D1E9"            /* ...ü....§§§ VéÑé */
	$"E9EA EA56 A0A7 CBFD F5F6 F5F6 F6F5 56FE"            /* éêêV §ËýõöõööõVþ */
	$"0000 00FC 0000 0000 A0A7 E6A7 7B56 FCFD"            /* ...ü.... §æ§{Vüý */
	$"FEFC F87C A7A7 A7A6 F6F5 F6F6 F5F6 56FE"            /* þüø|§§§¦öõööõöVþ */
	$"0000 00FC 0000 0000 57E5 A1A7 E57C 7B2C"            /* ...ü....Wå¡§å|{, */
	$"F775 A1A7 C5A1 CBFA F5F6 F6F5 F6F6 56FE"            /* ÷u¡§Å¡ËúõööõööVþ */
	$"0000 00FC 0000 0000 2BA1 A1A1 A1A1 A77A"            /* ...ü....+¡¡¡¡¡§z */
	$"51A7 A1A1 A1A1 ADF7 F5F6 F5F6 56F6 F8FE"            /* Q§¡¡¡¡­÷õöõöVöøþ */
	$"0000 00AC 0000 0000 007B A1A1 9BA1 C557"            /* ...¬.....{¡¡›¡ÅW */
	$"50E3 A19B A1CB 8101 F6F5 F656 FE56 56FE"            /* Pã¡›¡Ë.öõöVþVVþ */
	$"FC00 00FC 0000 0000 00F5 7C9B A19B A1F9"            /* ü..ü.....õ|›¡›¡ù */
	$"4BA1 77A1 A7A6 00F6 F5F6 F556 FEFF F8FF"            /* K¡w¡§¦.öõöõVþÿøÿ */
	$"ACFC 00AC 0000 0000 0000 F57B A1A1 A1F9"            /* ¬ü.¬......õ{¡¡¡ù */
	$"519B E5A7 81F5 F624 F6F5 F656 FE00 FF00"            /* Q›å§õö$öõöVþ.ÿ. */
	$"FC00 AC00 0000 0000 0000 0000 2B7B A6F9"            /* ü.¬.........+{¦ù */
	$"51A6 FA2B 2506 2407 F5F6 F556 FE00 0000"            /* Q¦ú+%.$.õöõVþ... */
	$"AC00 0000 0000 0000 0000 0000 F500 F5F5"            /* ¬...........õ.õõ */
	$"F5F5 F5F5 F5F5 F6F5 F6F5 F656 E000 0000"            /* õõõõõõöõöõöVà... */
	$"FC00 0000 0000 0000 0000 00F5 00F5 F500"            /* ü..........õ.õõ. */
	$"F5F5 F5F6 F5F5 F5F6 F5F6 F6F8 EA00 0000"            /* õõõöõõõöõööøê... */
	$"AC00 0000 0000 0000 0000 00F5 00F5 F5F5"            /* ¬..........õ.õõõ */
	$"F5F5 F5F5 F5F6 F5F5 F6F5 F656 E000 0000"            /* õõõõõöõõöõöVà... */
	$"AC00 0000 0000 0000 0000 F500 F500 F5F5"            /* ¬.........õ.õ.õõ */
	$"F5F5 F5F5 F5F5 F6F6 F5F6 F6F8 F400 0000"            /* õõõõõõööõööøô... */
	$"ACF5 0000 0000 0000 0000 00F5 00F5 F5F5"            /* ¬õ.........õ.õõõ */
	$"F5F5 F5F6 F5F5 F5F6 F5F6 F556 FF00 0000"            /* õõõöõõõöõöõVÿ... */
	$"ACF7 F600 0000 0000 0000 0000 F5F5 F5F5"            /* ¬÷ö.........õõõõ */
	$"F5F5 F5F5 F6F5 F6F5 F6F5 F656 FF00 0000"            /* õõõõöõöõöõöVÿ... */
	$"ACF9 5656 5656 5656 5656 5656 5656 5656"            /* ¬ùVVVVVVVVVVVVVV */
	$"5656 5656 5656 5656 5656 56F9 FF00 0000"            /* VVVVVVVVVVVùÿ... */
	$"00FD ACFD ACFD FDFD FDFD FDFE FDFE FDFE"            /* .ý¬ý¬ýýýýýýþýþýþ */
	$"FEFE FEFE FEEA E0EA E0FF FFFF 0000 0000"            /* þþþþþêàêàÿÿÿ.... */
	$"696C 3332 0000 0A72 1BFF 5E5E 5D5C 5B5A"            /* il32..Âr.ÿ^^]\[Z */
	$"5958 5755 5452 514F 4E4C 4A49 4745 4342"            /* YXWUTRQONLJIGECB */
	$"403E 3C3B 3981 FF03 5E80 BFE1 92FF 03E1"            /* @><;9ÿ.^€¿á’ÿ.á */
	$"BF80 3580 FF02 5EBF E183 FF13 FDFB F8F5"            /* ¿€5€ÿ.^¿áƒÿ.ýûøõ */
	$"F3F1 F0EE EDEC EBEA E9E7 E6E5 E4E1 9C34"            /* óñðîíìëêéçæåäáœ4 */
	$"80FF 015D E184 FF13 FDFB F8F5 F3F1 F0EE"            /* €ÿ.]á„ÿ.ýûøõóñðî */
	$"EDEC EBEA E9E7 E6E5 E4E3 9C32 80FF 005C"            /* íìëêéçæåäãœ2€ÿ.\ */
	$"85FF 13FD FBF8 F5F3 F1F0 EEED ECEB EAE9"            /* …ÿ.ýûøõóñðîíìëêé */
	$"E7E6 E5E4 E39C 3080 FF00 5B85 FF13 FDFB"            /* çæåäãœ0€ÿ.[…ÿ.ýû */
	$"F8F5 F3F1 F0EE EDEC EBEA E9E7 E6E5 E4E3"            /* øõóñðîíìëêéçæåäã */
	$"9C2F 80FF 005A 85FF 13FD FBF8 F5F3 F1F0"            /* œ/€ÿ.Z…ÿ.ýûøõóñð */
	$"EEED ECEB EAE9 E7E6 E5E4 E39C 2D80 FF00"            /* îíìëêéçæåäãœ-€ÿ. */
	$"5985 FF13 FDFB F8F5 F3F1 F0EE EDEC EBEA"            /* Y…ÿ.ýûøõóñðîíìëê */
	$"E9E7 E6E5 E4E3 9C2C 80FF 0258 FF55 83FF"            /* éçæåäãœ,€ÿ.XÿUƒÿ */
	$"13FD FBF8 B76E 6971 6F67 6EB7 EAE9 E7E6"            /* .ýûø·niqogn·êéçæ */
	$"E5E4 E39C 2A80 FF03 5655 FF52 82FF 1AFD"            /* åäãœ*€ÿ.VUÿR‚ÿ.ý */
	$"EC54 3277 5E47 4152 6F2E 54EC E7E6 E5E4"            /* ìT2w^GARo.Tìçæåä */
	$"E39C 28FF 25FF 55FF FF50 82FF 16ED 391A"            /* ãœ(ÿ%ÿUÿÿP‚ÿ.í9. */
	$"8B3D 2F30 2615 1E7E 1639 ECE6 E5E4 E39C"            /* ‹=/0&..~.9ìæåäãœ */
	$"2625 FF21 80FF 004F 82FF 1661 007F 4937"            /* &%ÿ!€ÿ.O‚ÿ.a..I7 */
	$"413A 2F24 102B 7600 5FE6 E5E4 E39C 25FF"            /* A:/$.+v._æåäãœ%ÿ */
	$"9C1F 80FF 004D 81FF 17C5 0324 8129 3C3E"            /* œ.€ÿ.Mÿ.Å.$)<> */
	$"392E 2319 0476 2302 C2E5 E4E3 E1BF E09C"            /* 9.#..v#.Âåäãá¿àœ */
	$"1E80 FF00 4B81 FF17 7000 3B78 2133 3531"            /* .€ÿ.Kÿ.p.;x!351 */
	$"281F 1600 6E3A 0069 E5E4 E3E2 E1E0 9C1C"            /* (...n:.iåäãâáàœ. */
	$"80FF 004A 81FF 173E 0F35 7D18 2829 2721"            /* €ÿ.Jÿ.>.5}.()'! */
	$"1A11 0079 3410 35E5 E4E3 E2E1 E09C 1A80"            /* ...y4.5åäãâáàœ.€ */
	$"FF00 4881 FF17 311D 1A8C 2617 1E1D 1913"            /* ÿ.Hÿ.1..Œ&..... */
	$"031D 8C1B 1D25 E5E4 E3E2 E1E0 9C18 80FF"            /* ..Œ..%åäãâáàœ.€ÿ */
	$"0046 81FF 1735 2319 5388 0A09 0E0B 0007"            /* .Fÿ.5#.SˆÂÆ.... */
	$"8C50 1A23 27E5 E4E3 E2E1 E09C 1680 FF00"            /* ŒP.#'åäãâáàœ.€ÿ. */
	$"4481 FF17 4823 281C 6890 4517 1548 9466"            /* Dÿ.H#(.hE..H”f */
	$"1D29 2237 E5E4 E3E2 E1E0 9C15 80FF 0042"            /* .)"7åäãâáàœ.€ÿ.B */
	$"81FF 177C 202E 2E22 487A B4B5 7C46 232D"            /* ÿ.| .."Hz´µ|F#- */
	$"2F17 6AE5 E4E3 E2E1 E09C 1380 FF00 4181"            /* /.jåäãâáàœ.€ÿ.A */
	$"FF17 C928 3233 332F 1388 8D1C 2E33 3431"            /* ÿ.É(233/.ˆ..341 */
	$"12C1 E5E4 E3E2 9CE0 9C11 80FF 003F 82FF"            /* .Áåäãâœàœ.€ÿ.?‚ÿ */
	$"1A77 2639 3839 2488 8F2C 3839 3A15 62E6"            /* .w&989$ˆ,89:.bæ */
	$"E5E4 E39C 149C 9C0F 42FF FF3D 82FF 1AEE"            /* åäãœ.œœ.Bÿÿ=‚ÿ.î */
	$"5B28 3C3F 2889 9332 3F3B 1843 EBE6 E5E4"            /* [(<?(‰“2?;.Cëæåä */
	$"E39C 1311 9C0E 403F FF3B 82FF 19FD ED72"            /* ãœ..œ.@?ÿ;‚ÿ.ýír */
	$"2A2F 2589 942D 2619 60EB E7E6 E5E4 E39C"            /* *.%‰”-&.`ëçæåäãœ */
	$"11FF 0EFF 3FFF 3B83 FF13 FDFB F8C3 7636"            /* .ÿ.ÿ?ÿ;ƒÿ.ýûøÃv6 */
	$"8F8D 356C BEEA E9E7 E6E5 E4E3 9C0F 80FF"            /* 5l¾êéçæåäãœ.€ÿ */
	$"003D 85FF 13FD FBF8 F5F3 F1F0 EEED ECEB"            /* .=…ÿ.ýûøõóñðîíìë */
	$"EAE9 E7E6 E5E4 E39C 0E80 FF00 3B85 FF13"            /* êéçæåäãœ.€ÿ.;…ÿ. */
	$"FDFB F8F5 F3F1 F0EE EDEC EBEA E9E7 E6E5"            /* ýûøõóñðîíìëêéçæå */
	$"E4E3 9C0C 80FF 0039 85FF 13FD FBF8 F5F3"            /* äãœ.€ÿ.9…ÿ.ýûøõó */
	$"F1F0 EEED ECEB EAE9 E7E6 E5E4 E39C 0A80"            /* ñðîíìëêéçæåäãœÂ€ */
	$"FF00 3785 FF13 FDFB F8F5 F3F1 F0EE EDEC"            /* ÿ.7…ÿ.ýûøõóñðîíì */
	$"EBEA E9E7 E6E5 E4E3 9C09 80FF 0136 E184"            /* ëêéçæåäãœÆ€ÿ.6á„ */
	$"FF13 FDFB F8F5 F3F1 F0EE EDEC EBEA E9E7"            /* ÿ.ýûøõóñðîíìëêéç */
	$"E6E5 E4E3 9C08 80FF 0234 BFE1 83FF 13FD"            /* æåäãœ.€ÿ.4¿áƒÿ.ý */
	$"FBF8 F5F3 F1F0 EEED ECEB EAE9 E7E6 E5E4"            /* ûøõóñðîíìëêéçæåä */
	$"E19C 0680 FF01 3280 969C 0180 0581 FF1A"            /* áœ.€ÿ.2€–œ.€.ÿ. */
	$"2F2D 2C2A 2927 2523 2220 1E1C 1B19 1715"            /* /-,*)'%#" ...... */
	$"1412 100F 0D0C 0A09 0706 0581 FF1B FF5E"            /* ......ÂÆ...ÿ.ÿ^ */
	$"5E5D 5C5B 5A59 5857 5554 5251 4F4E 4C4A"            /* ^]\[ZYXWUTRQONLJ */
	$"4947 4543 4240 3E3C 3B39 81FF 035E 80BF"            /* IGECB@><;9ÿ.^€¿ */
	$"E192 FF03 E1BF 8035 80FF 025E BFE1 83FF"            /* á’ÿ.á¿€5€ÿ.^¿áƒÿ */
	$"13FD FBF8 F5F3 F1F0 EEED ECEB EAE9 E7E6"            /* .ýûøõóñðîíìëêéçæ */
	$"E5E4 E19C 3480 FF01 5DE1 84FF 13FD FBF8"            /* åäáœ4€ÿ.]á„ÿ.ýûø */
	$"F5F3 F1F0 EEED ECEB EAE9 E7E6 E5E4 E39C"            /* õóñðîíìëêéçæåäãœ */
	$"3280 FF00 5C85 FF13 FDFB F8F5 F3F1 F0EE"            /* 2€ÿ.\…ÿ.ýûøõóñðî */
	$"EDEC EBEA E9E7 E6E5 E4E3 9C30 80FF 005B"            /* íìëêéçæåäãœ0€ÿ.[ */
	$"85FF 13FD FBF8 F5F3 F1F0 EEED ECEB EAE9"            /* …ÿ.ýûøõóñðîíìëêé */
	$"E7E6 E5E4 E39C 2F80 FF00 5A85 FF13 FDFB"            /* çæåäãœ/€ÿ.Z…ÿ.ýû */
	$"F8F5 F3F1 F0EE EDEC EBEA E9E7 E6E5 E4E3"            /* øõóñðîíìëêéçæåäã */
	$"9C2D 80FF 0059 85FF 13FD FBF8 F5F3 F1F0"            /* œ-€ÿ.Y…ÿ.ýûøõóñð */
	$"EEED ECEB EAE9 E7E6 E5E4 E39C 2C80 FF02"            /* îíìëêéçæåäãœ,€ÿ. */
	$"58FF 5583 FF13 FDFB F8B6 6A67 7572 666D"            /* XÿUƒÿ.ýûø¶jgurfm */
	$"B7EA E9E7 E6E5 E4E3 9C2A 80FF 0356 55FF"            /* ·êéçæåäãœ*€ÿ.VUÿ */
	$"5282 FF1A FDED 552F 94A6 9380 7978 2D55"            /* R‚ÿ.ýíU/”¦“€yx-U */
	$"ECE7 E6E5 E4E3 9C28 FF25 FF55 FFFF 5082"            /* ìçæåäãœ(ÿ%ÿUÿÿP‚ */
	$"FF16 EE3E 19B6 ADA5 987C 5848 861B 3DEC"            /* ÿ.î>.¶­¥˜|XH†.=ì */
	$"E6E5 E4E3 9C26 25FF 2180 FF00 4F82 FF16"            /* æåäãœ&%ÿ!€ÿ.O‚ÿ. */
	$"6F00 90AB B2BE AC8A 6942 3E7E 0667 E6E5"            /* o.«²¾¬ŠiB>~.gæå */
	$"E4E3 9C25 FF9C 1F80 FF00 4D81 FF17 CC22"            /* äãœ%ÿœ.€ÿ.Mÿ.Ì" */
	$"33A8 91B1 B8A7 8767 4A1C 7B42 19C5 E5E4"            /* 3¨‘±¸§‡gJ.{B.Ååä */
	$"E3E1 BFE0 9C1E 80FF 004B 81FF 1788 2A4F"            /* ãá¿àœ.€ÿ.Kÿ.ˆ*O */
	$"9F7B 979C 8F76 5D42 1273 5F28 76E5 E4E3"            /* Ÿ{—œv]B.s_(våäã */
	$"E2E1 E09C 1C80 FF00 4A81 FF17 6744 5398"            /* âáàœ.€ÿ.Jÿ.gDS˜ */
	$"5F76 7971 614C 3308 7E64 454D E5E4 E3E2"            /* _vyqaL3.~dEMåäãâ */
	$"E1E0 9C1A 80FF 0048 81FF 1766 5A4E 9C52"            /* áàœ.€ÿ.Hÿ.fZNœR */
	$"535B 5549 3616 1BA0 5A5A 45E5 E4E3 E2E1"            /* S[UI6.. ZZEåäãâá */
	$"E09C 1880 FF00 4681 FF17 7168 5F7C 922A"            /* àœ.€ÿ.Fÿ.qh_|’* */
	$"2E32 270F 038D 8B62 6847 E5E4 E3E2 E1E0"            /* .2'..‹bhGåäãâáà */
	$"9C16 80FF 0044 81FF 1787 7478 6B97 9745"            /* œ.€ÿ.Dÿ.‡txk——E */
	$"2119 419C A170 786F 54E5 E4E3 E2E1 E09C"            /* !.Aœ¡pxoTåäãâáàœ */
	$"1580 FF00 4281 FF17 AD7E 8687 7E96 A6B8"            /* .€ÿ.Bÿ.­~†‡~–¦¸ */
	$"C8B4 9980 868B 6478 E5E4 E3E2 E1E0 9C13"            /* È´™€†‹dxåäãâáàœ. */
	$"80FF 0041 81FF 17DD 8B98 9798 9873 9BC8"            /* €ÿ.Aÿ.Ý‹˜—˜˜s›È */
	$"8B94 9799 9748 C1E5 E4E3 E29C E09C 1180"            /* ‹”—™—HÁåäãâœàœ.€ */
	$"FF00 3F82 FF1A AF99 A9A6 A987 9CCE A1A6"            /* ÿ.?‚ÿ.¯™©¦©‡œÎ¡¦ */
	$"A8AC 666F E6E5 E4E3 9C14 9C9C 0F42 FFFF"            /* ¨¬foæåäãœ.œœ.Bÿÿ */
	$"3D82 FF1A F2A1 A1B7 BA94 9FD6 B1B9 B470"            /* =‚ÿ.ò¡¡·º”ŸÖ±¹´p */
	$"5AE9 E6E5 E4E3 9C13 119C 0E40 3FFF 3B82"            /* Zéæåäãœ..œ.@?ÿ;‚ */
	$"FF19 FDEF A691 AC97 A1DB AF91 5B71 E8E7"            /* ÿ.ýï¦‘¬—¡Û¯‘[qèç */
	$"E6E5 E4E3 9C11 FF0E FF3F FF3B 83FF 13FD"            /* æåäãœ.ÿ.ÿ?ÿ;ƒÿ.ý */
	$"FBF8 CF9F 6C9B B067 81BF EAE9 E7E6 E5E4"            /* ûøÏŸl›°g¿êéçæåä */
	$"E39C 0F80 FF00 3D85 FF13 FDFB F8F5 F3F1"            /* ãœ.€ÿ.=…ÿ.ýûøõóñ */
	$"F0EE EDEC EBEA E9E7 E6E5 E4E3 9C0E 80FF"            /* ðîíìëêéçæåäãœ.€ÿ */
	$"003B 85FF 13FD FBF8 F5F3 F1F0 EEED ECEB"            /* .;…ÿ.ýûøõóñðîíìë */
	$"EAE9 E7E6 E5E4 E39C 0C80 FF00 3985 FF13"            /* êéçæåäãœ.€ÿ.9…ÿ. */
	$"FDFB F8F5 F3F1 F0EE EDEC EBEA E9E7 E6E5"            /* ýûøõóñðîíìëêéçæå */
	$"E4E3 9C0A 80FF 0037 85FF 13FD FBF8 F5F3"            /* äãœÂ€ÿ.7…ÿ.ýûøõó */
	$"F1F0 EEED ECEB EAE9 E7E6 E5E4 E39C 0980"            /* ñðîíìëêéçæåäãœÆ€ */
	$"FF01 36E1 84FF 13FD FBF8 F5F3 F1F0 EEED"            /* ÿ.6á„ÿ.ýûøõóñðîí */
	$"ECEB EAE9 E7E6 E5E4 E39C 0880 FF02 34BF"            /* ìëêéçæåäãœ.€ÿ.4¿ */
	$"E183 FF13 FDFB F8F5 F3F1 F0EE EDEC EBEA"            /* áƒÿ.ýûøõóñðîíìëê */
	$"E9E7 E6E5 E4E1 9C06 80FF 0132 8096 9C01"            /* éçæåäáœ.€ÿ.2€–œ. */
	$"8005 81FF 1A2F 2D2C 2A29 2725 2322 201E"            /* €.ÿ./-,*)'%#" . */
	$"1C1B 1917 1514 1210 0F0D 0C0A 0907 0605"            /* ...........ÂÆ... */
	$"81FF 1BFF 5E5E 5D5C 5B5A 5958 5755 5452"            /* ÿ.ÿ^^]\[ZYXWUTR */
	$"514F 4E4C 4A49 4745 4342 403E 3C3B 3981"            /* QONLJIGECB@><;9 */
	$"FF03 5E80 BFE1 92FF 03E1 BF80 3580 FF02"            /* ÿ.^€¿á’ÿ.á¿€5€ÿ. */
	$"5EBF E183 FF13 FDFB F8F5 F3F1 F0EE EDEC"            /* ^¿áƒÿ.ýûøõóñðîíì */
	$"EBEA E9E7 E6E5 E4E1 9C34 80FF 015D E184"            /* ëêéçæåäáœ4€ÿ.]á„ */
	$"FF13 FDFB F8F5 F3F1 F0EE EDEC EBEA E9E7"            /* ÿ.ýûøõóñðîíìëêéç */
	$"E6E5 E4E3 9C32 80FF 005C 85FF 13FD FBF8"            /* æåäãœ2€ÿ.\…ÿ.ýûø */
	$"F5F3 F1F0 EEED ECEB EAE9 E7E6 E5E4 E39C"            /* õóñðîíìëêéçæåäãœ */
	$"3080 FF00 5B85 FF13 FDFB F8F5 F3F1 F0EE"            /* 0€ÿ.[…ÿ.ýûøõóñðî */
	$"EDEC EBEA E9E7 E6E5 E4E3 9C2F 80FF 005A"            /* íìëêéçæåäãœ/€ÿ.Z */
	$"85FF 13FD FBF8 F5F3 F1F0 EEED ECEB EAE9"            /* …ÿ.ýûøõóñðîíìëêé */
	$"E7E6 E5E4 E39C 2D80 FF00 5985 FF13 FDFB"            /* çæåäãœ-€ÿ.Y…ÿ.ýû */
	$"F8F5 F3F1 F0EE EDEC EBEA E9E7 E6E5 E4E3"            /* øõóñðîíìëêéçæåäã */
	$"9C2C 80FF 0258 FF55 83FF 13FD FBF8 B770"            /* œ,€ÿ.XÿUƒÿ.ýûø·p */
	$"696F 6E67 6FB7 EAE9 E7E6 E5E4 E39C 2A80"            /* iongo·êéçæåäãœ*€ */
	$"FF03 5655 FF52 82FF 1AFD EC54 3469 4423"            /* ÿ.VUÿR‚ÿ.ýìT4iD# */
	$"2447 6B2E 54EC E7E6 E5E4 E39C 28FF 25FF"            /* $Gk.Tìçæåäãœ(ÿ%ÿ */
	$"55FF FF50 82FF 04EC 371C 780F 8100 0D14"            /* UÿÿP‚ÿ.ì7.x.... */
	$"7B15 39EC E6E5 E4E3 9C26 25FF 2180 FF00"            /* {.9ìæåäãœ&%ÿ!€ÿ. */
	$"4F82 FF03 5C00 781B 8300 0C24 7200 5CE6"            /* O‚ÿ.\.x.ƒ..$r.\æ */
	$"E5E4 E39C 25FF 9C1F 80FF 004D 81FF 03C1"            /* åäãœ%ÿœ.€ÿ.Mÿ.Á */
	$"001E 6E85 000B 7316 00C1 E5E4 E3E1 BFE0"            /* ..n…..s..Áåäãá¿à */
	$"9C1E 80FF 004B 81FF 0367 0034 6385 000B"            /* œ.€ÿ.Kÿ.g.4c….. */
	$"6D28 0067 E5E4 E3E2 E1E0 9C1C 80FF 004A"            /* m(.gåäãâáàœ.€ÿ.J */
	$"81FF 032D 0027 7185 000B 771D 002D E5E4"            /* ÿ.-.'q…..w..-åä */
	$"E3E2 E1E0 9C1A 80FF 0048 81FF 0416 0007"            /* ãâáàœ.€ÿ.Hÿ.... */
	$"8715 8300 0C1D 8404 0016 E5E4 E3E2 E1E0"            /* ‡.ƒ...„...åäãâáà */
	$"9C18 80FF 0046 81FF 0516 0000 4386 0581"            /* œ.€ÿ.Fÿ....C†. */
	$"000D 0A8A 3900 0016 E5E4 E3E2 E1E0 9C16"            /* ..ÂŠ9...åäãâáàœ. */
	$"80FF 0044 81FF 002D 8000 0757 8D47 1415"            /* €ÿ.Dÿ.-€..WG.. */
	$"4B8F 4E80 0008 2DE5 E4E3 E2E1 E09C 1580"            /* KN€..-åäãâáàœ.€ */
	$"FF00 4281 FF00 6781 0005 2768 B1AC 6622"            /* ÿ.Bÿ.g..'h±¬f" */
	$"8100 0867 E5E4 E3E2 E1E0 9C13 80FF 0041"            /* ..gåäãâáàœ.€ÿ.A */
	$"81FF 00C1 8300 017E 7283 0008 C1E5 E4E3"            /* ÿ.Áƒ..~rƒ..Áåäã */
	$"E29C E09C 1180 FF00 3F82 FF00 5D82 0001"            /* âœàœ.€ÿ.?‚ÿ.]‚.. */
	$"7E70 8200 0D5D E6E5 E4E3 9C14 9C9C 0F42"            /* ~p‚..]æåäãœ.œœ.B */
	$"FFFF 3D82 FF01 EC3D 8100 017E 7181 000E"            /* ÿÿ=‚ÿ.ì=..~q.. */
	$"3DEC E6E5 E4E3 9C13 119C 0E40 3FFF 3B82"            /* =ìæåäãœ..œ.@?ÿ;‚ */
	$"FF02 FDEB 5F80 0001 7E71 8000 0E5F EBE7"            /* ÿ.ýë_€..~q€.._ëç */
	$"E6E5 E4E3 9C11 FF0E FF3F FF3B 83FF 13FD"            /* æåäãœ.ÿ.ÿ?ÿ;ƒÿ.ý */
	$"FBF8 BF67 2888 7C28 67BF EAE9 E7E6 E5E4"            /* ûø¿g(ˆ|(g¿êéçæåä */
	$"E39C 0F80 FF00 3D85 FF13 FDFB F8F5 F3F1"            /* ãœ.€ÿ.=…ÿ.ýûøõóñ */
	$"F0EE EDEC EBEA E9E7 E6E5 E4E3 9C0E 80FF"            /* ðîíìëêéçæåäãœ.€ÿ */
	$"003B 85FF 13FD FBF8 F5F3 F1F0 EEED ECEB"            /* .;…ÿ.ýûøõóñðîíìë */
	$"EAE9 E7E6 E5E4 E39C 0C80 FF00 3985 FF13"            /* êéçæåäãœ.€ÿ.9…ÿ. */
	$"FDFB F8F5 F3F1 F0EE EDEC EBEA E9E7 E6E5"            /* ýûøõóñðîíìëêéçæå */
	$"E4E3 9C0A 80FF 0037 85FF 13FD FBF8 F5F3"            /* äãœÂ€ÿ.7…ÿ.ýûøõó */
	$"F1F0 EEED ECEB EAE9 E7E6 E5E4 E39C 0980"            /* ñðîíìëêéçæåäãœÆ€ */
	$"FF01 36E1 84FF 13FD FBF8 F5F3 F1F0 EEED"            /* ÿ.6á„ÿ.ýûøõóñðîí */
	$"ECEB EAE9 E7E6 E5E4 E39C 0880 FF02 34BF"            /* ìëêéçæåäãœ.€ÿ.4¿ */
	$"E183 FF13 FDFB F8F5 F3F1 F0EE EDEC EBEA"            /* áƒÿ.ýûøõóñðîíìëê */
	$"E9E7 E6E5 E4E1 9C06 80FF 0132 8096 9C01"            /* éçæåäáœ.€ÿ.2€–œ. */
	$"8005 81FF 1A2F 2D2C 2A29 2725 2322 201E"            /* €.ÿ./-,*)'%#" . */
	$"1C1B 1917 1514 1210 0F0D 0C0A 0907 0605"            /* ...........ÂÆ... */
	$"81FF 6C38 6D6B 0000 0408 00FF FFFF FFFF"            /* ÿl8mk.....ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿ....ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF 00FF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿ.ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 FF00 FF00 00FF FFFF"            /* ÿÿÿÿÿÿÿ.ÿ.ÿ..ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿ...ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿ...ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿ...ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿ...ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿ...ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿ...ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿ...ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿ...ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿ...ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿ...ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF00 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿ..ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 FF00 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ.ÿ.ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿ...ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 00FF FFFF FFFF"            /* ÿÿÿÿÿÿÿ....ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000"                           /* ÿÿÿÿÿÿ.... */
};

data 'icns' (136, "Images Icon") {
	$"6963 6E73 0000 154C 4943 4E23 0000 0108"            /* icns...LICN#.... */
	$"01FF FFC0 01FF FFE0 06AE DB50 1957 FFE8"            /* .ÿÿÀ.ÿÿà.®ÛP.Wÿè */
	$"36ED DECC 65B6 FBC2 7AFF DF7F E55A FFFF"            /* 6íÞÌe¶ûÂzÿß.åZÿÿ */
	$"BFF7 F777 6DBD FFFF D7E7 EFFF 6BAE FEDF"            /* ¿÷÷wm½ÿÿ×çïÿk®þß */
	$"B45B FFFB 6AB7 F7EF 3655 BFFD 0AAF FFFB"            /* ´[ÿûj·÷ï6U¿ýÂ¯ÿû */
	$"1B5F 6FFF 067B FFBD 01FF DFEF 01F6 FFFB"            /* ._oÿ.{ÿ½.ÿßï.öÿû */
	$"01FF FFFF 01F7 BFED 01AF EEF7 017B 7FEB"            /* .ÿÿÿ.÷¿í.¯î÷.{.ë */
	$"01BF DFD7 01F5 F7AD 015F FFD7 01FB 6EAB"            /* .¿ß×.õ÷­._ÿ×.ûn« */
	$"01BF FF55 01FF DFB7 01FF FF9B 01FF FFFF"            /* .¿ÿU.ÿß·.ÿÿ›.ÿÿÿ */
	$"01FF FFC0 01FF FFE0 07FF FFF0 1FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð.ÿÿø */
	$"3FFF FFFC 7FFF FFFE 7FFF FFFF FFFF FFFF"            /* ?ÿÿü.ÿÿþ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"7FFF FFFF 7FFF FFFF 3FFF FFFF 1FFF FFFF"            /* .ÿÿÿ.ÿÿÿ?ÿÿÿ.ÿÿÿ */
	$"0FFF FFFF 03FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"6963 6C34 0000 0208 0000 000F FFFF FFFF"            /* icl4........ÿÿÿÿ */
	$"FFFF FFFF FF00 0000 0000 000F EEFE EFFE"            /* ÿÿÿÿÿ.......îþïþ */
	$"FF96 FEFF EFF0 0000 0000 CEDE DED5 8FF9"            /* ÿ–þÿïð....ÎÞÞÕù */
	$"5FE9 5959 9FDF 0000 000E ED8B 8ED9 EFE9"            /* _éYYŸß....í‹ŽÙïé */
	$"EF85 FEFE EF0D F000 00AF D888 999D 9599"            /* ï…þþï.ð..¯Øˆ™•™ */
	$"FE9F 695E 9F0C DF00 0EFC 8888 89AE DFE8"            /* þŸi^Ÿ.ß..üˆˆ‰®ßè */
	$"FF96 EE69 EFC0 0DF0 CF9D 8B8B 8A9F D9F9"            /* ÿ–îiïÀ.ðÏ‹‹ŠŸÙù */
	$"FE9E FEFE 9FFF FFFF B9ED A888 999F DEFE"            /* þžþþŸÿÿÿ¹í¨ˆ™ŸÞþ */
	$"FFEF 5965 96EE FEEF 9A9B 89A8 A9AF D89E"            /* ÿïYe–îþïš›‰¨©¯Øž */
	$"F9F6 95FF EF6F E69F A99D 9A8A 9AF9 D9A9"            /* ùö•ÿïoæŸ©šŠšùÙ© */
	$"FFF5 F5F6 5F5F 6F5F 99A8 D9FF 9FFD B99A"            /* ÿõõö__o_™¨ÙÿŸý¹š */
	$"FF69 5F6E 6F56 F5FF 8A89 DDEE FEDB 8A99"            /* ÿi_noVõÿŠ‰ÝîþÛŠ™ */
	$"FFFF F5F6 E6FF 5F5F D89B 8B8C CD88 989E"            /* ÿÿõöæÿ__Ø›‹ŒÍˆ˜ž */
	$"FF9E FEFE 6F5F 65EF 0988 988D D88A 88AF"            /* ÿžþþo_eïÆˆ˜ØŠˆ¯ */
	$"FFEF EFFF F5F6 EEEF 0B88 88AD 1888 8A9F"            /* ÿïïÿõöîï.ˆˆ­.ˆŠŸ */
	$"99F9 FEFF 5F6E 6E6F 008B 888D D888 A9F9"            /* ™ùþÿ_nno.‹ˆØˆ©ù */
	$"FA9A 9F6F F5F5 F68F 0008 B88C D88A 8FAF"            /* úšŸoõõö..¸ŒØŠ¯ */
	$"99AF FFFE FF5F 5966 0000 CDBB 98EE F9FF"            /* ™¯ÿþÿ_Yf..Í»˜îùÿ */
	$"9FF9 F9FF FFF6 E6DF 0000 000F FFF9 9F99"            /* Ÿùùÿÿöæß....ÿùŸ™ */
	$"A9FF AFFF EF5E E96F 0000 000F FFFF 99F9"            /* ©ÿ¯ÿï^éo....ÿÿ™ù */
	$"FFF9 FFFF FFF6 96EF 0000 000F FF99 9FAF"            /* ÿùÿÿÿö–ï....ÿ™Ÿ¯ */
	$"9F9F FFFE FF5E 686F 0000 000F F999 9A9F"            /* ŸŸÿþÿ^ho....ù™šŸ */
	$"99FF FFEF FFE6 959F 0000 000F 9999 99F9"            /* ™ÿÿïÿæ•Ÿ....™™™ù */
	$"F999 9FF9 FE69 795F 0000 000F 8989 9F9F"            /* ù™Ÿùþiy_....‰‰ŸŸ */
	$"AF99 FEFF FFE7 E79F 0000 000F 9899 99F9"            /* ¯™þÿÿççŸ....˜™™ù */
	$"F9FE 9F9F E687 857F 0000 000F 999E 899F"            /* ùþŸŸæ‡….....™ž‰Ÿ */
	$"9F99 9FFE F9D7 87EF 0000 000F 9999 9E9A"            /* Ÿ™Ÿþù×‡ï....™™žš */
	$"FAF9 F9FF 5787 785F 0000 000F 9999 F99F"            /* úùùÿW‡x_....™™ùŸ */
	$"9F99 9FFF 7877 8E7F 0000 000F 9999 9F99"            /* Ÿ™ŸÿxwŽ.....™™Ÿ™ */
	$"F9FE FF5F E78D 7DEF 0000 000F 999F FFFF"            /* ùþÿ_ç}ï....™Ÿÿÿ */
	$"9999 FFF6 E787 E79F 0000 000F FFFF 9FFF"            /* ™™ÿöç‡çŸ....ÿÿŸÿ */
	$"999F F5F5 9778 DE6F 0000 000F FFFF FFFF"            /* ™Ÿõõ—xÞo....ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 6963 6C38 0000 0408"            /* ÿÿÿÿÿÿÿÿicl8.... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"0000 0000 0000 00F4 FDCA D6AB D0F3 D0D0"            /* .......ôýÊÖ«ÐóÐÐ */
	$"D6F1 ACF1 F2AC F2F2 D0FF FF00 0000 0000"            /* Öñ¬ñò¬òòÐÿÿ..... */
	$"0000 0000 F7FB FB81 FAFB 8181 A7F3 EAA1"            /* ....÷ûûúû§óê¡ */
	$"D6D0 A6D0 ABCA B1CA A6FF F9FF 0000 0000"            /* ÖÐ¦Ð«Ê±Ê¦ÿùÿ.... */
	$"0000 F5FB AC7B 7BA1 A082 FAFD A6D0 ABCB"            /* ..õû¬{{¡ ‚úý¦Ð«Ë */
	$"D0B2 E5B1 F1AC F1D0 A6FF F6F9 FF00 0000"            /* Ð²å±ñ¬ñÐ¦ÿöùÿ... */
	$"00F5 ACFE 51A1 9BA1 A7CB AD7A FEFD D0A7"            /* .õ¬þQ¡›¡§Ë­zþýÐ§ */
	$"F3D0 A6F1 ABD0 B1FC CBFF F5F6 F9FF 0000"            /* óÐ¦ñ«Ð±üËÿõöùÿ.. */
	$"0081 FF7B 7CA1 9BA1 A1A7 E8AD F9EA ACA7"            /* .ÿ{|¡›¡¡§è­ùê¬§ */
	$"E9D6 E5B1 F1A6 D5D0 A6FF F5F5 F6F9 FF00"            /* éÖå±ñ¦ÕÐ¦ÿõõöùÿ. */
	$"2BE9 FD51 E577 A1A1 A1CB ADE9 FAAD E9E6"            /* +éýQåw¡¡¡Ë­éú­éæ */
	$"FED0 A7CF B1D0 B1CA A6FF FFFF FFFF FFFF"            /* þÐ§Ï±Ð±Ê¦ÿÿÿÿÿÿÿ */
	$"7BE9 A67B A7A1 A1A1 A7A7 E7E9 81A6 E9A6"            /* {é¦{§¡¡¡§§çé¦é¦ */
	$"F3F3 A6F2 F2AB F1CF FCD5 D0AC D5AC D0FF"            /* óó¦òò«ñÏüÕÐ¬Õ¬Ðÿ */
	$"ADE7 FC57 CBA7 A1A7 CBAD D1FF F9A7 E8AC"            /* ­çüWË§¡§Ë­Ñÿù§è¬ */
	$"EAE8 F2B1 A6F1 B1F2 A6F1 B1F1 A6D5 A6FF"            /* êèò±¦ñ±ò¦ñ±ñ¦Õ¦ÿ */
	$"E7A7 E756 A7CB ADCB ADE8 EAFD 56E7 A7E8"            /* ç§çV§Ë­Ë­èêýVç§è */
	$"E0F3 F3F1 D5B1 D5B1 D5B1 F1B1 F1B1 D5FF"            /* àóóñÕ±Õ±Õ±ñ±ñ±Õÿ */
	$"A7A7 E782 7AE9 E8E9 D1E0 FEF9 7CCB A7AD"            /* §§ç‚zéèéÑàþù|Ë§­ */
	$"F4F4 D6AB CEB1 D5B1 D5D5 B1F1 B1F1 B1FF"            /* ôôÖ«Î±Õ±ÕÕ±ñ±ñ±ÿ */
	$"A0A7 A7E6 7B56 FCFD FDFC 567B E6A7 E6AC"            /*  §§æ{VüýýüV{æ§æ¬ */
	$"FFF4 EAD6 FDD6 B1CE B1B1 F1D5 D5B1 D5D6"            /* ÿôêÖýÖ±Î±±ñÕÕ±ÕÖ */
	$"7BE5 A1A1 A7A0 572C 4F51 A1A1 A7A1 A7FD"            /* {å¡¡§ W,OQ¡¡§¡§ý */
	$"EAEA FDD0 FDD0 F4B1 CED5 B1F1 B1CF ABFF"            /* êêýÐýÐô±ÎÕ±ñ±Ï«ÿ */
	$"F6A1 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E8EA"            /* ö¡¡¡¡¡æzQæ¡¡¡¡èê */
	$"EAAD E8D1 E9FE F2E9 F1B1 F1B1 CEAB CFFF"            /* ê­èÑéþòéñ±ñ±Î«Ïÿ */
	$"007B A1A1 A19B A757 51A1 A1A1 9BA7 ACEA"            /* .{¡¡¡›§WQ¡¡¡›§¬ê */
	$"E9E8 ADD1 FDEA FDD6 FED5 B1D5 ABCF AAFF"            /* éè­ÑýêýÖþÕ±Õ«Ïªÿ */
	$"0000 7CA1 9BA1 A156 4BA1 9B9B A7D0 EAE9"            /* ..|¡›¡¡VK¡››§Ðêé */
	$"E8AD E7AD E9FE F3F3 FFB1 F1B1 F0AB C9FF"            /* è­ç­éþóóÿ±ñ±ð«Éÿ */
	$"0000 F575 A1A1 A17B 509B A7CB A6E9 E0D1"            /* ..õu¡¡¡{P›§Ë¦éàÑ */
	$"ADE8 D1D1 FEE9 FED6 E0D6 D5F1 B1CF A4D2"            /* ­èÑÑþéþÖàÖÕñ±Ï¤Ò */
	$"0000 0000 F775 A6F9 ACA7 ACA6 D1D1 FED1"            /* ....÷u¦ù¬§¬¦ÑÑþÑ */
	$"D1AD E8FD E9EA F4FE EAFE D5B1 CFAB C9FF"            /* Ñ­èýéêôþêþÕ±Ï«Éÿ */
	$"0000 0000 0000 00FF EAFF EAD1 CAE9 D1D1"            /* .......ÿêÿêÑÊéÑÑ */
	$"D1D1 FDE9 E0EA FFF3 FEF3 B1CF AACF ABFF"            /* ÑÑýéàêÿóþó±ÏªÏ«ÿ */
	$"0000 0000 0000 00FF FFF4 EAD1 E7E9 D1AD"            /* .......ÿÿôêÑçéÑ­ */
	$"E8FE E9E9 E9FF F4FE EAFE F1AB CFA4 CFFF"            /* èþéééÿôþêþñ«Ï¤Ïÿ */
	$"0000 0000 0000 00FF FFFF E8CA E7D1 FDD1"            /* .......ÿÿÿèÊçÑýÑ */
	$"D0D1 E9E0 EAFF EAF3 FEEA B1CE ABCF ABFF"            /* ÐÑéàêÿêóþê±Î«Ï«ÿ */
	$"0000 0000 0000 00FF E9E8 CBCA CAAD E9D1"            /* .......ÿéèËÊÊ­éÑ */
	$"E8E8 E0EA FFD0 EAFE EAFE EFAB CFAB C9FF"            /* èèàêÿÐêþêþï«Ï«Éÿ */
	$"0000 0000 0000 00FF CAE6 CAE6 CAD1 D1FD"            /* .......ÿÊæÊæÊÑÑý */
	$"D1E8 CAE8 D0E9 FEE9 FEFE B1C8 A5C8 CFFF"            /* ÑèÊèÐéþéþþ±È¥ÈÏÿ */
	$"0000 0000 0000 00FF CBC4 E5CA CAE8 ADD1"            /* .......ÿËÄåÊÊè­Ñ */
	$"E9AD D1CA E8E9 E9FE E0D6 CE9F C2C9 A5FF"            /* é­ÑÊèééþàÖÎŸÂÉ¥ÿ */
	$"0000 0000 0000 00FF E6E6 CAE6 CBCA E9E9"            /* .......ÿææÊæËÊéé */
	$"D1FE E8CA ADEA FEE9 FEAB 9FC2 989E CFFF"            /* ÑþèÊ­êþéþ«ŸÂ˜žÏÿ */
	$"0000 0000 0000 00FF CAE6 CACB CACA E8AD"            /* .......ÿÊæÊËÊÊè­ */
	$"E9D1 ADCA E9FE E9FE D0CF C274 C2C3 A4FF"            /* éÑ­ÊéþéþÐÏÂtÂÃ¤ÿ */
	$"0000 0000 0000 00FF E6CA CBCA E7CB CAE9"            /* .......ÿæÊËÊçËÊé */
	$"D1E9 D1CA D1E0 FEFE A5C8 99C2 98C9 A4FF"            /* ÑéÑÊÑàþþ¥È™Â˜É¤ÿ */
	$"0000 0000 0000 00FF CAE6 CAE8 D1CA E8E8"            /* .......ÿÊæÊèÑÊèè */
	$"D1FE D1CB FED1 FEF2 C89F 9E98 C3A4 C9FF"            /* ÑþÑËþÑþòÈŸž˜Ã¤Éÿ */
	$"0000 0000 0000 00FF CBCA E7E8 D1D1 D1D1"            /* .......ÿËÊçèÑÑÑÑ */
	$"CAE8 D1D0 E0EA F2B1 A4BC C29E 74C9 AAEA"            /* ÊèÑÐàêò±¤¼ÂžtÉªê */
	$"0000 0000 0000 00FF D0E8 E9E9 E9EA EAE0"            /* .......ÿÐèéééêêà */
	$"D1CA CAD1 EAF3 B1F0 ABC3 989F C2A4 C9FF"            /* ÑÊÊÑêó±ð«Ã˜ŸÂ¤Éÿ */
	$"0000 0000 0000 00FF D1E9 EAEA EAF4 FFE9"            /* .......ÿÑéêêêôÿé */
	$"E8E8 E8D0 EAF2 D5AB C89E C29E C9CF A4FF"            /* èèèÐêòÕ«ÈžÂžÉÏ¤ÿ */
	$"0000 0000 0000 00FF FFFF FFFF F4FF FFFF"            /* .......ÿÿÿÿÿôÿÿÿ */
	$"FFFF E0FF FFFF FFFF FFFF FFFF E0FF FFFF"            /* ÿÿàÿÿÿÿÿÿÿÿÿàÿÿÿ */
	$"696C 3332 0000 0A24 84FF 9000 8AFF 1300"            /* il32..Â$„ÿ.Šÿ.. */
	$"1819 0621 1404 0C13 0711 1C0F 0E18 1010"            /* ...!............ */
	$"1100 0086 FF17 B262 5A62 615A 605C 1D00"            /* ...†ÿ.²bZbaZ`\.. */
	$"021C 0410 2316 141D 1515 1D00 8000 83FF"            /* ....#.......€.ƒÿ */
	$"1AEC 5432 775E 4741 526F 2E38 1023 1C02"            /* .ìT2w^GARo.8.#.. */
	$"0722 1615 1B16 171F 00E6 8000 81FF 1CED"            /* .".......æ€.ÿ.í */
	$"391A 8B3D 2F30 2615 1E7E 1620 151B 0104"            /* 9.‹=/0&..~. .... */
	$"2016 161B 1817 1F00 E6E6 8000 80FF 1861"            /*  .......ææ€.€ÿ.a */
	$"007F 4937 413A 2F24 102B 7600 301C 0104"            /* ..I7A:/$.+v.0... */
	$"2117 171B 1816 2000 80E6 1B80 00FF C503"            /* !..... .€æ.€.ÿÅ. */
	$"2481 293C 3E39 2E23 1904 7623 021B 0104"            /* $)<>9.#..v#.... */
	$"2015 171C 1617 2084 007F 6500 3B78 2133"            /*  ..... „..e.;x!3 */
	$"3531 281F 1600 6E3A 0036 0203 1711 1619"            /* 51(...n:.6...... */
	$"1618 1D17 161C 1617 1D00 2F0F 357D 1828"            /* ........../.5}.( */
	$"2927 211A 1100 7934 1028 0218 010D 2A16"            /* )'!...y4.(....*. */
	$"1515 2C16 1515 2914 2B00 221D 1A8C 2617"            /* ..,...).+."..Œ&. */
	$"1E1D 1913 031D 8C1B 1D16 0201 0214 1817"            /* ......Œ......... */
	$"1817 1516 1615 1414 1500 2623 1953 880A"            /* ..........&#.SˆÂ */
	$"090E 0B00 078C 501A 2318 0002 0219 1F19"            /* Æ....ŒP.#....... */
	$"191A 1715 1617 1312 1500 7F39 2328 1C68"            /* ...........9#(.h */
	$"9045 1715 4894 661D 2922 2800 0001 090E"            /* E..H”f.)"(...Æ. */
	$"1018 1E1C 1614 1417 1617 0071 202E 2E22"            /* ...........q .." */
	$"487A B4B5 7C46 232D 2F17 2E00 050A 0E0F"            /* Hz´µ|F#-/....Â.. */
	$"1007 181D 1714 1118 1C17 00C9 2832 3333"            /* ...........É(233 */
	$"2F13 888D 1C2E 3334 3112 0205 1013 100C"            /* /.ˆ..341....... */
	$"0D0F 0616 1615 1C25 1314 00FF 7726 3938"            /* .......%...ÿw&98 */
	$"3924 888F 2C38 393A 1532 030B 1715 110C"            /* 9$ˆ,89:.2...... */
	$"0810 080A 1413 1E29 1216 003F FFEE 5B28"            /* ...Â...)...?ÿî[( */
	$"3C3F 2889 9332 3F3B 1822 0506 1117 1610"            /* <?(‰“2?;."...... */
	$"0B07 070B 0511 1311 1216 1500 FFFF ED72"            /* ............ÿÿír */
	$"2A2F 2589 942D 2619 3003 0606 1015 130F"            /* *.%‰”-&.0....... */
	$"0A07 040A 060D 1413 1319 1D00 81FF 1BC3"            /* Â..Â........ÿ.Ã */
	$"7636 803F 262F 2C00 0708 060E 1110 0D09"            /* v6€?&/,........Æ */
	$"0502 0806 0A14 1415 151D 0084 FF83 0012"            /* ....Â......„ÿƒ.. */
	$"090A 070D 0D0C 0907 0401 0307 0813 2427"            /* ÆÂ....Æ.......$' */
	$"1217 0084 FF83 0003 0A0A 0707 8008 0B05"            /* ...„ÿƒ..ÂÂ..€... */
	$"0201 0607 0815 1518 1614 0084 FF83 0003"            /* ...........„ÿƒ.. */
	$"0A0B 0A04 8005 0B03 0002 0807 081B 2513"            /* Â.Â.€.........%. */
	$"1615 0084 FF83 0003 0A0B 0B07 8002 0B00"            /* ...„ÿƒ..Â...€... */
	$"0105 0709 091F 2014 1714 0084 FF83 0012"            /* ...ÆÆ. ....„ÿƒ.. */
	$"090C 0B0B 0602 0102 0507 0709 0B18 1717"            /* Æ..........Æ.... */
	$"1814 0084 FF02 0002 0580 0001 060C 800B"            /* ...„ÿ....€....€. */
	$"0208 0007 8108 060F 1E18 1C16 1500 84FF"            /* .............„ÿ */
	$"0200 0508 8000 0002 810B 0209 0208 8009"            /* ....€.....Æ..€Æ */
	$"0708 1520 161C 1B16 0084 FF02 0001 0181"            /* ... .....„ÿ.... */
	$"0011 080B 0A0B 0E02 0808 090A 1713 1B3C"            /* ....Â.....ÆÂ...< */
	$"1A18 1B00 84FF 8400 1102 090A 0A08 0107"            /* ....„ÿ„...ÆÂÂ... */
	$"0808 0B21 171F 2916 1B1B 0084 FF83 0012"            /* ...!..)....„ÿƒ.. */
	$"0100 0208 0909 0308 0709 1318 2023 151E"            /* ....ÆÆ...Æ.. #.. */
	$"1A1B 0084 FF83 0012 0602 0001 0507 0306"            /* ...„ÿƒ.......... */
	$"050D 1720 2A12 2A40 1218 0084 FF83 0000"            /* ... *.*@...„ÿƒ.. */
	$"0180 000E 0201 0002 0011 1A19 1716 242B"            /* .€............$+ */
	$"1620 0084 FF8B 000A 0617 1D1A 1D1B 1A12"            /* . .„ÿ‹.Â........ */
	$"151F 0084 FF96 0084 FF90 008A FF13 0036"            /* ...„ÿ–.„ÿ.Šÿ..6 */
	$"5503 2948 0416 4604 0D45 0313 4100 1F32"            /* U.)H..F..E..A..2 */
	$"0000 86FF 17B1 5E5B 6668 5E5F 6670 0818"            /* ..†ÿ.±^[fh^_fp.. */
	$"7210 1B72 0C1B 5600 4267 0080 0083 FF1A"            /* r..r..V.Bg.€.ƒÿ. */
	$"ED55 2F94 A693 8079 782D 6318 3B73 111B"            /* íU/”¦“€yx-c.;s.. */
	$"730E 1548 0042 6800 E680 0081 FF1C EE3E"            /* s..H.Bh.æ€.ÿ.î> */
	$"19B6 ADA5 987C 5848 861B 272C 6C10 1E6C"            /* .¶­¥˜|XH†.',l..l */
	$"0E15 4900 4367 00E6 E680 0080 FF18 6F00"            /* ..I.Cg.ææ€.€ÿ.o. */
	$"90AB B2BE AC8A 6942 3E7E 063F 7210 1D72"            /* «²¾¬ŠiB>~.?r..r */
	$"0E15 4900 4268 0080 E61B 8000 FFCC 2233"            /* ..I.Bh.€æ.€.ÿÌ"3 */
	$"A891 B1B8 A787 674A 1C7B 4219 7312 1C73"            /* ¨‘±¸§‡gJ.{B.s..s */
	$"0E16 4A00 4369 8400 347D 2A4F 9F7B 979C"            /* ..J.Ci„.4}*OŸ{—œ */
	$"8F76 5D42 1273 5F28 5E0B 1051 090D 2D00"            /* v]B.s_(^..QÆ.-. */
	$"2A45 002F 4A00 2E42 0059 4453 985F 7679"            /* *E./J..B.YDS˜_vy */
	$"7161 4C33 087E 6445 4402 4800 0555 8000"            /* qaL3.~dED.H..U€. */
	$"0063 8000 1855 0064 0058 5A4E 9C52 535B"            /* .c€..U.d.XZNœRS[ */
	$"5549 3616 1BA0 5A5A 3604 0101 0B01 8600"            /* UI6.. ZZ6.....†. */
	$"1A01 0063 685F 7C92 2A2E 3227 0F03 8D8B"            /* ...ch_|’*.2'..‹ */
	$"6268 3800 0406 2919 0200 0402 8200 7F01"            /* bh8...).....‚... */
	$"0079 7478 6B97 9745 2119 419C A170 786F"            /* .ytxk——E!.Aœ¡pxo */
	$"4500 0004 161A 0E02 1A19 0400 0001 020D"            /* E............... */
	$"00A2 7E86 877E 96A6 B8C8 B499 8086 8B64"            /* .¢~†‡~–¦¸È´™€†‹d */
	$"3A01 0D22 2826 1A07 1E1F 0500 000C 1624"            /* :.."(&.........$ */
	$"00DD 8B98 9798 9873 9BC8 8B94 9799 9748"            /* .Ý‹˜—˜˜s›È‹”—™—H */
	$"070F 313C 3124 1A15 0809 0000 082D 2E36"            /* ..1<1$...Æ...-.6 */
	$"00FE AF99 A9A6 A987 9CCE A1A6 A8AC 663F"            /* .þ¯™©¦©‡œÎ¡¦¨¬f? */
	$"0F28 4542 3424 150F 0804 0000 0935 3C41"            /* .(EB4$......Æ5<A */
	$"4A00 FFF2 A1A1 B7BA 949F D6B1 B9B4 703F"            /* J.ÿò¡¡·º”ŸÖ±¹´p? */
	$"111F 3845 4233 2213 0804 0701 0000 0D3A"            /* ..8EB3"........: */
	$"5200 FFFF EFA6 91AC 97A1 DBAF 915B 5B23"            /* R.ÿÿï¦‘¬—¡Û¯‘[[# */
	$"1728 3740 3C2E 1F12 0705 0904 0000 0938"            /* .(7@<.....Æ...Æ8 */
	$"5900 80FF 1CFE CF9F 6C8C 5058 4154 3F29"            /* Y.€ÿ.þÏŸlŒPXAT?) */
	$"202F 3338 3628 1B0F 0506 0C08 0007 1D37"            /*  /386(.........7 */
	$"5200 84FF 8000 1510 364D 2C26 3933 2F2C"            /* R.„ÿ€...6M,&93/, */
	$"2215 0B02 060F 0B00 304B 3740 0084 FF80"            /* ".......0K7@.„ÿ€ */
	$"0015 163A 572E 2938 3225 241D 1006 0110"            /* ...:W.)82%$..... */
	$"110C 0019 3B41 3700 84FF 1800 000A 3551"            /* ....;A7.„ÿ...Â5Q */
	$"5D2F 2B2E 431C 1B14 0A03 0B16 110E 0224"            /* ]/+.C...Â......$ */
	$"3244 4000 84FF 1800 2840 636E 6A32 2B2B"            /* 2D@.„ÿ..(@cnj2++ */
	$"3F41 150E 0A13 1113 120E 0731 474E 4300"            /* ?A..Â......1GNC. */
	$"84FF 1800 6678 776F 6737 2A2A 2B40 4E4A"            /* „ÿ..fxwog7**+@NJ */
	$"4A24 1613 120C 0551 7263 4200 84FF 0600"            /* J$.....QrcB.„ÿ.. */
	$"767F 796E 6545 812B 0D37 7733 1D16 1412"            /* v.yneE+.7w3.... */
	$"0B36 8B9F 734C 0084 FF18 007A 8174 6865"            /* .6‹ŸsL.„ÿ..zthe */
	$"5F2C 2B2C 2B32 7227 1916 1513 1D8A B3AE"            /* _,+,+2r'.....Š³® */
	$"8258 0084 FF18 0075 786A 5D6B 6F42 2B29"            /* ‚X.„ÿ..uxj]koB+) */
	$"2939 6925 1817 1721 49A2 C8AF 896A 0084"            /* )9i%...!I¢È¯‰j.„ */
	$"FF18 006F 6D64 5656 675F 2E29 2731 6927"            /* ÿ..omdVVg_.)'1i' */
	$"1916 1853 78A5 B8A8 8B5D 0084 FF18 006E"            /* ...Sx¥¸¨‹].„ÿ..n */
	$"6B64 5140 554A 3F2D 252D 5F20 1714 155D"            /* kdQ@UJ?-%-_ ...] */
	$"97B3 A8A2 7855 0084 FF18 0060 6558 4534"            /* —³¨¢xU.„ÿ..`eXE4 */
	$"3B2D 3352 4131 3813 0D04 015E B7AE B0AB"            /* ;-3RA18....^·®°« */
	$"6B53 0084 FF18 0044 4825 1C20 0A0F 1942"            /* kS.„ÿ..DH%. Â..B */
	$"665E 2901 0100 0050 A3AC AA8F 6159 0084"            /* f^)....P£¬ªaY.„ */
	$"FF18 002E 250E 1412 0000 103D 4452 3D05"            /* ÿ...%......=DR=. */
	$"0200 0C4B 90B6 9E67 5053 0084 FF96 0084"            /* ...K¶žgPS.„ÿ–.„ */
	$"FF90 008A FF13 0047 2B44 5928 2F39 253D"            /* ÿ.Šÿ..G+DY(/9%= */
	$"4E3B 4E47 354E 4435 0000 86FF 17B2 645A"            /* N;NG5ND5..†ÿ.²dZ */
	$"6062 5964 770F 1F18 0B31 441B 574D 3258"            /* `bYdw....1D.WM2X */
	$"3A25 0080 0083 FF1A EC54 3469 4423 2447"            /* :%.€.ƒÿ.ìT4iD#$G */
	$"6B2E 2F32 3E07 2733 1C58 543B 5B3F 2800"            /* k./2>.'3.XT;[?(. */
	$"E680 0081 FF04 EC37 1C78 0F81 0013 147B"            /* æ€.ÿ.ì7.x....{ */
	$"152B 2404 1B2D 185B 573B 5F3F 2700 E6E6"            /* .+$..-.[W;_?'.ææ */
	$"8000 80FF 035C 0078 1B83 000E 2472 002F"            /* €.€ÿ.\.x.ƒ..$r./ */
	$"0516 2D18 5C58 3F5D 3D2A 0080 E606 8000"            /* ..-.\X?]=*.€æ.€. */
	$"FFC1 001E 6E85 000C 7316 0003 1629 1758"            /* ÿÁ..n…..s....).X */
	$"5840 5E3E 2A84 0003 5C00 3463 8500 176D"            /* X@^>*„..\.4c…..m */
	$"2800 2F12 2528 4F5A 4E60 4B3E 5B48 3B5A"            /* (./.%(OZN`K>[H;Z */
	$"4842 001E 0027 7185 0018 771D 001E 1212"            /* HB...'q…..w..... */
	$"324B 3B64 605B 325B 5A58 3559 3200 0700"            /* 2K;d`[2[ZX5Y2... */
	$"0787 1583 001A 1D84 0400 070E 112C 6169"            /* .‡.ƒ...„.....,ai */
	$"6567 6461 5E5B 595A 5B62 0007 0000 4386"            /* egda^[YZ[b....C† */
	$"0581 0016 0A8A 3900 0007 0318 3072 7B69"            /* ...ÂŠ9.....0r{i */
	$"6B6F 6C64 5F5F 5E5C 6300 1E80 0007 578D"            /* kold__^\c..€..W */
	$"4714 154B 8F4E 8000 111E 0508 122F 373D"            /* G..KN€....../7= */
	$"657A 7867 5E60 6664 6939 5C81 0005 2768"            /* ezxg^`fdi9\..'h */
	$"B1AC 6622 8100 112D 0206 101B 2B3A 1C5F"            /* ±¬f"..-....+:._ */
	$"796A 5F5E 6B70 6E00 C183 0001 7E72 8300"            /* yj_^kpn.Áƒ..~rƒ. */
	$"1201 0105 0509 0A22 3514 6164 616B 7B69"            /* .....ÆÂ"5.adak{i */
	$"6C00 FF5D 8200 017E 7082 0014 2E00 0102"            /* l.ÿ]‚..~p‚...... */
	$"0507 080A 3419 2661 5E6C 816A 7200 FFEC"            /* ...Â4.&a^ljr.ÿì */
	$"3D81 0001 7E71 8100 1621 0001 0000 0206"            /* =..~q..!...... */
	$"0607 152B 0A4F 5E5C 616F 7300 FFFF EB5F"            /* ...+ÂO^\aos.ÿÿë_ */
	$"8000 017E 7180 0013 3903 0102 0000 0104"            /* €..~q€..9....... */
	$"0505 0C29 0A33 605F 6471 7C00 81FF 1BBF"            /* ...)Â3`_dq|.ÿ.¿ */
	$"6728 7937 192C 310D 0502 0502 0303 0203"            /* g(y7.,1......... */
	$"0308 220A 205F 6569 6B7C 0084 FF80 0015"            /* .."Â _eik|.„ÿ€.. */
	$"0108 1205 020A 0A03 0302 0102 0517 0A15"            /* .....ÂÂ.......Â. */
	$"5E79 8168 7100 84FF 8100 140B 1504 0308"            /* ^yhq.„ÿ....... */
	$"0903 0404 0101 071A 0B10 6067 7071 6C00"            /* Æ.........`gpql. */
	$"84FF 8000 1509 1417 0503 040D 0404 0301"            /* „ÿ€..Æ.......... */
	$"0212 100A 1069 7B6A 716F 0084 FF18 0006"            /* ...Â.i{jqo.„ÿ... */
	$"0D1B 211D 0503 030C 1005 0405 0C0C 070C"            /* ..!............. */
	$"1471 786C 726F 0084 FF18 0019 1921 1F1A"            /* .qxlro.„ÿ....!.. */
	$"0702 0303 0A11 1A1F 0E02 060A 206B 757A"            /* ....Â......Â kuz */
	$"786F 0084 FF18 0015 0719 201A 0C02 0203"            /* xo.„ÿ..... ..... */
	$"0305 220E 0404 0809 3579 818A 7972 0084"            /* .."....Æ5yŠyr.„ */
	$"FF18 000E 051B 1C1A 1703 0304 0406 2A0C"            /* ÿ.............*. */
	$"0507 090A 5788 888B 8074 0084 FF18 0015"            /* ..ÆÂWˆˆ‹€t.„ÿ... */
	$"171C 171E 1D0E 0402 030E 260B 0508 0B2F"            /* ..........&..../ */
	$"7086 A88A 7F80 0084 FF18 001E 1E19 1213"            /* p†¨Š.€.„ÿ....... */
	$"1919 0503 0308 2008 0408 1272 7A8C 9685"            /* ...... ....rzŒ–… */
	$"8680 0084 FF18 001E 1E1A 1008 130D 0905"            /* †€.„ÿ.........Æ. */
	$"0307 1D06 030D 4F7B 8993 818A 807F 0084"            /* ......O{‰“Š€..„ */
	$"FF18 0017 1B15 0B03 1505 0412 0E09 1204"            /* ÿ............Æ.. */
	$"0F45 6883 9B84 93A0 767C 0084 FF18 000C"            /* .Ehƒ›„“ v|.„ÿ... */
	$"0D03 0000 0200 000A 1D1A 0B11 2A56 6B79"            /* .......Â....*Vky */
	$"8585 908B 7480 0084 FF02 0002 0183 000F"            /* ……‹t€.„ÿ....ƒ.. */
	$"0A0C 1211 0733 6775 7C87 928A 7170 7E00"            /* Â....3gu|‡’Šqp~. */
	$"84FF 9600 6C38 6D6B 0000 0408 0000 0000"            /* „ÿ–.l8mk........ */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿÿ.......... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÿ......... */
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 0000 0000 0000 00FF"            /* ÿÿÿÿÿÿÿÿ.......ÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿÿÿ.....ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿ...ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF00 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿ..ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ..ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 00FF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ...ÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF"                      /* ÿÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'icns' (137, "MIDI Music Icon") {
	$"6963 6E73 0000 15C4 4943 4E23 0000 0108"            /* icns...ÄICN#.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"01FF FFC0 01FF FFE0 07FF FFF0 1FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð.ÿÿø */
	$"3FFF FFFC 7FFF FFFE 7FFF FFFF FFFF FFFF"            /* ?ÿÿü.ÿÿþ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 7FFF FFFF 7FFF FFFF 3FFF FFFF"            /* ÿÿÿÿ.ÿÿÿ.ÿÿÿ?ÿÿÿ */
	$"1FFF FFFF 07FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 7FFF FFFF 7FFF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"7FFF FFFF 7FFF FFFF 7FFF FFFF 7FFF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"7FFF FFFF 7FFF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"6963 6C34 0000 0208 0000 000E DEED EEDE"            /* icl4........ÞíîÞ */
	$"EDEE EEEE EE00 0000 0000 000D 0000 0000"            /* íîîîî........... */
	$"0000 0000 0EE0 0000 0000 CDED EDEC 0000"            /* .....à....Íííì.. */
	$"C00C 00C0 0AEE 0000 000E ED8E 8EDE E000"            /* À..ÀÂî....íŽŽÞà. */
	$"0C00 C00C 0ECE E000 00AF C888 A8FD FE00"            /* ..À..Îà..¯Èˆ¨ýþ. */
	$"000C 00C0 CE0C EE00 0EFD 8888 899A DFEC"            /* ...ÀÎ.î..ýˆˆ‰šßì */
	$"00C0 0C00 0EC0 CEE0 C9AD 8B8B 8A9F D9F0"            /* .À...ÀÎàÉ­‹‹ŠŸÙð */
	$"C000 C00C 0FEE EEAF DF9D A888 99AF DE9E"            /* À.À..îî¯ß¨ˆ™¯Þž */
	$"000C 00C0 CCDD EDDE E9ED 89B9 9A9F D99E"            /* ...ÀÌÝíÞéí‰¹šŸÙž */
	$"0000 0C0C 00DD CDDE 899D A999 A9FE D99A"            /* .....ÝÍÞ‰©™©þÙš */
	$"00C0 C000 0C00 C0CF A8A8 DAF9 FFFD 8A99"            /* .ÀÀ...ÀÏ¨¨ÚùÿýŠ™ */
	$"0000 0C0C 00CC 0CDE 8A89 DDEE FECB A98A"            /* .....Ì.ÞŠ‰ÝîþË©Š */
	$"000C 00C0 0CC0 C0CF D888 98DC CB88 88AD"            /* ...À.ÀÀÏØˆ˜ÜËˆˆ­ */
	$"C000 C00C 000C 0CCF 0998 A88D D888 8A9C"            /* À.À....ÏÆ˜¨ØˆŠœ */
	$"00C0 0C00 C0C0 C0DE 0B88 88AB 1889 88D0"            /* .À..ÀÀÀÞ.ˆˆ«.‰ˆÐ */
	$"0C00 F0C0 0C0C 0CDE 008B 888D C888 AE0C"            /* ..ðÀ...Þ.‹ˆÈˆ®. */
	$"000C F000 C0C0 0CCF 000B 888D D889 D000"            /* ..ð.ÀÀ.Ï..ˆØ‰Ð. */
	$"0000 F0C0 0CF0 C0DF 0000 0DED BEBC 00F0"            /* ..ðÀ.ðÀß...í¾¼.ð */
	$"00C0 FC00 C0FC C0CF 0000 0009 0000 00F0"            /* .Àü.ÀüÀÏ...Æ...ð */
	$"C000 F00C 0CF0 0CDA 0000 000E 0000 00F0"            /* À.ð..ð.Ú.......ð */
	$"00C0 F00C 00F0 CCCF 0000 000A 0000 00F0"            /* .Àð..ðÌÏ...Â...ð */
	$"000C F0C0 C0FC 00DF 0000 000E 0DDD DDFD"            /* ..ðÀÀü.ß.....ÝÝý */
	$"DDDD FDDD DDFD DECF 0F9F AFFE FEAF EEAE"            /* ÝÝýÝÝýÞÏ.Ÿ¯þþ¯î® */
	$"FEEE F00C 0CF0 00DA 0E24 2242 4242 4242"            /* þîð..ð.Ú.$"BBBBB */
	$"423E FC00 C0FC C0DF 0F40 3320 2030 0003"            /* B>ü.ÀüÀß.@3  0.. */
	$"303E FDDD FFFD DDCF 0E20 0300 3030 3330"            /* 0>ýÝÿýÝÏ. ..0030 */
	$"303E 000F FFF0 00DF 0F20 3030 3030 3330"            /* 0>..ÿð.ß. 000030 */
	$"303E 0C0F FFFC CCCF 0E40 3030 3030 0003"            /* 0>..ÿüÌÏ.@0000.. */
	$"303E DDDD FFDD DDCF 0A33 3333 3333 3333"            /* 0>ÝÝÿÝÝÏÂ3333333 */
	$"33AE 00C0 0C00 C0DF 0FAE EFEE FEEF EEEE"            /* 3®.À..Àß.®ïîþïîî */
	$"EEEE C000 C0C0 C0DF 0000 000F 0000 0000"            /* îîÀ.ÀÀÀß........ */
	$"0000 0C00 0C0C 0CCF 0000 000E FFEF FFFF"            /* .......Ï....ÿïÿÿ */
	$"FFFF FFFF FFFF FFFF 6963 6C38 0000 0408"            /* ÿÿÿÿÿÿÿÿicl8.... */
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F5F5 00F5 F5F5 F5F5 F5AC FBFC 0000 0000"            /* õõ.õõõõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A082 FAAC FC00 00F5"            /* ..õû¬{|  ‚ú¬ü..õ */
	$"00F5 F5F5 F5F5 F5F6 F5FC F7FB AC00 0000"            /* .õõõõõõöõü÷û¬... */
	$"00F5 ACFE 50A1 A1A1 A7CB AD7A DFAC 0000"            /* .õ¬þP¡¡¡§Ë­zß¬.. */
	$"F500 F5F5 F5F5 F6F5 F6AC F5F7 FBAC 0000"            /* õ.õõõõöõö¬õ÷û¬.. */
	$"0081 FF7A 7D9B 9BA1 A1A7 E7AD FAEA FBF5"            /* .ÿz}››¡¡§ç­úêûõ */
	$"00F5 F5F5 F5F5 F5F5 F5AC F5F6 F7FB AC00"            /* .õõõõõõõõ¬õö÷û¬. */
	$"2BE9 AD57 C57D 9BA1 A1A7 E8E9 F9E8 E92B"            /* +é­WÅ}›¡¡§èéùèé+ */
	$"F5F5 F5F5 F5F5 F5F6 F5AC ACAC ACAC ACFD"            /* õõõõõõõöõ¬¬¬¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADEA FAA6 D1FB"            /* {éüu§¡¡¡§Ë­êú¦Ñû */
	$"00F5 00F5 F5F5 F6F5 F556 FAFA FAFA FAAC"            /* .õ.õõõöõõVúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBAD E8FF FAA7 E8A6"            /* ¦è¦ùË§¡§Ë­èÿú§è¦ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 5656 5656 56FD"            /* .õõõõõõöõõVVVVVý */
	$"A7A7 E8F9 A7E7 A7CB ADE8 EAFD 56E7 A7AD"            /* §§èù§ç§Ë­èêýVç§­ */
	$"F5F5 F5F5 F5F5 F5F5 F5F6 F6F5 F6F6 F8FD"            /* õõõõõõõõõööõööøý */
	$"A7A7 A7A6 56E9 E9D1 D1E0 EA56 A0A7 A7D0"            /* §§§¦VééÑÑàêV §§Ð */
	$"0000 F5F5 F5F5 F6F5 F6F5 F6F6 F5F6 56FD"            /* ..õõõõöõöõööõöVý */
	$"A0A7 E6A7 7B56 FCFD FEFC F87C A7A7 A7A7"            /*  §æ§{Vüýþüø|§§§§ */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F6F5 2B01 55FD"            /* .õõõõõõöõööõ+.Uý */
	$"57C5 A1A1 A7A0 572C 4F51 A1E6 A1A1 E7FA"            /* WÅ¡¡§ W,OQ¡æ¡¡çú */
	$"F500 F5F5 F5F5 F5F5 F6F5 F6F6 F6F6 56FD"            /* õ.õõõõõõöõööööVý */
	$"25A7 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E8F7"            /* %§¡¡¡¡æzQæ¡¡¡¡è÷ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 F6F5 F6F6 56FD"            /* .õõõõõõöõõöõööVý */
	$"0075 A1A1 A19B A757 51A1 A1A1 9BE7 FA00"            /* .u¡¡¡›§WQ¡¡¡›çú. */
	$"F5F5 F5F5 FFF5 F5F5 F6F6 F5F6 F6F6 56FD"            /* õõõõÿõõõööõöööVý */
	$"00F5 7CA1 9BA1 A157 4AA1 9B9B A7A6 00F5"            /* .õ|¡›¡¡WJ¡››§¦.õ */
	$"00F5 00F5 FFF5 F5F6 F5F5 F6F6 F5F6 F8FE"            /* .õ.õÿõõöõõööõöøþ */
	$"0000 F57B A19B E5F9 519B A7E7 8100 00F5"            /* ..õ{¡›åùQ›§ç..õ */
	$"F500 F5F5 FFF5 F5F5 F6F6 FFF5 F6F6 56FE"            /* õ.õõÿõõõööÿõööVþ */
	$"0000 0000 2B7B A6F9 57A6 7B2B 0000 FFF5"            /* ....+{¦ùW¦{+..ÿõ */
	$"00F5 F5F5 FFF5 F5F6 F5F5 FFF6 F6F5 56FE"            /* .õõõÿõõöõõÿööõVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 FF00"            /* .......¬......ÿ. */
	$"F500 F5F5 FFF5 F5F5 F6F6 FFF6 F6F6 56FE"            /* õ.õõÿõõõööÿöööVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 FFF5"            /* .......¬......ÿõ */
	$"00F5 F5F5 FFF5 F5F6 F5F5 FFF5 F6F6 F8FE"            /* .õõõÿõõöõõÿõööøþ */
	$"0000 0000 0000 00AC 0000 0000 00F5 FF00"            /* .......¬.....õÿ. */
	$"F500 F5F5 FFF5 F5F5 F6F6 FFF6 F5F6 56FE"            /* õ.õõÿõõõööÿöõöVþ */
	$"0000 0000 0000 00AC 00F9 FAFA FAF9 FFFA"            /* .......¬.ùúúúùÿú */
	$"FAFA FAF9 FFFA FAFA FAF9 FFF9 FAFA 56FE"            /* úúúùÿúúúúùÿùúúVþ */
	$"00FD FDFD FDFD FDAC FDAC ACAC ACAC ACFC"            /* .ýýýýýý¬ý¬¬¬¬¬¬ü */
	$"ACFC FCAC FF00 F5F5 F5F6 FFF5 F6F5 56FE"            /* ¬üü¬ÿ.õõõöÿõöõVþ */
	$"00FD 1C1C 1C16 1C1C 151C 1C1C 1C1C 1B1C"            /* .ý.............. */
	$"1C1C 23FC FFF5 F5F5 F6F5 FFF6 F6F6 56E0"            /* ..#üÿõõõöõÿöööVà */
	$"00FD 1B00 2323 2300 2300 2300 0000 0023"            /* .ý..###.#.#....# */
	$"2300 D9FC FFFA FAFA FFFF FFF9 FAF9 56EA"            /* #.ÙüÿúúúÿÿÿùúùVê */
	$"00FD 1C00 0023 0000 2300 2300 2323 2300"            /* .ý...#..#.#.###. */
	$"2300 47FC F5F5 F5FF FFFF FFF6 F5F6 F8F4"            /* #.Güõõõÿÿÿÿöõöøô */
	$"00FD 1600 2300 2300 2300 2300 2323 2300"            /* .ý..#.#.#.#.###. */
	$"2300 D9FC 00F5 F5FF FFFF FFF6 F6F6 56FF"            /* #.Ùü.õõÿÿÿÿöööVÿ */
	$"00AC 1C00 2300 2300 2300 2300 0000 0023"            /* .¬..#.#.#.#....# */
	$"2300 D9FB FAFA FAF9 FFFF F9FA F9FA F8FF"            /* #.Ùûúúúùÿÿùúùúøÿ */
	$"00FD 23D9 47D9 D947 D9D9 47D9 D947 D9D9"            /* .ý#ÙGÙÙGÙÙGÙÙGÙÙ */
	$"47D9 D9FC 00F5 F5F5 F5F6 F5F6 F6F5 56FF"            /* GÙÙü.õõõõöõööõVÿ */
	$"00AC ACAC ACAC ACAC ACAC FCAC FCFC FCFC"            /* .¬¬¬¬¬¬¬¬¬ü¬üüüü */
	$"FCFC FBFC F5F5 F5F5 F6F5 F6F5 F6F6 56FF"            /* üüûüõõõõöõöõööVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 0000"            /* .......ý........ */
	$"0000 F500 F5F5 F6F5 F5F6 F6F6 F5F6 56FF"            /* ..õ.õõöõõöööõöVÿ */
	$"0000 0000 0000 00FD FEFD FDFE FEFE FEFE"            /* .......ýþýýþþþþþ */
	$"FEFF FEFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* þÿþÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"696C 3332 0000 0A9C 84FF 125E 5D5C 5B5A"            /* il32..Âœ„ÿ.^]\[Z */
	$"5958 5756 5453 5150 4E4D 4B49 4846 8AFF"            /* YXWVTSQPNMKIHFŠÿ */
	$"135D FFFF FEFD FBFA F8F6 F5F3 F1EF EEEC"            /* .]ÿÿþýûúøöõóñïîì */
	$"EAE8 E744 4286 FF17 B76E 6968 6E66 6DB6"            /* êèçDB†ÿ.·nihnfm¶ */
	$"FBFA F8F6 F5F3 F1EF EEEC EAE8 E742 5942"            /* ûúøöõóñïîìêèçBYB */
	$"83FF 1AEC 5432 775E 4741 526F 2E54 FAF8"            /* ƒÿ.ìT2w^GARo.Túø */
	$"F6F5 F3F1 EFEE ECEA E8E7 40B3 5933 81FF"            /* öõóñïîìêèç@³Y3ÿ */
	$"1CED 391A 8B3D 2F30 2615 1E7E 1639 F8F6"            /* .í9.‹=/0&..~.9øö */
	$"F5F3 F1EF EEEC EAE8 E73D DEB3 5933 80FF"            /* õóñïîìêèç=Þ³Y3€ÿ */
	$"5861 007F 4937 413A 2F24 102B 7600 5FF6"            /* Xa..I7A:/$.+v._ö */
	$"F5F3 F1EF EEEC EAE8 E73B EEDE B359 33FF"            /* õóñïîìêèç;îÞ³Y3ÿ */
	$"C503 2481 293C 3E39 2E23 1904 7623 02C1"            /* Å.$)<>9.#..v#.Á */
	$"F5F3 F1EF EEEC EAE8 E739 3736 3432 312F"            /* õóñïîìêèç976421/ */
	$"7000 3B78 2133 3531 281F 1600 6E3A 0068"            /* p.;x!351(...n:.h */
	$"F5F3 F1EF EEEC EAE8 E79C 8275 1A2D 3E0F"            /* õóñïîìêèçœ‚u.->. */
	$"357D 1828 2927 211A 1100 7934 1034 F5F3"            /* 5}.()'!...y4.4õó */
	$"F1EF EEEC EAE8 E7E5 829C 7F2B 311D 1A8C"            /* ñïîìêèçå‚œ.+1..Œ */
	$"2617 1E1D 1913 031D 8C1B 1D24 F5F3 F1EF"            /* &.......Œ..$õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C29 3523 1953"            /* îìêèçåãâàßœ)5#.S */
	$"880A 090E 0B00 078C 501A 2326 F5F3 F1EF"            /* ˆÂÆ....ŒP.#&õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C27 4823 281C"            /* îìêèçåãâàßœ'H#(. */
	$"6890 4517 1548 9466 1D29 2236 F5F3 F1EF"            /* hE..H”f.)"6õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C25 7C20 2E2E"            /* îìêèçåãâàßœ%| .. */
	$"2248 7AB4 B57C 4623 2D2F 1769 F5F3 F1EF"            /* "Hz´µ|F#-/.iõóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C7F 23C9 2832"            /* îìêèçåãâàßœ.#É(2 */
	$"3333 2F13 888D 1C2E 3334 3112 C0F5 F3F1"            /* 33/.ˆ..341.Àõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 21FF 7726"            /* ïîìêèçåãâàßœ!ÿw& */
	$"3938 3924 888F 2C38 393A 1562 F6F5 F3F1"            /* 989$ˆ,89:.böõóñ */
	$"EF00 ECEA E8E7 E5E3 E2E0 DF9C 1FFF EE5B"            /* ï.ìêèçåãâàßœ.ÿî[ */
	$"283C 3F28 8993 323F 3B18 43F8 F6F5 F3F1"            /* (<?(‰“2?;.Cøöõóñ */
	$"EF00 ECEA E8E7 E5E3 E2E0 DF9C 1CFF FFED"            /* ï.ìêèçåãâàßœ.ÿÿí */
	$"722A 2F25 8994 2D26 1960 FAF8 F6F5 F3F1"            /* r*.%‰”-&.`úøöõóñ */
	$"EF00 ECEA E8E7 E500 E2E0 DF9C 001C 81FF"            /* ï.ìêèçå.âàßœ..ÿ */
	$"1BC3 6F2F 878C 346B BDFB FA00 F6F5 F3F1"            /* .Ão/‡Œ4k½ûú.öõóñ */
	$"EF00 ECEA E8E7 E500 E2E0 DF9C 1884 FF18"            /* ï.ìêèçå.âàßœ.„ÿ. */
	$"3DFF FFFE FDFB FA00 F6F5 F3F1 EF00 ECEA"            /* =ÿÿþýûú.öõóñï.ìê */
	$"E8E7 E500 E2E0 DF9C 1684 FF18 3BFF FFFE"            /* èçå.âàßœ.„ÿ.;ÿÿþ */
	$"FDFB FA00 F6F5 F3F1 EF00 ECEA E8E7 E500"            /* ýûú.öõóñï.ìêèçå. */
	$"E2E0 DF9C 1484 FF18 39FF FFFE FDFB FA00"            /* âàßœ.„ÿ.9ÿÿþýûú. */
	$"F6F5 F3F1 EF00 ECEA E8E7 E500 E2E0 DF9C"            /* öõóñï.ìêèçå.âàßœ */
	$"1284 FF01 37FF 8280 0000 8280 0000 8280"            /* .„ÿ.7ÿ‚€..‚€..‚€ */
	$"0000 8080 239C 10FF 2223 2527 292A 2C2D"            /* ..€€#œ.ÿ"#%')*,- */
	$"2F31 3234 3638 3A3B 3D3F 4100 ECEA E8E7"            /* /12468:;=?A.ìêèç */
	$"E500 E2E0 DF9C 10FF 238E FF0E 4400 ECEA"            /* å.âàßœ.ÿ#Žÿ.D.ìê */
	$"E8E7 E500 E2E0 DF9C 0CFF 258D FF02 BF46"            /* èçå.âàßœ.ÿ%ÿ.¿F */
	$"0080 8080 0080 8003 9C0A FF27 8DFF 04BF"            /* .€€€.€€.œÂÿ'ÿ.¿ */
	$"48EE ECEA 8100 06E2 E0DF 9C08 FF29 8DFF"            /* Hîìê..âàßœ.ÿ)ÿ */
	$"04BF 49EE ECEA 8100 06E2 E0DF 9C06 FF2A"            /* .¿Iîìê..âàßœ.ÿ* */
	$"8DFF 01BF 4B81 8001 0000 8180 049C 05FF"            /* ÿ.¿K€...€.œ.ÿ */
	$"2CFF 8DBF 2C4D EEEC EAE8 E7E5 E3E2 E0DF"            /* ,ÿ¿,Mîìêèçåãâàß */
	$"9C03 FF2D 2F31 3234 3638 3A3B 3D3F 4144"            /* œ.ÿ-/12468:;=?AD */
	$"4648 494B 4D4F EEEC EAE8 E7E5 E3E2 E0DF"            /* FHIKMOîìêèçåãâàß */
	$"9C03 84FF 1824 FFFF FEFD FBFA F8F6 F5F3"            /* œ.„ÿ.$ÿÿþýûúøöõó */
	$"F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C01 84FF"            /* ñïîìêèçåãâàßœ.„ÿ */
	$"1822 201F 1D1B 1A18 1615 1311 100E 0D0B"            /* ." ............. */
	$"0A08 0706 0504 0302 0100 84FF 125E 5D5C"            /* Â.........„ÿ.^]\ */
	$"5B5A 5958 5756 5453 5150 4E4D 4B49 4846"            /* [ZYXWVTSQPNMKIHF */
	$"8AFF 135D FFFF FEFD FBFA F8F6 F5F3 F1EF"            /* Šÿ.]ÿÿþýûúøöõóñï */
	$"EEEC EAE8 E744 4286 FF17 B66A 676C 7165"            /* îìêèçDB†ÿ.¶jglqe */
	$"6CB7 FBFA F8F6 F5F3 F1EF EEEC EAE8 E742"            /* l·ûúøöõóñïîìêèçB */
	$"5942 83FF 1AED 552F 94A6 9380 7978 2D55"            /* YBƒÿ.íU/”¦“€yx-U */
	$"FAF8 F6F5 F3F1 EFEE ECEA E8E7 40B3 5933"            /* úøöõóñïîìêèç@³Y3 */
	$"81FF 1CEE 3E19 B6AD A598 7C58 4886 1B3D"            /* ÿ.î>.¶­¥˜|XH†.= */
	$"F8F6 F5F3 F1EF EEEC EAE8 E73D DEB3 5933"            /* øöõóñïîìêèç=Þ³Y3 */
	$"80FF 586F 0090 ABB2 BEAC 8A69 423E 7E06"            /* €ÿXo.«²¾¬ŠiB>~. */
	$"67F6 F5F3 F1EF EEEC EAE8 E73B EEDE B359"            /* göõóñïîìêèç;îÞ³Y */
	$"33FF CC22 33A8 91B1 B8A7 8767 4A1C 7B42"            /* 3ÿÌ"3¨‘±¸§‡gJ.{B */
	$"19C4 F5F3 F1EF EEEC EAE8 E739 3736 3432"            /* .Äõóñïîìêèç97642 */
	$"312F 882A 4F9F 7B97 9C8F 765D 4212 735F"            /* 1/ˆ*OŸ{—œv]B.s_ */
	$"2875 F5F3 F1EF EEEC EAE8 E79C 8275 1A2D"            /* (uõóñïîìêèçœ‚u.- */
	$"6744 5398 5F76 7971 614C 3308 7E64 454C"            /* gDS˜_vyqaL3.~dEL */
	$"F5F3 F1EF EEEC EAE8 E7E5 829C 7F2B 665A"            /* õóñïîìêèçå‚œ.+fZ */
	$"4E9C 5253 5B55 4936 161B A05A 5A44 F5F3"            /* NœRS[UI6.. ZZDõó */
	$"F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C29 7168"            /* ñïîìêèçåãâàßœ)qh */
	$"5F7C 922A 2E32 270F 038D 8B62 6846 F5F3"            /* _|’*.2'..‹bhFõó */
	$"F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C27 8774"            /* ñïîìêèçåãâàßœ'‡t */
	$"786B 9797 4521 1941 9CA1 7078 6F53 F5F3"            /* xk——E!.Aœ¡pxoSõó */
	$"F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C25 AD7E"            /* ñïîìêèçåãâàßœ%­~ */
	$"8687 7E96 A6B8 C8B4 9980 868B 6477 F5F3"            /* †‡~–¦¸È´™€†‹dwõó */
	$"F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C7F 23DD"            /* ñïîìêèçåãâàßœ.#Ý */
	$"8B98 9798 9873 9BC8 8B94 9799 9748 C0F5"            /* ‹˜—˜˜s›È‹”—™—HÀõ */
	$"F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C 21FE"            /* óñïîìêèçåãâàßœ!þ */
	$"AF99 A9A6 A987 9CCE A1A6 A8AC 666F F6F5"            /* ¯™©¦©‡œÎ¡¦¨¬foöõ */
	$"F3F1 EF00 ECEA E8E7 E5E3 E2E0 DF9C 1FFF"            /* óñï.ìêèçåãâàßœ.ÿ */
	$"F2A1 A1B7 BA94 9FD6 B1B9 B470 5AF8 F6F5"            /* ò¡¡·º”ŸÖ±¹´pZøöõ */
	$"F3F1 EF00 ECEA E8E7 E5E3 E2E0 DF9C 1CFF"            /* óñï.ìêèçåãâàßœ.ÿ */
	$"FFEF A691 AC97 A1DB AF91 5B71 FAF8 F6F5"            /* ÿï¦‘¬—¡Û¯‘[qúøöõ */
	$"F3F1 EF00 ECEA E8E7 E500 E2E0 DF9C 001C"            /* óñï.ìêèçå.âàßœ.. */
	$"80FF 1CFE CF98 6593 AF66 80BE FBFA 00F6"            /* €ÿ.þÏ˜e“¯f€¾ûú.ö */
	$"F5F3 F1EF 00EC EAE8 E7E5 00E2 E0DF 9C18"            /* õóñï.ìêèçå.âàßœ. */
	$"84FF 183D FFFF FEFD FBFA 00F6 F5F3 F1EF"            /* „ÿ.=ÿÿþýûú.öõóñï */
	$"00EC EAE8 E7E5 00E2 E0DF 9C16 84FF 183B"            /* .ìêèçå.âàßœ.„ÿ.; */
	$"FFFF FEFD FBFA 00F6 F5F3 F1EF 00EC EAE8"            /* ÿÿþýûú.öõóñï.ìêè */
	$"E7E5 00E2 E0DF 9C14 84FF 1839 FFFF FEFD"            /* çå.âàßœ.„ÿ.9ÿÿþý */
	$"FBFA 00F6 F5F3 F1EF 00EC EAE8 E7E5 00E2"            /* ûú.öõóñï.ìêèçå.â */
	$"E0DF 9C12 84FF 0137 FF82 8000 0082 8000"            /* àßœ.„ÿ.7ÿ‚€..‚€. */
	$"0082 8000 0080 8023 9C10 FF22 2325 2729"            /* .‚€..€€#œ.ÿ"#%') */
	$"2A2C 2D2F 3132 3436 383A 3B3D 3F41 00EC"            /* *,-/12468:;=?A.ì */
	$"EAE8 E7E5 00E2 E0DF 9C10 FF23 8D40 1100"            /* êèçå.âàßœ.ÿ#@.. */
	$"4400 ECEA E8E7 E500 E2E0 DF9C 0CFF 2540"            /* D.ìêèçå.âàßœ.ÿ%@ */
	$"FF80 0003 FF00 FF00 81FF 0500 00FF 0046"            /* ÿ€..ÿ.ÿ.ÿ...ÿ.F */
	$"0080 8080 0080 800D 9C0A FF27 40FF FF00"            /* .€€€.€€.œÂÿ'@ÿÿ. */
	$"FFFF 00FF 00FF 8000 07FF 00FF 0048 EEEC"            /* ÿÿ.ÿ.ÿ€..ÿ.ÿ.Hîì */
	$"EA81 0010 E2E0 DF9C 08FF 2940 FF00 FF00"            /* ê..âàßœ.ÿ)@ÿ.ÿ. */
	$"FF00 FF00 FF80 0007 FF00 FF00 49EE ECEA"            /* ÿ.ÿ.ÿ€..ÿ.ÿ.Iîìê */
	$"8100 0FE2 E0DF 9C06 FF2A 40FF 00FF 00FF"            /* ..âàßœ.ÿ*@ÿ.ÿ.ÿ */
	$"00FF 0081 FF04 0000 FF00 4B81 8001 0000"            /* .ÿ.ÿ...ÿ.K€... */
	$"8180 039C 05FF 2C8E 002C 4DEE ECEA E8E7"            /* €.œ.ÿ,Ž.,Mîìêèç */
	$"E5E3 E2E0 DF9C 03FF 2D2F 3132 3436 383A"            /* åãâàßœ.ÿ-/12468: */
	$"3B3D 3F41 4446 4849 4B4D 4FEE ECEA E8E7"            /* ;=?ADFHIKMOîìêèç */
	$"E5E3 E2E0 DF9C 0384 FF18 24FF FFFE FDFB"            /* åãâàßœ.„ÿ.$ÿÿþýû */
	$"FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0"            /* úøöõóñïîìêèçåãâà */
	$"DF9C 0184 FF18 2220 1F1D 1B1A 1816 1513"            /* ßœ.„ÿ." ........ */
	$"1110 0E0D 0B0A 0807 0605 0403 0201 0084"            /* .....Â.........„ */
	$"FF12 5E5D 5C5B 5A59 5857 5654 5351 504E"            /* ÿ.^]\[ZYXWVTSQPN */
	$"4D4B 4948 468A FF13 5DFF FFFE FDFB FAF8"            /* MKIHFŠÿ.]ÿÿþýûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 4442 86FF 17B7"            /* öõóñïîìêèçDB†ÿ.· */
	$"7069 666D 666E B6FB FAF8 F6F5 F3F1 EFEE"            /* pifmfn¶ûúøöõóñïî */
	$"ECEA E8E7 4259 4283 FF1A EC54 3469 4423"            /* ìêèçBYBƒÿ.ìT4iD# */
	$"2447 6B2E 54FA F8F6 F5F3 F1EF EEEC EAE8"            /* $Gk.Túøöõóñïîìêè */
	$"E740 B359 3381 FF04 EC37 1C78 0F81 0013"            /* ç@³Y3ÿ.ì7.x... */
	$"147B 1539 F8F6 F5F3 F1EF EEEC EAE8 E73D"            /* .{.9øöõóñïîìêèç= */
	$"DEB3 5933 80FF 035C 0078 1B83 0018 2472"            /* Þ³Y3€ÿ.\.x.ƒ..$r */
	$"005C F6F5 F3F1 EFEE ECEA E8E7 3BEE DEB3"            /* .\öõóñïîìêèç;îÞ³ */
	$"5933 FFC1 001E 6E85 0017 7316 00C0 F5F3"            /* Y3ÿÁ..n…..s..Àõó */
	$"F1EF EEEC EAE8 E739 3736 3432 312F 6700"            /* ñïîìêèç976421/g. */
	$"3463 8500 0D6D 2800 66F5 F3F1 EFEE ECEA"            /* 4c…..m(.fõóñïîìê */
	$"E8E7 9C82 7504 2D2D 0027 7185 000D 771D"            /* èçœ‚u.--.'q…..w. */
	$"002C F5F3 F1EF EEEC EAE8 E7E5 829C 052B"            /* .,õóñïîìêèçå‚œ.+ */
	$"1600 0787 1583 001A 1D84 0400 15F5 F3F1"            /* ...‡.ƒ...„...õóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 2916 0000"            /* ïîìêèçåãâàßœ)... */
	$"4386 0581 0016 0A8A 3900 0015 F5F3 F1EF"            /* C†...ÂŠ9...õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C27 2D80 0007"            /* îìêèçåãâàßœ'-€.. */
	$"578D 4714 154B 8F4E 8000 112C F5F3 F1EF"            /* WG..KN€..,õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C48 6781 0005"            /* îìêèçåãâàßœHg.. */
	$"2768 B1AC 6622 8100 1166 F5F3 F1EF EEEC"            /* 'h±¬f"..fõóñïîì */
	$"EAE8 E7E5 E3E2 E0DF 9C23 C183 0001 7E72"            /* êèçåãâàßœ#Áƒ..~r */
	$"8300 12C0 F5F3 F1EF EEEC EAE8 E7E5 E3E2"            /* ƒ..Àõóñïîìêèçåãâ */
	$"E0DF 9C21 FF5D 8200 017E 7082 0014 5DF6"            /* àßœ!ÿ]‚..~p‚..]ö */
	$"F5F3 F1EF 00EC EAE8 E7E5 E3E2 E0DF 9C1F"            /* õóñï.ìêèçåãâàßœ. */
	$"FFEC 3D81 0001 7E71 8100 163D F8F6 F5F3"            /* ÿì=..~q..=øöõó */
	$"F1EF 00EC EAE8 E7E5 E3E2 E0DF 9C1C FFFF"            /* ñï.ìêèçåãâàßœ.ÿÿ */
	$"EB5F 8000 017E 7180 0013 5FFA F8F6 F5F3"            /* ë_€..~q€.._úøöõó */
	$"F1EF 00EC EAE8 E7E5 00E2 E0DF 9C1C 81FF"            /* ñï.ìêèçå.âàßœ.ÿ */
	$"1BBF 6021 807B 2766 BEFB FA00 F6F5 F3F1"            /* .¿`!€{'f¾ûú.öõóñ */
	$"EF00 ECEA E8E7 E500 E2E0 DF9C 1884 FF18"            /* ï.ìêèçå.âàßœ.„ÿ. */
	$"3DFF FFFE FDFB FA00 F6F5 F3F1 EF00 ECEA"            /* =ÿÿþýûú.öõóñï.ìê */
	$"E8E7 E500 E2E0 DF9C 1684 FF18 3BFF FFFE"            /* èçå.âàßœ.„ÿ.;ÿÿþ */
	$"FDFB FA00 F6F5 F3F1 EF00 ECEA E8E7 E500"            /* ýûú.öõóñï.ìêèçå. */
	$"E2E0 DF9C 1484 FF18 39FF FFFE FDFB FA00"            /* âàßœ.„ÿ.9ÿÿþýûú. */
	$"F6F5 F3F1 EF00 ECEA E8E7 E500 E2E0 DF9C"            /* öõóñï.ìêèçå.âàßœ */
	$"1284 FF01 37FF 8280 0000 8280 0000 8280"            /* .„ÿ.7ÿ‚€..‚€..‚€ */
	$"0000 8080 239C 10FF 2223 2527 292A 2C2D"            /* ..€€#œ.ÿ"#%')*,- */
	$"2F31 3234 3638 3A3B 3D3F 4100 ECEA E8E7"            /* /12468:;=?A.ìêèç */
	$"E500 E2E0 DF9C 10FF 238D 4011 0044 00EC"            /* å.âàßœ.ÿ#@..D.ì */
	$"EAE8 E7E5 00E2 E0DF 9C0C FF25 40FF 8000"            /* êèçå.âàßœ.ÿ%@ÿ€. */
	$"03FF 00FF 0081 FF05 0000 FF00 4600 8080"            /* .ÿ.ÿ.ÿ...ÿ.F.€€ */
	$"8000 8080 0D9C 0AFF 2740 FFFF 00FF FF00"            /* €.€€.œÂÿ'@ÿÿ.ÿÿ. */
	$"FF00 FF80 0007 FF00 FF00 48EE ECEA 8100"            /* ÿ.ÿ€..ÿ.ÿ.Hîìê. */
	$"10E2 E0DF 9C08 FF29 40FF 00FF 00FF 00FF"            /* .âàßœ.ÿ)@ÿ.ÿ.ÿ.ÿ */
	$"00FF 8000 07FF 00FF 0049 EEEC EA81 000F"            /* .ÿ€..ÿ.ÿ.Iîìê.. */
	$"E2E0 DF9C 06FF 2A40 FF00 FF00 FF00 FF00"            /* âàßœ.ÿ*@ÿ.ÿ.ÿ.ÿ. */
	$"81FF 0400 00FF 004B 8180 0100 0081 8003"            /* ÿ...ÿ.K€...€. */
	$"9C05 FF2C 8E00 2C4D EEEC EAE8 E7E5 E3E2"            /* œ.ÿ,Ž.,Mîìêèçåãâ */
	$"E0DF 9C03 FF2D 2F31 3234 3638 3A3B 3D3F"            /* àßœ.ÿ-/12468:;=? */
	$"4144 4648 494B 4D4F EEEC EAE8 E7E5 E3E2"            /* ADFHIKMOîìêèçåãâ */
	$"E0DF 9C03 84FF 1824 FFFF FEFD FBFA F8F6"            /* àßœ.„ÿ.$ÿÿþýûúøö */
	$"F5F3 F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C01"            /* õóñïîìêèçåãâàßœ. */
	$"84FF 1822 201F 1D1B 1A18 1615 1311 100E"            /* „ÿ." ........... */
	$"0D0B 0A08 0706 0504 0302 0100 6C38 6D6B"            /* ..Â.........l8mk */
	$"0000 0408 0000 0000 0000 00FF FFFF FFFF"            /* ...........ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 0000 0000 0000 0000 00FF FFFF FFFF"            /* ...........ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 0000 0000 0000 00FF FFFF FFFF FFFF"            /* .........ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 FFFF FFFF FFFF FFFF FFFF"            /* ÿ.....ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 0000 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿ...ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF00 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿ..ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ..ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 00FF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ...ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 0000 00FF FFFF FFFF FFFF"            /* ÿÿÿÿ.....ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 0000 0000 00FF FFFF FFFF"            /* ÿÿÿÿ.......ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 0000 0000 00FF FFFF FFFF"            /* ÿÿÿÿ.......ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 0000 0000 00FF FFFF FFFF"            /* ÿÿÿÿ.......ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 0000 0000 00FF FFFF FFFF"            /* ÿÿÿÿ.......ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿ.ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 0000 0000 00FF FFFF FFFF"            /* ÿÿÿÿ.......ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 0000 0000 0000 00FF FFFF FFFF"            /* ÿÿÿÿ.......ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF"                                          /* ÿÿÿÿ */
};

data 'icns' (138, "MML Script Icon") {
	$"6963 6E73 0000 1611 4943 4E23 0000 0108"            /* icns....ICN#.... */
	$"016D BBC0 0100 0020 05A0 0070 1B58 3C48"            /* .m»À... . .p.X<H */
	$"34EC 4254 35B6 8546 6AFB 177B EDB7 2555"            /* 4ìBT5¶…Fjû.{í·%U */
	$"56FA 5CAB EFF7 AB01 B7EB 5A02 7556 B491"            /* Vú\«ï÷«.·ëZ.uV´‘ */
	$"AAAD 6845 5D5B D121 266D 2013 1A9A 4419"            /* ª­hE][Ñ!&m ..šD. */
	$"0D69 200B 0256 400D 0109 088B 0106 002D"            /* .i ..V@..Æ.‹...- */
	$"0102 8013 0101 00B5 0100 9263 0101 C0D1"            /* ..€....µ..’c..ÀÑ */
	$"0103 4303 0106 A349 0105 2C83 0106 2A01"            /* ..C...£I..,ƒ..*. */
	$"0102 B423 0101 E801 0100 510B 01FF FFFF"            /* ..´#..è...Q..ÿÿÿ */
	$"01FF FFC0 01FF FFE0 07FF FFF0 1FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð.ÿÿø */
	$"3FFF FFFC 7FFF FFFE 7FFF FFFF FFFF FFFF"            /* ?ÿÿü.ÿÿþ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 7FFF FFFF 7FFF FFFF 3FFF FFFF"            /* ÿÿÿÿ.ÿÿÿ.ÿÿÿ?ÿÿÿ */
	$"1FFF FFFF 07FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"6963 6C34 0000 0208 0000 000E DEED EEDE"            /* icl4........ÞíîÞ */
	$"EDEE EEEE EE00 0000 0000 000D 0000 0000"            /* íîîîî........... */
	$"0000 0000 0EE0 0000 0000 CDED EDEC 0000"            /* .....à....Íííì.. */
	$"C00C 00C0 0AEE 0000 000E ED8E 8EDE E000"            /* À..ÀÂî....íŽŽÞà. */
	$"00FE EACC 0ECE E000 00AF C888 A8FD FE0C"            /* .þêÌ.Îà..¯Èˆ¨ýþ. */
	$"0E00 CCEC CE0C EE00 0EFD 8888 8A99 DFE0"            /* ..ÌìÎ.î..ýˆˆŠ™ßà */
	$"A00C CDDE CEC0 CEE0 C9AD 8B88 89AF DAFE"            /*  .ÍÞÎÀÎàÉ­‹ˆ‰¯Úþ */
	$"00CC DDEE DEEF EEAF DFED 98B8 A99F D99E"            /* .ÌÝîÞïî¯ßí˜¸©ŸÙž */
	$"0CCD DDED CDDD DDDE 999C 998A 8AFF DE9E"            /* .ÍÝíÍÝÝÞ™œ™ŠŠÿÞž */
	$"CCDD EFDD C0DD DDDE 8A9D 99A8 F9FA D999"            /* ÌÝïÝÀÝÝÞŠ™¨ùúÙ™ */
	$"CDDE EDFC 0C00 00CF A8AE CF9F 9FFC 8A9A"            /* ÍÞíü...Ï¨®ÏŸŸüŠš */
	$"CDDF DDCF C0C0 CCCF 8998 DDEA FEC8 98A9"            /* ÍßÝÏÀÀÌÏ‰˜ÝêþÈ˜© */
	$"DDFD DDCC ED0C 00DE D889 A8DC CC8A 889D"            /* ÝýÝÌí..ÞØ‰¨ÜÌŠˆ */
	$"EFDD DCC0 CECC C0DA C8A8 888D D8A8 899D"            /* ïÝÜÀÎÌÀÚÈ¨ˆØ¨‰ */
	$"FDDD C0CC 00FC 0CCF 0D88 888B 1888 B9BF"            /* ýÝÀÌ.ü.Ï.ˆˆ‹.ˆ¹¿ */
	$"CDDC CC00 C00F C0DE 00B8 B88D C888 9BDC"            /* ÍÜÌ.À.ÀÞ.¸¸Èˆ›Ü */
	$"DCC0 CC0C 000F DCCF 000B 888D D88A DDDD"            /* ÜÀÌ...ÜÏ..ˆØŠÝÝ */
	$"CCCC 00C0 0C00 FCDF 0000 CDED BED9 DCDC"            /* ÌÌ.À..üß..Íí¾ÙÜÜ */
	$"CCCC 0C00 C0CC EDDE 0000 000E 0000 FDCC"            /* ÌÌ..ÀÌíÞ......ýÌ */
	$"CC0C 000C 0C0C FDDF 0000 000E 0000 CECC"            /* Ì.....ýß......ÎÌ */
	$"C0C0 C000 C0CC FDCF 0000 0009 0000 0CFC"            /* ÀÀÀ.ÀÌýÏ...Æ...ü */
	$"CC00 0C0C 0CCF DCDF 0000 000E 0000 000F"            /* Ì....ÏÜß........ */
	$"00C0 C0C0 CCFD CDCF 0000 000A 0000 0C0C"            /* .ÀÀÀÌýÍÏ...Â.... */
	$"F00C 0C0C CFDD C0DF 0000 000F 0000 000F"            /* ð...ÏÝÀß........ */
	$"DF00 00CC FDCC CCCF 0000 000E 0000 00FD"            /* ß..ÌýÌÌÏ.......ý */
	$"DF0C C0CF DDC0 0CDF 0000 000F 0000 0FDD"            /* ß.ÀÏÝÀ.ß.......Ý */
	$"DCFC 0CFD DC0C C0CF 0000 000E 0000 0FDD"            /* Üü.ýÜ.ÀÏ.......Ý */
	$"CCF0 CFDC C0C0 CCCF 0000 000F 0000 0ADD"            /* ÌðÏÜÀÀÌÏ......ÂÝ */
	$"C0FC FDDC 0C0C 00DF 0000 000E 0000 0CFC"            /* ÀüýÜ...ß.......ü */
	$"C0FF DCCC 0C00 C0DF 0000 000F 0000 000E"            /* ÀÿÜÌ..Àß........ */
	$"FFFD DC00 C00C 0CCF 0000 000A 0000 00CC"            /* ÿýÜ.À..Ï...Â...Ì */
	$"CDCD 0C0C 0CC0 C0DF 0000 000F EFFF FFEF"            /* ÍÍ...ÀÀß....ïÿÿï */
	$"FEFF FFFF FFFF FFFF 6963 6C38 0000 0408"            /* þÿÿÿÿÿÿÿicl8.... */
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F5F5 00F5 F5F5 F5F5 F5AC FBFC 0000 0000"            /* õõ.õõõõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A082 FAAC FC00 00F5"            /* ..õû¬{|  ‚ú¬ü..õ */
	$"0000 ACAC ACFD 2BF6 F5FC F7FB AC00 0000"            /* ..¬¬¬ý+öõü÷û¬... */
	$"00F5 ACFE 50A1 A1A1 A7CB AD7A E0AC 00F5"            /* .õ¬þP¡¡¡§Ë­zà¬.õ */
	$"F5AC 00F5 F6F7 FD2B F6AC F5F8 FBAC 0000"            /* õ¬.õö÷ý+ö¬õøû¬.. */
	$"0081 FF7A 7D9B 9BA1 A1A7 E7AD FAEA FB00"            /* .ÿz}››¡¡§ç­úêû. */
	$"AC00 F5F6 F856 F9FD 2BAC F5F5 F7FB AC00"            /* ¬.õöøVùý+¬õõ÷û¬. */
	$"2BE9 AD57 C57D 9BA1 A1A7 E8E9 F9AD E9AC"            /* +é­WÅ}›¡¡§èéù­é¬ */
	$"00F5 2BF7 56FA FBAC 56AC ACFD ACAC ACFD"            /* .õ+÷Vúû¬V¬¬ý¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADEA 81A6 D181"            /* {éüu§¡¡¡§Ë­ê¦Ñ */
	$"F52B F756 FA81 ACFA 56F9 FAFA FAFA FAAC"            /* õ+÷Vú¬úVùúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBA7 D1FF FAA6 E8A6"            /* ¦è¦ùË§¡§Ë§Ñÿú¦è¦ */
	$"2BF7 56FA FBFD F956 2BF5 5656 5656 56FD"            /* +÷VúûýùV+õVVVVVý */
	$"A7A7 E8F9 A7CB ADCB ADD1 EAFD F9A7 E7AD"            /* §§èù§Ë­Ë­Ñêýù§ç­ */
	$"F756 FAFB AC81 FDF8 F6F6 F5F6 F5F6 F8FD"            /* ÷Vúû¬ýøööõöõöøý */
	$"A7A7 A7A6 56E9 E8E9 D1E0 EA56 A0A7 A7E8"            /* §§§¦VéèéÑàêV §§è */
	$"F8FA 81FD 81FA 56FD 2BF6 F6F5 F6F6 56FD"            /* øúýúVý+ööõööVý */
	$"A0A7 E6A7 7B56 FCFD FEFC F87C A7E6 A7A6"            /*  §æ§{Vüýþüø|§æ§¦ */
	$"FA81 FD81 FA56 F72B FDF7 F6F6 F6F6 F8FD"            /* úýúV÷+ý÷ööööøý */
	$"57C5 A1A1 A7A0 572C 4F51 A1A7 A1A1 E7FA"            /* WÅ¡¡§ W,OQ¡§¡¡çú */
	$"FBFD 81F9 56F7 2BF6 F6FD F7F6 F6F6 56FD"            /* ûýùV÷+ööý÷öööVý */
	$"2BA1 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E881"            /* +¡¡¡¡¡æzQæ¡¡¡¡è */
	$"FD81 FA56 F72B 2BF6 F5F5 FDF7 F6F5 56FD"            /* ýúV÷++öõõý÷öõVý */
	$"007B A1A1 9BA1 A157 51A1 A19B A1A7 81FD"            /* .{¡¡›¡¡WQ¡¡›¡§ý */
	$"F956 56F7 2B2B F606 F5F5 F5FE F7F6 56FD"            /* ùVV÷++ö.õõõþ÷öVý */
	$"0000 7CA1 A19B A157 509B 9BA1 CBA6 5656"            /* ..|¡¡›¡WP››¡Ë¦VV */
	$"56F8 F72B 2B2A F5F5 24F5 00FE 562B F8FE"            /* Vø÷++*õõ$õ.þV+øþ */
	$"0000 F575 A1A1 C5F9 4BA1 A1A7 8156 5656"            /* ..õu¡¡ÅùK¡¡§VVV */
	$"F8F7 F72B F6F5 F5F5 06F5 F5F6 FEF7 56FE"            /* ø÷÷+öõõõ.õõöþ÷Vþ */
	$"0000 0000 2B7B A6F9 57A6 FAFD F956 56F8"            /* ....+{¦ùW¦úýùVVø */
	$"F7F7 2B2A F6F5 F5F5 F5F5 2AF6 FE56 F9FD"            /* ÷÷+*öõõõõõ*öþVùý */
	$"0000 0000 0000 00AC 0000 00F5 FD56 F8F7"            /* .......¬...õýVø÷ */
	$"F7F6 F6F6 F5F5 F5F5 F5F6 F62B F4F9 F9FE"            /* ÷öööõõõõõöö+ôùùþ */
	$"0000 0000 0000 00AC 0000 0000 F6FD F731"            /* .......¬....öý÷1 */
	$"2BF6 F6F5 F506 F5F5 F6F6 2BF7 FEF9 56FE"            /* +ööõõ.õõöö+÷þùVþ */
	$"0000 0000 0000 00AC 0000 0000 F5F6 EAF6"            /* .......¬....õöêö */
	$"2BF6 0624 F5F5 F5F6 F62B F7F4 F956 F9FE"            /* +ö.$õõõöö+÷ôùVùþ */
	$"0000 0000 0000 00AC 0000 0000 0024 06EA"            /* .......¬.....$.ê */
	$"F6F5 F5F5 F5F5 F6F6 2BF7 FFF9 F9F7 56FE"            /* öõõõõõöö+÷ÿùù÷Vþ */
	$"0000 0000 0000 00AC 0000 0000 00F5 F5F6"            /* .......¬.....õõö */
	$"FEF5 F5F5 F5F6 F62B F7FE FA56 F7F6 56EA"            /* þõõõõöö+÷þúV÷öVê */
	$"0000 0000 0000 00AC 0000 0000 0000 00FE"            /* .......¬.......þ */
	$"81FE F5F5 F5F6 2A2B FEFA 56F7 F6F6 F8E0"            /* þõõõö*+þúV÷ööøà */
	$"0000 0000 0000 00AC 0000 0000 F500 FE81"            /* .......¬....õ.þ */
	$"FAFE F5F5 2AF6 F7FF FA56 2BF6 F6F6 56EA"            /* úþõõ*ö÷ÿúV+öööVê */
	$"0000 0000 0000 00FD 0000 0000 00FE 81FA"            /* .......ý.....þú */
	$"F9F7 F4F6 F62B E0FA 562B F6F6 F6F6 F8F4"            /* ù÷ôöö+àúV+ööööøô */
	$"0000 0000 0000 00AC 0000 0000 00E0 FA56"            /* .......¬.....àúV */
	$"F8F7 FEF6 F7FF F956 2BF6 F6F5 F6F6 56FF"            /* ø÷þö÷ÿùV+ööõööVÿ */
	$"0000 0000 0000 00FD 0000 0000 00FD F9F8"            /* .......ý.....ýùø */
	$"F7F5 F42B FEFA 562B F6F6 F5F6 F6F5 56FF"            /* ÷õô+þúV+ööõööõVÿ */
	$"0000 0000 0000 00FD 0000 0000 F5F5 EAF7"            /* .......ý....õõê÷ */
	$"F6F5 FEFF F956 2BF6 F5F6 F6F5 F6F6 56FF"            /* öõþÿùV+öõööõööVÿ */
	$"0000 0000 0000 00FD 0000 0000 00F5 F6FE"            /* .......ý.....õöþ */
	$"FEF4 FFF9 562B F6F5 F6F5 F5F6 F6F6 F8FF"            /* þôÿùV+öõöõõöööøÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F52B"            /* .......ý......õ+ */
	$"F856 5656 2BF5 F5F6 F5F6 F6F5 F6F6 56FF"            /* øVVV+õõöõööõööVÿ */
	$"0000 0000 0000 00FD FDFD FEFE FEFE FEFE"            /* .......ýýýþþþþþþ */
	$"E0FE EAE0 EAFF FFFF FFFF FFFF FFFF FFFF"            /* àþêàêÿÿÿÿÿÿÿÿÿÿÿ */
	$"696C 3332 0000 0AE9 84FF 125E 5D5C 5B5A"            /* il32..Âé„ÿ.^]\[Z */
	$"5958 5756 5453 5150 4E4D 4B49 4846 8AFF"            /* YXWVTSQPNMKIHFŠÿ */
	$"135D FFFF FEFD FBFA F8F6 F5F3 F1EF EEEC"            /* .]ÿÿþýûúøöõóñïîì */
	$"EAE8 E744 4286 FF17 B76E 6968 6E66 6DB6"            /* êèçDB†ÿ.·nihnfm¶ */
	$"FBFA F8F6 F5F3 F1EE EDEB E9E8 E742 5942"            /* ûúøöõóñîíëéèçBYB */
	$"83FF 1AEC 5432 775E 4741 526F 2E54 FAF8"            /* ƒÿ.ìT2w^GARo.Túø */
	$"F6F5 F339 3837 35D6 E2E7 40B3 5933 81FF"            /* öõó9875Öâç@³Y3ÿ */
	$"1CED 391A 8B3D 2F30 2615 1E7E 1639 F8F6"            /* .í9.‹=/0&..~.9øö */
	$"F538 FFEC D2B4 32C5 E13D DEB3 5933 80FF"            /* õ8ÿìÒ´2Åá=Þ³Y3€ÿ */
	$"5861 007F 4937 413A 2F24 102B 7600 5FF6"            /* Xa..I7A:/$.+v._ö */
	$"38FF ECD2 B498 7C2F C539 EEDE B359 33FF"            /* 8ÿìÒ´˜|/Å9îÞ³Y3ÿ */
	$"C503 2481 293C 3E39 2E23 1904 7623 0237"            /* Å.$)<>9.#..v#.7 */
	$"FFEC D2B4 987C 602D 9B34 3736 3432 312F"            /* ÿìÒ´˜|`-›476421/ */
	$"7000 3B78 2133 3531 281F 1600 6E3A 0062"            /* p.;x!351(...n:.b */
	$"ECD2 B498 7C60 2D7A 9B8F 8275 1A2D 3E0F"            /* ìÒ´˜|`-z›‚u.->. */
	$"357D 1828 2927 211A 1100 7934 1033 D2B4"            /* 5}.()'!...y4.3Ò´ */
	$"987C 602C 7B96 C5DF 829C 7F2B 311D 1A8C"            /* ˜|`,{–Åß‚œ.+1..Œ */
	$"2617 1E1D 1913 031D 8C1B 1D24 B498 7C60"            /* &.......Œ..$´˜|` */
	$"2C60 29B1 DBE4 E3E2 E0DF 9C29 3523 1953"            /* ,`)±Ûäãâàßœ)5#.S */
	$"880A 090E 0B00 078C 501A 2326 987C 602B"            /* ˆÂÆ....ŒP.#&˜|`+ */
	$"607C 9826 C5DF E3E2 E0DF 9C27 4823 281C"            /* `|˜&Åßãâàßœ'H#(. */
	$"6890 4517 1548 9466 1D29 2236 7C60 2A60"            /* hE..H”f.)"6|`*` */
	$"7C98 BCC9 23C3 DDE2 E0DF 9C25 7C20 2E2E"            /* |˜¼É#ÃÝâàßœ%| .. */
	$"2248 7AB4 B57C 4623 2D2F 1769 602A 607C"            /* "Hz´µ|F#-/.i`*`| */
	$"98BB C8D4 DF20 C1DC E0DF 9C7F 23C9 2832"            /* ˜»ÈÔß ÁÜàßœ.#É(2 */
	$"3333 2F13 888D 1C2E 3334 3112 6029 607C"            /* 33/.ˆ..341.`)`| */
	$"98BA C6D3 DEE7 ED1E C0DA DF9C 21FF 7726"            /* ˜ºÆÓÞçí.ÀÚßœ!ÿw& */
	$"3938 3924 888F 2C38 393A 1562 297C 9898"            /* 989$ˆ,89:.b)|˜˜ */
	$"B8C5 D1DD E6ED EFEF 1BBF D99C 1FFF EE5B"            /* ¸ÅÑÝæíïï.¿Ùœ.ÿî[ */
	$"283C 3F28 8993 323F 3B18 438F 9898 ACB7"            /* (<?(‰“2?;.C˜˜¬· */
	$"C3D0 DCE6 ECEF EFED 1996 CB9B 1CFF FFED"            /* ÃÐÜæìïïí.–Ë›.ÿÿí */
	$"722A 2F25 8994 2D26 1960 8D98 A1AB B6C2"            /* r*.%‰”-&.`˜¡«¶Â */
	$"CFDB E5EC EFEF EDE7 DE17 B998 0018 81FF"            /* ÏÛåìïïíçÞ.¹˜..ÿ */
	$"1BC3 6F2F 878C 346B 288C 97A0 AAB4 C1CD"            /* .Ão/‡Œ4k(Œ— ª´ÁÍ */
	$"D9E4 EBEF EFEE E8DF D415 968F 1884 FF0D"            /* ÙäëïïîèßÔ.–.„ÿ. */
	$"3DFF FFFD ED25 9FA8 B3BF CCD8 E3EA 80EE"            /* =ÿÿýí%Ÿ¨³¿ÌØãê€î */
	$"07E9 E0D5 C913 888B 1684 FF0C 3BFF FFFE"            /* .éàÕÉ.ˆ‹.„ÿ.;ÿÿþ */
	$"F6D7 22B2 BECA D7E1 EA80 EE08 EAE1 D7CA"            /* ö×"²¾Ê×áê€î.êá×Ê */
	$"BE12 888B 1484 FF0B 39FF FFFE FDF4 D520"            /* ¾.ˆ‹.„ÿ.9ÿÿþýôÕ  */
	$"C9D5 E0E9 80EE 09EA E3D8 CCBF 1176 968F"            /* ÉÕàé€îÆêãØÌ¿.v– */
	$"1284 FF18 37FF FFFE FDFB F3D3 1DDF E8EE"            /* .„ÿ.7ÿÿþýûóÓ.ßèî */
	$"EFEF EBE4 D9CD C111 7791 BE98 1084 FF18"            /* ïïëäÙÍÁ.w‘¾˜.„ÿ. */
	$"35FF FFFE FDFB FAF1 D11B EDEF EFEC E5DB"            /* 5ÿÿþýûúñÑ.íïïìåÛ */
	$"CFC2 1077 92BF D99C 0E84 FF18 33FF FFFE"            /* ÏÂ.w’¿Ùœ.„ÿ.3ÿÿþ */
	$"FDFB FAF7 1A60 18EF ECE6 DCD0 C310 7893"            /* ýûú÷.`.ïìæÜÐÃ.x“ */
	$"C0DA DF9C 0C84 FF18 31FF FFFE FDFB FA1A"            /* ÀÚßœ.„ÿ.1ÿÿþýûú. */
	$"6076 16ED E6DD D1C5 0F79 94C1 DCE0 DF9C"            /* `v.íæÝÑÅ.y”ÁÜàßœ */
	$"0A84 FF18 2FFF FFFE FDFB 1960 768E A713"            /* Â„ÿ./ÿÿþýû.`vŽ§. */
	$"DED3 C60F 7A95 C3DD E2E0 DF9C 0884 FF18"            /* ÞÓÆ.z•ÃÝâàßœ.„ÿ. */
	$"2DFF FFFE FDFA 1776 8EA7 C112 D4C8 0E7B"            /* -ÿÿþýú.vŽ§Á.ÔÈ.{ */
	$"96C5 DFE3 E2E0 DF9C 0684 FF18 2BFF FFFE"            /* –Åßãâàßœ.„ÿ.+ÿÿþ */
	$"FDFA 168E A7C1 DC10 C90E 7C97 C5E1 E5E3"            /* ýú.Ž§ÁÜ.É.|—Åáåã */
	$"E2E0 DF9C 0584 FF18 29FF FFFE FDFA E413"            /* âàßœ.„ÿ.)ÿÿþýúä. */
	$"C1DC EE0E 0D7D 99C7 E2E7 E5E3 E2E0 DF9C"            /* ÁÜî..}™Çâçåãâàßœ */
	$"0384 FF18 26FF FFFE FDFB F3D3 100F 0E0D"            /* .„ÿ.&ÿÿþýûóÓ.... */
	$"7E9A C9E4 E8E7 E5E3 E2E0 DF9C 0284 FF18"            /* ~šÉäèçåãâàßœ.„ÿ. */
	$"24FF FFFE FDFB FAF1 D1A4 9492 A0CB E6EA"            /* $ÿÿþýûúñÑ¤”’ Ëæê */
	$"E8E7 E5E3 E2E0 DF9C 0184 FF18 2220 1F1D"            /* èçåãâàßœ.„ÿ." .. */
	$"1B1A 1816 1411 0F0E 0D0D 0B0A 0807 0605"            /* ...........Â.... */
	$"0403 0201 0084 FF12 5E5D 5C5B 5A59 5857"            /* .....„ÿ.^]\[ZYXW */
	$"5654 5351 504E 4D4B 4948 468A FF13 5DFF"            /* VTSQPNMKIHFŠÿ.]ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* ÿþýûúøöõóñïîìêèç */
	$"4442 86FF 17B6 6A67 6C71 656C B7FB FAF8"            /* DB†ÿ.¶jglqel·ûúø */
	$"F6F5 F3F1 EEED EBE9 E8E7 4259 4283 FF1A"            /* öõóñîíëéèçBYBƒÿ. */
	$"ED55 2F94 A693 8079 782D 55FA F8F6 F5F3"            /* íU/”¦“€yx-Uúøöõó */
	$"3938 3735 D6E2 E740 B359 3381 FF1C EE3E"            /* 9875Öâç@³Y3ÿ.î> */
	$"19B6 ADA5 987C 5848 861B 3DF8 F6F5 38FF"            /* .¶­¥˜|XH†.=øöõ8ÿ */
	$"ECD2 B432 C5E1 3DDE B359 3380 FF58 6F00"            /* ìÒ´2Åá=Þ³Y3€ÿXo. */
	$"90AB B2BE AC8A 6942 3E7E 0667 F638 FFEC"            /* «²¾¬ŠiB>~.gö8ÿì */
	$"D2B4 987C 2FC5 39EE DEB3 5933 FFCC 2233"            /* Ò´˜|/Å9îÞ³Y3ÿÌ"3 */
	$"A891 B1B8 A787 674A 1C7B 4219 37FF ECD2"            /* ¨‘±¸§‡gJ.{B.7ÿìÒ */
	$"B498 7C60 2D9B 3437 3634 3231 2F88 2A4F"            /* ´˜|`-›476421/ˆ*O */
	$"9F7B 979C 8F76 5D42 1273 5F28 6EEC D2B4"            /* Ÿ{—œv]B.s_(nìÒ´ */
	$"987C 602D 7A9B 8F82 751A 2D67 4453 985F"            /* ˜|`-z›‚u.-gDS˜_ */
	$"7679 7161 4C33 087E 6445 4AD2 B498 7C60"            /* vyqaL3.~dEJÒ´˜|` */
	$"2C7B 96C5 DF82 9C7F 2B66 5A4E 9C52 535B"            /* ,{–Åß‚œ.+fZNœRS[ */
	$"5549 3616 1BA0 5A5A 44B4 987C 602C 6029"            /* UI6.. ZZD´˜|`,`) */
	$"B1DB E4E3 E2E0 DF9C 2971 685F 7C92 2A2E"            /* ±Ûäãâàßœ)qh_|’*. */
	$"3227 0F03 8D8B 6268 4698 7C60 2B60 7C98"            /* 2'..‹bhF˜|`+`|˜ */
	$"26C5 DFE3 E2E0 DF9C 2787 7478 6B97 9745"            /* &Åßãâàßœ'‡txk——E */
	$"2119 419C A170 786F 537C 602A 607C 98BC"            /* !.Aœ¡pxoS|`*`|˜¼ */
	$"C823 C3DD E2E0 DF9C 25AD 7E86 877E 96A6"            /* È#ÃÝâàßœ%­~†‡~–¦ */
	$"B8C8 B499 8086 8B64 7760 2A60 7C98 BAC7"            /* ¸È´™€†‹dw`*`|˜ºÇ */
	$"D3DE 20C1 DCE0 DF9C 7F23 DD8B 9897 9898"            /* ÓÞ ÁÜàßœ.#Ý‹˜—˜˜ */
	$"739B C88B 9497 9997 4860 2960 7C98 B9C5"            /* s›È‹”—™—H`)`|˜¹Å */
	$"D2DD E6EC 1EC0 DADF 9C21 FEAF 99A9 A6A9"            /* ÒÝæì.ÀÚßœ!þ¯™©¦© */
	$"879C CEA1 A6A8 AC66 6F29 7C98 98B8 C4D1"            /* ‡œÎ¡¦¨¬fo)|˜˜¸ÄÑ */
	$"DCE6 ECEE EE1B BFD9 9C1F FFF2 A1A1 B7BA"            /* Üæìîî.¿Ùœ.ÿò¡¡·º */
	$"949F D6B1 B9B4 705A 8E98 98AB B6C3 CFDB"            /* ”ŸÖ±¹´pZŽ˜˜«¶ÃÏÛ */
	$"E5EB EEEE EC19 96CB 9B1C FFFF EFA6 91AC"            /* åëîîì.–Ë›.ÿÿï¦‘¬ */
	$"97A1 DBAF 915B 718D 97A1 AAB5 C1CE DAE4"            /* —¡Û¯‘[q—¡ªµÁÎÚä */
	$"EBEE EEEC E6DD 17B9 9800 1880 FF1C FECF"            /* ëîîìæÝ.¹˜..€ÿ.þÏ */
	$"9865 93AF 6680 288C 96A0 A9B4 C0CC D8E3"            /* ˜e“¯f€(Œ– ©´ÀÌØã */
	$"EAEE EEED E7DE D315 968F 1884 FF0D 3DFF"            /* êîîíçÞÓ.–.„ÿ.=ÿ */
	$"FFFD ED25 9FA8 B2BE CBD7 E2E9 80ED 07E8"            /* ÿýí%Ÿ¨²¾Ë×âé€í.è */
	$"E0D5 C813 888B 1684 FF0C 3BFF FFFE F6D7"            /* àÕÈ.ˆ‹.„ÿ.;ÿÿþö× */
	$"22B1 BDCA D6E1 E980 ED08 E9E1 D6CA BD12"            /* "±½ÊÖáé€í.éáÖÊ½. */
	$"888B 1484 FF0B 39FF FFFE FDF4 D520 C8D5"            /* ˆ‹.„ÿ.9ÿÿþýôÕ ÈÕ */
	$"E0E8 80ED 09E9 E2D7 CBBE 1176 968F 1284"            /* àè€íÆéâ×Ë¾.v–.„ */
	$"FF18 37FF FFFE FDFB F3D3 1DDE E7ED EEEE"            /* ÿ.7ÿÿþýûóÓ.Þçíîî */
	$"EAE3 D8CC C011 7791 BE98 1084 FF18 35FF"            /* êãØÌÀ.w‘¾˜.„ÿ.5ÿ */
	$"FFFE FDFB FAF1 D11B ECEE EEEB E4DA CEC1"            /* ÿþýûúñÑ.ìîîëäÚÎÁ */
	$"1077 92BF D99C 0E84 FF18 33FF FFFE FDFB"            /* .w’¿Ùœ.„ÿ.3ÿÿþýû */
	$"FAF7 1A60 18EE EBE5 DBCF C310 7893 C0DA"            /* ú÷.`.îëåÛÏÃ.x“ÀÚ */
	$"DF9C 0C84 FF18 31FF FFFE FDFB FA1A 6076"            /* ßœ.„ÿ.1ÿÿþýûú.`v */
	$"16EC E6DC D1C4 0F79 94C1 DCE0 DF9C 0A84"            /* .ìæÜÑÄ.y”ÁÜàßœÂ„ */
	$"FF18 2FFF FFFE FDFB 1960 768E A713 DDD2"            /* ÿ./ÿÿþýû.`vŽ§.ÝÒ */
	$"C50F 7A95 C3DD E2E0 DF9C 0884 FF18 2DFF"            /* Å.z•ÃÝâàßœ.„ÿ.-ÿ */
	$"FFFE FDFA 1776 8EA7 C112 D3C7 0E7B 96C5"            /* ÿþýú.vŽ§Á.ÓÇ.{–Å */
	$"DFE3 E2E0 DF9C 0684 FF18 2BFF FFFE FDFA"            /* ßãâàßœ.„ÿ.+ÿÿþýú */
	$"168E A7C1 DB10 C80E 7C97 C5E1 E5E3 E2E0"            /* .Ž§ÁÛ.È.|—Åáåãâà */
	$"DF9C 0584 FF18 29FF FFFE FDFA E413 C1DB"            /* ßœ.„ÿ.)ÿÿþýúä.ÁÛ */
	$"ED0E 0D7D 99C7 E2E7 E5E3 E2E0 DF9C 0384"            /* í..}™Çâçåãâàßœ.„ */
	$"FF18 26FF FFFE FDFB F3D3 100F 0E0D 7E9A"            /* ÿ.&ÿÿþýûóÓ....~š */
	$"C9E4 E8E7 E5E3 E2E0 DF9C 0284 FF18 24FF"            /* Éäèçåãâàßœ.„ÿ.$ÿ */
	$"FFFE FDFB FAF1 D1A4 9492 A0CB E6EA E8E7"            /* ÿþýûúñÑ¤”’ Ëæêèç */
	$"E5E3 E2E0 DF9C 0184 FF18 2220 1F1D 1B1A"            /* åãâàßœ.„ÿ." .... */
	$"1816 1411 0F0E 0D0D 0B0A 0807 0605 0403"            /* .........Â...... */
	$"0201 0084 FF12 5E5D 5C5B 5A59 5857 5654"            /* ...„ÿ.^]\[ZYXWVT */
	$"5351 504E 4D4B 4948 468A FF13 5DFF FFFE"            /* SQPNMKIHFŠÿ.]ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 4442"            /* ýûúøöõóñïîìêèçDB */
	$"86FF 17B7 7069 666D 666E B6FB FAF8 F6F5"            /* †ÿ.·pifmfn¶ûúøöõ */
	$"F3F1 EEED EBE9 E8E7 4259 4283 FF1A EC54"            /* óñîíëéèçBYBƒÿ.ìT */
	$"3469 4423 2447 6B2E 54FA F8F6 F5F3 3938"            /* 4iD#$Gk.Túøöõó98 */
	$"3735 D6E2 E740 B359 3381 FF04 EC37 1C78"            /* 75Öâç@³Y3ÿ.ì7.x */
	$"0F81 0013 147B 1539 F8F6 F538 FFEC D2B4"            /* ....{.9øöõ8ÿìÒ´ */
	$"32C5 E13D DEB3 5933 80FF 035C 0078 1B83"            /* 2Åá=Þ³Y3€ÿ.\.x.ƒ */
	$"0018 2472 005C F638 FFEC D2B4 987C 2FC5"            /* ..$r.\ö8ÿìÒ´˜|/Å */
	$"39EE DEB3 5933 FFC1 001E 6E85 0017 7316"            /* 9îÞ³Y3ÿÁ..n…..s. */
	$"0037 FFEC D2B4 987C 602D 9B34 3736 3432"            /* .7ÿìÒ´˜|`-›47642 */
	$"312F 6700 3463 8500 0D6D 2800 60EC D2B4"            /* 1/g.4c…..m(.`ìÒ´ */
	$"987C 602D 7A9B 8F82 7504 2D2D 0027 7185"            /* ˜|`-z›‚u.--.'q… */
	$"000D 771D 002B D2B4 987C 602C 7B96 C5DF"            /* ..w..+Ò´˜|`,{–Åß */
	$"829C 052B 1600 0787 1583 001A 1D84 0400"            /* ‚œ.+...‡.ƒ...„.. */
	$"15B4 987C 602C 6029 B1DB E4E3 E2E0 DF9C"            /* .´˜|`,`)±Ûäãâàßœ */
	$"2916 0000 4386 0581 0016 0A8A 3900 0015"            /* )...C†...ÂŠ9... */
	$"987C 602B 607C 9826 C5DF E3E2 E0DF 9C27"            /* ˜|`+`|˜&Åßãâàßœ' */
	$"2D80 0007 578D 4714 154B 8F4E 8000 112C"            /* -€..WG..KN€.., */
	$"7C60 2A60 7C98 BFCC 23C3 DDE2 E0DF 9C48"            /* |`*`|˜¿Ì#ÃÝâàßœH */
	$"6781 0005 2768 B1AC 6622 8100 1166 602A"            /* g..'h±¬f"..f`* */
	$"607C 98BD CAD7 E320 C1DC E0DF 9C23 C183"            /* `|˜½Ê×ã ÁÜàßœ#Áƒ */
	$"0001 7E72 8300 1260 2960 7C98 BCC9 D6E2"            /* ..~rƒ..`)`|˜¼ÉÖâ */
	$"EBF1 1EC0 DADF 9C21 FF5D 8200 017E 7082"            /* ëñ.ÀÚßœ!ÿ]‚..~p‚ */
	$"0014 5D29 7C98 98BB C8D4 E0EA F1F3 F31B"            /* ..])|˜˜»ÈÔàêñóó. */
	$"BFD9 9C1F FFEC 3D81 0001 7E71 8100 163D"            /* ¿Ùœ.ÿì=..~q..= */
	$"9098 98AE B9C6 D3DF E9F0 F3F3 F119 96CB"            /* ˜˜®¹ÆÓßéðóóñ.–Ë */
	$"9B1C FFFF EB5F 8000 017E 7180 0013 5F8F"            /* ›.ÿÿë_€..~q€.._ */
	$"99A3 ADB8 C5D2 DEE8 F0F3 F3F1 EBE2 17B9"            /* ™£­¸ÅÒÞèðóóñëâ.¹ */
	$"9818 81FF 1BBF 6021 807B 2766 288E 98A2"            /* ˜.ÿ.¿`!€{'f(Ž˜¢ */
	$"ACB7 C3D0 DDE7 EFF3 F3F2 ECE3 D715 968F"            /* ¬·ÃÐÝçïóóòìã×.– */
	$"1884 FF0D 3DFF FFFD ED25 A1AB B5C2 CFDB"            /* .„ÿ.=ÿÿýí%¡«µÂÏÛ */
	$"E6EE 80F2 07ED E4D9 CC13 888B 1684 FF0C"            /* æî€ò.íäÙÌ.ˆ‹.„ÿ. */
	$"3BFF FFFE F6D7 22B4 C0CD DAE5 ED80 F208"            /* ;ÿÿþö×"´ÀÍÚåí€ò. */
	$"EDE5 DACD C012 888B 1484 FF0B 39FF FFFE"            /* íåÚÍÀ.ˆ‹.„ÿ.9ÿÿþ */
	$"FDF4 D520 CCD9 E4ED 80F2 09EE E6DB CFC2"            /* ýôÕ ÌÙäí€òÆîæÛÏÂ */
	$"1176 968F 1284 FF18 37FF FFFE FDFB F3D3"            /* .v–.„ÿ.7ÿÿþýûóÓ */
	$"1DE3 ECF2 F3F3 EFE7 DDD0 C311 7791 BE98"            /* .ãìòóóïçÝÐÃ.w‘¾˜ */
	$"1084 FF18 35FF FFFE FDFB FAF1 D11B F1F3"            /* .„ÿ.5ÿÿþýûúñÑ.ñó */
	$"F3F0 E8DE D2C5 1077 92BF D99C 0E84 FF18"            /* óðèÞÒÅ.w’¿Ùœ.„ÿ. */
	$"33FF FFFE FDFB FAF7 1A60 18F3 F0E9 DFD3"            /* 3ÿÿþýûú÷.`.óðéßÓ */
	$"C610 7893 C0DA DF9C 0C84 FF18 31FF FFFE"            /* Æ.x“ÀÚßœ.„ÿ.1ÿÿþ */
	$"FDFB FA1A 6076 16F1 EAE0 D4C8 0F79 94C1"            /* ýûú.`v.ñêàÔÈ.y”Á */
	$"DCE0 DF9C 0A84 FF18 2FFF FFFE FDFB 1960"            /* ÜàßœÂ„ÿ./ÿÿþýû.` */
	$"7690 A913 E2D6 C90F 7A95 C3DD E2E0 DF9C"            /* v©.âÖÉ.z•ÃÝâàßœ */
	$"0884 FF18 2DFF FFFE FDFA 1776 90A9 C412"            /* .„ÿ.-ÿÿþýú.v©Ä. */
	$"D7CA 0E7B 96C5 DFE3 E2E0 DF9C 0684 FF18"            /* ×Ê.{–Åßãâàßœ.„ÿ. */
	$"2BFF FFFE FDFA 1690 A9C4 DF10 CC0E 7C97"            /* +ÿÿþýú.©Äß.Ì.|— */
	$"C5E1 E5E3 E2E0 DF9C 0584 FF18 29FF FFFE"            /* Åáåãâàßœ.„ÿ.)ÿÿþ */
	$"FDFA E413 C4DF F20E 0D7D 99C7 E2E7 E5E3"            /* ýúä.Äßò..}™Çâçåã */
	$"E2E0 DF9C 0384 FF18 26FF FFFE FDFB F3D3"            /* âàßœ.„ÿ.&ÿÿþýûóÓ */
	$"100F 0E0D 7E9A C9E4 E8E7 E5E3 E2E0 DF9C"            /* ....~šÉäèçåãâàßœ */
	$"0284 FF18 24FF FFFE FDFB FAF1 D1A4 9492"            /* .„ÿ.$ÿÿþýûúñÑ¤”’ */
	$"A0CB E6EA E8E7 E5E3 E2E0 DF9C 0184 FF18"            /*  Ëæêèçåãâàßœ.„ÿ. */
	$"2220 1F1D 1B1A 1816 1411 0F0E 0D0D 0B0A"            /* " .............Â */
	$"0807 0605 0403 0201 006C 386D 6B00 0004"            /* .........l8mk... */
	$"0800 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF00 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿ..... */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 0000 0000 FFFF FFFF FFFF FFFF FFFF"            /* ......ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ....ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ.ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ.ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ..ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ...ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 FFFF FFFF FFFF FFFF FFFF"            /* ÿ.....ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF"                                                 /* ÿ */
};

data 'icns' (139, "Text File Icon") {
	$"6963 6E73 0000 1611 4943 4E23 0000 0108"            /* icns....ICN#.... */
	$"016D BBC0 0100 0020 0550 0070 1AB8 0048"            /* .m»À... .P.p.¸.H */
	$"35CC 0254 6B76 0046 6ABE 207B 76FB 0015"            /* 5Ì.Tkv.Fj¾ {vû.. */
	$"DB76 044B 6DF7 0001 B7EB DDB2 6BCD 0001"            /* Ûv.Km÷..·ëÝ²kÍ.. */
	$"B436 000B 6B5A EF71 34AC 0002 0B54 0001"            /* ´6..kZïq4¬...T.. */
	$"157B FBB3 0A40 0001 0100 0049 0100 0003"            /* .{û³Â@.....I.... */
	$"0100 0401 0100 0023 0100 1001 0100 0113"            /* .......#........ */
	$"0100 0001 0100 4043 0100 0001 0100 0213"            /* ......@C........ */
	$"0100 0001 0100 1083 0100 0011 01FF FFFF"            /* .......ƒ.....ÿÿÿ */
	$"01FF FFC0 01FF FFE0 07FF FFF0 1FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð.ÿÿø */
	$"3FFF FFFC 7FFF FFFE 7FFF FFFF FFFF FFFF"            /* ?ÿÿü.ÿÿþ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 7FFF FFFF 7FFF FFFF 3FFF FFFF"            /* ÿÿÿÿ.ÿÿÿ.ÿÿÿ?ÿÿÿ */
	$"1FFF FFFF 07FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"6963 6C34 0000 0208 0000 000E DEDE EDEE"            /* icl4........ÞÞíî */
	$"DEEE EEEE EE00 0000 0000 000D 0000 0000"            /* Þîîîî........... */
	$"C000 0000 0EE0 0000 0000 CDED EDEC 0000"            /* À....à....Íííì.. */
	$"000C 00C0 CEDE 0000 000E AD8B 8EDE E000"            /* ...ÀÎÞ....­‹ŽÞà. */
	$"00C0 0C0C 0ECE A000 00EF D888 999B FE0C"            /* .À...Î ..ïØˆ™›þ. */
	$"0000 C000 CECC EE00 0EFC 888A 89AE DFE0"            /* ..À.ÎÌî..üˆŠ‰®ßà */
	$"000C 000C 0E00 CEE0 CF9D 8818 8A9F D9FC"            /* ......ÎàÏˆ.ŠŸÙü */
	$"0000 C0C0 CEEF EEEA D9ED A898 A99F DE9B"            /* ..ÀÀÎïîêÙí¨˜©ŸÞ› */
	$"00C0 0C00 0DDD DDDF 9A9B 898A 8AFF D9A9"            /* .À...ÝÝßš›‰ŠŠÿÙ© */
	$"000C 00C0 C0DD CDDE A98D A9A8 F9FE D99F"            /* ...ÀÀÝÍÞ©©¨ùþÙŸ */
	$"00C0 000C 0C00 C0DE 8A9E D9FF 9FFD 8A8E"            /* .À....ÀÞŠžÙÿŸýŠŽ */
	$"EE0E AF0E E0FE 0CCF E898 DDE9 FECB 98A9"            /* î.¯.àþ.Ïè˜ÝéþË˜© */
	$"000C 000C 0C00 C0DE D8A8 8BDC CD88 A89B"            /* ......ÀÞØ¨‹ÜÍˆ¨› */
	$"0C00 C000 C0CC 0CCF 0989 888D B888 889C"            /* ..À.ÀÌ.ÏÆ‰ˆ¸ˆˆœ */
	$"EEE0 EFEF 0FEE C0DE 0B88 A88D C8A8 8AE0"            /* îàïï.îÀÞ.ˆ¨È¨Šà */
	$"000C 0000 C0C0 0CCF 00B8 888D 1888 9E00"            /* ....ÀÀ.Ï.¸ˆ.ˆž. */
	$"00C0 00C0 0C00 C0DF 000B 888D D88A D0EA"            /* .À.À..Àß..ˆØŠÐê */
	$"FEEF E0FA F0FF 0CC6 0000 CB9D D8BC 0000"            /* þïàúðÿ.Æ..ËØ¼.. */
	$"0000 0C00 0C00 C0DA 0000 000E 0000 00C0"            /* ......ÀÚ.......À */
	$"00C0 00C0 C0C0 0CCF 0000 000E 0000 0000"            /* .À.ÀÀÀ.Ï........ */
	$"0000 C00C 000C C0DF 0000 000E 0000 000C"            /* ..À...Àß........ */
	$"000C 0C00 C0C0 0CCF 0000 000E 0000 0C00"            /* ....ÀÀ.Ï........ */
	$"00C0 000C 0C00 C0DF 0000 000A 0000 0000"            /* .À....Àß...Â.... */
	$"0C00 C0C0 0C0C 0CCF 0000 000E 0000 000C"            /* ..ÀÀ...Ï........ */
	$"0000 0C00 C0C0 C0DF 0000 000F 0000 0000"            /* ....ÀÀÀß........ */
	$"00C0 00C0 0C0C 0CCF 0000 000E 0000 0000"            /* .À.À...Ï........ */
	$"C00C 0C00 C0C0 C0DF 0000 000F 0000 0000"            /* À...ÀÀÀß........ */
	$"0000 C00C 000C 0CCF 0000 000E 0000 000C"            /* ..À....Ï........ */
	$"00C0 00C0 C0C0 C0DF 0000 000A 0000 0000"            /* .À.ÀÀÀÀß...Â.... */
	$"0C00 C000 0C0C 0CCF 0000 000F 0000 0C00"            /* ..À....Ï........ */
	$"000C 00C0 C0C0 C0DF 0000 000E 0000 0000"            /* ...ÀÀÀÀß........ */
	$"C000 0C0C 0C0C 0CCF 0000 000F FAFF FFFF"            /* À......Ï....úÿÿÿ */
	$"AFFF FAFF FFFF FFFF 6963 6C38 0000 0408"            /* ¯ÿúÿÿÿÿÿicl8.... */
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F5F5 00F5 F5F5 F5F5 F5AC FBFC 0000 0000"            /* õõ.õõõõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A082 FAAC FC00 00F5"            /* ..õû¬{|  ‚ú¬ü..õ */
	$"00F5 F5F5 F5F5 F5F6 F5FC F7FB AC00 0000"            /* .õõõõõõöõü÷û¬... */
	$"00F5 ACFE 50A1 A1A1 A7CB AD7A DFAC 0000"            /* .õ¬þP¡¡¡§Ë­zß¬.. */
	$"F500 F5F5 F5F5 F6F5 F6AC F5F7 FBAC 0000"            /* õ.õõõõöõö¬õ÷û¬.. */
	$"0081 FF7A 7D9B 9BA1 A1A7 E7AD FAEA FBF5"            /* .ÿz}››¡¡§ç­úêûõ */
	$"00F5 F5F5 F5F5 F5F5 F5AC F5F6 F7FB AC00"            /* .õõõõõõõõ¬õö÷û¬. */
	$"2BE9 AD57 C57D 9BA1 A1A7 E8E9 F9E8 E92B"            /* +é­WÅ}›¡¡§èéùèé+ */
	$"F5F5 F5F5 F5F5 F5F6 F5AC ACAC ACAC ACFD"            /* õõõõõõõöõ¬¬¬¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADEA FAA6 D1FB"            /* {éüu§¡¡¡§Ë­êú¦Ñû */
	$"00F5 00F5 F5F5 F6F5 F556 FAFA FAFA FAAC"            /* .õ.õõõöõõVúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBAD E8FF FAA7 E8A6"            /* ¦è¦ùË§¡§Ë­èÿú§è¦ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 5656 5656 56FD"            /* .õõõõõõöõõVVVVVý */
	$"A7A7 E8F9 A7E7 A7CB ADE8 EAFD 56E7 A7AD"            /* §§èù§ç§Ë­èêýVç§­ */
	$"F5F5 F5F5 F5F5 F5F5 F5F6 F6F5 F6F6 F8FD"            /* õõõõõõõõõööõööøý */
	$"A7A7 A7A6 56E9 E9D1 D1E0 EA56 A0A7 A7D0"            /* §§§¦VééÑÑàêV §§Ð */
	$"88FC 00AC ACAC F5FD ACF5 FDFD F5F6 56FD"            /* ˆü.¬¬¬õý¬õýýõöVý */
	$"A0A7 E6A7 7B56 FCFD FEFC F87C A7A7 CBA6"            /*  §æ§{Vüýþüø|§§Ë¦ */
	$"0000 F5F5 F5F5 F5F5 F5F6 F5F5 F6F6 F8FD"            /* ..õõõõõõõöõõööøý */
	$"57C5 A1A1 A7A0 572C 4F51 A1E6 A1A1 A781"            /* WÅ¡¡§ W,OQ¡æ¡¡§ */
	$"F5F5 F5F5 F5F5 F5F6 F5F6 F6F6 F5F6 56FD"            /* õõõõõõõöõöööõöVý */
	$"25A7 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E82B"            /* %§¡¡¡¡æzQæ¡¡¡¡è+ */
	$"ACAC FD00 ACAC FDFD F5AC FDFD F6F6 56FD"            /* ¬¬ý.¬¬ýýõ¬ýýööVý */
	$"0075 A1A1 A19B A757 51A1 A1A1 9BE7 8100"            /* .u¡¡¡›§WQ¡¡¡›ç. */
	$"0000 00F5 F5F5 F5F5 F5F6 F5F5 F6F6 56FD"            /* ...õõõõõõöõõööVý */
	$"00F5 7CA1 9B7D C557 4AA1 779B E782 0000"            /* .õ|¡›}ÅWJ¡w›ç‚.. */
	$"F5F5 F5F5 F5F5 F5F5 F5F6 F5F6 F6F6 F8FE"            /* õõõõõõõõõöõöööøþ */
	$"0000 F57B A19B A17A 519B E6A7 8100 FDAC"            /* ..õ{¡›¡zQ›æ§.ý¬ */
	$"ACAC FDFD FDF5 FDFD FDF5 FEFD F5F6 56D2"            /* ¬¬ýýýõýýýõþýõöVÒ */
	$"0000 0000 2B7B A6F9 57A6 FAF7 0000 0000"            /* ....+{¦ùW¦ú÷.... */
	$"00F5 00F5 00F5 F5F5 F5F6 F5F5 F6F6 56FD"            /* .õ.õ.õõõõöõõööVý */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"F500 F5F5 F5F5 F5F6 F5F6 F6F6 F6F5 56E0"            /* õ.õõõõõöõööööõVà */
	$"0000 0000 0000 00AC 0000 0000 0000 00F5"            /* .......¬.......õ */
	$"00F5 F5F5 F5F5 F5F5 F6F5 F5F6 F5F6 56FE"            /* .õõõõõõõöõõöõöVþ */
	$"0000 0000 0000 00AC 0000 0000 F500 F500"            /* .......¬....õ.õ. */
	$"F500 F5F5 F5F5 F6F5 F5F6 F6F6 F6F6 56FE"            /* õ.õõõõöõõöööööVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 00F5"            /* .......¬.......õ */
	$"00F5 F5F5 F5F5 F5F5 F6F5 F6F5 F6F6 F8FE"            /* .õõõõõõõöõöõööøþ */
	$"0000 0000 0000 00AC 0000 0000 F500 F500"            /* .......¬....õ.õ. */
	$"F5F5 00F5 F5F5 F6F5 F5F6 F5F6 F6F6 56EA"            /* õõ.õõõöõõöõöööVê */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F6F5 F6F6 F8E0"            /* .õõõõõõöõööõööøà */
	$"0000 0000 0000 00AC 0000 0000 0000 F500"            /* .......¬......õ. */
	$"F500 F5F5 F5F5 F5F5 F6F5 F6F6 F6F5 56F4"            /* õ.õõõõõõöõöööõVô */
	$"0000 0000 0000 00FD 0000 0000 F500 00F5"            /* .......ý....õ..õ */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F5F6 F5F6 56FF"            /* .õõõõõõöõöõöõöVÿ */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"F500 F5F5 F5F5 F6F5 F6F5 F6F6 F6F6 56FF"            /* õ.õõõõöõöõööööVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F500"            /* .......ý......õ. */
	$"00F5 F5F5 F5F5 F5F5 F5F6 F6F5 F6F6 F8FF"            /* .õõõõõõõõööõööøÿ */
	$"0000 0000 0000 00FD 0000 0000 F500 00F5"            /* .......ý....õ..õ */
	$"F500 F5F5 F5F5 F6F5 F5F6 F5F6 F6F6 56FF"            /* õ.õõõõöõõöõöööVÿ */
	$"0000 0000 0000 00FD 0000 0000 00F5 00F5"            /* .......ý.....õ.õ */
	$"00F5 F5F5 F5F5 F5F5 F6F5 F6F6 F6F5 56FF"            /* .õõõõõõõöõöööõVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F500"            /* .......ý......õ. */
	$"F5F5 F5F5 F5F5 F5F6 F5F6 F5F6 F6F6 56FF"            /* õõõõõõõöõöõöööVÿ */
	$"0000 0000 0000 00FD FDFD FEFE FEFE FEFE"            /* .......ýýýþþþþþþ */
	$"EAFE FEFE FFFF FFFF FFFF FFFF FFFF FFFF"            /* êþþþÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"696C 3332 0000 0AE9 84FF 125E 5D5C 5B5A"            /* il32..Âé„ÿ.^]\[Z */
	$"5958 5756 5453 5150 4E4D 4B49 4846 8AFF"            /* YXWVTSQPNMKIHFŠÿ */
	$"135D FFFF FEFD FBFA F8F6 F5F3 F1EF EEEC"            /* .]ÿÿþýûúøöõóñïîì */
	$"EAE8 E744 4286 FF17 B76E 6968 6E66 6DB6"            /* êèçDB†ÿ.·nihnfm¶ */
	$"FBFA F8F6 F5F3 F1EF EEEC EAE8 E742 5942"            /* ûúøöõóñïîìêèçBYB */
	$"83FF 1AEC 5432 775E 4741 526F 2E54 FAF8"            /* ƒÿ.ìT2w^GARo.Túø */
	$"F6F5 F3F1 EFEE ECEA E8E7 40B3 5933 81FF"            /* öõóñïîìêèç@³Y3ÿ */
	$"1CED 391A 8B3D 2F30 2615 1E7E 1639 F8F6"            /* .í9.‹=/0&..~.9øö */
	$"F5F3 F1EF EEEC EAE8 E73D DEB3 5933 80FF"            /* õóñïîìêèç=Þ³Y3€ÿ */
	$"5861 007F 4937 413A 2F24 102B 7600 5FF6"            /* Xa..I7A:/$.+v._ö */
	$"F5F3 F1EF EEEC EAE8 E73B EEDE B359 33FF"            /* õóñïîìêèç;îÞ³Y3ÿ */
	$"C503 2481 293C 3E39 2E23 1904 7623 02C1"            /* Å.$)<>9.#..v#.Á */
	$"F5F3 F1EF EEEC EAE8 E739 3736 3432 312F"            /* õóñïîìêèç976421/ */
	$"7000 3B78 2133 3531 281F 1600 6E3A 0068"            /* p.;x!351(...n:.h */
	$"F5F3 F1EF EEEC EAE8 E79C 8275 1A2D 3E0F"            /* õóñïîìêèçœ‚u.->. */
	$"357D 1828 2927 211A 1100 7934 1034 F5F3"            /* 5}.()'!...y4.4õó */
	$"F1EF EEEC EAE8 E7E5 829C 7F2B 311D 1A8C"            /* ñïîìêèçå‚œ.+1..Œ */
	$"2617 1E1D 1913 031D 8C1B 1D24 F5F3 F1EF"            /* &.......Œ..$õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C29 3523 1953"            /* îìêèçåãâàßœ)5#.S */
	$"880A 090E 0B00 078C 501A 2326 4240 F13C"            /* ˆÂÆ....ŒP.#&B@ñ< */
	$"3B39 EA36 34E5 312F E0DF 9C27 4823 281C"            /* ;9ê64å1/àßœ'H#(. */
	$"6890 4517 1548 9466 1D29 2236 F5F3 F1EF"            /* hE..H”f.)"6õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C25 7C20 2E2E"            /* îìêèçåãâàßœ%| .. */
	$"2248 7AB4 B57C 4623 2D2F 1769 F5F3 F1EF"            /* "Hz´µ|F#-/.iõóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C7F 23C9 2832"            /* îìêèçåãâàßœ.#É(2 */
	$"3333 2F13 888D 1C2E 3334 3112 C03B 3937"            /* 33/.ˆ..341.À;97 */
	$"EF34 3231 2FE7 2C2A 29E0 DF9C 21FF 7726"            /* ï421/ç,*)àßœ!ÿw& */
	$"3938 3924 888F 2C38 393A 1562 F6F5 F3F1"            /* 989$ˆ,89:.böõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 1FFF EE5B"            /* ïîìêèçåãâàßœ.ÿî[ */
	$"283C 3F28 8993 323F 3B18 43F8 F6F5 F3F1"            /* (<?(‰“2?;.Cøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 1CFF FFED"            /* ïîìêèçåãâàßœ.ÿÿí */
	$"722A 2F25 8994 2D26 1960 FA37 3534 3231"            /* r*.%‰”-&.`ú75421 */
	$"2F2D EC2A 2927 E524 22E0 DF9C 0018 81FF"            /* /-ì*)'å$"àßœ..ÿ */
	$"1BC3 6F2F 878C 346B BDFB FAF8 F6F5 F3F1"            /* .Ão/‡Œ4k½ûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 1884 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"3DFF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* =ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 1684 FF18 3BFF FFFE"            /* èçåãâàßœ.„ÿ.;ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 1484 FF18 39FF FFFE FDFB FAF8"            /* âàßœ.„ÿ.9ÿÿþýûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"1284 FF18 37FF FFFE FDFB FAF8 F6F5 F3F1"            /* .„ÿ.7ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 1084 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"35FF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* 5ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 0E84 FF18 33FF FFFE"            /* èçåãâàßœ.„ÿ.3ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 0C84 FF18 31FF FFFE FDFB FAF8"            /* âàßœ.„ÿ.1ÿÿþýûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"0A84 FF18 2FFF FFFE FDFB FAF8 F6F5 F3F1"            /* Â„ÿ./ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 0884 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"2DFF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* -ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 0684 FF18 2BFF FFFE"            /* èçåãâàßœ.„ÿ.+ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 0584 FF18 29FF FFFE FDFB FAF8"            /* âàßœ.„ÿ.)ÿÿþýûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"0384 FF18 26FF FFFE FDFB FAF8 F6F5 F3F1"            /* .„ÿ.&ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 0284 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"24FF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* $ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 0184 FF18 2220 1F1D"            /* èçåãâàßœ.„ÿ." .. */
	$"1B1A 1816 1513 1110 0E0D 0B0A 0807 0605"            /* ...........Â.... */
	$"0403 0201 0084 FF12 5E5D 5C5B 5A59 5857"            /* .....„ÿ.^]\[ZYXW */
	$"5654 5351 504E 4D4B 4948 468A FF13 5DFF"            /* VTSQPNMKIHFŠÿ.]ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* ÿþýûúøöõóñïîìêèç */
	$"4442 86FF 17B6 6A67 6C71 656C B7FB FAF8"            /* DB†ÿ.¶jglqel·ûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 4259 4283 FF1A"            /* öõóñïîìêèçBYBƒÿ. */
	$"ED55 2F94 A693 8079 782D 55FA F8F6 F5F3"            /* íU/”¦“€yx-Uúøöõó */
	$"F1EF EEEC EAE8 E740 B359 3381 FF1C EE3E"            /* ñïîìêèç@³Y3ÿ.î> */
	$"19B6 ADA5 987C 5848 861B 3DF8 F6F5 F3F1"            /* .¶­¥˜|XH†.=øöõóñ */
	$"EFEE ECEA E8E7 3DDE B359 3380 FF58 6F00"            /* ïîìêèç=Þ³Y3€ÿXo. */
	$"90AB B2BE AC8A 6942 3E7E 0667 F6F5 F3F1"            /* «²¾¬ŠiB>~.göõóñ */
	$"EFEE ECEA E8E7 3BEE DEB3 5933 FFCC 2233"            /* ïîìêèç;îÞ³Y3ÿÌ"3 */
	$"A891 B1B8 A787 674A 1C7B 4219 C4F5 F3F1"            /* ¨‘±¸§‡gJ.{B.Äõóñ */
	$"EFEE ECEA E8E7 3937 3634 3231 2F88 2A4F"            /* ïîìêèç976421/ˆ*O */
	$"9F7B 979C 8F76 5D42 1273 5F28 75F5 F3F1"            /* Ÿ{—œv]B.s_(uõóñ */
	$"EFEE ECEA E8E7 9C82 751A 2D67 4453 985F"            /* ïîìêèçœ‚u.-gDS˜_ */
	$"7679 7161 4C33 087E 6445 4CF5 F3F1 EFEE"            /* vyqaL3.~dELõóñïî */
	$"ECEA E8E7 E582 9C7F 2B66 5A4E 9C52 535B"            /* ìêèçå‚œ.+fZNœRS[ */
	$"5549 3616 1BA0 5A5A 44F5 F3F1 EFEE ECEA"            /* UI6.. ZZDõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 2971 685F 7C92 2A2E"            /* èçåãâàßœ)qh_|’*. */
	$"3227 0F03 8D8B 6268 4642 40F1 3C3B 39EA"            /* 2'..‹bhFB@ñ<;9ê */
	$"3634 E531 2FE0 DF9C 2787 7478 6B97 9745"            /* 64å1/àßœ'‡txk——E */
	$"2119 419C A170 786F 53F5 F3F1 EFEE ECEA"            /* !.Aœ¡pxoSõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 25AD 7E86 877E 96A6"            /* èçåãâàßœ%­~†‡~–¦ */
	$"B8C8 B499 8086 8B64 77F5 F3F1 EFEE ECEA"            /* ¸È´™€†‹dwõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 7F23 DD8B 9897 9898"            /* èçåãâàßœ.#Ý‹˜—˜˜ */
	$"739B C88B 9497 9997 48C0 3B39 37EF 3432"            /* s›È‹”—™—HÀ;97ï42 */
	$"312F E72C 2A29 E0DF 9C21 FEAF 99A9 A6A9"            /* 1/ç,*)àßœ!þ¯™©¦© */
	$"879C CEA1 A6A8 AC66 6FF6 F5F3 F1EF EEEC"            /* ‡œÎ¡¦¨¬foöõóñïîì */
	$"EAE8 E7E5 E3E2 E0DF 9C1F FFF2 A1A1 B7BA"            /* êèçåãâàßœ.ÿò¡¡·º */
	$"949F D6B1 B9B4 705A F8F6 F5F3 F1EF EEEC"            /* ”ŸÖ±¹´pZøöõóñïîì */
	$"EAE8 E7E5 E3E2 E0DF 9C1C FFFF EFA6 91AC"            /* êèçåãâàßœ.ÿÿï¦‘¬ */
	$"97A1 DBAF 915B 71FA 3735 3432 312F 2DEC"            /* —¡Û¯‘[qú75421/-ì */
	$"2A29 27E5 2422 E0DF 9C00 1880 FF1C FECF"            /* *)'å$"àßœ..€ÿ.þÏ */
	$"9865 93AF 6680 BEFB FAF8 F6F5 F3F1 EFEE"            /* ˜e“¯f€¾ûúøöõóñïî */
	$"ECEA E8E7 E5E3 E2E0 DF9C 1884 FF18 3DFF"            /* ìêèçåãâàßœ.„ÿ.=ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* ÿþýûúøöõóñïîìêèç */
	$"E5E3 E2E0 DF9C 1684 FF18 3BFF FFFE FDFB"            /* åãâàßœ.„ÿ.;ÿÿþýû */
	$"FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0"            /* úøöõóñïîìêèçåãâà */
	$"DF9C 1484 FF18 39FF FFFE FDFB FAF8 F6F5"            /* ßœ.„ÿ.9ÿÿþýûúøöõ */
	$"F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C 1284"            /* óñïîìêèçåãâàßœ.„ */
	$"FF18 37FF FFFE FDFB FAF8 F6F5 F3F1 EFEE"            /* ÿ.7ÿÿþýûúøöõóñïî */
	$"ECEA E8E7 E5E3 E2E0 DF9C 1084 FF18 35FF"            /* ìêèçåãâàßœ.„ÿ.5ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* ÿþýûúøöõóñïîìêèç */
	$"E5E3 E2E0 DF9C 0E84 FF18 33FF FFFE FDFB"            /* åãâàßœ.„ÿ.3ÿÿþýû */
	$"FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0"            /* úøöõóñïîìêèçåãâà */
	$"DF9C 0C84 FF18 31FF FFFE FDFB FAF8 F6F5"            /* ßœ.„ÿ.1ÿÿþýûúøöõ */
	$"F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C 0A84"            /* óñïîìêèçåãâàßœÂ„ */
	$"FF18 2FFF FFFE FDFB FAF8 F6F5 F3F1 EFEE"            /* ÿ./ÿÿþýûúøöõóñïî */
	$"ECEA E8E7 E5E3 E2E0 DF9C 0884 FF18 2DFF"            /* ìêèçåãâàßœ.„ÿ.-ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* ÿþýûúøöõóñïîìêèç */
	$"E5E3 E2E0 DF9C 0684 FF18 2BFF FFFE FDFB"            /* åãâàßœ.„ÿ.+ÿÿþýû */
	$"FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0"            /* úøöõóñïîìêèçåãâà */
	$"DF9C 0584 FF18 29FF FFFE FDFB FAF8 F6F5"            /* ßœ.„ÿ.)ÿÿþýûúøöõ */
	$"F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C 0384"            /* óñïîìêèçåãâàßœ.„ */
	$"FF18 26FF FFFE FDFB FAF8 F6F5 F3F1 EFEE"            /* ÿ.&ÿÿþýûúøöõóñïî */
	$"ECEA E8E7 E5E3 E2E0 DF9C 0284 FF18 24FF"            /* ìêèçåãâàßœ.„ÿ.$ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* ÿþýûúøöõóñïîìêèç */
	$"E5E3 E2E0 DF9C 0184 FF18 2220 1F1D 1B1A"            /* åãâàßœ.„ÿ." .... */
	$"1816 1513 1110 0E0D 0B0A 0807 0605 0403"            /* .........Â...... */
	$"0201 0084 FF12 5E5D 5C5B 5A59 5857 5654"            /* ...„ÿ.^]\[ZYXWVT */
	$"5351 504E 4D4B 4948 468A FF13 5DFF FFFE"            /* SQPNMKIHFŠÿ.]ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 4442"            /* ýûúøöõóñïîìêèçDB */
	$"86FF 17B7 7069 666D 666E B6FB FAF8 F6F5"            /* †ÿ.·pifmfn¶ûúøöõ */
	$"F3F1 EFEE ECEA E8E7 4259 4283 FF1A EC54"            /* óñïîìêèçBYBƒÿ.ìT */
	$"3469 4423 2447 6B2E 54FA F8F6 F5F3 F1EF"            /* 4iD#$Gk.Túøöõóñï */
	$"EEEC EAE8 E740 B359 3381 FF04 EC37 1C78"            /* îìêèç@³Y3ÿ.ì7.x */
	$"0F81 0013 147B 1539 F8F6 F5F3 F1EF EEEC"            /* ....{.9øöõóñïîì */
	$"EAE8 E73D DEB3 5933 80FF 035C 0078 1B83"            /* êèç=Þ³Y3€ÿ.\.x.ƒ */
	$"0018 2472 005C F6F5 F3F1 EFEE ECEA E8E7"            /* ..$r.\öõóñïîìêèç */
	$"3BEE DEB3 5933 FFC1 001E 6E85 0017 7316"            /* ;îÞ³Y3ÿÁ..n…..s. */
	$"00C0 F5F3 F1EF EEEC EAE8 E739 3736 3432"            /* .Àõóñïîìêèç97642 */
	$"312F 6700 3463 8500 0D6D 2800 66F5 F3F1"            /* 1/g.4c…..m(.fõóñ */
	$"EFEE ECEA E8E7 9C82 7504 2D2D 0027 7185"            /* ïîìêèçœ‚u.--.'q… */
	$"000D 771D 002C F5F3 F1EF EEEC EAE8 E7E5"            /* ..w..,õóñïîìêèçå */
	$"829C 052B 1600 0787 1583 001A 1D84 0400"            /* ‚œ.+...‡.ƒ...„.. */
	$"15F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* .õóñïîìêèçåãâàßœ */
	$"2916 0000 4386 0581 0016 0A8A 3900 0015"            /* )...C†...ÂŠ9... */
	$"4240 F13C 3B39 EA36 34E5 312F E0DF 9C27"            /* B@ñ<;9ê64å1/àßœ' */
	$"2D80 0007 578D 4714 154B 8F4E 8000 112C"            /* -€..WG..KN€.., */
	$"F5F3 F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C48"            /* õóñïîìêèçåãâàßœH */
	$"6781 0005 2768 B1AC 6622 8100 1166 F5F3"            /* g..'h±¬f"..fõó */
	$"F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C23 C183"            /* ñïîìêèçåãâàßœ#Áƒ */
	$"0001 7E72 8300 12C0 3B39 37EF 3432 312F"            /* ..~rƒ..À;97ï421/ */
	$"E72C 2A29 E0DF 9C21 FF5D 8200 017E 7082"            /* ç,*)àßœ!ÿ]‚..~p‚ */
	$"0014 5DF6 F5F3 F1EF EEEC EAE8 E7E5 E3E2"            /* ..]öõóñïîìêèçåãâ */
	$"E0DF 9C1F FFEC 3D81 0001 7E71 8100 163D"            /* àßœ.ÿì=..~q..= */
	$"F8F6 F5F3 F1EF EEEC EAE8 E7E5 E3E2 E0DF"            /* øöõóñïîìêèçåãâàß */
	$"9C1C FFFF EB5F 8000 017E 7180 0013 5FFA"            /* œ.ÿÿë_€..~q€.._ú */
	$"3735 3432 312F 2DEC 2A29 27E5 2422 E0DF"            /* 75421/-ì*)'å$"àß */
	$"9C18 81FF 1BBF 6021 807B 2766 BEFB FAF8"            /* œ.ÿ.¿`!€{'f¾ûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"1884 FF18 3DFF FFFE FDFB FAF8 F6F5 F3F1"            /* .„ÿ.=ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 1684 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"3BFF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* ;ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 1484 FF18 39FF FFFE"            /* èçåãâàßœ.„ÿ.9ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 1284 FF18 37FF FFFE FDFB FAF8"            /* âàßœ.„ÿ.7ÿÿþýûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"1084 FF18 35FF FFFE FDFB FAF8 F6F5 F3F1"            /* .„ÿ.5ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 0E84 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"33FF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* 3ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 0C84 FF18 31FF FFFE"            /* èçåãâàßœ.„ÿ.1ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 0A84 FF18 2FFF FFFE FDFB FAF8"            /* âàßœÂ„ÿ./ÿÿþýûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"0884 FF18 2DFF FFFE FDFB FAF8 F6F5 F3F1"            /* .„ÿ.-ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 0684 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"2BFF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* +ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 0584 FF18 29FF FFFE"            /* èçåãâàßœ.„ÿ.)ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 0384 FF18 26FF FFFE FDFB FAF8"            /* âàßœ.„ÿ.&ÿÿþýûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"0284 FF18 24FF FFFE FDFB FAF8 F6F5 F3F1"            /* .„ÿ.$ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 0184 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"2220 1F1D 1B1A 1816 1513 1110 0E0D 0B0A"            /* " .............Â */
	$"0807 0605 0403 0201 006C 386D 6B00 0004"            /* .........l8mk... */
	$"0800 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF00 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿ..... */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 0000 0000 FFFF FFFF FFFF FFFF FFFF"            /* ......ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ....ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ.ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ.ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ..ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ...ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 FFFF FFFF FFFF FFFF FFFF"            /* ÿ.....ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF"                                                 /* ÿ */
};

data 'icns' (140, "Generic File Icon") {
	$"6963 6E73 0000 1611 4943 4E23 0000 0108"            /* icns....ICN#.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"01FF FFC0 01FF FFE0 07FF FFF0 1FFF FFF8"            /* .ÿÿÀ.ÿÿà.ÿÿð.ÿÿø */
	$"3FFF FFFC 7FFF FFFE 7FFF FFFF FFFF FFFF"            /* ?ÿÿü.ÿÿþ.ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"7FFF FFFF 7FFF FFFF 3FFF FFFF 1FFF FFFF"            /* .ÿÿÿ.ÿÿÿ?ÿÿÿ.ÿÿÿ */
	$"0FFF FFFF 03FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿÿ */
	$"6963 6C34 0000 0208 0000 000E DEED EEDE"            /* icl4........ÞíîÞ */
	$"EDEE EEEE EE00 0000 0000 000D 0000 0000"            /* íîîîî........... */
	$"0000 0000 0EE0 0000 0000 CDED EDEC 0000"            /* .....à....Íííì.. */
	$"C00C 00C0 0AEE 0000 000E ED8E 8EDE E000"            /* À..ÀÂî....íŽŽÞà. */
	$"0C00 C00C 0ECE E000 00AF C888 A8FD FE00"            /* ..À..Îà..¯Èˆ¨ýþ. */
	$"000C 00C0 CE0C EE00 0EFD 8888 899A DFEC"            /* ...ÀÎ.î..ýˆˆ‰šßì */
	$"00C0 0C00 0EC0 CEE0 C9AD 8B8B 8A9F D9F0"            /* .À...ÀÎàÉ­‹‹ŠŸÙð */
	$"C000 C00C 0FEE EEAF DF9D A888 99AF DE9E"            /* À.À..îî¯ß¨ˆ™¯Þž */
	$"000C 00C0 CCDD EDDE E9ED 89B9 9A9F D99E"            /* ...ÀÌÝíÞéí‰¹šŸÙž */
	$"0000 0C0C 00DD CDDE 899D A999 A9FE D99A"            /* .....ÝÍÞ‰©™©þÙš */
	$"00C0 C000 0C00 C0CF A8A8 DAF9 FFFD 8A99"            /* .ÀÀ...ÀÏ¨¨ÚùÿýŠ™ */
	$"0000 0C0C 00CC 0CDE 8A89 DDEE FECB A98A"            /* .....Ì.ÞŠ‰ÝîþË©Š */
	$"000C 00C0 0CC0 C0CF D888 98DC CB88 88AD"            /* ...À.ÀÀÏØˆ˜ÜËˆˆ­ */
	$"C000 C00C 000C 0CCF 0998 A88D D888 8A9C"            /* À.À....ÏÆ˜¨ØˆŠœ */
	$"00C0 0C00 C0C0 C0DE 0B88 88AB 1889 88D0"            /* .À..ÀÀÀÞ.ˆˆ«.‰ˆÐ */
	$"0C00 C00C 0C0C 0CDE 008B 888D C888 9E0C"            /* ..À....Þ.‹ˆÈˆž. */
	$"000C 000C 00C0 0CCF 000B 888D D88A B000"            /* .....À.Ï..ˆØŠ°. */
	$"C000 0CC0 C0CC 00D6 0000 0D9D 1EBC 0000"            /* À..ÀÀÌ.Ö....¼.. */
	$"0000 C000 0C00 CCCF 0000 000E 0000 000C"            /* ..À...ÌÏ........ */
	$"00C0 00C0 C0C0 0CCF 0000 000E 0000 0000"            /* .À.ÀÀÀ.Ï........ */
	$"0C00 0C0C 0C0C C0DF 0000 000F 0000 0000"            /* ......Àß........ */
	$"000C 0000 C0C0 0CCF 0000 000E 0000 0C00"            /* ....ÀÀ.Ï........ */
	$"0C00 C00C 0C00 C0DF 0000 000E 0000 0000"            /* ..À...Àß........ */
	$"C000 0C00 C0CC 00DF 0000 000A 0000 000C"            /* À...ÀÌ.ß...Â.... */
	$"000C 000C 0C00 CCCF 0000 000E 0000 0000"            /* ......ÌÏ........ */
	$"00C0 C0C0 00C0 0CDF 0000 000F 0000 0000"            /* .ÀÀÀ.À.ß........ */
	$"0C00 0000 CC0C C0CF 0000 000E 0000 C000"            /* ....Ì.ÀÏ......À. */
	$"000C 0C0C 00C0 0CDF 0000 000F 0000 000C"            /* .....À.ß........ */
	$"00C0 00C0 00C0 CCCF 0000 000E 0000 0000"            /* .À.À.ÀÌÏ........ */
	$"0C00 C00C 0C00 C0DF 0000 000F 0000 0000"            /* ..À...Àß........ */
	$"00C0 0C00 C00C C0CF 0000 000A 0000 0C00"            /* .À..À.ÀÏ...Â.... */
	$"C00C 00C0 0CC0 0CDF 0000 000F EFFF FAFF"            /* À..À.À.ß....ïÿúÿ */
	$"AFFF FFFF FFFF FFFF 6963 6C38 0000 0408"            /* ¯ÿÿÿÿÿÿÿicl8.... */
	$"0000 0000 0000 00FB 81FB FBFB FBFB FBFB"            /* .......ûûûûûûûû */
	$"FBFB FBFC FBFC FCFC FCFC 0000 0000 0000"            /* ûûûüûüüüüü...... */
	$"0000 0000 0000 0081 0000 0000 0000 0000"            /* ............... */
	$"F500 F5F5 F500 F5F5 F5FC FC00 0000 0000"            /* õ.õõõ.õõõüü..... */
	$"0000 0000 F781 8181 8181 81F7 0000 F500"            /* ....÷÷..õ. */
	$"F5F5 00F5 F5F5 F5F5 F5AC FBFC 0000 0000"            /* õõ.õõõõõõ¬ûü.... */
	$"0000 F5FB AC7B 7CA0 A082 FAAC FC00 00F5"            /* ..õû¬{|  ‚ú¬ü..õ */
	$"00F5 F5F5 F5F5 F5F6 F5FC F7FB AC00 0000"            /* .õõõõõõöõü÷û¬... */
	$"00F5 ACFE 50A1 A1A1 A7CB AD7A DFAC 0000"            /* .õ¬þP¡¡¡§Ë­zß¬.. */
	$"F500 F5F5 F5F5 F6F5 F6AC F5F7 FBAC 0000"            /* õ.õõõõöõö¬õ÷û¬.. */
	$"0081 FF7A 7D9B 9BA1 A1A7 E7AD FAEA FBF5"            /* .ÿz}››¡¡§ç­úêûõ */
	$"00F5 F5F5 F5F5 F5F5 F5AC F5F6 F7FB AC00"            /* .õõõõõõõõ¬õö÷û¬. */
	$"2BE9 AD57 C57D 9BA1 A1A7 E8E9 F9E8 E92B"            /* +é­WÅ}›¡¡§èéùèé+ */
	$"F5F5 F5F5 F5F5 F5F6 F5AC ACAC ACAC ACFD"            /* õõõõõõõöõ¬¬¬¬¬¬ý */
	$"7BE9 FC75 A7A1 A1A1 A7CB ADEA FAA6 D1FB"            /* {éüu§¡¡¡§Ë­êú¦Ñû */
	$"00F5 00F5 F5F5 F6F5 F556 FAFA FAFA FAAC"            /* .õ.õõõöõõVúúúúú¬ */
	$"A6E8 A6F9 CBA7 A1A7 CBAD E8FF FAA7 E8A6"            /* ¦è¦ùË§¡§Ë­èÿú§è¦ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 5656 5656 56FD"            /* .õõõõõõöõõVVVVVý */
	$"A7A7 E8F9 A7E7 A7CB ADE8 EAFD 56E7 A7AD"            /* §§èù§ç§Ë­èêýVç§­ */
	$"F5F5 F5F5 F5F5 F5F5 F5F6 F6F5 F6F6 F8FD"            /* õõõõõõõõõööõööøý */
	$"A7A7 A7A6 56E9 E9D1 D1E0 EA56 A0A7 A7D0"            /* §§§¦VééÑÑàêV §§Ð */
	$"0000 F5F5 F5F5 F6F5 F6F5 F6F6 F5F6 56FD"            /* ..õõõõöõöõööõöVý */
	$"A0A7 E6A7 7B56 FCFD FEFC F87C A7A7 A7A7"            /*  §æ§{Vüýþüø|§§§§ */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F6F5 2B01 55FD"            /* .õõõõõõöõööõ+.Uý */
	$"57C5 A1A1 A7A0 572C 4F51 A1E6 A1A1 E7FA"            /* WÅ¡¡§ W,OQ¡æ¡¡çú */
	$"F500 F5F5 F5F5 F5F5 F6F5 F6F6 F6F6 56FD"            /* õ.õõõõõõöõööööVý */
	$"25A7 A1A1 A1A1 E67A 51E6 A1A1 A1A1 E8F7"            /* %§¡¡¡¡æzQæ¡¡¡¡è÷ */
	$"00F5 F5F5 F5F5 F5F6 F5F5 F6F5 F6F6 56FD"            /* .õõõõõõöõõöõööVý */
	$"0075 A1A1 A19B A757 51A1 A1A1 9BE7 FA00"            /* .u¡¡¡›§WQ¡¡¡›çú. */
	$"F5F5 F5F5 F5F5 F5F5 F6F6 F5F6 F6F6 56FD"            /* õõõõõõõõööõöööVý */
	$"00F5 7CA1 9BA1 A157 4AA1 9B9B E782 00F5"            /* .õ|¡›¡¡WJ¡››ç‚.õ */
	$"F500 F5F5 F5F5 F5F6 F5F5 F6F6 F5F6 F8FE"            /* õ.õõõõõöõõööõöøþ */
	$"0000 F57B A19B E5F9 519B A1AD 8100 0000"            /* ..õ{¡›åùQ›¡­... */
	$"F5F5 00F5 F5F5 F6F5 F6F5 F6F6 F6F6 56A8"            /* õõ.õõõöõöõööööV¨ */
	$"0000 0000 2B7B A6F9 51A6 812B 0000 F5F5"            /* ....+{¦ùQ¦+..õõ */
	$"00F5 F5F5 F5F5 F5F5 F5F6 F6F5 F6F6 F8F4"            /* .õõõõõõõõööõööøô */
	$"0000 0000 0000 00AC 0000 0000 0000 00F5"            /* .......¬.......õ */
	$"F500 F5F5 F5F5 F6F5 F6F5 F6F6 F5F6 56FE"            /* õ.õõõõöõöõööõöVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 F500"            /* .......¬......õ. */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F5F6 F6F5 56FE"            /* .õõõõõõöõöõööõVþ */
	$"0000 0000 0000 00AC 0000 0000 F500 00F5"            /* .......¬....õ..õ */
	$"F500 F5F5 F5F5 F5F5 F6F5 F6F5 F6F6 56FE"            /* õ.õõõõõõöõöõööVþ */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F5F6 F6F6 56FE"            /* .õõõõõõöõöõöööVþ */
	$"0000 0000 0000 00AC 0000 0000 0000 F500"            /* .......¬......õ. */
	$"F5F5 00F5 F5F5 F5F5 F6F5 F6F6 F6F5 56FF"            /* õõ.õõõõõöõöööõVÿ */
	$"0000 0000 0000 00AC 0000 0000 F500 00F5"            /* .......¬....õ..õ */
	$"00F5 F5F5 F5F5 F5F6 F5F6 F6F5 F6F6 56FE"            /* .õõõõõõöõööõööVþ */
	$"0000 0000 0000 00AC 0000 0000 00F5 00F5"            /* .......¬.....õ.õ */
	$"F500 F5F5 F5F5 F6F5 F5F5 F6F6 F5F6 56FF"            /* õ.õõõõöõõõööõöVÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F500"            /* .......ý......õ. */
	$"00F5 F5F5 F5F5 F5F5 F6F6 F5F6 F6F6 F8FF"            /* .õõõõõõõööõöööøÿ */
	$"0000 0000 0000 00AC 0000 0000 F500 00F5"            /* .......¬....õ..õ */
	$"F500 F5F5 F5F5 F5F6 F5F5 F6F5 F6F6 56FF"            /* õ.õõõõõöõõöõööVÿ */
	$"0000 0000 0000 00FD 0000 0000 00F5 00F5"            /* .......ý.....õ.õ */
	$"00F5 F5F5 F5F5 F6F5 F5F6 F6F6 F6F6 F8FF"            /* .õõõõõöõõöööööøÿ */
	$"0000 0000 0000 00FD 0000 0000 0000 F500"            /* .......ý......õ. */
	$"F5F5 00F5 F5F5 F5F6 F5F5 F6F5 F6F6 56FF"            /* õõ.õõõõöõõöõööVÿ */
	$"0000 0000 0000 00FD 0000 0000 F500 00F5"            /* .......ý....õ..õ */
	$"00F5 F5F5 F5F5 F5F5 F6F6 F5F6 F6F5 56FF"            /* .õõõõõõõööõööõVÿ */
	$"0000 0000 0000 00FD 0000 0000 00F5 00F5"            /* .......ý.....õ.õ */
	$"F500 F5F5 F5F5 F6F5 F5F6 F6F6 F6F6 56FF"            /* õ.õõõõöõõöööööVÿ */
	$"0000 0000 0000 00FD FDFD FEFE FEFE FEFE"            /* .......ýýýþþþþþþ */
	$"FEE0 EAE0 F4FF FFFF FFFF FFFF FFFF FFFF"            /* þàêàôÿÿÿÿÿÿÿÿÿÿÿ */
	$"696C 3332 0000 0AE9 84FF 125E 5D5C 5B5A"            /* il32..Âé„ÿ.^]\[Z */
	$"5958 5756 5453 5150 4E4D 4B49 4846 8AFF"            /* YXWVTSQPNMKIHFŠÿ */
	$"135D FFFF FEFD FBFA F8F6 F5F3 F1EF EEEC"            /* .]ÿÿþýûúøöõóñïîì */
	$"EAE8 E744 4286 FF17 B76E 6968 6E66 6DB6"            /* êèçDB†ÿ.·nihnfm¶ */
	$"FBFA F8F6 F5F3 F1EF EEEC EAE8 E742 5942"            /* ûúøöõóñïîìêèçBYB */
	$"83FF 1AEC 5432 775E 4741 526F 2E54 FAF8"            /* ƒÿ.ìT2w^GARo.Túø */
	$"F6F5 F3F1 EFEE ECEA E8E7 40B3 5933 81FF"            /* öõóñïîìêèç@³Y3ÿ */
	$"1CED 391A 8B3D 2F30 2615 1E7E 1639 F8F6"            /* .í9.‹=/0&..~.9øö */
	$"F5F3 F1EF EEEC EAE8 E73D DEB3 5933 80FF"            /* õóñïîìêèç=Þ³Y3€ÿ */
	$"5861 007F 4937 413A 2F24 102B 7600 5FF6"            /* Xa..I7A:/$.+v._ö */
	$"F5F3 F1EF EEEC EAE8 E73B EEDE B359 33FF"            /* õóñïîìêèç;îÞ³Y3ÿ */
	$"C503 2481 293C 3E39 2E23 1904 7623 02C1"            /* Å.$)<>9.#..v#.Á */
	$"F5F3 F1EF EEEC EAE8 E739 3736 3432 312F"            /* õóñïîìêèç976421/ */
	$"7000 3B78 2133 3531 281F 1600 6E3A 0068"            /* p.;x!351(...n:.h */
	$"F5F3 F1EF EEEC EAE8 E79C 8275 1A2D 3E0F"            /* õóñïîìêèçœ‚u.->. */
	$"357D 1828 2927 211A 1100 7934 1034 F5F3"            /* 5}.()'!...y4.4õó */
	$"F1EF EEEC EAE8 E7E5 829C 7F2B 311D 1A8C"            /* ñïîìêèçå‚œ.+1..Œ */
	$"2617 1E1D 1913 031D 8C1B 1D24 F5F3 F1EF"            /* &.......Œ..$õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C29 3523 1953"            /* îìêèçåãâàßœ)5#.S */
	$"880A 090E 0B00 078C 501A 2326 F5F3 F1EF"            /* ˆÂÆ....ŒP.#&õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C27 4823 281C"            /* îìêèçåãâàßœ'H#(. */
	$"6890 4517 1548 9466 1D29 2236 F5F3 F1EF"            /* hE..H”f.)"6õóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C25 7C20 2E2E"            /* îìêèçåãâàßœ%| .. */
	$"2248 7AB4 B57C 4623 2D2F 1769 F5F3 F1EF"            /* "Hz´µ|F#-/.iõóñï */
	$"EEEC EAE8 E7E5 E3E2 E0DF 9C7F 23C9 2832"            /* îìêèçåãâàßœ.#É(2 */
	$"3333 2F13 888D 1C2E 3334 3112 C0F5 F3F1"            /* 33/.ˆ..341.Àõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 21FF 7726"            /* ïîìêèçåãâàßœ!ÿw& */
	$"3938 3924 888F 2C38 393A 1562 F6F5 F3F1"            /* 989$ˆ,89:.böõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 1FFF EE5B"            /* ïîìêèçåãâàßœ.ÿî[ */
	$"283C 3F28 8993 323F 3B18 43F8 F6F5 F3F1"            /* (<?(‰“2?;.Cøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 1CFF FFED"            /* ïîìêèçåãâàßœ.ÿÿí */
	$"722A 2F25 8994 2D26 1960 FAF8 F6F5 F3F1"            /* r*.%‰”-&.`úøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 0018 81FF"            /* ïîìêèçåãâàßœ..ÿ */
	$"1BC3 6F2F 878C 346B BDFB FAF8 F6F5 F3F1"            /* .Ão/‡Œ4k½ûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 1884 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"3DFF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* =ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 1684 FF18 3BFF FFFE"            /* èçåãâàßœ.„ÿ.;ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 1484 FF18 39FF FFFE FDFB FAF8"            /* âàßœ.„ÿ.9ÿÿþýûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"1284 FF18 37FF FFFE FDFB FAF8 F6F5 F3F1"            /* .„ÿ.7ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 1084 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"35FF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* 5ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 0E84 FF18 33FF FFFE"            /* èçåãâàßœ.„ÿ.3ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 0C84 FF18 31FF FFFE FDFB FAF8"            /* âàßœ.„ÿ.1ÿÿþýûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"0A84 FF18 2FFF FFFE FDFB FAF8 F6F5 F3F1"            /* Â„ÿ./ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 0884 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"2DFF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* -ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 0684 FF18 2BFF FFFE"            /* èçåãâàßœ.„ÿ.+ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 0584 FF18 29FF FFFE FDFB FAF8"            /* âàßœ.„ÿ.)ÿÿþýûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"0384 FF18 26FF FFFE FDFB FAF8 F6F5 F3F1"            /* .„ÿ.&ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 0284 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"24FF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* $ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 0184 FF18 2220 1F1D"            /* èçåãâàßœ.„ÿ." .. */
	$"1B1A 1816 1513 1110 0E0D 0B0A 0807 0605"            /* ...........Â.... */
	$"0403 0201 0084 FF12 5E5D 5C5B 5A59 5857"            /* .....„ÿ.^]\[ZYXW */
	$"5654 5351 504E 4D4B 4948 468A FF13 5DFF"            /* VTSQPNMKIHFŠÿ.]ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* ÿþýûúøöõóñïîìêèç */
	$"4442 86FF 17B6 6A67 6C71 656C B7FB FAF8"            /* DB†ÿ.¶jglqel·ûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 4259 4283 FF1A"            /* öõóñïîìêèçBYBƒÿ. */
	$"ED55 2F94 A693 8079 782D 55FA F8F6 F5F3"            /* íU/”¦“€yx-Uúøöõó */
	$"F1EF EEEC EAE8 E740 B359 3381 FF1C EE3E"            /* ñïîìêèç@³Y3ÿ.î> */
	$"19B6 ADA5 987C 5848 861B 3DF8 F6F5 F3F1"            /* .¶­¥˜|XH†.=øöõóñ */
	$"EFEE ECEA E8E7 3DDE B359 3380 FF58 6F00"            /* ïîìêèç=Þ³Y3€ÿXo. */
	$"90AB B2BE AC8A 6942 3E7E 0667 F6F5 F3F1"            /* «²¾¬ŠiB>~.göõóñ */
	$"EFEE ECEA E8E7 3BEE DEB3 5933 FFCC 2233"            /* ïîìêèç;îÞ³Y3ÿÌ"3 */
	$"A891 B1B8 A787 674A 1C7B 4219 C4F5 F3F1"            /* ¨‘±¸§‡gJ.{B.Äõóñ */
	$"EFEE ECEA E8E7 3937 3634 3231 2F88 2A4F"            /* ïîìêèç976421/ˆ*O */
	$"9F7B 979C 8F76 5D42 1273 5F28 75F5 F3F1"            /* Ÿ{—œv]B.s_(uõóñ */
	$"EFEE ECEA E8E7 9C82 751A 2D67 4453 985F"            /* ïîìêèçœ‚u.-gDS˜_ */
	$"7679 7161 4C33 087E 6445 4CF5 F3F1 EFEE"            /* vyqaL3.~dELõóñïî */
	$"ECEA E8E7 E582 9C7F 2B66 5A4E 9C52 535B"            /* ìêèçå‚œ.+fZNœRS[ */
	$"5549 3616 1BA0 5A5A 44F5 F3F1 EFEE ECEA"            /* UI6.. ZZDõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 2971 685F 7C92 2A2E"            /* èçåãâàßœ)qh_|’*. */
	$"3227 0F03 8D8B 6268 46F5 F3F1 EFEE ECEA"            /* 2'..‹bhFõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 2787 7478 6B97 9745"            /* èçåãâàßœ'‡txk——E */
	$"2119 419C A170 786F 53F5 F3F1 EFEE ECEA"            /* !.Aœ¡pxoSõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 25AD 7E86 877E 96A6"            /* èçåãâàßœ%­~†‡~–¦ */
	$"B8C8 B499 8086 8B64 77F5 F3F1 EFEE ECEA"            /* ¸È´™€†‹dwõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 7F23 DD8B 9897 9898"            /* èçåãâàßœ.#Ý‹˜—˜˜ */
	$"739B C88B 9497 9997 48C0 F5F3 F1EF EEEC"            /* s›È‹”—™—HÀõóñïîì */
	$"EAE8 E7E5 E3E2 E0DF 9C21 FEAF 99A9 A6A9"            /* êèçåãâàßœ!þ¯™©¦© */
	$"879C CEA1 A6A8 AC66 6FF6 F5F3 F1EF EEEC"            /* ‡œÎ¡¦¨¬foöõóñïîì */
	$"EAE8 E7E5 E3E2 E0DF 9C1F FFF2 A1A1 B7BA"            /* êèçåãâàßœ.ÿò¡¡·º */
	$"949F D6B1 B9B4 705A F8F6 F5F3 F1EF EEEC"            /* ”ŸÖ±¹´pZøöõóñïîì */
	$"EAE8 E7E5 E3E2 E0DF 9C1C FFFF EFA6 91AC"            /* êèçåãâàßœ.ÿÿï¦‘¬ */
	$"97A1 DBAF 915B 71FA F8F6 F5F3 F1EF EEEC"            /* —¡Û¯‘[qúøöõóñïîì */
	$"EAE8 E7E5 E3E2 E0DF 9C00 1880 FF1C FECF"            /* êèçåãâàßœ..€ÿ.þÏ */
	$"9865 93AF 6680 BEFB FAF8 F6F5 F3F1 EFEE"            /* ˜e“¯f€¾ûúøöõóñïî */
	$"ECEA E8E7 E5E3 E2E0 DF9C 1884 FF18 3DFF"            /* ìêèçåãâàßœ.„ÿ.=ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* ÿþýûúøöõóñïîìêèç */
	$"E5E3 E2E0 DF9C 1684 FF18 3BFF FFFE FDFB"            /* åãâàßœ.„ÿ.;ÿÿþýû */
	$"FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0"            /* úøöõóñïîìêèçåãâà */
	$"DF9C 1484 FF18 39FF FFFE FDFB FAF8 F6F5"            /* ßœ.„ÿ.9ÿÿþýûúøöõ */
	$"F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C 1284"            /* óñïîìêèçåãâàßœ.„ */
	$"FF18 37FF FFFE FDFB FAF8 F6F5 F3F1 EFEE"            /* ÿ.7ÿÿþýûúøöõóñïî */
	$"ECEA E8E7 E5E3 E2E0 DF9C 1084 FF18 35FF"            /* ìêèçåãâàßœ.„ÿ.5ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* ÿþýûúøöõóñïîìêèç */
	$"E5E3 E2E0 DF9C 0E84 FF18 33FF FFFE FDFB"            /* åãâàßœ.„ÿ.3ÿÿþýû */
	$"FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0"            /* úøöõóñïîìêèçåãâà */
	$"DF9C 0C84 FF18 31FF FFFE FDFB FAF8 F6F5"            /* ßœ.„ÿ.1ÿÿþýûúøöõ */
	$"F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C 0A84"            /* óñïîìêèçåãâàßœÂ„ */
	$"FF18 2FFF FFFE FDFB FAF8 F6F5 F3F1 EFEE"            /* ÿ./ÿÿþýûúøöõóñïî */
	$"ECEA E8E7 E5E3 E2E0 DF9C 0884 FF18 2DFF"            /* ìêèçåãâàßœ.„ÿ.-ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* ÿþýûúøöõóñïîìêèç */
	$"E5E3 E2E0 DF9C 0684 FF18 2BFF FFFE FDFB"            /* åãâàßœ.„ÿ.+ÿÿþýû */
	$"FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0"            /* úøöõóñïîìêèçåãâà */
	$"DF9C 0584 FF18 29FF FFFE FDFB FAF8 F6F5"            /* ßœ.„ÿ.)ÿÿþýûúøöõ */
	$"F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C 0384"            /* óñïîìêèçåãâàßœ.„ */
	$"FF18 26FF FFFE FDFB FAF8 F6F5 F3F1 EFEE"            /* ÿ.&ÿÿþýûúøöõóñïî */
	$"ECEA E8E7 E5E3 E2E0 DF9C 0284 FF18 24FF"            /* ìêèçåãâàßœ.„ÿ.$ÿ */
	$"FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7"            /* ÿþýûúøöõóñïîìêèç */
	$"E5E3 E2E0 DF9C 0184 FF18 2220 1F1D 1B1A"            /* åãâàßœ.„ÿ." .... */
	$"1816 1513 1110 0E0D 0B0A 0807 0605 0403"            /* .........Â...... */
	$"0201 0084 FF12 5E5D 5C5B 5A59 5857 5654"            /* ...„ÿ.^]\[ZYXWVT */
	$"5351 504E 4D4B 4948 468A FF13 5DFF FFFE"            /* SQPNMKIHFŠÿ.]ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 4442"            /* ýûúøöõóñïîìêèçDB */
	$"86FF 17B7 7069 666D 666E B6FB FAF8 F6F5"            /* †ÿ.·pifmfn¶ûúøöõ */
	$"F3F1 EFEE ECEA E8E7 4259 4283 FF1A EC54"            /* óñïîìêèçBYBƒÿ.ìT */
	$"3469 4423 2447 6B2E 54FA F8F6 F5F3 F1EF"            /* 4iD#$Gk.Túøöõóñï */
	$"EEEC EAE8 E740 B359 3381 FF04 EC37 1C78"            /* îìêèç@³Y3ÿ.ì7.x */
	$"0F81 0013 147B 1539 F8F6 F5F3 F1EF EEEC"            /* ....{.9øöõóñïîì */
	$"EAE8 E73D DEB3 5933 80FF 035C 0078 1B83"            /* êèç=Þ³Y3€ÿ.\.x.ƒ */
	$"0018 2472 005C F6F5 F3F1 EFEE ECEA E8E7"            /* ..$r.\öõóñïîìêèç */
	$"3BEE DEB3 5933 FFC1 001E 6E85 0017 7316"            /* ;îÞ³Y3ÿÁ..n…..s. */
	$"00C0 F5F3 F1EF EEEC EAE8 E739 3736 3432"            /* .Àõóñïîìêèç97642 */
	$"312F 6700 3463 8500 0D6D 2800 66F5 F3F1"            /* 1/g.4c…..m(.fõóñ */
	$"EFEE ECEA E8E7 9C82 7504 2D2D 0027 7185"            /* ïîìêèçœ‚u.--.'q… */
	$"000D 771D 002C F5F3 F1EF EEEC EAE8 E7E5"            /* ..w..,õóñïîìêèçå */
	$"829C 052B 1600 0787 1583 001A 1D84 0400"            /* ‚œ.+...‡.ƒ...„.. */
	$"15F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* .õóñïîìêèçåãâàßœ */
	$"2916 0000 4386 0581 0016 0A8A 3900 0015"            /* )...C†...ÂŠ9... */
	$"F5F3 F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C27"            /* õóñïîìêèçåãâàßœ' */
	$"2D80 0007 578D 4714 154B 8F4E 8000 112C"            /* -€..WG..KN€.., */
	$"F5F3 F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C48"            /* õóñïîìêèçåãâàßœH */
	$"6781 0005 2768 B1AC 6622 8100 1166 F5F3"            /* g..'h±¬f"..fõó */
	$"F1EF EEEC EAE8 E7E5 E3E2 E0DF 9C23 C183"            /* ñïîìêèçåãâàßœ#Áƒ */
	$"0001 7E72 8300 12C0 F5F3 F1EF EEEC EAE8"            /* ..~rƒ..Àõóñïîìêè */
	$"E7E5 E3E2 E0DF 9C21 FF5D 8200 017E 7082"            /* çåãâàßœ!ÿ]‚..~p‚ */
	$"0014 5DF6 F5F3 F1EF EEEC EAE8 E7E5 E3E2"            /* ..]öõóñïîìêèçåãâ */
	$"E0DF 9C1F FFEC 3D81 0001 7E71 8100 163D"            /* àßœ.ÿì=..~q..= */
	$"F8F6 F5F3 F1EF EEEC EAE8 E7E5 E3E2 E0DF"            /* øöõóñïîìêèçåãâàß */
	$"9C1C FFFF EB5F 8000 017E 7180 0013 5FFA"            /* œ.ÿÿë_€..~q€.._ú */
	$"F8F6 F5F3 F1EF EEEC EAE8 E7E5 E3E2 E0DF"            /* øöõóñïîìêèçåãâàß */
	$"9C18 81FF 1BBF 6021 807B 2766 BEFB FAF8"            /* œ.ÿ.¿`!€{'f¾ûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"1884 FF18 3DFF FFFE FDFB FAF8 F6F5 F3F1"            /* .„ÿ.=ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 1684 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"3BFF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* ;ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 1484 FF18 39FF FFFE"            /* èçåãâàßœ.„ÿ.9ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 1284 FF18 37FF FFFE FDFB FAF8"            /* âàßœ.„ÿ.7ÿÿþýûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"1084 FF18 35FF FFFE FDFB FAF8 F6F5 F3F1"            /* .„ÿ.5ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 0E84 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"33FF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* 3ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 0C84 FF18 31FF FFFE"            /* èçåãâàßœ.„ÿ.1ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 0A84 FF18 2FFF FFFE FDFB FAF8"            /* âàßœÂ„ÿ./ÿÿþýûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"0884 FF18 2DFF FFFE FDFB FAF8 F6F5 F3F1"            /* .„ÿ.-ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 0684 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"2BFF FFFE FDFB FAF8 F6F5 F3F1 EFEE ECEA"            /* +ÿÿþýûúøöõóñïîìê */
	$"E8E7 E5E3 E2E0 DF9C 0584 FF18 29FF FFFE"            /* èçåãâàßœ.„ÿ.)ÿÿþ */
	$"FDFB FAF8 F6F5 F3F1 EFEE ECEA E8E7 E5E3"            /* ýûúøöõóñïîìêèçåã */
	$"E2E0 DF9C 0384 FF18 26FF FFFE FDFB FAF8"            /* âàßœ.„ÿ.&ÿÿþýûúø */
	$"F6F5 F3F1 EFEE ECEA E8E7 E5E3 E2E0 DF9C"            /* öõóñïîìêèçåãâàßœ */
	$"0284 FF18 24FF FFFE FDFB FAF8 F6F5 F3F1"            /* .„ÿ.$ÿÿþýûúøöõóñ */
	$"EFEE ECEA E8E7 E5E3 E2E0 DF9C 0184 FF18"            /* ïîìêèçåãâàßœ.„ÿ. */
	$"2220 1F1D 1B1A 1816 1513 1110 0E0D 0B0A"            /* " .............Â */
	$"0807 0605 0403 0201 006C 386D 6B00 0004"            /* .........l8mk... */
	$"0800 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF00 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿ..... */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 0000 0000 FFFF FFFF FFFF FFFF FFFF"            /* ......ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ....ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ.ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ.ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ..ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿ...ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿ....ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* ÿ......ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF"                                                 /* ÿ */
};

data 'icns' (141, "Folder Icon") {
	$"6963 6E73 0000 13F8 4943 4E23 0000 0108"            /* icns...øICN#.... */
	$"2C00 0000 1B00 0000 A5C0 0000 9A30 0000"            /* ,.......¥À..š0.. */
	$"875D E000 21A2 5800 8875 2E00 920E AA00"            /* ‡]à.!¢X.ˆu..’.ª. */
	$"4123 5500 8489 A980 5044 5580 8111 3A80"            /* A#U.„‰©€PDU€.:€ */
	$"4A48 0D80 9082 A280 44FC 5780 932B 8B00"            /* JH.€‚¢€DüW€“+‹. */
	$"8DAD 5580 275E CB80 8CAB C680 9B5F 6A80"            /* ­U€'^Ë€Œ«Æ€›_j€ */
	$"DDEE E3F0 B55E D6BC 3EFD AB77 156B 65FF"            /* Ýîãðµ^Ö¼>ý«w.keÿ */
	$"0E95 D2DD 096A ABFF 0DAD 56F7 034A A9BE"            /* .•ÒÝÆj«ÿ.­V÷.J©¾ */
	$"01A7 4FDC 004C F378 0000 3AF0 0000 0FE0"            /* .§OÜ.Lóx..:ð...à */
	$"3C00 0000 3F00 0000 FFC0 0000 FFF0 0000"            /* <...?...ÿÀ..ÿð.. */
	$"FFFD E000 FFFF F800 FFFF FE00 FFFF FE00"            /* ÿýà.ÿÿø.ÿÿþ.ÿÿþ. */
	$"FFFF FF00 FFFF FF80 FFFF FF80 FFFF FF80"            /* ÿÿÿ.ÿÿÿ€ÿÿÿ€ÿÿÿ€ */
	$"FFFF FF80 FFFF FF80 FFFF FF80 FFFF FF80"            /* ÿÿÿ€ÿÿÿ€ÿÿÿ€ÿÿÿ€ */
	$"FFFF FF80 FFFF FF80 FFFF FF80 FFFF FF80"            /* ÿÿÿ€ÿÿÿ€ÿÿÿ€ÿÿÿ€ */
	$"FFFF FFF0 FFFF FFFC 3FFF FFFF 1FFF FFFF"            /* ÿÿÿðÿÿÿü?ÿÿÿ.ÿÿÿ */
	$"1FFF FFFF 1FFF FFFF 1FFF FFFF 0FFF FFFE"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿþ */
	$"07FF FFFC 03FF FFF8 0000 3FF0 0000 0FE0"            /* .ÿÿü.ÿÿø..?ð...à */
	$"6963 6C34 0000 0208 005D E500 0000 0000"            /* icl4.....]å..... */
	$"0000 0000 0000 0000 007D 7D5E 0000 0000"            /* .........}}^.... */
	$"0000 0000 0000 0000 5D5C C7D5 E500 0000"            /* ........]\ÇÕå... */
	$"0000 0000 0000 0000 D075 5CDC 7D5E 0000"            /* ........Ðu\Ü}^.. */
	$"0000 0000 0000 0000 5000 755D C7D5 E50E"            /* ........P.u]ÇÕå. */
	$"E5E0 0000 0000 0000 7CCC 00C5 5CDC 7D5D"            /* åà......|Ì.Å\Ü}] */
	$"7D55 E000 0000 0000 DCCC CCC0 755D C7DC"            /* }Uà.....ÜÌÌÀu]ÇÜ */
	$"C7CD 6EE0 0000 0000 5C0C CCCC 00C7 55CD"            /* ÇÍnà....\.ÌÌ.ÇUÍ */
	$"5C5C 7D50 0000 0000 DCCC 0CCC CCC0 DD6D"            /* \\}P....ÜÌ.ÌÌÀÝm */
	$"C5CD C57E 0000 0000 5CCC CCCC CCCC CCC5"            /* ÅÍÅ~....\ÌÌÌÌÌÌÅ */
	$"5C7D 5CD5 E000 0000 DCCC CCCC CCCC CCC0"            /* \}\Õà...ÜÌÌÌÌÌÌÀ */
	$"75DD D7C5 E000 0000 5CC0 CCCC CCCC CCCD"            /* uÝ×Åà...\ÀÌÌÌÌÌÍ */
	$"00D5 5C57 E000 0000 DCCC CCCC CCCC CCCC"            /* .Õ\Wà...ÜÌÌÌÌÌÌÌ */
	$"CCC0 D6C5 E000 0000 5CCC CCCC CCCC CCCC"            /* ÌÀÖÅà...\ÌÌÌÌÌÌÌ */
	$"DCCD C05D F000 0000 DCCC CCCC 5D5D 5EDC"            /* ÜÍÀ]ð...ÜÌÌÌ]]^Ü */
	$"CCDC C557 5000 0000 5CCC CC5E DD88 EDE5"            /* ÌÜÅWP...\ÌÌ^Ýˆíå */
	$"CDC0 7CE5 E000 0000 50CC CEED 88A8 99DE"            /* ÍÀ|åà...PÌÎíˆ¨™Þ */
	$"ECCD 05D5 9000 0000 7CCC DFD8 888A 99ED"            /* ìÍ.Õ...|ÌßØˆŠ™í */
	$"F5CC DC95 E000 0000 ECCC FED8 8188 A9FD"            /* õÌÜ•à...ìÌþØˆ©ý */
	$"9FCC C557 E000 0000 5CCD F8DA 8989 99FD"            /* ŸÌÅWà...\ÍøÚ‰‰™ý */
	$"E95C C7ED F000 0000 57CE 9ED8 9A8A 9AFD"            /* é\Çíð...WÎžØšŠšý */
	$"E9EC CC55 EFEF 0000 ED59 99D9 A89A 9FED"            /* éìÌUïï..íY™Ù¨šŸí */
	$"99ED DD6D FEFE FA00 00EE 9A8D 9FAF FFD8"            /* ™íÝmþþú..îšŸ¯ÿØ */
	$"A8AC C5E5 DFEF EFEF 0008 A89D DEF9 EC8A"            /* ¨¬Ååßïïï..¨ÞùìŠ */
	$"89EC DCE7 FEFE FEFE 000D 8989 BDCC D8E8"            /* ‰ìÜçþþþþ..‰‰½ÌØè */
	$"99DD C75D 5FEF EFEF 000C A8B8 88DD 8988"            /* ™ÝÇ]_ïïï..¨¸ˆÝ‰ˆ */
	$"8EDC DCE5 FEFE FEFE 0000 D88A 88DB 888B"            /* ŽÜÜåþþþþ..ØŠˆÛˆ‹ */
	$"9BCD C55D EFEF EFEF 0000 0B88 88DC 8B88"            /* ›ÍÅ]ïïïï...ˆˆÜ‹ˆ */
	$"ECDC DCE7 FEFE FEF0 0000 00B8 88DC 88AE"            /* ìÜÜçþþþð...¸ˆÜˆ® */
	$"D7CD DD55 EEEF EF00 0000 0000 BACD 8DC0"            /* ×ÍÝUîïï.....ºÍÀ */
	$"E555 C7E5 EFFE F000 0000 0000 0000 0000"            /* åUÇåïþð......... */
	$"00E5 5D5D EEFE 0000 0000 0000 0000 0000"            /* .å]]îþ.......... */
	$"0000 95EE FFE0 0000 6963 6C38 0000 0408"            /* ..•îÿà..icl8.... */
	$"0000 80AA ABAB 0000 0000 0000 0000 0000"            /* ..€ª««.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 8054 7F80 ABAB 0000 0000 0000 0000"            /* ..€T.€««........ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"7F80 AA55 5454 7F80 ABAB 0000 0000 0000"            /* .€ªUTT.€««...... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"802A 55AA AB7E 5454 7F80 ABAB 0000 0000"            /* €*Uª«~TT.€««.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"7F2A 2AF5 54AA AB54 547E 7F80 ABAB 00AB"            /* .**õTª«TT~.€««.« */
	$"ABAB AB00 0000 0000 0000 0000 0000 0000"            /* «««............. */
	$"802A 2A2A 2AF5 54AA AB55 5454 7F80 AB7F"            /* €****õTª«UTT.€«. */
	$"547F 80AB AB00 0000 0000 0000 0000 0000"            /* T.€««........... */
	$"7F2A 2A2A 2A2A 2A2A 54AA AB7E 547E 8054"            /* .*******Tª«~T~€T */
	$"7F54 7E7F 80AB AB00 0000 0000 0000 0000"            /* .T~.€««......... */
	$"802A 2A2A 2A2A 2A2A 2AF5 7E80 AB55 5455"            /* €********õ~€«UTU */
	$"547E 5555 7E7F AB00 0000 0000 0000 0000"            /* T~UU~.«......... */
	$"802A 2A2A 2A2A 302A 2A2A 2A2A 5AAA AB7F"            /* €*****0*****Zª«. */
	$"545B 785B 797F 80AA 0000 0000 0000 0000"            /* T[x[y.€ª........ */
	$"AA2A 2A2A 2A2A 2AF7 2A54 2A2A 2A24 54AA"            /* ª******÷*T***$Tª */
	$"AB7F 795A 785B 7FAB AB00 0000 0000 0000"            /* «.yZx[.««....... */
	$"802A 2A30 4E2A 2A2A 4E2A 314E 302A 5400"            /* €**0N***N*1N0*T. */
	$"7E80 AB7F 5B78 7F80 AB00 0000 0000 0000"            /* ~€«.[x.€«....... */
	$"AA2A 2A2A 2B30 2A4E 302A 2A30 4EF7 2A54"            /* ª***+0*N0**0N÷*T */
	$"2AF5 5AA4 AB7F 557F B200 0000 0000 0000"            /* *õZ¤«.U.²....... */
	$"802A 2A4E 2A2A 2A30 2A4E 2A2A 304E 2A2A"            /* €**N***0*N**0N** */
	$"554E 30F5 78AB 7F7F D000 0000 0000 0000"            /* UN0õx«..Ð....... */
	$"AA2A 302A 2A2A 304E 2A31 4E30 4E2A 3054"            /* ª*0***0N*1N0N*0T */
	$"2A30 4E54 302A AB7F AB00 0000 0000 0000"            /* *0NT0*«.«....... */
	$"802A 4E30 2A4F 2A56 FB81 8181 81FB F84E"            /* €*N0*O*VûûøN */
	$"304E 302A 5455 AB86 D000 0000 0000 0000"            /* 0N0*TU«†Ð....... */
	$"AA2A 2A2A 4E30 FBAC 7A7C 7CA6 7C81 AC87"            /* ª***N0û¬z||¦|¬‡ */
	$"4F30 4F54 2A7F ABA3 B200 0000 0000 0000"            /* O0OT*.«£²....... */
	$"8054 2B2A 2BAC FD51 A19B A1A1 E8A7 80D1"            /* €T+*+¬ýQ¡›¡¡è§€Ñ */
	$"FC30 542A 5554 FD7F D000 0000 0000 0000"            /* ü0T*UTý.Ð....... */
	$"AA2A 2A2A 81FF 57A0 A19B A1A1 A7E8 ADFA"            /* ª***ÿW ¡›¡¡§è­ú */
	$"F4A6 2A54 2A7F ABA4 B100 0000 0000 0000"            /* ô¦*T*.«¤±....... */
	$"AA2A 3055 D1FD 75A1 9BA1 A1A1 E7AD D1F9"            /* ª*0UÑýu¡›¡¡¡ç­Ñù */
	$"ADE9 5554 5455 AB86 D000 0000 0000 0000"            /* ­éUTTU«†Ð....... */
	$"AB2A 2A7B E9A6 57A7 A1A1 A1A7 A7E7 E980"            /* «**{é¦W§¡¡¡§§çé€ */
	$"A6EA 7B54 2A7F ABAA AC00 0000 0000 0000"            /* ¦ê{T*.«ª¬....... */
	$"807F 54A6 E8AC 7BCB A7A1 A7CB ADD1 E056"            /* €.T¦è¬{Ë§¡§Ë­ÑàV */
	$"A6E8 AC2A 5455 ABAA ABFE FDFD 0000 0000"            /* ¦è¬*TU«ª«þýý.... */
	$"ABAB AAA7 E7A7 56A6 E7A7 E7AD E8E9 FDF9"            /* ««ª§ç§V¦ç§ç­èéýù */
	$"E7A7 AD54 547F AB80 FDFD FDFD FDFD 0000"            /* ç§­TT.«€ýýýýýý.. */
	$"0000 ABA7 A7E7 82F9 FED1 D1D1 E0E0 56A0"            /* ..«§§ç‚ùþÑÑÑààV  */
	$"A7CB A754 3079 B1A4 B1AD D0B2 FDFD FDFD"            /* §Ë§T0y±¤±­Ð²ýýýý */
	$"0000 007C E6A7 A79F F9FC FDFE FBF9 9FA7"            /* ...|æ§§ŸùüýþûùŸ§ */
	$"A1A7 AC54 5455 AB86 ACD6 FDAD FDFD FDFD"            /* ¡§¬TTU«†¬Öý­ýýýý */
	$"0000 007B A1A1 A1A7 A07B F84F 7B7C A1A7"            /* ...{¡¡¡§ {øO{|¡§ */
	$"A1CB 7B54 547F ABA4 ABFD FDFD FDFD FDFD"            /* ¡Ë{TT.«¤«ýýýýýýý */
	$"0000 00F6 A1A1 A1A1 A1E6 F951 E5A1 A1A1"            /* ...ö¡¡¡¡¡æùQå¡¡¡ */
	$"A1E7 5530 5455 ABA4 B2FD FDFD FDFD FDFD"            /* ¡çU0TU«¤²ýýýýýýý */
	$"0000 0000 57E4 A1A1 9BA7 F951 A1A1 A19B"            /* ....Wä¡¡›§ùQ¡¡¡› */
	$"E781 544E 557E AB86 D0FD FDFD FDFD FDFD"            /* çTNU~«†Ðýýýýýýý */
	$"0000 0000 007C A177 A1A1 5150 9B9B 9BA7"            /* .....|¡w¡¡QP›››§ */
	$"FB4F 3054 545B ABA4 ABFD FDFD FDFD FD00"            /* ûO0TT[«¤«ýýýýýý. */
	$"0000 0000 0000 7BA1 9BA1 564A A1A1 A7AC"            /* ......{¡›¡VJ¡¡§¬ */
	$"805B 7855 5455 AB86 D0B2 FDFD FDFD 0000"            /* €[xUTU«†Ð²ýýýý.. */
	$"0000 0000 0000 002B 7BA6 50F8 A681 F700"            /* .......+{¦Pø¦÷. */
	$"ABAB 867F 557F ABA4 B1AD FDFD FD00 0000"            /* ««†.U.«¤±­ýýý... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 CFAB AA7F AB86 D0FD FDFD 0000 0000"            /* ..Ï«ª.«†Ðýýý.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 ABAB D0AC FDFD FD00 0000 0000"            /* ....««Ð¬ýýý..... */
	$"696C 3332 0000 08D0 05FF FF5F 502B 3C99"            /* il32...Ð.ÿÿ_P+<™ */
	$"FF05 5099 6E50 303C 95FF 0966 5F39 8699"            /* ÿ.P™nP0<•ÿÆf_9†™ */
	$"9D6E 5030 3C93 FF0B 64E4 814C 3986 9794"            /* nP0<“ÿ.däL9†—” */
	$"6E50 303C 91FF 0E62 CCCC E481 4C3C 8493"            /* nP0<‘ÿ.bÌÌäL<„“ */
	$"926E 5030 3CFF 8030 003C 8AFF 1460 D0CF"            /* ’nP0<ÿ€0.<Šÿ.`ÐÏ */
	$"CDCC DD81 4C3C 8495 8D6E 5030 6E8D 6E50"            /* ÍÌÝL<„•nP0nnP */
	$"303C 88FF 165F CFCD CDCB CAC9 DD81 4C3C"            /* 0<ˆÿ._ÏÍÍËÊÉÝL< */
	$"7E8D 8B6E 8D86 838B 6E50 303C 86FF 165D"            /* ~‹n†ƒ‹nP0<†ÿ.] */
	$"CECC CBCA C9C8 C7C5 DD81 4C3C 7E8B 8886"            /* ÎÌËÊÉÈÇÅÝL<~‹ˆ† */
	$"8381 8086 5A30 86FF 175B CCCB CAC9 C7C7"            /* ƒ€†Z0†ÿ.[ÌËÊÉÇÇ */
	$"C5C4 C3C2 DD81 4C3C 6E86 8381 807E 7850"            /* ÅÄÃÂÝL<n†ƒ€~xP */
	$"3C85 FF18 5ACB C9C9 C7C6 C5C4 C3C2 C0BF"            /* <…ÿ.ZËÉÉÇÆÅÄÃÂÀ¿ */
	$"BEDD 814C 3C6E 8180 7E78 6E30 3C84 FF18"            /* ¾ÝL<n€~xn0<„ÿ. */
	$"58CA C8C7 C7C6 C4C3 C1C1 BFBE BDBC BADD"            /* XÊÈÇÇÆÄÃÁÁ¿¾½¼ºÝ */
	$"814C 3C6E 7E7D 7850 3084 FF18 56C9 C7C7"            /* L<n~}xP0„ÿ.VÉÇÇ */
	$"C5C4 C3C1 C0BF BEBD BCBB B9B8 B7DD 814C"            /* ÅÄÃÁÀ¿¾½¼»¹¸·ÝL */
	$"3C6E 786E 2384 FF18 54C7 C6C5 C4C3 C2C0"            /* <nxn#„ÿ.TÇÆÅÄÃÂÀ */
	$"C0BE BDBC BABA B9B7 B6B4 B3DD 813C 6E6A"            /* À¾½¼ºº¹·¶´³Ý<nj */
	$"2384 FF18 53C7 C5C4 C3C2 C1BF BEBD BCBA"            /* #„ÿ.SÇÅÄÃÂÁ¿¾½¼º */
	$"BAB8 B7B6 B4B4 B2B1 B0D1 3060 2384 FF18"            /* º¸·¶´´²±°Ñ0`#„ÿ. */
	$"50C5 C4C3 C1C1 BE9A 6566 6D6B 6465 96B2"            /* PÅÄÃÁÁ¾šefmkde–² */
	$"B4B2 B1B0 AF7E 305A 2384 FF18 4FC4 C3C1"            /* ´²±°¯~0Z#„ÿ.OÄÃÁ */
	$"C0BC 4F3B 785E 4741 5270 364D B3B1 B0AF"            /* À¼O;x^GARp6M³±°¯ */
	$"AD7E 3058 2384 FF18 4DC3 C2C0 C03D 238B"            /* ­~0X#„ÿ.MÃÂÀÀ=#‹ */
	$"3D2F 3026 151E 7E1E 3CB3 AFAD AD7E 3051"            /* =/0&..~.<³¯­­~0Q */
	$"2384 FF18 4CC2 C0BF 6200 7F49 3741 3A2F"            /* #„ÿ.LÂÀ¿b..I7A:/ */
	$"2410 2B76 005D AFAC AB7E 304C 2384 FF18"            /* $.+v.]¯¬«~0L#„ÿ. */
	$"49C0 C0A5 0D24 8129 3C3E 392E 2319 047A"            /* IÀÀ¥.$)<>9.#..z */
	$"2507 A8AC AA7E 304C 2384 FF18 48C0 BE6E"            /* %.¨¬ª~0L#„ÿ.HÀ¾n */
	$"023B 7821 3335 3128 1F16 0275 3F01 67AA"            /* .;x!351(...u?.gª */
	$"A87E 304C 2384 FF18 467E 9939 0E34 7E18"            /* ¨~0L#„ÿ.F~™9.4~. */
	$"2829 2721 1A11 0179 3711 30A9 A87E 304C"            /* ()'!...y7.0©¨~0L */
	$"2380 2281 FF18 3037 4429 1D1B 8E2C 181E"            /* #€"ÿ.07D)..Ž,.. */
	$"1D19 1304 2188 1D1D 20A7 A67E 304C 2382"            /* ....!ˆ.. §¦~0L#‚ */
	$"2281 FF16 3029 231A 5176 0E0A 0E0B 0007"            /* "ÿ.0)#.Qv.Â.... */
	$"8453 1B23 21A6 A57E 304C 2384 2280 FF15"            /* „S.#!¦¥~0L#„"€ÿ. */
	$"4523 291E 4F89 4215 1444 815C 2229 2231"            /* E#).O‰B..D\")"1 */
	$"A5A4 7E30 4C23 8422 80FF 158B 232E 2E27"            /* ¥¤~0L#„"€ÿ.‹#..' */
	$"4D60 A2A9 6C49 272D 2F17 66A4 A37E 304C"            /* M`¢©lI'-/.f¤£~0L */
	$"2384 2280 FF15 CF32 3233 332E 1883 8B20"            /* #„"€ÿ.Ï2233..ƒ‹  */
	$"2F33 3431 18A3 A3A2 7E30 4C23 8422 80FF"            /* /341.££¢~0L#„"€ÿ */
	$"15FC 842B 3938 3924 8890 2C38 393A 1B5B"            /* .ü„+989$ˆ,89:.[ */
	$"A4A2 A07E 304C 2384 2281 FF14 F160 303C"            /* ¤¢ ~0L#„"ÿ.ñ`0< */
	$"3F28 8EA5 323F 3B20 46A9 A1A0 9F7E 304C"            /* ?(Ž¥2?; F©¡ Ÿ~0L */
	$"2383 2283 FF13 ED7B 3430 258C B030 2824"            /* #ƒ"ƒÿ.í{40%Œ°0($ */
	$"3462 7E90 9D9D 7E30 4C23 8222 85FF 12FC"            /* 4b~~0L#‚"…ÿ.ü */
	$"C474 37A8 9A36 6BBF FC30 3744 6E7E 7E30"            /* Ät7¨š6k¿ü07Dn~~0 */
	$"4C23 8122 92FF 0630 3744 6630 4C23 8022"            /* L#"’ÿ.07Df0L#€" */
	$"95FF 0330 3023 2380 2282 FF05 FFFF 5D50"            /* •ÿ.00##€"‚ÿ.ÿÿ]P */
	$"2B3C 99FF 0550 996D 5030 3C95 FF09 645D"            /* +<™ÿ.P™mP0<•ÿÆd] */
	$"3985 999D 6D50 303C 93FF 0B63 E380 4C39"            /* 9…™mP0<“ÿ.cã€L9 */
	$"8597 956D 5030 3C91 FF0E 61CC CCE3 804C"            /* …—•mP0<‘ÿ.aÌÌã€L */
	$"3C84 9392 6D50 303C FF80 3000 3C8A FF14"            /* <„“’mP0<ÿ€0.<Šÿ. */
	$"5FCF CECD CBDD 804C 3C84 958D 6D50 306D"            /* _ÏÎÍËÝ€L<„•mP0m */
	$"8D6D 5030 3C88 FF16 5ECD CDCC CAC9 C8DD"            /* mP0<ˆÿ.^ÍÍÌÊÉÈÝ */
	$"804C 3C7F 8D8B 6D8D 8582 8B6D 5030 3C86"            /* €L<.‹m…‚‹mP0<† */
	$"FF16 5BCD CCCB C9C8 C7C6 C5DD 804C 3C7F"            /* ÿ.[ÍÌËÉÈÇÆÅÝ€L<. */
	$"8B88 8582 817F 855B 3086 FF17 5ACB CAC9"            /* ‹ˆ…‚.…[0†ÿ.ZËÊÉ */
	$"C8C7 C5C5 C3C2 C1DD 804C 3C6E 8582 817F"            /* ÈÇÅÅÃÂÁÝ€L<n…‚. */
	$"7E78 503C 85FF 1858 CAC9 C8C7 C6C4 C3C2"            /* ~xP<…ÿ.XÊÉÈÇÆÄÃÂ */
	$"C1C0 BEBE DD80 4C3C 6E81 7F7E 786D 303C"            /* ÁÀ¾¾Ý€L<n.~xm0< */
	$"84FF 1856 C9C8 C7C5 C4C3 C3C1 C0BF BDBC"            /* „ÿ.VÉÈÇÅÄÃÃÁÀ¿½¼ */
	$"BBBA DD80 4C3C 6E7E 7D78 5030 84FF 1855"            /* »ºÝ€L<n~}xP0„ÿ.U */
	$"C8C7 C6C5 C3C2 C1C0 BEBE BDBB BAB9 B8B6"            /* ÈÇÆÅÃÂÁÀ¾¾½»º¹¸¶ */
	$"DD80 4C3C 6E78 6D23 84FF 1853 C7C5 C4C4"            /* Ý€L<nxm#„ÿ.SÇÅÄÄ */
	$"C2C1 C0BE BEBD BBBA B9B7 B7B6 B4B3 DD80"            /* ÂÁÀ¾¾½»º¹··¶´³Ý€ */
	$"3C6E 6923 84FF 1852 C5C4 C4C2 C1BF BFBD"            /* <ni#„ÿ.RÅÄÄÂÁ¿¿½ */
	$"BCBB BAB9 B7B7 B5B4 B3B2 B0B0 D030 6123"            /* ¼»º¹··µ´³²°°Ð0a# */
	$"84FF 1850 C4C3 C2C1 BFBD 9A61 6471 6E63"            /* „ÿ.PÄÃÂÁ¿½šadqnc */
	$"6396 B2B3 B2B1 AFAF 7D30 5B23 84FF 184E"            /* c–²³²±¯¯}0[#„ÿ.N */
	$"C3C3 C1C0 BC50 3896 A593 8078 7936 4EB3"            /* ÃÃÁÀ¼P8–¥“€xy6N³ */
	$"B1AF AEAD 7D30 5823 84FF 184C C2C1 C0BF"            /* ±¯®­}0X#„ÿ.LÂÁÀ¿ */
	$"4221 B6AD A598 7C58 4886 2441 B4AE AEAC"            /* B!¶­¥˜|XH†$A´®®¬ */
	$"7D30 5123 84FF 004A 80C0 146E 0390 ABB2"            /* }0Q#„ÿ.J€À.n.«² */
	$"BEAC 8A69 423E 7E0B 63AE ACAB 7D30 4C23"            /* ¾¬ŠiB>~.c®¬«}0L# */
	$"84FF 1849 C0BF AD2B 33A8 91B1 B8A7 8767"            /* „ÿ.IÀ¿­+3¨‘±¸§‡g */
	$"4A1B 7F43 20AA ABA9 7D30 4C23 84FF 1848"            /* J..C ª«©}0L#„ÿ.H */
	$"BFBD 842C 4E9F 7B97 9C8F 765D 4215 7C61"            /* ¿½„,NŸ{—œv]B.|a */
	$"2973 AAA9 7D30 4C23 84FF 1846 7E99 6344"            /* )sª©}0L#„ÿ.F~™cD */
	$"5399 5F76 7971 614C 3309 8066 4547 A9A7"            /* S™_vyqaL3Æ€fEG©§ */
	$"7D30 4C23 8022 81FF 1830 3744 5E5A 4F9C"            /* }0L#€"ÿ.07D^ZOœ */
	$"5753 5B55 4936 1621 9D5C 5A40 A8A6 7D30"            /* WS[UI6.!\Z@¨¦}0 */
	$"4C23 8222 81FF 1630 6568 6078 862C 3033"            /* L#‚"ÿ.0eh`x†,03 */
	$"2710 0389 8B63 6842 A7A5 7D30 4C23 8422"            /* '..‰‹chB§¥}0L#„" */
	$"80FF 1585 7478 6C7B 9246 1F18 4593 9472"            /* €ÿ.…txl{’F..E“”r */
	$"786F 4FA5 A47D 304C 2384 2280 FF15 B680"            /* xoO¥¤}0L#„"€ÿ.¶€ */
	$"8687 8199 8DA8 BAA4 9983 858B 6475 A4A3"            /* †‡™¨º¤™ƒ…‹du¤£ */
	$"7D30 4C23 8422 80FF 15E0 9198 9798 9876"            /* }0L#„"€ÿ.à‘˜—˜˜v */
	$"95C3 8E94 9799 974E A4A3 A17D 304C 2384"            /* •ÃŽ”—™—N¤£¡}0L#„ */
	$"2280 FF15 FDB7 9DA9 A6A9 879D CEA1 A6A8"            /* "€ÿ.ý·©¦©‡Î¡¦¨ */
	$"AC6C 67A4 A2A1 7D30 4C23 8422 81FF 14F5"            /* ¬lg¤¢¡}0L#„"ÿ.õ */
	$"A4A6 B6BA 94A4 DCB1 B8B3 785D A9A1 A09F"            /* ¤¦¶º”¤Ü±¸³x]©¡ Ÿ */
	$"7D30 4C23 8322 83FF 13F1 AB98 AD98 A6E2"            /* }0L#ƒ"ƒÿ.ñ«˜­˜¦â */
	$"B092 6643 627E 919D 9D7D 304C 2382 2285"            /* °’fCb~‘}0L#‚"… */
	$"FF12 FDD0 9E6D B1B7 677F C1FC 3037 446E"            /* ÿ.ýÐžm±·g.Áü07Dn */
	$"7D7D 304C 2381 2292 FF06 3037 4466 304C"            /* }}0L#"’ÿ.07Df0L */
	$"2380 2295 FF03 3030 2323 8022 82FF 05FF"            /* #€"•ÿ.00##€"‚ÿ.ÿ */
	$"FFAF A156 7999 FF05 A1FF C3A1 6179 95FF"            /* ÿ¯¡Vy™ÿ.¡ÿÃ¡ay•ÿ */
	$"09B8 AF72 EAFF FCC3 A161 7993 FF0B B6FE"            /* Æ¸¯rêÿüÃ¡ay“ÿ.¶þ */
	$"E699 72EA FDFA C3A1 6179 91FF 0EB3 FFFF"            /* æ™rêýúÃ¡ay‘ÿ.³ÿÿ */
	$"FEE6 9979 E8F9 F8C3 A161 79FF 8061 0079"            /* þæ™yèùøÃ¡ayÿ€a.y */
	$"8AFF 14B0 FDFC FDFC FEE6 9979 E8FB F3C3"            /* Šÿ.°ýüýüþæ™yèûóÃ */
	$"A161 C3F3 C3A1 6179 88FF 01AD FD82 FC0F"            /* ¡aÃóÃ¡ayˆÿ.­ý‚ü. */
	$"FEE6 9979 E2F3 F0C3 F3EA E7F0 C3A1 6179"            /* þæ™yâóðÃóêçðÃ¡ay */
	$"86FF 02AB FCFD 81FC 0FFB FBFE E699 79E2"            /* †ÿ.«üýü.ûûþæ™yâ */
	$"F0ED EAE7 E5E4 EAB5 6186 FF00 A880 FC01"            /* ðíêçåäêµa†ÿ.¨€ü. */
	$"FBFC 81FB 0DFA FEE6 9979 C4EA E7E5 E4E2"            /* ûüû.úþæ™yÄêçåäâ */
	$"DBA1 7985 FF04 A6FC FCFB FC80 FB82 FA0B"            /* Û¡y…ÿ.¦üüûü€û‚ú. */
	$"FEE6 9979 C4E5 E4E2 DBC3 6179 84FF 01A3"            /* þæ™yÄåäâÛÃay„ÿ.£ */
	$"FC82 FB81 FA0D F9FA FAF9 FEE6 9979 C4E2"            /* ü‚ûú.ùúúùþæ™yÄâ */
	$"E1DB A161 84FF 00A1 83FB 82FA 82F9 07FE"            /* áÛ¡a„ÿ.¡ƒû‚ú‚ù.þ */
	$"E699 79C4 DBC3 4784 FF04 9FFB FBFA FB81"            /* æ™yÄÛÃG„ÿ.Ÿûûúû */
	$"FA02 F9FA FA80 F909 F8F8 F9F8 FEE6 79C4"            /* ú.ùúú€ùÆøøùøþæyÄ */
	$"BD47 84FF 019C FC81 FB81 FA81 F983 F804"            /* ½G„ÿ.œüûúùƒø. */
	$"F7FE 61C2 4784 FF00 9980 FB14 FAFA F7B8"            /* ÷þaÂG„ÿ.™€û.úú÷¸ */
	$"6D6B 6F6E 686A B8F5 F8F7 F8F7 F7D9 61B5"            /* mkonhj¸õø÷ø÷÷Ùaµ */
	$"4784 FF01 96FB 80FA 0CE7 5F3D 6B44 2324"            /* G„ÿ.–û€ú.ç_=kD#$ */
	$"466C 3760 E5F8 80F7 03D9 61B0 4784 FF08"            /* Fl7`åø€÷.Ùa°G„ÿ. */
	$"94FB FAFB EB3D 2477 0F81 0004 147B 1D3E"            /* ”ûúûë=$w....{.> */
	$"E980 F703 D961 A347 84FF 0791 FBFA F76C"            /* é€÷.Ùa£G„ÿ.‘ûú÷l */
	$"0077 1B83 000A 2472 006B F4F7 F6D9 6199"            /* .w.ƒ.Â$r.kô÷öÙa™ */
	$"4784 FF06 8FFA FAC3 031E 6E85 0009 7B19"            /* G„ÿ.úúÃ..n….Æ{. */
	$"04CD F7F6 D961 9947 84FF 068C FAF9 7600"            /* .Í÷öÙa™G„ÿ.Œúùv. */
	$"3463 8500 0993 3800 6AF6 F6D9 6199 4784"            /* 4c….Æ“8.jööÙa™G„ */
	$"FF06 8AD8 FF29 0027 7285 0009 8627 002B"            /* ÿ.ŠØÿ).'r….Æ†'.+ */
	$"F6F6 D961 9947 8022 81FF 0761 6D87 1300"            /* ööÙa™G€"ÿ.am‡.. */
	$"0795 1E83 000A 258B 0500 15F6 F6D9 6199"            /* .•.ƒ.Â%‹...ööÙa™ */
	$"4782 2281 FF06 610D 0000 4D83 0A81 000B"            /* G‚"ÿ.a...MƒÂ.. */
	$"0A87 3F00 0015 F5F5 D961 9947 8422 80FF"            /* Â‡?...õõÙa™G„"€ÿ */
	$"0B29 0000 0153 8B46 1114 4C8B 5480 0006"            /* .)...S‹F..L‹T€.. */
	$"2BF6 F5D9 6199 4784 2280 FF00 7781 0005"            /* +öõÙa™G„"€ÿ.w.. */
	$"2C5F BEB7 642A 8100 066A F5F5 D961 9947"            /* ,_¾·d*..jõõÙa™G */
	$"8422 80FF 01C6 0882 0001 7F78 8200 0705"            /* „"€ÿ.Æ.‚...x‚... */
	$"CCF5 F5D9 6199 4784 2280 FF01 FC6E 8200"            /* ÌõõÙa™G„"€ÿ.ün‚. */
	$"017E 7282 0007 6CF3 F5F5 D961 9947 8422"            /* .~r‚..lóõõÙa™G„" */
	$"81FF 01EF 4281 0001 8489 8100 0142 E780"            /* ÿ.ïB..„‰..Bç€ */
	$"F503 D961 9947 8322 83FF 13EA 690A 0000"            /* õ.Ùa™Gƒ"ƒÿ.êiÂ.. */
	$"8097 0000 0A41 93D8 F5F4 F4D9 6199 4782"            /* €—..ÂA“ØõôôÙa™G‚ */
	$"2285 FF12 FCBF 6529 A38C 2965 BFFC 616D"            /* "…ÿ.ü¿e)£Œ)e¿üam */
	$"87C4 D9D9 6199 4781 2292 FF06 616D 87CC"            /* ‡ÄÙÙa™G"’ÿ.am‡Ì */
	$"6199 4780 2295 FF03 6161 4747 8022 82FF"            /* a™G€"•ÿ.aaGG€"‚ÿ */
	$"6C38 6D6B 0000 0408 0000 FFFF FF3C 0000"            /* l8mk......ÿÿÿ<.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 FFFF FFFF FF3C"            /* ..........ÿÿÿÿÿ< */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FF3C 0000 0000 0000 0000 0000 0000 0000"            /* ÿ<.............. */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FF3C 0000 0000 0000 0000 0000 0000"            /* ÿÿÿ<............ */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF3C 00FF FFFF 3C00 0000 0000"            /* ÿÿÿÿÿ<.ÿÿÿ<..... */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 3C00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ<... */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 3C00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ<. */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF3C"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ< */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"3C00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* <.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF4C 2C18 0000 0000 3CFF FFFF FFFF FFFF"            /* ÿL,.....<ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF99 7B4C 2C18 0000 0000 3CFF FFFF FFFF"            /* ÿ™{L,.....<ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFBD AF99 7B4C 2C11 0000 00FF FFFF FFFF"            /* ÿ½¯™{L,....ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFC0 BDB1 A086 632C 0000 00FF FFFF FFFF"            /* ÿÀ½± †c,...ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFC0 C0BD B1A0 893E 0000 0028 FFFF FFFF"            /* ÿÀÀ½± ‰>...(ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFC0 C0B9 AC97 7134 0000 0003 FFFF FFFF"            /* ÿÀÀ¹¬—q4....ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFC0 B9B1 A088 4C18 0000 0000 0CFF FFFF"            /* ÿÀ¹± ˆL......ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFB9 B09F 884C 2300 0000 0000 0011 FFFF"            /* ÿ¹°ŸˆL#.......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFAF 9F88 4C23 0000 0000 0000 0000 0335"            /* ÿ¯ŸˆL#.........5 */
	$"FFFF FFFF FFFF 3F03 3CFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿ?.<ÿÿÿÿÿÿÿ */
	$"FF9F 884C 2300 0000 0000 0000 0000 0000"            /* ÿŸˆL#........... */
	$"0000 0000 0000 0000 0000 3CFF FFFF FFFF"            /* ..........<ÿÿÿÿÿ */
	$"FF88 4C23 0000 0000 0000 0000 0000 0000"            /* ÿˆL#............ */
	$"0000 0000 0000 0000 0000 0000 3CFF FFFF"            /* ............<ÿÿÿ */
	$"9A4C 1800 0000 0000"                                /* šL...... */
};

data 'ics#' (128, "Item Icon") {
	$"0180 0FF0 1818 33CC 67EE 6FE6 EFF7 EFF7"            /* .€.ð..3Ìgîoæï÷ï÷ */
	$"EFE7 E7CF F19E 7C3E 7E7C 3E7C 1E78 0660"            /* ïççÏñž|>~|>|.x.` */
	$"07E0 1FF8 3FFC 7FFE 7FFE FFFF FFFF FFFF"            /* .à.ø?ü.þ.þÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 7FFE 7FFE 3FFC 1FF8 07E0"            /* ÿÿÿÿÿÿ.þ.þ?ü.ø.à */
};

data 'ics#' (129, "Map Icon") {
	$"0890 140C 6E46 DB25 AD02 5B01 A521 2A04"            /* ...nFÛ%­.[.¥!*. */
	$"1421 0009 1281 0104 1011 0109 1084 055B"            /* .!.Æ......Æ.„.[ */
	$"1FF8 3FFC 7FFE FFFF FFFF FFFF FFFF 7FFF"            /* .ø?ü.þÿÿÿÿÿÿÿÿ.ÿ */
	$"1FFF 1FFF 1FFF 1FFF 1FFF 1FFF 1FFF 1FFF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
};

data 'ics#' (130, "Sounds Icon") {
	$"0890 140C 6E02 DB07 AD01 5B08 A511 2A49"            /* ...n.Û.­.[.¥.*I */
	$"1448 0129 12D5 052E 16D5 0099 1000 055B"            /* .H.).Õ...Õ.™...[ */
	$"1FF8 3FFC 7FFE FFFF FFFF FFFF FFFF 7FFF"            /* .ø?ü.þÿÿÿÿÿÿÿÿ.ÿ */
	$"1FFF 1FFF 1FFF 1FFF 1FFF 1FFF 1FFF 1FFF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
};

data 'ics#' (131, "Shapes Icon") {
	$"0890 120C 6DC2 AE47 DD31 BB58 456D 2A39"            /* ...mÂ®GÝ1»XEm*9 */
	$"241C 1019 0069 1046 0085 1041 0004 16B5"            /* $....i.F.….A...µ */
	$"1FF8 3FFC 7FFE FFFF FFFF FFFF FFFF 7FFF"            /* .ø?ü.þÿÿÿÿÿÿÿÿ.ÿ */
	$"1FFF 1FFF 1FFF 1FFF 1FFF 1FFF 1FFF 1FFF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
};

data 'ics#' (132, "Music Icon") {
	$"0890 140C 6E02 DB07 AD01 5B08 A501 2A21"            /* ...n.Û.­.[.¥.*! */
	$"1424 0149 1225 00E4 175D 0309 1001 0ADD"            /* .$.I.%.ä.].Æ..ÂÝ */
	$"1FF8 3FFC 7FFE FFFF FFFF FFFF FFFF 7FFF"            /* .ø?ü.þÿÿÿÿÿÿÿÿ.ÿ */
	$"1FFF 1FFF 1FFF 1FFF 1FFF 1FFF 1FFF 1FFF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
};

data 'ics#' (133, "Saved Game Icon") {
	$"1120 0018 2044 000E 0902 22F0 06D2 256A"            /* . .. D..Æ."ð.Ò%j */
	$"0ED2 25B8 0252 25A2 0042 2004 0102 1A5A"            /* .Ò%¸.R%¢.B ....Z */
	$"3FF8 3FFC 3FFE 3FFE 3FFE 3FFE 3FFE 3FFE"            /* ?ø?ü?þ?þ?þ?þ?þ?þ */
	$"3FFE 3FFE 3FFE 3FFE 3FFE 3FFE 3FFE 3FFE"            /* ?þ?þ?þ?þ?þ?þ?þ?þ */
};

data 'ics#' (134, "Film Icon") {
	$"6AB0 2008 6014 2106 64C6 4BB4 64E6 2BDA"            /* j° .`.!.dÆK´dæ+Ú */
	$"6D66 4554 64A6 1142 620E 2000 6006 577E"            /* mfETd¦.Bb. .`.W~ */
	$"7FFE 7FFE 7FFE 7FFE 7FFE 7FFE 7FFE 7FFE"            /* .þ.þ.þ.þ.þ.þ.þ.þ */
	$"7FFE 7FFE 7FFE 7FFE 7FFE 7FFE 7FFE 7FFE"            /* .þ.þ.þ.þ.þ.þ.þ.þ */
};

data 'ics#' (135, "Physics") {
	$"4494 8000 0004 0002 83C4 46B1 0D62 4AD9"            /* D”€.....ƒÄF±.bJÙ */
	$"0DA1 4372 08A3 4544 8002 0004 8002 5BFC"            /* .¡Cr.£ED€...€.[ü */
	$"FFFE FFFE FFFE FFFE FFFF FFFF 7FFF 7FFF"            /* ÿþÿþÿþÿþÿÿÿÿ.ÿ.ÿ */
	$"7FFF 7FFF FFFF FFFF FFFE FFFE FFFE FFFE"            /* .ÿ.ÿÿÿÿÿÿþÿþÿþÿþ */
};

data 'ics#' (136, "Images Icon") {
	$"1FF8 2BBC 56EA ADFF DFFF BADF 49FD 53BF"            /* .ø+¼Vê­ÿßÿºßIýS¿ */
	$"2FFF 1EFD 0BFF 175D 0BF5 1DB9 0F75 0FFB"            /* /ÿ.ý.ÿ.].õ.¹.u.û */
	$"1FF8 3FFC 7FFE FFFF FFFF FFFF FFFF 7FFF"            /* .ø?ü.þÿÿÿÿÿÿÿÿ.ÿ */
	$"1FFF 1FFF 1FFF 1FFF 1FFF 1FFF 1FFF 1FFF"            /* .ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ.ÿ */
};

data 'ics#' (137, "MIDI Music Icon") {
	$"0890 140C 6E02 DB07 AD01 5B08 A501 2A21"            /* ...n.Û.­.[.¥.*! */
	$"1424 0149 1225 6DE4 AA9D 5569 7B81 0D6D"            /* .$.I.%mäªUi{.m */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'ics#' (138, "MML Script Icon") {
	$"0890 142C 6E12 DB37 AD61 5BC8 A555 2A82"            /* ..,n.Û7­a[È¥U*‚ */
	$"1503 0222 110D 0089 1151 02A0 1141 0AEF"            /* ..."...‰.Q. .AÂï */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'ics#' (139, "Text File Icon") {
	$"0890 140C 6E02 DB07 AD21 5B44 A529 2A81"            /* ...n.Û.­![D¥)* */
	$"1454 0001 1001 0000 1025 0001 1000 0ADB"            /* .T.......%....ÂÛ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'ics#' (140, "Generic File Icon") {
	$"0890 140C 6E02 DB07 AD01 5B08 A501 2A01"            /* ...n.Û.­.[.¥.*. */
	$"1411 0000 1005 0021 1001 0008 1001 056B"            /* .......!.......k */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
};

data 'ics#' (141, "Folder Icon") {
	$"5000 A800 2D80 8A70 1350 A450 0228 9D50"            /* P.¨.-€Šp.P¤P.(P */
	$"57B0 AA98 77D4 2E9F 32AF 055E 12B6 005C"            /* W°ª˜wÔ.Ÿ2¯.^.¶.\ */
	$"F800 FE80 FFE0 FFF0 FFF8 FFF8 FFF8 FFF8"            /* ø.þ€ÿàÿðÿøÿøÿøÿø */
	$"FFF8 FFFC FFFF FFFF 7FFF 3FFF 1FFE 00FC"            /* ÿøÿüÿÿÿÿ.ÿ?ÿ.þ.ü */
};

data 'ics4' (128, "Item Icon") {
	$"0000 CDED DEDC 0000 000E ED8B 8EDA E000"            /* ..ÍíÞÜ....í‹ŽÚà. */
	$"00AF D888 999D FE00 0EFD 8888 A8FE DFE0"            /* .¯Øˆ™þ..ýˆˆ¨þßà */
	$"CFED 88B8 8A9F D9FC D9EB 8B88 A99F DBFD"            /* Ïíˆ¸ŠŸÙüÙë‹ˆ©ŸÛý */
	$"999D 98A8 99AF D99E 8A9D A99A 9AF9 D9A9"            /* ™˜¨™¯ÙžŠ©ššùÙ© */
	$"E89B DA9F FFFD B98A 8A98 DDEA 9ECB 989E"            /* è›ÚŸÿý¹ŠŠ˜ÝêžË˜ž */
	$"D889 88DC CD88 8A9D 098B 988D D8A8 989C"            /* Ø‰ˆÜÍˆŠÆ‹˜Ø¨˜œ */
	$"0B88 8A8D B888 8AB0 00B8 888B C888 A900"            /* .ˆŠ¸ˆŠ°.¸ˆ‹Èˆ©. */
	$"000B 888D 1889 D000 0000 CD8D DEBC 0000"            /* ..ˆ.‰Ð...ÍÞ¼.. */
};

data 'ics4' (129, "Map Icon") {
	$"000C DCCC DCDC DC00 0CDE DDC0 0000 CEC0"            /* ..ÜÌÜÜÜ..ÞÝÀ..ÎÀ */
	$"CE88 8AEC 0DCC DCEC EB88 A9EE 0CD0 CEDE"            /* ÎˆŠì.ÌÜìëˆ©î.ÐÎÞ */
	$"9EA8 F9E9 00C0 CCCD 98DA EE89 0C0C 000D"            /* ž¨ùé.ÀÌÍ˜Úî‰.... */
	$"B88D D89B 0070 C0CD 088B 8890 C00C 0C0E"            /* ¸Ø›.pÀÍ.‹ˆÀ... */
	$"0C8D BD00 00BD C0CD 000D 0C00 07CC 00CD"            /* .½..½ÀÍ.....Ì.Í */
	$"000C C0CD C007 0C0E 000D 0002 C00C 0C0E"            /* ..ÀÍÀ.......À... */
	$"000C C000 0C00 70CD 000C C00D C0C0 BC0E"            /* ..À...pÍ..À.ÀÀ¼. */
	$"000D 00CD 0007 CC0D 000C DDDD DDDD DDDE"            /* ...Í..Ì...ÝÝÝÝÝÞ */
};

data 'ics4' (130, "Sounds Icon") {
	$"000C DCCC DCDC DC00 0CDE DDC0 0000 DEC0"            /* ..ÜÌÜÜÜ..ÞÝÀ..ÞÀ */
	$"CE88 8AEC 0000 DCEC EB88 A9EE 00C0 CEDE"            /* ÎˆŠì..Üìëˆ©î.ÀÎÞ */
	$"9EA8 FFE8 C00C 0CCD 98DA 9D8A 00C0 C00D"            /* ž¨ÿèÀ..Í˜ÚŠ.ÀÀ. */
	$"B88D D89E 0C0C DC0E 088B 8880 0E0C D0CD"            /* ¸Øž..Ü..‹ˆ€..ÐÍ */
	$"0CB8 DBC0 CE0C E00D 000C 00CD 0ECD DC0E"            /* .¸ÛÀÎ.à....Í.ÍÜ. */
	$"000D 0DDD CEED DECD 000C CEDD DDED EEDD"            /* ...ÝÎíÞÍ..ÎÝÝíîÝ */
	$"000D CDDC EDDE DDCE 000C 000C ECCE DD0E"            /* ..ÍÜíÞÝÎ....ìÎÝ. */
	$"000D 0000 C00D 000E 000C DDDD DDDD DDDE"            /* ....À.....ÝÝÝÝÝÞ */
};

data 'ics4' (131, "Shapes Icon") {
	$"000C DCCC CDCD DC00 0CDE DDC0 D000 CEC0"            /* ..ÜÌÍÍÜ..ÞÝÀÐ.ÎÀ */
	$"CE88 8AEC DDC0 DCEC EB88 A99E CE0C CEDE"            /* ÎˆŠìÝÀÜìëˆ©žÎ.ÎÞ */
	$"9EA8 FAE9 0DDE CCCD 98D9 EE89 0C9E DC0D"            /* ž¨úé.ÞÌÍ˜Ùî‰.žÜ. */
	$"B89D D89D C9FE 9DCD 088B 88BC 0CDF 9D0E"            /* ¸ØÉþÍ.‹ˆ¼.ß. */
	$"0CB8 C8C0 00CE FD0D 000C C000 00D9 EC0E"            /* .¸ÈÀ.Îý...À..Ùì. */
	$"000C 0000 CDEE 9D0D 000D C000 CE0C CE0E"            /* ....Íî...À.Î.Î. */
	$"000C 0000 DC00 CDCD 000D C000 CD0C 0D0E"            /* ....Ü.ÍÍ..À.Í... */
	$"000C 0000 0000 0CCD 000D DDDD DDDD DDDE"            /* .......Í..ÝÝÝÝÝÞ */
};

data 'ics4' (132, "Music Icon") {
	$"000C DCCC DCDC DC00 0CDE DDC0 0000 DEC0"            /* ..ÜÌÜÜÜ..ÞÝÀ..ÞÀ */
	$"CE88 8AEC 0000 DCEC EB88 A9EE 00C0 CEDE"            /* ÎˆŠì..Üìëˆ©î.ÀÎÞ */
	$"9EA8 FFE8 C00C 0CCD 98DA 9D8A 0000 0C0D"            /* ž¨ÿèÀ..Í˜ÚŠ.... */
	$"B88D D89E 0C0C 00CD 088B 8880 00D0 CC0E"            /* ¸Øž...Í.‹ˆ€.ÐÌ. */
	$"0CB8 DBCC 0CD0 CD0D 000C 000D 0CD0 CD0D"            /* .¸ÛÌ.ÐÍ......ÐÍ. */
	$"000D CCDD CCDC CDCE 000D 0C0D CFE0 DDCD"            /* ..ÌÝÌÜÍÎ....ÏàÝÍ */
	$"000C CCEE CFCD FECE 000C CDFD CCCD ADCD"            /* ..ÌîÏÍþÎ..ÍýÌÍ­Í */
	$"000D 00D0 0000 C00E 000C DDCD DDDD DDDE"            /* ...Ð..À...ÝÍÝÝÝÞ */
};

data 'ics4' (133, "Saved Game Icon") {
	$"00CD CCCD CDCD C000 00C0 0000 000C EC00"            /* .ÍÌÍÍÍÀ..À....ì. */
	$"00C0 CCCC 0CCD CEC0 00CC 0000 0C0C DDE0"            /* .ÀÌÌ.ÍÎÀ.Ì....Ýà */
	$"00C0 C0CC CCC0 CCD0 00CC 0CE8 99EC 0CD0"            /* .ÀÀÌÌÀÌÐ.Ì.è™ì.Ð */
	$"00C0 0AE8 8AE9 C0E0 00D0 D9B8 A9EA D0D0"            /* .ÀÂèŠéÀà.ÐÙ¸©êÐÐ */
	$"00C0 D9E9 9FE8 D0E0 00D0 C88B DB89 DCD0"            /* .ÀÙéŸèÐà.ÐÈ‹Û‰ÜÐ */
	$"00CC 0888 D889 00E0 00D0 0CB8 B89C 0CD0"            /* .Ì.ˆØ‰.à.Ð.¸¸œ.Ð */
	$"00CC 000D CC0C 0CE0 00D0 C0C0 00C0 0CD0"            /* .Ì..Ì..à.ÐÀÀ.À.Ð */
	$"00C0 000C 0C00 00E0 00DD CDDD DDDD DDE0"            /* .À.....à.ÝÍÝÝÝÝà */
};

data 'ics4' (134, "Film Icon") {
	$"0FAD DDDD DDDE C000 0DDC 0000 000D EC00"            /* .­ÝÝÝÞÀ..Ü....ì. */
	$"0FEC 0C0C 0C0D CEC0 CDD0 C0C0 C00C DEE0"            /* .ì....ÎÀÍÐÀÀÀ.Þà */
	$"0EFC 0CDE DEC0 CEAC 0DDC CE88 89EC CDD0"            /* .ü.ÞÞÀÎ¬.ÜÎˆ‰ìÍÐ */
	$"0FE0 EE8B 9AEE 0FE0 CDDC 8EA8 F9B9 CDDC"            /* .àî‹šî.àÍÜŽ¨ù¹ÍÜ */
	$"0AFC E8BF EE8E CEF0 0DD0 B88D D89D CDD0"            /* Âüè¿îŽÎð.Ð¸ØÍÐ */
	$"0FEC C888 B88C 0FEC CDDC 0C8C 8BC0 CDD0"            /* .ìÈˆ¸Œ.ìÍÜ.Œ‹ÀÍÐ */
	$"0FEC 000C 000C 0FA0 0DD0 C0C0 C0C0 CDDC"            /* .ì..... .ÐÀÀÀÀÍÜ */
	$"0FEC 0000 0000 0AE0 CDED DDDD DDDD DEE0"            /* .ì....ÂàÍíÝÝÝÝÞà */
};

data 'ics4' (135, "Physics") {
	$"DCCC CCDC CDCD CDC0 D000 0000 0000 0CD0"            /* ÜÌÌÜÍÍÍÀÐ......Ð */
	$"C000 0000 C0C0 CCD0 C000 000C 000C 0DC0"            /* À...ÀÀÌÐÀ......À */
	$"DCC0 0CDD 9DC0 0CDC CC00 CA88 99AC 0DDD"            /* ÜÀ.ÝÀ.ÜÌ.Êˆ™¬.Ý */
	$"0CC0 EE88 AEEE 0C0E 0D00 8EA9 9FE9 C0CD"            /* .Àîˆ®î....Ž©ŸéÀÍ */
	$"0C00 98DA EB8E 0C0E 0CC0 B98D D88B C0CD"            /* ..˜ÚëŽ...À¹Ø‹ÀÍ */
	$"CC00 C888 889C 0CEE DD00 00DB DBC0 0DDC"            /* Ì.Èˆˆœ.îÝ..ÛÛÀ.Ü */
	$"C000 0000 000C 0CD0 D000 0000 C0C0 0DD0"            /* À......ÐÐ...ÀÀ.Ð */
	$"D000 00C0 000C 0CD0 DDDE DEDE DEED EEC0"            /* Ð..À...ÐÝÞÞÞÞíîÀ */
};

data 'ics4' (136, "Images Icon") {
	$"000D FEFF EFEF FC00 0CDE DEE9 EEFE EDC0"            /* ..þÿïïü..ÞÞéîþíÀ */
	$"CE88 8EE9 FE69 9CEC E8B8 99EF E9F6 EFEF"            /* ÎˆŽéþiœìè¸™ïéöïï */
	$"9E8A AFE9 F5EF 5F5E 8EEA 9D8E FFF5 6F6F"            /* žŠ¯éõï_^ŽêŽÿõoo */
	$"D88D D88F 9FEF E5EF C88B 88A9 A9FF 6F6E"            /* ØØŸïåïÈ‹ˆ©©ÿon */
	$"0CB8 D99F F9FF F5E6 000D FF9F 9AFF F69E"            /* .¸ÙŸùÿõæ..ÿŸšÿöž */
	$"000D F8F9 FF9F AF69 000D 999F 99FF FE7E"            /* ..øùÿŸ¯i..™Ÿ™ÿþ~ */
	$"000D 9999 FE9F E785 000D 9999 F9FF 7879"            /* ..™™þŸç…..™™ùÿxy */
	$"000D F9FA 99F5 97DF 000D 9FAF FEF9 5EEE"            /* ..ùú™õ—ß..Ÿ¯þù^î */
};

data 'ics4' (137, "MIDI Music Icon") {
	$"000C DCCC DCDC DC00 0CDE DDC0 0000 DEC0"            /* ..ÜÌÜÜÜ..ÞÝÀ..ÞÀ */
	$"CE88 8AEC 0000 DCEC EB88 A9EE 00C0 CEDE"            /* ÎˆŠì..Üìëˆ©î.ÀÎÞ */
	$"9EA8 FFE8 C00C 0CCD 98DA 9D8A 0000 0C0D"            /* ž¨ÿèÀ..Í˜ÚŠ.... */
	$"B88D D89E 0C0C 00CD 088B 8880 00D0 CC0E"            /* ¸Øž...Í.‹ˆ€.ÐÌ. */
	$"0CB8 DBCC 0CD0 CD0D 000C 000D 0CD0 CD0E"            /* .¸ÛÌ.ÐÍ......ÐÍ. */
	$"00CD CCDD CDDC CDCD D3BB BEBB AEE0 CE0E"            /* .ÍÌÝÍÜÍÍÓ»¾»®àÎ. */
	$"DC44 D4D4 D3CD FDCD C42C 2042 2DCD FDCE"            /* ÜDÔÔÓÍýÍÄ, B-ÍýÎ */
	$"DBE3 D3DD 4B00 000E 000D DDDD DDDD DDDE"            /* ÛãÓÝK.....ÝÝÝÝÝÞ */
};

data 'ics4' (138, "MML Script Icon") {
	$"000C DCCC CDCC DC00 0CDE DDC0 0CCC CEC0"            /* ..ÜÌÍÌÜ..ÞÝÀ.ÌÎÀ */
	$"CE88 8AED CCDD DCEC EB88 9FEE 0CDE DEDE"            /* ÎˆŠíÌÝÜìëˆŸî.ÞÞÞ */
	$"9EA8 A9E9 CDED 0CCD 98DA EE89 DEDD D0CD"            /* ž¨©éÍí.Í˜Úî‰ÞÝÐÍ */
	$"B88D D8AE EDC0 CD0D 088B 888D DCCC 0DDD"            /* ¸Ø®íÀÍ..‹ˆÜÌ.Ý */
	$"0C8D B8DC CC00 0CDE 000D 00DD C00C 0CED"            /* .¸ÜÌ..Þ...ÝÀ..í */
	$"000C C00D C000 CECE 000D 000C E00C EDCD"            /* ..À.À.ÎÎ....à.íÍ */
	$"000C C0CE DDCD D00E 000D 00DD CDDD C0CD"            /* ..ÀÎÝÍÐ....ÝÍÝÀÍ */
	$"000C C00D DEC0 000E 000D DDDD EEDD DDDE"            /* ..À.ÞÀ....ÝÝîÝÝÞ */
};

data 'ics4' (139, "Text File Icon") {
	$"000C DCCC DCDC DC00 0CDE DDC0 0000 DEC0"            /* ..ÜÌÜÜÜ..ÞÝÀ..ÞÀ */
	$"CE88 8AEC 0000 DCEC EB88 A9EE 00C0 CEDE"            /* ÎˆŠì..Üìëˆ©î.ÀÎÞ */
	$"9EA8 F9E9 C0CC 0CCD 98DA EE89 CCCC CCCD"            /* ž¨ùéÀÌ.Í˜Úî‰ÌÌÌÍ */
	$"B88D D8AB CCCD CD0E 088B 888C CCCC CCCD"            /* ¸Ø«ÌÍÍ..‹ˆŒÌÌÌÍ */
	$"0C8D BDCC DCCC CC0D 000D 0000 0000 0C0E"            /* .½ÌÜÌÌ......... */
	$"000C C000 0C0C 0C0E 000D 000C 0000 C0CD"            /* ..À...........ÀÍ */
	$"000C C000 0C00 0C0E 000D 0000 00C0 C0CD"            /* ..À..........ÀÀÍ */
	$"000C C000 0000 000E 000D DDDD DDDD DDDE"            /* ..À.......ÝÝÝÝÝÞ */
};

data 'ics4' (140, "Generic File Icon") {
	$"000C DCCC DCDC DC00 0CDE DDC0 0000 DEC0"            /* ..ÜÌÜÜÜ..ÞÝÀ..ÞÀ */
	$"CE88 8AEC 0000 DCEC EB88 A9EE 00C0 CEDE"            /* ÎˆŠì..Üìëˆ©î.ÀÎÞ */
	$"9EA8 FFE8 C00C 0CCD 98DA 9D8A 00C0 0C0D"            /* ž¨ÿèÀ..Í˜ÚŠ.À.. */
	$"B88D D89E 000C 00CD 088B 8880 0C00 C0CD"            /* ¸Øž...Í.‹ˆ€..ÀÍ */
	$"0CB8 DBC0 00C0 0C0D 000C 0000 000C 0C0E"            /* .¸ÛÀ.À.......... */
	$"000D 000C 00C0 C0CD 000C C000 000C 000E"            /* .....ÀÀÍ..À..... */
	$"000D 0000 C000 CC0D 000C C000 0C0C 000E"            /* ....À.Ì...À..... */
	$"000D 0000 0000 0C0D 000C DDDD DDDD DDDE"            /* ..........ÝÝÝÝÝÞ */
};

data 'ics4' (141, "Folder Icon") {
	$"0D5C 0000 0000 0000 DD7D 5D00 0000 0000"            /* .\......Ý}]..... */
	$"CCC7 C5DD 5DC0 0000 DC00 DC57 C75D 0000"            /* ÌÇÅÝ]À..Ü.ÜWÇ].. */
	$"CCCC CCCD 5CD7 C000 DCCC CCCC C5C5 D000"            /* ÌÌÌÍ\×À.ÜÌÌÌÅÅÐ. */
	$"D0CC CCCC CC7D C000 DCCD DED5 CCC5 D000"            /* ÐÌÌÌÌ}À.ÜÍÞÕÌÅÐ. */
	$"DC5E 889E DCD5 D000 DCE8 8A9E A7C5 D000"            /* Ü^ˆžÜÕÐ.ÜèŠž§ÅÐ. */
	$"D58B 98FE EDD5 EED0 0D9E EAEE 85C5 EFFE"            /* Õ‹˜þíÕîÐ.žêî…Åïþ */
	$"0C88 8D88 9C7D FEEF 00B8 8D88 DCC5 EFFD"            /* .ˆˆœ}þï.¸ˆÜÅïý */
	$"000C 8CBD 5D75 EFD0 0000 0000 0C5D FD00"            /* ..Œ½]uïÐ.....]ý. */
};

data 'ics8' (128, "Item Icon") {
	$"0000 0000 F781 81FA 8181 FAF7 0000 0000"            /* ....÷úú÷.... */
	$"0000 00FB FD57 7BA1 A67C FAFD FCF5 0000"            /* ...ûýW{¡¦|úýüõ.. */
	$"00F5 FDFD 50A1 9BA1 A7E7 AD7A FEFC F500"            /* .õýýP¡›¡§ç­zþüõ. */
	$"00FB FFF9 A19B 9BA1 A1A7 E7AD F9FF FB00"            /* .ûÿù¡››¡¡§ç­ùÿû. */
	$"2BE9 FD51 A1A1 9BA1 A1CB ADE9 FAAD D12B"            /* +éýQ¡¡›¡¡Ë­éú­Ñ+ */
	$"7BE9 A67B A7A1 A1A1 A7A7 E8E9 81A6 E981"            /* {é¦{§¡¡¡§§èé¦é */
	$"A6E8 A657 E7A7 A7A7 CBE8 ADFF F9A7 E8A6"            /* ¦è¦Wç§§§Ëè­ÿù§è¦ */
	$"A7CB AD56 E8A7 CBA7 E8AD EAFD 56A7 CBAD"            /* §Ë­Vè§Ë§è­êýV§Ë­ */
	$"A6A7 CB7C 56EA D1E9 D1FF EA56 A0E7 A7FD"            /* ¦§Ë|VêÑéÑÿêV ç§ý */
	$"A0A1 A7E7 7B56 FCFD FEFC F87C A7A1 A7A6"            /*  ¡§ç{Vüýþüø|§¡§¦ */
	$"57E5 A1A1 A7A0 57F7 F775 A1A7 A1A7 E681"            /* Wå¡¡§ W÷÷u¡§¡§æ */
	$"2BA1 A1A1 A1A1 E57A 51A1 A1A1 A1A1 D12B"            /* +¡¡¡¡¡åzQ¡¡¡¡¡Ñ+ */
	$"007B A1A1 A1A1 A157 50C5 A19B A1CB FA00"            /* .{¡¡¡¡¡WPÅ¡›¡Ëú. */
	$"0000 7CA1 9B9B A1F9 4BA1 9BA1 E6FB 0100"            /* ..|¡››¡ùK¡›¡æû.. */
	$"0000 F57B A1A1 E457 519B A7A7 81F5 0000"            /* ..õ{¡¡äWQ›§§õ.. */
	$"0000 0000 F751 A6F9 56A6 7B2B 0000 0000"            /* ....÷Q¦ùV¦{+.... */
};

data 'ics8' (129, "Map Icon") {
	$"0000 00F7 56F8 F7F8 F8F8 F8F8 FAF7 0000"            /* ...÷Vø÷øøøøøú÷.. */
	$"002B 807C 81FA F600 0000 4800 56FB 2B00"            /* .+€|úö...H.Vû+. */
	$"2BAC 77A1 E5AC AC2B 0032 3248 F9F7 FBF7"            /* +¬w¡å¬¬+.22Hù÷û÷ */
	$"FBA6 A19B A7AD A6A6 000E F9F5 F7FB FBFC"            /* û¦¡›§­¦¦..ùõ÷ûûü */
	$"A7FB A6E7 E8FE A6A7 0624 4807 F5F7 2B81"            /* §û¦çèþ¦§.$H.õ÷+ */
	$"A6A1 FBAD AC81 A1AC 24F5 4F00 F6F5 F5FA"            /* ¦¡û­¬¡¬$õO.öõõú */
	$"75A7 A17A 75A1 A77C 00F5 48F6 F6F5 F681"            /* u§¡zu¡§|.õHööõö */
	$"F5A1 E37C 76A1 A12B F500 2B2B F5F6 F581"            /* õ¡ã|v¡¡+õ.++õöõ */
	$"00F6 7B7C 7B7B 2B00 0000 3939 484F 2580"            /* .ö{|{{+...99HO%€ */
	$"0000 00F8 F624 0000 254E 4F4F 252A F681"            /* ...øö$..%NOO%*ö */
	$"0000 00F8 F524 7315 4E49 004E F6F6 F5FB"            /* ...øõ$s.NI.Nööõû */
	$"0000 0056 F600 F539 F500 0625 4FF5 F581"            /* ...Vö.õ9õ..%Oõõ */
	$"0000 00F8 F600 0048 2401 F52A 6DF5 F6FB"            /* ...øö..H$.õ*mõöû */
	$"0000 0056 F600 0039 F6F6 24F6 392B F681"            /* ...Vö..9öö$ö9+ö */
	$"0000 00F8 F524 2439 4848 2A49 3849 2481"            /* ...øõ$$9HH*I8I$ */
	$"0000 0056 F9F9 7B56 5C57 FAF9 F9FA 5DFC"            /* ...Vùù{V\Wúùùú]ü */
};

data 'ics8' (130, "Sounds Icon") {
	$"0000 00F7 56F8 F7F8 F8F8 F8F8 FAF7 0000"            /* ...÷Vø÷øøøøøú÷.. */
	$"002B FA7C 81F9 F600 0000 0000 56FB 2B00"            /* .+ú|ùö.....Vû+. */
	$"2BAC A0A1 A1D1 AC2B 00F5 F5F5 56F7 FBF7"            /* +¬ ¡¡Ñ¬+.õõõV÷û÷ */
	$"A582 A19B CBAD A6FC 00F5 F500 F8FB FBFC"            /* ¥‚¡›Ë­¦ü.õõ.øûûü */
	$"A7A5 A7A7 D1FD A6A7 F5F5 F5F5 F5F7 2B81"            /* §¥§§Ñý¦§õõõõõ÷+ */
	$"A6A7 7BFD FD81 A7A6 F600 F5F6 F7F5 F5FA"            /* ¦§{ýý§¦ö.õö÷õõú */
	$"75A1 A775 51A1 E581 002B F52B FA00 F681"            /* u¡§uQ¡å.+õ+ú.ö */
	$"F6A1 E47C 76A1 A02B F5FB F5F7 FAF5 F581"            /* ö¡ä|v¡ +õûõ÷úõõ */
	$"00F6 7581 7BA0 0700 F6AC 00F7 FBF5 F580"            /* .öu{ ..ö¬.÷ûõõ€ */
	$"0000 0056 F600 4FF9 F6AC F856 81F6 F581"            /* ...Vö.Oùö¬øVöõ */
	$"0000 00F8 F5F8 FA81 2BFB FCF9 81FB 2B81"            /* ...øõøú+ûüùû+ */
	$"0000 00F8 F7FD FAF9 5681 FB81 FBFC 5681"            /* ...ø÷ýúùVûûüV */
	$"0000 00F8 2BF9 FAF7 FB56 FBFC 81FA F8FB"            /* ...ø+ùú÷ûVûüúøû */
	$"0000 0056 F500 F6F6 AC2B F8FB FAF9 F5FB"            /* ...Võ.öö¬+øûúùõû */
	$"0000 00F8 F500 0000 F800 00F8 F5F5 00FB"            /* ...øõ...ø..øõõ.û */
	$"0000 0056 FA56 F9F9 56F9 FAF9 FAFA FAFC"            /* ...VúVùùVùúùúúúü */
};

data 'ics8' (131, "Shapes Icon") {
	$"0000 00F7 56F8 F7F8 F8F8 F8F8 FAF7 0000"            /* ...÷Vø÷øøøøøú÷.. */
	$"002B FA7C 81F9 2B00 4FF5 0000 56FB 2B00"            /* .+ú|ù+.Oõ..Vû+. */
	$"2BAC A09B A7FD ACF6 A581 0000 56F8 FBF7"            /* +¬ ›§ý¬ö¥..Vøû÷ */
	$"A582 A1A1 E5AD A6A6 31CA F52B F8FB 81FC"            /* ¥‚¡¡å­¦¦1Êõ+øûü */
	$"A7A5 A7A7 D1FD 82A7 00F8 FAFB 322B 2B81"            /* §¥§§Ñý‚§.øúû2++ */
	$"A0A7 7BFD FD81 E6A6 F5F7 FDFD 8125 F5FA"            /*  §{ýýæ¦õ÷ýý%õú */
	$"7CA1 A17B 51A1 A1FB 25E9 E981 CAFB F581"            /* |¡¡{Q¡¡û%ééÊûõ */
	$"F6A0 A17B 76E4 A7F6 F5F7 F9E9 D1FA F581"            /* ö ¡{vä§öõ÷ùéÑúõ */
	$"0025 7C7B 7B7B 2B00 0000 F6FD EA50 F580"            /* .%|{{{+...öýêPõ€ */
	$"0000 00F8 F600 00F5 00F5 56D0 ACF6 F581"            /* ...øö..õ.õVÐ¬öõ */
	$"0000 00F8 F500 0000 F581 FCFD A6FA 06FB"            /* ...øõ...õüý¦ú.û */
	$"0000 0056 F600 0000 56FB F6F6 F8FC 2581"            /* ...Vö...Vûööøü% */
	$"0000 00F8 F600 0000 F82C F5F5 F681 F681"            /* ...øö...ø,õõöö */
	$"0000 0056 F500 F500 2B56 F5F5 F6F8 F6FB"            /* ...Võ.õ.+Võõöøöû */
	$"0000 0056 F500 0000 F6F5 0000 0032 F681"            /* ...Võ...öõ...2ö */
	$"0000 00F8 FA56 F9F9 F9F9 FAFA FA7B FAFC"            /* ...øúVùùùùúúú{úü */
};

data 'ics8' (132, "Music Icon") {
	$"0000 00F7 56F8 F7F8 F8F8 F8F8 FAF7 0000"            /* ...÷Vø÷øøøøøú÷.. */
	$"002B FA7C 81F9 F600 0000 0000 56FB 2B00"            /* .+ú|ùö.....Vû+. */
	$"2BAC A0A1 A1D1 AC2B 00F5 F5F5 56F7 FBF7"            /* +¬ ¡¡Ñ¬+.õõõV÷û÷ */
	$"A582 A19B CBAD A6FC 00F5 F500 F8FB FBFC"            /* ¥‚¡›Ë­¦ü.õõ.øûûü */
	$"A7A5 A7A7 D1FD A6A7 F5F5 F5F6 F5F7 2B81"            /* §¥§§Ñý¦§õõõöõ÷+ */
	$"A6A7 7BFD FD81 A7A6 F5F5 F5F5 F6F5 F5FA"            /* ¦§{ýý§¦õõõõöõõú */
	$"75A1 A775 51A1 E582 00F5 F5F5 F5F6 F581"            /* u¡§uQ¡å‚.õõõõöõ */
	$"25A1 A17B 76C5 A12A 002B F9F5 F6F6 F581"            /* %¡¡{vÅ¡*.+ùõööõ */
	$"00F6 76FB 7B7B 2B2B 002B F900 2BFA 0080"            /* .övû{{++.+ù.+ú.€ */
	$"0000 0056 F500 F5F9 00F6 F900 2BF9 F581"            /* ...Võ.õù.öù.+ùõ */
	$"0000 00F8 2B2B 5681 2B56 812B 5681 2BFB"            /* ...ø++V+V+V+û */
	$"0000 00F8 2BF6 F6F9 F8FF FBF5 F8FA F681"            /* ...ø+ööùøÿûõøúö */
	$"0000 00F8 2BF7 ACFA 56FD F8FA FFFB F6FB"            /* ...ø+÷¬úVýøúÿûöû */
	$"0000 0056 F6FA FF81 F6F7 2BF9 FEF9 2BFB"            /* ...Vöúÿö÷+ùþù+û */
	$"0000 00F8 F5F5 5600 F500 F5F5 F5F5 F581"            /* ...øõõV.õ.õõõõõ */
	$"0000 0056 F9F9 F8F9 F9FA F9FA F9FA FAFC"            /* ...Vùùøùùúùúùúúü */
};

data 'ics8' (133, "Saved Game Icon") {
	$"0000 F756 F7F8 F8F8 F8F8 F8FA F700 0000"            /* ..÷V÷øøøøøøú÷... */
	$"0000 F700 0000 00F5 00F5 0056 FB2B 0000"            /* ..÷....õ.õ.Vû+.. */
	$"0000 F8F5 F5F6 F62B F62B F556 F8FB F700"            /* ..øõõöö+ö+õVøû÷. */
	$"0000 F7F5 F500 00F5 00F5 F5F8 8181 FC00"            /* ..÷õõ..õ.õõøü. */
	$"0000 F8F5 F600 F6F8 F8F8 2B00 F6F7 8100"            /* ..øõö.öøøø+.ö÷. */
	$"0000 F7F6 0031 A6A0 A7A6 FCF8 00F6 FA00"            /* ..÷ö.1¦ §¦üø.öú. */
	$"0000 F8F5 F5AD 7C9B A1E8 FCAC 2BF5 8100"            /* ..øõõ­|›¡èü¬+õ. */
	$"0000 F800 50D0 A6A1 A7E8 FCE7 56F5 8100"            /* ..ø.PÐ¦¡§èüçVõ. */
	$"0000 F800 57A7 FBE7 D1FE 82A7 7B00 8100"            /* ..ø.W§ûçÑþ‚§{.. */
	$"0000 F8F5 2CE6 A1FA 817B A0CB 56F5 8100"            /* ..øõ,æ¡ú{ ËVõ. */
	$"0000 F8F5 F5A0 A1A1 51A1 A1A6 F6F5 8100"            /* ..øõõ ¡¡Q¡¡¦öõ. */
	$"0000 F8F6 0001 A0A1 7BA1 A0F7 F5F6 8100"            /* ..øö.. ¡{¡ ÷õö. */
	$"0000 F8F5 0000 F550 2B50 F6F5 F5F6 FB00"            /* ..øõ..õP+Pöõõöû. */
	$"0000 F8F6 F6F5 F5F5 F6F6 F5F5 F6F6 8100"            /* ..øööõõõööõõöö. */
	$"0000 F8F5 0000 00F6 00F5 0600 F500 FB00"            /* ..øõ...ö.õ..õ.û. */
	$"0000 56F9 56F9 56F9 F9FA 7BFA FAFA FC00"            /* ..VùVùVùùú{úúúü. */
};

data 'ics8' (134, "Film Icon") {
	$"00FE FDFA FAFA F9FA FAFA F9FC F800 0000"            /* .þýúúúùúúúùüø... */
	$"F6F9 F9F6 0000 F500 0000 0056 FBF7 0000"            /* öùùö..õ....Vû÷.. */
	$"F5FD FDF6 F5F6 F5F6 F5F5 F5FA F7FB F800"            /* õýýöõöõöõõõú÷ûø. */
	$"F5F9 FA2B F5F5 F6F5 F6F6 00F8 FAFB FCF5"            /* õùú+õõöõöö.øúûüõ */
	$"F5FD FD2B 00F7 817B 82FA F700 F6FD FDF5"            /* õýý+.÷{‚ú÷.öýýõ */
	$"F6F9 F9F6 F7AC 9AA1 E5FD ACF7 2BF9 F9F5"            /* öùùö÷¬š¡åý¬÷+ùùõ */
	$"F5FE FDF6 82A6 7DA1 A7E7 FC82 2AFD FE00"            /* õþýö‚¦}¡§çü‚*ýþ. */
	$"F5F9 F9F7 A6FB E7A7 E7FD 82CA 2CF9 F9F6"            /* õùù÷¦ûç§çý‚Ê,ùùö */
	$"F5FD FDF6 A7A1 FAD0 AD81 A1A6 2BFD FDF5"            /* õýýö§¡úÐ­¡¦+ýýõ */
	$"F5F9 F92B 75E5 A157 7BA1 E682 2BF9 F9F5"            /* õùù+uå¡W{¡æ‚+ùùõ */
	$"F5FE FDF6 F6A1 A1A0 75A1 A02B F6FE FDF5"            /* õþýöö¡¡ u¡ +öþýõ */
	$"F5F9 F92B F52B 757B 7B7C F700 2BF9 FAF5"            /* õùù+õ+u{{|÷.+ùúõ */
	$"F5FE FD2B F5F5 07F5 F6F5 F5F6 F6FD FDF5"            /* õþý+õõ.õöõõööýýõ */
	$"F5F9 F92B F5F5 F5F5 F5F6 F5F5 2BF9 F9F5"            /* õùù+õõõõõöõõ+ùùõ */
	$"F5FD FDF5 00F5 00F5 0000 F500 F6FD FDF5"            /* õýýõ.õ.õ..õ.öýýõ */
	$"F5FB FB81 FAFA FAFA FAFA FAFA FAFC FBF5"            /* õûûúúúúúúúúúüûõ */
};

data 'ics8' (135, "Physics") {
	$"5656 F7F8 F7F8 F8F8 F8F8 F8F8 56FA 2B00"            /* VV÷ø÷øøøøøøøVú+. */
	$"5600 0000 0000 0000 0000 00F5 00F8 5600"            /* V..........õ.øV. */
	$"F700 0000 0000 00F5 F5F5 F5F6 F5F8 5600"            /* ÷......õõõõöõøV. */
	$"F800 0000 0000 F5F5 F5F5 F5F5 F5F8 5600"            /* ø.....õõõõõõõøV. */
	$"F9F7 F500 00F7 FA82 A5FA F7F5 F5F8 FAF6"            /* ù÷õ..÷ú‚¥ú÷õõøúö */
	$"F6F8 F500 2BAC A09B A1AD ACF7 00F8 FAFA"            /* öøõ.+¬ ›¡­¬÷.øúú */
	$"00F8 F500 A682 A1A1 A7E8 FCA6 F5F6 F581"            /* .øõ.¦‚¡¡§èü¦õöõ */
	$"00F8 F500 A7A5 A7CB E8FD A6AD F6F5 F581"            /* .øõ.§¥§Ëèý¦­öõõ */
	$"00F8 F500 A7A7 7BFD AC81 A1A6 F6F6 F581"            /* .øõ.§§{ý¬¡¦ööõ */
	$"00F8 F500 7BE5 A057 7BA1 E582 F5F6 F681"            /* .øõ.{å W{¡å‚õöö */
	$"2BF8 F600 257D E47C 76A1 A62B F5F8 FCFB"            /* +øö.%}ä|v¡¦+õøüû */
	$"FAF8 0000 F524 7C7B 757B F7F5 F5F7 812B"            /* úø..õ$|{u{÷õõ÷+ */
	$"F800 0000 0000 00F5 F6F5 00F5 F556 5600"            /* ø......õöõ.õõVV. */
	$"5600 0000 00F5 00F5 00F5 F5F6 F5F8 F900"            /* V....õ.õ.õõöõøù. */
	$"F900 0000 0000 F5F5 F5F5 F5F5 F5F8 FA00"            /* ù.....õõõõõõõøú. */
	$"F981 FA81 8181 8181 8181 81FB 81FC F700"            /* ùúûü÷. */
};

data 'ics8' (136, "Images Icon") {
	$"0000 00F9 FEFD FDD0 F3FD D6FD FEF7 0000"            /* ...ùþýýÐóýÖýþ÷.. */
	$"002B FA7C FB81 D0E8 FDCF ACCF FD81 F700"            /* .+ú|ûÐèýÏ¬Ïý÷. */
	$"2BAC A09B A7A6 ACFD E7AC CFAC CA2B 81F8"            /* +¬ ›§¦¬ýç¬Ï¬Ê+ø */
	$"FBA6 A1A1 A7D1 FBE7 FECF B2CF D0B2 FDFE"            /* û¦¡¡§ÑûçþÏ²ÏÐ²ýþ */
	$"A7FB A1E7 E8AD 82D0 E9B2 CFB1 B1F1 ABD0"            /* §û¡çè­‚Ðé²Ï±±ñ«Ð */
	$"A6A1 FBAC FD81 A6A7 F4D0 B1D5 B1F1 F0B2"            /* ¦¡û¬ý¦§ôÐ±Õ±ñð² */
	$"7BC5 A157 75A1 A1D1 E0D1 D0D6 CFB1 CFAB"            /* {Å¡Wu¡¡ÑàÑÐÖÏ±Ï« */
	$"01A1 A17B 769B A7FE E8AD FEEA B2F0 ABCF"            /* .¡¡{v›§þè­þê²ð«Ï */
	$"00F6 769F 82CB D0E9 ADE8 EAF3 F4B1 CFAB"            /* .övŸ‚ËÐé­èêóô±Ï« */
	$"0000 00FA EAFE E8D1 E8FE EAF4 E0CF AAD0"            /* ...úêþèÑèþêôàÏªÐ */
	$"0000 00F9 FFCA E8FD D1E9 EAFE F3B1 C9FD"            /* ...ùÿÊèýÑéêþó±Éý */
	$"0000 00FA E7C4 E7E9 E8D0 D1E9 F3C9 A4CF"            /* ...úçÄçéèÐÑéóÉ¤Ï */
	$"0000 00F9 E7CB CAAD E9E7 FDE0 FCC2 98FD"            /* ...ùçËÊ­éçýàüÂ˜ý */
	$"0000 00F9 E7CA E7E7 FEE8 E9FE C898 9FCF"            /* ...ùçÊççþèéþÈ˜ŸÏ */
	$"0000 00FA E8D1 E9E9 CAE8 F3B1 C298 C8FC"            /* ...úèÑééÊèó±Â˜Èü */
	$"0000 0056 FEFE FEFE D1FD FED0 ABA6 CFAC"            /* ...VþþþþÑýþÐ«¦Ï¬ */
};

data 'ics8' (137, "MIDI Music Icon") {
	$"0000 00F7 56F8 F7F8 F8F8 F8F8 FAF7 0000"            /* ...÷Vø÷øøøøøú÷.. */
	$"002B FA7C 81F9 F600 0000 0000 56FB 2B00"            /* .+ú|ùö.....Vû+. */
	$"2BAC A0A1 A1D1 AC2B 00F5 F5F5 56F7 FBF7"            /* +¬ ¡¡Ñ¬+.õõõV÷û÷ */
	$"A582 A19B CBAD A6FC 00F5 F500 F8FB FBFC"            /* ¥‚¡›Ë­¦ü.õõ.øûûü */
	$"A7A5 A7A7 D1FD A6A7 F5F5 F5F6 F5F7 2B81"            /* §¥§§Ñý¦§õõõöõ÷+ */
	$"A6A7 7BFD FD81 A7A6 F5F5 F5F5 F6F5 F5FA"            /* ¦§{ýý§¦õõõõöõõú */
	$"75A1 A775 51A1 E582 00F5 F5F5 F5F6 F581"            /* u¡§uQ¡å‚.õõõõöõ */
	$"25A1 A17B 76C5 A12A 002B F9F5 F6F6 F581"            /* %¡¡{vÅ¡*.+ùõööõ */
	$"00F6 76FB 7B7B 2B2B 002B F900 2BFA F581"            /* .övû{{++.+ù.+úõ */
	$"0000 00F8 F500 F5F9 00F6 F900 2BF9 0081"            /* ...øõ.õù.öù.+ù. */
	$"00F5 2456 F7F8 56FB F756 812B 5681 2BFB"            /* .õ$V÷øVû÷V+V+û */
	$"5688 6464 6464 5D64 64DE 81F5 F8FA F681"            /* Vˆdddd]ddÞõøúö */
	$"F914 160E 150E 1515 155D 56FA FFFB F6FB"            /* ù........]Vúÿûöû */
	$"5615 0F14 150E 1515 1588 2BF9 FEF9 2BFB"            /* V........ˆ+ùþù+û */
	$"5664 8E65 6464 5D64 5D5D F500 F5F5 F581"            /* VdŽedd]d]]õ.õõõ */
	$"0024 00F8 FA7A 8056 FAFA FAFA FAFA FAFC"            /* .$.øúz€Vúúúúúúúü */
};

data 'ics8' (138, "MML Script Icon") {
	$"0000 00F7 56F8 F7F8 F8F7 F8F8 FAF7 0000"            /* ...÷Vø÷øø÷øøú÷.. */
	$"002B 807C 81FA F600 00F8 F8F5 F8FB 2B00"            /* .+€|úö..øøõøû+. */
	$"2BAC 77A1 E5AC ACF7 F8F6 56FA F9F8 FBF7"            /* +¬w¡å¬¬÷øöVúùøû÷ */
	$"FBA6 A19B A7AD A6FD F5F7 FAFB FAFB 81FC"            /* û¦¡›§­¦ýõ÷úûúûü */
	$"A7FB A6E7 E8FE 82CB F7FA ACFA F52B F7FA"            /* §û¦çèþ‚Ë÷ú¬úõ+÷ú */
	$"A1A6 7CAC FD81 A1A6 FAFC FAF9 56F5 F581"            /* ¡¦|¬ý¡¦úüúùVõõ */
	$"7BC5 A07B 51A1 A1FD FCF9 F7F5 F856 F581"            /* {Å {Q¡¡ýüù÷õøVõ */
	$"F5A1 A17C 76E3 A681 56F7 2BF5 00F9 F8FA"            /* õ¡¡|vã¦V÷+õ.ùøú */
	$"00F6 767B 7B82 7BF8 F72B 2AF5 002B 8181"            /* .öv{{‚{ø÷+*õ.+ */
	$"0000 0056 F6F5 80F8 2BF5 F5F5 F5F8 FB81"            /* ...Vöõ€ø+õõõõøû */
	$"0000 00F8 F600 F5F9 F6F5 F5F5 F7FB 56FB"            /* ...øö.õùöõõõ÷ûVû */
	$"0000 0056 F500 00F8 FBF5 F52B FBF9 F581"            /* ...Võ..øûõõ+ûùõ */
	$"0000 00F8 F600 F8FB 81F7 2AFC 56F5 F681"            /* ...øö.øû÷*üVõö */
	$"0000 0056 F600 F9FA 2B81 FB56 F6F5 F5FB"            /* ...Vö.ùú+ûVöõõû */
	$"0000 00F8 F500 00F9 FAFC F800 F5F5 00FB"            /* ...øõ..ùúüø.õõ.û */
	$"0000 0056 FA56 F9FA FCFB FAFA FAFA FAFC"            /* ...VúVùúüûúúúúúü */
};

data 'ics8' (139, "Text File Icon") {
	$"0000 00F7 56F8 F7F8 F8F8 F8F8 FAF7 0000"            /* ...÷Vø÷øøøøøú÷.. */
	$"002B FA7C 81F9 F600 0000 0000 56FB 2B00"            /* .+ú|ùö.....Vû+. */
	$"2BAC A0A1 A1D1 AC2B 00F5 F5F5 56F7 FBF7"            /* +¬ ¡¡Ñ¬+.õõõV÷û÷ */
	$"A582 A19B CBAD A6FC 00F5 F5F5 F7FB FBFC"            /* ¥‚¡›Ë­¦ü.õõõ÷ûûü */
	$"A7A5 A7A7 D1FD A6A7 F6F6 F6F6 2BF8 2B81"            /* §¥§§Ñý¦§öööö+ø+ */
	$"A6A7 7BFD FD81 A0AD F72B F8F7 F7F7 F5FA"            /* ¦§{ýý ­÷+ø÷÷÷õú */
	$"76A1 A775 57A1 E57B F82B F8F8 F756 F681"            /* v¡§uW¡å{ø+øø÷Vö */
	$"F6A0 A17B 76E3 A7F8 2B2B 2BF7 2BF7 F581"            /* ö ¡{vã§ø+++÷+÷õ */
	$"0025 7C7B 7B7B 2BF7 5656 F7F8 F8F8 F680"            /* .%|{{{+÷VV÷øøøö€ */
	$"0000 0056 F600 0000 0000 00F5 F5F5 F581"            /* ...Vö......õõõõ */
	$"0000 00F8 F500 0000 F5F5 F5F5 F6F6 F681"            /* ...øõ...õõõõööö */
	$"0000 0056 F600 00F5 00F5 F5F5 F5F6 F581"            /* ...Vö..õ.õõõõöõ */
	$"0000 00F8 F600 F500 F5F5 F5F5 F6F5 F6FB"            /* ...øö.õ.õõõõöõöû */
	$"0000 0056 F500 00F5 00F5 F5F6 F5F6 F581"            /* ...Võ..õ.õõöõöõ */
	$"0000 0056 F500 0000 0000 0000 F5F5 00FB"            /* ...Võ.......õõ.û */
	$"0000 0056 F9F9 F9F9 F9F9 FAFA FAFA FAFC"            /* ...Vùùùùùùúúúúúü */
};

data 'ics8' (140, "Generic File Icon") {
	$"0000 00F7 56F8 F7F8 F8F8 F8F8 FAF7 0000"            /* ...÷Vø÷øøøøøú÷.. */
	$"002B FA7C 81F9 F600 0000 0000 56FB 2B00"            /* .+ú|ùö.....Vû+. */
	$"2BAC A0A1 A1D1 AC2B 00F5 F5F5 56F7 FBF7"            /* +¬ ¡¡Ñ¬+.õõõV÷û÷ */
	$"A582 A19B CBAD A6FC 00F5 F500 F8FB FBFC"            /* ¥‚¡›Ë­¦ü.õõ.øûûü */
	$"A7A5 A7A7 D1FD A6A7 F5F5 F5F6 F5F7 2B81"            /* §¥§§Ñý¦§õõõöõ÷+ */
	$"A6A7 7BFD FD81 A6A7 F5F5 F5F5 F6F5 F5FA"            /* ¦§{ýý¦§õõõõöõõú */
	$"76A1 A775 51A1 E5FB 00F5 F5F5 F5F6 F581"            /* v¡§uQ¡åû.õõõõöõ */
	$"F6A0 A17B 76E4 A7F6 00F5 F5F6 F5F6 F581"            /* ö ¡{vä§ö.õõöõöõ */
	$"002B 76FB 757B F600 F5F5 F5F5 F6F6 F580"            /* .+vûu{ö.õõõõööõ€ */
	$"0000 0050 0700 00F5 00F5 F5F5 F5F6 F581"            /* ...P...õ.õõõõöõ */
	$"0000 00F8 F600 0000 F5F5 F5F5 F6F5 F6FB"            /* ...øö...õõõõöõöû */
	$"0000 00F8 F600 00F5 00F5 F5F6 F5F6 F581"            /* ...øö..õ.õõöõöõ */
	$"0000 0056 F500 F500 F5F5 F5F5 F6F5 F6FB"            /* ...Võ.õ.õõõõöõöû */
	$"0000 00F8 F600 00F5 00F5 F5F5 F5F6 F581"            /* ...øö..õ.õõõõöõ */
	$"0000 0056 F500 0000 0000 00F5 00F5 00FB"            /* ...Võ......õ.õ.û */
	$"0000 0056 F9F9 F9F9 F9F9 FAFA FAFA FAFC"            /* ...Vùùùùùùúúúúúü */
};

data 'ics8' (141, "Folder Icon") {
	$"F57F 8055 F500 0000 0000 0000 0000 0000"            /* õ.€Uõ........... */
	$"7F55 7F7F 8056 F500 F500 0000 0000 0000"            /* .U..€Võ.õ....... */
	$"552A 557F 7F7F 8080 8080 2B00 0000 0000"            /* U*U...€€€€+..... */
	$"4F06 2A2A 557F 7F55 7E55 AA55 0000 0000"            /* O.**U..U~UªU.... */
	$"552A 2A2A 2A2A 557F 7F7F 54AA 2B00 0000"            /* U*****U...Tª+... */
	$"552A 2A2A 2A30 2A2A 557F 7F80 5600 0000"            /* U****0**U..€V... */
	$"552A 2A2A 542A 542A 2A30 79AA 5500 0000"            /* U***T*T**0yªU... */
	$"552A 4EF9 FB7C 8280 552A 54AB 5600 0000"            /* U*Nùû|‚€U*T«V... */
	$"5B2A FBA6 9BA1 CBFC FB4E 55AB 5500 0000"            /* [*û¦›¡ËüûNU«U... */
	$"554F FD7C A1A1 E8FC AD55 54AB F9F5 0000"            /* UOý|¡¡èü­UT«ùõ.. */
	$"80A5 A7A5 A7E7 ADA5 E780 54AB FDAC 56F5"            /* €¥§¥§ç­¥ç€T«ý¬Võ */
	$"067B CB82 A6AC FD7C A780 55AA ABFD FEAC"            /* .{Ë‚¦¬ý|§€Uª«ýþ¬ */
	$"00F6 A1A1 A056 A0A1 E655 5AA5 B2FE FDFD"            /* .ö¡¡ V ¡æUZ¥²þýý */
	$"0000 75A1 A157 9BC5 8130 7986 D0FD FE81"            /* ..u¡¡W›Å0y†Ðýþ */
	$"0000 0050 A050 7CF9 807F 55AA ACFE 8100"            /* ...P P|ù€.Uª¬þ. */
	$"0000 0000 0000 0000 F555 86AB FD81 0000"            /* ........õU†«ý.. */
};

data 'ictb' (2000) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
};

data 'ictb' (2100) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000"                                          /* .... */
};

data 'ictb' (4001) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
};

data 'ictb' (4005) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000"                                          /* .... */
};

data 'kind' (128) {
	$"3236 2E41 0000 0000 000C 7363 6541 0D41"            /* 26.A......sceA.A */
	$"6C65 7068 204F 6E65 204D 6170 7367 6141"            /* leph One MapsgaA */
	$"1441 6C65 7068 204F 6E65 2053 6176 6564"            /* .Aleph One Saved */
	$"2047 616D 6500 6669 6C41 0E41 6C65 7068"            /*  Game.filA.Aleph */
	$"204F 6E65 2046 696C 6D00 7068 7941 1641"            /*  One Film.phyA.A */
	$"6C65 7068 204F 6E65 2050 6879 7369 6373"            /* leph One Physics */
	$"2046 696C 6500 7368 7041 1541 6C65 7068"            /*  File.shpA.Aleph */
	$"204F 6E65 2053 6861 7065 7320 4669 6C65"            /*  One Shapes File */
	$"736E 6441 1541 6C65 7068 204F 6E65 2053"            /* sndA.Aleph One S */
	$"6F75 6E64 7320 4669 6C65 696D 6741 1541"            /* ounds FileimgA.A */
	$"6C65 7068 204F 6E65 2049 6D61 6765 7320"            /* leph One Images  */
	$"4669 6C65 6D75 7341 1441 6C65 7068 204F"            /* FilemusA.Aleph O */
	$"6E65 204D 7573 6963 2046 696C 6500 4D49"            /* ne Music File.MI */
	$"4449 1341 6C65 7068 204F 6E65 204D 4944"            /* DI.Aleph One MID */
	$"4920 4669 6C65 4D4D 4C20 0F4D 4D4C 2053"            /* I FileMML .MML S */
	$"6372 6970 7420 4669 6C65 7072 6566 1A41"            /* cript Filepref.A */
	$"6C65 7068 204F 6E65 2050 7265 6665 7265"            /* leph One Prefere */
	$"6E63 6573 2046 696C 6500 5445 5854 1341"            /* nces File.TEXT.A */
	$"6C65 7068 204F 6E65 2054 6578 7420 4669"            /* leph One Text Fi */
	$"6C65"                                               /* le */
};

data 'ldes' (450) {
	$"0000 0000 0001 0000 0000 0100 0000 0000"            /* ................ */
	$"0000"                                               /* .. */
};

data 'vers' (1) {
	$"0000 8001 0000 0630 2E32 302E 3306 302E"            /* ..€....0.20.3.0. */
	$"3230 2E33"                                          /* 20.3 */
};

