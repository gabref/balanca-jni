/******************************************************************************
 *  File: com_ccibm_ect_perifericos_BalancaPadraoSara.c
 *
 *  Description: This C source file contains the implementation of the BalancaPadraoSara
 *               class, which serves as a Java Native Interface (JNI) bridge to interact
 *               with the E1_Balanca01.dll library provided by ELGIN. The E1_Balanca01.dll
 *               library offers functionalities to communicate with ELGIN weighing scales
 *               and retrieve their model information and weight measurements.
 * 
 *  Created on: July 27, 2023
 *
 *  Last Modified: July 29, 2023
 *
 *  Version: 1.0
 *
 *  Revision History:
 *    - 1.0: Initial version with basic functionalities.
 *
 ******************************************************************************/

// include header files for JNI interface , custom error codes, and scale configs
#include "com_ccibm_ect_perifericos_BalancaPadraoSara.h"
#include "errors.h"
#include "balanca_padrao_sara.h"

// function pointers to the dll functions
int (*ConfigurarModeloBalanca)(int);
int (*ConfigurarProtocoloComunicacao)(int);
int (*ObterModeloBalanca)();
int (*AbrirSerial)(const char *, int, int, char, int);
int (*Fechar)();
const char *(*LerPeso)(int);
int (*DirectIO)(const char *, unsigned int, char *, unsigned int);

// handle to the loaded dll
HINSTANCE hDll;

// structure to store scale configurations
struct ScaleConfig scaleConfig;

// path to external dll
char *DLL_PATH = "E1_Balanca01.dll";

// flag to cehck if the communication protocol is configured
int protocol_configured = 0;

int loadDll() {
    // check if dll is loaded
    if (hDll != NULL) 
        return SUCCESS;

    hDll = LoadLibrary(DLL_PATH);
    if (hDll == NULL) {
        // printf("Failed to load the DLL.\n");
        return ERRO_CARGA_DLL;
    }
    // printf("Library loaded\n");

    // get the function pointers for the dll functions
    GET_FUNCTION_PTR(hDll, ConfigurarModeloBalanca);
    GET_FUNCTION_PTR(hDll, ConfigurarProtocoloComunicacao);
    GET_FUNCTION_PTR(hDll, ObterModeloBalanca);
    GET_FUNCTION_PTR(hDll, AbrirSerial);
    GET_FUNCTION_PTR(hDll, Fechar);
    GET_FUNCTION_PTR(hDll, LerPeso);
    GET_FUNCTION_PTR(hDll, DirectIO);

    // printf("Functions loaded\n");

    return SUCCESS;
}

void unloadDll() {
    FreeLibrary(hDll);
    // printf("Library unloaded\n");
}

void resolvePort(int option, char *device) {
    switch (option) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        sprintf(device, "COM%d", option);
        break;
    case 10:
    case 11:
    case 12:
    case 13:
        sprintf(device, "USB%d", option - 9);
        break;
    default:
        sprintf(device, "Unknown");
        break;
    }
}

void setDefaultScaleConfig(char *serialPort) {
    // check if scale configuration is not set yet
    if (!scaleConfig.model) {
        scaleConfig.model = MODELO_BALANCA;
        scaleConfig.protocol = PROTOCOLO_COMUNICACAO;
        scaleConfig.length = LENGTH;
        scaleConfig.parity = PARITY;
        scaleConfig.stopbits = STOPBITS;

        // determine the baudrate based on the serial port type
        scaleConfig.baudrate = strncmp(serialPort, "COM", 3) == 0 ? BAUDRATE_COM : BAUDRATE_USB;

        strcpy(scaleConfig.serialPort, serialPort);
    }
    else if (strcmp(serialPort, scaleConfig.serialPort) != 0) {
        // if the serial port changed, update the configs accordingly
        scaleConfig.baudrate = strncmp(serialPort, "COM", 3) == 0 ? BAUDRATE_COM : BAUDRATE_USB;
        strcpy(scaleConfig.serialPort, serialPort);
    }
}

// function to configure the scale communication settings
int configureScale() {
    int ret;

    int modelo = ObterModeloBalanca();
    if (modelo == -1) {
        ret = ConfigurarModeloBalanca(scaleConfig.model);
        if (ret != SUCCESS)
            return ret;
    }

    if (protocol_configured == 0) {
        ret = ConfigurarProtocoloComunicacao(scaleConfig.protocol);
        if (ret != SUCCESS)
            return ret;
        protocol_configured = 1;
    }

    // for x86 compability, why? i have no idea
    int length = scaleConfig.length;

    ret = AbrirSerial(scaleConfig.serialPort, scaleConfig.baudrate, length, scaleConfig.parity, scaleConfig.stopbits);
    if (ret != SUCCESS)
        return ret;

    return ret;
}

// =============== SERIAL NUMBER STUFF ===============

// int randomInRange(int min, int max) {
//     return rand() % (max - min + 1) + min;
// }

// int randomInRandom(int min, int max, int n) {
//     int randoms[n];
//     for (int i = 0; i < n; i++)
//         randoms[i] = randomInRange(min, max);
    
//     int randomIndex = randomInRange(0, n - 1);
//     return randoms[randomIndex];
// }

// void generateSerialNumber(char *serialNumber) {
//     // seed the random number generaton only once
//     srand(time(NULL));

//     // get current year
//     time_t t = time(NULL);
//     struct tm *currentTime = localtime(&t);
//     int currentYear = currentTime->tm_year + 1900; // adjust to get current year

//     int randomNumber = randomInRandom(1000, 9999, 10);

//     sprintf(serialNumber, "ELGECT%04d%04d", currentYear, randomNumber);
// }

int sizeOfSerialNumber(char *source) {
    char stxChar = '\x02'; // STX character in hexadecimal
    char etxChar = '\x03'; // ETX character in hexadecimal

    // Search for the first occurrence of STX in the inputString
    char *stxPtr = strchr(source, stxChar);

    if (stxPtr == NULL) // If STX is not found, return -1
        return -1;

    // Search for the first occurrence of ETX after STX in the inputString
    char *etxPtr = strchr(stxPtr, etxChar);

    if (etxPtr == NULL) // If ETX is not found after STX, return -1
        return -1;

    // Calculate and return the size of the serial number between STX and ETX
    return etxPtr - stxPtr - 1;
}

void extractHexString(char *source, char *destination, int length) {
    strncpy(destination, source + 1, length);
    destination[length] = '\0'; // Null-terminate the destination string
}

int getSerialNumber(char *serialNumber) {
    strcpy(serialNumber, "23002916");
    return 0;
}

// =============== JNI =====================

// function to handle errors and throw Java exceptions
jstring handleError(JNIEnv *env, int errorCode) {
    jstring ret;
    jclass newExceptionClass;

    // clear any pending java exceptions
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);

    // convert Elgin error code to SARA error code
    char saraError[10];
    mapE1ErrorCodesToSara(errorCode, saraError);
    // fprintf(stderr, "Error: DLL - %d, SARA - %s\n", errorCode, saraError);

    // find the java exception class and throw the corresponding exception
    newExceptionClass = (*env)->FindClass(env, "com/ccibm/ect/perifericos/SaraPerifericosException");
    if (newExceptionClass == NULL) {
        // if class not found, return a default error message
        ret = (*env)->NewStringUTF(env, "-1");
        return ret;
    }
    (*env)->ThrowNew(env, newExceptionClass, saraError);

    ret = (*env)->NewStringUTF(env, saraError);
    return ret;
}

// jni function to obtain the serial number
JNIEXPORT jstring JNICALL Java_com_ccibm_ect_perifericos_BalancaPadraoSara_obterNumeroSerie(JNIEnv *env, jobject obj)
{
    // printf("\n\nEntrando na funcao ObterNumeroSerie\n");

    int retLoad = loadDll();
    if (retLoad != SUCCESS) {
        return handleError(env, retLoad);
    }

    // check if scaleConfig has been initialized
    // the lerPeso function should be called first, therefore the scaleConfig will be initialized
    if (!scaleConfig.model) {
        return handleError(env, ERRO_NUMERO_SERIE);
    }
    setDefaultScaleConfig(scaleConfig.serialPort);
    int retConfigure = configureScale();
    if (retConfigure != 0) {
        return handleError(env, retConfigure);
    }


    // check and register a serial number in the scale
    // to be implemented

    char serialNumber[9];
    int retSN = getSerialNumber(serialNumber);

    if (retSN == -1) {
        return handleError(env, ERRO_NUMERO_SERIE);
    }

    jstring ret;
    ret = (*env)->NewStringUTF(env, serialNumber);

    int retFecha = Fechar();
    if (retFecha != 0) {
        return handleError(env, retFecha);
    }

    // printf("Saindo da funcao ObterNumeroSerie, ret: %s\n", serialNumber);
    // free(serialNumber);
    return ret;
}

// jni function to read the weight from the scale
JNIEXPORT jstring JNICALL Java_com_ccibm_ect_perifericos_BalancaPadraoSara_lerPeso(JNIEnv *env, jobject obj, jint portaSerial)
{
    // printf("\n\nEntrando na funcao lerPeso\n");

    jstring ret;

    int retLoad = loadDll();
    if (retLoad != SUCCESS) {
        return handleError(env, retLoad);
    }

    char serialPort[5];
    resolvePort((int)portaSerial, serialPort);
    setDefaultScaleConfig(serialPort);

    int retConfigure = configureScale();
    if (retConfigure != 0) {
        return handleError(env, retConfigure);
    }

    char *peso = (char *)LerPeso(1);
    if (strcmp(peso, "IIIII") == 0) {
        strcpy(peso, "-1");
    }
    else if (strcmp(peso, "") == 0) {
        // scale is off
        return handleError(env, DISPOSITIVO_NAO_EXISTE);
    }

    ret = (*env)->NewStringUTF(env, peso);

    int retFecha = Fechar();
    if (retFecha != 0) {
        return handleError(env, retFecha);
    }

    // printf("Saindo da funcao LerPeso, ret: %s\n", peso);
    return ret;
}
