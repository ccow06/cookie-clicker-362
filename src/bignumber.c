#include <stdio.h>
#include <math.h>
#include "bignumber.h"

// Set properties of BN to a "structure" we want
// i.e., keep the mantissa to 1-3 digits before the decimal,
//       keep the exponent to a factor of 3 (but 6 at lowest)
void normalizeBN(BigNumber* a) {

    int man = (int)(a->mantissa) * pow(10, a->exponent);
    int digs = 0;

    while (man != 0)
    {
        man /= 10;
        digs++;
    }

    if (digs < 7)
    {
        digs = 1; // ensures a->exponent becomes 0, and a->mantissa is "full"
    }
    digs = (digs - 1) / 3 * 3; // if 7 digits, goes to 6. if 9 digits, goes to 6. if 10 digits, goes to 9.
    // basically, keeps exponent value to multiple of 3, next lowest if alr a multiple of 3

    a->mantissa = a->mantissa / (pow(10, digs));
    a->exponent = digs;
}

void addBN(BigNumber a, BigNumber b, BigNumber* c) {

    while (b.exponent < a.exponent)
    {
        b.mantissa /= 10;
        b.exponent++;
    }
    while (a.exponent < b.exponent)
    {
        a.mantissa /= 10;
        a.exponent++;
    }

    c->mantissa = a.mantissa + b.mantissa;
    c->exponent = a.exponent;

    normalizeBN(c);
}

void subBN(BigNumber a, BigNumber b, BigNumber* c) {

    b.mantissa = -b.mantissa;
    addBN(a,b,c);
}

void multBN(BigNumber a, BigNumber b, BigNumber* c) {

    c->mantissa = a.mantissa * b.mantissa;
    c->exponent = a.exponent + b.exponent;

    normalizeBN(c);
}

void divBN(BigNumber a, BigNumber b, BigNumber* c) {

    c->mantissa = a.mantissa / b.mantissa;
    c->exponent = a.exponent - b.exponent;

    normalizeBN(c);
}