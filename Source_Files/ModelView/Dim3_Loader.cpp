/*

	Copyright (C) 1991-2001 and beyond by Bungie Studios, Inc.
	and the "Aleph One" developers.
 
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	This license is contained in the file "COPYING",
	which is included with this source code; it is available online at
	http://www.gnu.org/licenses/gpl.html

	Dim3 Object Loader
	
	By Loren Petrich, Dec 29, 2001

	Derived from the work of
	
	Brian Barnes (bbarnes@klinksoftware.com)
	
*/

#include "cseries.h"

#ifdef HAVE_OPENGL

#include <math.h>

#include "Dim3_Loader.h"
#include "world.h"
#include "InfoTree.h"
#include "Logging.h"


const float DegreesToInternal = float(FULL_CIRCLE)/float(360);

// Convert angle from degrees to the Marathon engine's internal units
static int16 GetAngle(float InAngle)
{
	float A = DegreesToInternal*InAngle;
	int16 IA = (A >= 0) ? int16(A + 0.5) : - int16(-A + 0.5);
	return NORMALIZE_ANGLE(IA);
}


// Local globals; these are to be persistent across calls when loading several files.

// Bone-tag and name-tag intermediate arrays:

const int BoneTagSize = 8;

struct BoneTagWrapper
{
	char Tag0[BoneTagSize], Tag1[BoneTagSize];
};

// For VertexBoneTags, this means major bone tag, then minor bone tag.
// For BoneOwnTags, this means its own tag, then its parent tag.
static vector<BoneTagWrapper> VertexBoneTags, BoneOwnTags;

// Translation from read-in bone order to "true" order
static vector<size_t> BoneIndices;

// Names of frames and seqeunces:

const int NameTagSize = 32;

struct NameTagWrapper
{
	char Tag[NameTagSize];
};

static vector<NameTagWrapper> FrameTags;

// Normals (per vertex source)
static vector<GLfloat> Normals;


static void parse_bounding_box(const InfoTree& root, Model3D& Model)
{
	float x_size = 0;
	float y_size = 0;
	float z_size = 0;
	float x_offset = 0;
	float y_offset = 0;
	float z_offset = 0;
	
	std::string tempstr;
	if (root.read_attr("size", tempstr))
		sscanf(tempstr.c_str(), "%f,%f,%f", &x_size, &y_size, &z_size);
	if (root.read_attr("offset", tempstr))
		sscanf(tempstr.c_str(), "%f,%f,%f", &x_offset, &y_offset, &z_offset);
	root.read_attr("x_size", x_size);
	root.read_attr("y_size", x_size);
	root.read_attr("z_size", x_size);
	root.read_attr("x_offset", x_offset);
	root.read_attr("y_offset", y_offset);
	root.read_attr("z_offset", z_offset);
	
	Model.BoundingBox[0][0] = x_offset - x_size/2;
	Model.BoundingBox[0][1] = y_offset - y_size;
	Model.BoundingBox[0][2] = z_offset - z_size/2;
	
	Model.BoundingBox[1][0] = x_offset + x_size/2;
	Model.BoundingBox[1][1] = y_offset;
	Model.BoundingBox[1][2] = z_offset + z_size/2;
}

static void parse_dim3(const InfoTree& root, Model3D& Model)
{
	// ignored: Creator Center Light Shading Shadow_Box Effects
	
	BOOST_FOREACH(InfoTree bound_box, root.children_named("Bound_Box"))
	{
		parse_bounding_box(bound_box, Model);
	}
	BOOST_FOREACH(InfoTree view_box, root.children_named("View_Box"))
	{
		parse_bounding_box(view_box, Model);
	}
	BOOST_FOREACH(InfoTree vertexes, root.children_named("Vertexes"))
	{
		BOOST_FOREACH(InfoTree v, vertexes.children_named("v"))
		{
			Model3D_VertexSource data;
			data.Position[0] = data.Position[1] = data.Position[2] = 0;
			data.Bone0 = data.Bone1 = static_cast<GLushort>(NONE);
			data.Blend = 0;
			
			float norm[3];
			norm[0] = norm[1] = norm[2] = 0;
			
			BoneTagWrapper bt;
			bt.Tag0[0] = bt.Tag1[0] = '\0';
			
			std::string tempstr;
			if (root.read_attr("c3", tempstr))
				sscanf(tempstr.c_str(), "%f,%f,%f", &data.Position[0], &data.Position[1], &data.Position[2]);
			if (root.read_attr("n3", tempstr))
				sscanf(tempstr.c_str(), "%f,%f,%f", &norm[0], &norm[1], &norm[2]);
			root.read_attr("x", data.Position[0]);
			root.read_attr("y", data.Position[1]);
			root.read_attr("z", data.Position[2]);
			
			if (root.read_attr("major", tempstr))
				strncpy(bt.Tag0, tempstr.c_str(), BoneTagSize);
			if (root.read_attr("minor", tempstr))
				strncpy(bt.Tag1, tempstr.c_str(), BoneTagSize);
			
			float factor;
			if (root.read_attr("factor", factor))
				data.Blend = 1 - factor/100;
			
			Model.VtxSources.push_back(data);
			Normals.push_back(norm[0]);
			Normals.push_back(norm[1]);
			Normals.push_back(norm[2]);
			VertexBoneTags.push_back(bt);
		}
	}
	BOOST_FOREACH(InfoTree bones, root.children_named("Bones"))
	{
		BOOST_FOREACH(InfoTree bone, root.children_named("Bone"))
		{
			Model3D_Bone data;
			data.Position[0] = data.Position[1] = data.Position[2] = 0;
			data.Flags = 0;
			
			std::string tempstr;
			if (root.read_attr("c3", tempstr))
				sscanf(tempstr.c_str(), "%f,%f,%f", &data.Position[0], &data.Position[1], &data.Position[2]);
			root.read_attr("x", data.Position[0]);
			root.read_attr("y", data.Position[1]);
			root.read_attr("z", data.Position[2]);
			
			BoneTagWrapper bt;
			bt.Tag0[0] = bt.Tag1[0] = '\0';
			if (bone.read_attr("tag", tempstr))
				strncpy(bt.Tag0, tempstr.c_str(), BoneTagSize);
			if (bone.read_attr("parent", tempstr))
				strncpy(bt.Tag1, tempstr.c_str(), BoneTagSize);
			
			Model.Bones.push_back(data);
			BoneOwnTags.push_back(bt);
		}
	}
	BOOST_FOREACH(InfoTree fills, root.children_named("Fills"))
	{
		BOOST_FOREACH(InfoTree fill, fills.children_named("Fill"))
		{
			BOOST_FOREACH(InfoTree triangles, fill.children_named("Triangles"))
			{
				BOOST_FOREACH(InfoTree v, triangles.children_named("v"))
				{
					uint16 vid = static_cast<uint16>(NONE);
					float txtr_x, txtr_y;
					txtr_x = txtr_y = 0.5;
					float norm_x, norm_y, norm_z;
					norm_x = norm_y = norm_z = 0;
					
					v.read_attr("ID", vid);
					std::string tempstr;
					if (v.read_attr("uv", tempstr))
						sscanf(tempstr.c_str(), "%f,%f", &txtr_x, &txtr_y);
					if (v.read_attr("n", tempstr))
						sscanf(tempstr.c_str(), "%f,%f,%f", &norm_x, &norm_z, &norm_y);
					v.read_attr("xtxt", txtr_x);
					v.read_attr("ytxt", txtr_y);
					
					GLushort index = static_cast<GLushort>(Model.VertIndices.size());
					Model.VertIndices.push_back(index);
					Model.VtxSrcIndices.push_back(vid);
					Model.TxtrCoords.push_back(txtr_x);
					Model.TxtrCoords.push_back(txtr_y);
				}
			}
		}
	}
	BOOST_FOREACH(InfoTree poses, root.children_named("Poses"))
	{
		BOOST_FOREACH(InfoTree pose, poses.children_named("Pose"))
		{
			vector<Model3D_Frame> read_frame;
			size_t num_bones = Model.Bones.size();
			read_frame.resize(num_bones);
			objlist_clear(&read_frame[0], num_bones);
			
			NameTagWrapper nt;
			nt.Tag[0] = '\0';
			std::string tempstr;
			if (pose.read_attr("name", tempstr))
				strncpy(nt.Tag, tempstr.c_str(), NameTagSize);
			
			BOOST_FOREACH(InfoTree bones, pose.children_named("Bones"))
			{
				BOOST_FOREACH(InfoTree bone, bones.children_named("Bone"))
				{
					Model3D_Frame data;
					obj_clear(data);
					
					if (bone.read_attr("rot", tempstr))
					{
						float in_angle[3];
						if (sscanf(tempstr.c_str(), "%f,%f,%f", &in_angle[0], &in_angle[1], &in_angle[2]) == 3)
						{
							for (int c = 0; c < 3; ++c)
								data.Angles[c] = GetAngle(-in_angle[c]);
						}
					}
					float tempfloat;
					if (bone.read_attr("xrot", tempfloat))
						data.Angles[0] = GetAngle(tempfloat);
					if (bone.read_attr("yrot", tempfloat))
						data.Angles[1] = GetAngle(-tempfloat);
					if (bone.read_attr("zrot", tempfloat))
						data.Angles[2] = GetAngle(-tempfloat);
					
					if (bone.read_attr("move", tempstr))
						sscanf(tempstr.c_str(), "%f,%f,%f", &data.Offset[0], &data.Offset[1], &data.Offset[2]);
					bone.read_attr("xmove", data.Offset[0]);
					bone.read_attr("ymove", data.Offset[1]);
					bone.read_attr("zmove", data.Offset[2]);
					
					std::string bone_tag;
					bone.read_attr("tag", bone_tag);
					
					size_t num_bones = BoneOwnTags.size();
					size_t ib;
					for (ib = 0; ib < num_bones; ++ib)
					{
						if (bone_tag == BoneOwnTags[ib].Tag0)
							break;
					}
					if (ib < num_bones)
						obj_copy(read_frame[BoneIndices[ib]], data);
				}
			}
			
			for (size_t b = 0; b < read_frame.size(); ++b)
				Model.Frames.push_back(read_frame[b]);
			FrameTags.push_back(nt);
		}
	}
	BOOST_FOREACH(InfoTree animations, root.children_named("Animations"))
	{
		BOOST_FOREACH(InfoTree animation, animations.children_named("Animation"))
		{
			BOOST_FOREACH(InfoTree poses, animation.children_named("Poses"))
			{
				BOOST_FOREACH(InfoTree pose, poses.children_named("Pose"))
				{
					Model3D_SeqFrame data;
					obj_clear(data);
					data.Frame = NONE;
					
					std::string tempstr;
					if (pose.read_attr("sway", tempstr))
					{
						float in_angle[3];
						if (sscanf(tempstr.c_str(), "%f,%f,%f", &in_angle[0], &in_angle[1], &in_angle[2]) == 3)
						{
							for (int c = 0; c < 3; ++c)
								data.Angles[c] = GetAngle(-in_angle[c]);
						}
					}
					float tempfloat;
					if (pose.read_attr("xsway", tempfloat))
						data.Angles[0] = GetAngle(tempfloat);
					if (pose.read_attr("ysway", tempfloat))
						data.Angles[1] = GetAngle(-tempfloat);
					if (pose.read_attr("zsway", tempfloat))
						data.Angles[2] = GetAngle(-tempfloat);

					if (pose.read_attr("move", tempstr))
						sscanf(tempstr.c_str(), "%f,%f,%f", &data.Offset[0], &data.Offset[1], &data.Offset[2]);
					pose.read_attr("xmove", data.Offset[0]);
					pose.read_attr("ymove", data.Offset[1]);
					pose.read_attr("zmove", data.Offset[2]);
					
					if (pose.read_attr("name", tempstr))
					{
						// Find which frame
						size_t num_frames = FrameTags.size();
						size_t ifr;
						for (ifr = 0; ifr < num_frames; ++ifr)
						{
							if (tempstr == FrameTags[ifr].Tag)
								break;
						}
						if (ifr >= num_frames) ifr = static_cast<size_t>(NONE);
						data.Frame = static_cast<GLshort>(ifr);
					}
					
					Model.SeqFrames.push_back(data);
				}
			}
			
			if (Model.SeqFrmPointers.empty())
				Model.SeqFrmPointers.push_back(0);
			Model.SeqFrmPointers.push_back(static_cast<GLushort>(Model.SeqFrames.size()));
		}
	}
}

bool LoadModel_Dim3(FileSpecifier& Spec, Model3D& Model, int WhichPass)
{
	if (WhichPass == LoadModelDim3_First)
	{
		// Clear everything
		Model.Clear();
		VertexBoneTags.clear();
		BoneOwnTags.clear();
		BoneIndices.clear();
		FrameTags.clear();
		Normals.clear();
	}
	
	bool parse_error = false;
	try {
		InfoTree fileroot = InfoTree::load_xml(Spec);
		BOOST_FOREACH(InfoTree root, fileroot.children_named("Model"))
		{
			parse_dim3(root, Model);
		}
	} catch (InfoTree::parse_error ex) {
		logError("Error parsing Dim3 file (%s): %s", Spec.GetPath(), ex.what());
		parse_error = true;
	} catch (InfoTree::path_error ep) {
		logError("Path error parsing Dim3 file (%s): %s", Spec.GetPath(), ep.what());
		parse_error = true;
	} catch (InfoTree::data_error ed) {
		logError("Data error parsing Dim3 file (%s): %s", Spec.GetPath(), ed.what());
		parse_error = true;
	} catch (InfoTree::unexpected_error ee) {
		logError("Unexpected error parsing Dim3 file (%s): %s", Spec.GetPath(), ee.what());
		parse_error = true;
	}
	if (parse_error) return false;
	
	// Set these up now
	if (Model.InverseVSIndices.empty()) Model.BuildInverseVSIndices();
	
	if (!Normals.empty())
	{
		Model.NormSources.resize(3*Model.VtxSrcIndices.size());
		Model.Normals.resize(Model.NormSources.size());
		for (unsigned k=0; k<Model.VtxSources.size(); k++)
		{
			GLfloat *Norm = &Normals[3*k];
			GLushort P0 = Model.InvVSIPointers[k];
			GLushort P1 = Model.InvVSIPointers[k+1];
			for (int p=P0; p<P1; p++)
			{
				GLfloat *DestNorm = &Model.NormSources[3*Model.InverseVSIndices[p]];
				for (int c=0; c<3; c++)
					DestNorm[c] = Norm[c];
			}
		}
		// All done with them
		
		Normals.clear();
	}
	
	// First, find the neutral-position vertices
	Model.FindPositions_Neutral(false);
	
	// Work out the sorted order for the bones; be sure not to repeat this if already done.
	if (BoneIndices.empty() && !Model.Bones.empty())
	{
		size_t NumBones = Model.Bones.size();
		BoneIndices.resize(NumBones);
		fill(BoneIndices.begin(),BoneIndices.end(),(size_t)UNONE);	// No bones listed -- yet
		vector<Model3D_Bone> SortedBones(NumBones);
		vector<size_t> BoneStack(NumBones);
		vector<bool> BonesUsed(NumBones);
		fill(BonesUsed.begin(),BonesUsed.end(),false);
		
		// Add the bones, one by one;
		// the bone stack's height is originally zero
		int StackTop = -1;
		for (vector<size_t>::value_type ib=0; ib<NumBones; ib++)
		{		
			// Scan down the bone stack to find a bone that's the parent of some unlisted bone;
			vector<size_t>::value_type ibsrch = 
				static_cast<vector<size_t>::value_type>(NumBones);	// "Bone not found" value
			int ibstck = -1;		// Empty stack
			for (ibstck=StackTop; ibstck>=0; ibstck--)
			{
				// Note: the bone stack is indexed relative to the original,
				// as is the bones-used list
				char *StackBoneTag = BoneOwnTags[BoneStack[ibstck]].Tag0;
				for (ibsrch=0; ibsrch<NumBones; ibsrch++)
				{
					if (BonesUsed[ibsrch]) continue;
					char *ParentTag = BoneOwnTags[ibsrch].Tag1;
					if (strncmp(ParentTag,StackBoneTag,BoneTagSize)==0)
						break;
				}
				// If a bone was found, then readjust the stack size appropriately and quit.
				if (ibsrch < NumBones)
				{
					if (ibstck < StackTop)
					{
						// Be sure to get the traversal push/pop straight.
						Model.Bones[BoneStack[ibstck+1]].Flags |= Model3D_Bone::Push;
						Model.Bones[ibsrch].Flags |= Model3D_Bone::Pop;
						StackTop = ibstck;
					}
					break;
				}
			}
			// If none was found, then the bone's parent is the assumed root bone.
			if (ibstck < 0)
			{
				for (ibsrch=0; ibsrch<NumBones; ibsrch++)
				{
					if (BonesUsed[ibsrch]) continue;
					
					// Check if the parent is not one of the bones
					char *ParentTag = BoneOwnTags[ibsrch].Tag1;
					size_t ibsx;
					for (ibsx=0; ibsx<NumBones; ibsx++)
					{
						if (strncmp(ParentTag,BoneOwnTags[ibsx].Tag0,BoneTagSize)==0)
							break;
					}
					// If a match was not found, then quit searching
					if (ibsx >= NumBones) break;
				}
				
				// Not sure how to handle this sort of error;
				// it could be produced by circular bone references:
				// B1 -> B2 -> B3 -> ... -> B1
				assert(ibsrch < NumBones);
				
				// Be sure to get the traversal push/pop straight.
				if (StackTop >= 0)
				{
					Model.Bones[BoneStack[0]].Flags |= Model3D_Bone::Push;
					Model.Bones[ibsrch].Flags |= Model3D_Bone::Pop;
					StackTop = -1;
				}
			}
			
			// Add the bone to the stack
			BoneStack[++StackTop] = ibsrch;
			
			// Don't look for it anymore
			BonesUsed[ibsrch] = true;
			
			// Index for remapping
			BoneIndices[ibsrch] = ib;
		}
		
		// Reorder the bones
		for (size_t ib=0; ib<NumBones; ib++)
			SortedBones[BoneIndices[ib]] = Model.Bones[ib];
		
		// Put them back into the model in one step
		Model.Bones.swap(SortedBones);
		
		// Find the vertex bone indices; this assumes that the vertices have already been read in.
		for (unsigned iv=0; iv<Model.VtxSources.size(); iv++)
		{
			Model3D_VertexSource& VS = Model.VtxSources[iv];
			size_t ibsx;
			char *Tag;
			
			Tag = VertexBoneTags[iv].Tag0;
			for (ibsx=0; ibsx<NumBones; ibsx++)
			{
				if (strncmp(Tag,BoneOwnTags[ibsx].Tag0,BoneTagSize)==0)
				break;
			}
			VS.Bone0 = ibsx < NumBones ? BoneIndices[ibsx] : static_cast<GLshort>(NONE);
			
			Tag = VertexBoneTags[iv].Tag1;
			for (ibsx=0; ibsx<NumBones; ibsx++)
			{
				if (strncmp(Tag,BoneOwnTags[ibsx].Tag0,BoneTagSize)==0)
				break;
			}
			VS.Bone1 = ibsx < NumBones ? BoneIndices[ibsx] : static_cast<GLshort>(NONE);
		}
	}
		
	return (!Model.Positions.empty() && !Model.VertIndices.empty());
}


// HAVE_OPENGL
#endif
