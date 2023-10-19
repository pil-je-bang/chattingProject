#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h> //Winsock 헤더파일 include. WSADATA 들어있음.
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <mysql/jdbc.h>
#include <thread>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <regex>

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

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "(()()&pj0907"; // 데이터베이스 접속 비밀번호


int chat_recv() {
	char buf[MAX_SIZE] = { };
	string msg;

	while (1) {
		ZeroMemory(&buf, MAX_SIZE);
		if (recv(client_sock, buf, MAX_SIZE, 0) > 0) {
			msg = buf;
			cout << "버퍼" << buf;
			std::stringstream ss(msg);  // 문자열을 스트림화
			string user;
			ss >> user; // 스트림을 통해, 문자열을 공백 분리해 변수에 할당. 보낸 사람의 이름만 user에 저장됨.
			if (user != my_nick) {
				cout << buf << endl; // 내가 보낸 게 아닐 경우에만 출력하도록.
			}
		}
		else {
			cout << "Server Off" << endl;
			return -1;
		}
	}
}

string get_time()
{
	using namespace std::chrono;
	system_clock::time_point tp = system_clock::now();
	std::stringstream str;
	__time64_t t1 = system_clock::to_time_t(tp);
	system_clock::time_point t2 = system_clock::from_time_t(t1);
	if (t2 > tp)
		t1 = system_clock::to_time_t(tp - seconds(1));

	tm tm{};
	localtime_s(&tm, &t1);

	str << std::put_time(&tm, "[%Y-%m-%d %H:%M:%S]") << std::setfill('0') << std::setw(3)
		<< (std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count() % 1000);

	return str.str();
}

string sign_up() {
	try {
		driver = sql::mysql::get_mysql_driver_instance();
		con = driver->connect(server, username, password);
	}
	catch (sql::SQLException& e) {
		cout << "Could not connect to server. Error message: " << e.what() << endl;
		exit(1);
	}
	// 데이터베이스 선택
	con->setSchema("kdt");

	// db 한글 저장을 위한 셋팅 
	stmt = con->createStatement();
	stmt->execute("set names euckr");
	if (stmt) { delete stmt; stmt = nullptr; }

	vector<string> user_info = { "아이디","이름","비밀번호(영어,숫자,특수문자 조합)","birth(yyyy-mm-dd)","연락처 (010-xxxx-xxxx)","email","address" };
	int specialList[12] = { '!','@','#','$','%','^','&','*','(',')','-','+' };
	bool numberCheck = false;  // 숫자 check
	bool englishCheck = false; // 영어 check
	bool specialCheck = false; // 특수 check
	bool is_there_same = 1;
	string in;



	while (is_there_same) {
		in = "";
		cout << "============회원가입==============" << endl;
		cout << "아이디 :";
		getline(cin, in);
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
			stmt = con->createStatement();

			res = stmt->executeQuery("SELECT id FROM user_info");
			while (res->next()) {
				std::string id = res->getString(1);
				if (in == id) {
					cout << "이미 존재하는 아이디 입니다." << endl;
					Sleep(500);
					is_there_same = true;
					break;
				}
				else {
					is_there_same = false;
				}

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

	user_info[0] = in;
	string input = "";
	int y = 0;

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
				else
					user_info[i] = input;
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
				}
			}
		}
		else {
			cout << user_info[i] << ": ";
			cin >> input;
			user_info[i] = input;
		}
	}

	//stmt->execute("DROP TABLE IF EXISTS chatting");// 채팅 시작 할때 생성해야 됨
	//stmt->execute("CREATE TABLE chatting (id VARCHAR(50), 내용 VARCHAR(255));"); // CREATE
	//delete stmt;

	//stmt->execute("DROP TABLE IF EXISTS user_info");// 채팅 시작 할때 생성해야 됨
	//stmt->execute("CREATE TABLE user_info (id VARCHAR(50), name VARCHAR(50), pw VARCHAR(50), birth date, num VARCHAR(50), email VARCHAR(50), address VARCHAR(50));"); // CREATE
	
	pstmt = con->prepareStatement("INSERT INTO user_info(id,name,pw,birth,num,email,address) VALUES(?,?,?,?,?,?,?)"); // INSERT
	for (int i = 0; i < 7; i++) {
		pstmt->setString(i + 1, user_info[i]);
	}
	pstmt->execute();
	delete pstmt;
	cout << "가입완료" << endl;
	return in;
}

string login(/*string input_id, string input_pw*/) {
	string input_id = "";
	string input_pw = "";
	bool login_success = false;

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


		// 서버에 로그인 정보 전송
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
			cout << "로그인 실패" << std::endl;
			login_success == false;
		}
	}
	return input_id;
}

string withdrawal() {
	string input_id = "";
	string input_pw = "";
	bool login = false;
	bool pass = false;
	bool withdrawal = false;


	while (1) {
		cout << "==========회원탈퇴===========" << endl;
		cout << "id:";
		cin >> input_id;
		cout << "pw:";
		cin >> input_pw;

		try {
			driver = sql::mysql::get_mysql_driver_instance();
			con = driver->connect(server, username, password);
		}
		catch (sql::SQLException& e) {
			cout << "Could not connect to server. Error message: " << e.what() << endl;
			exit(1);
		}

		// 데이터베이스 선택
		con->setSchema("kdt");

		// db 한글 저장을 위한 셋팅 
		stmt = con->createStatement();
		stmt->execute("set names euckr");
		if (stmt) { delete stmt; stmt = nullptr; }

		// 데이터베이스 쿼리 실행
		stmt = con->createStatement();

		res = stmt->executeQuery("SELECT id FROM user_info");
		while (res->next() == true) {
			std::string id = res->getString("id");
			if (input_id == id) { login = true; }
		}

		res = stmt->executeQuery("SELECT pw FROM user_info where id =\"" + input_id + "\"");
		while (res->next() == true) {
			std::string pw = res->getString("pw");
			if (input_pw == pw) { pass = true; }
		}



		if (login && pass) {
			while (1) {
				cout << "정말 회원 탈퇴를 하시겠습니까?" << endl;
				cout << "탈퇴를 원하시면 Yes를 입력하시고 아니면 No를 입력하세요";
				string withdrawalYN;
				cin >> withdrawalYN;
				if (withdrawalYN == "yes") {
					string newId = "알수없음";

					cout << "그동안 이용해주셔서 감사합니다." << endl;

					pstmt = con->prepareStatement("UPDATE chatting SET id = ? WHERE id = ?");
					pstmt->setString(1, newId);     // newId는 변경하려는 새로운 값
					pstmt->setString(2, input_id); // input_id는 변경하려는 특정 id
					pstmt->executeUpdate();

					pstmt = con->prepareStatement("DELETE FROM user_info WHERE id = ?");
					pstmt->setString(1, input_id);
					pstmt->executeUpdate();
					cout << "회원 탈퇴가 완료되었습니다.";
					withdrawal == true;
					break;
				}
				else if (withdrawalYN == "no") {
					cout << "다시 돌아오신걸 환영합니다.";
					break;
				}
				else {
					continue;
				}
			}
			break;
		}


		else {
			cout << "회원정보가 맞지 않습니다."<<endl;
		}
	}
	delete stmt;
	// MySQL Connector/C++ 정리
	delete res;
	delete con;

}

void beforechatting() {
	pstmt = con->prepareStatement("SELECT id, 내용 FROM chatting");
	res = pstmt->executeQuery();
	vector<string> id;
	vector<string> chatting;
	int i = 0;
	while(res->next()) {
		id.push_back(res->getString(1));
		chatting.push_back(res->getString(2));
	}
	for (int i = 0; i < id.size(); i++) {
		cout << id.at(i) << " : " << chatting.at(i)<<endl;
	}
}

void revise() {
	bool complete_revise = true;
	while (complete_revise) {
		string input_pw;
		cout << "비밀번호를 입력하세요 : ";
		cin >> input_pw;

		bool passwordMatch = false;

		try {
			driver = sql::mysql::get_mysql_driver_instance();
			con = driver->connect(server, username, password);
		}
		catch (sql::SQLException& e) {
			cout << "Could not connect to server. Error message: " << e.what() << endl;
			exit(1);
		}

		// 데이터베이스 선택
		con->setSchema("kdt");

		// db 한글 저장을 위한 셋팅 
		stmt = con->createStatement();
		stmt->execute("set names euckr");
		if (stmt) { delete stmt; stmt = nullptr; }

		// 데이터베이스 쿼리 실행
		stmt = con->createStatement();

		res = stmt->executeQuery("SELECT pw FROM user_info");
		while (res->next() == true) {
			std::string pw = res->getString("pw");
			if (input_pw == pw) {
				int revise_number;
				passwordMatch = true;

				cout << "수정할 정보를 고르세요." << endl;
				cout << "1. name  2. pw  3. birth  4. num  5. email  6. address" << endl;
				cin >> revise_number;
				if (revise_number == 1) {
					string revise_name;
					cout << "변경된 이름을 입력하세요 : ";
					cin >> revise_name;
					pstmt = con->prepareStatement("UPDATE user_info SET name = ? WHERE pw = ?");
					pstmt->setString(1, revise_name);
					pstmt->setString(2, input_pw);
					pstmt->executeUpdate();
					cout << "변경이 완료되었습니다.";
					complete_revise = false;
					cout << complete_revise;
					break;

				}
				else if (revise_number == 2) {
					string revise_pw;
					cout << "변경된 pw을 입력하세요 : ";
					cin >> revise_pw;
					pstmt = con->prepareStatement("UPDATE user_info SET pw = ? WHERE pw = ?");
					pstmt->setString(1, pw);
					pstmt->setString(2, input_pw);
					pstmt->executeUpdate();
					cout << "변경이 완료되었습니다.";
					break;
				}
				else if (revise_number == 3) {
					string revise_birth;
					cout << "변경된 birth을 입력하세요 : ";
					cin >> revise_birth;
					pstmt = con->prepareStatement("UPDATE user_info SET birth = ? WHERE pw = ?");
					pstmt->setString(1, revise_birth);
					pstmt->setString(2, input_pw);
					pstmt->executeUpdate();
					cout << "변경이 완료되었습니다.";
					break;
				}
				else if (revise_number == 4) {
					string revise_num;
					cout << "변경된 num을 입력하세요 : ";
					cin >> revise_num;
					pstmt = con->prepareStatement("UPDATE user_info SET num = ? WHERE pw = ?");
					pstmt->setString(1, revise_num);
					pstmt->setString(2, input_pw);
					pstmt->executeUpdate();
					cout << "변경이 완료되었습니다.";
					break;
				}
				else if (revise_number == 5) {
					string revise_email;
					cout << "변경된 email을 입력하세요 : ";
					cin >> revise_email;
					pstmt = con->prepareStatement("UPDATE user_info SET email = ? WHERE pw = ?");
					pstmt->setString(1, revise_email);
					pstmt->setString(2, input_pw);
					pstmt->executeUpdate();
					cout << "변경이 완료되었습니다.";
					break;
				}
				else if (revise_number == 6) {
					string revise_address;
					cout << "변경된 address을 입력하세요 : ";
					cin >> revise_address;
					pstmt = con->prepareStatement("UPDATE user_info SET address = ? WHERE pw = ?");
					pstmt->setString(1, revise_address);
					pstmt->setString(2, input_pw);
					pstmt->executeUpdate();
					cout << "변경이 완료되었습니다.";
					break;
				}
				else {
					cout << "올바른 숫자를 입력하세요" << endl;

				}
				break;
			}

		}
		if (!passwordMatch) {
			cout << "비밀번호가 틀립니다." << endl;
		}
	}

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
		InetPton(AF_INET, TEXT("192.168.0.63"), &client_addr.sin_addr); //192.168.0.63   192.168.195.243

		while (1) {
			if (!connect(client_sock, (SOCKADDR*)&client_addr, sizeof(client_addr))) { // 위에 설정한 정보에 해당하는 server로 연결!
				cout << "Server Connect" << endl;
				break;
			}
			cout << "Connecting..." << endl;
		}

	cout << "1번 로그인,  2번 회원가입,  3번 회원탈퇴, 4번 회원정보수정 ";
	int a;
	cin >> a;

	/*a = atoi(argv[1]);*/
	if (a == 1) {
		//string id = "";
		//string pw = "";
		login();
		/*beforechatting();*/
	}
	else if (a == 2) {
		my_nick = sign_up();
	}
	else if (a == 3) {
		withdrawal();
	}
	else if (a == 4) {
		revise();
	}


		std::thread th2(chat_recv);
		cout << "채팅이 시작되었습니다.";
		cout << "\n";

		while (1) {
			string text;
			/*std::getline(cin, text);*/
			cin >> text;
			text += get_time();
			const char* buffer = text.c_str(); // string형을 char* 타입으로 변환
			send(client_sock, buffer, strlen(buffer), 0);
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