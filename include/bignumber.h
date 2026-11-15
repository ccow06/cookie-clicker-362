#ifndef BIGNUMBER_H
#define BIGNUMBER_H

typedef struct {
    double mantissa;
    int exponent;
} BigNumber;

void normalizeBN(BigNumber* a);
void addBN(BigNumber a, BigNumber b, BigNumber* c);
void subBN(BigNumber a, BigNumber b, BigNumber* c);
void multBN(BigNumber a, BigNumber b, BigNumber* c);
void divBN(BigNumber a, BigNumber b, BigNumber* c);
double compBN(BigNumber a, BigNumber b);
void powToBN(double a, int b, BigNumber* c);


#endif