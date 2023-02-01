#define _PI  3.141592654f
#define _2PI  6.283185307f
#define _1DIVPI  0.318309886f
#define _1DIV2PI  0.159154943f
#define _PIDIV2  1.570796327f
#define _PIDIV4  0.785398163f

#define POW_CLAMP 0.000001f

float Pow2(float x) {return x * x;}

float Pow3(float x) {return x * x * x;}

float Pow4(float x) {return x * x * x * x;}

float Pow5(float x) {return x * x * x * x * x;}

float SmoothStep(float e0, float e1, float x) { 
    float t = saturate((x - e0) / e1 - e0);
    return t * t * (3.0 - 2.0 * t);
}

float Dot(float4 x, float4 y) { return x.x * y.x + x.y * y.y + x.z * y.z + x.w * y.w; }

uint AlignUp (float a, float b) {
    return uint((a + (b - 1)) / b);
}

#define GreaterThan(A, B) ((A) > (B))
#define GreaterThanEqual(A, B) ((A) >= (B))
#define LessThan(A, B) ((A) < (B))
#define LessThanEqual(A, B) ((A) <= (B))

#define AllGreaterThan(X, Y) all(GreaterThan(X, Y))
#define AllGreaterThanEqual(X, Y) all(GreaterThanEqual(X, Y))
#define AllLessThan(X, Y) all(LessThan(X, Y))
#define AllLessThanEqual(X, Y) all(LessThanEqual((X), (Y)))

#define AnyGreaterThan(X, Y) any(GreaterThan(X, Y))
#define AnyGreaterThanEqual(X, Y) any(GreaterThanEqual(X, Y))
#define AnyLessThan(X, Y) any(LessThan(X, Y))
#define AnyLessThanEqual(X, Y) any(LessThanEqual((X), (Y)))