//-------------------------------------------------------------------------
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <vector>
#define _CRT_SECURE_NO_WARNINGS
class Server
{
private:
	char ServerIp[16];
	unsigned short Serverport;
public:
	Server(char ServerIP[16], unsigned short ServerPort) {
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
		SOCKET ServSock = socket(AF_INET, SOCK_STREAM, 0);//Семейство IPv4,тип сокета(TCP).
		if (ServSock == INVALID_SOCKET) {
			std::cout << "ошибка инициализации сокета: " << WSAGetLastError() << "\n";
			closesocket(ServSock);//закрытие сокета
			WSACleanup();		  //закрытие библиотек WSA
			system("pause");
			exit(1);
		}
		else
			std::cout << "сокет инициализирован успешно\n";
		return ServSock;
	}
	sockaddr_in Transf_Binding_Listening(int erStat, char* ServerIP, unsigned short ServerPort, SOCKET ServSock) {

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
		//
		erStat = bind(ServSock, (sockaddr*)&ServSockAddrInfo,
			sizeof(ServSockAddrInfo));
		if (erStat != 0) {
			std::cout << "ошибка привязки сокета к адресу/порту "
				<< WSAGetLastError() << "\n";
			closesocket(ServSock);
			WSACleanup();
			system("pause");
			exit(1);
		}
		else
			std::cout << "привязка сокета осуществлена успешно\n";
		// прослушивание сети для установления соединения (поиск клиентов)
		erStat = listen(ServSock, SOMAXCONN);//слушающий сокет,макс. количество процессов подключенных.
		if (erStat != 0) {
			std::cout << "ошибка старта процесса прослушивания сети: "
				<< WSAGetLastError() << "\n";
			closesocket(ServSock);
			WSACleanup();
			system("pause");
			exit(1);
		}
		else
			std::cout << "прослушивание сети...\n";
		return ServSockAddrInfo;
	}
// установление, подтверждение соединения с клиентом (канала связи)
	SOCKET Confirmation(SOCKET ServSock, sockaddr_in ServSockAddrInfo) {
		sockaddr_in ClientSockAddrInfo;//настройка парметров сокета
		ZeroMemory(&ClientSockAddrInfo, sizeof(ClientSockAddrInfo));
		int clientInfo_size = sizeof(ClientSockAddrInfo);
		//
		SOCKET ClientConn = accept(ServSock, (sockaddr*)&ClientSockAddrInfo, &clientInfo_size);//accept - подтверждение связи с клиентом,стоп программы,пока не будет обращения на сокет.
		if (ClientConn == INVALID_SOCKET)
		{
			std::cout << "клиент определен, но соединение установить не удалось: "
				<< WSAGetLastError() << "\n";
			closesocket(ServSock);
			closesocket(ClientConn);
			WSACleanup();
			system("pause");
			exit(1);

		}
		else
		{
			std::cout << "соединение с клиентом установлено\n";
			show(ServSockAddrInfo, "ServSock"); // информация об открытых сокетах
			show(ClientSockAddrInfo, "ClientConn");
			return ClientConn;
		}
	}
	void shutdown(SOCKET SevrSock)
	{
		closesocket(SevrSock);
		WSACleanup();
		std::cout << "Завершение работы" << std::endl;
		system("pause");
	}
	void sendResponse(SOCKET ClientConn) {
		const char* responses[] = {
			"Конечно! Вот что у меня есть для тебя:",
"Дай мне подумать об этом",
"Хм, интересный вопрос! Вот ответ:",
"Понял! Вот сенсация:",
"Ах, вопрос! Позвольте мне дать ответ:",
"Считайте, что дело сделано! Вот мой ответ:",
"Дай мне секунду, чтобы придумать ответ ...",
"Хорошо, вот что у меня есть для тебя:",
"Позвольте мне немного поразмыслить над этим...",
"Вот ответ специально для вас:",
"Попался! Вот подноготная:",
"Держись крепче, пока я сгенерирую ответ ...",
"Хорошо, давай посмотрим... Вот что я придумал.",
"Хм, позвольте мне придумать подходящий ответ...",
"Понятно! Вот то, что вы ищете:",
"Я занимаюсь этим! Вот вам ответ:",
"Подожди, пока я соберусь с мыслями...",
"Ах, вопрос! Позвольте мне высказать некоторое понимание:",
"Считайте, что это решено! Вот мой ответ:",
"Хорошо, вот что у меня есть для тебя на этот раз:"
		};

		srand(time(nullptr));
		int random_index = rand() % (sizeof(responses) / sizeof(responses[0]));

		int packet_size = send(ClientConn, responses[random_index], strlen(responses[random_index]), 0);
		if (packet_size == SOCKET_ERROR) {
			std::cout << "Ошибка отправки случайного ответа на клиент: # " << WSAGetLastError() << "\n";
			closesocket(ClientConn);
			WSACleanup();
			system("pause");
			exit(1);
		}
		else
			std::cout << "Случайный ответ отправлен клиенту.\n";
	}

	void sends(SOCKET ClientConn) {
		while (true) {
			char buffer[1024];
			int recv_size = recv(ClientConn, buffer, sizeof(buffer), 0);
			if (recv_size == SOCKET_ERROR) {
				std::cout << "Ошибка получения сообщения от клиента: # " << WSAGetLastError() << "\n";
				closesocket(ClientConn);
				WSACleanup();
				system("pause");
				exit(1);
			}
			buffer[recv_size] = '\0';

			if (strcmp(buffer, "Shut_Up") == 0) {
				std::cout << "Клиент запросил завершение соединения\n";
				closesocket(ClientConn);
				WSACleanup();
				break;
			}

			sendResponse(ClientConn);
		}
	}
};

int main()
{
	setlocale(LC_ALL, "Russian");
	char serverIP[16];
	unsigned short serverPort;
	ZeroMemory(&serverIP, sizeof(serverIP));
	std::cout << "Введите ServerIP= ";
	std::cin >> serverIP;
	std::cout << "Введите ServerPort= ";
	std::cin >> serverPort;
	Server server(serverIP, serverPort);//создание объекта
	server.DLL_Connection(serverIP, serverPort);//подключение к DLL библ. WSA
	SOCKET servSock = server.Initialization_Socket();//инициализация сокета
	sockaddr_in servsockaddrinfo = server.Transf_Binding_Listening(0, serverIP, serverPort, servSock);//передача и прослушивание
	SOCKET ClientConn = server.Confirmation(servSock, servsockaddrinfo); // Подтверждение соединения с клиентом
	server.sends(ClientConn);
	server.shutdown(servSock);//Завершение работы сервера.
}