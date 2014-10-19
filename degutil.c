/*
 * $Id: degutil.c,v 1.1.1.1 2003/03/16 07:03:49 kenta Exp $
 *
 * Copyright 2003 Kenta Cho. All rights reserved.
 */

/**
 * Changing the cordinate into the angle.
 *
 * @version $Revision: 1.1.1.1 $
 */
#include <math.h>
#include "degutil.h"

//senquack - after converting everything to fixed point, we can now use
//    this internal tan table for tan computations elsewhere:
//static int tantbl[TAN_TABLE_SIZE+2];
int tantbl[TAN_TABLE_SIZE + 2];

int sctbl[SC_TABLE_SIZE + SC_TABLE_SIZE / 4];

#if defined (FIXEDMATH) && (defined(ARM) || defined (GP2X))
//senquack - fast ARM ASM 16:16 fixed point divide routine:
//             Credit goes to Henry Thomas and the website is
//             http://me.henri.net/fp-div.html
//int32_t fpdiv(register int32_t numerator, register int32_t denominator)
int
fpdiv (register int numerator, register int denominator)
{
//senquack
//    register int32_t quotient;
   register int quotient;
//    asm("num     .req %[numerator]      @ Map Register Equates\n\t"
//    __asm__ volatile ("num     .req %[numerator]      @ Map Register Equates\n\t"
   asm volatile ("num     .req %[numerator]      @ Map Register Equates\n\t"
                 "den     .req %[denominator]\n\t"
                 "mod     .req r2\n\t"
                 "cnt     .req r3\n\t"
                 "quo     .req r4\n\t" "sign    .req r12\n\t"
                 /* set sign and ensure numerator and denominator are positive */
                 "cmp den, #0                    @ exceptioin if den == zero\n\t"
                 "beq .div0\n\t"
                 "eor sign, num, den             @ sign = num ^ den\n\t"
                 "rsbmi den, den, #0             @ den = -den if den < 0\n\t"
                 "subs mod, den, #1              @ mod = den - 1\n\t"
                 "beq .div1                      @ return if den == 1\n\t"
                 "movs cnt, num                  @ num = -num if num < 0\n\t"
                 "rsbmi num, num, #0\n\t"
                 /* skip if deniminator >= numerator */
                 "movs cnt, num, lsr #16         @ return if den >= num << 16\n\t"
                 "bne .cont\n\t"
                 "cmp den, num, lsl #16\n\t" "bhs .numLeDen\n\t"
                 "\n.cont:\n\t"
                 /* test if denominator is a power of two */
                 "tst den, mod                   @ if(den & (den - 1) == 0)\n\t"
//senquack - was missing terminating quote here:
                 "beq .powerOf2                  @ den is power of 2\n\t"
                 /* count leading zeros */
                 "stmfd sp!, {r4}                @ push r4 (quo) onto the stack\n\t"
                 "mov cnt, #28                   @ count difference in leading zeros\n\t"
                 "mov mod, num, lsr #4           @ between num and den\n\t"
                 "cmp den, mod, lsr #12; subls cnt, cnt, #16; movls mod, mod, lsr #16\n\t"
                 "cmp den, mod, lsr #4 ; subls cnt, cnt, #8 ; movls mod, mod, lsr #8\n\t"
                 "cmp den, mod         ; subls cnt, cnt, #4 ; movls mod, mod, lsr #4\n\t"
                 /* shift numerator left by cnt bits */
                 "mov num, num, lsl cnt          @ mod:num = num << cnt\n\t"
                 "mov quo, #0\n\t"
                 "rsb den, den, #0               @ negate den for divide loop\n\t"
                 /* skip cnt iterations in the divide loop */
                 "adds num, num, num             @ start: num = mod:num / den\n\t"
                 "add pc, pc, cnt, lsl #4        @ skip cnt x 4 x 4 iterations\n\t"
                 "nop                            @ nop instruction takes care of pipeline\n\t"
                 /* inner loop unrolled x 48 */
                 ".rept 47                       @ inner loop x 48\n\t"
                 "    adcs mod, den, mod, lsl #1\n\t"
                 "    subcc mod, mod, den\n\t"
                 "    adc quo, quo, quo\n\t"
                 "    adds num, num, num\n\t"
                 ".endr\n\t"
                 "adcs mod, den, mod, lsl #1\n\t"
                 "subcc mod, mod, den\n\t" "adc quo, quo, quo\n\t"
                 /* negate quotient if signed */
                 "cmp sign, #0                   @ negate quotient if sign < 0\n\t"
                 "mov num, quo\n\t"
                 "rsbmi num, num, #0\n\t"
                 "ldmfd sp!, {r4}                @ pop r4 (quo) off the stack\n\t"
                 "mov pc, lr                     @return\n\t"
                 /* divide by zero handler */
                 "\n.div0:\n\t"
                 "mov num, #0\n\t"
                 "mov pc, lr                     @return\n\t"
                 /* divide by one handler */
                 "\n.div1:\n\t"
                 "cmp sign, #0\n\t"
                 "mov num, num, asl #16\n\t"
                 "rsbmi num, num, #0\n\t"
                 "mov pc, lr                     @return\n\t"
                 /* numerator less than or equal to denominator handler */
                 "\n.numLeDen:\n\t"
                 "mov num, #0                    @ quotient = 0 if num < den\n\t"
                 "moveq num, sign, asr #31       @ negate quotient if sign < 0\n\t"
                 "orreq num, num, #1             @ quotient = 1 if num == den\n\t"
                 "mov pc, lr                     @return\n\t"
                 /* power of two handler */
                 "\n.powerOf2:\n\t"
                 "mov cnt, #0\n\t"
                 "cmp den, #(1 << 16); movhs cnt, #16    ; movhs den, den, lsr #16\n\t"
                 "cmp den, #(1 << 8) ; addhs cnt, cnt, #8; movhs den, den, lsr #8\n\t"
                 "cmp den, #(1 << 4) ; addhs cnt, cnt, #4; movhs den, den, lsr #4\n\t"
                 "cmp den, #(1 << 2) ; addhi cnt, cnt, #3; addls cnt, cnt, den, lsr #1\n\t"
                 "rsb mod, cnt, #32\n\t"
                 "mov den, num, lsr #16          @ den:num = num << 16\n\t"
                 "mov num, num, lsl #16\n\t"
                 "mov num, num, lsr cnt          @ num = num >> cnt | den << mod\n\t"
                 "orr num, num, den, lsl mod\n\t"
                 "cmp sign, #0\n\t"
                 "rsbmi num, num, #0             @ negate quotient if sign < 0"
                 /* output registers */
                 :[quotient] "=r" (quotient)
                 /* input registers */
                 :[numerator] "0" (numerator),[denominator] "r" (denominator)
                 /* clobbered registers */
                 :"r2" /* mod */ , "r3" /* cnt */ , "r12" /* sign */ );
   return quotient;
}
#endif

//senquack - the famous Quake square root, for speed in our gluLookat implementation
float
magic_sqrt (float number)
{
   long i;
   float f = 1.5, x = number / 2, y = number;
   i = *(unsigned long *) &y;
   i = 0x5f3759df - (i >> 1);
   y = *(float *) &i;
   y = y * (f - x * y * y);
   return number * y;
}

#ifdef FIXEDMATH
//senquack - credit for this fast sqrt goes to Wilco Dijkstra http://www.finesse.demon.co.uk/steven/sqrt.html
#define iter1(N) \
    try = root + (1 << (N)); \
    if (n >= try << (N))   \
    {   n -= try << (N);   \
        root |= 2 << (N); \
    }
//non-fixed point version (unmodified)
//uint32 fast_sqrt (uint32 n)
//{
//    uint32 root = 0, try;
//    iter1 (15);    iter1 (14);    iter1 (13);    iter1 (12);
//    iter1 (11);    iter1 (10);    iter1 ( 9);    iter1 ( 8);
//    iter1 ( 7);    iter1 ( 6);    iter1 ( 5);    iter1 ( 4);
//    iter1 ( 3);    iter1 ( 2);    iter1 ( 1);    iter1 ( 0);
//    return root >> 1;
//}
unsigned int
fpsqrt (unsigned int n)
{
   unsigned int root = 0, try;
   iter1 (15);
   iter1 (14);
   iter1 (13);
   iter1 (12);
   iter1 (11);
   iter1 (10);
   iter1 (9);
   iter1 (8);
   iter1 (7);
   iter1 (6);
   iter1 (5);
   iter1 (4);
   iter1 (3);
   iter1 (2);
   iter1 (1);
   iter1 (0);
//    return root >> 1;
   return root << 7;            //senquack - convert to 16.16 fixed point
}
#endif //FIXEDMATH

//// Fast integer square root adapted from algorithm, 
//  // Martin Guy @ UKC, June 1985.
//  // Originally from a book on programming abaci by Mr C. Woo.
// int fastSqrt(int n)
// {
//    /*
//     * Logically, these are unsigned. 
//   * We need the sign bit to test
//     * whether (op - res - one) underflowed.
//     */
//    int op, res, one;
//    op = n;
//    res = 0;
//    /* "one" starts at the highest power of four <= than the argument. */
//    one = 1 << 30; /* second-to-top bit set */
//    while (one > op) one >>= 2;
//    while (one != 0) 
//    {
//       if (op >= res + one) 
//       {
//          op = op - (res + one);
//          res = res +  (one<<1);
//       }
//       res >>= 1;
//       one >>= 2;
//    }
////     return(res);
//    return(res<<8);   //senquack - convert to 16.16 floating point
// }



void
initDegutil ()
{
   int i, d = 0;
   //senquack TODO: make sure conversion to floats from doubles here didn't mess up the bullet patterns, etc:
//senquack - complete conversion to floats:
//  double od = 6.28/DIV;
   float od = 6.28 / DIV;
   for (i = 0; i < TAN_TABLE_SIZE; i++) {
      while ((int) (sin (d * od) / cos (d * od) * TAN_TABLE_SIZE) < i)
         d++;
      tantbl[i] = d;
   }
   tantbl[TAN_TABLE_SIZE] = tantbl[TAN_TABLE_SIZE + 1] = 128;

   for (i = 0; i < SC_TABLE_SIZE + SC_TABLE_SIZE / 4; i++) {
      sctbl[i] = (int) (sin (i * (6.28 / SC_TABLE_SIZE)) * 256);
   }
}

int
getDeg (int x, int y)
{
   int tx, ty;
   int f, od, tn;

   if (x == 0 && y == 0) {
      return (512);
   }

   if (x < 0) {
      tx = -x;
      if (y < 0) {
         ty = -y;
         if (tx > ty) {
            f = 1;
            od = DIV * 3 / 4;
            tn = ty * TAN_TABLE_SIZE / tx;
         } else {
            f = -1;
            od = DIV;
            tn = tx * TAN_TABLE_SIZE / ty;
         }
      } else {
         ty = y;
         if (tx > ty) {
            f = -1;
            od = DIV * 3 / 4;
            tn = ty * TAN_TABLE_SIZE / tx;
         } else {
            f = 1;
            od = DIV / 2;
            tn = tx * TAN_TABLE_SIZE / ty;
         }
      }
   } else {
      tx = x;
      if (y < 0) {
         ty = -y;
         if (tx > ty) {
            f = -1;
            od = DIV / 4;
            tn = ty * TAN_TABLE_SIZE / tx;
         } else {
            f = 1;
            od = 0;
            tn = tx * TAN_TABLE_SIZE / ty;
         }
      } else {
         ty = y;
         if (tx > ty) {
            f = 1;
            od = DIV / 4;
            tn = ty * TAN_TABLE_SIZE / tx;
         } else {
            f = -1;
            od = DIV / 2;
            tn = tx * TAN_TABLE_SIZE / ty;
         }
      }
   }
   return ((od + tantbl[tn] * f) & (DIV - 1));
}

int
getDistance (int x, int y)
{
   if (x < 0)
      x = -x;
   if (y < 0)
      y = -y;
   if (x > y) {
      return x + (y >> 1);
   } else {
      return y + (x >> 1);
   }
}

float
getDistanceFloat (float x, float y)
{
   if (x < 0)
      x = -x;
   if (y < 0)
      y = -y;
   if (x > y) {
      return x + (y / 2);
   } else {
      return y + (x / 2);
   }
}
