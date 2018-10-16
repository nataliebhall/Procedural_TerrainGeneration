#pragma once
#include <math.h>
#include"quickVect.h"


class raytriangle 
{
public:
	float Dot(quickVect v1, quickVect v2)
	{
		float dot;
		dot = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;

		return dot;
	}

	quickVect Cross(quickVect v1, quickVect v2)
	{
		quickVect cross;
		cross.x = v1.y*v2.z - v1.z*v2.y;
		cross.y = v1.z*v2.x - v1.x*v2.z;
		cross.z = v1.x*v2.y - v1.y*v2.x;

		return cross;
	}

	quickVect Sub(quickVect v1, quickVect v2)
	{
		quickVect sub;
		sub.x = v1.x - v2.x;
		sub.y = v1.y - v2.y;
		sub.z = v1.z - v2.z;

		return sub;
	}

	quickVect Add(quickVect v1, quickVect v2)
	{
		quickVect add;
		add.x = v1.x + v2.x;
		add.y = v1.y + v2.y;
		add.z = v1.z + v2.z;

		return add;
	}

	float Length(quickVect v)
	{
		float ans;
		ans = sqrtf(v.x* v.x + v.y*v.y + v.z*v.z);

		return ans;
	}

	quickVect Normalize(quickVect v)
	{
		float l;
		l = Length(v);

		v.x = v.x / l;
		v.y = v.y / l;
		v.z = v.z / l;

		return v;
	}

	quickVect Multiply(quickVect v, float f)
	{
		v.x = v.x * f;
		v.y = v.y * f;
		v.z = v.z * f;

		return v;
	}

	#define EPSILON 0.000001f

	
	bool RayIntersectsTriangle(quickVect rayOrigin,
		quickVect rayVector,
		quickVect vertex0,
		quickVect vertex1,
		quickVect vertex2,
		quickVect& outIntersectionPoint);

};
