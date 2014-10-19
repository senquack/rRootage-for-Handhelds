/*
 * $Id: degutil.h,v 1.1.1.1 2003/03/16 07:03:49 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Degutil header file.
 *
 * @version $Revision: 1.1.1.1 $
 */
#define DIV 1024
#define TAN_TABLE_SIZE 1024
#define SC_TABLE_SIZE 1024

extern int sctbl[];

//senquack - after converting everything to fixed point, we can now use
//    this internal tan table for tan computations elsewhere:
extern int tantbl[];

//senquack - added for fixed-point stuff:
#ifdef FIXEDMATH
#define FNUM_PI (GLfixed)(3.14159f*65536)   // 2^16=65536
//senquack - for wiz
#define f2x(x) ((int)((x) * 65536.0f))
#define x2f(x) ((float)(x) / 65536.0f)
#define FNUM GLfixed
// Convert from int to fixed number
#define INT2FNUM(x) ((x)<<16)
// Convert from fixed math to int
#define FNUM2INT(x) ((x)>>16)
#endif //FIXEDMATH


//#define FMUL(x,y) (int)(((x)*(y))>>16)
//#define FMUL(x,y) (int)(((long long)(x)*(long long)(y))>>16)
#define FMUL(x,y) (int)(((long long)(x)*(y))>>16)

//senquack - this appears to cause bugs unfortunately:
//senquack - fast ARM ASM 16:16 fixed point multiply routine:
//             Credit goes to Henry Thomas and the website is
//             http://me.henri.net/fp-div.html
//#define FMUL(A, B) ({ \
//    register int __a = (A), __b = (B), __c; asm( \
//    "smull r2, r3, %[a], %[b] @; i_fpmul()\n\t" \
//    "mov %[c], r2, lsr #16\n\t" \
//    "orr %[c], r2, r3, asl #16" \
//    : [c] "=r" (__c) : [a] "0" (__a), [b] "r" (__b) : "r2", "r3"); \
//    __c; })

//#define FDIV(x,y) ((x/y)>>16)
//#define FDIV(x,y) (((long long)(x)<<16) / (y))
#if defined(FIXEDMATH) 
#if (defined(ARM) || defined(GP2X))
#define FDIV(x,y) fpdiv(x,y)
//senquack - fast ARM ASM 16:16 fixed point divide routine:
//             Credit goes to Henry Thomas and the website is
//             http://me.henri.net/fp-div.html
int fpdiv (register int numerator, register int denominator);
#else
#define FDIV(x,y) (int32_t)(((int64_t)x << 16) / y)
#endif
#endif //FIXEDMATH


//senquack - fast fixed point sqrt:
#ifdef FIXEDMATH
unsigned fpsqrt (unsigned n);
#define FSQRT(x) fpsqrt(x)
//#define FSQRT(x) fastSqrt(x)
int fastSqrt (int n);
#endif //FIXEDMATH

//senquack - the famous Quake square root, for speed in our gluLookat implementation
float magic_sqrt (float number);

void initDegutil ();
int getDeg (int x, int y);
int getDistance (int x, int y);
float getDistanceFloat (float x, float y);
