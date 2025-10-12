#include <iostream>
#include <string>
#include <regex>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <ws2tcpip.h>
#include <conio.h>
#include <chrono>
#include <windows.h>
#include <thread>
#include <atomic>


std::string ip;
std::string name;
int port = -1;
bool end = false;
std::string userBuffer;

BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT || signal == CTRL_LOGOFF_EVENT || signal == CTRL_SHUTDOWN_EVENT) {
        end = true;
    }
    return TRUE;
}

void getMessage(SOCKET sock, sockaddr_in addr) {
    int counter = 0;
    char mbuffer[1024];
    sockaddr_in fromAddr{};
    int len = sizeof(fromAddr);
    DWORD timeout = 10;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    int mess = recvfrom(sock, mbuffer, sizeof(mbuffer) - 1, 0, (sockaddr*)&fromAddr, &len);
    if (mess == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAETIMEDOUT) std::cerr << "Failed to receive message: " << err << std::endl;
        timeout = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    }
    timeout = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    mbuffer[mess] = '\0';
    if (mess > 0) {
        std::cout << mbuffer << std::endl;
    }
}

std::string getInputNonBlocking() {
    if (_kbhit()) {
        char ch = _getch();
        if (ch == '\r') {
            std::string result = userBuffer;
            userBuffer.clear();
            std::cout << '\n';
            return result;
        } else if (ch == '\b') {
            if (!userBuffer.empty()) {
                userBuffer.pop_back();
                std::cout << "\b \b" << std::flush;
            }
        } else {
            userBuffer += ch;
            std::cout << ch << std::flush;
        }
    }
    return "";
}



void start(bool undefined = false) {
    ip = "";
    port = -1;
    if (undefined) {
        std::cerr << "Invalid input (Not correct IP address or Port, or maybe name is empty)" << std::endl;
        ip = "exit";
        return;
    }
    std::cout << "Enter the server IP: " << std::flush;
    std::getline(std::cin, ip);
    if (ip.empty()) return start(true);
    if (ip == "exit") return;
    std::regex pattern(R"(^(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])(\.(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])){3}$)");
    if (!std::regex_match(ip, pattern) && ip != "exit") return start(true);
    std::cout << "Enter the server port: " << std::flush;
    std::cin >> port;
    std::cin.ignore();
    if (port < 0 || port > 65535) return start(true);
    std::cout << "Enter your name: " << std::flush;
    std::getline(std::cin, name);
    if (name.empty()) return start(true);

}
bool tryConnect(SOCKET sock, sockaddr_in addr) {
    std::string testMes = "CONNECT:" + name;
    int test = sendto(sock, testMes.c_str(), testMes.length(), 0, (sockaddr*)&addr, sizeof(addr));
    if (test == SOCKET_ERROR) {
        std::cerr << "Failed to send message: " << WSAGetLastError() << std::endl;
        return false;
    } else {
        std::cout << "Waiting for a response from server..." << std::endl;
        DWORD timeout = 5000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        char testBuffer[1024];
        sockaddr_in fromAddr{};
        int fromLen = sizeof(fromAddr);
        int recieved = recvfrom(sock, testBuffer, sizeof(testBuffer) - 1, 0, (sockaddr*)&fromAddr, &fromLen);
        if (recieved == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAETIMEDOUT) std::cerr << "Couldn't recieve message " << err << std::endl;
            else std::cerr << "Failed to receive message: " << err << std::endl;
            return false;
        }
        testBuffer[recieved] = '\0';
        //std::cout << "Response: " << testBuffer << std::endl;
        std::string sBuffer = testBuffer;
        if (sBuffer == "yeah:done") {
            std::cout << "Successfully connected!" << std::endl;
        }
    }
    return true;
}

bool ping(SOCKET sock, sockaddr_in addr) {
    std::string toSend = "ping";
    int sendm = sendto(sock, toSend.c_str(), toSend.length(), 0, (sockaddr*)&addr, sizeof(addr));
    if (sendm == SOCKET_ERROR) {
        std::cerr << "Failed to send message: " << WSAGetLastError() << std::endl;
        return false;
    }
    DWORD timeout = 5000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    char pingBuf[1024];
    sockaddr_in fromAddr{};
    int fromlen = sizeof(fromAddr);
    int recieved = recvfrom(sock, pingBuf, sizeof(pingBuf) - 1, 0, (sockaddr*)&fromAddr, &fromlen);
    if (recieved == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAETIMEDOUT) std::cerr << "No ping lol: " << err << std::endl;
        else std::cerr << "Failed to receive message: " << err << std::endl;
        return false;
    }
    pingBuf[recieved] = '\0';
    timeout = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    return true;
}
bool sendMessage(std::string msg, SOCKET sock, sockaddr_in addr) {
    std::string tosend = "message:" + msg;
    int sendm = sendto(sock, tosend.c_str(), tosend.length(), 0, (sockaddr*)&addr, sizeof(addr));
    if (sendm == SOCKET_ERROR) {
        std::cerr << "Failed to send message: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}
bool disconnectme(SOCKET sock, sockaddr_in addr) {
    std::string tosend = "disconnectme";
    int sendm = sendto(sock, tosend.c_str(), tosend.length(), 0, (sockaddr*)&addr, sizeof(addr));
    if (sendm == SOCKET_ERROR) {
        std::cerr << "Failed to send message: " << WSAGetLastError() << std::endl;
        return false;
    }
    std::cout << "Disconnected from server!" << std::endl;
    return true;
}
int main() {
    std::cout << "Client started!" << std::endl;
    start();
    if (ip == "exit") return 0;
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    InetPtonA(AF_INET, ip.c_str(), &addr.sin_addr);
    addr.sin_port = htons(port);

    bool connected = tryConnect(sock, addr);
    if (!connected) {
        std::cerr << "Couldn't connect to " << ip << ":" << port << ", try a new IP?" << std::endl;
        return 1;
    }
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    auto last = std::chrono::steady_clock::now();
    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
        if (ms >= 5000 && !ping(sock, addr)) {
            disconnectme(sock, addr);
            break;
        }
        if (ms >= 5000) last = now;
        if (end) {
            disconnectme(sock, addr);
            break;
        }
        std::string toSend = getInputNonBlocking();
        if (!toSend.empty()) sendMessage(toSend, sock, addr);
        if (ms >= 1000) {
            getMessage(sock, addr);
        }
    }
    std::cout << "Main loop ended! app is shutting down." << std::endl;
    closesocket(sock);
    WSACleanup();
    SetConsoleCtrlHandler(ConsoleHandler, FALSE);
    return 0;
}