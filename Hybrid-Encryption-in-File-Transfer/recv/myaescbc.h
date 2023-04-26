#ifndef MYAESCBC_H
#define MYAESCBC_H

#include <QObject>
#include <QByteArray>

typedef unsigned long DWORD;
typedef unsigned char byte;

//enum KeySize { Bits128, Bits192, Bits256 }; // key size, in bits, for construtor
#define Bits128    16
#define Bits192    24
#define Bits256    32

class MyAesCBC: public QObject
{
    Q_OBJECT

public:
    MyAesCBC();
    MyAesCBC(int keySize, unsigned char* keyBytes);
    ~MyAesCBC();

    unsigned char State[4][4];

    DWORD OnAesEncrypt(QByteArray InBuffer, DWORD InLength, QByteArray &OutBuffer);
    DWORD OnAesUncrypt(QByteArray InBuffer, DWORD InLength, QByteArray &OutBuffer);

private:
    int Nb; // block size in 32-bit words. Always 4 for AES. (128 bits).
    int Nk; // key size in 32-bit words. 4, 6, 8. (128, 192, 256 bits).
    int Nr; // number of rounds. 10, 12, 14.

    unsigned char key[32];
    unsigned char w[16 * 15];

    void Cipher(QByteArray input, QByteArray &output); // encipher 16-bit input
    void InvCipher(QByteArray input, QByteArray &output); // decipher 16-bit input
    void SetNbNkNr(int keySize);
    void AddRoundKey(int round);
    void SubBytes();
    void InvSubBytes();
    void ShiftRows();
    void InvShiftRows();
    void MixColumns();
    void InvMixColumns();
    unsigned char gfmultby01(unsigned char b);
    unsigned char gfmultby02(unsigned char b);
    unsigned char gfmultby03(unsigned char b);
    unsigned char gfmultby09(unsigned char b);
    unsigned char gfmultby0b(unsigned char b);
    unsigned char gfmultby0d(unsigned char b);
    unsigned char gfmultby0e(unsigned char b);
    void KeyExpansion();
    unsigned char* SubWord(unsigned char* word);
    unsigned char* RotWord(unsigned char* word);
};

#endif // MYAESCBC_H
