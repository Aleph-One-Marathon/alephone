R"(

precision highp float;

uniform sampler2D texture0;
uniform float fogMode;
uniform float fogStart;
uniform float fogEnd;

varying highp vec4 fogColor;
varying vec2 textureUV;
varying vec4 fSxOxSyOy;
varying vec4 fBsBtFlSl;
varying vec4 fPuWoDeGl;
varying vec4 fClipPlane0;
varying vec4 fClipPlane1;
varying vec4 fClipPlane5;
varying vec3 viewXY;
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
	bool unwantedFragment = false;
	if( dot( vPosition_eyespace, fClipPlane0) < 0.0 ) {discard;}
	if( dot( vPosition_eyespace, fClipPlane1) < 0.0 ) {discard;}
	if( dot( vPosition_eyespace, fClipPlane5) < 0.0 ) {discard;}

	float pulsate = fPuWoDeGl.x;
	float wobble = fPuWoDeGl.y;
	
	// infravision sees right through fog, and see textures at full intensity
	vec3 texCoords = vec3(textureUV.xy, 0.0);
	vec3 normXY = normalize(viewXY);
	texCoords += vec3(normXY.y * -pulsate, normXY.x * pulsate, 0.0);
	texCoords += vec3(normXY.y * -wobble * texCoords.y, wobble * texCoords.y, 0.0);
	vec4 color = texture2D(texture0, texCoords.xy);
	float avg = (color.r + color.g + color.b) / 3.0;
	float fogFactor = getFogFactor(length(viewDir));
	gl_FragColor = vec4(mix(fogColor.rgb, vertexColor.rgb * avg, fogFactor), vertexColor.a * color.a);
}

)"
