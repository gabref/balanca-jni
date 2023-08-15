#ifndef TEST_H
#define TEST_H

#include <assert.h>
#include <string.h>
#include <jni.h>

#include "errors.h"
#include "balanca_padrao_sara.h"

void test_loadDll();
void test_resolvePort();
void test_setDefaultScaleConfig();
void test_randomSerialNumber();

#endif