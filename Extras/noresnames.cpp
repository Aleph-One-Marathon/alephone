#include <stdio.h>
#include <string.h>

#include <Resources.h>

int main(
	int argc,
	char **argv)
{
	int argix;
	Str255 name;
	int len;
	FSSpec spec;
	OSStatus err;
	short ref;
	int typecnt,typeix;
	ResType type,otype;
	int rescnt,resix;
	Handle res;
	short id;

	for (argix=1; argix<argc; argix++) {
		len=strlen(argv[argix]);
		if (len>255) {
			fprintf(stderr,"### %s: filename too long: %s\n",argv[0],argv[argix]);
			continue;
		}
		name[0]=len;
		memcpy(name+1,argv[argix],len);
		err=FSMakeFSSpec(0,0,name,&spec);
		if (err!=noErr) {
			fprintf(stderr,"### %s: FSMakeFSSpec returned %d: %s\n",argv[0],err,argv[argix]);
			continue;
		}
		SetResLoad(false);
		ref=FSpOpenResFile(&spec,fsRdWrPerm);
		err=ResError();
		SetResLoad(true);
		if (ref==-1) {
			fprintf(stderr,"### %s: FSpOpenResFile returned %d: %s\n",argv[0],err,argv[argix]);
			continue;
		}
		typecnt=Count1Types();
		for (typeix=1; typeix<=typecnt; typeix++) {
			Get1IndType(&otype,typeix);
			type=otype;
			rescnt=Count1Resources(type);
			for (resix=1; resix<=rescnt; resix++) {
				SetResLoad(false);
				res=Get1IndResource(type,resix);
				SetResLoad(true);
				if (!res)
					continue;
				GetResInfo(res,&id,&otype,name);
				if (name[0])
					SetResInfo(res,id,"\p");
				ReleaseResource(res);
			}
		}
		CloseResFile(ref);
	}
	return 0;
}

