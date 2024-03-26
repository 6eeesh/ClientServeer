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

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        cout << "Не удалось создать сокет." << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Не удалось присвоить сокету локальный адрес." << endl;
        closesocket(server);
        WSACleanup();
        return 1;
    }

    if (listen(server, 1) == SOCKET_ERROR) {
        cout << "Ошибка при попытке перевести сокет в режим ожидания соединений." << endl;
        closesocket(server);
        WSACleanup();
        return 1;
    }

    SOCKET client;
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    cout << "Ожидание подключения..." << endl;
    client = accept(server, (sockaddr*)&clientAddr, &clientAddrSize);

    if (client == INVALID_SOCKET) {
        cout << "Ошибка при попытке принять соединение." << endl;
        closesocket(server);
        WSACleanup();
        return 1;
    }

    cout << "Клиент подключен." << endl;

    int x = 5, y = 5;
    drawSmiley(x, y);

    char direction;
    while (true) {
        int move;
        recv(client, (char*)&move, sizeof(int), 0);

        clearSmiley(x, y);

        switch (move) {
        case 1:
            x--;
            break;
        case 2:
            x++;
            break;
        case 3:
            y--;
            break;
        case 4:
            y++;
            break;
        }

        x = max(x, 1);
        x = min(x, 80 - SMILEY_WIDTH);
        y = max(y, 1);
        y = min(y, 25 - SMILEY_HEIGHT);

        send(client, (const char*)&x, sizeof(int), 0);
        send(client, (const char*)&y, sizeof(int), 0);
        drawSmiley(x, y);

        direction = getch();

        if (direction == 27)
            break;

        clearSmiley(x, y);

        switch (direction) {
        case 75:
            x--;
            break;
        case 77:
            x++;
            break;
        case 72:
            y--;
            break;
        case 80:
            y++;
            break;
        }

        x = max(x, 1);
        x = min(x, 80 - SMILEY_WIDTH);
        y = max(y, 1);
        y = min(y, 25 - SMILEY_HEIGHT);

        send(client, (const char*)&x, sizeof(int), 0);
        send(client, (const char*)&y, sizeof(int), 0);
        drawSmiley(x, y);
    }

    closesocket(client);
    closesocket(server);
    WSACleanup();
    return 0;
}