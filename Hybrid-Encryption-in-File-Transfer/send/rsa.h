#ifndef RSA_H
#define RSA_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <algorithm>
using namespace std;

typedef long long ll;

int gcd(int a, int b);
int PrimarityTest(int a, int i);
int ModularExponention(int a, int b, int n);
void extgcd(ll a,ll b,ll& d,ll& x,ll& y);
int ModularInverse(int a,int b);
int Decryption(int value,int d, int n);
int Encryption(int value,int e, int n);
int *KeyGeneration(int key[]);

#endif // RSA_H
