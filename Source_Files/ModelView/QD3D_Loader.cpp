/*
	Alias|Wavefront Object Loader
	
	By Loren Petrich, September 1, 2001
	
	Much of the QuickDraw 3D code uses code inspired by
	Brian Greenstone's utility 3DMF Mapper; some of it is actually copied.
*/

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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "cseries.h"
#include "QD3D_Loader.h"


// Debug-message destination
static FILE *DBOut = NULL;

// QD3D's presence and whether it was checked
static bool QD3D_Present = false;
static bool QD3D_Presence_Checked = false;

void SetDebugOutput_QD3D(FILE *DebugOutput)
{
	DBOut = DebugOutput;
}

// The level of tesselation to be used on curved surfaces
// So as to automatically get a reasonable value
const TQ3SubdivisionMethod DefaultTesselationMethod = kQ3SubdivisionMethodConstant;
// Reduced from BG's value of 8; important to avoid polygon overload
const float DefaultTesselationLength = 4;
static TQ3SubdivisionStyleData TesselationData =
{
	DefaultTesselationMethod,
	DefaultTesselationLength,
	DefaultTesselationLength
};

void SetTesselation_QD3D(bool IsWorldLength, float TessLength)
{
	TesselationData.method =
		IsWorldLength ? kQ3SubdivisionMethodWorldSpace : kQ3SubdivisionMethodConstant;
	TesselationData.c1 = TesselationData.c2 = TessLength;
}

void SetDefaultTesselation_QD3D()
{	
	TesselationData.method = DefaultTesselationMethod;
	TesselationData.c1 = TesselationData.c2 = DefaultTesselationLength;
}

// Local globals needed for triangulating objects:
static TQ3XObjectClass TriangulatorClass = NULL;
static TQ3ObjectType TriangulatorClassType = NULL;
static TQ3ViewObject TriangulatorView = NULL;

// For displaying QD3D errors:
static void QD3D_Error_Handler(TQ3Error FirstError, TQ3Error LastError, long UserData)
{
	if (DBOut) fprintf(DBOut,"QD3D Error: %d\n",LastError);
}

// Loads a QD3D model as a group object; returns NULL if none could be loaded
static TQ3Object LoadModel(FileSpecifier& File);

// Creates a fake renderer (TriangulatorView) that decomposes surfaces into triangles;
// if already created, this call will not do anything
static bool CreateTriangulator();
static TQ3XFunctionPointer TriangulatorMetaHandler(TQ3XMethodType MethodType);
static TQ3Status TriangulatorStartFrame(TQ3ViewObject View, void *PrivateData,
	TQ3DrawContextObject DrawContext);
static TQ3Status TriangulatorStartPass(TQ3ViewObject View, void *PrivateData,
	TQ3CameraObject Camera, TQ3GroupObject LightGroup);
static TQ3ViewStatus TriangulatorEndPass(TQ3ViewObject View, void *PrivateData);
static void TriangulatorCancel(TQ3ViewObject View, void *PrivateData);
static TQ3XFunctionPointer TriangulatorGeometry_MetaHandler(TQ3XMethodType MethodType);
static TQ3Status TriangulatorGeometry_Triangle(TQ3ViewObject View, void *PrivateData,
	TQ3GeometryObject Triangle, const TQ3TriangleData *TriangleData);

// DEBUG:
static int Index = 0;

bool LoadModel_QD3D(FileSpecifier& Spec, Model3D& Model)
{
	// Clear out the final model object
	Model.Clear();
	
	// Test for QD3D/Quesa's presence and initialize it if not present
	if (QD3D_Presence_Checked)
	{
		if (!QD3D_Present) return false;
	}
	else
	{
		QD3D_Presence_Checked = true;
		
		// MacOS QD3D; modify this for Quesa as appropriate
		if ((void*)Q3Initialize != (void*)kUnresolvedCFragSymbolAddress)
		{
			TQ3Status Success = Q3Initialize();
			QD3D_Present = (Success == kQ3Success);
		}
		
		// Do additional setup;
		// if the triangulator could not be created, then act as if
		// QD3D/Quesa had not been loaded
		if (QD3D_Present)
		{
			Q3Error_Register(QD3D_Error_Handler,0);
			QD3D_Present = CreateTriangulator();
		}
		if (!QD3D_Present)
			Q3Exit();
	}
	
	if (DBOut)
	{
		// Read buffer
		const int BufferSize = 256;
		char Buffer[BufferSize];
		Spec.GetName(Buffer);
		fprintf(DBOut,"Loading QuickDraw-3D model file %s\n",Buffer);
	}
	
	TQ3Object ModelObject = LoadModel(Spec);
	if (!ModelObject) return false;
	
	// DEBUG
	Index = 0;
	
	if (Q3View_StartRendering(TriangulatorView) == kQ3Failure)
	{
		if (DBOut) fprintf(DBOut,"ERROR: couldn't start triangulation 'rendering'\n");
		return false;
	}
	do
	{
		Q3SubdivisionStyle_Submit(&TesselationData, TriangulatorView);
		if (Q3Object_Submit(ModelObject, TriangulatorView) == kQ3Failure)
		{
			if (DBOut) fprintf(DBOut,"ERROR: model could not be 'rendered'\n");
		}
	}
	while (Q3View_EndRendering(TriangulatorView) == kQ3ViewStatusRetraverse);

	// Clean up
	Q3Object_Dispose(ModelObject); 
	
	return false;
}


// Load a QD3D model object:
TQ3Object LoadModel(FileSpecifier& Spec)
{
	// First create a file object and a storage object,
	// and associate the storage object with the file object.
	
	// MacOS / FSSpec version:
	// modify as necessary for Quesa and SDL --
	// Quesa has similar calls with the MacOS FSSpec being replaced by a Windows file handle
	// and a Unix file path
	TQ3StorageObject StorageObject = Q3FSSpecStorage_New(&Spec.GetSpec());
	if (!StorageObject) return NULL;

	TQ3FileObject FileObject = Q3File_New();
	if (!FileObject)
	{
		Q3Object_Dispose(StorageObject);
		return NULL;
	}
	
	Q3File_SetStorage(FileObject, StorageObject);
	Q3Object_Dispose(StorageObject);
	
	// Read in the model object
	if (Q3File_OpenRead(FileObject, NULL) != kQ3Success)
	{
		Q3Object_Dispose(FileObject);
		return NULL;
	}
	
	// Create a group for holding all the read-in objects
	TQ3Object ModelObject = Q3DisplayGroup_New();
	if (!ModelObject)
	{
		Q3Object_Dispose(FileObject);
		return NULL;
	}
	
	// All at once: slurp in now and process later
	while (Q3File_IsEndOfFile(FileObject) == kQ3False)
	{
		// Grab an object from the file
		TQ3Object MemberObject = Q3File_ReadObject(FileObject);
		if (!MemberObject)
			continue;
		
		// Only interested in renderable objects (no hints or comments or ...)
		if (Q3Object_IsDrawable(MemberObject))
			Q3Group_AddObject(ModelObject, MemberObject);
		
		// Clean up
		if (MemberObject) 
			Q3Object_Dispose(MemberObject);
	}
	
	// Done with the file object
	Q3Object_Dispose(FileObject);

	if (Q3Error_Get(NULL) != kQ3ErrorNone)
	{
		if (ModelObject)
			Q3Object_Dispose(ModelObject); 
		return NULL;
	}
	
	// Finally!
	return ModelObject;
}


// Returns whether the creation was a success
bool CreateTriangulator(void)
{
	// No need to create it more than once
	if (TriangulatorView) return true;
	
	TriangulatorClass =
		Q3XObjectHierarchy_RegisterClass(
			kQ3SharedTypeRenderer,
			&TriangulatorClassType,
			"BG-Inspired Triangulator",
			TriangulatorMetaHandler,
			NULL, 0, 0);
	
	if(!TriangulatorClass) return false;
	if(!TriangulatorClassType) return false;
	
	// Dummy draw context: a pixmap (image buffer) that is empty
	TQ3PixmapDrawContextData PixmapContextData =
	{
		// drawContextData
		{
			kQ3ClearMethodWithColor,
			{0,0,0,0},
			{{0,0},{0,0}},
			kQ3False,
			NULL,
			kQ3False,
			kQ3True
		},
		// pixmap
		{
			NULL,
			0,
			0,
			0,
			32,
			kQ3PixelTypeARGB32,
			kQ3EndianBig,
			kQ3EndianBig
		}
	};
	
	TQ3DrawContextObject DrawContextObject = Q3PixmapDrawContext_New(&PixmapContextData);
	if (!DrawContextObject) return false;
	
	TQ3RendererObject RendererObject = Q3Renderer_NewFromType(TriangulatorClassType);
	if (!RendererObject)
	{
		Q3Object_Dispose(DrawContextObject);
		return false;
	}
	
	TriangulatorView = Q3View_New();
	bool Success = TriangulatorView != NULL;
	if(Success)
		Success = (Q3View_SetDrawContext(TriangulatorView, DrawContextObject) == kQ3Success);
	if(Success)
		Success = (Q3View_SetRenderer(TriangulatorView, RendererObject) == kQ3Success);
	
	// No longer needed	
	Q3Object_Dispose(DrawContextObject);
	Q3Object_Dispose(RendererObject);
	return Success;
}

static TQ3XFunctionPointer TriangulatorMetaHandler(TQ3XMethodType MethodType)
{
	switch(MethodType)
	{
	case kQ3XMethodTypeRendererStartFrame:
		return (TQ3XFunctionPointer)TriangulatorStartFrame;
	
	case kQ3XMethodTypeRendererStartPass:
		return (TQ3XFunctionPointer)TriangulatorStartPass;
	
	case kQ3XMethodTypeRendererEndPass:
		return (TQ3XFunctionPointer)TriangulatorEndPass;
	
	case kQ3XMethodTypeRendererCancel:
		return (TQ3XFunctionPointer)TriangulatorCancel;
	
	case kQ3XMethodTypeRendererSubmitGeometryMetaHandler:
		return (TQ3XFunctionPointer)TriangulatorGeometry_MetaHandler;
	
	case kQ3XMethodTypeRendererIsInteractive:	// Why does BG's code do this?
		return (TQ3XFunctionPointer)kQ3True;
	
	default:
		return NULL;
	}
}

TQ3Status TriangulatorStartFrame(TQ3ViewObject View, void *PrivateData,
	TQ3DrawContextObject DrawContext)
{
	return kQ3Success;
}

TQ3Status TriangulatorStartPass(TQ3ViewObject View, void *PrivateData,
	TQ3CameraObject Camera, TQ3GroupObject LightGroup)
{
	return kQ3Success;
}

TQ3ViewStatus TriangulatorEndPass(TQ3ViewObject View, void *PrivateData)
{
	return kQ3ViewStatusDone;
}

void TriangulatorCancel(TQ3ViewObject View, void *PrivateData)
{
}

TQ3XFunctionPointer TriangulatorGeometry_MetaHandler(TQ3XMethodType MethodType)
{
	// Force the renderer to make triangles (Brian Greenstone's idea)
	switch(MethodType)
	{
	case kQ3GeometryTypeTriangle:
		return (TQ3XFunctionPointer)TriangulatorGeometry_Triangle;
	
	default:
		return NULL;
	}
}

TQ3Status TriangulatorGeometry_Triangle(TQ3ViewObject View, void *PrivateData,
	TQ3GeometryObject Triangle, const TQ3TriangleData *TriangleData)
{
	if (!TriangleData) return kQ3Success;
	
	// For now, just indicate that a triangle had been found
	printf("Triangle %d\n",Index++);
	for (int k=0; k<3; k++)
	{
		const TQ3Point3D P = TriangleData->vertices[k].point;
		printf("%9f %9f %9f\n",P.x,P.y,P.z);
	}
	
	return kQ3Success;
}
