/*
 *  FSRenderer.h
 *  Created by Clemens Unterkofler on 1/20/09.
 *  for Aleph One
 *
 *  http://www.gnu.org/licenses/gpl.html
 */


#ifndef _FS_RASTERIZER__H
#define _FS_RASTERIZER__H

#include "cseries.h"
#include "map.h"
#include "FBO.h"
#include "RenderRasterize.h"
#include "Rasterizer_OGL.h"

typedef enum {
	kDiffuse,
	kGlow,
} RenderStep;

struct FSVertex {
	GLint loc[2];
	GLfloat tex[2];
};

struct FSPolygonIndex {
	GLushort offset, count;
};

/*
 * this data structure should hold static level geometry
 */
class FSRenderData {
private:
	std::vector<FSVertex> vertices;
	std::vector<GLushort> indices;
	std::vector<std::vector<FSPolygonIndex> > polys;
	GLuint buffers[2];
public:

	FSRenderData() {
		buffers[0] = buffers[1] = NULL;
	}

	void drawPoly(const GLuint& poly, const GLushort& side, const GLenum& mode) {
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffers[0]);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, buffers[1]);
		glDrawElements(mode, polys[poly][side].count, GL_UNSIGNED_SHORT, (GLvoid*)polys[poly][side].offset);
	}

	void debugDraw() {
		for(GLuint i = 0; i < polys.size(); ++i) {
			for(GLushort j = 0; i < polys[j].size(); ++j) {
				drawPoly(i, j, GL_LINE_STRIP);
			}
		}
	}

	void setupVBOs() {
		glGenBuffersARB(2, buffers);

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffers[0]);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, vertices.size() * sizeof(FSVertex), &vertices[0], GL_STATIC_DRAW_ARB);

		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, buffers[1]);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indices.size() * sizeof(GLushort), &indices[0], GL_STATIC_DRAW_ARB);

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

		// we won't need these anymore
		indices.clear();
		vertices.clear();
	}

	void destroy() {
		polys.clear();
		if(buffers[0]) {
			glDeleteBuffersARB(2, buffers);
		}
	}

	void loadLevel();
};

class FSRenderer : public RenderRasterizerClass, public Rasterizer_OGL_Class {

	FSRenderData renderData;
	Blur* blurLarge;
	Blur* blurSmall;

public:

	FSRenderer() : blurSmall(NULL), blurLarge(NULL), RenderRasterizerClass(), Rasterizer_OGL_Class() {}

	virtual void setupGL();
	virtual void render_node_floor_or_ceiling(
		  clipping_window_data *window, polygon_data *polygon, horizontal_surface_data *surface,
		  bool void_present, bool ceil, const RenderStep& renderStep);
	virtual void render_node_side(
		  clipping_window_data *window, vertical_surface_data *surface,
		  bool void_present, const RenderStep& renderStep);

	virtual void render_node_object(render_object_data *object, bool other_side_of_media, const view_data& view, const RenderStep& renderStep);

	virtual void render_tree();
	virtual void render_tree(const RenderStep&);

	virtual void render_viewer_sprite_layer(view_data *view);

	virtual void SetView(view_data& View);
	virtual void Begin();
	virtual void End();

	bool setupTexture(const shape_descriptor& texture, short transferMode, short transfer_phase, const RenderStep& renderStep);
};

void FlatTexture();

#endif