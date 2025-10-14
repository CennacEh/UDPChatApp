#include <vector>
#include <string>
#include <iostream>

#include "client.h"
#include "raylib.h"

void initGui() {
    InitWindow(800, 600, "ChatApp");
    SetTargetFPS(30);
}

void gui() {
    if (WindowShouldClose()) end = true;
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 126) userBuffer.push_back((char)key);
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !userBuffer.empty()) userBuffer.pop_back();
    else if (IsKeyDown(KEY_ENTER)) {
        if (!userBuffer.empty()) {
            sendMessageToSer(userBuffer);
            userBuffer.clear();
        } else DrawText("> |", 10, 10 + 20 * messages.size(), 20, RED);
    }
    BeginDrawing(); {
        ClearBackground(RAYWHITE);
        if (!messages.empty()) for (size_t i = 0; i < messages.size(); i++) DrawText(messages[i].c_str(), 10, 10 + 20 * i, 20, BLACK);
        if (!userBuffer.empty()) {
            std::string toDraw = "> " + userBuffer + "|";
            DrawText(toDraw.c_str(), 10, 10 + 20 * messages.size(), 20, BLACK);
        }
        else DrawText("> |", 10, 10 + 20 * messages.size(), 20, BLACK);
    } EndDrawing();
}

void deinitGui() {
    CloseWindow();
}
