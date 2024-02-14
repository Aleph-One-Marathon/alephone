R"(

uniform mat4 MS_ModelViewProjectionMatrix;
uniform mat4 MS_ModelViewMatrix;
uniform mat4 landscapeInverseMatrix; //What is this for?
uniform vec4 uFogColor;

attribute vec4 vPosition;

attribute vec4 vColor;
attribute vec4 vClipPlane0;
attribute vec4 vClipPlane1;
attribute vec4 vClipPlane5;
attribute vec4 vSxOxSyOy;
attribute vec4 vBsBtFlSl;
attribute vec4 vPuWoDeGl;

varying vec4 fSxOxSyOy;
varying vec4 fBsBtFlSl;
varying vec4 fPuWoDeGl;
varying vec4 fClipPlane0;
varying vec4 fClipPlane1;
varying vec4 fClipPlane5;

varying vec4 fogColor;
varying vec3 relDir;
varying vec4 vertexColor;
varying vec4 vPosition_eyespace;

void main(void) {
	gl_Position = MS_ModelViewProjectionMatrix * vPosition;
	vPosition_eyespace = MS_ModelViewMatrix * vPosition;
	relDir = (MS_ModelViewMatrix * vPosition).xyz;
	vertexColor = vColor;
	fogColor = uFogColor;
	
	fSxOxSyOy = vSxOxSyOy;
	fBsBtFlSl = vBsBtFlSl;
	fPuWoDeGl = vPuWoDeGl;
	fClipPlane0 = vClipPlane0;
	fClipPlane1 = vClipPlane1;
	fClipPlane5 = vClipPlane5;
}

)"

