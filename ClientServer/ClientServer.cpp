#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
static struct termios old, new_term;

void initTermios(int echo) {
    tcgetattr(0, &old);
    new_term = old;
    new_term.c_lflag &= ~ICANON;
    new_term.c_lflag &= echo ? ECHO : ~ECHO;
    tcsetattr(0, TCSANOW, &new_term);
}

void resetTermios() {
    tcsetattr(0, TCSANOW, &old);
}

char getch_(int echo) {
    char ch;
    initTermios(echo);
    ch = getchar();
    resetTermios();
    return ch;
}

char getch() {
    return getch_(0);
}
#endif

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888

using namespace std;

const int SMILEY_WIDTH = 5;
const int SMILEY_HEIGHT = 3;
const char SMILEY[SMILEY_HEIGHT][SMILEY_WIDTH + 1] = {
    " :') ",
    "@>-->",
    "     "
};

void drawSmiley(int x, int y) {
    for (int i = 0; i < SMILEY_HEIGHT; i++) {
        cout << "\033[" << y + i << ";" << x << "H" << SMILEY[i] << flush;
    }
}

void clearSmiley(int x, int y) {
    for (int i = 0; i < SMILEY_HEIGHT; i++) {
        for (int j = 0; j < SMILEY_WIDTH; j++) {
            cout << "\033[" << y + i << ";" << x + j << "H " << flush;
        }
    }
}

int main() {
    WSAData wsaData;
    WORD DllVersion = MAKEWORD(2, 1);
    if (WSAStartup(DllVersion, &wsaData) != 0) {
        cout << "Ошибка инициализации WinSock." << endl;
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cout << "Не удалось создать сокет." << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sock, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        cout << "Не удалось подключиться к серверу." << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    int x = 5, y = 5;
    drawSmiley(x, y);

    char direction;
    while (true) {
        direction = getch();

        if (direction == 27)
            break;

        int move = 0;
        switch (direction) {
        case 75:
            move = 1;
            break;
        case 77:
            move = 2;
            break;
        case 72:
            move = 3;
            break;
        case 80:
            move = 4;
            break;
        }

        if (move > 0) {
            send(sock, (const char*)&move, sizeof(int), 0);
            recv(sock, (char*)&x, sizeof(int), 0);
            recv(sock, (char*)&y, sizeof(int), 0);
            clearSmiley(x, y);
            drawSmiley(x, y);
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}