/*
 * MultiTex.c
 *
 * Written by Nick Triantos
 *
 * Copyright (c) 1998, NVIDIA Corporation.  All rights reserved
 *
 * Draw a single multitextured quad
 */

#include <GL/glut.h>
#include "glext.h"
#include <stdio.h>

#define TEX0_SIZE               128
#define TEX0_COMPONENTS         3
#define TEX1_SIZE               256
#define TEX1_COMPONENTS         4
#define QUAD_SIZE               400.0f
#define QUAD_X_OFFSET           100.0f
#define QUAD_Y_OFFSET           10.0f
#define WINDOW_TITLE            "Multitexture test"
#define LINE_HEIGHT             15
#define DEG2RAD                 0.017453292519943
#define PI                      3.141592653589793

typedef struct Vertex {
    float x, y;
    float u0, v0;
    float u1, v1;
} Vertex;

void InitVertices(void);
void Display(void);

unsigned char tex0[TEX0_COMPONENTS*TEX0_SIZE*TEX0_SIZE];
unsigned char tex1[TEX1_COMPONENTS*TEX1_SIZE*TEX1_SIZE];

void   *font                        = GLUT_BITMAP_9_BY_15;
int    windowWidth                  = 600;
int    windowHeight                 = 600;
float  angle                        = 135.0f;
int    multitexturing               = 1;
int    panTexture0                  = 0;
Vertex vtx[4];
GLuint texNames[2];

PFNGLACTIVETEXTUREARBPROC          glActiveTextureARB          = NULL;
PFNGLMULTITEXCOORD2FARBPROC        glMultiTexCoord2fARB        = NULL;


/* ---------- Build a texture of the specified size ------------------------ */

void
MakeTextures()
{
    int i, j;
    unsigned char *lpTex;

    glGenTextures(2, texNames);

    /* Texture 0 is a simple checkerboard */

    glBindTexture(GL_TEXTURE_2D, texNames[0]);
    lpTex = tex0;
    for (j=0; j<TEX0_SIZE; j++) {
        for (i=0; i<TEX0_SIZE; i++) {
            if ((i ^ j) & 0x20) {
                lpTex[0] = lpTex[1] = lpTex[2] = 0x00;
            } else {
                lpTex[0] = lpTex[1] = lpTex[2] = 0xff;
            }
            lpTex += TEX0_COMPONENTS;
        }
    }

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 (TEX0_COMPONENTS==3 ? GL_RGB8 : GL_RGBA8),
                 TEX0_SIZE, TEX0_SIZE,
                 0,
                 (TEX0_COMPONENTS==3 ? GL_RGB : GL_RGBA),
                 GL_UNSIGNED_BYTE,
                 tex0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);


    /* Texture 1 is a groovy multicolor pattern */

    glBindTexture(GL_TEXTURE_2D, texNames[1]);
    {
        float fXCenter, fYCenter;
        float fA;
        float fXD, fYD, fDist;

        fXCenter = TEX1_SIZE/2.0f;
        fYCenter = TEX1_SIZE/2.0f;
        lpTex = tex1;
        for (j=0; j<TEX1_SIZE; j++) {
            fYD = fYCenter - ((float)j);
            for (i=0; i<TEX1_SIZE; i++) {
                fXD = fXCenter - ((float)i);
                fDist = (fXD*fXD + fYD*fYD) / (fXCenter*fXCenter);
                if (fDist > 1.0f) fDist = 1.0f;
                fA = 1.0f - fDist;

                if ((j < (TEX1_SIZE>>1)) && (i < (TEX1_SIZE>>1))) {
                    lpTex[0] = 0xff;
                    lpTex[1] = 0x00;
                    lpTex[2] = 0x00;
                } else if (j < (TEX1_SIZE>>1)) {
                    lpTex[0] = 0x00;
                    lpTex[1] = 0xff;
                    lpTex[2] = 0x00;
                } else if (i < (TEX1_SIZE>>1)) {
                    lpTex[0] = 0x00;
                    lpTex[1] = 0x00;
                    lpTex[2] = 0xff;
                } else {
                    lpTex[0] = 0xff;
                    lpTex[1] = 0xff;
                    lpTex[2] = 0x00;
                }
                lpTex[3] = fA * 255.0f;
                lpTex += TEX1_COMPONENTS;
            }
        }
    }

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 (TEX1_COMPONENTS==3 ? GL_RGB8 : GL_RGBA8),
                 TEX1_SIZE, TEX1_SIZE,
                 0,
                 (TEX1_COMPONENTS==3 ? GL_RGB : GL_RGBA),
                 GL_UNSIGNED_BYTE,
                 tex1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}


/* ---------- Misc --------------------------------------------------------- */

void
Reshape(int w, int h)
{
    glViewport(0, 0, w, h);       /* Establish viewing area to cover entire window. */
    glMatrixMode(GL_PROJECTION);  /* Start modifying the projection matrix. */
    glLoadIdentity();             /* Reset project matrix. */
    glOrtho(0, w, 0, h, -1, 1);   /* Map abstract coords directly to window coords. */
    glScalef(1, -1, 1);           /* Invert Y axis so increasing Y goes down. */
    glTranslatef(0, -h, 0);       /* Shift origin up to upper-left corner. */
}


void
Idle(void)
{
    angle += 2.0f;

    Display();
}


void
GetGLProcs()
{
    char *strExtensions;

    strExtensions = (char*)glGetString(GL_EXTENSIONS);
    if (strstr(strExtensions, "GL_ARB_multitexture") == 0) {
        OutputDebugString("GL_ARB_multitexture NOT found.\n");
        multitexturing = 0;
        return;
    }

    glActiveTextureARB         = (void*)wglGetProcAddress("glActiveTextureARB");
    glMultiTexCoord2fARB       = (void*)wglGetProcAddress("glMultiTexCoord2fARB");
}


/* ---------- Draw the scene ----------------------------------------------- */

void
OutputString(int x, int y, char *string)
{
    int len, i;

    glRasterPos2f(x, y);
    len = (int)strlen(string);
    for (i=0; i<len; i++) {
        glutBitmapCharacter(font, string[i]);
    }
}


void
OutputShadowedString(int x, int y, char *string)
{
    glColor3f(0.0, 0.0, 0.0);
    OutputString(x+1, y+1, string);
    glColor3f(1.0, 1.0, 0.0);
    OutputString(x, y, string);
}


void
DisplayInfo()
{
    char str[256];
    int  y;

    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, windowHeight, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    y = windowHeight - 120;
    OutputShadowedString(20, y, "Keys:");
    y += LINE_HEIGHT;
    OutputShadowedString(20, y, "'P' to pan texture 0");
    y += LINE_HEIGHT;
    OutputShadowedString(20, y, "'R' to rotate texture 0");
    y += LINE_HEIGHT;
    OutputShadowedString(20, y, "TAB to toggle pan/rotate");
    y += 2 * LINE_HEIGHT;
    sprintf(str, "SPACE to toggle multitexture (currently %s)", multitexturing ? "ON" : "OFF");
    OutputShadowedString(20, y, str);
    y += LINE_HEIGHT;
    if (glActiveTextureARB == NULL) {
        OutputShadowedString(20, y, "No GL_ARB_multitexture");
        y += LINE_HEIGHT;
    }
}


void
Display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (multitexturing) {
        (*glActiveTextureARB)(GL_TEXTURE0_ARB);
        glEnable(GL_TEXTURE_2D);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        if (panTexture0) {
            /* Pan texture0, to show off how cool texture matrices are. */
            glTranslatef(-2.0*angle*0.002, -angle*0.002, 0);
        } else {
            /* Rotate texture0 in the opposite direction of texture1 */
            glTranslatef(0.5f, 0.5f, 0.0f);
            glRotatef(-angle, 0.0f, 0.0f, 1.0f);
            glTranslatef(-0.5f, -0.5f, 0.0f);
        }
        glMatrixMode(GL_MODELVIEW);

        (*glActiveTextureARB)(GL_TEXTURE1_ARB);
        glEnable(GL_TEXTURE_2D);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslatef(0.5f, 0.5f, 0.0f);
        glRotatef(angle, 0.0f, 0.0f, 1.0f);
        glTranslatef(-0.5f, -0.5f, 0.0f);
        glMatrixMode(GL_MODELVIEW);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        glBegin(GL_QUADS);
        {
            glColor3f(1.0, 1.0, 1.0);  /* white */

            (*glMultiTexCoord2fARB)(GL_TEXTURE0_ARB, vtx[0].u0, vtx[0].v0);
            (*glMultiTexCoord2fARB)(GL_TEXTURE1_ARB, vtx[0].u1, vtx[0].v1);
            glVertex2f(vtx[0].x, vtx[0].y);

            (*glMultiTexCoord2fARB)(GL_TEXTURE0_ARB, vtx[1].u0, vtx[1].v0);
            (*glMultiTexCoord2fARB)(GL_TEXTURE1_ARB, vtx[1].u1, vtx[1].v1);
            glVertex2f(vtx[1].x, vtx[1].y);

            (*glMultiTexCoord2fARB)(GL_TEXTURE0_ARB, vtx[2].u0, vtx[2].v0);
            (*glMultiTexCoord2fARB)(GL_TEXTURE1_ARB, vtx[2].u1, vtx[2].v1);
            glVertex2f(vtx[2].x, vtx[2].y);

            (*glMultiTexCoord2fARB)(GL_TEXTURE0_ARB, vtx[3].u0, vtx[3].v0);
            (*glMultiTexCoord2fARB)(GL_TEXTURE1_ARB, vtx[3].u1, vtx[3].v1);
            glVertex2f(vtx[3].x, vtx[3].y);

        }
        glEnd();

        (*glActiveTextureARB)(GL_TEXTURE1_ARB);
        glDisable(GL_TEXTURE_2D);
        (*glActiveTextureARB)(GL_TEXTURE0_ARB);
        glDisable(GL_TEXTURE_2D);
    } else {
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);

        /* Quad 0 */
        glBindTexture(GL_TEXTURE_2D, texNames[0]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        if (panTexture0) {
            /* Pan texture0, to show off how cool texture matrices are. */
            glTranslatef(-2.0*angle*0.002, -angle*0.002, 0);
        } else {
            /* Rotate texture0 in the opposite direction of texture1 */
            glTranslatef(0.5f, 0.5f, 0.0f);
            glRotatef(-angle, 0.0f, 0.0f, 1.0f);
            glTranslatef(-0.5f, -0.5f, 0.0f);
        }
        glMatrixMode(GL_MODELVIEW);

        glBegin(GL_QUADS);
        {
            glColor3f(1.0, 1.0, 1.0);  /* white */

            glTexCoord2f(vtx[0].u0, vtx[0].v0);
            glVertex2f(vtx[0].x, vtx[0].y);

            glTexCoord2f(vtx[1].u0, vtx[1].v0);
            glVertex2f(vtx[1].x, vtx[1].y);

            glTexCoord2f(vtx[2].u0, vtx[2].v0);
            glVertex2f(vtx[2].x, vtx[2].y);

            glTexCoord2f(vtx[3].u0, vtx[3].v0);
            glVertex2f(vtx[3].x, vtx[3].y);

        }
        glEnd();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        /* Quad 1 */
        glBindTexture(GL_TEXTURE_2D, texNames[1]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslatef(0.5f, 0.5f, 0.0f);
        glRotatef(angle, 0.0f, 0.0f, 1.0f);
        glTranslatef(-0.5f, -0.5f, 0.0f);
        glMatrixMode(GL_MODELVIEW);

        glBegin(GL_QUADS);
        {
            glColor3f(1.0, 1.0, 1.0);  /* white */

            glTexCoord2f(vtx[0].u1, vtx[0].v1);
            glVertex2f(vtx[0].x, vtx[0].y);

            glTexCoord2f(vtx[1].u1, vtx[1].v1);
            glVertex2f(vtx[1].x, vtx[1].y);

            glTexCoord2f(vtx[2].u1, vtx[2].v1);
            glVertex2f(vtx[2].x, vtx[2].y);

            glTexCoord2f(vtx[3].u1, vtx[3].v1);
            glVertex2f(vtx[3].x, vtx[3].y);

        }
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }

    DisplayInfo();

    glutSwapBuffers();
}


void
Init()
{
    if (multitexturing) {
        (*glActiveTextureARB)(GL_TEXTURE0_ARB);
        glBindTexture(GL_TEXTURE_2D, texNames[0]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        (*glActiveTextureARB)(GL_TEXTURE1_ARB);
        glBindTexture(GL_TEXTURE_2D, texNames[1]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
}


/* ---------- Events ------------------------------------------------------- */

void
Keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case ' ':
        if (!glActiveTextureARB)
            break;
        multitexturing = !multitexturing;
        break;
    case 'q':
    case 'Q':
        exit(0);
    case 'p':
    case 'P':
        panTexture0 = TRUE;
        break;
    case 'r':
    case 'R':
        panTexture0 = FALSE;
        break;
    case 0x9:
        panTexture0 = !panTexture0;
        break;
    default:
        return;
    }

    Init();
}


/* ---------- Initialization ----------------------------------------------- */

void
InitVertices()
{
    int i;
    float fXN, fYN;

    for (i=0; i<4; i++) {
        fXN         = (i < 2) ? 0.0f : 1.0f;
        fYN         = ((i+1) % 4 < 2) ? 0.0f : 1.0f;
        vtx[i].x    = fXN * QUAD_SIZE + QUAD_X_OFFSET;
        vtx[i].y    = fYN * QUAD_SIZE + QUAD_Y_OFFSET;
        vtx[i].u0   = fXN;
        vtx[i].v0   = fYN;
        vtx[i].u1   = fXN;
        vtx[i].v1   = fYN;
    }
}

int
main(int argc, char **argv)
{
    char str[256];
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow(WINDOW_TITLE);
    sprintf(str, WINDOW_TITLE ": Renderer: %s", glGetString(GL_RENDERER));
    glutSetWindowTitle(str);

    GetGLProcs();
    MakeTextures();
    InitVertices();
    Init();

    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutIdleFunc(Idle);
    glutMainLoop();
    return 0;             /* ANSI C requires main to return int. */
}

