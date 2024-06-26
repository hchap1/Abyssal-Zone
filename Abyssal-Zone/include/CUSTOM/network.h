#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include "CUSTOM/renderer.h"
#include "CUSTOM/packet.h"
#include <map>
#include <cmath>
using namespace std;

vector<string> split_with_delimiter(const string& input, const string& delimiter) {
    vector<string> result;
    size_t start = 0;
    size_t end;

    while ((end = input.find(delimiter, start)) != string::npos) {
        end += delimiter.length();
        string component = input.substr(start, end - start);
        if (!component.empty() && !all_of(component.begin(), component.end(), [](char c) { return c == '\0'; })) {
            if (component.back() == '!' && component.front() == '<') {
                result.push_back(component);
            }
        }
        start = end;
    }

    if (start < input.length()) {
        string component = input.substr(start);
        if (!component.empty() && !all_of(component.begin(), component.end(), [](char c) { return c == '\0'; })) {
            if (component.back() == '!' && component.front() == '<') {
                result.push_back(component);
            }
        }
    }

    return result;
}



float r4dp(float number) {
    return std::round(number * 10000.0) / 10000.0;
}

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
    if (encoded.find('.') == std::string::npos) {
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
    else {
        vector<string> components = splitString(encoded, ':');
        return make_tuple(components[0], stoi(components[1]));
    }
}

class PlayerData {
public:
    float x, y;
    bool crouching, isRed;
    int frame, direction;
    string name;
    PlayerData() : x(0.0f), y(0.0f), crouching(false), frame(0), direction(0), isRed(false) {}
    PlayerData(float x, float y, bool crouching, int frame, int direction, bool isRed) : x(x), y(y), 
        crouching(crouching), frame(frame), direction(direction), isRed(isRed) {

    }
};

class EnemyData {
public:
    float x, y;
    EnemyData() : x(0.0f), y(0.0f) {}
    EnemyData(float x, float y) : x(x), y(y) {

    }
};

class Client {
public:
    //  name    data
    map<string, PlayerData> multiplayerData;
    map<string, EnemyData> enemyData;

    Client() {}
    Client(string joinCode, float halfPlayerWidth, 
        float halfPlayerHeight, float* px, float* py, 
        bool* ic, float* frame, float* direction,
        string ID, bool* RCV, string* RCV_str, float blockWidth, 
        float blockHeight, int* health, float* rFT) : frame(frame),
        direction(direction), ID(ID), RCV(RCV), RCV_str(RCV_str),
        blockWidth(blockWidth), blockHeight(blockHeight),
        health(health), rFT(rFT){
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
        string tilemap_buffer = "";
        string coord_buffer = "";
        while (running) {
            char buffer[500000];
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
                    vector<string> packets = split_with_delimiter(message, "!");
                    for (string packet : packets) {
                        string identifier = splitString(packet, '>')[0].substr(1);
                        string data = splitString(splitString(packet, '>')[1], '!')[0];
                        // Entire tilemap sent over...
                        if (identifier == "tilemap_info") {
                            if (data == "1") {
                                tilemap_buffer = "";
                                coord_buffer = "";
                            }
                            if (data == "0") {
                                tilemap_buffer[tilemap_buffer.length() - 1] = '!';
                                *RCV_str = "initial>" + coord_buffer + "|" + tilemap_buffer;
                                *RCV = true;
                            }
                        }
                        // Spawn coordinates in format x,y
                        if (identifier == "sp") {
                            coord_buffer = data;
                        }
                        // Entire tilemap row
                        if (identifier == "tmr") {
                            tilemap_buffer += data + "/";
                        }

                        // Block changed
                        if (identifier == "bc") {

                        }

                        // Player connected
                        if (identifier == "pcon" && data != ID) {
                            if (multiplayerData.find(data) == multiplayerData.end()) {
                                multiplayerData[data] = PlayerData();
                            }
                            string reply = "<pexi>" + ID + "!" + "<pp>" + ID + "," + to_string(r4dp((*playerX + hpw * 1.5f) / (-blockWidth))) + "," + to_string(r4dp((*playerY + hph) / (-blockHeight))) + "!";;
                            send(clientSocket, reply.data(), strlen(reply.data()), 0);
                        }
                        // Player replying to connection packet
                        if (identifier == "pexi" && data != ID) {
                            if (multiplayerData.find(data) == multiplayerData.end() || true) {
                                multiplayerData[data] = PlayerData();
                            }
                        }
                        // Player disconnected
                        if (identifier == "pdis") {
                            multiplayerData.erase(data);
                        }
                        // Player position packet
                        if (identifier == "pp") {
                            vector<string> components = splitString(data, ',');
                            if (multiplayerData.find(components[0]) != multiplayerData.end()) {
                                multiplayerData[components[0]].x = stof(components[1]);
                                multiplayerData[components[0]].y = stof(components[2]);
                            }
                        }
                        // Player frame update
                        if (identifier == "pf") {
                            vector<string> components = splitString(data, ',');
                            if (multiplayerData.find(components[0]) != multiplayerData.end()) {
                                multiplayerData[components[0]].frame = stoi(components[1]);
                            }
                        }
                        // Player crouching update
                        if (identifier == "pc") {
                            vector<string> components = splitString(data, ',');
                            if (multiplayerData.find(components[0]) != multiplayerData.end()) {
                                multiplayerData[components[0]].crouching = components[1] == "1";
                            }
                        }
                        // Player direction update
                        if (identifier == "pd") {
                            vector<string> components = splitString(data, ',');
                            if (multiplayerData.find(components[0]) != multiplayerData.end()) {
                                multiplayerData[components[0]].direction = stoi(components[1]);
                            }
                        }
                        // Health packet
                        if (identifier == "ph") {
                            vector<string> components = splitString(data, ',');
                            if (components[0] == ID) {
                                *health += stoi(components[1]);
                                *rFT = 0.5f;
                            }
                        }
                        if (identifier == "pr") {
                            vector<string> components = splitString(data, ',');
                            if (multiplayerData.find(components[0]) != multiplayerData.end()) {
                                multiplayerData[components[0]].isRed = components[1] == "1";
                            }
                        }

                        // New enemy
                        if (identifier == "ne") {
                            if (enemyData.find(data) == enemyData.end()) {
                                enemyData[data] = EnemyData();
                            }
                        }
                        // Delete enemy
                        if (identifier == "de") {
                            enemyData.erase(data);
                        }
                        // Enemy position packet
                        if (identifier == "ep") {
                            vector<string> components = splitString(data, ',');
                            if (enemyData.find(components[0]) != enemyData.end()) {
                                enemyData[components[0]].x = stof(components[1]);
                                enemyData[components[0]].y = stof(components[2]);
                            }
                        }
                    }
                }
            }
        }
    }

    void terminate() {
        running = false;
    }

    int getPlayerCount() { return playerCount; }

    void sendData() {
        string m = "<pcon>" + ID + "!" + "<pp>" + ID + "," + to_string(r4dp((*playerX + hpw * 1.5f) / (-blockWidth))) + "," + to_string(r4dp((*playerY + hph) / (-blockHeight))) + "!";
        int initalSend = send(clientSocket, m.data(), strlen(m.data()), 0);
        this_thread::sleep_for(chrono::milliseconds(10));
        while (running) {
            this_thread::sleep_for(chrono::milliseconds(10));
            // Positional update
            if (*playerX != lastState.x || *playerY != lastState.y) {
                string message = "<pp>" + ID + "," + to_string(r4dp((*playerX + hpw * 1.5f) / (-blockWidth))) + "," + to_string(r4dp((*playerY + hph) / (-blockHeight))) + "!";
                int bytesSent = send(clientSocket, message.data(), strlen(message.data()), 0);
            }
            if (*frame != lastState.frame) {
                string message = "<pf>" + ID + "," + to_string(*frame) + "!";
                int bytesSent = send(clientSocket, message.data(), strlen(message.data()), 0);
            }
            if (*direction != lastState.direction) {
                string message = "<pd>" + ID + "," + to_string(*direction) + "!";
                int bytesSent = send(clientSocket, message.data(), strlen(message.data()), 0);
            }
            if (*crouching != lastState.crouching) {
                string crString = "0";
                if (*crouching) { crString = "1"; }
                string message = "<pc>" + ID + "," + crString + "!";
                int bytesSent = send(clientSocket, message.data(), strlen(message.data()), 0);
            }
            if (*health <= 0) {
                string message = "<pdis>" + ID + "!";
                int bytesSent = send(clientSocket, message.data(), strlen(message.data()), 0);
            }
            if (*rFT > 0.0f != lastState.isRed) {
                string bStr = "0";
                if (*rFT > 0.0f) {
                    bStr = "1";
                }
                string message = "<pdis>" + ID + "," + bStr + "!";
                int bytesSent = send(clientSocket, message.data(), strlen(message.data()), 0);
            }
            lastState.x = *playerX;
            lastState.y = *playerY;
            lastState.crouching = *crouching;
            lastState.direction = *direction;
            lastState.frame = *frame;
            lastState.isRed = *rFT > 0.0f;
        }
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
    float* rFT;
    float blockWidth;
    float blockHeight;
    PlayerData lastState;

    bool* RCV;
    int* health;
    string* RCV_str;
};