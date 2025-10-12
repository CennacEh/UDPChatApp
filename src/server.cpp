#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#pragma comment(lib, "ws2_32.lib")


struct Client {
    sockaddr_in addr{};
    int len;
    std::string name;
};

std::vector<Client> clients;

int main() {
    WSAData wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(4218);

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Server started on port 4218" << std::endl;
    char buffer[1024];
    sockaddr_in fromAddr{};

    while (true) {
        int fromlen = sizeof(fromAddr);
        int recieved = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&fromAddr, &fromlen);
        if (recieved == SOCKET_ERROR) {
            std::cerr << "recvfrom failed: " << WSAGetLastError() << std::endl;
            continue;
        }
        buffer[recieved] = '\0';
        std::string msg(buffer);
        if (msg.rfind("CONNECT:", 0) == 0) {
            std::string name = msg.replace(0, 8, "");
            std::cout << "Connecting " << name << " with port " << fromAddr.sin_port << std::endl;
            std::string response = "yeah:done";
            sendto(sock, response.c_str(), response.length(), 0, (sockaddr*)&fromAddr, fromlen);
            bool exists = false;
            for (auto &client : clients) {
                if (client.addr.sin_addr.s_addr == fromAddr.sin_addr.s_addr && client.addr.sin_port == fromAddr.sin_port) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                clients.push_back({ fromAddr, fromlen, name });
                for (auto &client : clients) {
                    std::string joinmes = name + " joined the Chat!";
                    sendto(sock, joinmes.c_str(), joinmes.length(), 0, (sockaddr*)&client.addr, client.len);
                }
            } else {
                sendto(sock, "You are already connected! Please restart client if this is an error.", 71, 0, (sockaddr*)&fromAddr, fromlen);
            }
        } else if (msg == "ping") {
            std::string response = "pong";
            sendto(sock, response.c_str(), response.length(), 0, (sockaddr*)&fromAddr, fromlen);
        } else if (msg == "disconnectme") {
            for (int i = 0; i < clients.size(); i++) {
                auto &client = clients[i];
                if (client.addr.sin_addr.s_addr == fromAddr.sin_addr.s_addr && client.addr.sin_port == fromAddr.sin_port) {
                    clients.erase(clients.begin() + i);
                    std::cout << "Disconnected " << client.name << std::endl;
                    break;
                }
            }
        } else if (msg.rfind("message:", 0) == 0) {
            std::cout << msg << std::endl;
            std::string message = msg.replace(0, 8, "");
            std::string senderName;
            for (auto &client : clients) {
                if (client.addr.sin_addr.s_addr == fromAddr.sin_addr.s_addr && client.addr.sin_port == fromAddr.sin_port) senderName = client.name;
            }
            if (senderName.empty()) {
                std::cout << "Message sent by this user is unknown!" << std::endl;
            } else {
                for (auto &client : clients) {
                    std::string sended = senderName + ":" + message;
                    sendto(sock, sended.c_str(), sended.length(), 0, (sockaddr*)&client.addr, client.len);
                    std::cout << "Sent " << sended << std::endl;
                }
            }
        } else {
            std::cout << "unknown?: " << msg <<std::endl;
        }
        memset(buffer, 0, sizeof(buffer));
        msg = "";
    }
    closesocket(sock);
    WSACleanup();
    return 0;
}