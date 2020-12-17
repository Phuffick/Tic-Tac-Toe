#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <tchar.h>
#include <string>
#pragma comment(lib, "WS2_32.lib")
#pragma warning(disable:4996)

#define SERVER_PORT 2145
#define UDP_PORT 3874
//#define SERVER_ADDR "10.1.0.43"
#define SERVER_ADDR "127.0.0.1"
#define serviceName TEXT("XOGameService")

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle;


struct GamePart {
	const std::vector<std::vector<int>>& matrix;
	const SOCKET& firstClientSocketInputOutput = INVALID_SOCKET;
	const SOCKET& secondClientSocketInputOutput = INVALID_SOCKET;
};

DWORD WINAPI CheckIfAnyoneWon(LPVOID param) {
	GamePart gamePart = *(GamePart*)param;

	const std::vector<std::vector<int>>& matrix = gamePart.matrix;
	const SOCKET& firstClientSocketInputOutput = gamePart.firstClientSocketInputOutput;
	const SOCKET& secondClientSocketInputOutput = gamePart.secondClientSocketInputOutput;

	while (true) {
		if (
			(matrix[0][0] == 1 && matrix[0][1] == 1 && matrix[0][2] == 1)
			|| (matrix[1][0] == 1 && matrix[1][1] == 1 && matrix[1][2] == 1)
			|| (matrix[2][0] == 1 && matrix[2][1] == 1 && matrix[2][2] == 1)
			) {
			send(firstClientSocketInputOutput, "wo", 2, 0);
			send(secondClientSocketInputOutput, "lo", 2, 0);
			return 0;
		}
		else if (
			(matrix[0][0] == 1 && matrix[1][0] == 1 && matrix[2][0] == 1)
			|| (matrix[0][1] == 1 && matrix[1][1] == 1 && matrix[2][1] == 1)
			|| (matrix[0][2] == 1 && matrix[1][2] == 1 && matrix[2][2] == 1)
			) {
			send(firstClientSocketInputOutput, "wo", 2, 0);
			send(secondClientSocketInputOutput, "lo", 2, 0);
			return 0;
		}
		else if (
			(matrix[0][0] == 1 && matrix[1][1] == 1 && matrix[2][2] == 1)
			|| (matrix[0][2] == 1 && matrix[1][1] == 1 && matrix[2][0] == 1)
			) {
			send(firstClientSocketInputOutput, "wo", 2, 0);
			send(secondClientSocketInputOutput, "lo", 2, 0);
			return 0;
		}
		else if (
			(matrix[0][0] == 2 && matrix[0][1] == 2 && matrix[0][2] == 2)
			|| (matrix[1][0] == 2 && matrix[1][1] == 2 && matrix[1][2] == 2)
			|| (matrix[2][0] == 2 && matrix[2][1] == 2 && matrix[2][2] == 2)
			) {
			send(firstClientSocketInputOutput, "lo", 2, 0);
			send(secondClientSocketInputOutput, "wo", 2, 0);
			return 0;
		}
		else if (
			(matrix[0][0] == 2 && matrix[1][0] == 2 && matrix[2][0] == 2)
			|| (matrix[0][1] == 2 && matrix[1][1] == 2 && matrix[2][1] == 2)
			|| (matrix[0][2] == 2 && matrix[1][2] == 2 && matrix[2][2] == 2)
			) {
			send(firstClientSocketInputOutput, "lo", 2, 0);
			send(secondClientSocketInputOutput, "wo", 2, 0);
			return 0;
		}
		else if (
			(matrix[0][0] == 2 && matrix[1][1] == 2 && matrix[2][2] == 2)
			|| (matrix[0][2] == 2 && matrix[1][1] == 2 && matrix[2][0] == 2)
			) {
			send(firstClientSocketInputOutput, "lo", 2, 0);
			send(secondClientSocketInputOutput, "wo", 2, 0);
			return 0;
		}
		else if (
			(matrix[0][0] != 0 && matrix[0][1] != 0 && matrix[0][2] != 0)
			&& (matrix[1][0] != 0 && matrix[1][1] != 0 && matrix[1][2] != 0)
			&& (matrix[2][0] != 0 && matrix[2][1] != 0 && matrix[2][2] != 0)
			) {
			send(firstClientSocketInputOutput, "no", 2, 0);
			send(secondClientSocketInputOutput, "no", 2, 0);
			return 0;
		}
	}

	return 0;
}

DWORD WINAPI FindClients(LPVOID param) {
	char allowBroadcast = '1';
	sockaddr_in udpReciever;
	SOCKET udpSocket = INVALID_SOCKET;
	if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		std::cerr << "Winsock UDP socket creation failed, errCode = "
			<< WSAGetLastError() << '\n';
		WSACleanup();
		exit(-1);
	}
	if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST,
		&allowBroadcast, sizeof(allowBroadcast)) == INVALID_SOCKET) {
		std::cerr << "Winsock broadcasting UDP socket failed, errCode = "
			<< WSAGetLastError() << '\n';
		closesocket(udpSocket);
		WSACleanup();
		exit(-1);
	}
	udpReciever.sin_family = AF_INET;
	//udpReciever.sin_addr.s_addr = inet_addr("10.1.0.255");
	udpReciever.sin_addr.s_addr = inet_addr("255.255.255.255");
	udpReciever.sin_port = htons(UDP_PORT);

	while (true) {
		sendto(udpSocket, SERVER_ADDR, strlen(SERVER_ADDR), 0,
			(sockaddr*)&udpReciever, sizeof(udpReciever));
	}

	return 0;
}

class Game {
public:
	Game(SOCKET serverSocketT) {
		serverSocket = serverSocketT;

		for (int i = 0; i < 3; i++) {
			std::vector<int> temp;
			for (int j = 0; j < 3; j++) {
				temp.push_back(0);
			}
			matrix.push_back(temp);
		}
	};

	int ñonnectPlayers() {
		int firstClientSocketInputOutputLen
			= sizeof(firstClientSA);

		int secondClientSocketInputOutputLen
			= sizeof(secondClientSA);

		std::cout << "Accepted 0 clients\n";

		while (firstClientSocketInputOutput == INVALID_SOCKET) {
			firstClientSocketInputOutput = accept(serverSocket,
				(sockaddr*)&firstClientSA, &firstClientSocketInputOutputLen);
		}
		std::cout << "Accepted 1 client\n";

		while (secondClientSocketInputOutput == INVALID_SOCKET) {
			secondClientSocketInputOutput = accept(serverSocket,
				(sockaddr*)&secondClientSA, &secondClientSocketInputOutputLen);
		}
		std::cout << "Accepted 2 clients\n";

		send(firstClientSocketInputOutput, "X", 1, 0);
		send(secondClientSocketInputOutput, "O", 1, 0);

		send(firstClientSocketInputOutput, "ok", 2, 0);
		send(secondClientSocketInputOutput, "ok", 2, 0);

		return 0;
	}

	int execute() {
		char buffer[2];

		GamePart gamePart = { matrix,
							firstClientSocketInputOutput,
							secondClientSocketInputOutput };

		DWORD TID;
		thread = CreateThread(NULL, NULL, CheckIfAnyoneWon,
			&gamePart, NULL, &TID);


		while (firstClientSocketInputOutput
			&& secondClientSocketInputOutput) {
			int recvBytes = recv(firstClientSocketInputOutput,
				&buffer[0], sizeof(buffer), 0);

			send(firstClientSocketInputOutput, "wt", 2, 0);
			send(secondClientSocketInputOutput, "yr", 2, 0);

			if (recvBytes == SOCKET_ERROR) {
				send(secondClientSocketInputOutput, "wo", 2, 0);
				TerminateThread(thread, 1);
				return 0;
			}
			std::cout << "Got message from client 1\n";

			char i = buffer[0];
			char j = buffer[1];

			matrix[atoi(&i)][atoi(&j)] = 1;


			send(secondClientSocketInputOutput,
				&buffer[0], recvBytes, 0);

			if (recvBytes == SOCKET_ERROR) {
				send(firstClientSocketInputOutput, "wo", 2, 0);
				TerminateThread(thread, 1);
				return 0;
			}
			std::cout << "Sended message to client 2\n";


			recvBytes = recv(secondClientSocketInputOutput,
				&buffer[0], sizeof(buffer), 0);

			send(firstClientSocketInputOutput, "yr", 2, 0);
			send(secondClientSocketInputOutput, "wt", 2, 0);

			if (recvBytes == SOCKET_ERROR) {
				send(firstClientSocketInputOutput, "wo", 2, 0);
				TerminateThread(thread, 1);
				return 0;
			}
			std::cout << "Got message from client 2\n";

			i = buffer[0];
			j = buffer[1];

			matrix[atoi(&i)][atoi(&j)] = 2;


			send(firstClientSocketInputOutput,
				&buffer[0], recvBytes, 0);

			if (recvBytes == SOCKET_ERROR) {
				send(secondClientSocketInputOutput, "wo", 2, 0);
				TerminateThread(thread, 1);
				return 0;
			}
			std::cout << "Sended message to client 1\n";
		}

		shutdown(firstClientSocketInputOutput, SD_BOTH);
		shutdown(secondClientSocketInputOutput, SD_BOTH);

		closesocket(firstClientSocketInputOutput);
		closesocket(secondClientSocketInputOutput);

		return 0;
	}

private:
	HANDLE thread;
	int gameId;
	std::vector<std::vector<int>> matrix;

	sockaddr_in firstClientSA;
	sockaddr_in secondClientSA;

	SOCKET serverSocket = INVALID_SOCKET;
	SOCKET firstClientSocketInputOutput = INVALID_SOCKET;
	SOCKET secondClientSocketInputOutput = INVALID_SOCKET;
};

DWORD WINAPI DoMatch(LPVOID param) {
	Game game = *(Game*)param;
	game.execute();

	return 0;
}

class Server {
public:
	int connect() {
		WSADATA wsaData;
		sockaddr_in serverSA;

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			std::cerr << "Winsock init failed errCode = "
				<< WSAGetLastError() << '\n';
			exit(-1);
		}

		if (
			(serverSocket
				= socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET
			) {
			std::cerr << "Winsock socket creation failed, errCode = "
				<< WSAGetLastError() << '\n';
			WSACleanup();
			exit(-1);
		}

		ZeroMemory(&serverSA, sizeof(sockaddr_in));
		serverSA.sin_family = AF_INET;
		serverSA.sin_addr.s_addr = inet_addr(SERVER_ADDR);
		serverSA.sin_port = htons(SERVER_PORT);

		if (bind(serverSocket,
			(sockaddr*)&serverSA,
			sizeof(serverSA)) != 0
			) {
			std::cerr << "Winsock binding failed, errCode = "
				<< WSAGetLastError() << '\n';
			closesocket(serverSocket);
			WSACleanup();
			exit(-1);
		}

		if (listen(serverSocket, 256) == SOCKET_ERROR) {
			std::cerr << "Winsock listening failed, errCode = "
				<< WSAGetLastError() << '\n';
			closesocket(serverSocket);
			WSACleanup();
			exit(-1);
		}

		DWORD TID;
		HANDLE tread = CreateThread(NULL, NULL, FindClients,
			&serverSA.sin_addr.s_addr, NULL, &TID);

		return 0;
	}

	int execute() {
		while (serviceStatus.dwCurrentState == SERVICE_RUNNING 
			|| serviceStatus.dwCurrentState == SERVICE_STOP_PENDING) {
			if (serviceStatus.dwCurrentState == SERVICE_STOP_PENDING) {
				Sleep(serviceStatus.dwWaitHint);

				serviceStatus.dwCurrentState = SERVICE_RUNNING;
				serviceStatus.dwWaitHint = 0;
			}

			Game game(serverSocket);
			game.ñonnectPlayers();

			DWORD TID;
			HANDLE tread = CreateThread(NULL, NULL, DoMatch,
				&game, NULL, &TID);

			Sleep(1);
		}

		return 0;
	}

private:
	SOCKET serverSocket = INVALID_SOCKET;
};


int logMessage(const char* text) {
	return printf(text);
}

void ServiceCtrlHandlerEx(DWORD dwControl,
						  DWORD dwEventType,
						  LPVOID lpEventData,
						  LPVOID lpContext) {
	if (dwControl == SERVICE_CONTROL_STOP) {
		logMessage("Stopped.\n");

		serviceStatus.dwCurrentState = SERVICE_STOPPED;

		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		return;
	}
	else if (dwControl == SERVICE_CONTROL_SHUTDOWN) {
		logMessage("Shutdowned.\n");

		serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		serviceStatus.dwWaitHint = 100000;

		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		return;
	}
	SetServiceStatus(serviceStatusHandle, &serviceStatus);

	return;
}

int InstallXOService() {
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL) {
		logMessage("Opening SCM failed.\n");
		return -1;
	}

	TCHAR path[MAX_PATH];
	if (!GetModuleFileName(NULL, path, MAX_PATH)) {
		logMessage("Unstalling service failed.\n");
		return -1;
	}

	SC_HANDLE hService = CreateService(
		hSCM,
		serviceName,
		serviceName,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		path,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	);

	if (!hService) {
		int error = GetLastError();

		if (error == ERROR_ACCESS_DENIED) {
			logMessage("ERROR_ACCESS_DENIED.\n");
		}
		else if (error == ERROR_CIRCULAR_DEPENDENCY) {
			logMessage("ERROR_CIRCULAR_DEPENDENCY.\n");
		}
		else if (error == ERROR_DUPLICATE_SERVICE_NAME) {
			logMessage("ERROR_DUPLICATE_SERVICE_NAME.\n");
		}
		else if (error == ERROR_INVALID_HANDLE) {
			logMessage("ERROR_INVALID_HANDLE.\n");
		}
		else if (error == ERROR_INVALID_NAME) {
			logMessage("ERROR_INVALID_NAME.\n");
		}
		else if (error == ERROR_INVALID_PARAMETER) {
			logMessage("ERROR_INVALID_PARAMETER.\n");
		}
		else if (error == ERROR_INVALID_SERVICE_ACCOUNT) {
			logMessage("ERROR_INVALID_SERVICE_ACCOUNT.\n");
		}
		else if (error == ERROR_SERVICE_EXISTS) {
			logMessage("ERROR_SERVICE_EXISTS.\n");
		}
		else {
			logMessage("Undefined.\n");
		}

		CloseServiceHandle(hSCM);
		return -1;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);
	logMessage("Installed successfully.\n");

	return 0;
}

int RemoveXOService() {
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCM) {
		logMessage("Opening SCM failed.\n");
		return -1;
	}

	SC_HANDLE hService = OpenService(hSCM, serviceName, SERVICE_STOP | DELETE);
	if (!hService) {
		logMessage("Removing service failed.\n");
		CloseServiceHandle(hSCM);
		return -1;
	}

	DeleteService(hService);
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);

	logMessage("Service removed successfully.\n");

	return 0;
}

int StartXOService(DWORD argc, 
				   TCHAR* argv[]) {
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_ALL_ACCESS);

	if (!StartService(hService, argc, (LPCTSTR*)argv)) {
		int error = GetLastError();
		if (error == ERROR_INVALID_HANDLE) {
			logMessage("ERROR_INVALID_HANDLE.\n");
		}
		if (error == ERROR_ACCESS_DENIED) {
			logMessage("ERROR_ACCESS_DENIED.\n");
		}
		if (error == ERROR_PATH_NOT_FOUND) {
			logMessage("ERROR_PATH_NOT_FOUND.\n");
		}
		if (error == ERROR_SERVICE_ALREADY_RUNNING) {
			logMessage("ERROR_SERVICE_ALREADY_RUNNING.\n");
		}
		if (error == ERROR_SERVICE_DATABASE_LOCKED) {
			logMessage("ERROR_SERVICE_DATABASE_LOCKED.\n");
		}
		if (error == ERROR_SERVICE_DEPENDENCY_DELETED) {
			logMessage("ERROR_SERVICE_DEPENDENCY_DELETED.\n");
		}
		if (error == ERROR_SERVICE_DEPENDENCY_FAIL) {
			logMessage("ERROR_SERVICE_DEPENDENCY_FAIL.\n");
		}
		if (error == ERROR_SERVICE_DISABLED) {
			logMessage("ERROR_SERVICE_DISABLED.\n");
		}
		if (error == ERROR_SERVICE_LOGON_FAILED) {
			logMessage("ERROR_SERVICE_LOGON_FAILED.\n");
		}
		if (error == ERROR_SERVICE_MARKED_FOR_DELETE) {
			logMessage("ERROR_SERVICE_MARKED_FOR_DELETE.\n");
		}
		if (error == ERROR_SERVICE_NO_THREAD) {
			logMessage("ERROR_SERVICE_NO_THREAD.\n");
		}
		if (error == ERROR_SERVICE_REQUEST_TIMEOUT) {
			logMessage("ERROR_SERVICE_REQUEST_TIMEOUT.\n");
		}
		if (error == ERROR_FILE_NOT_FOUND) {
			logMessage("ERROR_FILE_NOT_FOUND.\n");
		}
		CloseServiceHandle(hSCManager);
		logMessage("Starting service failed.");

		return -1;
	}
	logMessage("Service started successfully.");

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);

	return 0;
}

void ServiceMain(DWORD argc, 
				 TCHAR* argv[]) {
	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	serviceStatus.dwWin32ExitCode = NO_ERROR;
	serviceStatus.dwServiceSpecificExitCode = 0;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;

	serviceStatusHandle = RegisterServiceCtrlHandlerEx(serviceName, 
		(LPHANDLER_FUNCTION_EX)ServiceCtrlHandlerEx, NULL);
	if (serviceStatusHandle == (SERVICE_STATUS_HANDLE)0) {
		return;
	}

	SetServiceStatus(serviceStatusHandle, &serviceStatus);


	serviceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(serviceStatusHandle, &serviceStatus);

	Server server;
	server.connect();
	server.execute();

	serviceStatus.dwCurrentState = SERVICE_STOPPED;
	serviceStatus.dwWin32ExitCode = 0;
	SetServiceStatus(serviceStatusHandle, &serviceStatus);

	return;
}


int main(DWORD argc, 
		 TCHAR* argv[]) {
	if ( (argc - 1) == 0) {

		SERVICE_TABLE_ENTRY serviceTable[] = {
			{
				(LPTSTR)serviceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain
			},
			{
				NULL, NULL
			}
		};

		if (!StartServiceCtrlDispatcher(serviceTable)) {
			logMessage("StartServiceCtrlDispatcher failed.\n");
		}
	}
	else if (_tcscmp(argv[argc - 1], TEXT("install")) == 0) {
		InstallXOService();
	}
	else if (_tcscmp(argv[argc - 1], TEXT("remove")) == 0) {
		RemoveXOService();
	}
	else if (_tcscmp(argv[argc - 1], TEXT("start")) == 0) {
		StartXOService(argc, argv);
	}

	return 0;
}
