#ifndef RJHASH_H_
#define RJHASH_H_

/*  rjhash() function devised by Robert J. Jenkins Jr.
    This version was written by Richard O'Keefe.

    Speed note:  if you don't know the length of a string before you start,
    rjhash(str, strlen(str), 0) is somewhat slower than strhash(str);
    if you do know the length of a block, rjhash(ptr, len, 0) is
    somewhat faster than memhash(ptr, len).

    Speed isn't everything, though.  Some distribution figures may be
    of more interest.  They were obtained by hashing all the words in
    (an extension of) a Scrabble dictionary (just under 150 thousand).
    The hash values were reduced modulo a modulus (256, 1024, 65536),
    that is, the bottom bits were taken.  The minimum and maximum
    bucket sizes for a hash table using these hash values were determined,
    as was a chi-squared value where smaller means "less variation".

    strhash(): min=225 max=2260 chisq=37586.4 mod=256
    rjhash() : min=502 max= 652 chisq=  254.2 mod=256

    strhash(): min= 35 max=1119 chisq=71331.1 mod=1024
    rjhash() : min=109 max= 189 chisq= 1028.2 mod=1024

    strhash(): min=0 max=128 chisq=444079.0 mod=65536
    rjhash() : min=0 max= 11 chisq= 65668.9 mod=65536

    From this we see that rjhash() does a better job of spreading these
    values out.  It is therefore likely to be better in real use.
*/

extern unsigned long rjhash(void const *p, size_t L, register unsigned long c);

#endif
