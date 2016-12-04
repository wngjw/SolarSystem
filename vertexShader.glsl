#version 430 core

const int SUN = 0;
const int PLANET = 1;

layout(location=0) in vec4 sunCoords;
layout(location=1) in vec3 sunNormal;
layout(location=2) in vec2 sunTexCoords;
layout(location=3) in vec4 planetCoords;
layout(location=4) in vec3 planetNormal;
layout(location=5) in vec2 planetTexCoords;

uniform mat4 modelViewMat;
uniform mat4 projMat;
uniform mat3 normalMat;
uniform uint object;

out vec4 frontAmbDiffExport, frontSpecExport, backAmbDiffExport, backSpecExport;
out vec2 texCoordsExport;

struct Light
{
    vec4 ambCols;
    vec4 difCols;
    vec4 specCols;
    vec4 coords;
};
uniform Light light0;
uniform Light light1;

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
    }
    if (object == PLANET)
    {
        coords = planetCoords;
        normal = planetNormal;
        texCoordsExport = planetTexCoords;
    }

    normal = normalize(normalMat * normal);
    lightDirection = normalize(vec3(light0.coords));
    eyeDirection = -1.0f * normalize(vec3(modelViewMat * coords));
    halfway = (length(lightDirection + eyeDirection) == 0.0f) ? vec3(0.0) : (lightDirection + eyeDirection)/length(lightDirection + eyeDirection);

    frontEmit = sunMaterial.emitCols;
    frontGlobAmb = globAmb * sunMaterial.ambRefl;
    frontAmb = light0.ambCols * sunMaterial.ambRefl;
    //frontDif = max(dot(normal, lightDirection), 0.0f) * (light0.difCols * sunMaterial.difRefl);
    frontDif = dot(normal, lightDirection) * (light0.difCols) * (sunMaterial.difRefl);
    //frontSpec = pow(max(dot(normal, halfway), 0.0f), sunMaterial.shininess) * (light0.specCols * sunMaterial.specRefl);
    frontSpec = vec4(0.0);
    frontAmbDiffExport =  vec4(vec3(min(frontEmit + frontGlobAmb + frontAmb + frontDif, vec4(1.0))), 1.0);
    frontSpecExport =  vec4(vec3(min(frontSpec, vec4(1.0))), 1.0);

    normal = -1.0f * normal;

    gl_Position = projMat * modelViewMat * coords;
}
