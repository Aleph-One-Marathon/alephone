R"(

uniform sampler2D texture0;
uniform float usefog;
uniform float scalex;
uniform float scaley;
uniform float offsetx;
uniform float offsety;
uniform float yaw;
uniform float pitch;
varying vec3 relDir;
varying vec4 vertexColor;
const float zoom = 1.2;
const float pitch_adjust = 0.96;
void main(void) {
	vec3 facev = vec3(cos(yaw), sin(yaw), sin(pitch));
	vec3 relv  = (relDir);
	float x = relv.x / (relv.z * zoom) + atan(facev.x, facev.y);
	float y = relv.y / (relv.z * zoom) - (facev.z * pitch_adjust);
	vec4 color = texture2D(texture0, vec2(offsetx - x * scalex, offsety - y * scaley));
	float avg = (color.r + color.g + color.b) / 3.0;
	vec3 intensity = vertexColor.rgb * avg;
	if (usefog > 0.0) {
		intensity = gl_Fog.color.rgb;
	}
	gl_FragColor = vec4(intensity, 1.0);
}


)"
