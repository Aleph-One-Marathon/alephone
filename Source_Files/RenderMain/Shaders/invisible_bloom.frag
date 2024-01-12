R"(

uniform sampler2D texture0;
uniform float visibility;
uniform float fogMode;
varying vec3 viewDir;
varying vec4 vertexColor;

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

void main(void) {
	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);
	vec3 intensity = vec3(0.0, 0.0, 0.0);
	float fogFactor = getFogFactor(length(viewDir));
	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), intensity, fogFactor), vertexColor.a * color.a * visibility);
}

)"
