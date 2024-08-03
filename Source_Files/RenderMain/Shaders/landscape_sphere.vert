R"(

uniform mat4 MS_ModelViewProjectionMatrix;
uniform mat4 MS_ModelViewMatrix;
uniform vec4 clipPlane0;
uniform vec4 clipPlane1;
uniform vec4 clipPlane5;
uniform vec4 uFogColor;
uniform float yaw;
uniform float pitch;
uniform mat4 landscapeInverseMatrix;  //What is this for?

attribute vec4 vPosition;
attribute vec4 vColor;

varying vec4 vertexColor;
varying vec4 vPosition_eyespace;
varying vec3 relDir;
varying float cosPitch;
varying float sinPitch;
varying float cosYaw;
varying float sinYaw;

void main(void) {
	gl_Position = MS_ModelViewProjectionMatrix * vPosition;
	vPosition_eyespace = MS_ModelViewMatrix * vPosition;

	relDir = (MS_ModelViewMatrix * vPosition).xyz;

	cosYaw = cos(yaw);
	cosPitch = cos(-pitch);
	sinYaw = sin(yaw);
	sinPitch = sin(-pitch);
	
	vertexColor = vColor;
}

)"

