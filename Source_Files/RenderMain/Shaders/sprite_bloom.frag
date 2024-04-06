R"(

uniform sampler2D texture0;
uniform float glow;
uniform float bloomScale;
uniform float bloomShift;
uniform float fogMode;
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
	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);
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
