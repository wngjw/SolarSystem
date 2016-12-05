#version 430 core

const int SUN = 0;
const int PLANET = 1;
const int SKY = 2;

layout(location=0) in vec4 sunCoords;
layout(location=1) in vec3 sunNormal;
layout(location=2) in vec2 sunTexCoords;
layout(location=3) in vec4 planetCoords;
layout(location=4) in vec3 planetNormal;
layout(location=5) in vec2 planetTexCoords;
layout(location=6) in vec4 skyCoords;
layout(location=7) in vec3 skyNormal;
layout(location=8) in vec2 skyTexCoords;

uniform mat4 modelViewMat;
uniform mat4 projMat;
uniform mat3 normalMat;
uniform uint object;

out vec4 frontAmbDiffExport, frontSpecExport;
out vec2 texCoordsExport;

struct Light
{
    vec4 ambCols;
    vec4 difCols;
    vec4 specCols;
    vec4 coords;
};
uniform Light light0;

uniform vec4 globAmb;

struct Material
{
    vec4 ambRefl;
    vec4 difRefl;
    vec4 specRefl;
    vec4 emitCols;
    float shininess;
};
uniform Material sunMaterial;
uniform Material planetMaterial;

Material mat;

vec3 normal, lightDirection, eyeDirection, halfway;
vec4 frontEmit, frontGlobAmb, frontAmb, frontDif, frontSpec,
     backEmit, backGlobAmb, backAmb, backDif, backSpec;
vec4 coords;

void main(void)
{
    if (object == SUN)
    {
        coords = sunCoords;
        normal = sunNormal;
        texCoordsExport = sunTexCoords;
        mat.ambRefl = sunMaterial.ambRefl;
        mat.difRefl = sunMaterial.difRefl;
        mat.specRefl = sunMaterial.specRefl;
        mat.emitCols = sunMaterial.emitCols;
        mat.shininess = sunMaterial.shininess;
    }
    if (object == PLANET)
    {
        coords = planetCoords;
        normal = planetNormal;
        texCoordsExport = planetTexCoords;
        mat.ambRefl = planetMaterial.ambRefl;
        mat.difRefl = planetMaterial.difRefl;
        mat.specRefl = planetMaterial.specRefl;
        mat.emitCols = planetMaterial.emitCols;
        mat.shininess = planetMaterial.shininess;
    }

	if (object == SKY)
	{
		coords = skyCoords;
		normal = skyNormal;
		texCoordsExport = skyTexCoords;
	}

    normal = normalize(normalMat * normal);
    lightDirection = normalize(vec3(light0.coords - modelViewMat * coords));
    eyeDirection = -1.0f * normalize(vec3(modelViewMat * coords));
    halfway = (length(lightDirection + eyeDirection) == 0.0f) ? vec3(0.0) : (lightDirection + eyeDirection)/length(lightDirection + eyeDirection);

    frontEmit = mat.emitCols;
    frontGlobAmb = globAmb * mat.ambRefl;
    frontAmb = light0.ambCols * mat.ambRefl;
    frontDif = max(dot(normal, lightDirection), 0.0f) * (light0.difCols * mat.difRefl);
    frontSpec = pow(max(dot(normal, halfway), 0.0f), mat.shininess) * (light0.specCols * mat.specRefl);
    frontAmbDiffExport =  vec4(vec3(min(frontEmit + frontGlobAmb + frontAmb + frontDif, vec4(1.0))), 1.0);
    frontSpecExport =  vec4(vec3(min(frontSpec, vec4(1.0))), 1.0);

    normal = -1.0f * normal;

    gl_Position = projMat * modelViewMat * coords;
}
