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


std::string removeNullChars(const std::string& input) {
    std::string result = input;
    result.erase(std::remove(result.begin(), result.end(), '\0'), result.end());
    return result;
}

std::vector<std::string> split_with_delimiter(const std::string& input, const std::string& delimiter) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end;

    while ((end = input.find(delimiter, start)) != std::string::npos) {
        end += delimiter.length();
        std::string component = input.substr(start, end - start);
        if (!component.empty() && !all_of(component.begin(), component.end(), [](char c) { return c == '\0'; })) {
            if (component.back() == '!' && component.front() == '<') {
                result.push_back(component);
            }
        }
        start = end;
    }

    if (start < input.length()) {
        std::string component = input.substr(start);
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

std::string encodeIP(std::string IP, int port) {
    std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string ID = IP.substr(8);
    size_t dotIndex = ID.find('.');
    std::string a = ID.substr(0, dotIndex);
    std::string b = ID.substr(dotIndex + 1);
    size_t portIndex = ID.length() - 1;
    std::string aLetter = "";
    aLetter += alphabet[dotIndex];
    std::string bLetter = "";
    bLetter += alphabet[portIndex];
    return aLetter + bLetter + a + b + std::to_string(port);
}

std::tuple<std::string, int> decodeIP(std::string encoded) {
    if (encoded.find('.') == std::string::npos) {
        std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        size_t dotIndex = alphabet.find(encoded[0]);
        size_t portIndex = alphabet.find(encoded[1]);
        std::string IPstr = encoded.substr(2);
        int port = std::stoi(IPstr.substr(portIndex));
        IPstr.replace(portIndex, IPstr.length() - portIndex, "");
        std::string a = IPstr.substr(0, dotIndex);
        std::string b = IPstr.substr(dotIndex);
        return std::make_tuple("192.168." + a + "." + b, port);
    }
    else {
        std::vector<std::string> components = splitString(encoded, ':');
        return std::make_tuple(components[0], std::stoi(components[1]));
    }
}

class PlayerData {
public:
    float x, y;
    bool crouching, isRed;
    int frame, direction;
    std::string name;
    PlayerData() : x(0.0f), y(0.0f), crouching(false), frame(0), direction(0), isRed(false) {}
    PlayerData(float x, float y, bool crouching, int frame, int direction, bool isRed) : x(x), y(y), 
        crouching(crouching), frame(frame), direction(direction), isRed(isRed) {

    }
};

class EnemyData {
public:
    float x, y, rotation;
    EnemyData() : x(0.0f), y(0.0f), rotation(0.0f) {}
    EnemyData(float x, float y, float rotation) : x(x), y(y), rotation(rotation) {}
};

class Client {
public:
    //  name    data
    std::map<std::string, PlayerData> multiplayerData;
    std::map<std::string, EnemyData> enemyData;

    Client() {}
    Client(std::string joinCode, float halfPlayerWidth,
        float halfPlayerHeight, float* px, float* py, 
        bool* ic, float* frame, float* direction,
        std::string ID, bool* RCV, std::string* RCV_str, float blockWidth,
        float blockHeight, int* health, float* rFT) : frame(frame),
        direction(direction), ID(ID), RCV(RCV), RCV_str(RCV_str),
        blockWidth(blockWidth), blockHeight(blockHeight),
        health(health), rFT(rFT){
        hpw = halfPlayerWidth;
        hph = halfPlayerHeight;
        playerX = px;
        playerY = py;
        crouching = ic;
        std::tuple<std::string, int> serverData = decodeIP(joinCode);
        std::cout << std::get<0>(serverData) << " @ " << std::get<1>(serverData) << std::endl;
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

        address = std::get<0>(serverData);
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr);
        serverAddr.sin_port = htons(std::get<1>(serverData));
        connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    }

    void recvData() {
        std::string tilemap_buffer = "";
        std::string coord_buffer = "";
        while (running) {
            char buffer[500000];
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived == 0) {
                std::cout << "Connection closed by server." << std::endl;
                running = false;
            }
            else if (bytesReceived == -1) {
                std::cout << "Connection closed. ERR: " << WSAGetLastError() << std::endl;
                running = false;
            }
            else {
                buffer[bytesReceived] = '\0';
                std::string message(buffer);
                if (!message.empty()) {
                    std::vector<std::string> packets = split_with_delimiter(message, "!");
                    for (std::string packet : packets) {
                        std::string identifier = splitString(packet, '>')[0].substr(1);
                        std::string data = splitString(splitString(packet, '>')[1], '!')[0];
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
                            std::string reply = "<pexi>";
                            reply += ID.c_str();
                            reply += "!";
                            reply += "<pp>"; 
                            reply += ID.c_str();
                            reply += ",";
                            reply += std::to_string(r4dp((*playerX + hpw * 1.5f) / (-blockWidth))).c_str();
                            reply += ",";
                            reply += std::to_string(r4dp((*playerY + hph) / (-blockHeight))).c_str();
                            reply += "!";
                            send(clientSocket, reply.c_str(), strlen(reply.c_str()), 0);
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
                            std::vector<std::string> components = splitString(data, ',');
                            if (multiplayerData.find(components[0]) != multiplayerData.end()) {
                                multiplayerData[components[0]].x = std::stof(components[1]);
                                multiplayerData[components[0]].y = std::stof(components[2]);
                            }
                        }
                        // Player frame update
                        if (identifier == "pf") {
                            std::vector<std::string> components = splitString(data, ',');
                            if (multiplayerData.find(components[0]) != multiplayerData.end()) {
                                multiplayerData[components[0]].frame = std::stoi(components[1]);
                            }
                        }
                        // Player crouching update
                        if (identifier == "pc") {
                            std::vector<std::string> components = splitString(data, ',');
                            if (multiplayerData.find(components[0]) != multiplayerData.end()) {
                                multiplayerData[components[0]].crouching = components[1] == "1";
                            }
                        }
                        // Player direction update
                        if (identifier == "pd") {
                            std::vector<std::string> components = splitString(data, ',');
                            if (multiplayerData.find(components[0]) != multiplayerData.end()) {
                                multiplayerData[components[0]].direction = stoi(components[1]);
                            }
                        }
                        // Health packet
                        if (identifier == "ph") {
                            std::vector<std::string> components = splitString(data, ',');
                            if (components[0] == ID.c_str()) {
                                *health += stoi(components[1]);
                                *rFT = 0.5f;
                            }
                        }
                        if (identifier == "pr") {
                            std::vector<std::string> components = splitString(data, ',');
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
                            std::vector<std::string> components = splitString(data, ',');
                            if (enemyData.find(components[0]) != enemyData.end()) {
                                enemyData[components[0]].x = std::stof(components[1]);
                                enemyData[components[0]].y = std::stof(components[2]);
                                enemyData[components[0]].rotation = std::stof(components[3]);
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
        std::string m = "<pcon>";
        m += ID.c_str();
        m += "!<pp>";
        m += ID.c_str();
        m += ", ";
        m += std::to_string(r4dp((*playerX + hpw * 1.5f) / (-blockWidth))).c_str();
        m += ", ";
        m += std::to_string(r4dp((*playerY + hph) / (-blockHeight))).c_str();
        m += "!";
        std::cout << "Initial packet: " << m << std::endl;
        int initalSend = send(clientSocket, m.c_str(), strlen(m.c_str()), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            // Positional update
            if (*playerX != lastState.x || *playerY != lastState.y) {
                std::string message = "<pp>";
                message += ID.c_str();
                message += ",";
                message += std::to_string(r4dp((*playerX + hpw * 1.5f) / (-blockWidth))).c_str();
                message += ",";
                message += std::to_string(r4dp((*playerY + hph) / (-blockHeight))).c_str();
                message += "!";
                int bytesSent = send(clientSocket, removeNullChars(message).c_str(), strlen(message.c_str()), 0);
            }
            if (*frame != lastState.frame) {
                std::string message = "<pf>";
                message += ID.c_str();
                message += ",";
                message += std::to_string(*frame).c_str();
                message += "!";
                int bytesSent = send(clientSocket, removeNullChars(message).c_str(), strlen(message.c_str()), 0);
            }
            if (*direction != lastState.direction) {
                std::string message = "<pd>";
                message += ID.c_str();
                message += ",";
                message += std::to_string(*direction).c_str();
                message += "!";
                int bytesSent = send(clientSocket, removeNullChars(message).c_str(), strlen(message.c_str()), 0);
            }
            if (*crouching != lastState.crouching) {
                std::string message = "<pc>";
                message += ID.c_str();
                message += ",";
                message += *crouching ? "1" : "0";
                message += "!";
                int bytesSent = send(clientSocket, removeNullChars(message).c_str(), strlen(message.c_str()), 0);
            }
            if (*health <= 0) {
                std::string message = "<pdis>";
                message += ID.c_str();
                message += "!";
                int bytesSent = send(clientSocket, removeNullChars(message).c_str(), strlen(message.c_str()), 0);
            }
            if (*rFT > 0.0f != lastState.isRed) {
                std::string message = "<pdis>";
                message += ID.c_str();
                message += ",";
                message += (*rFT > 0.0f) ? "1" : "0";
                message += "!";
                int bytesSent = send(clientSocket, removeNullChars(message).c_str(), strlen(message.c_str()), 0);
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
    std::string address;
	SOCKET clientSocket;
    std::thread recvThread;
    std::string ID;
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
    std::string* RCV_str;
};