/*
 * rasonly.c - version 2
 *	Demonstrates the use of OpenGL for rasterization-only, with
 *	perspective-correction
 *
 * Michael I. Gold <gold@berkelium.com>
 * NVIDIA Corporation, March 1998
 *
 * Since current low-end 3D accelerators support only rasterization in
 * hardware, a number of developers have expressed interested in using
 * OpenGL as an interface to rasterization hardware while retaining
 * control of transformations and lighting in the application code.
 * Many OpenGL implementations detect and optimize for identity xforms,
 * so this approach is entirely reasonable.
 *
 * Setting up rasterization-only is fairly straightforward.  The projection
 * and modelview matrices are set up identity, and OpenGL performs only
 * clipping and the viewport transformation.  If the application performs
 * its own clipping, OpenGL will still perform clip testing (which is fast)
 * but will avoid clipping, which you can probably do more efficiently.
 *
 *	glMatrixMode(GL_PROJECTION);
 *	glLoadIdentity();
 *	glMatrixMode(GL_MODELVIEW);
 *	glLoadIdentity();
 *	glViewport(0, 0, width, height);
 *
 * where (width, height) represent the window dimensions.
 *
 * Now transformed geometry may be specified directly through the standard
 * interfaces (e.g. glVertex4fv()).  To ensure that perspective correction
 * occurs, the homogeneous w coordinate must be specified.
 *
 * version 2, March 1998 - It turns out that some hardware will perform
 * perspective correct shading and depth buffering if the vertex W is
 * specified.  Version 1 was written with the assumption that only the
 * texture cordinates needed perspective correction.  This is a significant
 * change in that the implementation must now be allowed to perform the
 * perspective divide and viewport transformation.  Ultimately this is
 * more efficient anyway, since the divide can overlap the clip testing,
 * less work is done by the application, and the projective transform is
 * truly identity.
 *
 * version 1, February 1997 - Created
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <assert.h>

GLboolean motion = GL_TRUE;

/* Matrices */
GLfloat rot = 0.0f;
GLfloat speed = 0.5;
GLfloat ModelView[16];
GLfloat Projection[16];

/* Sample geometry */
GLfloat quadV[][4] = {
    { -1.0f, 0.0f, -1.0f, 1.0f },
    {  1.0f, 0.0f, -1.0f, 1.0f },
    {  1.0f, 0.5f, -0.2f, 1.0f },
    { -1.0f, 0.5f, -0.2f, 1.0f },
};

GLfloat quadC[][3] = {
    { 1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f },
};

GLfloat quadT[][2] = {
    { 0.0f, 0.0f },
    { 0.0f, 1.0f },
    { 1.0f, 1.0f },
    { 1.0f, 0.0f },
};

/*********************************************************************
 * Utility functions
 */

int texWidth = 128;
int texHeight = 128;

/* Create and download the application texture map */
static void
setCheckedTexture(void)
{
    int texSize;
    void *textureBuf;
    GLubyte *p;
    int i,j;

    /* malloc for rgba as worst case */
    texSize = texWidth*texHeight*4;

    textureBuf = malloc(texSize);
    if (NULL == textureBuf) return;

    p = (GLubyte *)textureBuf;
    for (i=0; i < texWidth; i++) {
	for (j=0; j < texHeight; j++) {
	    if ((i ^ j) & 8) {
		p[0] = 0xff; p[1] = 0xff; p[2] = 0xff; p[3] = 0xff;
	    } else {
		p[0] = 0x08; p[1] = 0x08; p[2] = 0x08; p[3] = 0xff;
	    }
	    p += 4;
	}
    }
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, texWidth, texHeight, 
		 GL_RGBA, GL_UNSIGNED_BYTE, textureBuf);
    free(textureBuf);
}

/* Perform one transform operation */
static void
Transform(GLfloat *matrix, GLfloat *in, GLfloat *out)
{
    int ii;

    for (ii=0; ii<4; ii++) {
	out[ii] = 
	    in[0] * matrix[0*4+ii] +
	    in[1] * matrix[1*4+ii] +
	    in[2] * matrix[2*4+ii] +
	    in[3] * matrix[3*4+ii];
    }
}

/* Transform a vertex from object coordinates to window coordinates.
 * Lighting is left as an exercise for the reader.
 */
static void
DoTransform(GLfloat *in, GLfloat *out)
{
    GLfloat tmp[4];

    /* Modelview xform */
    Transform(ModelView, in, tmp);

    /* We now have eye coordinates. Lighting calculation goes here! */

    /* Projection xform */
    Transform(Projection, tmp, out);
}

/*********************************************************************
 * Application code begins here
 */

/* For the sake of brevity, I'm using OpenGL to compute my matrices. */
void UpdateModelView(void)
{
    /* Calculate projection matrix */
    glPushMatrix();
    glLoadIdentity();
    gluLookAt(0.0f, 1.0f, -4.0f,
	      0.0f, 0.0f, 0.0f,
	      0.0f, 1.0f, 0.0f);
    glRotatef(rot, 0.0f, 1.0f, 0.0f);
    /* Retrieve the matrix */
    glGetFloatv(GL_MODELVIEW_MATRIX, ModelView);
    glPopMatrix();
}

void UpdateProjection(void)
{
    /* Calculate projection matrix */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(45.0f, 1.0f, 1.0f, 100.0f);
    /* Retrieve the matrix */
    glGetFloatv(GL_PROJECTION_MATRIX, Projection);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Init()
{
    glClearColor(0.2f, 0.2f, 0.6f, 1.0f);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    setCheckedTexture();

    /* Load identity matrices to bypass OpenGL's transformation code */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Calculate current viewing and projection matrices */
    UpdateProjection();
    UpdateModelView();
}

void Redraw(void)
{
    GLfloat tmp[4];
    int ii;

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glBegin(GL_QUADS);

    for (ii = 0; ii < 4; ii++) {

	/* Transform a vertex from object to window coordinates. */
	DoTransform(quadV[ii], tmp);

        /* You may prefer to do your own clipping, if you think you can
         * do a better job.  OpenGL will still perform clip testing, but
         * won't re-clip if you do it here.
         */

	/* Ideally the colors will be computed by the lighting equation,
	 * but I've hard-coded values for this example.
	 */
	glColor3fv(quadC[ii]);
	glTexCoord2fv(quadT[ii]);
	glVertex4fv(tmp);
    }

    glEnd();

    glutSwapBuffers();
}

void Motion(void)
{
    rot += speed;
    if (rot >= 360.0f) rot -= 360.0f;
    UpdateModelView();
    Redraw();
}

void Key(unsigned char key, int x, int y)
{
    switch (key) {
    case 27:
	exit(0);
    case 'm':
	motion = !motion;
	glutIdleFunc(motion ? Motion : NULL);
	break;
    case '+':
    case '=':
        speed *= 2;
        break;
    case '-':
    case '_':
        speed /= 2;
        break;
        break;
    }
}

void Button(int button, int state, int x, int y)
{
    switch (button) {
    case GLUT_LEFT_BUTTON:
	if (state == GLUT_DOWN) {
	    rot -= 5.0f;
	    UpdateModelView();
	    Redraw();
	}
	break;
    case GLUT_RIGHT_BUTTON:
	if (state == GLUT_DOWN) {
	    rot += 5.0f;
	    UpdateModelView();
	    Redraw();
	}
	break;
    }
}

void Reshape(int width, int height)
{
    glViewport(0, 0, width, height);
}

int
#ifdef WIN32
__cdecl
#endif
main(int argc, char *argv[])
{
    char *t;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowSize(400, 400);
    glutCreateWindow((t=strrchr(argv[0], '\\')) != NULL ? t+1 : argv[0]);

    Init();

    glutDisplayFunc(Redraw);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Key);
    glutMouseFunc(Button);
    glutIdleFunc(Motion);

    glutMainLoop();

    return 0;
}
