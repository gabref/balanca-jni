#include <time.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <stdbool.h>

#define MAX_TIMEOUT_MS 5000
int BAUDRATE;

int sendHexCommand(const char *portName, const char *hexCommand, unsigned char *response, int responseSize) {
    HANDLE hSerial = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error opening the serial port.\n");
        exit(1);
        return -1;
    }

    if (!SetCommMask(hSerial, EV_RXCHAR)) {
        printf("Error setting communication event mask.\n");
        CloseHandle(hSerial);
        return -1;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        printf("Error getting serial port attributes.\n");
        CloseHandle(hSerial);
        return -1;
    }

    dcbSerialParams.BaudRate = BAUDRATE;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        printf("Error setting serial port attributes.\n");
        CloseHandle(hSerial);
        return -1;
    }

    COMMTIMEOUTS commTimeouts = { 0 };
    commTimeouts.ReadIntervalTimeout = MAXDWORD;
    commTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
    commTimeouts.ReadTotalTimeoutConstant = 0;

    if (!SetCommTimeouts(hSerial, &commTimeouts)) {
        printf("Error setting serial port timeouts.\n");
        CloseHandle(hSerial);
        return -1;
    }

    DWORD dwEventMask;
    if (!SetCommMask(hSerial, EV_RXCHAR)) {
        printf("Error setting communication event mask.\n");
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

    DWORD bytesRead = 0;
    clock_t startTime = clock();
    while (bytesRead < responseSize && (clock() - startTime) < (CLOCKS_PER_SEC * (MAX_TIMEOUT_MS / 1000))) {
        DWORD availableBytes;
        COMSTAT comStat;
        if (!ClearCommError(hSerial, NULL, &comStat)) {
            printf("Error checking available bytes in the serial port.\n");
            CloseHandle(hSerial);
            return -1;
        }
        
        availableBytes = comStat.cbInQue;
        
        if (availableBytes > 0) {
            DWORD bytesToRead = min(responseSize - bytesRead, availableBytes);
            if (!ReadFile(hSerial, response + bytesRead, bytesToRead, &bytesToRead, NULL)) {
                printf("Error reading from the serial port.\n");
                CloseHandle(hSerial);
                return -1;
            }
            bytesRead += bytesToRead;
        }
    }

    CloseHandle(hSerial);
    return bytesRead; // Return the number of bytes read from the serial port (response size)
}

void printStatitics(double *testTimes, int number_tests, int successCount, int failureCount) {

    // calculate the average time
    double sum = 0;
    int validCount = 0;
    for (int i = 0; i < number_tests; i++) {
        if ((testTimes[i] * 1000) < MAX_TIMEOUT_MS) {
            sum += testTimes[i] * 1000;
            validCount++;
        }
    }
    double average = validCount > 0 ? sum / validCount : 0;

    // calculate standard deviation
    double variance = 0;
    for (int i = 0; i < number_tests; i++) {
        if ((testTimes[i] * 1000) < MAX_TIMEOUT_MS)
            variance += pow(((testTimes[i]) * 1000) - average, 2);
    }
    double standardDeviation = validCount > 0 ? sqrt(variance / validCount) : 0;
    double cvStandardDeviation = (standardDeviation / average) * 100;

    // find the best (minimum) and worst (maximum) times
    double bestTime = testTimes[0] * 1000;
    double worstTime = testTimes[0] * 1000; 
    for (int i = 1; i < number_tests; i++) {
        double testTime = testTimes[i] * 1000;
        if (testTime < bestTime) {
            bestTime = testTime;
        }
        if (testTime > worstTime && testTime < MAX_TIMEOUT_MS) {
            worstTime = testTime;
        }
    }

    // calculate success rate and failure rate
    double successRate = (double) successCount / number_tests * 100;
    double failureRate = (double) failureCount / number_tests * 100;

    printf("\n\n");
    printf("\tStatistics Summary: %d Tests\n", number_tests);
    printf("\t=====================================\n");
    printf("\tAverage Time:            %.0f ms\n", average);
    printf("\tBest Time:               %.0f ms\n", bestTime);
    printf("\tWorst Time:              %.0f ms\n", worstTime);
    printf("\tStandard Deviation:      %.3f\n", standardDeviation);
    printf("\tCV Standard Deviation:   %.0f%%\n", cvStandardDeviation);
    printf("\tSuccess Rate:            %.0f%%\n", successRate);
    printf("\tFailure Rate:            %.0f%%\n", failureRate);
    printf("\t=====================================\n");
}

void writeDataToFile(double *testTimes, int number_tests, int *testResults, const char *filename) {

    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }

    for (int i = 0; i < number_tests; i++) {
        fprintf(file, "%f,", testTimes[i]);
        if (testResults[i] == 1) {
            fprintf(file, "1\n");
        } else {
            fprintf(file, "0\n");
        }
    }

    fclose(file);
}

int testCommand(char *portName, const char *hexCommand, int numReads, double *testTimes, int *testResults, int responseSize) {
    unsigned char response[responseSize]; // Buffer to store the response data

    int success_count = 0;

    for (int j = 0; j < numReads; j++) {
        printf("%02d%% - ", (int) (j * 100 / numReads));

        clock_t start, end;
        double cpu_time_used;

        start = clock();

        int bytesRead = sendHexCommand(portName, hexCommand, response, responseSize);

        end = clock();

        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        testTimes[j] = cpu_time_used;

        if (bytesRead > 0) {
            success_count++;
            testResults[j] = 1;

            printf("ok  - %.6f\t", cpu_time_used);
            char asciiResponse[256];

            for (int i = 0; i < bytesRead; i++) {
                // printf("%02X ", response[i]);
                sprintf(&asciiResponse[i], "%c", response[i]);
            }
            printf("response: %s\r", asciiResponse);
        } else {
            testResults[j] = 0;
            printf("nok - %.6f\r", cpu_time_used);
        }
    }
    return success_count;
}

int main(int argc, char *argv[]) {
    // Default values
    char *portName = "COM4";
    BAUDRATE = CBR_19200;
    int number_tests = 100;

    if (argc > 1) {
        portName = argv[1]; // Use the COM port passed as an argument
    }
    if (argc > 2) {
        BAUDRATE = atoi(argv[2]); // Use the baud rate passed as an argument
    } 
    if (argc > 3) {
        number_tests = atoi(argv[3]); // Use the baud rate passed as an argument
    } 

    double testTimes[number_tests];
    int testResults[number_tests];

    int success_count = testCommand(portName, "05", number_tests, testTimes, testResults, 7);
    int failure_count = number_tests - success_count;
    printStatitics(testTimes, number_tests, success_count, failure_count);
    writeDataToFile(testTimes, number_tests, testResults, "times_weight.txt");

    printf("\n\n");

    success_count = testCommand(portName, "1b4832", number_tests, testTimes, testResults, 10);
    failure_count = number_tests - success_count;
    printStatitics(testTimes, number_tests, success_count, failure_count);
    writeDataToFile(testTimes, number_tests, testResults, "times_serial_number.txt");

    // system("python plot_data.py times_weight.txt");
    // system("python plot_data.py times_serial_number.txt");

    return 0;
}

