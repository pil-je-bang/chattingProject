#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h> //Winsock ������� include. WSADATA �������.
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <iomanip>
#include <vector>
#include <regex>
#include <conio.h>
#include<Windows.h>

#define MAX_SIZE 1024

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::to_string;
using namespace std;

SOCKET ClientSock;
string MyNick;

void GoToXY(int x, int y, int z) {
	COORD Pos;  //x, y�� ������ �ִ� ����ü
	Pos.X = x;  //x�� �����̴� ����
	Pos.Y = z + 2 * y;//z=24 �ʱⰪ
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

int FirstMenu(int z, int j) {
	int Menu = 0;
	int Count = 0;
	int Direction;
	for (;;) {
		GoToXY(10, Count, z);
		if (1) {        //Ű���� �Է� Ȯ�� (true / false)
			Direction = _getch();      // ����Ű �Է½� 224 00�� ������ �Ǳ⿡ �տ� �ִ� �� 224�� ����
			if (Direction == 224)
				Direction = _getch();  // ���� �Է°��� �Ǻ��Ͽ� �����¿� ���
			if (Count >= 0 && Count <= j) {
				switch (Direction) {
				case 72:
					if (Count > 0)
						Count--;
					continue;
				case 80:
					if (Count < j)
						Count++;
					continue;
				case 13:
					Menu = Count;
					break;
				}
			}
		}
		break;
	}
	return Menu;
}

int StartMenu()
{
	cout << "\n";
	cout << " "; cout << "*************************************************\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*       *******      *       *       *  *       *\n";
	cout << " "; cout << "*          *        * *      *       * *        *\n";
	cout << " "; cout << "*          *       *****     *       **         *\n";
	cout << " "; cout << "*          *      *     *    *       * *        *\n";
	cout << " "; cout << "*          *     *       *   *****   *  *       *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               < ���� ȭ�� >                   *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               1. �α���                       *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               2. ȸ������                     *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               0. ����                         *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*************************************************\n\n";
	return FirstMenu(13, 2);
}

int MainMenu()
{
	cout << "\n";
	cout << " "; cout << "*************************************************\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*       *******      *       *       *  *       *\n";
	cout << " "; cout << "*          *        * *      *       * *        *\n";
	cout << " "; cout << "*          *       *****     *       **         *\n";
	cout << " "; cout << "*          *      *     *    *       * *        *\n";
	cout << " "; cout << "*          *     *       *   *****   *  *       *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               < ���� ȭ�� >                   *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               1. ä�ù�                       *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               2. ȸ��Ż��                     *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               3. ��������                     *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               4. �α׾ƿ�                     *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*************************************************\n\n";
	return FirstMenu(13, 3);
}

string FindSubstr(string const& str, int n) { //���� ã�� �Լ�
	if (str.length() < n) {
		return str;
	}
	return str.substr(0, n);
}


int ChatRecv() {
	char ChatBuf[MAX_SIZE] = { };
	string Msg;

	while (1) {
		ZeroMemory(&ChatBuf, MAX_SIZE);
		if (recv(ClientSock, ChatBuf, MAX_SIZE, 0) > 0) {
			Msg = ChatBuf;
			size_t startPos = Msg.find("] ") + 2;
			Msg = Msg.substr(startPos, Msg.find(" : ", startPos) - startPos);
			std::stringstream ss(Msg);  // ���ڿ��� ��Ʈ��ȭ
			string User;
			ss >> User; // ��Ʈ���� ����, ���ڿ��� ���� �и��� ������ �Ҵ�. ���� ����� �̸��� user�� �����.
			if (User != MyNick) {
				if (FindSubstr(ChatBuf, 3) == "/dm") { //�޽��� �޾Ƽ� dm���� �ƴ��� �����ؼ�  ���
					string Dm = Msg.substr(Msg.find("m")+1);
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
					cout << Dm << endl; // ���� ���� �� �ƴ� ��쿡�� ����ϵ���.
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
				}
				else {
					cout << ChatBuf << endl; // ���� ���� �� �ƴ� ��쿡�� ����ϵ���.
				}
			}
		}
		else {
			cout << "Server Off" << endl;
			return -1;
		}
	}
}


void SignUp() { //ȸ������
	send(ClientSock, "2", 1, 0);

	vector<string> UserInfo = { "���̵�","�̸�","��й�ȣ(����,����,Ư������ ����)","birth(yyyy-mm-dd)","����ó (010-xxxx-xxxx)","email","address" };
	
	int SpecialList[12] = { '!','@','#','$','%','^','&','*','(',')','-','+' };
	bool NumberCheck = false;  // ���� check
	bool EnglishCheck = false; // ���� check
	bool SpecialCheck = false; // Ư�� check
	bool Id = true;
	

	while (Id) {
		char SignUpBuf[MAX_SIZE] = {};
		string InputId = "";
		cout << "============ȸ������==============" << endl;
		cout << "���̵� :";
		cin >> InputId;
		int InputIdLen = InputId.length();
		for (int i = 0; i < InputIdLen; i++)
		{
			char Check = InputId[i];
			if (!NumberCheck)
				NumberCheck = isdigit(Check);
			if (!EnglishCheck)
				EnglishCheck = isalpha(Check);
		}

		if (NumberCheck && EnglishCheck) {
			send(ClientSock, InputId.c_str(), InputIdLen, 0);
			recv(ClientSock, SignUpBuf, sizeof(SignUpBuf), 0);
			if (strcmp(SignUpBuf, "false") == 0) {
				Id = true;
				cout << "���̵� �ߺ��Դϴ�.";
				Sleep(4000);
			}
			else {
				Id = false;
				Sleep(1500);
			}
		}
		else
		{
			cout << "���ڿ� ������ �������� id�� �����ϼ���." << endl;
			NumberCheck = false;  // ���� check
			EnglishCheck = false; // ���� check
			SpecialCheck = false; // Ư�� check
			Sleep(1500);
		}
		system("cls");
	}

	string Input = "";
	for (int i = 1; i < 7; i++) {

		if (i == 2) {
			cout << UserInfo[i] << ": ";
			while (!(NumberCheck && EnglishCheck && SpecialCheck)) {
				cin >> Input;
				int PwLen = Input.length();
				for (int i = 0; i < PwLen; i++)
				{
					char Check = Input[i];
					if (!NumberCheck)
						NumberCheck = isdigit(Check);

					if (!EnglishCheck)
						EnglishCheck = isalpha(Check);

					for (int j = 0; j < 12; j++) {
						if (!SpecialCheck) {
							if (Check == SpecialList[j]) {
								SpecialCheck = 1;
								break;
							}
						}
					}
				}
				if (!(NumberCheck && EnglishCheck && SpecialCheck)) {
					cout << "��й�ȣ�� [����,����,Ư������]�� �̷������ �մϴ�." << endl;
					NumberCheck = false;  // ���� check
					EnglishCheck = false; // ���� check
					SpecialCheck = false; // Ư�� check
					Sleep(1500);
					cout << "pw :";
				}
				else {
					UserInfo[i] = Input;
					send(ClientSock, Input.c_str(), Input.length(), 0);
				}
			}

		}
		else if (i == 3) {
			cout << UserInfo[i] << ": ";
			bool Cnt = true;
			while (Cnt) {
				cin >> Input;
				regex Birth("\\d{4}-\\d{2}-\\d{2}");

				if (!(regex_match(Input, Birth))) {
					cout << "������ ���� �ʽ��ϴ�." << endl;
				}
				else {
					UserInfo[i] = Input;
					Cnt = false;
					send(ClientSock, Input.c_str(), Input.length(), 0);
				}
			}
		}
		else if (i == 4) {
			cout << UserInfo[i] << ": ";
			bool Cnt = true;
			while (Cnt) {
				cin >> Input;
				regex Phone("[01]{3}-\\d{4}-\\d{4}");
				if (!(regex_match(Input, Phone))) {
					cout << "������ ���� �ʽ��ϴ�." << endl;
				}
				else {
					UserInfo[i] = Input;
					Cnt = false;
					send(ClientSock, Input.c_str(), Input.length(), 0);
				}
			}
		}
		else if (i == 5) {
			cout << UserInfo[i] << ": ";
			bool Cnt = true;
			while (Cnt) {
				cin >> Input;
				regex mail("[_a-z0-9-]+(.[_a-z0-9-]+)*@[a-z0-9-]+(.[a-z0-9-]+)*(.[a-z]{2,4})");
				if (!(regex_match(Input, mail))) {
					cout << "������ ���� �ʽ��ϴ�." << endl;
				}
				else {
					UserInfo[i] = Input;
					Cnt = false;
					send(ClientSock, Input.c_str(), Input.length(), 0);
				}
			}
		}
		else if (i == 1) {
			cout << UserInfo[i] << ": ";
			cin >> Input;
			UserInfo[i] = Input;
			send(ClientSock, Input.c_str(), Input.length(), 0);
			Input = "";
		}
		else if (i == 6) {
			cout << UserInfo[i] << ": ";
			cin >> Input;
			UserInfo[i] = Input;
			send(ClientSock, Input.c_str(), Input.length(), 0);
		}
	}
	cout << "ȸ�������� �Ϸ�Ǿ����ϴ�.";
	Sleep(1500);
}

void LogIn() { //�α���
	bool LoginSuccess = false;
	send(ClientSock, "1", 1, 0);

	while (LoginSuccess == false) {
		string InputId = "";
		string InputPw = "";
		cout << "==========login===========" << endl;
		cout << "id :";
		cin >> InputId;
		cout << "pw :";
		cin >> InputPw;
		MyNick = InputId;
		string Login;

		Login = (InputId + "-" + InputPw);

		// ������ �α��� ���� ����
		send(ClientSock, Login.c_str(), Login.length(), 0);

		// �����κ��� �α��� ��� �ޱ�
		char LoginBuf[MAX_SIZE] = { };
		recv(ClientSock, LoginBuf, MAX_SIZE, 0);

		// ��� ���
		if (strcmp(LoginBuf, "true") == 0) {
			cout << "�α��� ����" << std::endl;
			LoginSuccess == true;
			break;
		}
		else {
			cout << "�α��� ����" << std::endl;
			LoginSuccess == false;
		}
	}
}

void Withdrawal() { //ȸ��Ż��
	string InputId = "";
	string InputPw = "";
	send(ClientSock, "3", 1, 0);

	while (1) {
		cout << "==========ȸ��Ż��===========" << endl;
		cout << "id:";
		cin >> InputId;
		cout << "pw:";
		cin >> InputPw;
		string WithdrawalInfo;
		WithdrawalInfo = InputId + "-" + InputPw;

		send(ClientSock, WithdrawalInfo.c_str(), WithdrawalInfo.length(), 0);
		char WithdrawalBuf[MAX_SIZE] = { };
		recv(ClientSock, WithdrawalBuf, MAX_SIZE, 0);

		// ��� ���
		if (strcmp(WithdrawalBuf, "true") == 0) {
			while (1) {
				cout << "���� ȸ�� Ż�� �Ͻðڽ��ϱ�?" << endl;
				cout << "Ż�� ���Ͻø� yes�� �Է��Ͻð� �ƴϸ� no�� �Է��ϼ���";
				string WithdrawalYN;
				cin >> WithdrawalYN;
				send(ClientSock, WithdrawalYN.c_str(), WithdrawalYN.length(), 0);

				char WithdrawalBuf2[MAX_SIZE] = { };
				recv(ClientSock, WithdrawalBuf2, MAX_SIZE, 0);

				if (strcmp(WithdrawalBuf2, "true") == 0) {
					cout << "�׵��� �̿����ּż� �����մϴ�." << endl;
					cout << "ȸ�� Ż�� �Ϸ�Ǿ����ϴ�."<< endl;
					Sleep(1500);
					break;
				}
				else if (strcmp(WithdrawalBuf2, "false") == 0) {
					cout << "�ٽ� ���ƿ��Ű� ȯ���մϴ�."<< endl;
					Sleep(1500);
					break;
				}
				else {
					cout << "yes�� no�� �Է��ϼ���" << endl;
				}
			}
		}
		else {
			cout << "ȸ�������� ���� �ʽ��ϴ�." << endl;
			continue;
		}
		break;
	}
}


void Revise() { //ȸ������ ����
	send(ClientSock, "4", 1, 0);
	bool CompleteRevise = true;
	char ReviseBuf[MAX_SIZE] = { };

	while (CompleteRevise) {
		string ReviseNumber;
		string InputId;
		string InputPw;
		string ReviseLogin;
		cout << "���̵� �Է��ϼ��� : ";
		cin >> InputId;
		cout << "��й�ȣ�� �Է��ϼ��� : ";
		cin >> InputPw;
		ReviseLogin = (InputId + "-" + InputPw);
		string ReviseInfo;
		string ReviseInformation;

		send(ClientSock, ReviseLogin.c_str(), ReviseLogin.length(), 0);
		recv(ClientSock, ReviseBuf, MAX_SIZE, 0); // ��� ������ true, Ʋ���� false

		if (strcmp(ReviseBuf, "true") == 0) {
			cout << "������ ������ ������." << endl;
			cout << "1. name  2. pw  3. birth  4. num  5. email  6. address" << endl;
			cin >> ReviseNumber;
		}

		if (ReviseNumber == "1") {
			cout << "����� �̸��� �Է��ϼ��� : ";
			cin >> ReviseInformation;
			ReviseInfo = ReviseNumber + "-" + ReviseInformation;
		}
		else if (ReviseNumber == "2") {
			cout << "����� pw�� �Է��ϼ��� : ";
			cin >> ReviseInformation;
			ReviseInfo = ReviseNumber + "-" + ReviseInformation;
		}
		else if (ReviseNumber == "3") {
			cout << "����� birth�� �Է��ϼ��� : ";
			cin >> ReviseInformation;
			ReviseInfo = ReviseNumber + "-" + ReviseInformation;
		}
		else if (ReviseNumber == "4") {
			cout << "����� number�� �Է��ϼ��� : ";
			cin >> ReviseInformation;
			ReviseInfo = ReviseNumber + "-" + ReviseInformation;
		}
		else if (ReviseNumber == "5") {
			cout << "����� email�� �Է��ϼ��� : ";
			cin >> ReviseInformation;
			ReviseInfo = ReviseNumber + "-" + ReviseInformation;
		}
		else if (ReviseNumber == "6") {
			cout << "����� email�� �Է��ϼ��� : ";
			cin >> ReviseInformation;
			ReviseInfo = ReviseNumber + "-" + ReviseInformation;
		}
		send(ClientSock, ReviseInfo.c_str(), ReviseInfo.length(), 0);
		char ReviseBuf2[MAX_SIZE] = { };
		recv(ClientSock, ReviseBuf2, MAX_SIZE, 0);

		if (strcmp(ReviseBuf2, "true") == 0) {
			cout << "������ �Ϸ�Ǿ����ϴ�.";
			Sleep(1500);
			CompleteRevise = false;
			break;
		}
	}
}

void LogOut() { //�α׾ƿ�
	send(ClientSock, "5", 1, 0);
}



int main()
{
	WSADATA wsa;

	// Winsock�� �ʱ�ȭ�ϴ� �Լ�. MAKEWORD(2, 2)�� Winsock�� 2.2 ������ ����ϰڴٴ� �ǹ�.
	// ���࿡ �����ϸ� 0��, �����ϸ� �� �̿��� ���� ��ȯ.
	// 0�� ��ȯ�ߴٴ� ���� Winsock�� ����� �غ� �Ǿ��ٴ� �ǹ�.
	int code = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (!code) {
		ClientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

		// ������ ���� ���� ���� �κ�
		SOCKADDR_IN ClientAddr = {};
		ClientAddr.sin_family = AF_INET;
		ClientAddr.sin_port = htons(7777);
		InetPton(AF_INET, TEXT("192.168.195.243"), &ClientAddr.sin_addr); //192.168.0.63   192.168.195.243

		while (1) {
			if (!connect(ClientSock, (SOCKADDR*)&ClientAddr, sizeof(ClientAddr))) { // ���� ������ ������ �ش��ϴ� server�� ����!
				break;
			}
			cout << "Connecting..." << endl;
		}

		while (1) {
			int MenuNum = StartMenu(); //startMenu

			if (MenuNum == 0) { //menu_num�� 0�̸� �α���
				system("cls");
				LogIn(); //�α���
				system("cls");
				int MainNum = MainMenu(); //mainMenu

				if (MainNum == 0) { //main_num�� 0�̸� ä�� ����
					system("cls");
					send(ClientSock, "1", strlen("1"), 0);
					std::thread ChatThread(ChatRecv);
					cout << "ä���� ���۵Ǿ����ϴ�.";
					cout << "\n";

					while (1) {
						string Text;
						std::getline(cin, Text);
						const char* ChatBuffer = Text.c_str(); // string���� char* Ÿ������ ��ȯ
						send(ClientSock, ChatBuffer, strlen(ChatBuffer), 0);
					}
					ChatThread.join();
					closesocket(ClientSock);
				}
				else if (MainNum == 1) { //main_num�� 1�̸� ȸ��Ż��
					system("cls");
					Withdrawal();
					system("cls");
				}
				else if (MainNum == 2) { //main_num�� 2�̸� ȸ������
					system("cls");
					Revise();
					system("cls");
				}
				else if (MainNum == 3) { //main_num�� 3�̸� �α׾ƿ�
					system("cls");
					LogOut();
				}
			}
			else if (MenuNum == 1) { //menu_num�� 1�̸� ȸ������
				system("cls");
				SignUp();
				system("cls");
			}
		}
		WSACleanup();
		return 0;
	}
}