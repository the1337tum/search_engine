#include <stddef.h>                                                                                                                                          
#include "rjhash.hpp"

// rjhash() mixing macro
#define mix(a, b, c) \
    a -= b, a -= c, a ^= c>>13, \
    b -= c, b -= a, b ^= a<< 8, \
    c -= a, c -= b, c ^= b>>13, \
    a -= b, a -= c, a ^= c>>12, \
    b -= c, b -= a, b ^= a<<16, \
    c -= a, c -= b, c ^= b>> 5, \
    a -= b, a -= c, a ^= c>> 3, \
    b -= c, b -= a, b ^= a<<10, \
    c -= a, c -= b, c ^= b>>15

unsigned long rjhash(
    void const                   *p,
    size_t                        L,
    register unsigned long        c
) {
    register unsigned char const *k = (unsigned char const *)p;
    register unsigned long        a;
    register unsigned long        b;
    size_t                        n;

    a = b = (unsigned long)0x9e3779b9;
    for (n = L; n >= 12; n -= 12) {
		a += ( (unsigned long)k[ 0] +
		      ((unsigned long)k[ 1] <<  8) +
		      ((unsigned long)k[ 2] << 16) +
		      ((unsigned long)k[ 3] << 24) );
		b += ( (unsigned long)k[ 4] +
		      ((unsigned long)k[ 5] <<  8) +
		      ((unsigned long)k[ 6] << 16) +
		      ((unsigned long)k[ 7] << 24) );
		c += ( (unsigned long)k[ 8] +
		      ((unsigned long)k[ 9] <<  8) +
		      ((unsigned long)k[10] << 16) +
		      ((unsigned long)k[11] << 24) );
		mix(a, b, c);
		k += 12;
    }
    c += L;
    switch (n) {
	case 11: c += (unsigned long)k[10] << 24; /*FALLTHROUGH*/
	case 10: c += (unsigned long)k[ 9] << 16; /*FALLTHROUGH*/
	case  9: c += (unsigned long)k[ 8] <<  8; /*FALLTHROUGH*/
	case  8: b += (unsigned long)k[ 7] << 24; /*FALLTHROUGH*/
	case  7: b += (unsigned long)k[ 6] << 16; /*FALLTHROUGH*/
	case  6: b += (unsigned long)k[ 5] <<  8; /*FALLTHROUGH*/
	case  5: b += (unsigned long)k[ 4];       /*FALLTHROUGH*/
	case  4: a += (unsigned long)k[ 3] << 24; /*FALLTHROUGH*/
	case  3: a += (unsigned long)k[ 2] << 16; /*FALLTHROUGH*/
	case  2: a += (unsigned long)k[ 1] <<  8; /*FALLTHROUGH*/
	case  1: a += (unsigned long)k[ 0];       /*FALLTHROUGH*/
	default: break;
    }
    mix(a, b, c);
    return c;
}

