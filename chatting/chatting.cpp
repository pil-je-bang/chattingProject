#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h> //Winsock ������� include. WSADATA �������.
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <mysql/jdbc.h>
#include <thread>
#include <sstream>
#include <iomanip>
#include <vector>
#include <regex>
#include <conio.h>
#include<Windows.h>
#include<algorithm>

#define MAX_SIZE 1024

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::to_string;
using namespace std;

SOCKET client_sock;
string my_nick;

// MySQL Connector/C++ �ʱ�ȭ
sql::mysql::MySQL_Driver* driver;// ���� �������� �ʾƵ� Connector/C++�� �ڵ����� ������ ��
sql::Connection* con;
sql::Statement* stmt;
sql::PreparedStatement* pstmt;
sql::ResultSet* res = NULL;

const string server = "tcp://127.0.0.1:3306"; // �����ͺ��̽� �ּ�
const string username = "root"; // �����ͺ��̽� �����
const string password = "admin"; // �����ͺ��̽� ���� ��й�ȣ

void gotoxy(int x, int y, int z) {
	COORD Pos;  //x, y�� ������ �ִ� ����ü
	Pos.X = x;  //x�� �����̴� ����
	Pos.Y = z + 2 * y;//z=24 �ʱⰪ
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

int firstmenu(int z, int j) {
	int menu = 0;
	int count = 0;
	int c;
	for (;;) {
		gotoxy(10, count, z);
		if (1) {        //Ű���� �Է� Ȯ�� (true / false)
			c = _getch();      // ����Ű �Է½� 224 00�� ������ �Ǳ⿡ �տ� �ִ� �� 224�� ����
			if (c == 224)
				c = _getch();  // ���� �Է°��� �Ǻ��Ͽ� �����¿� ���
			if (count >= 0 && count <= j) {
				switch (c) {
				case 72:
					if (count > 0)
						count--;
					continue;
				case 80:
					if (count < j)
						count++;
					continue;
				case 13:
					menu = count;
					break;
				}
			}
		}
		break;
	}
	return menu;
}

int startMenu()
{

	cout << "\n";
	/*vector<string> menu;*/
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
	cout << " "; cout << "*               3. ȸ��Ż��                     *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               4. ��������                     *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               0. ����                         *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*************************************************\n\n";
	/*show(menu);*/
	int menu_num = firstmenu(13, 4);
	return menu_num;
}
string findsubstr(string const& str, int n) {
	if (str.length() < n) {
		return str;
	}
	return str.substr(0, n);
}
int chat_recv() {
	char buf[MAX_SIZE] = { };
	string msg;

	while (1) {
		ZeroMemory(&buf, MAX_SIZE);
		if (recv(client_sock, buf, MAX_SIZE, 0) > 0) {
			msg = buf;
			size_t startPos = msg.find("] ") + 2;
			msg = msg.substr(startPos, msg.find(" : ", startPos) - startPos);
			std::stringstream ss(msg);  // ���ڿ��� ��Ʈ��ȭ
			string user;
			ss >> user; // ��Ʈ���� ����, ���ڿ��� ���� �и��� ������ �Ҵ�. ���� ����� �̸��� user�� �����.
			if (user != my_nick) {
				cout << buf << endl; // ���� ���� �� �ƴ� ��쿡�� ����ϵ���.
			}
		

		}
		else {
			cout << "Server Off" << endl;
			return -1;
		}
	}
}
string sign_up() {
	send(client_sock, "2", 1, 0);

	vector<string> user_info = { "���̵�","�̸�","��й�ȣ(����,����,Ư������ ����)","birth(yyyy-mm-dd)","����ó (010-xxxx-xxxx)","email","address" };
	int specialList[12] = { '!','@','#','$','%','^','&','*','(',')','-','+' };
	bool numberCheck = false;  // ���� check
	bool englishCheck = false; // ���� check
	bool specialCheck = false; // Ư�� check
	bool is_there_same = 1;
	string in;

	bool id = true;

	while (id) {

		char buf[MAX_SIZE] = {};
		in = "";
		cout << "============ȸ������==============" << endl;
		cout << "���̵� :";
		cin >> in;
		int id_len = in.length();
		for (int i = 0; i < id_len; i++)
		{
			char check = in[i];
			if (!numberCheck)
				numberCheck = isdigit(check);
			if (!englishCheck)
				englishCheck = isalpha(check);
		}

		if (numberCheck && englishCheck) {
			send(client_sock, in.c_str(), id_len, 0);
			recv(client_sock, buf, sizeof(buf), 0);
			if (strcmp(buf, "false") == 0) {
				id = true;
			}
			else {
				id = false;
				Sleep(1500);
			}
		}
		else
		{
			cout << "���ڿ� ������ �������� id�� �����ϼ���." << endl;
			numberCheck = false;  // ���� check
			englishCheck = false; // ���� check
			specialCheck = false; // Ư�� check
			Sleep(1500);
		}
		system("cls");
	}

	string input = "";
	for (int i = 1; i < 7; i++) {

		if (i == 2) {
			cout << user_info[i] << ": ";
			while (!(numberCheck && englishCheck && specialCheck)) {
				cin >> input;
				int pw_len = input.length();
				for (int i = 0; i < pw_len; i++)
				{
					char check = input[i];
					if (!numberCheck)
						numberCheck = isdigit(check);

					if (!englishCheck)
						englishCheck = isalpha(check);

					for (int j = 0; j < 12; j++) {
						if (!specialCheck) {
							if (check == specialList[j]) {
								specialCheck = 1;
								break;
							}
						}
					}
				}
				if (!(numberCheck && englishCheck && specialCheck)) {
					cout << "��й�ȣ�� [����,����,Ư������]�� �̷������ �մϴ�." << endl;
					numberCheck = false;  // ���� check
					englishCheck = false; // ���� check
					specialCheck = false; // Ư�� check
					Sleep(1500);
					cout << "pw :";
				}
				else {
					user_info[i] = input;
					send(client_sock, input.c_str(), input.length(), 0);

				}
			}

		}
		else if (i == 3) {
			cout << user_info[i] << ": ";
			bool cnt = true;
			while (cnt) {
				cin >> input;

				regex birth("\\d{4}-\\d{2}-\\d{2}");

				if (!(regex_match(input, birth))) {
					cout << "������ ���� �ʽ��ϴ�." << endl;
				}
				else {
					user_info[i] = input;
					cnt = false;
					send(client_sock, input.c_str(), input.length(), 0);
				}
			}

		}
		else if (i == 4) {
			cout << user_info[i] << ": ";
			bool cnt = true;
			while (cnt) {
				cin >> input;
				regex phone("[01]{3}-\\d{4}-\\d{4}");
				if (!(regex_match(input, phone))) {
					cout << "������ ���� �ʽ��ϴ�." << endl;
				}
				else {
					user_info[i] = input;
					cnt = false;
					send(client_sock, input.c_str(), input.length(), 0);
				}
			}
		}
		else if (i == 5) {
			cout << user_info[i] << ": ";
			bool cnt = true;
			while (cnt) {
				cin >> input;
				regex mail("[_a-z0-9-]+(.[_a-z0-9-]+)*@[a-z0-9-]+(.[a-z0-9-]+)*(.[a-z]{2,4})");
				if (!(regex_match(input, mail))) {
					cout << "������ ���� �ʽ��ϴ�." << endl;
				}
				else {
					user_info[i] = input;
					cnt = false;
					send(client_sock, input.c_str(), input.length(), 0);
				}
			}
		}
		else if (i == 1) {
			cout << user_info[i] << ": ";
			cin >> input;
			user_info[i] = input;
			send(client_sock, input.c_str(), input.length(), 0);
			input = "";
		}
		else if (i == 6) {
			cout << user_info[i] << ": ";
			cin >> input;
			user_info[i] = input;
			send(client_sock, input.c_str(), input.length(), 0);
		}
	}

	//stmt->execute("DROP TABLE IF EXISTS chatting");// ä�� ���� �Ҷ� �����ؾ� ��
	//stmt->execute("CREATE TABLE chatting (id VARCHAR(50), ���� VARCHAR(255));"); // CREATE
	//delete stmt;

	//stmt->execute("DROP TABLE IF EXISTS user_info");// ä�� ���� �Ҷ� �����ؾ� ��
	//stmt->execute("CREATE TABLE user_info (id VARCHAR(50), name VARCHAR(50), pw VARCHAR(50), birth date, num VARCHAR(50), email VARCHAR(50), address VARCHAR(50));"); // CREATE


	return in;
}
string login(/*string input_id, string input_pw*/) {
	string input_id = "";
	string input_pw = "";
	bool login_success = false;
	send(client_sock, "1", 1, 0);

	while (login_success == false) {

		cout << "==========login===========" << endl;
		cout << "id :";
		//if (input_id.length() == 0)
		//    getline(cin, input_id);
		cin >> input_id;
		cout << "pw :";
		//if (input_pw.length() == 0)
		//    getline(cin, input_pw);
		cin >> input_pw;
		my_nick = input_id;
		string login;

		login = (input_id + "-" + input_pw);


		// ������ �α��� ���� ����
		send(client_sock, login.c_str(), login.length(), 0);

		// �����κ��� �α��� ��� �ޱ�
		char buf[MAX_SIZE] = { };
		recv(client_sock, buf, MAX_SIZE, 0);

		// ��� ���
		if (strcmp(buf, "true") == 0) {
			cout << "�α��� ����" << std::endl;
			login_success == true;
			break;
		}
		else {
			cout << "�α��� ����" << std::endl;
			login_success == false;
		}
	}
	return input_id;
}
void withdrawal() {
	string input_id = "";
	string input_pw = "";
	send(client_sock, "3", 1, 0);


	while (1) {
		cout << "==========ȸ��Ż��===========" << endl;
		cout << "id:";
		cin >> input_id;
		cout << "pw:";
		cin >> input_pw;
		string withdrawal_info;
		withdrawal_info = input_id + "-" + input_pw;

		send(client_sock, withdrawal_info.c_str(), withdrawal_info.length(), 0);

		char buf[MAX_SIZE] = { };
		recv(client_sock, buf, MAX_SIZE, 0);

		// ��� ���
		if (strcmp(buf, "true") == 0) {
			while (1) {
				cout << "���� ȸ�� Ż�� �Ͻðڽ��ϱ�?" << endl;
				cout << "Ż�� ���Ͻø� yes�� �Է��Ͻð� �ƴϸ� no�� �Է��ϼ���";
				string withdrawalYN;
				cin >> withdrawalYN;
				send(client_sock, withdrawalYN.c_str(), withdrawalYN.length(), 0);

				char buf2[MAX_SIZE] = { };
				recv(client_sock, buf2, MAX_SIZE, 0);

				if (strcmp(buf2, "true") == 0) {
					cout << "�׵��� �̿����ּż� �����մϴ�." << endl;
					cout << "ȸ�� Ż�� �Ϸ�Ǿ����ϴ�.";
					break;
				}
				else if (strcmp(buf2, "false") == 0) {
					cout << "�ٽ� ���ƿ��Ű� ȯ���մϴ�.";
					break;
				}
				else {
					cout << "yes�� no�� �Է��ϼ���";
					continue;
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
void beforechatting() {
	pstmt = con->prepareStatement("SELECT id, ���� FROM chatting");
	res = pstmt->executeQuery();
	vector<string> id;
	vector<string> chatting;
	int i = 0;
	while (res->next()) {
		id.push_back(res->getString(1));
		chatting.push_back(res->getString(2));
	}
	for (int i = 0; i < id.size(); i++) {
		cout << id.at(i) << " : " << chatting.at(i) << endl;
	}
}
void revise() {
	send(client_sock, "4", 1, 0);
	bool complete_revise = true;
	char buf[MAX_SIZE] = { };


	while (complete_revise) {
		string revise_number;
		string input_pw;
		cout << "��й�ȣ�� �Է��ϼ��� : ";
		cin >> input_pw;
		string revise_info;
		string revise_information;

		send(client_sock, input_pw.c_str(), input_pw.length(), 0);

		recv(client_sock, buf, MAX_SIZE, 0);

		if (strcmp(buf, "true") == 0) {
			cout << "������ ������ ������." << endl;
			cout << "1. name  2. pw  3. birth  4. num  5. email  6. address" << endl;
			cin >> revise_number;
		}
		char buf1[MAX_SIZE] = { };

		if (revise_number == "1") {
			cout << "����� �̸��� �Է��ϼ��� : ";
			cin >> revise_information;
			revise_info = revise_number + "-" + revise_information;
		}
		else if (revise_number == "2") {
			cout << "����� pw�� �Է��ϼ��� : ";
			cin >> revise_information;
			revise_info = revise_number + "-" + revise_information;
		}
		else if (revise_number == "3") {
			cout << "����� birth�� �Է��ϼ��� : ";
			cin >> revise_information;
			revise_info = revise_number + "-" + revise_information;
		}
		else if (revise_number == "4") {
			cout << "����� number�� �Է��ϼ��� : ";
			cin >> revise_information;
			revise_info = revise_number + "-" + revise_information;
		}
		else if (revise_number == "5") {
			cout << "����� email�� �Է��ϼ��� : ";
			cin >> revise_information;
			revise_info = revise_number + "-" + revise_information;
		}
		else if (revise_number == "6") {
			cout << "����� email�� �Է��ϼ��� : ";
			cin >> revise_information;
			revise_info = revise_number + "-" + revise_information;
		}
		send(client_sock, revise_info.c_str(), revise_info.length(), 0);

		recv(client_sock, buf1, MAX_SIZE, 0);

		if (strcmp(buf1, "true") == 0) {
			cout << "������ �Ϸ�Ǿ����ϴ�.";
			complete_revise = false;
			break;
		}
	}
}
int main(/*int argc, char *argv[]*/)
{
	WSADATA wsa;

	// Winsock�� �ʱ�ȭ�ϴ� �Լ�. MAKEWORD(2, 2)�� Winsock�� 2.2 ������ ����ϰڴٴ� �ǹ�.
	// ���࿡ �����ϸ� 0��, �����ϸ� �� �̿��� ���� ��ȯ.
	// 0�� ��ȯ�ߴٴ� ���� Winsock�� ����� �غ� �Ǿ��ٴ� �ǹ�.
	int code = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (!code) {
		client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // 

		// ������ ���� ���� ���� �κ�
		SOCKADDR_IN client_addr = {};
		client_addr.sin_family = AF_INET;
		client_addr.sin_port = htons(7777);
		InetPton(AF_INET, TEXT("192.168.0.28"), &client_addr.sin_addr); //192.168.0.63   192.168.195.243

		while (1) {
			if (!connect(client_sock, (SOCKADDR*)&client_addr, sizeof(client_addr))) { // ���� ������ ������ �ش��ϴ� server�� ����!
				cout << "Server Connect" << endl;
				break;
			}
			cout << "Connecting..." << endl;
		}

		while (1) {
			int menu_num = startMenu();

			//while(1) {
			//	cout << "1�� �α���,  2�� ȸ������,  3�� ȸ��Ż��, 4�� ȸ���������� ";
			//	int a;
			//	cin >> a;
			/*a = atoi(argv[1]);*/
			if (menu_num == 0) {
				system("cls");
				//string id = "";
				//string pw = "";
				login();
				break;
				/*beforechatting();*/
			}
			else if (menu_num == 1) {
				system("cls");
				sign_up();
			}
			else if (menu_num == 2) {
				system("cls");
				withdrawal();
			}
			else if (menu_num == 3) {
				system("cls");
				revise();
				system("cls");
			}
		}


		std::thread th2(chat_recv);
		cout << "ä���� ���۵Ǿ����ϴ�.";
		cout << "\n";

		while (1) {
			string text;
			std::getline(cin, text);
			/*cin >> text;*/
			const char* buffer = text.c_str(); // string���� char* Ÿ������ ��ȯ
			if (findsubstr(text, sizeof("/������")) == "/������") {
				send(client_sock, buffer, strlen(buffer), 0);
			}
			else {
				send(client_sock, buffer, strlen(buffer), 0);
			}
		}
		th2.join();
		closesocket(client_sock);

		delete stmt;
		delete res;
		delete con;
	}
	WSACleanup();

	return 0;
}