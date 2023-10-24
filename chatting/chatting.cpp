#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h> //Winsock 헤더파일 include. WSADATA 들어있음.
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
	COORD Pos;  //x, y를 가지고 있는 구조체
	Pos.X = x;  //x의 움직이는 범위
	Pos.Y = z + 2 * y;//z=24 초기값
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

int FirstMenu(int z, int j) {
	int Menu = 0;
	int Count = 0;
	int Direction;
	for (;;) {
		GoToXY(10, Count, z);
		if (1) {        //키보드 입력 확인 (true / false)
			Direction = _getch();      // 방향키 입력시 224 00이 들어오게 되기에 앞에 있는 값 224를 없앰
			if (Direction == 224)
				Direction = _getch();  // 새로 입력값을 판별하여 상하좌우 출력
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
	return FirstMenu(13, 3);
}

string FindSubstr(string const& str, int n) {
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
			std::stringstream ss(Msg);  // 문자열을 스트림화
			string User;
			ss >> User; // 스트림을 통해, 문자열을 공백 분리해 변수에 할당. 보낸 사람의 이름만 user에 저장됨.
			if (User != MyNick) {
				if (FindSubstr(ChatBuf, 3) == "/dm") {
					string Dm = Msg.substr(Msg.find("m")+1);
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
					cout << Dm << endl; // 내가 보낸 게 아닐 경우에만 출력하도록..
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
				}
				else {
					cout << ChatBuf << endl; // 내가 보낸 게 아닐 경우에만 출력하도록.
				}
			}
		}
		else {
			cout << "Server Off" << endl;
			return -1;
		}
	}
}
void SignUp() {
	send(ClientSock, "2", 1, 0);
	vector<string> UserInfo = { "아이디","이름","비밀번호(영어,숫자,특수문자 조합)","birth(yyyy-mm-dd)","연락처 (010-xxxx-xxxx)","email","address" };
	
	int SpecialList[12] = { '!','@','#','$','%','^','&','*','(',')','-','+'};
	bool NumberCheck = false;  // 숫자 check
	bool EnglishCheck = false; // 영어 check
	bool SpecialCheck = false; // 특수 check
	bool Id = true;
	


	while (Id) {
		char SignUpBuf[MAX_SIZE] = {};
		string InputId = "";
		cout << "============회원가입==============" << endl;
		cout << "아이디 :";
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
			}
			else {
				Id = false;
				Sleep(1500);
			}
		}
		else
		{

			cout << "숫자와 영어의 조합으로 id를 생성하세요." << endl;
			NumberCheck = false;  // 숫자 check
			EnglishCheck = false; // 영어 check
			SpecialCheck = false; // 특수 check

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
					cout << "비밀번호는 [영어,숫자,특수문자]로 이루어져야 합니다." << endl;
					NumberCheck = false;  // 숫자 check
					EnglishCheck = false; // 영어 check
					SpecialCheck = false; // 특수 check
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
					cout << "형식이 맞지 않습니다." << endl;
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
					cout << "형식이 맞지 않습니다." << endl;
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
					cout << "형식이 맞지 않습니다." << endl;
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

	cout << "회원가입이 완료되었습니다.";
  
  //stmt->execute("DROP TABLE IF EXISTS chatting");// 채팅 시작 할때 생성해야 됨
	//stmt->execute("CREATE TABLE chatting (id VARCHAR(50), 내용 VARCHAR(255));"); // CREATE
	//delete stmt;

	//stmt->execute("DROP TABLE IF EXISTS user_info");// 채팅 시작 할때 생성해야 됨
	//stmt->execute("CREATE TABLE user_info (id VARCHAR(50), name VARCHAR(50), pw VARCHAR(50), birth date, num VARCHAR(50), email VARCHAR(50), address VARCHAR(50));"); // CREATE
	Sleep(1500);
}

void LogIn() { //로그인
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

		// 서버에 로그인 정보 전송
		send(ClientSock, Login.c_str(), Login.length(), 0);

		// 서버로부터 로그인 결과 받기
		char LoginBuf[MAX_SIZE] = { };
		recv(ClientSock, LoginBuf, MAX_SIZE, 0);

		// 결과 출력
		if (strcmp(LoginBuf, "true") == 0) {
			cout << "로그인 성공" << std::endl;
			LoginSuccess == true;
			break;
		}
		else {
			cout << "로그인 실패" << std::endl;
			LoginSuccess == false;
		}
	}
}

void Withdrawal() { //회원탈퇴
	string InputId = "";
	string InputPw = "";
	send(ClientSock, "3", 1, 0);

	while (1) {
		cout << "==========회원탈퇴===========" << endl;
		cout << "id:";
		cin >> InputId;
		cout << "pw:";
		cin >> InputPw;
		string WithdrawalInfo;
		WithdrawalInfo = InputId + "-" + InputPw;

		send(ClientSock, WithdrawalInfo.c_str(), WithdrawalInfo.length(), 0);
		char WithdrawalBuf[MAX_SIZE] = { };
		recv(ClientSock, WithdrawalBuf, MAX_SIZE, 0);

		// 결과 출력
		if (strcmp(WithdrawalBuf, "true") == 0) {
			while (1) {
				cout << "정말 회원 탈퇴를 하시겠습니까?" << endl;
				cout << "탈퇴를 원하시면 yes를 입력하시고 아니면 no를 입력하세요";
				string WithdrawalYN;
				cin >> WithdrawalYN;
				send(ClientSock, WithdrawalYN.c_str(), WithdrawalYN.length(), 0);

				char WithdrawalBuf2[MAX_SIZE] = { };
				recv(ClientSock, WithdrawalBuf2, MAX_SIZE, 0);

				if (strcmp(WithdrawalBuf2, "true") == 0) {
					cout << "그동안 이용해주셔서 감사합니다." << endl;
					cout << "회원 탈퇴가 완료되었습니다." << endl;
					Sleep(1500);
					break;
				}
				else if (strcmp(WithdrawalBuf2, "false") == 0) {
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

void Revise() { //정보수정하기
	send(ClientSock, "4", 1, 0);
	bool CompleteRevise = true;
	char ReviseBuf[MAX_SIZE] = { };

	while (CompleteRevise) {
		string ReviseNumber;
		string InputPw;
		cout << "비밀번호를 입력하세요 : ";
		cin >> InputPw;
		string ReviseInfo;
		string ReviseInformation;

		send(ClientSock, InputPw.c_str(), InputPw.length(), 0);
		recv(ClientSock, ReviseBuf, MAX_SIZE, 0); // 비번 맞으면 true, 틀리면 false

		if (strcmp(ReviseBuf, "true") == 0) {
			cout << "수정할 정보를 고르세요." << endl;
			cout << "1. name  2. pw  3. birth  4. num  5. email  6. address" << endl;
			cin >> ReviseNumber;
		}


		if (ReviseNumber == "1") {
			cout << "변경된 이름을 입력하세요 : ";
			cin >> ReviseInformation;
			ReviseInfo = ReviseNumber + "-" + ReviseInformation;
		}
		else if (ReviseNumber == "2") {
			cout << "변경된 pw를 입력하세요 : ";
			cin >> ReviseInformation;
			ReviseInfo = ReviseNumber + "-" + ReviseInformation;
		}
		else if (ReviseNumber == "3") {
			cout << "변경된 birth를 입력하세요 : ";
			cin >> ReviseInformation;
			ReviseInfo = ReviseNumber + "-" + ReviseInformation;
		}
		else if (ReviseNumber == "4") {
			cout << "변경된 number를 입력하세요 : ";
			cin >> ReviseInformation;
			ReviseInfo = ReviseNumber + "-" + ReviseInformation;
		}
		else if (ReviseNumber == "5") {
			cout << "변경된 email를 입력하세요 : ";
			cin >> ReviseInformation;
			ReviseInfo = ReviseNumber + "-" + ReviseInformation;
		}
		else if (ReviseNumber == "6") {
			cout << "변경된 address를 입력하세요 : ";
			cin >> ReviseInformation;
			ReviseInfo = ReviseNumber + "-" + ReviseInformation;
    }

		send(ClientSock, ReviseInfo.c_str(), ReviseInfo.length(), 0);
		char ReviseBuf2[MAX_SIZE] = { };
		recv(ClientSock, ReviseBuf2, MAX_SIZE, 0);


		if (strcmp(ReviseBuf2, "true") == 0) {
			cout << "변경이 완료되었습니다.";
			Sleep(1500);
			CompleteRevise = false;
			break;
		}
	}
}

void LogOut() { //LogOut
	send(ClientSock, "5", 1, 0);
}



int main()
{
	WSADATA wsa;

	// Winsock를 초기화하는 함수. MAKEWORD(2, 2)는 Winsock의 2.2 버전을 사용하겠다는 의미.
	// 실행에 성공하면 0을, 실패하면 그 이외의 값을 반환.
	// 0을 반환했다는 것은 Winsock을 사용할 준비가 되었다는 의미.
	int code = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (!code) {
		ClientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

		// 연결할 서버 정보 설정 부분
		SOCKADDR_IN ClientAddr = {};
		ClientAddr.sin_family = AF_INET;
		ClientAddr.sin_port = htons(7777);
		InetPton(AF_INET, TEXT("192.168.0.63"), &ClientAddr.sin_addr); //192.168.0.63   192.168.195.243

		while (1) {
			if (!connect(ClientSock, (SOCKADDR*)&ClientAddr, sizeof(ClientAddr))) { // 위에 설정한 정보에 해당하는 server로 연결!
				break;
			}
			cout << "Connecting..." << endl;
		}

		while (1) {
			int MenuNum = StartMenu(); //startMenu

			if (MenuNum == 0) {
				system("cls");
				LogIn();
				system("cls");
				int MainNum = MainMenu(); //mainMenu

				if (MainNum == 0) { 
					system("cls");
					send(ClientSock, "1", strlen("1"), 0);
					std::thread ChatThread(ChatRecv);
					cout << "채팅이 시작되었습니다.";
					cout << "\n";

					while (1) {
						string Text;
						std::getline(cin, Text);
						const char* ChatBuffer = Text.c_str(); 
						send(ClientSock, ChatBuffer, strlen(ChatBuffer), 0);
					}
					ChatThread.join();
					closesocket(ClientSock);
				}
				else if (MainNum == 1) {
					system("cls");
					Withdrawal();
					system("cls");
				}
				else if (MainNum == 2) {
					system("cls");
					Revise();
					system("cls");
				}
				else if (MainNum == 3) {
					system("cls");
					LogOut();
				}
			}
			else if (MenuNum == 1) {
				system("cls");
				SignUp();
				system("cls");
			}
			else if (MenuNum == 2) {
				system("cls");
				break;
			}
		}
		WSACleanup();
		return 0;
	}
}
