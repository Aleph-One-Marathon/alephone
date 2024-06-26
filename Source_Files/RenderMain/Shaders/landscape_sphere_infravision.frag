R"(

uniform sampler2D texture0;
uniform float fogMix;
varying vec3 relDir;
varying vec4 vertexColor;
varying float cosPitch;
varying float sinPitch;
varying float cosYaw;
varying float sinYaw;
const float zoom = 1.2;
const float pitch_adjust = 0.96;
const float M_PI = 3.14156;
void main(void) {
	mat3 rotateYaw = mat3(cosYaw, 0, sinYaw,
						  0, 1, 0,
						  -sinYaw, 0, cosYaw);

	mat3 rotatePitch = mat3(1, 0, 0,
							0, cosPitch, -sinPitch,
							0, sinPitch, cosPitch);

	vec3 normRelDir = rotateYaw * rotatePitch * normalize(relDir);
	
	float theta = atan(normRelDir.x, normRelDir.z);
	float phi = acos(normRelDir.y);

	float u = (theta + M_PI) / (2.0 * M_PI);
	float v = phi / M_PI;

	vec4 color = texture2D(texture0, vec2(u, v));
    float avg = (color.r + color.g + color.b) / 3.0;
	vec3 intensity = mix(vertexColor.rgb * avg, gl_Fog.color.rgb, fogMix);
	gl_FragColor = vec4(intensity, 1.0);
}

)"
