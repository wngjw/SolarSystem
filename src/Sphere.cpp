////////////////////////////////////////////////////////////////
// Sphere.cpp
//
// Tony Wang and Luke Johnson
////////////////////////////////////////////////////////////////

#include "../include/Sphere.h"

Sphere::Sphere(float radius)
{

}

Sphere::~Sphere()
{
    delete[] points;
    delete[] colors;
    delete[] indices;
    delete[] indices;
}
