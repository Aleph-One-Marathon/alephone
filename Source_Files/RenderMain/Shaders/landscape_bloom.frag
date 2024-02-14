R"(

precision highp float;
uniform mat4 MS_ModelViewProjectionMatrix;
uniform mat4 MS_ModelViewMatrix;
uniform sampler2D texture0;
uniform float usefog;
uniform float fogMix;

varying vec4 fSxOxSyOy; 
varying vec4 fBsBtFlSl; 
varying vec4 fPuWoDeGl; 
varying vec4 fClipPlane0;   
varying vec4 fClipPlane1;   
varying vec4 fClipPlane5;   

uniform float yaw;
uniform float pitch;
uniform float bloomScale;
varying highp vec4 fogColor; 
varying vec3 relDir;
varying vec4 vertexColor;
const float zoom = 1.205;
const float pitch_adjust = 0.955;
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
    vec3 relv  = normalize(relDir);
    float x = relv.x / (relv.z * zoom) + atan(facev.x, facev.y);
    float y = relv.y / (relv.z * zoom) - (facev.z * pitch_adjust);
    vec4 color = texture2D(texture0, vec2(offsetx - x * scalex, offsety - y * scaley));
    float intensity = clamp(bloomScale, 0.0, 1.0);
    if (usefog > 0.0) {
        intensity = 0.0;
    }
#ifdef GAMMA_CORRECTED_BLENDING
    //intensity = intensity * intensity;
    color.rgb = (color.rgb - 0.01) * 1.01;
#else
    color.rgb = (color.rgb - 0.1) * 1.11;
#endif
    gl_FragColor = vec4(color.rgb * intensity * (1.0 - fogMix), 1.0);
}


)"
