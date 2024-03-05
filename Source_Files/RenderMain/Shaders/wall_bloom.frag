R"(

precision highp float;
uniform sampler2D texture0;
uniform float fogMode;
uniform float fogStart;
uniform float fogEnd;
uniform vec4 clipPlane0;
uniform vec4 clipPlane1;
uniform vec4 clipPlane5;
uniform vec4 uFogColor;
uniform float pulsate;
uniform float wobble;
uniform float glow;
uniform float flare;
uniform float bloomScale;
uniform float bloomShift;

varying vec4 fogColor;
varying vec2 textureUV;
varying vec3 viewXY;
varying vec3 viewDir;
varying vec4 vertexColor;
varying float FDxLOG2E;
varying vec4 vPosition_eyespace;
varying vec3 eyespaceNormal;

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
	
    vec3 texCoords = vec3(textureUV.xy, 0.0);
    vec3 normXY = normalize(viewXY);
    texCoords += vec3(normXY.y * -pulsate, normXY.x * pulsate, 0.0);
    texCoords += vec3(normXY.y * -wobble * texCoords.y, wobble * texCoords.y, 0.0);
    vec4 color = texture2D(texture0, texCoords.xy);
    vec3 intensity = clamp(vertexColor.rgb, glow, 1.0);
    float diffuse = abs(dot(vec3(0.0, 0.0, 1.0), normalize(viewDir)));
    intensity = clamp(intensity * bloomScale + bloomShift, 0.0, 1.0);
#ifdef GAMMA_CORRECTED_BLENDING
    intensity = intensity * intensity; // approximation of pow(intensity, 2.2)
#endif
	float fogFactor = getFogFactor(length(viewDir));
    gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), color.rgb * intensity, fogFactor), color.a) * vertexColor;
}

)"
