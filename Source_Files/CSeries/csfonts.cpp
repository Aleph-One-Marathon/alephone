// LP: not sure who originally wrote these cseries files: Bo Lindbergh?
#include <Resources.h>
#include <Quickdraw.h>

#include "csfonts.h"

static TextSpec null_text_spec;

void GetNewTextSpec(
	TextSpec *spec,
	short resid,
	short item)
{
	Handle res;
	int cnt;
	TextSpec *src;

	res=GetResource('finf',resid);
	if (!res)
		goto notfound;
	if (!*res)
		LoadResource(res);
	if (!*res)
		goto notfound;
	cnt=*(short *)*res;
	if (item<0 || item>=cnt)
		goto notfound;
	src=(TextSpec *)(*res+sizeof (short));
	*spec=src[item];
	return;
notfound:
	*spec=null_text_spec;
}

void GetFont(
	TextSpec *spec)
{
	GrafPtr port;

	GetPort(&port);
	spec->font=port->txFont;
	spec->style=port->txFace;
	spec->size=port->txSize;
}

void SetFont(
	TextSpec *spec)
{
	TextFont(spec->font);
	TextFace(spec->style);
	TextSize(spec->size);
}

