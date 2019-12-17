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

constexpr float inchToMm = 25.4f;
static const Vec3 kDefaultBackgroundColor = {0.235294f, 0.67451f, 0.843137f};

#if 0
template <typename T>
inline Vec3<T> mix(const Vec3<T> &a, const Vec3<T> &b, const float &mixValue)
{
	return a * (1 - mixValue) + b * mixValue;
}

template <typename T>
inline Vec3<T> reflect(const Vec3<T> &I, const Vec3<T> &N)
{
	return I - 2 * I.dotProduct(N) * N;
}
template <typename T>
inline float edgeFunction(const Vec3<T> &a, const Vec3<T> &b, const Vec3<T> &c)
{
	return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}
#else
inline f32 edgeFunction(Vec3 &a, Vec3 &b, Vec3 &c)
{
	return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}
#endif
// [comment]
// Compute refraction direction using Snell's law
//
// We need to handle with care the two possible situations:
//
//    - When the ray is inside the object
//
//    - When the ray is outside.
//
// If the ray is outside, you need to make cosi positive cosi = -N.I
//
// If the ray is inside, you need to invert the refractive indices and negate the normal N
// [/comment]
#if 0
template <typename T>
Vec3<T> refract(const Vec3<T> &I, const Vec3<T> &N, const float &ior)
{
	float cosi = scratch::utils::clamp(-1, 1, I.dotProduct(N));
	float etai = 1, etat = ior;
	Vec3f n = N;
	if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n= -N; }
	float eta = etai / etat;
	float k = 1 - eta * eta * (1 - cosi * cosi);
	return k < 0 ? 0 : eta * I + (eta * cosi - sqrtf(k)) * n;
}
#endif
// [comment]
// Compute Fresnel equation
//
// \param I is the incident view direction
//
// \param N is the normal at the intersection point
//
// \param ior is the material refractive index
//
// \param[out] kr is the amount of light reflected
// [/comment]
#if 0
template <typename T>
void fresnel(const Vec3<T> &I, const Vec3<T> &N, const float &ior, float &kr)
{
	float cosi = scratch::utils::clamp(-1, 1, I.dotProduct(N));
	float etai = 1, etat = ior;
	if (cosi > 0) {  std::swap(etai, etat); }
	// Compute sini using Snell's law
	float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
	// Total internal reflection
	if (sint >= 1) {
		kr = 1;
	}
	else {
		float cost = sqrtf(std::max(0.f, 1 - sint * sint));
		cosi = fabsf(cosi);
		float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
		float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
		kr = (Rs * Rs + Rp * Rp) / 2;
	}
	// As a consequence of the conservation of energy, transmittance is given by:
	// kt = 1 - kr;
}
#endif
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
