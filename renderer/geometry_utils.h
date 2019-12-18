/*
 * Copyright (C) 2012  www.scratchapixel.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "geometry.h"
#include "utils.h"


namespace scratch
{
namespace geometry_utils
{
bool rayTriangleIntersect(
	const Vec3 &orig, const Vec3 &dir,
	const Vec3 &v0, const Vec3 &v1, const Vec3 &v2,
	float &t, float &u, float &v)
{
	Vec3 v0v1 = v1 - v0;
	Vec3 v0v2 = v2 - v0;
	Vec3 pvec = crossProduct(dir, v0v2);
	float det = dotProduct(v0v1, pvec);

	// ray and triangle are parallel if det is close to 0
	if (fabs(det) < kEpsilon) return false;

	float invDet = 1 / det;

	Vec3 tvec = orig - v0;
	u = dotProduct(tvec, pvec) * invDet;
	if (u < 0 || u > 1) return false;

	Vec3 qvec = crossProduct(tvec, v0v1);
	v = dotProduct(dir, qvec) * invDet;
	if (v < 0 || u + v > 1) return false;

	t = dotProduct(v0v2, qvec) * invDet;

	return (t > 0) ? true : false;
}
}
}
