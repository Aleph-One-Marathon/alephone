uniform sampler2DRect texture0;

void main (void) {

	vec3 t = 12870.0 * texture2DRect(texture0, gl_TexCoord[0].xy).rgb;

	t += 11440.0 * texture2DRect(texture0, gl_TexCoord[0].xy - vec2(1, 0)).rgb;
	t += 11440.0 * texture2DRect(texture0, gl_TexCoord[0].xy + vec2(1, 0)).rgb;

	t += 8008.0 * texture2DRect(texture0, gl_TexCoord[0].xy - vec2(2, 0)).rgb;
	t += 8008.0 * texture2DRect(texture0, gl_TexCoord[0].xy + vec2(2, 0)).rgb;

	t += 4368.0 * texture2DRect(texture0, gl_TexCoord[0].xy - vec2(3, 0)).rgb;
	t += 4368.0 * texture2DRect(texture0, gl_TexCoord[0].xy + vec2(3, 0)).rgb;

	t += 1820.0 * texture2DRect(texture0, gl_TexCoord[0].xy - vec2(4, 0)).rgb;
	t += 1820.0 * texture2DRect(texture0, gl_TexCoord[0].xy + vec2(4, 0)).rgb;

	t += 560.0 * texture2DRect(texture0, gl_TexCoord[0].xy - vec2(5, 0)).rgb;
	t += 560.0 * texture2DRect(texture0, gl_TexCoord[0].xy + vec2(5, 0)).rgb;

	t += 120.0 * texture2DRect(texture0, gl_TexCoord[0].xy - vec2(6, 0)).rgb;
	t += 120.0 * texture2DRect(texture0, gl_TexCoord[0].xy + vec2(6, 0)).rgb;

	t += 16.0 * texture2DRect(texture0, gl_TexCoord[0].xy - vec2(7, 0)).rgb;
	t += 16.0 * texture2DRect(texture0, gl_TexCoord[0].xy + vec2(7, 0)).rgb;

	t += texture2DRect(texture0, gl_TexCoord[0].xy - vec2(8, 0)).rgb;
	t += texture2DRect(texture0, gl_TexCoord[0].xy + vec2(8, 0)).rgb;

	t /= 65536.0;

	gl_FragColor = vec4(t, 1.0);
}
