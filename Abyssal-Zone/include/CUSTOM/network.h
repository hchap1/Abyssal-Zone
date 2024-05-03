#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "CUSTOM/renderer.h"
#include "CUSTOM/packet.h"
using namespace std;

tuple<string, int> decodeIP(string encoded) {
    string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    size_t dotIndex = alphabet.find(encoded[0]);
    size_t portIndex = alphabet.find(encoded[1]);
    string IPstr = encoded.substr(2);
    string a = IPstr.substr(0, dotIndex);
    string b = IPstr.substr(dotIndex, portIndex - 2);
    int port = stoi(IPstr.substr(portIndex));
    return make_tuple("192.168." + a + "." + b, port);
}

class Client {
public:
    Client() {}
    Client(string joinCode, RenderLayer* multiplayerRenderer, float halfPlayerWidth, float halfPlayerHeight, float* px, float* py, bool* ic){
        playerRenderer = multiplayerRenderer; 
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
                    Packet packet(message);
                    int numTriangles = packet.constructPlayerVertices(playerRenderer, hpw, hph);
                }
            }
        }
    }

    void terminate() {
        running = false;
    }

    void sendData() {
        while (running) {
            this_thread::sleep_for(chrono::milliseconds(20));
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
    float* playerX;
    float* playerY;
    bool* crouching;
};