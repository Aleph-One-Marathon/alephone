R"(

uniform sampler2D texture0;
uniform float pulsate;
uniform float wobble;
uniform vec4 clipPlane0;
uniform vec4 clipPlane1;
uniform vec4 clipPlane5;
varying vec3 viewXY;
varying vec3 viewDir;
varying vec4 vertexColor;
varying float FDxLOG2E;
varying float classicDepth;
varying vec4 vPosition_eyespace;

void main (void) {
    bool unwantedFragment = false;
    if( dot( vPosition_eyespace, clipPlane0) < 0.0 ) {unwantedFragment = true;}
    if( dot( vPosition_eyespace, clipPlane1) < 0.0 ) {unwantedFragment = true;}
    if( dot( vPosition_eyespace, clipPlane5) < 0.0 ) {unwantedFragment = true;}

	// infravision sees right through fog, and see textures at full intensity
	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);
	vec3 normXY = normalize(viewXY);
	texCoords += vec3(normXY.y * -pulsate, normXY.x * pulsate, 0.0);
	texCoords += vec3(normXY.y * -wobble * texCoords.y, wobble * texCoords.y, 0.0);
	vec4 color = texture2D(texture0, texCoords.xy);
	float avg = (color.r + color.g + color.b) / 3.0;
	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);
	gl_FragColor = vec4(mix(gl_Fog.color.rgb, vertexColor.rgb * avg, fogFactor), vertexColor.a * color.a);
    if( unwantedFragment ) {gl_FragColor.a = 0.0;}
}

)"
