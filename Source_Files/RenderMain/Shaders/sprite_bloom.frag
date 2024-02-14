R"(

precision highp float;
uniform sampler2D texture0;
uniform float glow;
uniform float bloomScale;
uniform float bloomShift;
uniform vec4 clipPlane0;
uniform vec4 clipPlane1;
uniform vec4 clipPlane5;
uniform float fogMode;
uniform float fogStart;
uniform float fogEnd;
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
    vec4 color = texture2D(texture0, textureUV.xy);
    vec3 intensity = clamp(vertexColor.rgb, glow, 1.0);
    //intensity = intensity * clamp(2.0 - length(viewDir)/8192.0, 0.0, 1.0);
    intensity = clamp(intensity * bloomScale + bloomShift, 0.0, 1.0);
#ifdef GAMMA_CORRECTED_BLENDING
    intensity = intensity * intensity;  // approximation of pow(intensity, 2.2)
    color.rgb = (color.rgb - 0.06) * 1.02;
#else
  color.rgb = (color.rgb - 0.2) * 1.25;
#endif
	float fogFactor = getFogFactor(length(viewDir));
	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), color.rgb * intensity, fogFactor), vertexColor.a * color.a);
}

)"
