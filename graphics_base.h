#pragma once
#include <cmath>
#include <vector>
#include <array>
#include <algorithm>

using byte = unsigned char;

template<typename T>
void remove(std::vector<T>& v, T& elem)
{
	v.erase(std::remove(v.begin(), v.end(), elem), v.end());
}

/*template<typename T>
void remove(std::basic_string<T>& s, size_t pos)
{
	s.erase(pos);
	v.erase(std::remove(v.begin(), v.end(), elem), v.end());
}*/

bool is_finite(double x){ return std::isfinite(x); }
bool is_finite(float x){ return std::isfinite(x); }
bool is_finite(long double x){ return std::isfinite(x); }

template<typename T, typename R = std::enable_if_t<std::is_integral<T>::value>>
bool is_finite(T x){ return true; }

constexpr double pi = 3.14159265358979323846264338327950288419716939937510582097;
constexpr double deg2rad = pi/180;
constexpr double rad2deg = 180/pi;

template<typename T> T sq  (T x){ return x*x;   }
template<typename T> T cube(T x){ return x*x*x; }

template<typename F>
auto sum(int n, F&& f)
{
	std::result_of_t<F(int)> acc = 0;
	for(int i=0; i<n; ++i){ acc += f(i); }
	return acc;
}

template<typename T> T ChebyshevT(int n, T const& x){ return cos(n*acos(x)); }

//2D geometry related:

template<typename T>
auto clamp(T x, T min, T max){ return x < min ? min : (x > max ? max : x); }

template<typename T, typename U, typename R = float>
auto rescale( U const& old_min, U const& old_max, U const& old_val, T const& new_min, T const& new_max )
{
	R r = ((R)(old_val - old_min)) / ((R)(old_max - old_min));
	return (T)(new_min + (r * (new_max - new_min)));
}

template<typename T, typename U, typename R = float>
auto rescalewc( U const& old_min, U const& old_max, U const& old_val, T const& new_min, T const& new_max )
{
	if(old_val <= old_min){ return new_min; }
	if(old_val >= old_max){ return new_max; }
	R r = ((R)(old_val - old_min)) / ((R)(old_max - old_min));
	return (T)(new_min + (r * (new_max - new_min)));
}

template<typename T> struct pos2
{
	T x, y;
	auto& operator+=(pos2<T> const& v){ x += v.x; y += v.y; return *this; }
	auto& operator-=(pos2<T> const& v){ x -= v.x; y -= v.y; return *this; }
};
template<typename T> auto operator-(pos2<T> p, pos2<T> q){ return pos2<T>{p.x-q.x, p.y-q.y}; }
template<typename T> auto operator+(pos2<T> p, pos2<T> q){ return pos2<T>{p.x+q.x, p.y+q.y}; }
template<typename T> auto operator*(pos2<T> p, T c){ return pos2<T>{p.x*c, p.y*c}; }
template<typename T> auto operator/(pos2<T> p, T c){ return pos2<T>{p.x/c, p.y/c}; }

template<typename T> struct size2{ T w, h; T area() const { return w*h; } };

template<typename T> auto operator+(size2<T> p, size2<T> q){ return size2<T>{p.w+q.w, p.h+q.h}; }
template<typename T> auto operator-(size2<T> p, size2<T> q){ return size2<T>{p.w-q.w, p.h-q.h}; }

template<typename T> struct rect2
{
	T x, y, w, h;
	void zero(){ x = y = w = h = (T)0; }
	T area() const { return w*h; }
	pos2<T>  pos()  const { return {x, y}; }
	size2<T> size() const { return {w, h}; }
	void operator=( pos2<T> inp){ x = inp.x; y = inp.y; }
	void operator=(size2<T> inp){ w = inp.w; h = inp.h; }
	void shift_by( pos2<T> d ){ x += d.x; y += d.y; }
	void shift_to( pos2<T> d ){ x  = d.x; y  = d.y; }
};

enum class HAlign  : byte { NoAlign, OuterLeft, LeftCenter, InnerLeft, HCenter, InnerRight, RightCenter, OuterRight };
enum class VAlign  : byte { NoAlign, OuterTop, TopCenter, InnerTop, VCenter, InnerBottom, BottomCenter, OuterBottom };
enum class SizeRef : byte { NoSize, RefWidth, RefHeight, ContentWidth, ContentHeight }; 

using pos2i  = pos2<int>;
using size2i = size2<int>;
using rect2i = rect2<int>;

template<typename T>
bool is_inside( T p, T x, T w )
{
	if(x <= p && p <= x + w){ return true; }
	return false;
}

template<typename T>
bool is_inside( rect2<T> r, T x, T y )
{
	if(r.x <= x && x <= r.x + r.w &&
	   r.y <= y && y <= r.y + r.h ){ return true; }
	return false;
}
template<typename T> bool    is_inside ( rect2<T>  r, pos2<T> p ){ return is_inside(r, p.x, p.y); }
template<typename T> T       get_center( T x, T w ){ return x + w / 2; }
template<typename T> pos2<T> get_center( rect2<T>  r            ){ return pos2<T>{r.x + r.w/2, r.y + r.h/2}; }
template<typename T> void    center_to ( T& x, T w, T p ){ x = p - w / 2; }
template<typename T> void    center_to ( rect2<T>& r, pos2<T> p ){ r.x = p.x - r.w / 2; r.y = p.y - r.h / 2; }
template<typename T> T       left      ( rect2<T> const& r ){ return r.x; }
template<typename T> T       right     ( rect2<T> const& r ){ return r.x + r.w; }
template<typename T> T       hcenter   ( rect2<T> const& r ){ return r.x + r.w / 2; }
template<typename T> T       top       ( rect2<T> const& r ){ return r.y; }
template<typename T> T       bottom    ( rect2<T> const& r ){ return r.y + r.h; }
template<typename T> T       vcenter   ( rect2<T> const& r ){ return r.y + r.h / 2; }

template<typename T> pos2<T> left_bottom  ( rect2<T> const& r ){ return {r.x, r.y + r.h}; }
template<typename T> pos2<T> middle_bottom( rect2<T> const& r ){ return {r.x + r.w/2, r.y + r.h}; }
template<typename T> pos2<T> right_bottom ( rect2<T> const& r ){ return {r.x + r.w, r.y + r.h}; }
template<typename T> pos2<T> right_top    ( rect2<T> const& r ){ return {r.x + r.w, r.y}; }
template<typename T> pos2<T> left_top     ( rect2<T> const& r ){ return {r.x, r.y}; }

template<typename T> void  shrink_lo(T& x, T& w, T amount){ x += amount; w -= amount; }
template<typename T> void enlarge_lo(T& x, T& w, T amount){ x -= amount; w += amount; }
template<typename T> void  shrink_hi(T& x, T& w, T amount){ w -= amount; }
template<typename T> void enlarge_hi(T& x, T& w, T amount){ w += amount; }

template<typename T>
auto intersect( rect2<T> a, rect2<T> b )
{
	pos2<T> p = {std::max( left(a),  left(b)), std::max(   top(a),    top(b))};
	pos2<T> q = {std::min(right(a), right(b)), std::min(bottom(a), bottom(b))};
	return rect2<T>{p.x, p.y, q.x-p.x, q.y-p.y};
}

template<typename T>
void center_shrink( T& x, T& w, T gap )
{
	auto c = get_center(x, w);
	w = w - 2*gap;
	if(w < 0){ w = gap; }
	x = c - w/2;
}

template<typename T>
rect2<T> center_shrink( rect2<T> r0, T gapx, T gapy )
{
	auto r = r0;
	center_shrink(r.x, r.w, gapx);
	center_shrink(r.y, r.h, gapy);
	return r;
}

template<typename T>
std::vector<rect2<T>> subdivide( rect2<T> const& src, int nx, int ny, int gapx, int gapy )
{
	int w = nx < 2 ? src.w / nx : (src.w - gapx*(nx-1)) / nx;
	int h = ny < 2 ? src.h / ny : (src.h - gapy*(ny-1)) / ny;
	std::vector<rect2<T>> res; res.reserve((size_t)nx*(size_t)ny);
	int y = src.y;
	for(int j=0; j<ny; ++j)
	{
		int x = src.x;
		for(int i=0; i<nx; ++i)
		{
			res.push_back(rect2<int>{x, y, w, h});
			x += (w + gapx);
		}
		y += (h + gapy);
	}
	return res;
}

//Color related:
template<typename T> T normalized_max(){ return std::is_integral<T>::value ? std::numeric_limits<T>::max() : (T)1; }

/*template<typename T, typename R = std::enable_if<std::is_integral<T>::value>>
auto norm_to_fl(T      x){ return x / (float)normalized_max<T>(); }
auto norm_to_fl(float  x){ return x; }
auto norm_to_fl(double x){ return x; }

template<typename T, typename R = std::enable_if<std::is_integral<T>::value>>
auto norm_to_8i(T      x){ return x;     }
auto norm_to_8i(float  x){ return x*255; }
auto norm_to_8i(double x){ return x*255; }

template<typename T, typename R = std::enable_if<std::is_integral<T>::value>>
auto norm_to_16i(T      x){ return x;       }
auto norm_to_16i(float  x){ return x*65535; }
auto norm_to_16i(double x){ return x*65535; }*/

template<typename T> struct Color{ T b, g, r, a; };
using Color8 = Color<unsigned char>;

template<typename T> auto color(T r, T g, T b, T a){ return Color<T>{b, g, r, a}; }
auto color8(int r, int g, int b, int a = 255){ return Color<unsigned char>{(unsigned char)b, (unsigned char)g, (unsigned char)r, (unsigned char)a}; }

Color8 blend8(Color8 dst, unsigned char alpha, Color8 src)
{
	using U = unsigned short;
	U q  =          (U)alpha;
	U nq = (U)255 - (U)alpha;

	U b = (U)dst.b * nq + (U)src.b * q;
	U g = (U)dst.g * nq + (U)src.g * q;
	U r = (U)dst.r * nq + (U)src.r * q;
	U a = (U)dst.a * nq + (U)src.a * q;
	using T = unsigned char;
	return {(T)(b/255), (T)(g/255), (T)(r/255), (T)(a/255)};
}

template<typename T>
Color<T> recolor(T A, Color<T> bk, Color<T> fg)
{
	auto r = rescale<T, T, float>((T)0, normalized_max<T>(), A, bk.r, fg.r);
	auto g = rescale<T, T, float>((T)0, normalized_max<T>(), A, bk.g, fg.g);
	auto b = rescale<T, T, float>((T)0, normalized_max<T>(), A, bk.b, fg.b);
	auto a = rescale<T, T, float>((T)0, normalized_max<T>(), A, bk.a, fg.a);
	return color(r, g, b, a);
};

template<typename T>
Color<T> lighten(Color<T> c, float f)
{
	f += 1.0f;
	//return color8( (int)(c.r * f), (int)(c.g * f), (int)(c.b * f) );
	return color8( (int)clamp(c.r * f, 0.0f, 255.0f), (int)clamp(c.g * f, 0.0f, 255.0f), (int)clamp(c.b * f, 0.0f, 255.0f) );
	//return color8( clamp((int)(c.r * f), 0, 255), clamp((int)(c.g * f), 0, 255), clamp((int)(c.b * f), 0, 255) );
}

#ifdef _WIN32
template<typename T> unsigned long packed_color(Color<T> const& c){ return RGB((unsigned char)c.r, (unsigned char)c.g, (unsigned char)c.b); }
#else
template<typename T> unsigned long packed_color(Color<T> const& c){ return ((((unsigned long)c.a*256 + (unsigned long)c.r)*256)+(unsigned long)c.g)*256+(unsigned long)c.b; }
#endif

template<typename C>
struct Image2
{
	size2<int> sz;
	std::vector<C> data;

	Image2():sz{0, 0}, data{}{}

	int w() const { return sz.w; }
	int h() const { return sz.h; }
	size2<int> size() const { return sz; }
	rect2<int> rect() const { return {0, 0, sz.w, sz.h}; }

	void resize(size2<int> sz_)       { data.resize((size_t)sz_.area()     ); sz = sz_; }
	void resize(size2<int> sz_, C val){ data.resize((size_t)sz_.area(), val); sz = sz_; }

	C &      operator()(int x, int y)      { return data[(size_t)y*(size_t)sz.w+(size_t)x]; }
	C const& operator()(int x, int y)const { return data[(size_t)y*(size_t)sz.w+(size_t)x]; }

	C const& operator[](int i) const { return data[i]; }
	C&       operator[](int i)       { return data[i]; }

	void fill(C c){ std::fill(data.begin(), data.end(), c); }
};

std::array<int, 4> reduce_margins(Image2<unsigned char>& img)
{
	int w = img.w();
	int h = img.h();
	if(w == 0 && h == 0){ return {0,0,0,0}; }
	int xmin = w, xmax = 0, ymin = 0, ymax = 0;
	bool empty_from_up = true;
	bool prev_line_empty = true;
	for(int y=0; y<h; ++y)
	{
		int lxmin = 0;
		int lxmax = w - 1;
		for(int x=0;       x <  w; ++x){ if( img(x, y) == 0 ){ lxmin += 1; }else{ break; } }
		for(int x=w-1; x >= 0;     --x){ if( img(x, y) == 0 ){ lxmax -= 1; }else{ break; } }
		bool this_line_empty = lxmin > lxmax;
		if(this_line_empty)
		{
			if(empty_from_up){ ymin = y; }
			if(!prev_line_empty){ ymax = y; }
			
		}
		else
		{
			ymax = y;
			empty_from_up = false;
			xmin = std::min(xmin, lxmin);
			xmax = std::max(xmax, lxmax);
		}
		prev_line_empty = this_line_empty;
	}

	Image2<unsigned char> tmp;
	int neww = xmax-xmin+1;
	int newh = ymax-ymin+1;
	if(neww < 0 || newh < 0)
	{
		neww = 0;
		newh = 0;
	}
	tmp.resize({neww, newh});

	for(int y=0; y<tmp.h(); ++y)
	{
		for(int x=0; x<tmp.w(); ++x)
		{
			tmp(x, y) = img(x + xmin, y + ymin);
		}
	}

	std::swap(img.data, tmp.data);
	std::swap(img.sz,   tmp.sz  );

	return {xmin, xmax, ymin, ymax};
}

struct PrerenderedText
{
	Image2<unsigned char> img;
	rect2i text_align_box; //real text box to align to
	int baseline;          //measured from top
	int dh;                //height to next baseline

	PrerenderedText():baseline{0}, dh{0}{}
	void resize(int w, int h){ img.resize({w, h}, 0); }
	size2<int> size() const { return img.size(); }
	rect2<int> rect() const { return img.rect(); }

	void reduce_margins()
	{
		Image2<unsigned char> tmp = img;
		auto Ds = ::reduce_margins(tmp);
		if(tmp.rect().area() == 0){ text_align_box.w = img.w(); return; }

		baseline -= Ds[2];
		text_align_box.x -= Ds[0];
		text_align_box.y += Ds[2];
		std::swap(img.data, tmp.data);
		std::swap(img.sz,   tmp.sz);
	}
};

template<typename T>
struct Histogram1
{
	std::vector<int> data;
	T tmin, tmax;
	int n;

	Histogram1():tmin{(T)0}, tmax{(T)0}, n{0}{}

	void set(T tmin_in, T tmax_in, int n_in ){ tmin = tmin_in; tmax = tmax_in; n = n_in; data.resize(n, 0); }
	void add(T val)
	{
		if(val >= tmin && val < tmax){ data[rescale(tmin, tmax, val, 0, n-1)] += 1; }
	}

	void clear(){ std::fill(data.begin(), data.end(), 0); }
};

//2D vector struct:

template<typename T>
struct Vector2
{
	T x, y;

	auto& operator+=(Vector2<T> const& v){ x += v.x; y += v.y; return *this; }
	auto& operator-=(Vector2<T> const& v){ x -= v.x; y -= v.y; return *this; }
};

template<typename T> Vector2<T> operator+( Vector2<T> const& u, Vector2<T> const& v ){ return {u.x+v.x, u.y+v.y}; }
template<typename T> Vector2<T> operator-( Vector2<T> const& u, Vector2<T> const& v ){ return {u.x-v.x, u.y-v.y}; }
template<typename T> Vector2<T> operator*( T const& c, Vector2<T> const& v ){ return {c*v.x, c*v.y}; }
template<typename T> Vector2<T> operator*( Vector2<T> const& v, T const& c ){ return {v.x*c, v.y*c}; }
template<typename T> Vector2<T> operator/( Vector2<T> const& v, T const& c ){ return {v.x/c, v.y/c}; }
template<typename T> T dot( Vector2<T> const& u, Vector2<T> const& v ){ return u.x*v.x + u.y*v.y; }

template<typename T> T length( Vector2<T> const& v ){ return std::sqrt(dot(v, v)); }
template<typename T> T sqlength( Vector2<T> const& v ){ return dot(v, v); }
template<typename T> Vector2<T> normalize( Vector2<T> const& v ){ return v / length(v); }
/*template<typename T> Vector2<T> rotate( Vector2<T> const& v, T angle )
{
	auto ca = std::cos(angle);
	auto sa = std::sin(angle);
	return v * ca + cross(axis, v)*sa + axis*dot(axis, v)*(1 - ca);
}*/

//3D vector struct:

template<typename T>
struct Vector3
{
	T x, y, z;

	auto& operator+=(Vector3<T> const& v){ x += v.x; y += v.y; z += v.z; return *this; }
};


template<typename T> Vector3<T> operator+( Vector3<T> const& u, Vector3<T> const& v ){ return {u.x+v.x, u.y+v.y, u.z+v.z}; }
template<typename T> Vector3<T> operator-( Vector3<T> const& u, Vector3<T> const& v ){ return {u.x-v.x, u.y-v.y, u.z-v.z}; }
template<typename T> Vector3<T> operator*( T const& c, Vector3<T> const& v ){ return {c*v.x, c*v.y, c*v.z}; }
template<typename T> Vector3<T> operator*( Vector3<T> const& v, T const& c ){ return {v.x*c, v.y*c, v.z*c}; }
template<typename T> Vector3<T> operator/( Vector3<T> const& v, T const& c ){ return {v.x/c, v.y/c, v.z/c}; }
template<typename T> T dot( Vector3<T> const& u, Vector3<T> const& v ){ return u.x*v.x + u.y*v.y + u.z*v.z; }
template<typename T> Vector3<T> cross( Vector3<T> const& a, Vector3<T> const& b )
{
	return {a.y*b.z - a.z*b.y,
		    a.z*b.x - a.x*b.z,
		    a.x*b.y - a.y*b.x}; }
template<typename T> T length( Vector3<T> const& v ){ return std::sqrt(dot(v, v)); }
template<typename T> T sqlength( Vector3<T> const& v ){ return dot(v, v); }
template<typename T> Vector3<T> normalize( Vector3<T> const& v ){ return v / length(v); }
template<typename T> Vector3<T> rotate( Vector3<T> const& v, Vector3<T> const& axis, T angle )
{
	auto ca = std::cos(angle);
	auto sa = std::sin(angle);
	return v * ca + cross(axis, v)*sa + axis*dot(axis, v)*(1 - ca);
}

template<typename T>
struct Matrix3
{
	T a11, a12, a13;
	T a21, a22, a23;
	T a31, a32, a33;
};

template<typename T>
Matrix3<T> operator+(Matrix3<T> const& A, Matrix3<T> const& B)
{
	return {A.a11 + B.a11, A.a12 + B.a12, A.a13 + B.a13, A.a21 + B.a21, A.a22 + B.a22, A.a23 + B.a23, A.a31 + B.a31, A.a32 + B.a32, A.a33 + B.a33};
}

template<typename T>
Matrix3<T> operator-(Matrix3<T> const& A, Matrix3<T> const& B)
{
	return {A.a11 - B.a11, A.a12 - B.a12, A.a13 - B.a13, A.a21 - B.a21, A.a22 - B.a22, A.a23 - B.a23, A.a31 - B.a31, A.a32 - B.a32, A.a33 - B.a33};
}

template<typename T>
Matrix3<T> operator*(Matrix3<T> const& m, T const& c)
{
	return {m.a11 * c, m.a12 * c, m.a13 * c, m.a21 * c, m.a22 * c, m.a23 * c, m.a31 * c, m.a32 * c, m.a33 * c};
}

template<typename T>
Matrix3<T> operator/(Matrix3<T> const& m, T const& c)
{
	return {m.a11 / c, m.a12 / c, m.a13 / c, m.a21 / c, m.a22 / c, m.a23 / c, m.a31 / c, m.a32 / c, m.a33 / c};
}

template<typename T>
Matrix3<T> operator*(T const& c, Matrix3<T> const& m)
{
	return {c * m.a11, c * m.a12, c * m.a13, c * m.a21, c * m.a22, c * m.a23, c * m.a31, c * m.a32, c * m.a33};
}

template<typename T>
Vector3<T> operator*(Matrix3<T> const& m, Vector3<T> const& v)
{
	T x = m.a11 * v.x + m.a12 * v.y + m.a13 * v.z;
	T y = m.a21 * v.x + m.a22 * v.y + m.a23 * v.z;
	T z = m.a31 * v.x + m.a32 * v.y + m.a33 * v.z;
	return Vector3<T>{x, y, z};
}

template<typename T>
T det(Matrix3<T> const& M)
{
	return M.a11 * (M.a22*M.a33 - M.a23*M.a32) + 
		   M.a12 * (M.a23*M.a31 - M.a21*M.a33) + 
		   M.a13 * (M.a21*M.a32 - M.a22*M.a31);
}

template<typename T>
Matrix3<T> invert(Matrix3<T> const& M)
{
	T A = (M.a22*M.a33 - M.a23*M.a32);
	T B = (M.a23*M.a31 - M.a21*M.a33);
	T C = (M.a21*M.a32 - M.a22*M.a31);

	T D = (M.a13*M.a32 - M.a12*M.a33);
	T E = (M.a11*M.a33 - M.a13*M.a31);
	T F = (M.a12*M.a31 - M.a11*M.a32);

	T G = (M.a12*M.a23 - M.a13*M.a22);
	T H = (M.a13*M.a21 - M.a11*M.a23);
	T I = (M.a11*M.a22 - M.a12*M.a21);

	T invdet = (T)1 / (M.a11 * A + M.a12 * B + M.a13 * C);

	return invdet * Matrix3<T>{A, D, G,
	                           B, E, H,
	                           C, F, I };
};

template<typename T>
Matrix3<T> transpose(Matrix3<T> const& M)
{
	return {M.a11, M.a21, M.a31,
		    M.a12, M.a22, M.a32,
		    M.a13, M.a23, M.a33};
}
