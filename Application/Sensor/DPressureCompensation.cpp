/**
* Baker Hughes Confidential
* Copyright 2020. Baker Hughes.
*
* NOTICE:  All information contained herein is, and remains the property of Baker Hughes and its suppliers, and
* affiliates if any.  The intellectual and technical concepts contained herein are proprietary to Baker Hughes
* and its suppliers and affiliates and may be covered by U.S. and Foreign Patents, patents in process, and are
* protected by trade secret or copyright law.  Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Baker Hughes.
*
* @file     DProductionTest.cpp
* @version  1.00.00
* @author   Harvinder Bhuhi
* @date     18 September 2020
*
* @brief    The production test functions source file
*/

#include <math.h>
#include "DPressureCompensation.h"

unsigned short psvi;
/************************************************************************/
/* spam -                                                               */
/* input parameters -                                                   */
/*                  coefs  Array of spam cubics                         */
/*                  n No of cubics (1 less than the number of points)   */
/*                  x the value to return the approximation for         */
/* output parameter                                                     */
/* Returns the approximation for value of x using the qubics previously */
/*   calculated by a spamfit() call                                     */
/************************************************************************/

float spam(sSpamCubic_t  *coefs, const short n, const float xa)
{
    // find right polynomial to use in table based on xa value
    // n is total number of polynomials in table coefs (last index is n-1)
    // poly.x marks LOWER bound of x range.
    // This is basically a search until we find that x is between
    // two bounding points - or we are off the table ends
    // Several tricks are exploited to minimise search time:
    //
    // There is no need to test last point in table at  n-1 as we default
    // to last interval  if x is outside range of table.
    //
    // Each trial would normally involve 2 compares in the form coefs[i].x < x < coefs[i+1].x
    // however, as search is sequencially decreasing (or increasing) i, one of the compares
    // has already been done - apart from first iteration.
    // So a search taking "m" trials takes m+1 compares instead of 2m compares.
    //
    // The application is assocaited with slow changing x values, thus sequential calls are more than likely
    // to yeild the same table position. We exploit this by always starting at the last known table position.
    //
    // Binary search techniques are not used as the total number of points is the table is modest - the above
    // optimisations are more efficient in a linear form.

    unsigned short i = psvi; //  Last used index is starting point
    unsigned short lower_limit = 1u;
    unsigned short upper_limit = static_cast<unsigned short>(n - 2);

    //  Sanity check - prevents problem when i start off at zero and we dont use end points
    if(i < lower_limit)
    {
        i = lower_limit;
    }

    else if(i > upper_limit)
    {
        i = upper_limit;
    }

    else
    {
        /* Do Nothing*/
    }

    sSpamCubic_t  *cptr = &coefs[i]; // use pointer we can increment

    // first decision is to work out if we should search higher or lower than current position

    if(xa >= cptr->x)
    {
        //search higher but stop  before comparing last position

        while(i < upper_limit)
        {
            //search higher in table
            if(xa < (cptr + 1)->x)
            {
                break; // exit if we have gone too far
            }

            cptr++;
            i++;
        }
    }

    else
    {
        //search lower but stop at bottom limit

        while(i > lower_limit)
        {
            i--;
            cptr--;

            if(xa >= cptr->x)
            {
                break;
            }
        }
    }

    // poly is relative to lower bound so we use x=xa-lowerbound
    // this small overhead is made up by better precision
    float x = xa - cptr->x;

    psvi = i; //Keep for next time.

    return (((cptr->kx3 * x + cptr->kx2) * x + cptr->kx) * x + cptr->k);
}

///////////////////////////////////////////////////////////////////////////
// Lagranges interpolation for quadratic
// 3 points (X1,y1)...(x2,y2) define unique Quadratic
// This function returns value (yp) of that quadratic at point xp
// This is  more accurate that solving the quadratic and then using polynomial
// It is also much faster when only one point is required.
//
float lagrange(const float xp, const float x[3], const float y[3])
{
    float yp;
    float pmx2_x1mx3, pmx3_x1mx2, pmx1_x2mx3;

    // Lagranges equation is
    //yi = (xi-x[2])*(xi-x[3])/((x[1]-x[2])*(x[1]-x[3]))*y[1]
    //+ (xi-x[1])*(xi-x[3])/((x[2]-x[1])*(x[2]-x[3]))*y[2]
    //+ (xi-x[1])*(xi-x[2])/((x[3]-x[1])*(x[3]-x[2]))*y[3];

    //define some common factors and reindex to base 1

    float    x1 = x[0];
    float    x2 = x[1];
    float    x3 = x[2];

    //pull out common sub_epressions to help compiler make fast code
    // and make it more readable
    pmx2_x1mx3 = (xp - x2) / (x1 - x3);
    pmx3_x1mx2 = (xp - x3) / (x1 - x2);
    pmx1_x2mx3 = (xp - x1) / (x2 - x3);

    //beware!  y index base or zero
    yp = (pmx2_x1mx3  * y[0] - pmx1_x2mx3 * y[1]) * pmx3_x1mx2 + pmx1_x2mx3 * pmx2_x1mx3 * y[2];

    return yp;
}

void spamfit(const point_t P[], const short n, sSpamCubic_t PolyArray[])
{
    short i;

    // Find Unique Polynomials to fit consequetive sets of 4 points
    // Pi-1,..i+2 where P0...n-1 represent an ordered set of data points.
    // The polynomial will be used for interpolation over range P=i..i+1
    //
    // The polynomial is unique cubic interpolating 4 data points and
    // unique quadratic interpolating 3 data points. Calculation method does not
    // change this FACT.
    //
    // The Newton form of the polynomial is created for accuracy and speed
    //  b0 + b1(x-x0) + b2(x-x0)(x-x1) + b3(x-x0)(x-x1)(x-x2)
    //  b0=y0,b1==x[i,k],b2=x[i,k,h],b3=x[i,k,h,m] for points xi,xk,xh and xm
    //  These coeffeicients are found by divided difference:
    //
    // [xi]=yi
    // [xik]=(yi-yk)/(xi-xk)
    // [xikh] = ([xik]-[xkh])/(xi-xh)
    // [xikhm] = ([xikh]-[xkhm])/(xi-xm)
    //
    // b0=y0
    // b1=[x10]=(y1-y0)/(x1-x0)
    // b2=[x210]=([x21]-[x10])/(x2-x0)
    // b3=[x3210]=([x321]-[x210])/(x3-x0)
    //
    // This is then converted to the expanded polynomial form A.x.x.x+ B.x.x.x + C.x + D
    // that are returned "poly" structure.
    // End points are treated differently. Here a quadratic is fitted to
    // first/last three points.
    // All equation x values are referenced to the second point xi,yi -which
    // also marks the start of the interpolation interval.
    //
    // Notes:
    // Newton form of the polynomial is simple to calculate and use recursive/incremental
    // methods - this allows minimal re-calculation when progressing thru the
    // data table.
    // It also has the advantage that it will EXACTLY pass thru the data points.
    // Evaluation of the Newton polynomial is also recursive and takes 3 multiply
    // and 6 add operations (cubic). So it could be used directly.
    // The expanded form loses the accuracy attribute but is slightly faster to
    // evaluate (3 multiple+3 add) and does not require the original point data.
    // The x-axis of the polynomial is referenced to the interval data point
    // to minimise errors in polynomial calculation. For example, with a large X offset,
    // the constant term of a polynomial becomes very large - much larger than the
    // data values - thus add/sub operation become significant sources or errors.
    // This problem is intrinics to the expanded form - Newtons form avoid this problem
    // completely.

    float xik[3], xikh[2];//divided differences for quadratic
    sSpamCubic_t *polyi; // a pointer for cubic[i]

    const point_t (*pp); //pointer to Xi,Yi
    polyi = PolyArray; //  pointer to poly[i]

    // first Interval 0...1
    // have to use quadratic for first interval
    // we also need to start of table of divided differences for
    // newton method will give quadratic polynomial with first 3 points (x0,y0) (x1,y & (x2,y2)
    //
    // Get pointer to points pp[0].x=x0,pp[0].y=y0 ( as moving is expensive)
    pp = &P[0];

    // calculate divide diffs for quadratic
    xik[0] = (pp[0].y - pp[1].y) / (pp[0].x - pp[1].x);
    xik[1] = (pp[1].y - pp[2].y) / (pp[1].x - pp[2].x);
    xikh[0] = (xik[0] - xik[1]) / (pp[0].x - pp[2].x);

    // Note:these will be in right place for start of cubic loop
    polyi += 1; //next poly

    // There are n-1 points which will give n-2 equations for intervals
    // This loop handles CUBIC interpolation
    //pp pointer points to first point of 4 data points pp[1] is start of interval pp[0] is also P[i-1]
    for(i = 1; i < (n - 2); i++)
    {
        //interpolation of points i-1 to i+2;
        //but we already have newton poly for i-1..i+1  so we only need to add i+2 data

        xik[2]  = (pp[2].y - pp[3].y) / (pp[2].x - pp[3].x);
        xikh[1] = (xik[1] - xik[2]) / (pp[1].x - pp[3].x);

        // We now have coefficients for Newtons poly
        // b0 + b1(x-x0) + b2(x-x0)(x-x1) + b3(x-x0)(x-x1)(x-x2)
        // where b0=y0, b1=xik[0], b2=xikh[0],b3=xikhm[0]
        //
        // Now re-calculate coefficients in basic form AXXX +BXX +CX + D
        // (not really a great idea as errors are poor - however it
        // is slightly fast to calculate and needs no datapoints
        //
        // We use heavily factored solution to reduce maths
        //
        //Z0=xikhm[0];
        //Z1=xikh[0]-Z0 * pp[2].x;
        //Z2=xik[0]-Z1 * pp[1].x;
        //
        // A=kx3=Z0
        // B=kx2=Z1-Z0*(pp[1].x+pp[0].x) also xikh[0]-Z0*(x[0]+x[1]+x[2]);
        // C=kx =Z2-pp[0].x*(polyi->kx2 + Z0.pp[0].x)
        // D=k  =pp[0].y-Z2 * pp[0].x
        //
        //   alternate method (- 3 less add)
        //     Z3=Z1-Z0*x[1];
        //     C=Z2-Z3*x[0];
        //     B=Z3-Z0*x[0];
        //
        // To restate polynomial at different x value (i.e. shift axis) the Newton divided diffs
        // are unchanged. We just substitute x0'=x0-xzero, x1'=x1-xzero, x2'=x2-xzero
        //
        // continue newtons method to find newton cubic that fits 4 points
        float xikhm = (xikh[0] - xikh[1]) / (pp[0].x - pp[3].x);

        // PRODUCE POLYNOMIAL where X=0 at point x1=x[i];
        // Note that newtons poly divided diffs  does not change at all if we relocate axis!!
        // just the data points do
        // but the expanded equation coefficients A,B,C,D do
        //common factors
        float p1m0 = pp[1].x - pp[0].x; //possible to avoid this if we save pp2-pp1 for next loop
        float Z1 = xikh[0] - xikhm * (pp[2].x - pp[1].x);

        // Following expands newton and collects terms using reloacted data points.
        polyi->kx3 = xikhm;
        polyi->kx2 = Z1 + xikhm * p1m0;
        polyi->kx  = xik[0] + Z1 * p1m0;
        polyi->k   = pp[1].y;          // we know k must be y1   as x1=0

        polyi->x = pp[1].x;    // save LOWER boundary of this poly
        polyi += 1; //next poly

        //We need to save coefficients for next time?

        // move newton coeffeicients into right place for next loop
        // basically we keep  quadratic part of newtons polynomial
        // x,y points dont have to be moved - as they are handled by pointer

        xik[0] = xik[1];
        xik[1] = xik[2];
        xikh[0] = xikh[1];
        pp++; // next data point
    }//for
}

/************************************************************************/
/* quadsolve                                                            */
/*                                                                      */
/* Finds the unique quadratic y = uXX + vX + w, passing through points  */
/*   p1, p2 & p3.                                                       */
/*  where X=x-p1[x]                                                     */
/* This axis shift greatly improve accuracy                             */
/************************************************************************/
void quadsolve(point_t p[3], sQuadratic_t &quad)
{
    float a = p[0].x;
    float b = p[1].x;
    float c = p[2].x;
    float e = p[0].y;
    float f = p[1].y;
    float g = p[2].y;

    //extract common factors
    float slope = (g - e) / (c - a);

    //x^2 coeff
    quad.x2 = (slope - (f - e) / (b - a)) / (c - b);

    // back substitute P1 and P3 to get x coeff
    quad.x = slope - quad.x2 * (a + c);

    // back substitute P1 to get constant
    quad.k = e - (quad.x2 * a + quad.x) * a;


}

