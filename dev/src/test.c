#include "test.h"

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

void test_randomSerialNumber() {
    // test random number
    for (int i = 0; i < 5; i++) {
        int min = 5;
        int max = 100;
        int result = randomInRange(min, max);
        assert(result >= min && result <= max);
    }

    // test random in random
    for (int i = 0; i < 5; i++) {
        int min = 0;
        int max = 100;
        int n = 10;
        int result = randomInRandom(min, max, n);
        assert(result >= min && result <= max);
    }

    // test serial random number
    char serialNumber[13];
    generateSerialNumber(serialNumber);

    // ensure the generated serial number matches the expected format "ELGECTAAAAXXXX"
    int year;
    int randomNumber;
    sscanf(serialNumber, "ELGECT%04d%04d", &year, &randomNumber);

    // get the current year
    time_t t = time(NULL);
    struct tm *currentTime = localtime(&t);
    int currentYear = currentTime->tm_year + 1900;

    assert(year == currentYear);
    assert(randomNumber >= 1000 && randomNumber <= 9999);
}

int main() {
    printf("\nRunning tests!\n\n");
    test_loadDll();
    test_resolvePort();
    test_setDefaultScaleConfig();
    test_randomSerialNumber();
    printf("\n\nTests runned successfully\n\n");
}