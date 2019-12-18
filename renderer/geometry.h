#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cstdlib>
#include <cstdint>
#include <cmath>


constexpr float inchToMm = 25.4f;

struct Vec2
{
    f32 x;
    f32 y;
    f32 z;
};

inline Vec2& operator*=(Vec2 &v, f32 scalar)
{
    v.x *= scalar;
    v.y *= scalar;

    return v;
}

inline Vec2 operator*(Vec2 v, f32 scalar)
{
    Vec2 result = {};
    result.x = v.x * scalar;
    result.y = v.y * scalar;

    return result;
}

inline Vec2 operator+(Vec2 v1, Vec2 v2)
{
    Vec2 result = {};
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;

    return result;
}

struct Vec3
{
    f32 x;
    f32 y;
    f32 z;
};

inline Vec3 operator-(Vec3 v1, Vec3 v2)
{
    Vec3 result = {};
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;

    return result;
}

inline Vec3 operator-(Vec3 v)
{
    Vec3 result = {-v.x, -v.y, -v.z};

    return result;
}

inline Vec3 crossProduct(Vec3 v1, Vec3 v2)
{
    Vec3 result = {};
    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;

    return result;
}

inline f32 dotProduct(Vec3 v1, Vec3 v2)
{
    f32 result = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;

    return result;
}

inline f32 norm(Vec3 v)
{
    f32 result = v.x * v.x + v.y * v.y + v.z * v.z;

    return result;
}

Vec3 normalize(Vec3 v)
{
    Vec3 result = {v.x, v.y, v.z};
    f32 n = norm(v);
    if (n > 0) {
        f32 factor = 1 / sqrt(n);
        result.x *= factor;
        result.y *= factor;
        result.z *= factor;
    }

    return result;
}

struct Vec3u
{
    u32 x;
    u32 y;
    u32 z;
};

inline Vec3u operator-(Vec3u v1, Vec3u v2)
{
    Vec3u result = {};
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;

    return result;
}

inline Vec3u crossProduct(Vec3u v1, Vec3u v2)
{
    Vec3u result = {};
    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;

    return result;
}

inline u32 dotProduct(Vec3u v1, Vec3u v2)
{
    u32 result = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;

    return result;
}

class Matrix44
{
public:

	f32 x[4][4] = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
	static const Matrix44 kIdentity;

	Matrix44() {}

	Matrix44 (f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h,
			  f32 i, f32 j, f32 k, f32 l, f32 m, f32 n, f32 o, f32 p)
	{
		x[0][0] = a;
		x[0][1] = b;
		x[0][2] = c;
		x[0][3] = d;
		x[1][0] = e;
		x[1][1] = f;
		x[1][2] = g;
		x[1][3] = h;
		x[2][0] = i;
		x[2][1] = j;
		x[2][2] = k;
		x[2][3] = l;
		x[3][0] = m;
		x[3][1] = n;
		x[3][2] = o;
		x[3][3] = p;
	}

	const f32* operator[](int i) const
    {
        return x[i];
    }
	f32* operator[](int i)
    {
        return x[i];
    }

	// Multiply the current matrix with another matrix (rhs)
	Matrix44 operator * (const Matrix44 &v) const
	{
		Matrix44 tmp;
		multiply (*this, v, tmp);

		return tmp;
	}

	//[comment]
	// To make it easier to understand how a matrix multiplication works, the fragment of code
	// included within the #if-#else statement, show how this works if you were to iterate
	// over the coefficients of the resulting matrix (a). However you will often see this
	// multiplication being done using the code contained within the #else-#end statement.
	// It is exactly the same as the first fragment only we have litteraly written down
	// as a series of operations what would actually result from executing the two for() loops
	// contained in the first fragment. It is supposed to be faster, however considering
	// matrix multiplicatin is not necessarily that common, this is probably not super
	// useful nor really necessary (but nice to have -- and it gives you an example of how
	// it can be done, as this how you will this operation implemented in most libraries).
	//[/comment]
	static void multiply(const Matrix44 &a, const Matrix44 &b, Matrix44 &c)
	{
#if 0
		for (uint8_t i = 0; i < 4; ++i) {
			for (uint8_t j = 0; j < 4; ++j) {
				c[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] +
					a[i][2] * b[2][j] + a[i][3] * b[3][j];
			}
		}
#else
		// A restrict qualified pointer (or reference) is basically a promise
		// to the compiler that for the scope of the pointer, the target of the
		// pointer will only be accessed through that pointer (and pointers
		// copied from it.
		const f32 * __restrict ap = &a.x[0][0];
		const f32 * __restrict bp = &b.x[0][0];
			  f32 * __restrict cp = &c.x[0][0];

		f32 a0, a1, a2, a3;

		a0 = ap[0];
		a1 = ap[1];
		a2 = ap[2];
		a3 = ap[3];

		cp[0]  = a0 * bp[0]  + a1 * bp[4]  + a2 * bp[8]  + a3 * bp[12];
		cp[1]  = a0 * bp[1]  + a1 * bp[5]  + a2 * bp[9]  + a3 * bp[13];
		cp[2]  = a0 * bp[2]  + a1 * bp[6]  + a2 * bp[10] + a3 * bp[14];
		cp[3]  = a0 * bp[3]  + a1 * bp[7]  + a2 * bp[11] + a3 * bp[15];

		a0 = ap[4];
		a1 = ap[5];
		a2 = ap[6];
		a3 = ap[7];

		cp[4]  = a0 * bp[0]  + a1 * bp[4]  + a2 * bp[8]  + a3 * bp[12];
		cp[5]  = a0 * bp[1]  + a1 * bp[5]  + a2 * bp[9]  + a3 * bp[13];
		cp[6]  = a0 * bp[2]  + a1 * bp[6]  + a2 * bp[10] + a3 * bp[14];
		cp[7]  = a0 * bp[3]  + a1 * bp[7]  + a2 * bp[11] + a3 * bp[15];

		a0 = ap[8];
		a1 = ap[9];
		a2 = ap[10];
		a3 = ap[11];

		cp[8]  = a0 * bp[0]  + a1 * bp[4]  + a2 * bp[8]  + a3 * bp[12];
		cp[9]  = a0 * bp[1]  + a1 * bp[5]  + a2 * bp[9]  + a3 * bp[13];
		cp[10] = a0 * bp[2]  + a1 * bp[6]  + a2 * bp[10] + a3 * bp[14];
		cp[11] = a0 * bp[3]  + a1 * bp[7]  + a2 * bp[11] + a3 * bp[15];

		a0 = ap[12];
		a1 = ap[13];
		a2 = ap[14];
		a3 = ap[15];

		cp[12] = a0 * bp[0]  + a1 * bp[4]  + a2 * bp[8]  + a3 * bp[12];
		cp[13] = a0 * bp[1]  + a1 * bp[5]  + a2 * bp[9]  + a3 * bp[13];
		cp[14] = a0 * bp[2]  + a1 * bp[6]  + a2 * bp[10] + a3 * bp[14];
		cp[15] = a0 * bp[3]  + a1 * bp[7]  + a2 * bp[11] + a3 * bp[15];
#endif
	}

	// \brief return a transposed copy of the current matrix as a new matrix
	Matrix44 transposed() const
	{
#if 0
		Matrix44 t;
		for (uint8_t i = 0; i < 4; ++i) {
			for (uint8_t j = 0; j < 4; ++j) {
				t[i][j] = x[j][i];
			}
		}

		return t;
#else
		return Matrix44 (x[0][0],
						 x[1][0],
						 x[2][0],
						 x[3][0],
						 x[0][1],
						 x[1][1],
						 x[2][1],
						 x[3][1],
						 x[0][2],
						 x[1][2],
						 x[2][2],
						 x[3][2],
						 x[0][3],
						 x[1][3],
						 x[2][3],
						 x[3][3]);
#endif
	}

	// \brief transpose itself
	Matrix44& transpose ()
	{
		Matrix44 tmp (x[0][0],
					  x[1][0],
					  x[2][0],
					  x[3][0],
					  x[0][1],
					  x[1][1],
					  x[2][1],
					  x[3][1],
					  x[0][2],
					  x[1][2],
					  x[2][2],
					  x[3][2],
					  x[0][3],
					  x[1][3],
					  x[2][3],
					  x[3][3]);
		*this = tmp;

		return *this;
	}

	//[comment]
	// This method needs to be used for point-matrix multiplication. Keep in mind
	// we don't make the distinction between points and vectors at least from
	// a programming point of view, as both (as well as normals) are declared as Vec3.
	// However, mathematically they need to be treated differently. Points can be translated
	// when translation for vectors is meaningless. Furthermore, points are implicitly
	// be considered as having homogeneous coordinates. Thus the w coordinates needs
	// to be computed and to convert the coordinates from homogeneous back to Cartesian
	// coordinates, we need to divided x, y z by w.
	//
	// The coordinate w is more often than not equals to 1, but it can be different than
	// 1 especially when the matrix is projective matrix (perspective projection matrix).
	//[/comment]
	void multVecMatrix(const Vec3 &src, Vec3 &dst)
        {
            f32 a, b, c, w;

            a = src.x * x[0][0] + src.y * x[1][0] + src.z * x[2][0] + x[3][0];
            b = src.x * x[0][1] + src.y * x[1][1] + src.z * x[2][1] + x[3][1];
            c = src.x * x[0][2] + src.y * x[1][2] + src.z * x[2][2] + x[3][2];
            w = src.x * x[0][3] + src.y * x[1][3] + src.z * x[2][3] + x[3][3];

            dst.x = a / w;
            dst.y = b / w;
            dst.z = c / w;
        }

	// This method needs to be used for vector-matrix multiplication. Look at the differences
	// with the previous method (to compute a point-matrix multiplication). We don't use
	// the coefficients in the matrix that account for translation (x[3][0], x[3][1], x[3][2])
	// and we don't compute w.
	void multDirMatrix(Vec3 &src, Vec3 &dst)
        {
            f32 a, b, c;

            a = src.x * x[0][0] + src.y * x[1][0] + src.z * x[2][0];
            b = src.x * x[0][1] + src.y * x[1][1] + src.z * x[2][1];
            c = src.x * x[0][2] + src.y * x[1][2] + src.z * x[2][2];

            dst.x = a;
            dst.y = b;
            dst.z = c;
        }

	// Compute the inverse of the matrix using the Gauss-Jordan (or reduced row) elimination method.
	// We didn't explain in the lesson on Geometry how the inverse of matrix can be found. Don't
	// worry at this point if you don't understand how this works. But we will need to be able to
	// compute the inverse of matrices in the first lessons of the "Foundation of 3D Rendering" section,
	// which is why we've added this code. For now, you can just use it and rely on it
	// for doing what it's supposed to do. If you want to learn how this works though, check the lesson
	// on called Matrix Inverse in the "Mathematics and Physics of Computer Graphics" section.

	Matrix44 inverse() const
	{
		int i, j, k;
		Matrix44 s;
		Matrix44 t (*this);

		// Forward elimination
		for (i = 0; i < 3 ; i++) {
			int pivot = i;

			f32 pivotsize = t[i][i];

			if (pivotsize < 0) {
				pivotsize = -pivotsize;
			}

			for (j = i + 1; j < 4; j++) {
				f32 tmp = t[j][i];

				if (tmp < 0) {
					tmp = -tmp;
				}

				if (tmp > pivotsize) {
					pivot = j;
					pivotsize = tmp;
				}
			}

			if (pivotsize == 0) {
				// Cannot invert singular matrix
				return Matrix44();
			}

			if (pivot != i) {
				for (j = 0; j < 4; j++) {
					f32 tmp;

					tmp = t[i][j];
					t[i][j] = t[pivot][j];
					t[pivot][j] = tmp;

					tmp = s[i][j];
					s[i][j] = s[pivot][j];
					s[pivot][j] = tmp;
				}
			}

			for (j = i + 1; j < 4; j++) {
				f32 f = t[j][i] / t[i][i];

				for (k = 0; k < 4; k++) {
					t[j][k] -= f * t[i][k];
					s[j][k] -= f * s[i][k];
				}
			}
		}

		// Backward substitution
		for (i = 3; i >= 0; --i) {
			f32 f;

			if ((f = t[i][i]) == 0) {
				// Cannot invert singular matrix
				return Matrix44();
			}

			for (j = 0; j < 4; j++) {
				t[i][j] /= f;
				s[i][j] /= f;
			}

			for (j = 0; j < i; j++) {
				f = t[j][i];

				for (k = 0; k < 4; k++) {
					t[j][k] -= f * t[i][k];
					s[j][k] -= f * s[i][k];
				}
			}
		}

		return s;
	}

	// \brief set current matrix to its inverse
	const Matrix44& invert()
	{
		*this = inverse();
		return *this;
	}

};

Vec3 multVecMatrix(Matrix44 mat, Vec3 &v)
{
    Vec3 result = {};

    f32 a, b, c, w;

    a = v.x * mat.x[0][0] + v.y * mat.x[1][0] + v.z * mat.x[2][0] + mat.x[3][0];
    b = v.x * mat.x[0][1] + v.y * mat.x[1][1] + v.z * mat.x[2][1] + mat.x[3][1];
    c = v.x * mat.x[0][2] + v.y * mat.x[1][2] + v.z * mat.x[2][2] + mat.x[3][2];
    w = v.x * mat.x[0][3] + v.y * mat.x[1][3] + v.z * mat.x[2][3] + mat.x[3][3];

    result.x = a / w;
    result.y = b / w;
    result.z = c / w;

    return result;
}

inline f32 edgeFunction(Vec3 &a, Vec3 &b, Vec3 &c)
{
	return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

static const Vec3 kDefaultBackgroundColor = {0.235294f, 0.67451f, 0.843137f};
#endif // GEOMETRY_H
