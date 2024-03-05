R"(

precision highp float;

uniform sampler2D texture0;
uniform float fogMode;
uniform float fogStart;
uniform float fogEnd;

uniform float pulsate;
uniform float wobble;
uniform vec4 clipPlane0;
uniform vec4 clipPlane1;
uniform vec4 clipPlane5;

varying vec4 fogColor;
varying vec2 textureUV;
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
	if( dot( vPosition_eyespace, clipPlane0) < 0.0 ) {discard;}
	if( dot( vPosition_eyespace, clipPlane1) < 0.0 ) {discard;}
	if( dot( vPosition_eyespace, clipPlane5) < 0.0 ) {discard;}
	
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
