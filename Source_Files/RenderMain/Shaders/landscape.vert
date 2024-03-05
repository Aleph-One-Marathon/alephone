R"(

uniform mat4 MS_ModelViewProjectionMatrix;
uniform mat4 MS_ModelViewMatrix;
uniform mat4 landscapeInverseMatrix; //What is this for?
uniform vec4 uFogColor;

uniform vec4 clipPlane0;
uniform vec4 clipPlane1;
uniform vec4 clipPlane5;

attribute vec4 vPosition;

attribute vec4 vColor;

varying vec3 relDir;
varying vec4 vertexColor;
varying vec4 vPosition_eyespace;

void main(void) {
	gl_Position = MS_ModelViewProjectionMatrix * vPosition;
	vPosition_eyespace = MS_ModelViewMatrix * vPosition;
	relDir = (MS_ModelViewMatrix * vPosition).xyz;
	vertexColor = vColor;
}

)"

