#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
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
    Client(string joinCode, float halfPlayerWidth, 
        float halfPlayerHeight, float* px, float* py, 
        bool* ic, float* frame, float* direction,
        string ID, bool* RCV, string* RCV_str) : frame(frame), 
        direction(direction), ID(ID), RCV(RCV), RCV_str(RCV_str) {
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
        bool receivedInitial = false;
        while (!receivedInitial) {
            char i_buffer[1048576];
            int i_bytesReceived = recv(clientSocket, i_buffer, sizeof(i_buffer) - 1, 0);
            if (i_bytesReceived == 0) {
                cout << "Connection closed by server before first packet." << std::endl;
                running = false;
            }
            else if (i_bytesReceived == -1) {
                cout << "Connection closed before first packet. ERR: " << WSAGetLastError() << endl;
                running = false;
            }
            else {
                i_buffer[i_bytesReceived] = '\0';
                string message(i_buffer);
                if (!message.empty()) {
                    if (message.substr(0, 9) == "<initial>") {
                        *RCV_str = message;
                        *RCV = true;
                        receivedInitial = true;
                    }
                }
            }
        }
        while (running) {
            char buffer[1024];
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived == 0) {
                cout << "Connection closed by server." << std::endl;
                running = false;
            }
            else if (bytesReceived == -1) {
                cout << "Connection closed. ERR: " << WSAGetLastError() << endl;
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
                    playerFrames = packet.playerFrames;
                    playerDirections = packet.playerDirections;
                    playerIDs = packet.playerIDs;
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
            string crString = "0";
            if (*crouching) { crString = "1"; }
            string message = to_string(*playerX) + "," + to_string(*playerY) + "," + crString + "," + to_string(*frame) + "," + to_string(*direction) + "," + ID + "!";
            int bytesSent = send(clientSocket, message.data(), strlen(message.data()), 0);
        }
    }

    tuple<vector<float>, vector<float>, vector<bool>, vector<float>, vector<float>, vector<string>, bool> getVertexArray() {
        return make_tuple(playerXPositions, playerYPositions, playerCrouchingBools, playerFrames, playerDirections, playerIDs, hasVertexData);
    }
	
private:
	string address;
	SOCKET clientSocket;
    thread recvThread;
    string ID;
    bool running = true;
    float hpw;
    float hph;
    float* playerX;
    float* playerY;
    bool* crouching;
    float* frame;
    float* direction;
    int playerCount;
    vector<vector<int>>* tilemap;
    RenderLayer* tilemapRenderer;
    int* windowWidth;
    int* windowHeight;
    float* blocksize;
    float* blockWidth;
    float* blockHeight;
    float* t;
    vector<float> playerXPositions;
    vector<float> playerYPositions;
    vector<bool> playerCrouchingBools;
    vector<float> playerFrames;
    vector<float> playerDirections;
    vector<string> playerIDs;
    bool* RCV;
    string* RCV_str;
    bool hasVertexData;
};