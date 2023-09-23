R"(

uniform mat4 landscapeInverseMatrix;
varying vec3 relDir;
varying vec4 vertexColor;
void main(void) {
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#ifndef DISABLE_CLIP_VERTEX
	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
#endif
	relDir = (gl_ModelViewMatrix * gl_Vertex).xyz;
	vertexColor = gl_Color;
}

)"

