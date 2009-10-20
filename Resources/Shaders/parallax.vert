varying vec3 viewXY;
varying vec3 viewDir;
varying vec4 vertexColor;

varying float FDxLOG2E;
varying float MLxLOG2E;

void main(void) {

	gl_Position  = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

	/* SETUP TBN MATRIX in normal matrix coords, gl_MultiTexCoord1 = tangent vector */
	vec3 n = normalize(gl_NormalMatrix * gl_Normal);
	vec3 t = normalize(gl_NormalMatrix * gl_MultiTexCoord1.xyz);
	vec3 b = normalize(cross(n, t) * gl_MultiTexCoord1.w);
	/* (column wise) */
	mat3 tbnMatrix = mat3(t.x, b.x, n.x, t.y, b.y, n.y, t.z, b.z, n.z);
	
	/* SETUP VIEW DIRECTION in unprojected local coords */
	viewDir = tbnMatrix * (gl_ModelViewMatrix * gl_Vertex).xyz;
	viewXY = -(gl_TextureMatrix[0] * vec4(viewDir.xyz, 1.0)).xyz;
	viewDir = -viewDir;
	vertexColor = gl_Color;
	FDxLOG2E = -gl_Fog.density * gl_Fog.density * 1.442695;
	MLxLOG2E = -0.0000002 * 1.442695;
}
