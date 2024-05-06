#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "CUSTOM/renderer.h"
#include "CUSTOM/packet.h"
using namespace std;

string encodeIP(string IP, int port) {
    string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    string ID = IP.substr(8);
    size_t dotIndex = ID.find('.');
    string a = ID.substr(0, dotIndex);
    string b = ID.substr(dotIndex + 1);
    size_t portIndex = ID.length() - 1;
    string aLetter = "";
    aLetter += alphabet[dotIndex];
    string bLetter = "";
    bLetter += alphabet[portIndex];
    return aLetter + bLetter + a + b + to_string(port);
}

tuple<string, int> decodeIP(string encoded) {
    string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    size_t dotIndex = alphabet.find(encoded[0]);
    size_t portIndex = alphabet.find(encoded[1]);
    string IPstr = encoded.substr(2);
    int port = stoi(IPstr.substr(portIndex));
    IPstr.replace(portIndex, IPstr.length() - portIndex, "");
    string a = IPstr.substr(0, dotIndex);
    string b = IPstr.substr(dotIndex);
    return make_tuple("192.168." + a + "." + b, port);
}

class Client {
public:
    Client() {}
    Client(string joinCode, float halfPlayerWidth, float halfPlayerHeight, float* px, float* py, bool* ic){
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
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived == 0) {
                cout << "Connection closed by server." << std::endl;
                running = false;
            }
            else if (bytesReceived == -1) {
                cout << "CONNECTION COMMITTED SUICIDE. ERR: " << WSAGetLastError() << endl;
                running = false;
            }
            else {
                buffer[bytesReceived] = '\0';
                string message(buffer);
                if (!message.empty()) {
                    Packet packet(message);
                    playerCrouchingBools = packet.playerCrouchingBools;
                    playerXPositions = packet.playerXPositions;
                    playerYPositions = packet.playerYPositions;
                    hasVertexData = true;
                }
            }
        }
    }

    void terminate() {
        running = false;
    }

    int getPlayerCount() { return playerCount; }

    void sendData() {
        while (running) {
            this_thread::sleep_for(chrono::milliseconds(16));
            string crString = "false";
            if (*crouching) { crString = "true"; }
            string message = to_string(*playerX) + "," + to_string(*playerY) + "," + crString;
            int bytesSent = send(clientSocket, message.data(), strlen(message.data()), 0);
        }
    }

    tuple<vector<float>, vector<float>, vector<bool>, bool> getVertexArray() {
        return make_tuple(playerXPositions, playerYPositions, playerCrouchingBools, hasVertexData);
    }
	
private:
	string address;
	SOCKET clientSocket;
    thread recvThread;
    bool running = true;
    float hpw;
    float hph;
    float* playerX;
    float* playerY;
    bool* crouching;
    int playerCount;
    
    vector<float> playerXPositions;
    vector<float> playerYPositions;
    vector<bool> playerCrouchingBools;

    bool hasVertexData;
};