// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#ifndef _CSERIES_FILES_
#define _CSERIES_FILES_

extern OSErr get_file_spec(
	FSSpec *spec,
	short listid,
	short item,
	short pathsid);

extern OSErr get_my_fsspec(
	FSSpec *spec);

#endif
