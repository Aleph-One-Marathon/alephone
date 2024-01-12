R"(

uniform sampler2D texture0;
uniform float pulsate;
uniform float wobble;
uniform float fogMode;
varying vec3 viewXY;
varying vec3 viewDir;
varying vec4 vertexColor;
varying float classicDepth;

float getFogFactor(float distance) {
	if (fogMode == 0.0) {
        return clamp((gl_Fog.end - distance) / (gl_Fog.end - gl_Fog.start), 0.0, 1.0);
    } else if (fogMode == 1.0) {
        return clamp(exp(-gl_Fog.density * distance), 0.0, 1.0);
    } else if (fogMode == 2.0) {
        return clamp(exp(-gl_Fog.density * gl_Fog.density * distance * distance), 0.0, 1.0);
	} else {
		return 1.0;
	}
}

void main (void) {
	// infravision sees right through fog, and see textures at full intensity
	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);
	vec3 normXY = normalize(viewXY);
	texCoords += vec3(normXY.y * -pulsate, normXY.x * pulsate, 0.0);
	texCoords += vec3(normXY.y * -wobble * texCoords.y, wobble * texCoords.y, 0.0);
	vec4 color = texture2D(texture0, texCoords.xy);
	float avg = (color.r + color.g + color.b) / 3.0;
	float fogFactor = getFogFactor(length(viewDir));
	gl_FragColor = vec4(mix(gl_Fog.color.rgb, vertexColor.rgb * avg, fogFactor), vertexColor.a * color.a);
}

)"
