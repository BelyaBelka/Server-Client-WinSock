#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#define _CRT_SECURE_NO_WARNINGS
class Client
{
private:
	char ServerIp[16];
	unsigned short Serverport;
public:
	Client(char ServerIP[16], unsigned short ServerPort) {
		strcpy_s(ServerIp, ServerIP);
		Serverport = ServerPort;
		if (setlocale(LC_ALL, "Rus") == NULL)
		{
			std::cout << "Error";
		}
	}
	// подпрограмма вывода на экран данных: IP-адрес\Port
	void show(sockaddr_in SockAddrInfo, const char* name)
	{
		std::cout << name << " IP= "
			<< int(SockAddrInfo.sin_addr.S_un.S_un_b.s_b1) << "."
			<< int(SockAddrInfo.sin_addr.S_un.S_un_b.s_b2) << "."
			<< int(SockAddrInfo.sin_addr.S_un.S_un_b.s_b3) << "."
			<< int(SockAddrInfo.sin_addr.S_un.S_un_b.s_b4)
			<< " Port= " << htons(SockAddrInfo.sin_port)
			<< " StackFamily= " << SockAddrInfo.sin_family
			<< " sizeof= " << sizeof(SockAddrInfo) << "\n";
	}
	// подключение DLL библиотеки WSA
	void DLL_Connection(char* ServerIP, unsigned short ServerPort)
	{
		int erStat = 0; // код ошибки (статуса)
		WSAData WSADataStruct; //структура для инициализации WinSock
		WORD DLLVersion = MAKEWORD(2, 2);//аргументы - минимальная и максимальная версия реализации сокетов
		erStat = WSAStartup(DLLVersion, &WSADataStruct);//WSAStartup возвращает 0 в случае успеха.
		if (erStat)//если 1
		{
			std::cout << "ошибка подключения DLL: " << WSAGetLastError() << "\n";
			system("pause");
			exit(1);
		}
		else
			std::cout << "WSA DLL успешно подключена\n";
	}
	SOCKET Initialization_Socket()
	{// Инициализация сокета
		SOCKET ClientSock = socket(AF_INET, SOCK_STREAM, 0);//Семейство IPv4,тип сокета(TCP).
		if (ClientSock == INVALID_SOCKET) {
			std::cout << "ошибка инициализации сокета: " << WSAGetLastError() << "\n";
			closesocket(ClientSock);//закрытие сокета
			WSACleanup();		  //закрытие библиотек WSA
			system("pause");
			exit(1);
		}
		else
			std::cout << "сокет инициализирован успешно\n";
		return ClientSock;
	}
	void Transf_Binding_Listening(int erStat, char* ServerIP, unsigned short ServerPort, SOCKET ClientSock) {

		// преобразование строки с IP-адресом в числовой формат in_addr
		in_addr IPtoNum;
		erStat = inet_pton(AF_INET, ServerIP, &IPtoNum);//перевод текстовойф константной строки,содржащий IP, в числовой формат
		if (erStat <= 0)
		{
			std::cout << "ошибка перевода IP в специальный числовой формат\n";
			system("pause");
			exit(1);
		}
		else
			std::cout << "IP-адрес переведен в специальный числовой формат \n";
		// Привязка сокета к паре IP-адрес/Порт
		sockaddr_in ServSockAddrInfo;
		ZeroMemory(&ServSockAddrInfo, sizeof(ServSockAddrInfo)); // очистка
		ServSockAddrInfo.sin_addr = IPtoNum; // IP-адрес в нужном формате
		ServSockAddrInfo.sin_family = AF_INET; // семейство (стек) протоколов TCP/IP
		ServSockAddrInfo.sin_port = htons(ServerPort); // порт
		show(ServSockAddrInfo, "ServSock");
		// установление соединения с сервером
		erStat = connect(ClientSock, (sockaddr*)&ServSockAddrInfo,
			sizeof(ServSockAddrInfo));
		if (erStat != 0) {
			std::cout << "установить соединение с сервером не удалось: Ошибка "
				<< WSAGetLastError() << "\n";
			closesocket(ClientSock);
			WSACleanup();
			system("pause");
			exit(1);
		}
		else
			std::cout << "соединение с сервером установлено\n";
	}
	void shutdown(SOCKET ClientSock)
	{
		closesocket(ClientSock); // завершение работы
		WSACleanup();
		std::cout << "Завершение работы" << std::endl;
		system("pause");
	}
	void sendRequest(SOCKET ClientSock) {
		char NickName[6];
		char request[1024];
		std::cout << "Введите свой ник (до 5 символов): ";
		std::cin >> NickName;
		while (strlen(NickName) > 5)
		{
			std::cout << "Никнейм слишком длинный,введите никнейм до 5 символов" << std::endl;
			ZeroMemory(NickName, sizeof(NickName));
			std::cout << "Введите свой ник (до 5 символов): ";
			std::cin >> NickName;
		}
		while (true) {
			std::cin.ignore(); // очищаем входной буфер
			std::cout << "Введите свой вопрос: ";
			std::cin.getline(request, sizeof(request));

			int packet_size = send(ClientSock, request, strlen(request), 0);
			if (packet_size == SOCKET_ERROR) {
				std::cout << "Ошибка отправки запроса на сервер: # " << WSAGetLastError() << "\n";
				closesocket(ClientSock);
				WSACleanup();
				system("pause");
				exit(1);
			}

			char recvBuff[1024];
			int recv_size = recv(ClientSock, recvBuff, sizeof(recvBuff), 0);
			if (recv_size == SOCKET_ERROR) {
				std::cout << "Сообщение об ошибке при получении сообщения от сервера: # " << WSAGetLastError() << "\n";
				closesocket(ClientSock);
				WSACleanup();
				system("pause");
				exit(1);
			}
			recvBuff[recv_size] = '\0'; // Ensure null termination
			std::cout << "Ответ сервера: " << recvBuff << std::endl;
			if (strcmp(recvBuff, "Shut_Up") == 0) {
				std::cout << "Клиент запросил завершение соединения\n";
				closesocket(ClientSock);
				WSACleanup();
				break;
			}
		}
	}
 };
//-----------------------------------------------------------------------------
int main()
{
	setlocale(LC_ALL, "Rus");
	char serverIP[16];
	unsigned short serverPort = 0;
	ZeroMemory(&serverIP, sizeof(serverIP));
	std::cout << "Введите ServerIP= ";
	std::cin >> serverIP;
	std::cout << "Введите ServerPort= ";
	std::cin >> serverPort;
	Client client(serverIP, serverPort);
	client.DLL_Connection(serverIP, serverPort);
	SOCKET ClientSocket = client.Initialization_Socket();
	client.Transf_Binding_Listening(0, serverIP, serverPort, ClientSocket);
	client.sendRequest(ClientSocket);
	client.shutdown(ClientSocket);
}
