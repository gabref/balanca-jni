#include <stdio.h>
#include <windows.h>

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

int main() {
    const char *portName = "COM4"; // Replace with the correct serial port name
    const char *hexCommand = "1b4832"; // Replace with your hexadecimal command

    unsigned char response[256]; // Buffer to store the response data
    int responseSize = sizeof(response);

    int bytesRead = sendHexCommand(portName, hexCommand, response, responseSize);
    if (bytesRead > 0) {
        printf("Response received (%d bytes): ", bytesRead);
        for (int i = 0; i < bytesRead; i++) {
            printf("%02X ", response[i]);
        }
        printf("\n");
    } else {
        printf("Failed to send hex command or receive response.\n");
    }

    return 0;
}

