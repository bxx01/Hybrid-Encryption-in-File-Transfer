#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <algorithm>
#include <sstream>
#include <QFileDialog>
#include <QFile>
#include <qmetatype.h>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QVector>
#include <QNetworkInterface>
using namespace std;
typedef long long ll;
int gcd(int a, int b)
{
    int c = 0;
    if(a < b) swap(a, b);
    c = b;
    do
    {
        b = c;
        c = a % b;
        a = b;
    }
    while (c != 0);
    return b;
}

int PrimarityTest(int a, int i)
{
    int flag = 0;
    for(; a < i; a++)
    {
        if(i % a == 0)
        {
            flag = 1;
            break;
        }
    }
    if(flag) return 0;
    return 1;
}

int ModularExponention(int a, int b, int n)
{
    int y = 1;

    while(b != 0)
    {
        /*For each 1 in b, accumulate y*/

        if(b & 1)
            y = (y * a) % n;

        /*For each bit in b, calculate the square of a*/
        a = (a * a) % n;

        /*Prepare the next digit in b*/
        b = b >> 1;
    }

    return y;
}

void extgcd(ll a, ll b, ll& d, ll& x, ll& y) //Get the result of (1/a) modb
{
    if(!b)
    {
        d = a;
        x = 1;
        y = 0;
    }
    else
    {
        extgcd(b, a % b, d, y, x);
        y -= x * (a / b);
    }
}

int ModularInverse(int a,int b) //Get the result of (1/a) modb
{
    ll d, x, y;
    extgcd(a, b, d, x, y);
    return d == 1 ? (x + b) % b : -1;
}

//产生密钥函数  其中p,q都为质数  ，公钥P{e,n)，私钥S{d,n}
//在主函数调用
void KeyGeneration(unsigned int &e, unsigned int &d, unsigned int &n)
{
    int p, q;
    int phi_n;

    srand((unsigned int)time(NULL));
    do
    {
        do
            p = rand() % 100 + 23;
        while (p % 2 == 0);

    }
    while (!PrimarityTest(2, p));

    do
    {
        do
            q = rand() % 100 + 23;
        while (q % 2 == 0);
    }
    while (!PrimarityTest(2, q));

    qDebug() << "p:" << p;
    qDebug() << "q:" << q;

    n = p * q;
    phi_n = (p - 1) * (q - 1);

    do
    {
        e = rand() % (phi_n - 2) + 2; // 1 < e < phi_n
    }while (gcd(e, phi_n) != 1);

    d = ModularInverse(e, phi_n);
}

int Encryption(int value, int e, int n)
{
    int cipher;
    cipher = ModularExponention(value, e, n);
    return cipher;
}

int Decryption(int value, int d, int n)
{
    int decipher;
    decipher = ModularExponention(value, d, n);
    return decipher;
}
