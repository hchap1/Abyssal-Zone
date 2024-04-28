#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

class Client {
public:
    Client(std::string ip, float* playerXRef, float* playerYRef) {
        playerX = playerXRef;
        playerY = playerYRef;
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

        address = ip;
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr);
        serverAddr.sin_port = htons(69);
        connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
        recvThread = thread(&Client::recvData, this);
    }
    void recvData(float* playerX, float* playerY) {
        while (true) {
            char buffer[1024];
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived == 0) {
                cout << "Connection closed by server." << std::endl;
                break;
            }
            else {
                buffer[bytesReceived] = '\0';
                string message(buffer);
                if (!message.empty()) {
                    playerX = &stof(message);
                }
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
	
private:
	string address;
	SOCKET clientSocket;
    thread recvThread;
    float* playerX;
    float* playerY;
};
