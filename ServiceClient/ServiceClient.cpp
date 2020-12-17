#include "framework.h"
#include "ServiceClient.h"
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <CommCtrl.h>
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib,"ComCtl32.Lib")
#pragma warning(disable:4996)

#define ID_STATUS 999
#define MAX_LOADSTRING 100

#define SERVER_PORT 2145
#define UDP_PORT 3874

#define MATRIX_0_0 100
#define MATRIX_0_1 101
#define MATRIX_0_2 102

#define MATRIX_1_0 110
#define MATRIX_1_1 111
#define MATRIX_1_2 112

#define MATRIX_2_0 120
#define MATRIX_2_1 121
#define MATRIX_2_2 122

#define WAITING    201


bool waiting = true;

std::vector<std::vector<int>> matrix;

WCHAR symbol;
WCHAR symbolOtherPerson;

SOCKET clientSocket;
sockaddr_in clientSA;
WSADATA wsaData;

HWND hWnd;
HWND hWndSb;

HWND hwnd00;
HWND hwnd01;
HWND hwnd02;

HWND hwnd10;
HWND hwnd11;
HWND hwnd12;

HWND hwnd20;
HWND hwnd21;
HWND hwnd22;

HWND hwndWaitingWnd;

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];


ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

DWORD               WINAPI Handler(LPVOID param);
void                EnableOrDisableButton(bool enableOrDisable, HWND hButton, int i, int j);
void                EnableOrDisableAllButtons(bool enableOrDisable);
BOOL                CreateField();
void                UpdateField(int i, int j);
void                UpdateClientWnd(int i, int j);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SERVICECLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SERVICECLIENT));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SERVICECLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SERVICECLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

void UpdateField(int i,
                 int j) {
    if (symbol == 'X') {
        matrix[i][j] = 1;
    }
    else {
        matrix[i][j] = 2;
    }
}

void UpdateClientWnd(int i,
                     int j) {
    UpdateField(i, j);
    EnableOrDisableAllButtons(0);
    SendMessage(hWndSb, SB_SETTEXT, 0, (LPARAM)L"Wait for another player's move.");
}

void EnableOrDisableButton(bool enableOrDisable,
                           HWND hButton,
                           int i,
                           int j) {
    if (enableOrDisable) {
        if (matrix.at(i).at(j) == 0) {
            EnableWindow(hButton, enableOrDisable);
        }
    }
    else {
        EnableWindow(hButton, enableOrDisable);
    }
}

void EnableOrDisableAllButtons(bool enableOrDisable) {
    EnableOrDisableButton(enableOrDisable, hwnd00, 0, 0);
    EnableOrDisableButton(enableOrDisable, hwnd01, 0, 1);
    EnableOrDisableButton(enableOrDisable, hwnd02, 0, 2);
    EnableOrDisableButton(enableOrDisable, hwnd10, 1, 0);
    EnableOrDisableButton(enableOrDisable, hwnd11, 1, 1);
    EnableOrDisableButton(enableOrDisable, hwnd12, 1, 2);
    EnableOrDisableButton(enableOrDisable, hwnd20, 2, 0);
    EnableOrDisableButton(enableOrDisable, hwnd21, 2, 1);
    EnableOrDisableButton(enableOrDisable, hwnd22, 2, 2);
}

BOOL CreateField() {
    hwnd00 = CreateWindow(L"BUTTON", L"",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        0, 0, 100, 100, hWnd, (HMENU)MATRIX_0_0,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

    hwnd01 = CreateWindow(L"BUTTON", L"",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        100, 0, 100, 100, hWnd, (HMENU)MATRIX_0_1,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

    hwnd02 = CreateWindow(L"BUTTON", L"",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        200, 0, 100, 100, hWnd, (HMENU)MATRIX_0_2,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);


    hwnd10 = CreateWindow(L"BUTTON", L"",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        0, 100, 100, 100, hWnd, (HMENU)MATRIX_1_0,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

    hwnd11 = CreateWindow(L"BUTTON", L"",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        100, 100, 100, 100, hWnd, (HMENU)MATRIX_1_1,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

    hwnd12 = CreateWindow(L"BUTTON", L"",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        200, 100, 100, 100, hWnd, (HMENU)MATRIX_1_2,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);


    hwnd20 = CreateWindow(L"BUTTON", L"",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        0, 200, 100, 100, hWnd, (HMENU)MATRIX_2_0,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

    hwnd21 = CreateWindow(L"BUTTON", L"",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        100, 200, 100, 100, hWnd, (HMENU)MATRIX_2_1,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

    hwnd22 = CreateWindow(L"BUTTON", L"",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        200, 200, 100, 100, hWnd, (HMENU)MATRIX_2_2,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

    hWndSb = CreateStatusWindow(WS_CHILD | WS_VISIBLE, (PCTSTR)NULL, hWnd, ID_STATUS);

    UpdateWindow(hWnd);

    return TRUE;
}

DWORD WINAPI Handler(LPVOID param) {
    while (true) {
        char buffer[2];

        int recvBytes = recv(clientSocket,
            &buffer[0], sizeof(buffer), 0);

        if (buffer[0] == '0'
            || buffer[0] == '1'
            || buffer[0] == '2'
            ) {
            char i = buffer[0];
            char j = buffer[1];

            UpdateField(atoi(&i), atoi(&j));

            if (buffer[0] == '0' && buffer[1] == '0') {
                SetWindowText((HWND)hwnd00, &symbol);
            }
            else if (buffer[0] == '0' && buffer[1] == '1') {
                SetWindowText((HWND)hwnd01, &symbol);
            }
            else if (buffer[0] == '0' && buffer[1] == '2') {
                SetWindowText((HWND)hwnd02, &symbol);
            }
            else if (buffer[0] == '1' && buffer[1] == '0') {
                SetWindowText((HWND)hwnd10, &symbol);
            }
            else if (buffer[0] == '1' && buffer[1] == '1') {
                SetWindowText((HWND)hwnd11, &symbol);
            }
            else if (buffer[0] == '1' && buffer[1] == '2') {
                SetWindowText((HWND)hwnd12, &symbol);
            }
            else if (buffer[0] == '2' && buffer[1] == '0') {
                SetWindowText((HWND)hwnd20, &symbol);
            }
            else if (buffer[0] == '2' && buffer[1] == '1') {
                SetWindowText((HWND)hwnd21, &symbol);
            }
            else if (buffer[0] == '2' && buffer[1] == '2') {
                SetWindowText((HWND)hwnd22, &symbol);
            }
            EnableOrDisableAllButtons(1);
            SendMessage(hWndSb, SB_SETTEXT, 0, (LPARAM)L"Your turn. Make a move.");
        }
        else if (buffer[0] == 'w' && buffer[1] == 'o') {
            MessageBox(NULL, TEXT("Congradulations! You've won this match!"), TEXT("You win!"),
                MB_OK | MB_ICONASTERISK);
            exit(0);
        }
        else if (buffer[0] == 'l' && buffer[1] == 'o') {
            MessageBox(NULL, TEXT("Sadly, you've lost this match. Try again."), TEXT("You lose!"),
                MB_OK | MB_ICONASTERISK);
            exit(0);
        }
        else if (buffer[0] == 'n' && buffer[1] == 'o') {
            MessageBox(NULL, TEXT("Draw. Try again."), TEXT("End of game."),
                MB_OK | MB_ICONASTERISK);
            exit(0);
        }
    }

    return 0;
}

BOOL InitInstance(HINSTANCE hInstance, 
                  int nCmdShow) {
    for (int i = 0; i < 3; i++) {
        std::vector<int> temp;
        for (int j = 0; j < 3; j++) {
            temp.push_back(0);
        }
        matrix.push_back(temp);
    }

    hInst = hInstance;

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED,
        CW_USEDEFAULT, 0, 315, 377, NULL, NULL, hInst, NULL);

    if (!hWnd) {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        TCHAR errCode[10];
        wsprintf(errCode, L"Error code - %d", WSAGetLastError());
        MessageBox(NULL, errCode, TEXT("Error"),
            MB_OK | MB_ICONASTERISK);
        exit(-1);
    };
    if ((clientSocket
        = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET
        ) {
        TCHAR errCode[10];
        wsprintf(errCode, L"Error code - %d", WSAGetLastError());
        MessageBox(NULL, errCode, TEXT("Error"),
            MB_OK | MB_ICONASTERISK);
        WSACleanup();
        exit(-1);
    }

    ZeroMemory(&clientSA, sizeof(sockaddr_in));
    clientSA.sin_family = AF_INET;
    clientSA.sin_port = htons(SERVER_PORT);

    char allowBroadcast = '1';
    sockaddr_in udpReciever;
    sockaddr_in udpSender;
    SOCKET udpSocket;
    if ((udpSocket
        = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        TCHAR errCode[10];
        wsprintf(errCode, L"Error code = %d",
            WSAGetLastError());
        MessageBox(NULL, errCode, TEXT("Error"),
            MB_OK | MB_ICONASTERISK);
        WSACleanup();
        exit(-1);
    }
    if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST,
        &allowBroadcast, sizeof(allowBroadcast)) == INVALID_SOCKET) {
        TCHAR errCode[10];
        wsprintf(errCode, L"Error code = %d",
            WSAGetLastError());
        MessageBox(NULL, errCode, TEXT("Error"),
            MB_OK | MB_ICONASTERISK);
        WSACleanup();
        exit(-1);
    }

    udpReciever.sin_family = AF_INET;
    udpReciever.sin_addr.s_addr = INADDR_ANY;
    udpReciever.sin_port = htons(UDP_PORT);

    while (bind(udpSocket,
        (sockaddr*)&udpReciever,
        sizeof(udpReciever)) == SOCKET_ERROR) {
    }

    char buffer[10] = "";
    int len = sizeof(sockaddr_in);
    recvfrom(udpSocket, buffer, sizeof(buffer), 0,
        (sockaddr*)&udpSender, &len);

    clientSA.sin_addr.s_addr = inet_addr(buffer);

    closesocket(udpSocket);


    if (connect(clientSocket,
        (sockaddr*)&clientSA, sizeof(clientSA)) == SOCKET_ERROR) {
        TCHAR errCode[10];
        wsprintf(errCode, L"Error code - %d", WSAGetLastError());
        MessageBox(NULL, errCode, TEXT("Error"),
            MB_OK | MB_ICONASTERISK);
        closesocket(clientSocket);
        WSACleanup();
        exit(-1);
    };

    hwndWaitingWnd = CreateWindow(L"STATIC", L"Waiting for another player...",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        100, 100, 100, 100, hWnd, (HMENU)WAITING,
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

    UpdateWindow(hWnd);

    char symbolT = ' ';
    int recvBytes = recv(clientSocket,
        &symbolT, sizeof(symbolT), 0);
    MultiByteToWideChar(CP_ACP, NULL, &symbolT, 1, &symbol, 1);

    while (waiting) {
        char buffer[2];
        recvBytes = recv(clientSocket,
            &buffer[0], sizeof(buffer), 0);
        if (buffer[0] == 'o' && buffer[1] == 'k') {
            waiting = false;
            CreateField();
            CloseWindow(hwndWaitingWnd);
        }
    }

    if (symbolT == 'X') {
        symbolOtherPerson = L'O';
    }
    else {
        symbolOtherPerson = L'X';
        EnableOrDisableAllButtons(0);
    }

    DWORD TID;
    CreateThread(NULL, NULL, Handler,
        NULL, NULL, &TID);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd,
                         UINT message, 
                         WPARAM wParam, 
                         LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        case MATRIX_0_0:
            send(clientSocket, "00", 2, 0);
            SetWindowText((HWND)hwnd00, &symbolOtherPerson);
            UpdateClientWnd(0, 0);
            break;

        case MATRIX_0_1:
            send(clientSocket, "01", 2, 0);
            SetWindowText((HWND)hwnd01, &symbolOtherPerson);
            UpdateClientWnd(0, 1);
            break;

        case MATRIX_0_2:
            send(clientSocket, "02", 2, 0);
            SetWindowText((HWND)hwnd02, &symbolOtherPerson);
            UpdateClientWnd(0, 2);
            break;

        case MATRIX_1_0:
            send(clientSocket, "10", 2, 0);
            SetWindowText((HWND)hwnd10, &symbolOtherPerson);
            UpdateClientWnd(1, 0);
            break;

        case MATRIX_1_1:
            send(clientSocket, "11", 2, 0);
            SetWindowText((HWND)hwnd11, &symbolOtherPerson);
            UpdateClientWnd(1, 1);
            break;

        case MATRIX_1_2:
            send(clientSocket, "12", 2, 0);
            SetWindowText((HWND)hwnd12, &symbolOtherPerson);
            UpdateClientWnd(1, 2);
            break;

        case MATRIX_2_0:
            send(clientSocket, "20", 2, 0);
            SetWindowText((HWND)hwnd20, &symbolOtherPerson);
            UpdateClientWnd(2, 0);
            break;

        case MATRIX_2_1:
            send(clientSocket, "21", 2, 0);
            SetWindowText((HWND)hwnd21, &symbolOtherPerson);
            UpdateClientWnd(2, 1);
            break;

        case MATRIX_2_2:
            send(clientSocket, "22", 2, 0);
            SetWindowText((HWND)hwnd22, &symbolOtherPerson);
            UpdateClientWnd(2, 2);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, 
                       UINT message, 
                       WPARAM wParam,
                       LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
