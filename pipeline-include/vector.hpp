#pragma once

#ifdef NDEBUG
#define Assert( _exp ) ((void)0)
#else
#define Assert(x)
#endif

#include <sstream>

#define CHECK_VALID( _v ) 0

#define FastSqrt(x)	(sqrt)(x)

#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h

#define M_PI_2      (M_PI * 2.f)

#define M_PI_F		((float)(M_PI))	// Shouldn't collide with anything.

#define M_PHI		1.61803398874989484820 // golden ratio

// NJS: Inlined to prevent floats from being autopromoted to doubles, as with the old system.
#ifndef RAD2DEG
#define RAD2DEG(x)  ((float)(x) * (float)(180.f / M_PI_F))
#endif

#ifndef DEG2RAD
#define DEG2RAD(x)  ((float)(x) * (float)(M_PI_F / 180.f))
#endif

// MOVEMENT INFO
enum
{
	PITCH = 0,	// up / down
	YAW,		// left / right
	ROLL		// fall over
};

// decls for aligning data


#define ALIGN16 __declspec(align(16))
#define VALVE_RAND_MAX 0x7fff
#define vectorExpand(v) (v).x, (v).y, (v).z

typedef float vec_t;

class vector
{
public:
	float x, y, z;
	vector(void);
	vector(float X, float Y);
	vector(int X, int Y);
	vector(float X, float Y, float Z);
	void init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f);
	bool is_valid() const;
	float operator[](int i) const;
	float& operator[](int i);
	inline void zero();
	bool operator==(const vector& v) const;
	bool operator!=(const vector& v) const;
	inline vector& operator+=(const vector& v);
	inline vector& operator-=(const vector& v);
	inline vector& operator*=(const vector& v);
	inline vector& operator*=(float s);
	inline vector& operator/=(const vector& v);
	inline vector& operator/=(float s);
	inline vector& operator+=(float fl);
	inline vector& operator-=(float fl);
	inline float	length() const;
	inline float length_sqr(void) const
	{
		CHECK_VALID(*this);
		return (x * x + y * y + z * z);
	}
	bool is_zero(float tolerance = 0.01f) const
	{
		return (x > -tolerance && x < tolerance &&
			y > -tolerance && y < tolerance &&
			z > -tolerance && z < tolerance);
	}
	vector	normalize();
	float	normalize_in_place();
	inline float	dist_to(const vector& vOther) const;
	inline float	dist_to_sqr(const vector& vOther) const;
	float	dot(const vector& vOther) const;
	float	length_2d(void) const;
	float	length_2d_sqr(void) const;
	void	mul_add(const vector& a, const vector& b, float scalar);
	vector& operator=(const vector& vOther);
	vector	operator-(void) const;
	vector	operator+(const vector& v) const;
	vector	operator-(const vector& v) const;
	vector	operator*(const vector& v) const;
	vector	operator/(const vector& v) const;
	vector	operator*(float fl) const;
	vector	operator/(float fl) const;
	vector	operator+(float fl) const;
	// Base address...
	float* Base();
	float const* Base() const;
};


inline void vector::init(float ix, float iy, float iz)
{
	x = ix; y = iy; z = iz;
	CHECK_VALID(*this);
}

inline vector::vector(float X, float Y, float Z)
{
	x = X; y = Y; z = Z;
	CHECK_VALID(*this);
}

inline vector::vector(void) { }

inline vector::vector(float X, float Y)
{
	x = X; y = Y; z = 0;
	CHECK_VALID(*this);
}

inline vector::vector(int X, int Y)
{
	x = X; y = Y; z = 0;
	CHECK_VALID(*this);
}

inline void vector::zero()
{
	x = y = z = 0.0f;
}

inline void vector_clear(vector& a)
{
	a.x = a.y = a.z = 0.0f;
}

inline vector& vector::operator=(const vector& vOther)
{
	CHECK_VALID(vOther);
	x = vOther.x; y = vOther.y; z = vOther.z;
	return *this;
}

inline float& vector::operator[](int i)
{
	Assert((i >= 0) && (i < 3));
	return ((float*)this)[i];
}

inline float vector::operator[](int i) const
{
	Assert((i >= 0) && (i < 3));
	return ((float*)this)[i];
}

inline bool vector::operator==(const vector& src) const
{
	CHECK_VALID(src);
	CHECK_VALID(*this);
	return (src.x == x) && (src.y == y) && (src.z == z);
}

inline bool vector::operator!=(const vector& src) const
{
	CHECK_VALID(src);
	CHECK_VALID(*this);
	return (src.x != x) || (src.y != y) || (src.z != z);
}

inline void vector_copy(const vector& src, vector& dst)
{
	CHECK_VALID(src);
	dst.x = src.x;
	dst.y = src.y;
	dst.z = src.z;
}

inline  vector& vector::operator+=(const vector& v)
{
	CHECK_VALID(*this);
	CHECK_VALID(v);
	x += v.x; y += v.y; z += v.z;
	return *this;
}

inline  vector& vector::operator-=(const vector& v)
{
	CHECK_VALID(*this);
	CHECK_VALID(v);
	x -= v.x; y -= v.y; z -= v.z;
	return *this;
}

inline  vector& vector::operator*=(float fl)
{
	x *= fl;
	y *= fl;
	z *= fl;
	CHECK_VALID(*this);
	return *this;
}

inline  vector& vector::operator*=(const vector& v)
{
	CHECK_VALID(v);
	x *= v.x;
	y *= v.y;
	z *= v.z;
	CHECK_VALID(*this);
	return *this;
}

inline vector& vector::operator+=(float fl)
{
	x += fl;
	y += fl;
	z += fl;
	CHECK_VALID(*this);
	return *this;
}

inline vector& vector::operator-=(float fl)
{
	x -= fl;
	y -= fl;
	z -= fl;
	CHECK_VALID(*this);
	return *this;
}

inline  vector& vector::operator/=(float fl)
{
	Assert(fl != 0.0f);
	float oofl = 1.0f / fl;
	x *= oofl;
	y *= oofl;
	z *= oofl;
	CHECK_VALID(*this);
	return *this;
}

inline  vector& vector::operator/=(const vector& v)
{
	CHECK_VALID(v);
	Assert(v.x != 0.0f && v.y != 0.0f && v.z != 0.0f);
	x /= v.x;
	y /= v.y;
	z /= v.z;
	CHECK_VALID(*this);
	return *this;
}

inline float vector::length(void) const
{
	CHECK_VALID(*this);

	float root = 0.0f;

	float sqsr = x * x + y * y + z * z;

	root = sqrt(sqsr);

	return root;
}

inline float vector::length_2d(void) const
{
	CHECK_VALID(*this);

	float root = 0.0f;

	float sqst = x * x + y * y;

	root = sqrt(sqst);

	return root;
}

inline float vector::length_2d_sqr(void) const
{
	return (x * x + y * y);
}

inline vector cross_product(const vector& a, const vector& b)
{
	return vector(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

float vector::dist_to(const vector& vOther) const
{
	vector delta;

	delta.x = x - vOther.x;
	delta.y = y - vOther.y;
	delta.z = z - vOther.z;

	return delta.length();
}
float vector::dist_to_sqr(const vector& vOther) const
{
	vector delta;

	delta.x = x - vOther.x;
	delta.y = y - vOther.y;
	delta.z = z - vOther.z;

	return delta.length_sqr();
}

inline vector vector::normalize()
{
	vector vector;
	float length = this->length();

	if (length != 0)
	{
		vector.x = x / length;
		vector.y = y / length;
		vector.z = z / length;
	}
	else
	{
		vector.x = vector.y = 0.0f; vector.z = 1.0f;
	}

	return vector;
}

// changed that to fit awall, paste from xaE
inline float vector::normalize_in_place()
{
	float radius = FastSqrt(x * x + y * y + z * z);

	// FLT_EPSILON is added to the radius to eliminate the possibility of divide by zero.
	float iradius = 1.f / (radius + FLT_EPSILON);

	x *= iradius;
	y *= iradius;
	z *= iradius;

	return radius;
}

inline void vector::mul_add(const vector& a, const vector& b, float scalar)
{
	x = a.x + b.x * scalar;
	y = a.y + b.y * scalar;
	z = a.z + b.z * scalar;
}

inline float vectorNormalize(vector& v)
{
	Assert(v.is_valid());
	float l = v.length();
	if (l != 0.0f)
	{
		v /= l;
	}
	else
	{
		// FIXME:
		// Just copying the existing implemenation; shouldn't res.z == 0?
		v.x = v.y = 0.0f; v.z = 1.0f;
	}
	return l;
}

inline float vectorNormalize(float* v)
{
	return vectorNormalize(*(reinterpret_cast<vector*>(v)));
}

inline vector vector::operator+(const vector& v) const
{
	vector res;
	res.x = x + v.x;
	res.y = y + v.y;
	res.z = z + v.z;
	return res;
}


inline vector vector::operator-(const vector& v) const
{
	vector res;
	res.x = x - v.x;
	res.y = y - v.y;
	res.z = z - v.z;
	return res;
}

inline vector vector::operator*(float fl) const
{
	vector res;
	res.x = x * fl;
	res.y = y * fl;
	res.z = z * fl;
	return res;
}

inline vector vector::operator*(const vector& v) const
{
	vector res;
	res.x = x * v.x;
	res.y = y * v.y;
	res.z = z * v.z;
	return res;
}

inline vector vector::operator/(float fl) const
{
	vector res;
	res.x = x / fl;
	res.y = y / fl;
	res.z = z / fl;
	return res;
}

inline vector vector::operator+(float fl) const
{
	return vector(x + fl, y + fl, z + fl);
}

inline vector vector::operator/(const vector& v) const
{
	vector res;
	res.x = x / v.x;
	res.y = y / v.y;
	res.z = z / v.z;
	return res;
}
inline float vector::dot(const vector& b) const
{
	if (b.x == NULL && b.y == NULL && b.z == NULL) {
		return NULL;
	}

	return (x * b.x + y * b.y + z * b.z);
}

//-----------------------------------------------------------------------------
// length
//-----------------------------------------------------------------------------

inline float vectorLength(const vector& v)
{
	CHECK_VALID(v);
	return (float)FastSqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

//vector SUBTRAC
inline void vectorSubtract(const vector& a, const vector& b, vector& c)
{
	CHECK_VALID(a);
	CHECK_VALID(b);
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
}

//vectorADD
inline void vectorAdd(const vector& a, const vector& b, vector& c)
{
	CHECK_VALID(a);
	CHECK_VALID(b);
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
}

//-----------------------------------------------------------------------------
// Base address...
//-----------------------------------------------------------------------------
inline float* vector::Base()
{
	return (float*)this;
}

inline float const* vector::Base() const
{
	return (float const*)this;
}

inline void vectorMAInline(const float* start, float scale, const float* direction, float* dest)
{
	dest[0] = start[0] + direction[0] * scale;
	dest[1] = start[1] + direction[1] * scale;
	dest[2] = start[2] + direction[2] * scale;
}

inline void vectorMAInline(const vector& start, float scale, const vector& direction, vector& dest)
{
	dest.x = start.x + direction.x * scale;
	dest.y = start.y + direction.y * scale;
	dest.z = start.z + direction.z * scale;
}

inline void vectorMA(const vector& start, float scale, const vector& direction, vector& dest)
{
	vectorMAInline(start, scale, direction, dest);
}

inline void vectorMA(const float* start, float scale, const float* direction, float* dest)
{
	vectorMAInline(start, scale, direction, dest);
}


class ALIGN16 vectorAligned : public vector
{
public:
	inline vectorAligned(void) {};
	inline vectorAligned(float X, float Y, float Z)
	{
		init(X, Y, Z);
	}

#ifdef vector_NO_SLOW_OPERATIONS

private:
	// No copy constructors allowed if we're in optimal mode
	vectorAligned(const vectorAligned& vOther);
	vectorAligned(const vector& vOther);

#else
public:
	explicit vectorAligned(const vector& vOther)
	{
		init(vOther.x, vOther.y, vOther.z);
	}

	vectorAligned& operator=(const vector& vOther)
	{
		init(vOther.x, vOther.y, vOther.z);
		return *this;
	}

#endif
	float w;	// this space is used anyway
};


inline unsigned long& FloatBits(float& f)
{
	return *reinterpret_cast<unsigned long*>(&f);
}

inline bool IsFinite(float f)
{
	return ((FloatBits(f) & 0x7F800000) != 0x7F800000);
}

class vector2
{
public:
	// Members
	float x, y;

	// Construction/destruction
	vector2(void);
	vector2(float X, float Y);
	vector2(const float* pFloat);

	// Initialization
	void init(float ix = 0.0f, float iy = 0.0f);

	// Got any nasty NAN's?
	bool is_valid() const;

	// array access...
	float  operator[](int i) const;
	float& operator[](int i);

	// Base address...
	float* base();
	float const* base() const;

	// Initialization methods
	void random(float minVal, float maxVal);

	// equality
	bool operator==(const vector2& v) const;
	bool operator!=(const vector2& v) const;

	// arithmetic operations
	vector2& operator+=(const vector2& v);
	vector2& operator-=(const vector2& v);
	vector2& operator*=(const vector2& v);
	vector2& operator*=(float           s);
	vector2& operator/=(const vector2& v);
	vector2& operator/=(float           s);

	// negate the vector_2d components
	void Negate();

	// Get the vector_2d's magnitude.
	float Length() const;

	// Get the vector_2d's magnitude squared.
	float LengthSqr(void) const;

	// return true if this vector is (0,0) within tolerance
	bool IsZero(float tolerance = 0.01f) const
	{
		return (x > -tolerance && x < tolerance &&
			y > -tolerance && y < tolerance);
	}

	float Normalize();

	// Normalize in place and return the old length.
	float NormalizeInPlace();

	// Compare length.
	bool IsLengthGreaterThan(float val) const;
	bool IsLengthLessThan(float    val) const;

	// Get the distance from this vector_2d to the other one.
	float DistTo(const vector2& vOther) const;

	// Get the distance from this vector_2d to the other one squared.
	float DistToSqr(const vector2& vOther) const;

	// Copy
	void CopyToArray(float* rgfl) const;

	// Multiply, add, and assign to this (ie: *this = a + b * scalar). This
	// is about 12% faster than the actual vector_2d equation (because it's done per-component
	// rather than per-vector_2d).
	void MulAdd(const vector2& a, const vector2& b, float scalar);

	// Dot product.
	float Dot(const vector2& vOther) const;

	// assignment
	vector2& operator=(const vector2& vOther);

#ifndef vector_NO_SLOW_OPERATIONS
	// copy constructors
	vector2(const vector2& vOther);

	// arithmetic operations
	vector2 operator-(void) const;

	vector2 operator+(const vector2& v) const;
	vector2 operator-(const vector2& v) const;
	vector2 operator*(const vector2& v) const;
	vector2 operator/(const vector2& v) const;
	vector2 operator+(const int       i1) const;
	vector2 operator+(const float     fl) const;
	vector2 operator*(const float     fl) const;
	vector2 operator/(const float     fl) const;

	// Cross product between two vectors.
	vector2 Cross(const vector2& vOther) const;

	// Returns a vector_2d with the min or max in X, Y, and Z.
	vector2 Min(const vector2& vOther) const;
	vector2 Max(const vector2& vOther) const;

#else

private:
	// No copy constructors allowed if we're in optimal mode
	vector_2d(const vector_2d& vOther);
#endif
};

//-----------------------------------------------------------------------------

const vector2 vec2_origin(0, 0);
//const vector_2d vec2_invalid(3.40282347E+38F, 3.40282347E+38F);

//-----------------------------------------------------------------------------
// vector_2d related operations
//-----------------------------------------------------------------------------

// vector_2d clear
void vector_2dClear(vector2& a);

// Copy
void vector_2dCopy(const vector2& src, vector2& dst);

// vector_2d arithmetic
void vector_2dAdd(const vector2& a, const vector2& b, vector2& result);
void vector_2dSubtract(const vector2& a, const vector2& b, vector2& result);
void vector_2dMultiply(const vector2& a, float           b, vector2& result);
void vector_2dMultiply(const vector2& a, const vector2& b, vector2& result);
void vector_2dDivide(const vector2& a, float           b, vector2& result);
void vector_2dDivide(const vector2& a, const vector2& b, vector2& result);
void vector_2dMA(const vector2& start, float       s, const vector2& dir, vector2& result);

// Store the min or max of each of x, y, and z into the result.
void vector_2dMin(const vector2& a, const vector2& b, vector2& result);
void vector_2dMax(const vector2& a, const vector2& b, vector2& result);

#define vector_2dExpand( v ) (v).x, (v).y

// Normalization
float vector_2dNormalize(vector2& v);

// Length
float vector_2dLength(const vector2& v);

// Dot Product
float DotProduct2D(const vector2& a, const vector2& b);

// Linearly interpolate between two vectors
void vector_2dLerp(const vector2& src1, const vector2& src2, float t, vector2& dest);


//-----------------------------------------------------------------------------
//
// Inlined vector_2d methods
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// constructors
//-----------------------------------------------------------------------------

inline vector2::vector2(void)
{
#ifdef _DEBUG
	// Initialize to NAN to catch errors
	//x = y = float_NAN;
#endif
}

inline vector2::vector2(float X, float Y)
{
	x = X; y = Y;
	Assert(is_valid());
}

inline vector2::vector2(const float* pFloat)
{
	Assert(pFloat);
	x = pFloat[0]; y = pFloat[1];
	Assert(is_valid());
}


//-----------------------------------------------------------------------------
// copy constructor
//-----------------------------------------------------------------------------

inline vector2::vector2(const vector2& vOther)
{
	Assert(vOther.is_valid());
	x = vOther.x; y = vOther.y;
}

//-----------------------------------------------------------------------------
// initialization
//-----------------------------------------------------------------------------

inline void vector2::init(float ix, float iy)
{
	x = ix; y = iy;
	Assert(is_valid());
}

inline void vector2::random(float minVal, float maxVal)
{
	x = minVal + ((float)rand() / VALVE_RAND_MAX) * (maxVal - minVal);
	y = minVal + ((float)rand() / VALVE_RAND_MAX) * (maxVal - minVal);
}

inline void vector_2dClear(vector2& a)
{
	a.x = a.y = 0.0f;
}

//-----------------------------------------------------------------------------
// assignment
//-----------------------------------------------------------------------------

inline vector2& vector2::operator=(const vector2& vOther)
{
	Assert(vOther.is_valid());
	x = vOther.x; y = vOther.y;
	return *this;
}

//-----------------------------------------------------------------------------
// Array access
//-----------------------------------------------------------------------------

inline float& vector2::operator[](int i)
{
	Assert((i >= 0) && (i < 2));
	return ((float*)this)[i];
}

inline float vector2::operator[](int i) const
{
	Assert((i >= 0) && (i < 2));
	return ((float*)this)[i];
}

//-----------------------------------------------------------------------------
// Base address...
//-----------------------------------------------------------------------------

inline float* vector2::base()
{
	return (float*)this;
}

inline float const* vector2::base() const
{
	return (float const*)this;
}

//-----------------------------------------------------------------------------
// is_valid?
//-----------------------------------------------------------------------------

inline bool vector2::is_valid() const
{
	return IsFinite(x) && IsFinite(y);
}

//-----------------------------------------------------------------------------
// comparison
//-----------------------------------------------------------------------------

inline bool vector2::operator==(const vector2& src) const
{
	Assert(src.is_valid() && is_valid());
	return (src.x == x) && (src.y == y);
}

inline bool vector2::operator!=(const vector2& src) const
{
	Assert(src.is_valid() && is_valid());
	return (src.x != x) || (src.y != y);
}


//-----------------------------------------------------------------------------
// Copy
//-----------------------------------------------------------------------------

inline void vector_2dCopy(const vector2& src, vector2& dst)
{
	Assert(src.is_valid());
	dst.x = src.x;
	dst.y = src.y;
}

inline void	vector2::CopyToArray(float* rgfl) const
{
	Assert(is_valid());
	Assert(rgfl);
	rgfl[0] = x; rgfl[1] = y;
}

//-----------------------------------------------------------------------------
// standard math operations
//-----------------------------------------------------------------------------

inline void vector2::Negate()
{
	Assert(is_valid());
	x = -x; y = -y;
}

inline vector2& vector2::operator+=(const vector2& v)
{
	Assert(is_valid() && v.is_valid());
	x += v.x; y += v.y;
	return *this;
}

inline vector2& vector2::operator-=(const vector2& v)
{
	Assert(is_valid() && v.is_valid());
	x -= v.x; y -= v.y;
	return *this;
}

inline vector2& vector2::operator*=(float fl)
{
	x *= fl;
	y *= fl;
	Assert(is_valid());
	return *this;
}

inline vector2& vector2::operator*=(const vector2& v)
{
	x *= v.x;
	y *= v.y;
	Assert(is_valid());
	return *this;
}

inline vector2& vector2::operator/=(float fl)
{
	Assert(fl != 0.0f);
	float oofl = 1.0f / fl;
	x *= oofl;
	y *= oofl;
	Assert(is_valid());
	return *this;
}

inline vector2& vector2::operator/=(const vector2& v)
{
	Assert(v.x != 0.0f && v.y != 0.0f);
	x /= v.x;
	y /= v.y;
	Assert(is_valid());
	return *this;
}

inline void vector_2dAdd(const vector2& a, const vector2& b, vector2& c)
{
	Assert(a.is_valid() && b.is_valid());
	c.x = a.x + b.x;
	c.y = a.y + b.y;
}

inline void vector_2dAdd(const vector2& a, const int b, vector2& c)
{
	Assert(a.is_valid());
	c.x = a.x + b;
	c.y = a.y + b;
}

inline void vector_2dAdd(const vector2& a, const float b, vector2& c)
{
	Assert(a.is_valid());
	c.x = a.x + b;
	c.y = a.y + b;
}

inline void vector_2dSubtract(const vector2& a, const vector2& b, vector2& c)
{
	Assert(a.is_valid() && b.is_valid());
	c.x = a.x - b.x;
	c.y = a.y - b.y;
}

inline void vector_2dMultiply(const vector2& a, const float b, vector2& c)
{
	Assert(a.is_valid() && IsFinite(b));
	c.x = a.x * b;
	c.y = a.y * b;
}

inline void vector_2dMultiply(const vector2& a, const vector2& b, vector2& c)
{
	Assert(a.is_valid() && b.is_valid());
	c.x = a.x * b.x;
	c.y = a.y * b.y;
}


inline void vector_2dDivide(const vector2& a, const float b, vector2& c)
{
	Assert(a.is_valid());
	Assert(b != 0.0f);
	float oob = 1.0f / b;
	c.x = a.x * oob;
	c.y = a.y * oob;
}

inline void vector_2dDivide(const vector2& a, const vector2& b, vector2& c)
{
	Assert(a.is_valid());
	Assert((b.x != 0.0f) && (b.y != 0.0f));
	c.x = a.x / b.x;
	c.y = a.y / b.y;
}

inline void vector_2dMA(const vector2& start, float s, const vector2& dir, vector2& result)
{
	Assert(start.is_valid() && IsFinite(s) && dir.is_valid());
	result.x = start.x + s * dir.x;
	result.y = start.y + s * dir.y;
}

// FIXME: Remove
// For backwards compatability
inline void	vector2::MulAdd(const vector2& a, const vector2& b, float scalar)
{
	x = a.x + b.x * scalar;
	y = a.y + b.y * scalar;
}

inline void vector_2dLerp(const vector2& src1, const vector2& src2, float t, vector2& dest)
{
	dest[0] = src1[0] + (src2[0] - src1[0]) * t;
	dest[1] = src1[1] + (src2[1] - src1[1]) * t;
}

//-----------------------------------------------------------------------------
// dot, cross
//-----------------------------------------------------------------------------
inline float DotProduct2D(const vector2& a, const vector2& b)
{
	Assert(a.is_valid() && b.is_valid());
	return(a.x * b.x + a.y * b.y);
}

// for backwards compatability
inline float vector2::Dot(const vector2& vOther) const
{
	return DotProduct2D(*this, vOther);
}


//-----------------------------------------------------------------------------
// length
//-----------------------------------------------------------------------------
inline float vector_2dLength(const vector2& v)
{
	Assert(v.is_valid());
	return (float)FastSqrt(v.x * v.x + v.y * v.y);
}

inline float vector2::LengthSqr(void) const
{
	Assert(is_valid());
	return (x * x + y * y);
}

inline float vector2::NormalizeInPlace()
{
	return vector_2dNormalize(*this);
}

inline bool vector2::IsLengthGreaterThan(float val) const
{
	return LengthSqr() > val * val;
}

inline bool vector2::IsLengthLessThan(float val) const
{
	return LengthSqr() < val * val;
}

inline float vector2::Length(void) const
{
	return vector_2dLength(*this);
}


inline void vector_2dMin(const vector2& a, const vector2& b, vector2& result)
{
	result.x = (a.x < b.x) ? a.x : b.x;
	result.y = (a.y < b.y) ? a.y : b.y;
}


inline void vector_2dMax(const vector2& a, const vector2& b, vector2& result)
{
	result.x = (a.x > b.x) ? a.x : b.x;
	result.y = (a.y > b.y) ? a.y : b.y;
}


//-----------------------------------------------------------------------------
// Normalization
//-----------------------------------------------------------------------------
inline float vector_2dNormalize(vector2& v)
{
	Assert(v.is_valid());
	float l = v.Length();
	if (l != 0.0f)
	{
		v /= l;
	}
	else
	{
		v.x = v.y = 0.0f;
	}
	return l;
}


//-----------------------------------------------------------------------------
// Get the distance from this vector_2d to the other one
//-----------------------------------------------------------------------------
inline float vector2::DistTo(const vector2& vOther) const
{
	vector2 delta;
	vector_2dSubtract(*this, vOther, delta);
	return delta.Length();
}

inline float vector2::DistToSqr(const vector2& vOther) const
{
	vector2 delta;
	vector_2dSubtract(*this, vOther, delta);
	return delta.LengthSqr();
}


//-----------------------------------------------------------------------------
// Computes the closest point to vecTarget no farther than flMaxDist from vecStart
//-----------------------------------------------------------------------------
inline void ComputeClosestPoint2D(const vector2& vecStart, float flMaxDist, const vector2& vecTarget, vector2* pResult)
{
	vector2 vecDelta;
	vector_2dSubtract(vecTarget, vecStart, vecDelta);
	float flDistSqr = vecDelta.LengthSqr();
	if (flDistSqr <= flMaxDist * flMaxDist)
	{
		*pResult = vecTarget;
	}
	else
	{
		vecDelta /= FastSqrt(flDistSqr);
		vector_2dMA(vecStart, flMaxDist, vecDelta, *pResult);
	}
}



//-----------------------------------------------------------------------------
//
// Slow methods
//
//-----------------------------------------------------------------------------

#ifndef vector_NO_SLOW_OPERATIONS
#endif
//-----------------------------------------------------------------------------
// Returns a vector_2d with the min or max in X, Y, and Z.
//-----------------------------------------------------------------------------

inline vector2 vector2::Min(const vector2& vOther) const
{
	return vector2(x < vOther.x ? x : vOther.x,
		y < vOther.y ? y : vOther.y);
}

inline vector2 vector2::Max(const vector2& vOther) const
{
	return vector2(x > vOther.x ? x : vOther.x,
		y > vOther.y ? y : vOther.y);
}


//-----------------------------------------------------------------------------
// arithmetic operations
//-----------------------------------------------------------------------------

inline vector2 vector2::operator-(void) const
{
	return vector2(-x, -y);
}

inline vector2 vector2::operator+(const vector2& v) const
{
	vector2 res;
	vector_2dAdd(*this, v, res);
	return res;
}

inline vector2 vector2::operator-(const vector2& v) const
{
	vector2 res;
	vector_2dSubtract(*this, v, res);
	return res;
}

inline vector2 vector2::operator+(const int i1) const
{
	vector2 res;
	vector_2dAdd(*this, i1, res);
	return res;
}

inline vector2 vector2::operator+(const float fl) const
{
	vector2 res;
	vector_2dAdd(*this, fl, res);
	return res;
}

inline vector2 vector2::operator*(const float fl) const
{
	vector2 res;
	vector_2dMultiply(*this, fl, res);
	return res;
}

inline vector2 vector2::operator*(const vector2& v) const
{
	vector2 res;
	vector_2dMultiply(*this, v, res);
	return res;
}

inline vector2 vector2::operator/(const float fl) const
{
	vector2 res;
	vector_2dDivide(*this, fl, res);
	return res;
}

inline vector2 vector2::operator/(const vector2& v) const
{
	vector2 res;
	vector_2dDivide(*this, v, res);
	return res;
}

inline vector2 operator*(const float fl, const vector2& v)
{
	return v * fl;
}

using QAngle = vector;


struct rect_t { int x, y, w, h; };