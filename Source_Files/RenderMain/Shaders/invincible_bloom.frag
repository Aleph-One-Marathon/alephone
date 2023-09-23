R"(

uniform sampler2D texture0;
uniform float time;
uniform float transferFadeOut;
varying vec3 viewDir;
varying vec4 vertexColor;
varying float FDxLOG2E;
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
	float blockHeight=2.0;
	float blockWidth=2.0;
	float darkBlockProbability=0.8;
	float moment=fract(time/10000.0);
	float eX=moment*round((gl_FragCoord.x + mod(time, 60.0)*7.0) / blockWidth);
	float eY=moment*round((gl_FragCoord.y + mod(time, 60.0)*11.0) / blockHeight);
	vec2 entropy = vec2 (eX, eY); 
	float sr = rand(entropy); 
	float sg = rand(entropy*sr); 
	float sb = rand(entropy*sg); 
	vec3 intensity = vec3(0.0,0.0,0.0);
	if (rand(entropy*sr*sg*sb) > darkBlockProbability) {
		intensity = vec3(sr*sr, sg*sg, sb); }
	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);
	float dropFactor = max(max(intensity.r, intensity.g), intensity.b);
	dropFactor = dropFactor*dropFactor;
	if( dropFactor < transferFadeOut ) {color.a = 0.0;} 
#ifdef GAMMA_CORRECTED_BLENDING
	intensity = intensity * intensity;  // approximation of pow(intensity, 2.2)
#endif
	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);
	gl_FragColor = vec4(mix(gl_Fog.color.rgb, intensity, fogFactor), vertexColor.a * color.a);
}

)"
