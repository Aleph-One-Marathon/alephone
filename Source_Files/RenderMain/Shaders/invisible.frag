R"(

precision highp float;
uniform sampler2D texture0;
uniform float visibility;
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
    vec3 intensity = vec3(0.0, 0.0, 0.0);
    float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);
    gl_FragColor = vec4(mix(fogColor.rgb, intensity, fogFactor), vertexColor.a * color.a * visibility);
}

)"
