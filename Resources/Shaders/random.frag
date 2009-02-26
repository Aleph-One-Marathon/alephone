uniform sampler2D texture0;
uniform float time;

void main(void) {

	float a = fract(sin(gl_TexCoord[0].x * 133.0 + gl_TexCoord[0].y * 471.0 + time * 7.0) * 43757.0); 
	float b = fract(sin(gl_TexCoord[0].x * 2331.0 + gl_TexCoord[0].y * 63.0 + time * 3.0) * 32451.0); 
	float c = fract(sin(gl_TexCoord[0].x * 41.0 + gl_TexCoord[0].y * 12911.0 + time * 31.0) * 34563.0);
	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);
	gl_FragColor = vec4(vec3(a, b, c), color.a);
}
