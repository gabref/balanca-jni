#include "com_ccibm_ect_perifericos_BalancaPadraoSara.h"
#include "errors.h"
#include "balanca_padrao_sara.h"

// get pointer to functions
int (*ConfigurarModeloBalanca)(int);
int (*ConfigurarProtocoloComunicacao)(int);
int (*ObterModeloBalanca)();
int (*AbrirSerial)(const char *, int, int, char, int);
int (*Fechar)();
const char *(*LerPeso)(int);

HINSTANCE hDll;
struct ScaleConfig scaleConfig;
char *DLL_PATH = "E1_Balanca01.dll";
int protocol_configured = 0;

int loadDll() {
    hDll = LoadLibrary(DLL_PATH);
    if (hDll == NULL) {
        printf("Failed to load the DLL.\n");
        return ERRO_CARGA_DLL;
    }
    printf("Library loaded\n");

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
    if (!scaleConfig.model) {
        scaleConfig.model = MODELO_BALANCA;
        scaleConfig.protocol = PROTOCOLO_COMUNICACAO;
        scaleConfig.length = LENGTH;
        scaleConfig.parity = PARITY;
        scaleConfig.stopbits = STOPBITS;

        scaleConfig.baudrate = strncmp(serialPort, "COM", 3) == 0 ? BAUDRATE_COM : BAUDRATE_USB;

        strcpy(scaleConfig.serialPort, serialPort);
    }
    else if (strcmp(serialPort, scaleConfig.serialPort) != 0) {
        scaleConfig.baudrate = strncmp(serialPort, "COM", 3) == 0 ? BAUDRATE_COM : BAUDRATE_USB;
        strcpy(scaleConfig.serialPort, serialPort);
    }
}

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

    ret = AbrirSerial(scaleConfig.serialPort, scaleConfig.baudrate, scaleConfig.length, scaleConfig.parity, scaleConfig.stopbits);
    if (ret != SUCCESS)
        return ret;

    return ret;
}

jstring handleError(JNIEnv *env, int errorCode) {
    jstring ret;
    jclass newExceptionClass;
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);

    char saraError[10];
    mapE1ErrorCodesToSara(errorCode, saraError);
    fprintf(stderr, "Error Loading DLL: DLL - %d, SARA - %s\n", errorCode, saraError);

    newExceptionClass = (*env)->FindClass(env, "com/ccibm/ect/perifericos/SaraPerifericosException");
    if (newExceptionClass == NULL) {
        ret = (*env)->NewStringUTF(env, "-1");
        return ret;
    }
    (*env)->ThrowNew(env, newExceptionClass, saraError);

    ret = (*env)->NewStringUTF(env, saraError);
    return ret;
}

JNIEXPORT jstring JNICALL Java_com_ccibm_ect_perifericos_BalancaPadraoSara_obterNumeroSerie(JNIEnv *env, jobject obj)
{

    printf("\n\nEntrando na funcao ObterNumeroSerie\n");

    jstring ret;

    // check if dll is loaded
    if (hDll == NULL) {
        int retLoad = loadDll();
        if (retLoad != SUCCESS) {
            return handleError(env, retLoad);
        }
    }

    char serialPort[5];
    resolvePort(4, serialPort);
    setDefaultScaleConfig(serialPort);
    int retConfigure = configureScale();

    int modelo = ObterModeloBalanca();
    if (modelo == -1) {
        return handleError(env, modelo);
    }

    char buff[10];
    snprintf(buff, sizeof(buff), "%d", modelo);
    ret = (*env)->NewStringUTF(env, buff);

    int retFecha = Fechar();
    if (retFecha != 0) {
        return handleError(env, retFecha);
    }

    printf("Saindo da funcao ObterNumeroSerie, ret: %s\n", buff);
    return ret;
}

JNIEXPORT jstring JNICALL Java_com_ccibm_ect_perifericos_BalancaPadraoSara_lerPeso(JNIEnv *env, jobject obj, jint portaSerial)
{

    printf("\n\nEntrando na funcao lerPeso\n");

    jstring ret;

    // check if dll is loaded
    if (hDll == NULL) {
        int retLoad = loadDll();
        if (retLoad != SUCCESS) {
            return handleError(env, retLoad);
        }
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

    printf("Saindo da funcao LerPeso, ret: %s\n", peso);
    return ret;
}
