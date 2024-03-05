R"(

precision highp float;
uniform sampler2D texture0;
uniform float glow;
uniform float flare;
uniform float selfLuminosity;
uniform float fogMode;
uniform float fogStart;
uniform float fogEnd;
uniform vec4 clipPlane0;
uniform vec4 clipPlane1;
uniform vec4 clipPlane5;
uniform float logicalWidth;
uniform float logicalHeight;
uniform float pixelWidth;
uniform float pixelHeight;
varying highp vec4 fogColor; 
varying highp vec2 textureUV; 
varying vec3 viewDir;
varying vec4 vertexColor;
varying float FDxLOG2E;
varying float classicDepth;
varying vec4 vPosition_eyespace;

float getFogFactor(float distance) {
	if (fogMode == 0.0) {
		return clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);
	} else if (fogMode == 1.0) {
		return clamp(exp(-fogColor.a * distance), 0.0, 1.0);
	} else if (fogMode == 2.0) {
		return clamp(exp(-fogColor.a * fogColor.a * distance * distance), 0.0, 1.0);
	} else {
		return 1.0;
	}
}

void main (void) {
  if( dot( vPosition_eyespace, clipPlane0) < 0.0 ) {discard;}
  if( dot( vPosition_eyespace, clipPlane1) < 0.0 ) {discard;}
  if( dot( vPosition_eyespace, clipPlane5) < 0.0 ) {discard;}
    float mlFactor = clamp(selfLuminosity + flare - classicDepth, 0.0, 1.0);
    // more realistic: replace classicDepth with (length(viewDir)/8192.0)
    vec3 intensity;
    if (vertexColor.r > mlFactor) {
        intensity = vertexColor.rgb + (mlFactor * 0.5); }
    else {
        intensity = (vertexColor.rgb * 0.5) + mlFactor; }
    intensity = clamp(intensity, glow, 1.0);
#ifdef GAMMA_CORRECTED_BLENDING
    intensity = intensity * intensity; // approximation of pow(intensity, 2.2)
#endif
    vec4 color = texture2D(texture0, textureUV.xy);
	float fogFactor = getFogFactor(length(viewDir));
    gl_FragColor = vec4(mix(fogColor.rgb, color.rgb * intensity, fogFactor), color.a * vertexColor.a);
//        "    if ( gl_FragColor.a == 0.0 ) {discard;} //discard transparent fragments so they don't write on the depth buffer \n "
}

)"
