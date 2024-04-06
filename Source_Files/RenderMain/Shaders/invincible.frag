R"(

uniform sampler2D texture0;
uniform float time;
uniform float logicalWidth;
uniform float logicalHeight;
uniform float pixelWidth;
uniform float pixelHeight;
uniform float transferFadeOut;
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

float rand(vec2 co){ 
	float a = 12.9898; 
	float b = 78.233; 
	float c = 43758.5453; 
	float dt= dot(co.xy ,vec2(a,b)); 
	float sn= mod(dt,3.14); 
	return fract(sin(sn) * c); 
} 
float round(float n){ 
	float nSign = 1.0; 
	if ( n < 0.0 ) { nSign = -1.0; }; 
	return nSign * floor(abs(n)+0.5); 
} 
void main(void) {
	float blockSize = round((logicalHeight/320.0) * (pixelHeight/logicalHeight));
	blockSize = max(blockSize, 1.0);
	float moment=fract(time/10000.0);
	float eX=moment*round(gl_FragCoord.x / blockSize);
	float eY=moment*round(gl_FragCoord.y / blockSize);
	vec2 entropy = vec2 (eX,eY); 
	float sr = rand(entropy); 
	float sg = rand(entropy*sr); 
	float sb = rand(entropy*sg); 
	vec3 intensity = vec3(sr, sg, sb); 
	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);
	float dropFactor = max(max(intensity.r, intensity.g), intensity.b);
	dropFactor = dropFactor*dropFactor*dropFactor;
	if( dropFactor < transferFadeOut ) {color.a = 0.0;} 
#ifdef GAMMA_CORRECTED_BLENDING
	intensity = intensity * intensity;  // approximation of pow(intensity, 2.2)
#endif
	float fogFactor = getFogFactor(length(viewDir));
	gl_FragColor = vec4(mix(gl_Fog.color.rgb, intensity, fogFactor), vertexColor.a * color.a);
}

)"
