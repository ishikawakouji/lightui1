#pragma once

#include <iostream>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

class TcpSender
{
public:
	TcpSender() {};
	~TcpSender() {

	}

private:
    // IPアドレス
#define IPADDR_SIZE 16
    char ipAddr[IPADDR_SIZE];
    int ipPort;

    int iResult;
    WSADATA wsaData;

    SOCKET ConnectSocket = INVALID_SOCKET;
    struct sockaddr_in clientService;

#define DEFAULT_BUFLEN 64

    int recvbuflen = DEFAULT_BUFLEN;
    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN];

    bool flagConnecting = false;


public:
    void SetServer(char* ipaddr, int port) {
        memcpy(ipAddr, ipaddr, IPADDR_SIZE);
        ipPort = port;
    }

    bool Connected() { return flagConnecting; }


    int Connect() {

        // もともとの電源は、192.168.0.30 1000 へ
        if (flagConnecting) return 0;

        //----------------------
        // Initialize Winsock
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != NO_ERROR) {
            wprintf(L"WSAStartup failed with error: %d\n", iResult);
            return 1;
        }

        //----------------------
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ConnectSocket == INVALID_SOCKET) {
            wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        //----------------------
        // The sockaddr_in structure specifies the address family,
        // IP address, and port of the server to be connected to.
        clientService.sin_family = AF_INET;

        clientService.sin_addr.s_addr = inet_addr(ipAddr);
        //clientService.sin_addr.s_addr = inet_addr("127.0.0.1"); // とりあえず
        clientService.sin_port = htons(ipPort);

        //----------------------
        // Connect to server.
        iResult = connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService));
        if (iResult == SOCKET_ERROR) {
            wprintf(L"connect failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        flagConnecting = true;

        return 0;
	}

    int Send(char* command)
    {
        //----------------------
        // Send an initial buffer
        // iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
        //sprintf_s(sendbuf, DEFAULT_BUFLEN, "W12010000");
        sprintf_s(sendbuf, DEFAULT_BUFLEN, command);

        iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return -1;
        }

        printf("Bytes Sent: %d\n", iResult);

        // Receive until the peer closes the connection
        // なくなるまで取得してもいいが、とりあえず一回だけ聞く
        memset(recvbuf, '\0', recvbuflen);
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n[%s]\n", iResult, recvbuf);
        }
        else if (iResult == 0)
            wprintf(L"Connection closed\n");
        else
            wprintf(L"recv failed with error: %d\n", WSAGetLastError());


        return iResult;
    }

    int Close() {
        if (!flagConnecting) {
            return 0;
        }

        flagConnecting = false;

        // shutdown the connection since no more data will be sent
        iResult = shutdown(ConnectSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        // close the socket
        iResult = closesocket(ConnectSocket);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"close failed with error: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        WSACleanup();

        return 0;

    }

};