////////////////////////////////////////////////////////////////
// Sphere.cpp
//
// Tony Wang and Luke Johnson
////////////////////////////////////////////////////////////////

#include "../include/Sphere.h"
#include<cmath>

Sphere::Sphere(float radius, int slices, vec4 color)
{
    points = new vec4[2 * (slices + 1)];
    colors = new vec4[2 * (slices + 1)];
    normals = new vec4[2 * (slices + 1)];
    indices = new int[2 * (slices + 1)];

}

Sphere::~Sphere()
{
    delete[] points;
    delete[] colors;
    delete[] indices;
    delete[] indices;
}
