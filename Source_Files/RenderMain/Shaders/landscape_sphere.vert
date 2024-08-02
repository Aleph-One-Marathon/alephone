R"(

uniform mat4 landscapeInverseMatrix;
varying vec3 relDir;
varying vec4 vertexColor;
varying float cosPitch;
varying float sinPitch;
varying float cosYaw;
varying float sinYaw;
uniform float yaw;
uniform float pitch;

void main(void) {
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
#ifndef DISABLE_CLIP_VERTEX
	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
#endif
	relDir = (gl_ModelViewMatrix * gl_Vertex).xyz;
	vertexColor = gl_Color;
	
	cosYaw = cos(yaw);
	cosPitch = cos(-pitch);
	sinYaw = sin(yaw);
	sinPitch = sin(-pitch);
}

)"

