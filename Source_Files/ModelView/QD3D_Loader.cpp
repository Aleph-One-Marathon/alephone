/*
	Alias|Wavefront Object Loader
	
	By Loren Petrich, September 1, 2001
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "cseries.h"
#include "QD3D_Loader.h"

#include <QD3D.h>
#include <QD3DGroup.h>
#include <QD3DIO.h>
#include <QD3DStorage.h>
#include <QD3DErrors.h>
#include <QD3DShader.h>
#include <QD3DCamera.h>
#include <QD3DMath.h>
#include <QD3DLight.h>
#include <QD3DExtension.h>
#include <QD3DRenderer.h>
#include <QD3DGeometry.h>


// Debug-message destination
static FILE *DBOut = NULL;

// QD3D's presence and whether it was checked
static bool QD3D_Present = false;
static bool QD3D_Presence_Checked = false;

void SetDebugOutput_QD3D(FILE *DebugOutput)
{
	DBOut = DebugOutput;
}


bool LoadModel_QD3D(FileSpecifier& Spec, Model3D& Model)
{
	// Clear out the final model object
	Model.Clear();
	
	// Test for QD3D's presence and initialize it
	// Modify for Quesa as suitable
	if (QD3D_Presence_Checked)
	{
		if (!QD3D_Present) return false;
	}
	else if ((void*)Q3Initialize != (void*)kUnresolvedCFragSymbolAddress)
	{
		TQ3Status Success = Q3Initialize();
		QD3D_Present = (Success == kQ3Success);
		QD3D_Presence_Checked = Success;
	}
	
	if (DBOut)
	{
		// Read buffer
		const int BufferSize = 256;
		char Buffer[BufferSize];
		Spec.GetName(Buffer);
		fprintf(DBOut,"Loading QuickDraw 3D model file %s\n",Buffer);
	}

	// Create a QD3D model object; replace with code appropriate for Quesa as necessary
	
	// First create a file object and a storage object,
	// and associate the storage object with the file object.
	TQ3StorageObject StorageObject = Q3FSSpecStorage_New(&Spec.GetSpec());
	if (!StorageObject) return false;
	
	TQ3FileObject FileObject = Q3File_New();
	if (!FileObject)
	{
		Q3Object_Dispose(StorageObject);
		return false;
	}
	
	Q3File_SetStorage(FileObject, StorageObject);
	Q3Object_Dispose(StorageObject);
	
	// Read in the model object
    if (Q3File_OpenRead(file, NULL) != kQ3Success)
	{
		Q3Object_Dispose(FileObject);
		return false;
	}
	
	// Create a group for holding all the read-in objects
	TQ3Object *Q3Model = Q3DisplayGroup_New();
	if (!Q3Model)
	{
		Q3Object_Dispose(FileObject);
		return false;
	}
		
	// All at once
    while (Q3File_IsEndOfFile(FileObject) == kQ3False)
    {
		// Grab an object from the file
		TQ3Object Q3Object = NULL;
        Q3Object = Q3File_ReadObject(FileObject);
        if (!Q3Object)
            continue;
		
		// Only interested in renderable objects (no hints or comments or ...)
        if (Q3Object_IsDrawable(myObject))
			Q3Group_AddObject(Q3Model, Q3Object);
		
		// Clean up
        if (Q3Object) 
            Q3Object_Dispose(Q3Object);
    }
	// Done with the file object
	Q3Object_Dispose(FileObject);
    
    if (Q3Error_Get(NULL) != kQ3ErrorNone)
    {
		if (Q3Model)
			Q3Object_Dispose(Q3Model); 
		return false;
	}

	
	// Clean up
	Q3Object_Dispose(Q3Model); 
	return false;
}
