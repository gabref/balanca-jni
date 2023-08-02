jni/balanca_padrao_sara.h

#ifndef BALANCA_JNI_H
#define BALANCA_JNI_H

#include <stdlib.h>
#include <string.h>
#include <windows.h>

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
};

int loadDll();
void resolvePort(int option, char *device);
void setDefaultScaleConfig(char *serialPort);
int configureScale();
jstring handleError(JNIEnv *env, int errorCode);

#endif

jni/com_ccibm_ect_perifericos_BalancaPadraoSara.c

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

// handle to the loaded dll
HINSTANCE hDll;

// structure to store scale configurations
struct ScaleConfig scaleConfig;

// path to external dll
char *DLL_PATH = "E1_Balanca01.dll";

// flag to cehck if the communication protocol is configured
int protocol_configured = 0;

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

    ret = AbrirSerial(scaleConfig.serialPort, scaleConfig.baudrate, scaleConfig.length, scaleConfig.parity, scaleConfig.stopbits);
    if (ret != SUCCESS)
        return ret;

    return ret;
}

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
    fprintf(stderr, "Error Loading DLL: DLL - %d, SARA - %s\n", errorCode, saraError);

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
    printf("\n\nEntrando na funcao ObterNumeroSerie\n");

    jstring ret;

    // check if dll is loaded
    int retLoad = loadDll();
    if (retLoad != SUCCESS) {
        return handleError(env, retLoad);
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

// jni function to read the weight from the scale
JNIEXPORT jstring JNICALL Java_com_ccibm_ect_perifericos_BalancaPadraoSara_lerPeso(JNIEnv *env, jobject obj, jint portaSerial)
{
    printf("\n\nEntrando na funcao lerPeso\n");

    jstring ret;

    // check if dll is loaded
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

    printf("Saindo da funcao LerPeso, ret: %s\n", peso);
    return ret;
}


jni/com_ccibm_ect_perifericos_BalancaPadraoSara.h

/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_ccibm_ect_perifericos_BalancaPadraoSara */

#ifndef _Included_com_ccibm_ect_perifericos_BalancaPadraoSara
#define _Included_com_ccibm_ect_perifericos_BalancaPadraoSara
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_ccibm_ect_perifericos_BalancaPadraoSara
 * Method:    obterNumeroSerie
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_ccibm_ect_perifericos_BalancaPadraoSara_obterNumeroSerie
  (JNIEnv *, jobject);

/*
 * Class:     com_ccibm_ect_perifericos_BalancaPadraoSara
 * Method:    lerPeso
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_ccibm_ect_perifericos_BalancaPadraoSara_lerPeso
  (JNIEnv *, jobject, jint);

#ifdef __cplusplus
}
#endif
#endif

jni/errors.h

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#include <stdio.h>
#include <string.h>

typedef enum {
    SUCCESS = 0,

    ERRO_NUMERO_SERIE = -1,
    TIPO_INVALIDO = -2,
    MODELO_INVALIDO = -3,
    PORTA_FECHADA = -4,
    CONEXAO_NEGADA = -5,
    CONEXAO_ATIVA = -6,

    BAUDRATE_INVALIDO = -11,
    DISPOSITIVO_NAO_EXISTE = -12,
    PERMISSAO_NEGADA = -13,
    ERRO_SERIAL_DESCONHECIDO = -14,
    DISPOSITIVO_JA_ESTA_ABERTO = -15,
    RECURSO_INDISPONIVEL = -16,
    OPERACAO_NAO_SUPORTADA = -17,
    SERIAL_TIMEOUT = -18,
    DISPOSITIVO_REMOVIDO_INESPERADAMENTE = -19,
    
    BAL_CONEXAO_ATIVA = -791,
    BAL_BALANCA_EM_USO = -792,
    BAL_BALANCA_INVALIDA = -793,
    BAL_NENHUMA_BALANCA_EM_USO = -794,
    
    BAL_PROTOCOLO_INVALIDO = -801,
    BAL_PROTOCOLO_EM_USO = -802,
    BAL_PROTOCOLO_NAO_SUPORTADO = -803,
    BAL_PROTOCOLO_NAO_SUPORTADO_PELAS_CONFIGS_SERIAIS = -804,
    BAL_NENHUM_PROTOCOLO_EM_USO = -805,
    
    BAL_BAUDRATE_NAO_SUPORTADO = -811,
    BAL_LENGTH_NAO_SUPORTADO = -812,
    BAL_PARITY_NAO_SUPORTADO = -813,
    BAL_STOPBITS_NAO_SUPORTADO = -814,
    BAL_BAUDRATE_INVALIDO = -815,
    BAL_LENGTH_INVALIDO = -816,
    BAL_PARITY_INVALIDO = -817,
    BAL_STOPBITS_INVALIDO = -818,
    BAL_COMBINACAO_DE_PARAMETROS_INVALIDA = -819,
    BAL_CONFIGS_SERIAIS_NAO_SUPORTADAS_PELO_PROTOCOLO = -820,
    
    BAL_QTD_LEITURAS_INVALIDA = -821,
    BAL_COMANDO_NAO_SUPORTADO_PELA_BALANCA = -822,
    BAL_COMANDO_NAO_SUPORTADO_PELO_PROTOCOLO = -823,
    BAL_PRECO_INVALIDO = -824,
    BAL_FALHA_NA_LEITURA_DO_PESO = -825,
    BAL_FALHA_AO_ENVIAR_PRECO = -826,
    CHAMADA_NAO_PERMITIDA = -827,

    ERRO_CARGA_DLL = -1234,
    ERRO_OBTENCAO_NUMERO_SERIE = -1235,
} E1ErrorCode;

typedef enum {
    SARA_EXCECAO_GENERICA = 4000200,
    SARA_ERRO_CARGA_DLL_FABRICANTE = 4010201,
    SARA_PERIFERICO_DESLIGADO = 4020200,
    SARA_ERRO_ABERTURA_OU_FECHAMENTO_PORTA = 4030200,
    SARA_TIMEOUT = 4090200,
    SARA_ERRO_OBTER_NUMERO_SERIE = 4130200,
} SaraErrorCode;

void mapE1ErrorCodesToSara(E1ErrorCode errorCode, char *saraError);

#endif

jni/errors.c

#include "errors.h"

char * intToStr(int intToConvert, char *strConverted) {
    sprintf(strConverted, "%d", intToConvert);
}

void mapE1ErrorCodesToSara(E1ErrorCode errorCode, char *saraError) {
    // initialize the saraError Buffer to empty string
    memset(saraError, 0, sizeof(saraError));

    switch(errorCode) {
        case ERRO_CARGA_DLL:
            intToStr(SARA_ERRO_CARGA_DLL_FABRICANTE, saraError);
            break;

        case TIPO_INVALIDO:
        case MODELO_INVALIDO:
        case DISPOSITIVO_NAO_EXISTE:
        case PERMISSAO_NEGADA:
        case SERIAL_TIMEOUT:
        case DISPOSITIVO_REMOVIDO_INESPERADAMENTE:
            intToStr(SARA_PERIFERICO_DESLIGADO, saraError);
            break;

        case PORTA_FECHADA:
        case CONEXAO_NEGADA:
        case CONEXAO_ATIVA:
        case BAUDRATE_INVALIDO:
        case ERRO_SERIAL_DESCONHECIDO:
        case BAL_PROTOCOLO_INVALIDO:
        case BAL_PROTOCOLO_EM_USO:
        case BAL_PROTOCOLO_NAO_SUPORTADO:
        case BAL_PROTOCOLO_NAO_SUPORTADO_PELAS_CONFIGS_SERIAIS:
        case BAL_NENHUM_PROTOCOLO_EM_USO:
        case BAL_BAUDRATE_NAO_SUPORTADO:
        case BAL_LENGTH_NAO_SUPORTADO:
        case BAL_PARITY_NAO_SUPORTADO:
        case BAL_STOPBITS_NAO_SUPORTADO:
        case BAL_BAUDRATE_INVALIDO:
        case BAL_LENGTH_INVALIDO:
        case BAL_PARITY_INVALIDO:
        case BAL_STOPBITS_INVALIDO:
        case BAL_COMBINACAO_DE_PARAMETROS_INVALIDA:
        case BAL_CONFIGS_SERIAIS_NAO_SUPORTADAS_PELO_PROTOCOLO:
            intToStr(SARA_ERRO_ABERTURA_OU_FECHAMENTO_PORTA, saraError);
            break;

        case BAL_QTD_LEITURAS_INVALIDA:
        case BAL_PRECO_INVALIDO:
        case BAL_FALHA_NA_LEITURA_DO_PESO:
        case BAL_FALHA_AO_ENVIAR_PRECO:
            intToStr(SARA_TIMEOUT, saraError);
            break;

        case ERRO_NUMERO_SERIE:
        case ERRO_OBTENCAO_NUMERO_SERIE:
            intToStr(SARA_ERRO_OBTER_NUMERO_SERIE, saraError);
            break;

        case DISPOSITIVO_JA_ESTA_ABERTO:
        case RECURSO_INDISPONIVEL:
        case OPERACAO_NAO_SUPORTADA:
        case BAL_CONEXAO_ATIVA:
        case BAL_BALANCA_EM_USO:
        case BAL_BALANCA_INVALIDA:
        case BAL_NENHUMA_BALANCA_EM_USO:
        case CHAMADA_NAO_PERMITIDA:
        case BAL_COMANDO_NAO_SUPORTADO_PELA_BALANCA:
        case BAL_COMANDO_NAO_SUPORTADO_PELO_PROTOCOLO:
            intToStr(SARA_EXCECAO_GENERICA, saraError);
            break;

        default:
            intToStr(SARA_EXCECAO_GENERICA, saraError);
    }
}

jni/test.h

#ifndef TEST_H
#define TEST_H

#include <assert.h>
#include <string.h>
#include <jni.h>

#include "errors.h"

void test_loadDll();
void test_resolvePort();
void test_setDefaultScaleConfig();

#endif

jni/test.c

#include "test.h"
#include "balanca_padrao_sara.h"

extern struct ScaleConfig scaleConfig;

void test_loadDll() {
    assert(loadDll() == 0);
}

void test_resolvePort() {
    char serialPort[5];
    resolvePort(1, serialPort);
    assert(strcmp(serialPort, "COM1") == 0);

    resolvePort(6, serialPort);
    assert(strcmp(serialPort, "COM6") == 0);

    resolvePort(10, serialPort);
    assert(strcmp(serialPort, "USB1") == 0);

    resolvePort(13, serialPort);
    assert(strcmp(serialPort, "USB4") == 0);

    resolvePort(14, serialPort);
    assert(strcmp(serialPort, "Unknown") == 0);
}

void test_setDefaultScaleConfig() {
    setDefaultScaleConfig("COM3");
    assert(strcmp(scaleConfig.serialPort, "COM3") == 0);
    assert(scaleConfig.baudrate == 2400);

    setDefaultScaleConfig("USB1");
    assert(strcmp(scaleConfig.serialPort, "USB1") == 0);
    assert(scaleConfig.baudrate == 9600);

    setDefaultScaleConfig("COM5");
    assert(strcmp(scaleConfig.serialPort, "COM5") == 0);
    assert(scaleConfig.baudrate == 2400);

    setDefaultScaleConfig("USB3");
    assert(strcmp(scaleConfig.serialPort, "USB3") == 0);
    assert(scaleConfig.baudrate == 9600);
}

int main() {
    printf("\nRunning tests!\n\n");
    test_loadDll();
    test_resolvePort();
    test_setDefaultScaleConfig();
    printf("\n\nTests runned successfully\n\n");
}

Makefile

# Makefile for compiling JNI code and generating DLLs for different scale models

# list of all scale models
ALL_MODELS := 0 1 2 3

# set the default scale model if not already defined
ifndef MODEL
MODEL = 3
endif

# define the dll name based on the scale model
ifeq ($(MODEL),0)
MODEL_NAME = DP30T
else ifeq ($(MODEL),1)
MODEL_NAME = SA100
else ifeq ($(MODEL),2)
MODEL_NAME = DPSC
else ifeq ($(MODEL),3)
MODEL_NAME = DP30CK
else
$(error Unsupported scale model: $(MODEL))
endif

# define the target dll name
output_dll = ECTSARA_BAL_ELGIN_$(MODEL_NAME).dll

output_dir_x86 = release_dlls/$(MODEL_NAME)/x86
output_dir_x64 = release_dlls/$(MODEL_NAME)/x64
output_dll_x86 = $(output_dir_x86)/$(output_dll)
output_dll_x64 = $(output_dir_x64)/$(output_dll)

e1_dll_x86 = e1_balanca/x86/E1_Balanca01.dll
e1_dll_x64 = e1_balanca/x64/E1_Balanca01.dll

e1_dll = E1_Balanca01.dll
dir_class = com/ccibm/ect/perifericos
package = com.ccibm.ect.perifericos
class = BalancaPadraoSara
native = com_ccibm_ect_perifericos_$(class)
jni_include = "C:\Program Files\Java\jdk1.6.0_23\include"
jni_include_win = "C:\Program Files\Java\jdk1.6.0_23\include\win32" 
jni_include_x86 = "C:\Program Files (x86)\Java\jdk1.6.0_23\include"
jni_include_win_x86 = "C:\Program Files (x86)\Java\jdk1.6.0_23\include\win32"
error_f = errors.c
test = test

java_x86 = 'C:\Program Files (x86)\Java\jdk1.6.0_23\bin\java.exe'
javac_x86 = 'C:\Program Files (x86)\Java\jdk1.6.0_23\bin\javac.exe'

.PHONY: all compile run clean header dll_x86 dll_x64 dll_all sig signatures

compile:
	javac $(dir_class)/*.java

run:
	java $(package).Test

compile_x86:
	$(javac_x86) $(dir_class)/*.java
run_x86:
	$(java_x86) $(package).Test

clean:
	rm $(dir_class)/*.class

# "make header" to generate the .h file
header:
	mkdir -p jni
	javac $(dir_class)/$(class).java
	javah $(package).$(class)
	mv $(native).h jni
	rm $(dir_class)/$(class).class

dll: jni/$(native).c
	gcc $< jni/$(error_f) $(e1_dll) -I$(jni_include) -I$(jni_include_win) -shared -o $(output_dll)
	file $(output_dll)
	nm $(output_dll) | grep "Java" || true
	ldd $(output_dll)

test: jni/$(test).c jni/$(test).h
	gcc $< jni/$(native).c jni/$(error_f) -I$(jni_include) -I$(jni_include_win) -o $(test).exe
	./$(test).exe
	rm $(test).exe

dll_x86: jni/$(native).c | $(output_dir_x86)
	cp $(e1_dll_x86) $(output_dir_x86)
	i686-w64-mingw32-gcc -m32 $< jni/$(error_f) $(e1_dll_x86) -I$(jni_include_x86) -I$(jni_include_win_x86) -shared -o $(output_dll_x86)
	file $(output_dll_x86)
	nm $(output_dll_x86) | grep "Java" || true
	ldd $(output_dll_x86)

dll_x64: jni/$(native).c | $(output_dir_x64)
	cp $(e1_dll_x64) $(output_dir_x64)
	gcc -m64 $< jni/$(error_f) $(e1_dll_x64) -I$(jni_include) -I$(jni_include_win) -shared -o $(output_dll_x64)
	file $(output_dll_x64)
	nm $(output_dll_x64) | grep "Java" || true
	ldd $(output_dll_x64)

dll_allv: dll_x86 dll_x64

$(output_dir_x86):
	mkdir -p $(output_dir_x86)
$(output_dir_x64):
	mkdir -p $(output_dir_x64)
		
dll_all: 
	$(foreach model,$(ALL_MODELS), \
		$(MAKE) MODEL=$(model) dll_x64 dll_x86; \
	)

# "make sig" to ask the user for a class name, then print the field and method signatures for that class
sig:
	@bash -c 'read -p "Fully-qualified class name (example: java.util.List) ? " CLASSNAME && javap -s $$CLASSNAME';
		
# "make signatures" to print the field and method signatures for the EasyD2XX class
signatures:
	javac com/ccibm/ect/perifericos/BalancaPadraoSara.java -d bin
	javap -s -p bin/BalancaPadraoSara.class


com/ccibm/ect/perifericos/BalancaPadraoSara.java

package com.ccibm.ect.perifericos;

public class BalancaPadraoSara {

    static {
        try {
            System.loadLibrary("ECTSARA_BAL_ELGIN_DP30CK");
        } catch(UnsatisfiedLinkError e) {
            System.out.println("Problems finding library: " + e.getMessage());
        }
    }

    /**
     * Obtém o número de série do equipamento
     * 
     * @return valor do número de série armazenado na memória interna do equipamento
     * @exception java.lang.Exception
     */
    private native String obterNumeroSerie() throws Exception;

    /**
     * Obtém o peso de um dado object
     * 
     * @param portaSerial valor padrão da porta de entrada, se não informado o valor utilizado pela DLL
para comunicação deverá ser 3 – COM3 – se informado, os valores referenciados 
são os mesmos do atributo portaSerial da seção subsequente
     * @return Peso lido em gramas, retornar -1 quando o peso não estiver estável
     * @exception java.lang.Exception
     */
    private native String lerPeso(int portaSerial) throws Exception;

    /**
     * Método responsável por receber dados desse periferico
     * 
     * @return String representando valor recebido do periferico
     * @exception java.lang.Exception
     */
    public String obterPeso() throws Exception {
        boolean isPesoEstavel = false;
        int tentativasLeitura = 0;
        String retorno = null;
        String peso = null;
        while (!isPesoEstavel) {
            tentativasLeitura++;
            peso = lerPeso(4); // this.getPortaSerial()
            // Balança ESTAVEL
            if (!peso.equals("-1")) { // PESO_INCORRETO
                isPesoEstavel = true;
                retorno = peso;
                // Balança OSCILANDO
            } else if (tentativasLeitura > 10) { // MAX_TENTATIVAS
                retorno = "0";
                break;
            }
        }
        return retorno;
    }

    public String numeroSerie() throws Exception {
        return obterNumeroSerie();
    }
}

com/ccibm/ect/perifericos/SaraPerifericosException.java
com/ccibm/ect/perifericos/Test.java