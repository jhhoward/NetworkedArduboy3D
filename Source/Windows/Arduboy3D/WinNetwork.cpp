#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include "Platform.h"
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "9999"
WSADATA wsaData;
SOCKET ConnectSocket = INVALID_SOCKET;
extern bool running;

bool InitNetwork()
{
	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return false;
	}

	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return false;
	}

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	// Connect to server.
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("Could not connect to server\n");
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		return false;
	}

	setsockopt(ConnectSocket, SO_SNDBUF, 0, 0, 0);
	setsockopt(ConnectSocket, SO_RCVBUF, 0, 0, 0);

	return true;
}

void ShutdownNetwork()
{
	// shutdown the send half of the connection since no more data will be sent
	int iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
	}

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();
}

bool PlatformNet::IsAvailable()
{
	if (ConnectSocket == INVALID_SOCKET)
		return false;

	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(ConnectSocket, &readSet);
	TIMEVAL timeout;
	timeout.tv_sec = timeout.tv_usec = 0;

	int result = select(0, &readSet, nullptr, nullptr, &timeout);
	return result > 0;
}

bool PlatformNet::IsAvailableForWrite()
{
	return ConnectSocket != INVALID_SOCKET;
}

bool didPeek = false;
uint8_t peekedData = 0;

uint8_t PlatformNet::Read()
{
	if (didPeek)
	{
		didPeek = false;
		return peekedData;
	}

	char data = 0;

	int iResult = recv(ConnectSocket, &data, 1, 0);
	if (iResult == 0)
	{
		printf("Connection closed\n");
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		running = false;
	}
	else if (iResult == SOCKET_ERROR)
	{
		printf("recv failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		running = false;
	}

	return data;
}

uint8_t PlatformNet::Peek()
{
	peekedData = Read();
	didPeek = true;
	return peekedData;
}

void PlatformNet::Write(uint8_t data)
{
	char sendBuffer = data;
	// Send an initial buffer
	int iResult = send(ConnectSocket, &sendBuffer, 1, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		running = false;
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		//WSACleanup();
	}
}

char PlatformNet::GenerateRandomNetworkToken()
{
	srand((unsigned int) time(NULL));
	return (char) rand() | 1;
}
