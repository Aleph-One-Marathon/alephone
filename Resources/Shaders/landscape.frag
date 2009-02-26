uniform sampler2D texture0;

varying vec3 viewDir;

void main(void) {
	float pi = 2.0 * asin(1.0);
	vec3 viewv = normalize(viewDir);
	float x = atan(viewv.x, viewv.y) / pi;
	float y = 0.5 - viewv.z*0.5;
	gl_FragColor = texture2D(texture0, vec2(x, y));
}
