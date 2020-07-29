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

