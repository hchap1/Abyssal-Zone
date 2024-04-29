#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include <chrono>
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
    Client(std::string joinCode, RenderLayer* multiplayerRenderer, float halfPlayerWidth, float halfPlayerHeight) {
        playerRenderer = multiplayerRenderer;
        hpw = halfPlayerWidth;
        hph = halfPlayerHeight;
        tuple<string, int> serverData = decodeIP(joinCode);
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
                break;
            }
            else {
                buffer[bytesReceived] = '\0';
                string message(buffer);
                if (!message.empty()) {
                    decodePacket(message);
                }
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }

    void terminate() {
        running = false;
    }

    void decodePacket(string packet) {
        // Packet: packet:ENEMYDATA|PLAYERDATA
        // ENEMYDATA: x,y,ID/x,y,ID
        // PLAYERDATA: x,y,crouching/x,y,crouching
        vector<string> packetData = splitString(packet, '|');
        string enemyData = packetData[0];
        string playerData = packetData[1];
        vector<string> enemies = splitString(enemyData, '/');
        vector<string> players = splitString(playerData, '/');
        vector<string> data;
        const size_t playerArraySize = sizeof(players) * 30;
        float playerVertexData[playerArraySize];
        int index = 0;
        for (string player : players) {
            data = splitString(player, ',');
            float xPos = stof(data[0]);
            float yPos = stof(data[1]);
            float crouching = 0.0f;
            if (data[2] == "true") { crouching = 1.0f; }
            playerVertexData[index++] = -hpw;
            playerVertexData[index++] = -hph;
            playerVertexData[index++] = 0.0f;
            playerVertexData[index++] = 0.5f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] = -hpw;
            playerVertexData[index++] =  hph;
            playerVertexData[index++] = 0.0f;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] =  hpw;
            playerVertexData[index++] = -hph;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = 0.5f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] =  hpw;
            playerVertexData[index++] =  hph;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] = -hpw;
            playerVertexData[index++] =  hph;
            playerVertexData[index++] = 0.0f;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = crouching;

            playerVertexData[index++] =  hpw;
            playerVertexData[index++] = -hph;
            playerVertexData[index++] = 1.0f;
            playerVertexData[index++] = 0.5f;
            playerVertexData[index++] = crouching;
        }
    }
	
private:
    RenderLayer* playerRenderer;
	string address;
	SOCKET clientSocket;
    thread recvThread;
    float* playerX;
    float* playerY;
    bool running = true;
    float hpw;
    float hph;
};