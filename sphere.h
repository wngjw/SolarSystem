#ifndef SPHERE_H
#define SPHERE_H

#include "vertex.h"

const float PI = 3.14159265;
const int SPHERE_LONGS = 50;
const int SPHERE_LATS = 50;

void fillSphereVertexArray(Vertex sphereVertices[(SPHERE_LONGS + 1) * (SPHERE_LATS + 1)]);
void fillSphereIndices(unsigned int sphereIndices[SPHERE_LATS][2*(SPHERE_LONGS+1)]);
void fillSphereCounts(int cylCounts[SPHERE_LATS]);
void fillSphereOffsets(void* cylOffsets[SPHERE_LATS]);

void fillSphereinder(Vertex sphereVertices[(SPHERE_LONGS + 1) * (SPHERE_LATS + 1)],
	         unsigned int sphereIndices[SPHERE_LATS][2 * (SPHERE_LONGS+1)],
			 int sphereCounts[SPHERE_LATS],
			 void* sphereOffsets[SPHERE_LATS]);

#endif
