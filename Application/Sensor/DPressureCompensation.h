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

#ifndef NEWPCOMP_H
#define NEWPCOMP_H

typedef struct  { float x, kx3, kx2, kx, k; }sSpamCubic_t ;
typedef struct  {float x2,x,k;}sQuadratic_t ;
typedef struct  {float x,y;}point_t ;

void spamfit (const point_t P[], const short n, sSpamCubic_t PolyArray[]);
float spam( sSpamCubic_t *coefs, const short n, const float xa);
void quadsolve (point_t p[3], sQuadratic_t &quad);
float lagrange( const float xp,const float x[3],const float y[3]);

#endif

