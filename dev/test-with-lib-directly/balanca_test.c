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
    printf("made it here: %d\n", ret);
    if (ret != SUCCESS)
        return ret;
    printf("different than 0\n");

    // SetContinuousReadTime(2500);

    return ret;
}

int sendHexCommand(const char *portName, const char *hexCommand, unsigned char *response, int responseSize) {
    HANDLE hSerial = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error opening the serial port.\n");
        return -1;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        printf("Error getting serial port attributes.\n");
        CloseHandle(hSerial);
        return -1;
    }

    dcbSerialParams.BaudRate = CBR_2400; // Set the baud rate (replace with the desired rate)
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        printf("Error setting serial port attributes.\n");
        CloseHandle(hSerial);
        return -1;
    }

    // Set timeouts for read and write operations
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 2000; // 100 milliseconds read timeout
    timeouts.WriteTotalTimeoutMultiplier = 10; // 10 milliseconds write timeout
    timeouts.WriteTotalTimeoutConstant = 100; // 100 milliseconds write timeout

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        printf("Error setting serial port timeouts.\n");
        CloseHandle(hSerial);
        return -1;
    }

    // Convert the hexadecimal command to binary data
    size_t len = strlen(hexCommand);
    unsigned char binaryData[len / 2];
    for (size_t i = 0; i < len; i += 2) {
        sscanf(&hexCommand[i], "%2hhx", &binaryData[i / 2]);
    }

    DWORD bytesWritten;
    if (!WriteFile(hSerial, binaryData, len / 2, &bytesWritten, NULL)) {
        printf("Error writing to the serial port.\n");
        CloseHandle(hSerial);
        return -1;
    }

    // Wait for a response
    DWORD bytesRead;
    if (!ReadFile(hSerial, response, responseSize, &bytesRead, NULL)) {
        printf("Error reading from the serial port.\n");
        CloseHandle(hSerial);
        return -1;
    }

    CloseHandle(hSerial);
    return bytesRead; // Return the number of bytes read from the serial port (response size)
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
int getSerialNumber(char *serialNumber) {
    printf("About to read serial number\n");

    const char comandoDeEnvio[3] = { '\x1b', '\x48', '\x32' };
    // const char comandoDeEnvio[1] = { '\x05' };
    char retorno[13];
    int retDirect = DirectIO(comandoDeEnvio, 3, retorno, 12);
    printf("retDirect: %d, retorno: %s\n", retDirect, retorno);

    // const char commandReadSerialNumber[3] = {'\x1b', '\x48', '\x32'};
    // char serialNumberGarbage[20];
    // int retDirect = DirectIO(commandReadSerialNumber, 3, serialNumberGarbage, 20);

    // printf("retDirect: %d, retorno: %s\n", retDirect, serialNumberGarbage);

    return -1;
    // int size = sizeOfSerialNumber(serialNumberGarbage);
    // if (size > 0) {
    //     serialNumber = (char *) malloc(sizeof(char) * (size + 1));

    //     extractHexString(serialNumberGarbage, serialNumber, size);
    //     // strcpy(scaleConfig.serialNumber, serialNumber);

    //     printf("Extracted Serial Number: %s\n", serialNumber);
    //     return 0;
    // } else {
    //     printf("Invalid serial number.\n");
    //     return -1;
    // }
}
int main() {


    // check if dll is loaded
    // int retLoad = loadDll();
    // if (retLoad != SUCCESS) {
    //     fprintf(stderr, "Deu erro no load: %d\n", retLoad);
    //     exit(1);
    // }

    // setDefaultScaleConfig("COM4");
    // int retConfigure = configureScale();
    // printf("ret configure %d\n", retConfigure);
    // if (retConfigure != 0) {
    //     fprintf(stderr, "Deu erro no configureScale: %d\n", retConfigure);
    //     exit(1);
    // }

    // printf("About to read\n");
    // char *peso = (char *)LerPeso(1);
    // printf("peso: len: %d - %s\n", strlen(peso), peso);
    // if (strcmp(peso, "IIIII") == 0) {
    //     strcpy(peso, "-1");
    // }
    // else if (strcmp(peso, "") == 0) {
    //     // scale is off
    //     fprintf(stderr, "Deu erro na leitura: %s\n", peso);
    //     exit(1);
    // }
    // printf("peso lido: %s\n", peso);

    // TESTES SERIAL NUMBER

    // char *serialNumber;
    // int retSN;
    // // for (int i = 0; i < 3; i++) {
    //     retSN = getSerialNumber(serialNumber);
    // }
    // if (retSN == -1) exit(1);
    // printf("\n%d - Definitive SerialNumber: %s\n\n", serialNumber);
    // for (int i = 0; i < 3; i++) {



        // printf("\n\nAbout to read serial number\n");
        // // const char comandoDeEnvio[3] = { '\x1b', '\x48', '\x32' };
        // const char comandoDeEnvio[3] = { (char) 0x1b, (char) 0x48, (char) 0x32 };

        // int retDirect = DirectIO(comandoDeEnvio, 3, NULL, 0);
        // printf("retDirect 1 : %d\n", retDirect);
        // Sleep(2000);

        // char retorno[13];
        // memset(retorno, 0, 13);
        // retDirect = DirectIO(comandoDeEnvio, 0, retorno, 13);


        // printf("retDirect 2 : %d\n", retDirect);
        // printf("retDirect: %d, retorno: %s\n", retDirect, retorno);

    //     int size = sizeOfSerialNumber(retorno);
    //     if (size > 0)
    //     {
    //         char serialNumbertest[size + 1];
    //         extractHexString(retorno, serialNumbertest, size);
    //         printf("Extracted Serial Number: %s\n", serialNumbertest);
    //     }
    //     else
    //     {
    //         printf("Invalid serial number.\n");
    //         exit(1);
    //     }
    // // }


    // FECHAR

    // int retFecha = Fechar();
    // if (retFecha != 0) {
    //     fprintf(stderr, "Deu erro no fecha: %s\n", retFecha);
    //     exit(1);
    // }

    // printf("Saindo da funcao LerPeso, ret: %s\n", peso);
    // printf("peso lido: %s\n\n", peso);


    // TEST READING DIRECTLY
    const char *portName = "COM4"; // Replace with the correct serial port name
    const char *pesoCommand = "05";

    unsigned char responsePeso[256]; // Buffer to store the response data
    int responseSizePeso = sizeof(responsePeso);

    char *peso;
    int bytesReadPeso = sendHexCommand(portName, pesoCommand, responsePeso, responseSizePeso);
    if (bytesReadPeso > 0) {
        printf("Response received (%d bytes): ", bytesReadPeso);
        // Convert the binary response to ASCII and store it in a string
        char asciiResponse[256];
        for (int i = 0; i < bytesReadPeso; i++) {
            printf("%02X ", responsePeso[i]);
            sprintf(&asciiResponse[i], "%c", responsePeso[i]);
            // printf("%c", asciiResponse[i]);
        }
        printf("\n");
        printf("ascii serial number: %s\n", asciiResponse);

        int size = sizeOfSerialNumber(asciiResponse);
        if (size > 0) {
            peso = (char *) malloc(sizeof(char) * (size + 1));
            if (peso == NULL) {
                printf("not able to allocate memory\n");
                exit(1);
            }

            extractHexString(asciiResponse, peso, size);
            // strcpy(scaleConfig.serialNumber, serialNumber);

            // printf("Extracted Serial Number: %s\n", serialNumber);
            // return 0;
        } else {
            printf("Invalid serial number.\n");
            return -1;
        }
        printf("serial number: %s\n", peso);
        printf("\n");
        free(peso);
    }

    // serial number
    const char *hexCommand = "1b4832"; // Replace with your hexadecimal command

    unsigned char response[256]; // Buffer to store the response data
    int responseSize= sizeof(response);

    char *serialNumber;
    int bytesRead = sendHexCommand(portName, hexCommand, response, responseSize);
    if (bytesRead > 0) {
        printf("Response received (%d bytes): ", bytesRead);
        // Convert the binary response to ASCII and store it in a string
        char asciiResponse[256];
        for (int i = 0; i < bytesRead; i++) {
            printf("%02X ", response[i]);
            sprintf(&asciiResponse[i], "%c", response[i]);
            // printf("%c", asciiResponse[i]);
        }
        printf("\n");
        printf("ascii serial number: %s\n", asciiResponse);

        int size = sizeOfSerialNumber(asciiResponse);
        if (size > 0) {
            serialNumber = (char *) malloc(sizeof(char) * (size + 1));
            if (serialNumber == NULL) {
                printf("not able to allocate memory\n");
                exit(1);
            }

            extractHexString(asciiResponse, serialNumber, size);
            // strcpy(scaleConfig.serialNumber, serialNumber);

            // printf("Extracted Serial Number: %s\n", serialNumber);
            // return 0;
        } else {
            printf("Invalid serial number.\n");
            return -1;
        }
        printf("serial number: %s\n", serialNumber);
        printf("\n");
        free(serialNumber);
    } else {
        printf("Failed to send hex command or receive response.\n");
        free(serialNumber);
    }
    return 0;
}