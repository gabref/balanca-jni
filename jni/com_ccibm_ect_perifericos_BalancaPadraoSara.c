#include "com_ccibm_ect_perifericos_BalancaPadraoSara.h"
// #include "e1_balanca.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define GET_FUNCTION_PTR(hDLL, funcName)                             \
  funcName = (typeof(funcName))GetProcAddress(hDLL, #funcName);      \
  if (funcName == NULL)                                              \
  {                                                                  \
    printf("Failed to get function pointer for '%s'.\n", #funcName); \
    FreeLibrary(hDLL);                                               \
    exit(1);                                                        \
  }


// get pointer to functions
int (*ObterModeloBalanca)();
int (*AbrirSerial)(const char *, int, int, char, int);
int (*Fechar)();
const char * (*LerPeso)(int);

HINSTANCE hDll;

void loadDll() {
  hDll = LoadLibrary("E1_Balanca01.dll");
  if (hDll == NULL) {
    printf("Failed to load the DLL.\n");
    exit(1);
  }
  printf("Library loaded\n");

  // ObterModeloBalanca = (int (*)())GetProcAddress(hDll, "ObterModeloBalanca");
  // AbrirSerial = (int (*)(const char *, int, int, char, int))GetProcAddress(hDll, "AbrirSerial");
  // Fechar = (int (*)())GetProcAddress(hDll, "Fechar");
  // LerPeso = (const char *(*)(int))GetProcAddress(hDll, "LerPeso");

  GET_FUNCTION_PTR(hDll, ObterModeloBalanca);
  GET_FUNCTION_PTR(hDll, AbrirSerial);
  GET_FUNCTION_PTR(hDll, Fechar);
  GET_FUNCTION_PTR(hDll, LerPeso);

  printf("Functions loaded\n");
}

void unloadDll() {
  FreeLibrary(hDll);
  printf("Library unloaded\n");
}

JNIEXPORT jstring JNICALL Java_com_ccibm_ect_perifericos_BalancaPadraoSara_obterNumeroSerie
  (JNIEnv *env , jobject obj) {
    loadDll();

    int modelo = ObterModeloBalanca();

    char buff[5];
    snprintf(buff, sizeof(buff), "%d", modelo);
    jstring ret = (*env)->NewStringUTF(env, buff);

    unloadDll();
    return ret;
  }

JNIEXPORT jstring JNICALL Java_com_ccibm_ect_perifericos_BalancaPadraoSara_lerPeso
  (JNIEnv *env , jobject obj, jint portaSerial) {
    loadDll();

    char *buff = (char *) malloc(sizeof(char) * 10);
    strcpy(buff, "20.43");
    jstring ret = (*env)->NewStringUTF(env, buff);

    unloadDll();
    return ret;
  }
