#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "CUSTOM/renderer.h"
#include "CUSTOM/packet.h"
using namespace std;

tuple<string, int> decodeIP(string encodedIP) {
    string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    size_t dotLocation = alphabet.find(encodedIP[0]);
    size_t portLocation = alphabet.find(encodedIP[1]);
    string digit1 = encodedIP.substr(2, dotLocation - 2);
    string digit2 = encodedIP.substr(dotLocation, (portLocation + 1) - dotLocation);
    int port = stoi(encodedIP.substr(portLocation + 1));
    return make_tuple("192.168." + digit1 + "." + digit2, port);
}

class Client {
public:
    Client() {

    }
    Client(std::string joinCode, RenderLayer* multiplayerRenderer, float halfPlayerWidth, float halfPlayerHeight, int* triRef, float* px, float* py, bool* ic) {
        cout << "Beginning client initialization." << endl;
        playerRenderer = multiplayerRenderer;
        multiplayerTriangleCount = triRef;
        hpw = halfPlayerWidth;
        hph = halfPlayerHeight;
        playerX = px;
        playerY = py;
        crouching = ic;
        tuple<string, int> serverData = decodeIP(joinCode);
        cout << get<0>(serverData) << " @ " << get<1>(serverData) << endl;
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

        address = get<0>(serverData);
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr);
        serverAddr.sin_port = htons(get<1>(serverData));
        connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    }
    void recvData() {
        while (running) {
            char buffer[1024];
            cout << "LISTENING" << endl;
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived == 0) {
                cout << "Connection closed by server." << std::endl;
                break;
            }
            else {
                buffer[bytesReceived] = '\0';
                string message(buffer);
                if (!message.empty()) {
                    cout << "RECEIVED PACKET! DECONSTRUCTING." << endl;
                    Packet packet(message);
                    int triangleCount = packet.constructPlayerVertices(playerRenderer, hpw, hph);
                    *multiplayerTriangleCount = triangleCount;
                }
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
    void terminate() {
        running = false;
    }

    void sendData() {
        while (running) {
            this_thread::sleep_for(chrono::milliseconds(16));
            string message = to_string(*playerX) + "," + to_string(*playerY) + "," + to_string(*crouching);
            int bytesSent = send(clientSocket, message.data(), strlen(message.data()), 0);
        }
    }
	
private:
    RenderLayer* playerRenderer;
	string address;
	SOCKET clientSocket;
    thread recvThread;
    bool running = true;
    float hpw;
    float hph;
    int* multiplayerTriangleCount = 0;
    float* playerX;
    float* playerY;
    bool* crouching;
};