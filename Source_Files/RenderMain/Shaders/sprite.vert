R"(

uniform float depth;
uniform float strictDepthMode;
varying vec3 viewDir;
varying vec4 vertexColor;
varying float classicDepth;
void main(void) {
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	classicDepth = gl_Position.z / 8192.0;
#ifndef DISABLE_CLIP_VERTEX
	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
#endif
	vec4 v = gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);
	viewDir = (gl_Vertex - v).xyz;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	vertexColor = gl_Color;
}

)"
