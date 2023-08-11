#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// path to external dll
char *DLL_PATH = "E1_Balanca01.dll";

#define GET_FUNCTION_PTR(hDLL, funcName)                                 \
    funcName = (typeof(funcName))GetProcAddress(hDLL, #funcName);        \
    if (funcName == NULL)                                                \
    {                                                                    \
        printf("Failed to get function pointer for '%s'.\n", #funcName); \
        printf("Probably your dll version is very wrong\n");             \
        FreeLibrary(hDLL);                                               \
        exit(1);                                                         \
    }

#define SUCCESS 0
#define ERRO_CARGA -1234
#define ERRO_CARGA_DLL 69

#define MODELO_BALANCA 0

#define PROTOCOLO_COMUNICACAO 0
#define BAUDRATE_COM 19200
#define BAUDRATE_USB 9600
#define LENGTH 8
#define PARITY 'N'
#define STOPBITS 1

// function pointers to the dll functions
int (*ConfigurarModeloBalanca)(int);
int (*ConfigurarProtocoloComunicacao)(int);
int (*ObterModeloBalanca)();
int (*AbrirSerial)(const char *, int, int, char, int);
int (*Fechar)();
const char *(*LerPeso)(int);
int (*DirectIO)(const char *, unsigned int, char *, unsigned int);
void (*SetContinuousReadTime)(int);

struct ScaleConfig
{
    int model;
    int protocol;
    int baudrate;
    int length;
    char parity;
    int stopbits;
    char serialPort[5];
};

// structure to store scale configurations
struct ScaleConfig scaleConfig;

// handle to the loaded dll
HINSTANCE hDll;

// flag to cehck if the communication protocol is configured
int protocol_configured = 0;
int bal_baudrate;
int bal_model;
char bal_serialPort[4];

int loadDll() {
    if (hDll != NULL) 
        return SUCCESS;

    hDll = LoadLibrary(DLL_PATH);
    if (hDll == NULL) {
        printf("Failed to load the DLL.\n");
        return ERRO_CARGA_DLL;
    }
    printf("Library loaded\n");

    // get the function pointers for the dll functions
    GET_FUNCTION_PTR(hDll, ConfigurarModeloBalanca);
    GET_FUNCTION_PTR(hDll, ConfigurarProtocoloComunicacao);
    GET_FUNCTION_PTR(hDll, ObterModeloBalanca);
    GET_FUNCTION_PTR(hDll, AbrirSerial);
    GET_FUNCTION_PTR(hDll, Fechar);
    GET_FUNCTION_PTR(hDll, LerPeso);
    GET_FUNCTION_PTR(hDll, DirectIO);
    GET_FUNCTION_PTR(hDll, SetContinuousReadTime);


    printf("Functions loaded\n");

    return SUCCESS;
}

void unloadDll() {
    FreeLibrary(hDll);
    printf("Library unloaded\n");
}

void setDefaultScaleConfig(char *serialPort) {
    // check if scale configuration is not set yet
    if (!scaleConfig.model) {
        scaleConfig.model = MODELO_BALANCA;
        scaleConfig.protocol = PROTOCOLO_COMUNICACAO;
        scaleConfig.length = LENGTH;
        scaleConfig.parity = PARITY;
        scaleConfig.stopbits = STOPBITS;
        bal_model = MODELO_BALANCA;

        // determine the baudrate based on the serial port type
        scaleConfig.baudrate = strncmp(serialPort, "COM", 3) == 0 ? BAUDRATE_COM : BAUDRATE_USB;

        strcpy(scaleConfig.serialPort, serialPort);

        bal_baudrate = strncmp(serialPort, "COM", 3) == 0 ? BAUDRATE_COM : BAUDRATE_USB;
        strcpy(bal_serialPort, serialPort);
    }
    else if (strcmp(serialPort, scaleConfig.serialPort) != 0) {
        // if the serial port changed, update the configs accordingly
        scaleConfig.baudrate = strncmp(serialPort, "COM", 3) == 0 ? BAUDRATE_COM : BAUDRATE_USB;
        strcpy(scaleConfig.serialPort, serialPort);

        bal_baudrate = strncmp(serialPort, "COM", 3) == 0 ? BAUDRATE_COM : BAUDRATE_USB;
        strcpy(bal_serialPort, serialPort);
    }
}

// function to configure the scale communication settings
int configureScale() {
    int ret;

    int modelo = ObterModeloBalanca();
    if (modelo == -1) {
        ret = ConfigurarModeloBalanca(MODELO_BALANCA);
        if (ret != SUCCESS)
            return ret;
    }

    if (protocol_configured == 0) {
        ret = ConfigurarProtocoloComunicacao(PROTOCOLO_COMUNICACAO);
        if (ret != SUCCESS)
            return ret;
        protocol_configured = 1;
    }


    // int ret;
    int length = scaleConfig.length;
    ret = AbrirSerial(scaleConfig.serialPort, scaleConfig.baudrate, length, scaleConfig.parity, scaleConfig.stopbits);
    if (ret != SUCCESS)
        return ret;

    // SetContinuousReadTime(2500);

    return ret;
}

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
char * getSerialNumber() {
    printf("About to read serial number\n");

    const char commandReadSerialNumber[3] = {'\x1b', '\x48', '\x32'};
    char serialNumberGarbage[20];
    int retDirect = DirectIO(commandReadSerialNumber, 3, serialNumberGarbage, 20);

    printf("retDirect: %d, retorno: %s\n", retDirect, serialNumberGarbage);

    int size = sizeOfSerialNumber(serialNumberGarbage);
    if (size > 0) {
        char *serialNumber = (char *) malloc(sizeof(char) * (size + 1));

        extractHexString(serialNumberGarbage, serialNumber, size);
        // strcpy(scaleConfig.serialNumber, serialNumber);

        printf("Extracted Serial Number: %s\n", serialNumber);
        return serialNumber;
    } else {
        printf("Invalid serial number.\n");
        return NULL;
    }
}

int main(int argc, char *argv[]) {
    // check if dll is loaded
    int retLoad = loadDll();
    if (retLoad != SUCCESS) {
        fprintf(stderr, "Deu erro no load: %d\n", retLoad);
        exit(1);
    }

    setDefaultScaleConfig("COM4");

    if (argc >= 2) {
        setDefaultScaleConfig(argv[1]);
    }
    int customBaudrate = BAUDRATE_COM;
    if (argc >= 3) {
        customBaudrate = atoi(argv[2]);
    }
    scaleConfig.baudrate = customBaudrate;

    int retConfigure = configureScale();
    if (retConfigure != 0) {
        fprintf(stderr, "Deu erro no configureScale: %d\n", retConfigure);
        exit(1);
    }

    // peso

    printf("\n");
    char *peso = (char *)LerPeso(1);
    printf("peso: len: %d - %s\n", strlen(peso), peso);
    if (strcmp(peso, "IIIII") == 0) {
        strcpy(peso, "-1");
    }
    else if (strcmp(peso, "") == 0) {
        // scale is off
        fprintf(stderr, "Deu erro na leitura: %s\n", peso);
        // exit(1);
    }
    printf("peso lido: %s\n", peso);

    // TESTES SERIAL NUMBER

    printf("\nAbout to read serial number\n");

    char *serialNumber = getSerialNumber();
    if (serialNumber == NULL) {
        printf("could not get serial number\n");
    } else {
        printf("serial number is: %s\n", serialNumber);
    }
    free(serialNumber);
    // const char comandoDeEnvio[3] = { '\x1b', '\x48', '\x32' };

    // char retorno[12];
    // memset(retorno, 0, 12);
    // int retDirect = DirectIO(comandoDeEnvio, 3, retorno, 11);

    // printf("retDirect: %d, retorno: %s\n", retDirect, retorno);

    // int size = sizeOfSerialNumber(retorno);
    // if (size > 0) {
    //     char serialNumbertest[size + 1];
    //     extractHexString(retorno, serialNumbertest, size);
    //     printf("Extracted Serial Number: %s\n", serialNumbertest);
    // }
    // else {
    //     printf("Invalid serial number.\n");
    // }

    // FECHAR

    int retFecha = Fechar();
    if (retFecha != 0) {
        fprintf(stderr, "Deu erro no fecha: %s\n", retFecha);
        exit(1);
    }

    return 0;
}