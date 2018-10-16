#include "raytriangle.h"

bool raytriangle::RayIntersectsTriangle(quickVect rayOrigin,
	quickVect rayVector,	
	quickVect vertex0,
	quickVect vertex1,
	quickVect vertex2,
	quickVect& outIntersectionPoint)
{
	quickVect edge1, edge2, h, s, q;
	float a, f, u, v;

	edge1 = Sub(vertex1, vertex0);
	edge2 = Sub(vertex2, vertex0);

	h = Cross(rayVector, edge2);
	a = Dot(edge1, h);

	if (a > -EPSILON && a < EPSILON)
	{
		return false;
	}

	f = 1 / a;
	s = Sub(rayOrigin, vertex0);
	u = f * (Dot(s, h));

	if (u < 0.0 || u > 1.0)
	{
		return false;
	}

	q = Cross(s, edge1);
	v = f * Dot(rayVector, q);

	if (v < 0.0 || u + v > 1.0)
	{
		return false;
	}

	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = f * Dot(edge2, q);

	if (t > EPSILON) // ray intersection
	{
		outIntersectionPoint = Add(rayOrigin, Multiply(rayVector, t));
		return true;
	}

	else // This means that there is a line intersection but not a ray intersection.
	{
		return false;
	}
}