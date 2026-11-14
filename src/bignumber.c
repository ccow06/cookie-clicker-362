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

// returns - if a < b, 0 if a = b, + if a > b
int compBN(BigNumber a, BigNumber b) {

    normalizeBN(&a);
    normalizeBN(&b);

    if (a.exponent > b.exponent)
        return 1;
    else if (b.exponent > a.exponent)
        return -1;
    else
        return a.mantissa - b.mantissa;
}

// a ^ b, where a is a double and b is an int
// generally, just going to be used for cost scaling, so nothing fancy
void powToBN(double a, int b, BigNumber* c) {

    *c = (BigNumber){1,0};
    // blocks of 100 will be sufficient for this
    do
    {
        // if b is greater than 100, use 100 (first few loops, only when you've purchased > 100 buildings)
        // if not, use b (last loop)
        int b2 = (b > 100) ? 100 : b;

        BigNumber d = {pow(a, b2), 0};

        b -= 100;
        multBN(d, *c, c);
    } while(b > 0);
}