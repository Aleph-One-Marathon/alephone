uniform sampler2DRect texture0;
varying vec4 vertexColor;

void main (void) {

	vec3 t = 12870.0 * texture2DRect(texture0, gl_TexCoord[0].xy).rgb;
	
	t += 11440.0 * texture2DRect(texture0, gl_TexCoord[0].xy - vec2(0, 1)).rgb;
	t += 11440.0 * texture2DRect(texture0, gl_TexCoord[0].xy + vec2(0, 1)).rgb;

	t += 8008.0 * texture2DRect(texture0, gl_TexCoord[0].xy - vec2(0, 2)).rgb;
	t += 8008.0 * texture2DRect(texture0, gl_TexCoord[0].xy + vec2(0, 2)).rgb;

	t += 4368.0 * texture2DRect(texture0, gl_TexCoord[0].xy - vec2(0, 3)).rgb;
	t += 4368.0 * texture2DRect(texture0, gl_TexCoord[0].xy + vec2(0, 3)).rgb;

	t += 1820.0 * texture2DRect(texture0, gl_TexCoord[0].xy - vec2(0, 4)).rgb;
	t += 1820.0 * texture2DRect(texture0, gl_TexCoord[0].xy + vec2(0, 4)).rgb;

	t += 560.0 * texture2DRect(texture0, gl_TexCoord[0].xy - vec2(0, 5)).rgb;
	t += 560.0 * texture2DRect(texture0, gl_TexCoord[0].xy + vec2(0, 5)).rgb;

	t += 120.0 * texture2DRect(texture0, gl_TexCoord[0].xy - vec2(0, 6)).rgb;
	t += 120.0 * texture2DRect(texture0, gl_TexCoord[0].xy + vec2(0, 6)).rgb;

	t += 16.0 * texture2DRect(texture0, gl_TexCoord[0].xy - vec2(0, 7)).rgb;
	t += 16.0 * texture2DRect(texture0, gl_TexCoord[0].xy + vec2(0, 7)).rgb;

	t += texture2DRect(texture0, gl_TexCoord[0].xy - vec2(0, 8)).rgb;
	t += texture2DRect(texture0, gl_TexCoord[0].xy + vec2(0, 8)).rgb;

	t /= 65536.0;

	gl_FragColor = vec4(t, 1.0) * vertexColor;
}
