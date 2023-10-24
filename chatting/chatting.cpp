#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h> //Winsock 헤더파일 include. WSADATA 들어있음.
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

// MySQL Connector/C++ 초기화
sql::mysql::MySQL_Driver* driver;// 추후 해제하지 않아도 Connector/C++가 자동으로 해제해 줌
sql::Connection* con;
sql::Statement* stmt;
sql::PreparedStatement* pstmt;
sql::ResultSet* res = NULL;

//void show(vector<string> k) {
//	cout << setw(30) <<"<시작화면>" << endl << endl;
//	for (auto i : k) {
//		cout << setw(30)<<i << endl << endl;
//	}
//}

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "admin"; // 데이터베이스 접속 비밀번호
void gotoxy(int x, int y, int z) {
	COORD Pos;  //x, y를 가지고 있는 구조체
	Pos.X = x;  //x의 움직이는 범위
	Pos.Y = z + 2 * y;//z=24 초기값
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

int firstmenu(int z, int j) {
	int menu = 0;
	int count = 0;
	int c;
	for (;;) {
		gotoxy(10, count, z);
		if (1) {        //키보드 입력 확인 (true / false)
			c = _getch();      // 방향키 입력시 224 00이 들어오게 되기에 앞에 있는 값 224를 없앰
			if (c == 224)
				c = _getch();  // 새로 입력값을 판별하여 상하좌우 출력
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
	cout << " "; cout << "*               < 시작 화면 >                   *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               1. 로그인                       *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               2. 회원가입                     *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               0. 종료                         *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*************************************************\n\n";
	/*show(menu);*/
	return firstmenu(13, 2);
}

int mainMenu()
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
	cout << " "; cout << "*               < 메인 화면 >                   *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               1. 채팅방                       *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               2. 회원탈퇴                     *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               3. 정보수정                     *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*               4. 로그아웃                     *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*                                               *\n";
	cout << " "; cout << "*************************************************\n\n";
	/*show(menu);*/
	return firstmenu(13, 3);
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
			std::stringstream ss(msg);  // 문자열을 스트림화
			string user;
			ss >> user; // 스트림을 통해, 문자열을 공백 분리해 변수에 할당. 보낸 사람의 이름만 user에 저장됨.
			if (user != my_nick) {
				if (findsubstr(buf, 3) == "/dm") {
					string dm = msg.substr(msg.find("m") + 1);
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
					cout << dm << endl; // 내가 보낸 게 아닐 경우에만 출력하도록.
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
				}
				else {
					cout << buf << endl; // 내가 보낸 게 아닐 경우에만 출력하도록.
				}
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

	vector<string> user_info = { "아이디","이름","비밀번호(영어,숫자,특수문자 조합)","birth(yyyy-mm-dd)","연락처 (010-xxxx-xxxx)","email","address" };
	int specialList[12] = { '!','@','#','$','%','^','&','*','(',')','-','+' };
	bool numberCheck = false;  // 숫자 check
	bool englishCheck = false; // 영어 check
	bool specialCheck = false; // 특수 check
	bool is_there_same = 1;
	string in;

	bool id = true;

	while (id) {

		char buf[MAX_SIZE] = {};
		in = "";
		cout << "============회원가입==============" << endl;
		cout << "아이디 :";
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
			cout << "숫자와 영어의 조합으로 id를 생성하세요." << endl;
			numberCheck = false;  // 숫자 check
			englishCheck = false; // 영어 check
			specialCheck = false; // 특수 check
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
					cout << "비밀번호는 [영어,숫자,특수문자]로 이루어져야 합니다." << endl;
					numberCheck = false;  // 숫자 check
					englishCheck = false; // 영어 check
					specialCheck = false; // 특수 check
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
					cout << "형식이 맞지 않습니다." << endl;
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
					cout << "형식이 맞지 않습니다." << endl;
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
					cout << "형식이 맞지 않습니다." << endl;
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

	//stmt->execute("DROP TABLE IF EXISTS chatting");// 채팅 시작 할때 생성해야 됨
	//stmt->execute("CREATE TABLE chatting (id VARCHAR(50), 내용 VARCHAR(255));"); // CREATE
	//delete stmt;

	//stmt->execute("DROP TABLE IF EXISTS user_info");// 채팅 시작 할때 생성해야 됨
	//stmt->execute("CREATE TABLE user_info (id VARCHAR(50), name VARCHAR(50), pw VARCHAR(50), birth date, num VARCHAR(50), email VARCHAR(50), address VARCHAR(50));"); // CREATE

	cout << "회원가입이 완료되었습니다.";
	Sleep(1500);
	return in;
}

void login(/*string input_id, string input_pw*/) {

	bool login_success = false;
	send(client_sock, "1", 1, 0);


	while (login_success == false) {
		string input_id = "";
		string input_pw = "";
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


		// 서버에 로그인 정보(아이디-비번) 전송
		send(client_sock, login.c_str(), login.length(), 0);

		// 서버로부터 로그인 결과 받기
		char buf[MAX_SIZE] = { };
		recv(client_sock, buf, MAX_SIZE, 0);

		// 결과 출력
		if (strcmp(buf, "true") == 0) {
			cout << "로그인 성공" << std::endl;
			login_success == true;
			break;
		}
		else {
			cout << "로그인 실패(접속중이거나 없는 아이디 입니다.)" << std::endl;
			login_success == false;
		}
	}
}

void withdrawal() {
	string input_id = "";
	string input_pw = "";
	send(client_sock, "3", 1, 0);


	while (1) {
		cout << "==========회원탈퇴===========" << endl;
		cout << "id:";
		cin >> input_id;
		cout << "pw:";
		cin >> input_pw;
		string withdrawal_info;
		withdrawal_info = input_id + "-" + input_pw;

		send(client_sock, withdrawal_info.c_str(), withdrawal_info.length(), 0);

		char buf[MAX_SIZE] = { };
		recv(client_sock, buf, MAX_SIZE, 0);

		// 결과 출력
		if (strcmp(buf, "true") == 0) {
			while (1) {
				cout << "정말 회원 탈퇴를 하시겠습니까?" << endl;
				cout << "탈퇴를 원하시면 yes를 입력하시고 아니면 no를 입력하세요";
				string withdrawalYN;
				cin >> withdrawalYN;
				send(client_sock, withdrawalYN.c_str(), withdrawalYN.length(), 0);

				char buf2[MAX_SIZE] = { };
				recv(client_sock, buf2, MAX_SIZE, 0);

				if (strcmp(buf2, "true") == 0) {
					cout << "그동안 이용해주셔서 감사합니다." << endl;
					cout << "회원 탈퇴가 완료되었습니다." << endl;
					Sleep(1500);
					break;
				}
				else if (strcmp(buf2, "false") == 0) {
					cout << "다시 돌아오신걸 환영합니다." << endl;
					Sleep(1500);
					break;
				}
				else {
					cout << "yes나 no만 입력하세요" << endl;
					continue;
				}
			}

		}
		else {
			cout << "회원정보가 맞지 않습니다." << endl;
			continue;
		}
		break;
	}
}

void beforechatting() {
	pstmt = con->prepareStatement("SELECT id, 내용 FROM chatting");
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
		cout << "비밀번호를 입력하세요 : ";
		cin >> input_pw;
		string revise_info;
		string revise_information;

		send(client_sock, input_pw.c_str(), input_pw.length(), 0);

		recv(client_sock, buf, MAX_SIZE, 0); // 비번 맞는지 확인
		if (strcmp(buf, "true") == 0) {
			cout << "수정할 정보를 고르세요." << endl;
			cout << "1. name  2. pw  3. birth  4. num  5. email  6. address" << endl;
			cin >> revise_number;
		}
		char buf1[MAX_SIZE] = { };

		if (revise_number == "1") {
			cout << "변경된 이름을 입력하세요 : ";
			cin >> revise_information;
			revise_info = revise_number + "-" + revise_information;
		}
		else if (revise_number == "2") {
			cout << "변경된 pw를 입력하세요 : ";
			cin >> revise_information;
			revise_info = revise_number + "-" + revise_information;
		}
		else if (revise_number == "3") {
			cout << "변경된 birth를 입력하세요 : ";
			cin >> revise_information;
			revise_info = revise_number + "-" + revise_information;
		}
		else if (revise_number == "4") {
			cout << "변경된 number를 입력하세요 : ";
			cin >> revise_information;
			revise_info = revise_number + "-" + revise_information;
		}
		else if (revise_number == "5") {
			cout << "변경된 email를 입력하세요 : ";
			cin >> revise_information;
			revise_info = revise_number + "-" + revise_information;
		}
		else if (revise_number == "6") {
			cout << "변경된 email를 입력하세요 : ";
			cin >> revise_information;
			revise_info = revise_number + "-" + revise_information;
		}
		send(client_sock, revise_info.c_str(), revise_info.length(), 0);

		recv(client_sock, buf1, MAX_SIZE, 0);

		if (strcmp(buf1, "true") == 0) {
			cout << "변경이 완료되었습니다.";
			Sleep(1500);
			complete_revise = false;
			break;
		}
	}
}

void logout() {
	send(client_sock, "5", 1, 0);
}



int main(/*int argc, char *argv[]*/)
{
	WSADATA wsa;

	// Winsock를 초기화하는 함수. MAKEWORD(2, 2)는 Winsock의 2.2 버전을 사용하겠다는 의미.
	// 실행에 성공하면 0을, 실패하면 그 이외의 값을 반환.
	// 0을 반환했다는 것은 Winsock을 사용할 준비가 되었다는 의미.
	int code = WSAStartup(MAKEWORD(2, 2), &wsa);



	if (!code) {
		client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // 

		// 연결할 서버 정보 설정 부분
		SOCKADDR_IN client_addr = {};
		client_addr.sin_family = AF_INET;
		client_addr.sin_port = htons(7777);
		InetPton(AF_INET, TEXT("192.168.0.28"), &client_addr.sin_addr); //192.168.0.63   192.168.195.243

		while (1) {
			if (!connect(client_sock, (SOCKADDR*)&client_addr, sizeof(client_addr))) { // 위에 설정한 정보에 해당하는 server로 연결!
				/*cout << "Server Connect" << endl;*/
				break;
			}
			cout << "Connecting..." << endl;
		}

		while (1) {

			int menu_num = startMenu();
			/*a = atoi(argv[1]);*/
			if (menu_num == 0) {
				system("cls");
				//string id = "";
				//string pw = "";
				login();
				system("cls");
				int main_num = mainMenu();


				if (main_num == 0) {
					system("cls");
					send(client_sock, "1", strlen("1"), 0);
					std::thread th2(chat_recv);
					cout << "채팅이 시작되었습니다.";
					cout << "\n";

					while (1) {
						string text;
						std::getline(cin, text);
						/*cin >> text;*/
						const char* buffer = text.c_str(); // string형을 char* 타입으로 변환

						send(client_sock, buffer, strlen(buffer), 0);

					}
					closesocket(client_sock);
					th2.join();
				}

				else if (main_num == 1) {
					system("cls");
					withdrawal();
					system("cls");
				}
				else if (main_num == 2) {
					system("cls");
					revise();
					system("cls");
				}
				else if (main_num == 3) {
					system("cls");
					logout();
				}
			}
			else if (menu_num == 1) {
				system("cls");
				sign_up();
				system("cls");

			}
			else if (menu_num == 2) {
				system("cls");
				break;
			}

		}
		WSACleanup();
		return 0;
	}
}