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

// Loads a QD3D model as a group object; returns NULL if it could not be loaded
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

// Accumulated triangle vertices:
bool TxtrCoordsPresent = false;
bool NormalsPresent = false;
bool ColorsPresent = false;

struct FullVertexData
{
	GLfloat Position[3];
	GLfloat TxtrCoord[2];
	GLfloat Normal[3];
	GLfloat Color[3];
};
static vector<FullVertexData> FullVertexList;

static void StartAccumulatingVertices();
static void GetVerticesIntoModel(Model3D& Model);


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
	
	StartAccumulatingVertices();
	
	if (Q3View_StartRendering(TriangulatorView) == kQ3Failure)
	{
		if (DBOut) fprintf(DBOut,"ERROR: couldn't start triangulation 'rendering'\n");
		Q3Object_Dispose(ModelObject);
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

	// Done with the model
	Q3Object_Dispose(ModelObject);
	
	GetVerticesIntoModel(Model);
	
	return !Model.Positions.empty();
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
	// Just in case...
	if (!TriangleData) return kQ3Success;
	
	// Get the overall normal and color (DiffuseColor)
	TQ3Boolean OverallColorPresent =
		Q3AttributeSet_Contains(TriangleData->triangleAttributeSet,kQ3AttributeTypeDiffuseColor);
	
	TQ3ColorRGB OverallColor;
	if (OverallColorPresent);
		Q3AttributeSet_Get(TriangleData->triangleAttributeSet,kQ3AttributeTypeDiffuseColor,&OverallColor);
	
	// Load the triangle's contents into a vertex object
	for (int k=0; k<3; k++)
	{
		FullVertexData FullVertex;
		obj_clear(FullVertex);
		
		const TQ3Vertex3D& Vertex = TriangleData->vertices[k];
		FullVertex.Position[0] = Vertex.point.x;
		FullVertex.Position[1] = Vertex.point.y;
		FullVertex.Position[2] = Vertex.point.z;
		
		TQ3Param2D TxtrCoord;
		TQ3Boolean TxtrCoord_Present =
			Q3AttributeSet_Contains(Vertex.attributeSet,kQ3AttributeTypeShadingUV);
		if (TxtrCoord_Present)
			Q3AttributeSet_Get(Vertex.attributeSet,kQ3AttributeTypeShadingUV,&TxtrCoord);
		else
		{
			TxtrCoord_Present =
				Q3AttributeSet_Contains(Vertex.attributeSet,kQ3AttributeTypeSurfaceUV);
			if (TxtrCoord_Present)
				Q3AttributeSet_Get(Vertex.attributeSet,kQ3AttributeTypeSurfaceUV,&TxtrCoord);
			else
				TxtrCoordsPresent = false;
		}
		
		if (TxtrCoordsPresent)
		{
			FullVertex.TxtrCoord[0] = TxtrCoord.u;
			FullVertex.TxtrCoord[1] = TxtrCoord.v;
		}
		
		TQ3Vector3D Normal;
		TQ3Boolean Normal_Present =
			Q3AttributeSet_Contains(Vertex.attributeSet,kQ3AttributeTypeNormal);
		if (Normal_Present)
			Q3AttributeSet_Get(Vertex.attributeSet,kQ3AttributeTypeNormal,&Normal);
		else
			NormalsPresent = false;
		
		if (NormalsPresent)
		{
			FullVertex.Normal[0] = Normal.x;
			FullVertex.Normal[1] = Normal.y;
			FullVertex.Normal[2] = Normal.z;
		}
		
		TQ3ColorRGB Color;
		TQ3Boolean Color_Present =
			Q3AttributeSet_Contains(Vertex.attributeSet,kQ3AttributeTypeDiffuseColor);
		if (Color_Present)
			Q3AttributeSet_Get(Vertex.attributeSet,kQ3AttributeTypeDiffuseColor,&Color);
		else if (OverallColorPresent)
			Color = OverallColor;
		else
			ColorsPresent = false;
		
		if (ColorsPresent)
		{
			FullVertex.Color[0] = Color.r;
			FullVertex.Color[1] = Color.g;
			FullVertex.Color[2] = Color.b;
		}
		FullVertexList.push_back(FullVertex);
	}
	
	return kQ3Success;
}


void StartAccumulatingVertices()
{
	// One of them absent means ignore all of them
	TxtrCoordsPresent = true;
	NormalsPresent = true;
	ColorsPresent = true;
	FullVertexList.clear();
}

void GetVerticesIntoModel(Model3D& Model)
{
	int NumVertices = FullVertexList.size();
	Model.Positions.resize(3*NumVertices);
	if (TxtrCoordsPresent)
		Model.TxtrCoords.resize(2*NumVertices);
	if (NormalsPresent)
		Model.Normals.resize(3*NumVertices);
	if (ColorsPresent)
		Model.Colors.resize(3*NumVertices);
	Model.VertIndices.resize(NumVertices);
	
	// Need to optimize the model by getting rid of redundant vertices;
	// will search for them by index-sorting them,
	// which will be done parameter by parameter.
	
	for (int k=0; k<NumVertices; k++)
	{
		FullVertexData& FullVertex = FullVertexList[k];
		
		GLfloat *Position = &Model.Positions[3*k];
		for (int c=0; c<3; c++)
			Position[c] = FullVertex.Position[c];
		
		if (TxtrCoordsPresent)
		{
			GLfloat *TxtrCoord = &Model.TxtrCoords[2*k];
			for (int c=0; c<3; c++)
				TxtrCoord[c] = FullVertex.TxtrCoord[c];
		}
		
		if (NormalsPresent)
		{
			GLfloat *Normal = &Model.Normals[3*k];
			for (int c=0; c<3; c++)
				Normal[c] = FullVertex.Normal[c];
		}
		
		if (ColorsPresent)
		{
			GLfloat *Color = &Model.Colors[3*k];
			for (int c=0; c<3; c++)
				Color[c] = FullVertex.Color[c];
		}
		
		Model.VertIndices[k] = k;
	}
	
	// All done!
	FullVertexList.clear();
}
