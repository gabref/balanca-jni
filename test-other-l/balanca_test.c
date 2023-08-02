#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// path to external dll
char *DLL_PATH = "E1_Balanca32.dll";

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

#define MODELO_BALANCA 3

#define PROTOCOLO_COMUNICACAO 0
#define BAUDRATE_COM 2400
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


    char porta[5];
    strcpy(porta, scaleConfig.serialPort);
    int baudrate = scaleConfig.baudrate;
    int length = scaleConfig.length;
    char par = scaleConfig.parity;
    int stopbits = scaleConfig.stopbits;

    printf("%s - %s\n", scaleConfig.serialPort, porta);
    printf("%d - %d\n", scaleConfig.baudrate, baudrate);
    printf("%d - %d\n", scaleConfig.length, length);
    printf("%c - %c\n", scaleConfig.parity, par);
    printf("%d - %d\n", scaleConfig.stopbits, stopbits);

    ret = AbrirSerial(scaleConfig.serialPort, scaleConfig.baudrate, length, scaleConfig.parity, scaleConfig.stopbits);
    printf("made it here: %d\n", ret);
    if (ret != SUCCESS)
        return ret;
    printf("different than 0\n");

    return ret;
}

int main() {

    printf("\n\nEntrando na funcao lerPeso\n");

    // check if dll is loaded
    int retLoad = loadDll();
    if (retLoad != SUCCESS) {
        fprintf(stderr, "Deu erro no load: %d\n", retLoad);
        exit(1);
    }

    setDefaultScaleConfig("COM4");
    int retConfigure = configureScale();
    printf("ret configure %d\n", retConfigure);
    if (retConfigure != 0) {
        fprintf(stderr, "Deu erro no configureScale: %d\n", retConfigure);
        exit(1);
    }

    printf("About to read\n");
    char *peso = (char *)LerPeso(1);
    if (strcmp(peso, "IIIII") == 0) {
        strcpy(peso, "-1");
    }
    else if (strcmp(peso, "") == 0) {
        // scale is off
        fprintf(stderr, "Deu erro na leitura: %s\n", peso);
        exit(1);
    }

    int retFecha = Fechar();
    if (retFecha != 0) {
        fprintf(stderr, "Deu erro no fecha: %s\n", retFecha);
        exit(1);
    }

    printf("Saindo da funcao LerPeso, ret: %s\n", peso);
    printf("peso lido: %s", peso);
    return 0;
}