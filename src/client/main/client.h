#pragma once
#include <vector>
#include <string>

extern std::vector<std::string> messages;
extern std::string userBuffer;
extern bool end;

bool sendMessageToSer(std::string msg);