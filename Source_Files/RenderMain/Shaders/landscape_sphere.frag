R"(

uniform sampler2D texture0;
uniform float fogMix;
varying vec3 relDir;
varying vec4 vertexColor;
varying float cosPitch;
varying float sinPitch;
varying float cosYaw;
varying float sinYaw;
uniform float offsetx; // azimuth in the sphere map
const float M_PI = 3.14156;
void main(void) {
	mat3 rotateYaw = mat3(cosYaw, 0, sinYaw,
						  0, 1, 0,
						  -sinYaw, 0, cosYaw);

	mat3 rotatePitch = mat3(1, 0, 0,
							0, cosPitch, -sinPitch,
							0, sinPitch, cosPitch);

	vec3 normRelDir = rotateYaw * rotatePitch * normalize(relDir);
	
	float theta = atan(normRelDir.x, normRelDir.z) - offsetx;
	float phi = acos(normRelDir.y);

	float u = (M_PI - theta) / (2.0 * M_PI);
	float v = phi / M_PI;

	vec4 color = texture2D(texture0, vec2(u, v));
	vec3 intensity = mix(color.rgb, gl_Fog.color.rgb, fogMix);
	gl_FragColor = vec4(intensity, 1.0);
}

)"
