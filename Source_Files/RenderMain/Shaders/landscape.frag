R"(

precision highp float;
varying highp vec4 fogColor; 
uniform sampler2D texture0;
uniform float usefog;

varying vec4 fSxOxSyOy; 
varying vec4 fBsBtFlSl; 
varying vec4 fPuWoDeGl; 
varying vec4 fClipPlane0;   
varying vec4 fClipPlane1;   
varying vec4 fClipPlane5;   


uniform float yaw;
uniform float pitch;
varying vec3 relDir;
varying vec4 vertexColor;
const float zoom = 1.2;
const float pitch_adjust = 0.96;
varying vec4 vPosition_eyespace;

void main(void) {
	if( dot( vPosition_eyespace, fClipPlane0) < 0.0 ) {discard;}
	if( dot( vPosition_eyespace, fClipPlane1) < 0.0 ) {discard;}
	if( dot( vPosition_eyespace, fClipPlane5) < 0.0 ) {discard;}
	
	float scalex = fSxOxSyOy.x;
	float scaley = fSxOxSyOy.z;
	float offsetx = fSxOxSyOy.y;
	float offsety = fSxOxSyOy.w;
	
	vec3 facev = vec3(cos(yaw), sin(yaw), sin(pitch));
	vec3 relv  = (relDir);
	float x = relv.x / (relv.z * zoom) + atan(facev.x, facev.y);
	float y = relv.y / (relv.z * zoom) - (facev.z * pitch_adjust);
	vec4 color = texture2D(texture0, vec2(offsetx - x * scalex, offsety - y * scaley));
	vec3 intensity = color.rgb;
	if (usefog > 0.0) {
		intensity = fogColor.rgb;
	}
	gl_FragColor = vec4(intensity, 1.0);
}

)"
