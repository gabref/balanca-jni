#ifndef BALANCA_JNI_H
#define BALANCA_JNI_H

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GET_FUNCTION_PTR(hDLL, funcName)                                 \
    funcName = (typeof(funcName))GetProcAddress(hDLL, #funcName);        \
    if (funcName == NULL)                                                \
    {                                                                    \
        printf("Failed to get function pointer for '%s'.\n", #funcName); \
        printf("Probably your dll version is very wrong\n");             \
        FreeLibrary(hDLL);                                               \
        exit(1);                                                         \
    }

// Modelos Balança
// 0 - DP-3005, DP-1502
// 1 - SA-110
// 2 - DPSC
// 3 - DP30CK
#ifndef MODELO_BALANCA
#define MODELO_BALANCA 3
#endif

#define PROTOCOLO_COMUNICACAO 0
#define BAUDRATE_COM 2400
#define BAUDRATE_USB 9600
#define LENGTH 8
#define PARITY 'N'
#define STOPBITS 1

struct ScaleConfig
{
    int model;
    int protocol;
    int baudrate;
    int length;
    char parity;
    int stopbits;
    char serialPort[5];
    char serialNumber[13];
};

int loadDll();
void resolvePort(int option, char *device);
void setDefaultScaleConfig(char *serialPort);
int configureScale();
jstring handleError(JNIEnv *env, int errorCode);

// int randomInRange(int min, int max);
// int randomInRandom(int min, int max, int n);
// void generateSerialNumber(char *serialNumber);
int sizeOfSerialNumber(char *source);
void extractHexString(char *source, char *destination, int length);
char *getSerialNumber();

#endif