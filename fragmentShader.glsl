#version 430 core

const int SUN = 0;
const int PLANET = 1;
const int SKY = 2;

in vec4 frontAmbDiffExport, frontSpecExport;
in vec2 texCoordsExport;

uniform sampler2D sunTex;
uniform sampler2D planetTex;
uniform sampler2D skyTex;
uniform uint object;

out vec4 colorsOut;

vec4 texColor;

void main(void)
{
    if (object == SUN) texColor = texture(sunTex, texCoordsExport);
    if (object == PLANET) texColor = texture(planetTex, texCoordsExport);
	if (object == SKY) texColor = texture(skyTex, texCoordsExport);

	if (object == SKY) colorsOut = texColor;
	if (object == SUN) colorsOut = (frontAmbDiffExport * texColor + frontSpecExport);
	if (object == PLANET) colorsOut = (frontAmbDiffExport * texColor + frontSpecExport);
}
