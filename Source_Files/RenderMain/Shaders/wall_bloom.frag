R"(

uniform sampler2D texture0;
uniform float pulsate;
uniform float wobble;
uniform float glow;
uniform float flare;
uniform float bloomScale;
uniform float bloomShift;
varying vec3 viewXY;
varying vec3 viewDir;
varying vec4 vertexColor;
varying float FDxLOG2E;
void main (void) {
	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);
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
	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);
	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), color.rgb * intensity, fogFactor), vertexColor.a * color.a);
}

)"
