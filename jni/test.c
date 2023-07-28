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

    setDefaultScaleConfig("USB1");
    assert(strcmp(scaleConfig.serialPort, "USB1") == 0);

    setDefaultScaleConfig("COM5");
    assert(strcmp(scaleConfig.serialPort, "COM5") == 0);

    setDefaultScaleConfig("USB3");
    assert(strcmp(scaleConfig.serialPort, "USB3") == 0);
}

int main() {
    printf("\nRunning tests!\n\n");
    test_loadDll();
    test_resolvePort();
    test_setDefaultScaleConfig();
    printf("\n\nTests runned successfully\n\n");
}