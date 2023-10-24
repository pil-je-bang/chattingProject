#pragma comment(lib, "ws2_32.lib") //명시적인 라이브러리의 링크. 윈속 라이브러리 참조

#include <WinSock2.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <mysql/jdbc.h>
#include <thread>
#include <vector>
#include <sstream>
#include <chrono>
#include <iomanip>

#define MAX_SIZE 1024
#define MAX_CLIENT 3
using std::cout;
using std::cin;
using std::endl;
using std::string;

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "(()()&pj0907"; // 데이터베이스 접속 비밀번호



struct SOCKET_INFO { // 연결된 소켓 정보에 대한 틀 생성
    SOCKET sck;
    string user;
};



// MySQL Connector/C++ 초기화
sql::mysql::MySQL_Driver* driver; // 추후 해제하지 않아도 Connector/C++가 자동으로 해제해 줌
sql::Connection* con;
sql::Statement* stmt;
sql::PreparedStatement* pstmt;
sql::ResultSet* res;

std::vector<SOCKET_INFO> sck_list; // 연결된 클라이언트 소켓들을 저장할 배열 선언.
SOCKET_INFO server_sock; // 서버 소켓에 대한 정보를 저장할 변수 선언.
SOCKET_INFO server_sock1;
int client_count = 0; // 현재 접속해 있는 클라이언트를 count 할 변수 선언.

void server_init(); // socket 초기화 함수. socket(), bind(), listen() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void add_client(); // 소켓에 연결을 시도하는 client를 추가(accept)하는 함수. client accept() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void send_msg(const char* msg); // send() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void recv_msg(int idx); // recv() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void del_client(int idx); // 소켓에 연결되어 있는 client를 제거하는 함수. closesocket() 실행됨. 자세한 내용은 함수 구현부에서 확인.
string get_time(); //시간

int main() {
    WSADATA wsa;

    // Winsock를 초기화하는 함수. MAKEWORD(2, 2)는 Winsock의 2.2 버전을 사용하겠다는 의미.
    // 실행에 성공하면 0을, 실패하면 그 이외의 값을 반환.
    // 0을 반환했다는 것은 Winsock을 사용할 준비가 되었다는 의미.
    int code = WSAStartup(MAKEWORD(2, 2), &wsa);

    if (!code) {
        server_init();

        std::thread th1[MAX_CLIENT];
        for (int i = 0; i < MAX_CLIENT; i++) {
            // 인원 수 만큼 thread 생성해서 각각의 클라이언트가 동시에 소통할 수 있도록 함.
            th1[i] = std::thread(add_client);
        }
        //std::thread th1(add_client); // 이렇게 하면 하나의 client만 받아짐...


        while (1) { // 무한 반복문을 사용하여 서버가 계속해서 채팅 보낼 수 있는 상태를 만들어 줌. 반복문을 사용하지 않으면 한 번만 보낼 수 있음.
            string text, msg = "";
            std::getline(cin, text);
            const char* buf = text.c_str();
            msg = server_sock.user + " : " + buf;
            send_msg(msg.c_str());
        }

        for (int i = 0; i < MAX_CLIENT; i++) {
            th1[i].join();
            //join : 해당하는 thread 들이 실행을 종료하면 리턴하는 함수.
            //join 함수가 없으면 main 함수가 먼저 종료되어서 thread가 소멸하게 됨.
            //thread 작업이 끝날 때까지 main 함수가 끝나지 않도록 해줌.
        }
        //th1.join();

        closesocket(server_sock.sck);
    }
    else {
        cout << "프로그램 종료. (Error code : " << code << ")";
    }

    WSACleanup();

    return 0;
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

    str << std::put_time(&tm, "[%Y-%m-%d %H:%M:%S]");

    return str.str();
}

void server_init() {
    server_sock.sck = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Internet의 Stream 방식으로 소켓 생성
   // SOCKET_INFO의 소켓 객체에 socket 함수 반환값(디스크립터 저장)
   // 인터넷 주소체계, 연결지향, TCP 프로토콜 쓸 것. 

    SOCKADDR_IN server_addr = {}; // 소켓 주소 설정 변수
    // 인터넷 소켓 주소체계 server_addr

    server_addr.sin_family = AF_INET; // 소켓은 Internet 타입 
    server_addr.sin_port = htons(7777); // 서버 포트 설정
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 서버이기 때문에 local 설정한다. 
    //Any인 경우는 호스트를 127.0.0.1로 잡아도 되고 localhost로 잡아도 되고 양쪽 다 허용하게 할 수 있따. 그것이 INADDR_ANY이다.
    //ip 주소를 저장할 땐 server_addr.sin_addr.s_addr -- 정해진 모양?

    bind(server_sock.sck, (sockaddr*)&server_addr, sizeof(server_addr)); // 설정된 소켓 정보를 소켓에 바인딩한다.
    listen(server_sock.sck, SOMAXCONN); // 소켓을 대기 상태로 기다린다.
    server_sock.user = "server";

    cout << "Server On" << endl;

}


void add_client() {
    
    string input_id, input_pw;
    SOCKET_INFO new_client = {};
    SOCKADDR_IN addr = {};
    int addrsize = sizeof(addr);
    new_client.sck = accept(server_sock.sck, (sockaddr*)&addr, &addrsize);
    ZeroMemory(&addr, addrsize); // addr의 메모리 영역을 0으로 초기화

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

    while (1) {
        bool ID = false;
        bool PW = false;
        char buf[MAX_SIZE] = { };
        char buf2[MAX_SIZE] = { };
        recv(new_client.sck, buf, MAX_SIZE, 0);

        if (strcmp(buf, "1") == 0) { // 로그인
            while (1) {
                char buf[MAX_SIZE] = { };
                recv(new_client.sck, buf, MAX_SIZE, 0);
                new_client.user = string(buf);
                input_id = new_client.user.substr(0, new_client.user.find("-"));
                input_pw = new_client.user.substr(new_client.user.find("-") + 1);
                new_client.user = input_id;
                stmt = con->createStatement();

                res = stmt->executeQuery("SELECT id FROM user_info");
                while (res->next() == true) {
                    std::string id = res->getString("id");
                    if (input_id == id) { ID = true; }
                }

                stmt = con->createStatement();
                res = stmt->executeQuery("SELECT pw FROM user_info where id =\"" + input_id + "\"");
                while (res->next() == true) {
                    std::string pw = res->getString("pw");
                    if (input_pw == pw) { PW = true; }
                }

                if (ID && PW) {
                    send(new_client.sck, "true", strlen("true"), 0);
                    sck_list.push_back(new_client);
                    break;
                }
                else {
                    send(new_client.sck, "false", strlen("false"), 0);
                }

            }
            recv(new_client.sck, buf2, sizeof("1"), 0); //채팅시작
            if (strcmp(buf2, "1") == 0) {
                string msg = "[공지] " + input_id + " 님이 입장했습니다.";
                cout << msg << endl;


                std::thread th(recv_msg, client_count);
                // 다른 사람들로부터 오는 메시지를 계속해서 받을 수 있는 상태로 만들어 두기.

                client_count++; // client 수 증가.
                cout << "[공지] 현재 접속자 수 : " << client_count << "명" << endl;
                send_msg(msg.c_str()); // c_str : string 타입을 const chqr* 타입으로 바꿔줌.
                th.join();
            }

            else if ((strcmp(buf2, "3") == 0)) {
                char buf4[MAX_SIZE] = {};
                while (1) {
                    bool ID = false;
                    bool PW = false;
                    recv(new_client.sck, buf4, MAX_SIZE, 0);
                    new_client.user = string(buf4);
                    input_id = new_client.user.substr(0, new_client.user.find("-"));
                    input_pw = new_client.user.substr(new_client.user.find("-") + 1);

                    // 데이터베이스 쿼리 실행
                    stmt = con->createStatement();
                    res = stmt->executeQuery("SELECT id FROM user_info");
                    while (res->next() == true) {
                        std::string id = res->getString("id");
                        if (input_id == id) {
                            ID = true;
                        }
                    }
                    res = stmt->executeQuery("SELECT pw FROM user_info where id =\"" + input_id + "\"");
                    while (res->next() == true) {
                        std::string pw = res->getString("pw");
                        if (input_pw == pw) { 
                            PW = true;
                        }
                    }

                    if (ID && PW) {
                        send(new_client.sck, "true", strlen("true"), 0);
                        /*sck_list.push_back(new_client);*/
                        break;
                    }
                    else {
                        send(new_client.sck, "false", strlen("false"), 0);
                    }
                }
                char buf2[MAX_SIZE] = {};
                recv(new_client.sck, buf2, MAX_SIZE, 0);

                if (strcmp(buf2, "yes") == 0) {
                    string newId = "알수없음";

                    pstmt = con->prepareStatement("UPDATE chatting SET id = ? WHERE id = ?");
                    pstmt->setString(1, newId);     // newId는 변경하려는 새로운 값
                    pstmt->setString(2, input_id); // input_id는 변경하려는 특정 id
                    pstmt->executeUpdate();

                    pstmt = con->prepareStatement("DELETE FROM user_info WHERE id = ?");
                    pstmt->setString(1, input_id);
                    pstmt->executeUpdate();

                    send(new_client.sck, "true", strlen("true"), 0);
                }
                else if (strcmp(buf2, "no") == 0) {
                    send(new_client.sck, "false", strlen("false"), 0);
                }
                else {
                    send(new_client.sck, "잘못입력", strlen("잘못입력"), 0);
                }

            }
            else if ((strcmp(buf2, "4") == 0)) {  //정보수정
                char buf3[MAX_SIZE] = { };
                recv(new_client.sck, buf3, MAX_SIZE, 0);// 비번 들고옴
                cout << buf3;
                stmt = con->createStatement();
                res = stmt->executeQuery("SELECT pw FROM user_info");
                while (res->next() == true) {
                    std::string pw = res->getString("pw");
                    if (strcmp(buf3, pw.c_str()) == 0) {
                        send(new_client.sck, "true", strlen("true"), 0);
                        break;
                    }
                }
                char buf1[MAX_SIZE] = { };
                recv(new_client.sck, buf1, MAX_SIZE, 0);

                string revise_num, revise_info, revise_col;
                new_client.user = string(buf1);
                revise_num = new_client.user.substr(0, new_client.user.find("-"));
                revise_info = new_client.user.substr(new_client.user.find("-") + 1);
                cout << revise_num << "+" << revise_info;

                if (revise_num == "1") {
                    revise_col = "name";
                }
                else if (revise_num == "2") {
                    revise_col = "pw";
                }
                else if (revise_num == "3") {
                    revise_col = "birth";
                }
                else if (revise_num == "4") {
                    revise_col = "num";
                }
                else if (revise_num == "5") {
                    revise_col = "email";
                }
                else if (revise_num == "6") {
                    revise_col = "address";
                }

                pstmt = con->prepareStatement("UPDATE user_info SET " + revise_col + " = ? WHERE pw = ? ");
                pstmt->setString(1, revise_info);
                pstmt->setString(2, buf3);
                pstmt->executeUpdate();
                send(new_client.sck, "true", strlen("true"), 0);

            }
            else if (strcmp(buf, "5") == 0) {
                while (1) {
                    char buf[MAX_SIZE] = { };
                    recv(new_client.sck, buf, MAX_SIZE, 0);
                    new_client.user = string(buf);
                    input_id = new_client.user.substr(0, new_client.user.find("-"));
                    input_pw = new_client.user.substr(new_client.user.find("-") + 1);
                    new_client.user = input_id;
                    stmt = con->createStatement();

                    res = stmt->executeQuery("SELECT id FROM user_info");
                    while (res->next() == true) {
                        std::string id = res->getString("id");
                        if (input_id == id) { ID = true; }
                    }

                    stmt = con->createStatement();
                    res = stmt->executeQuery("SELECT pw FROM user_info where id =\"" + input_id + "\"");
                    while (res->next() == true) {
                        std::string pw = res->getString("pw");
                        if (input_pw == pw) { PW = true; }
                    }

                    if (ID && PW) {
                        send(new_client.sck, "true", strlen("true"), 0);
                        sck_list.push_back(new_client);
                        break;
                    }
                    else {
                        send(new_client.sck, "false", strlen("false"), 0);
                    }

                }

            }
        }
        else if (strcmp(buf, "2") == 0) {
            std::vector<string> user_info = { "아이디","이름","비밀번호(영어,숫자,특수문자 조합)","birth(yyyy-mm-dd)","연락처 (010-xxxx-xxxx)","email","address" };


            for (int i = 0; i < 7; i++) {
                char buf[MAX_SIZE] = { };
                recv(new_client.sck, buf, sizeof(buf), 0);
                if (i == 0) {
                    bool t = true;
                    stmt = con->createStatement();
                    res = stmt->executeQuery("SELECT id FROM user_info");
                    while (res->next() == true) {
                        std::string id = res->getString("id");
                        if (buf == id) {
                            t = false;
                            send(new_client.sck, "false", sizeof("false"), 0);
                            i--;
                            break;
                        }
                    }
                    if (t)
                        send(new_client.sck, "true", sizeof("true"), 0);
                }
                if (i >= 0) {
                    user_info[i] = buf;
                }
            }
            pstmt = con->prepareStatement("INSERT INTO user_info(id,name,pw,birth,num,email,address) VALUES(?,?,?,?,?,?,?)"); // INSERT
            for (int i = 0; i < 7; i++) {
                pstmt->setString(i + 1, user_info[i]);
            }
            pstmt->execute();
            delete pstmt;
        }
    }     
}

void send_msg(const char* msg) {
    for (int i = 0; i < client_count; i++) { // 접속해 있는 모든 client에게 메시지 전송
        send(sck_list[i].sck, msg, MAX_SIZE, 0);
    }
}
void send_dm_msg(const char* msg, int i, int index) { //dm
    string q = "/dm" + sck_list[index].user + "의 귓속말:";
    send(sck_list[i].sck, (q + msg).c_str(), MAX_SIZE, 0);
}

string findsubstr(string const& str, int n) {
    if (str.length() < n) {
        return str;
    }
    return str.substr(0, n);
}

void recv_msg(int idx) {
    char buf[MAX_SIZE] = { };
    string msg = "";
    string msg_time = get_time();

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

    ////cout << sck_list[idx].user << endl;

    while (1) {
        ZeroMemory(&buf, MAX_SIZE);
        if (recv(sck_list[idx].sck, buf, MAX_SIZE, 0) > 0) { // 오류가 발생하지 않으면 recv는 수신된 바이트 수를 반환. 0보다 크다는 것은 메시지가 왔다는 것
            string t = buf;
            if (findsubstr(t, 3) == "/dm") {
                string a = buf;
                cout << buf << endl;
                string dm = a.substr(4);
                cout << dm << endl;
                string dm_message = dm.substr(dm.find(" ") + 1);
                cout << dm_message << endl;
                string dm_id = dm.erase(dm.find(" "), dm.size());
                cout << dm_id << endl;

                for (int i = 0; i < sck_list.size(); i++) {
                    if (sck_list[i].user == dm_id) {
                        pstmt = con->prepareStatement("INSERT INTO dm_chatting(시간, 보낸사람, 받는사람, 내용) VALUES(?,?,?,?)"); // INSERT
                        pstmt->setString(1, msg_time);
                        pstmt->setString(2, sck_list[idx].user);
                        pstmt->setString(3, dm_id);
                        pstmt->setString(4, dm_message);
                        pstmt->execute();
                        // MySQL Connector/C++ 정리
                        delete pstmt;

                        send_dm_msg(dm_message.c_str(), i, idx);
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
                        cout << sck_list[idx].user << "가 " << dm_id<< "에게 " << dm_message;
                    }
                }
            }
            else {
                sck_list[idx].user = sck_list[idx].user.substr(0, sck_list[idx].user.find("-"));
                msg = sck_list[idx].user + " : " + buf;
                msg = msg_time + " " + sck_list[idx].user + " : " + buf;
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
                cout << msg << endl;
                send_msg(msg.c_str());

                pstmt = con->prepareStatement("INSERT INTO chatting(시간,id, 내용) VALUES(?,?,?)"); // INSERT
                pstmt->setString(1, msg_time);
                pstmt->setString(2, sck_list[idx].user);
                pstmt->setString(3, buf);
                pstmt->execute();
                // MySQL Connector/C++ 정리
                delete pstmt;
            }
        }
        else { //그렇지 않을 경우 퇴장에 대한 신호로 생각하여 퇴장 메시지 전송
            msg = "[공지] " + sck_list[idx].user + " 님이 퇴장했습니다.";
            cout << msg << endl;
            send_msg(msg.c_str());
            del_client(idx); // 클라이언트 삭제
            return;
        }

    }
}



void del_client(int idx) {
    closesocket(sck_list[idx].sck);
    sck_list.erase(sck_list.begin() + idx); // 배열에서 클라이언트를 삭제하게 될 경우 index가 달라지면서 런타임 오류 발생....ㅎ
    client_count--;
}