#version 430 core

const int SUN = 0;
const int PLANET = 1;

in vec4 frontAmbDiffExport, frontSpecExport;
in vec2 texCoordsExport;

uniform sampler2D sunTex;
uniform sampler2D planetTex;
uniform uint object;

out vec4 colorsOut;

vec4 texColor;

void main(void)
{
    if (object == SUN) texColor = texture(sunTex, texCoordsExport);
    if (object == PLANET) texColor = texture(planetTex, texCoordsExport);

    colorsOut = (frontAmbDiffExport * texColor + frontSpecExport);
}
