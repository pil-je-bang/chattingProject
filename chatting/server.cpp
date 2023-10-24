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

#define MAX_SIZE 1024 // 버퍼 크기
#define MAX_CLIENT 3  // 채팅방 정원

using std::cout;
using std::cin;
using std::endl;
using std::string;

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root";               // 데이터베이스 사용자
const string password = "(()()&pj0907";       // 데이터베이스 접속 비밀번호

struct SOCKET_INFO {                          // 연결된 소켓 정보에 대한 틀 생성
    SOCKET sck;
    string user;
    int ti;
};

// MySQL Connector/C++ 초기화
sql::mysql::MySQL_Driver* driver; // 추후 해제하지 않아도 Connector/C++가 자동으로 해제해 줌
sql::Connection* con;
sql::Statement* stmt;
sql::PreparedStatement* pstmt;
sql::ResultSet* res;

std::vector<SOCKET_INFO> SocketList; // 연결된 클라이언트 소켓들을 저장할 배열 선언.
SOCKET_INFO ServerSocket;           // 서버 소켓에 대한 정보를 저장할 변수 선언.
int ClientCount = 0;              // 현재 접속해 있는 클라이언트를 count 할 변수 선언.
int recreate = -1;
string GetTime();                 //채팅시간

void ServerInit();             // socket 초기화 함수. socket(), bind(), listen() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void AddClient(int ti);        // 소켓에 연결을 시도하는 client를 추가(accept)하는 함수. client accept() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void SendMsg(const char* msg); // send() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void RecvMsg(string user);     // recv() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void DelClient(int idx);       // 소켓에 연결되어 있는 client를 제거하는 함수. closesocket() 실행됨. 자세한 내용은 함수 구현부에서 확인.
int RemoveSocket(string user);  // user 기준으로 소켓 삭제  
int LogoutSoket(string user);   // 로그아웃시 소켓 삭제
void RecreateThread();          // 쓰레드 삭제 후 재생성
SOCKET GetSocket(string user);

std::thread ClientThread[MAX_CLIENT];    // 채팅방 정원 만큼 쓰레드배열 생성





int main() {
    WSADATA wsa;
    int code = WSAStartup(MAKEWORD(2, 2), &wsa);    // Winsock를 초기화하는 함수. MAKEWORD(2, 2)는 Winsock의 2.2 버전을 사용하겠다는 의미.
    // 실행에 성공하면 0을, 실패하면 그 이외의 값을 반환.
    // 0을 반환했다는 것은 Winsock을 사용할 준비가 되었다는 의미.
    if (!code) {
        ServerInit();
        for (int i = 0; i < MAX_CLIENT; i++) {      // 인원 수 만큼 thread 생성 후 배열로 관리
            ClientThread[i] = std::thread(AddClient, i);
        }
        std::thread th2(RecreateThread);            // 쓰레드 재생성 쓰레드
        while (1) {
            string text, msg = "";
            std::getline(cin, text);
            const char* buf = text.c_str();
            msg = ServerSocket.user + " : " + buf;
            SendMsg(msg.c_str());
        }
        for (int i = 0; i < MAX_CLIENT; i++) {
            ClientThread[i].join();
        }

        th2.join();

        closesocket(ServerSocket.sck);
    }
    else {
        cout << "프로그램 종료. (Error code : " << code << ")";
    }
    WSACleanup();

    return 0;
}

void ServerInit() {
    ServerSocket.sck = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Internet의 Stream 방식으로 소켓 생성
    // SOCKET_INFO의 소켓 객체에 socket 함수 반환값(디스크립터 저장)
    // 인터넷 주소체계, 연결지향, TCP 프로토콜 쓸 것. 

    SOCKADDR_IN ServerAddr = {};                     // 소켓 주소 설정 변수
    // 인터넷 소켓 주소체계 server_addr
    ServerAddr.sin_family = AF_INET;                 // 소켓은 Internet 타입 
    ServerAddr.sin_port = htons(7777);               // 서버 포트 설정
    ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // 서버이기 때문에 local 설정한다. 
    bind(ServerSocket.sck, (sockaddr*)&ServerAddr, sizeof(ServerAddr)); // 설정된 소켓 정보를 소켓에 바인딩한다.
    listen(ServerSocket.sck, SOMAXCONN);                                  // 소켓을 대기 상태로 기다린다.
    ServerSocket.user = "server";
    cout << "Server On" << endl;
}

void AddClient(int ti) {

    string InputId, InputPw;
    SOCKET_INFO NewClient = {};
    SOCKADDR_IN addr = {};
    int addrsize = sizeof(addr);

    ZeroMemory(&addr, addrsize);

    NewClient.sck = accept(ServerSocket.sck, (sockaddr*)&addr, &addrsize);

    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(server, username, password);
    }
    catch (sql::SQLException& e) {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        exit(1);
    }

    con->setSchema("kdt");

    stmt = con->createStatement();
    stmt->execute("set names euckr");

    if (stmt) {
        delete stmt; stmt = nullptr;
    }

    while (1) {

        bool ID = false;
        bool PW = false;
        char MenuBuf[MAX_SIZE] = { };
        char SecondMenu[MAX_SIZE] = { };

        recv(NewClient.sck, MenuBuf, MAX_SIZE, 0); // 시작메뉴 실행 할지 받아옴


        if (strcmp(MenuBuf, "1") == 0) { // 로그인
            while (1) {
                char LoginBuf[MAX_SIZE] = { };
                bool SameId = false;

                recv(NewClient.sck, LoginBuf, MAX_SIZE, 0);

                NewClient.user = string(LoginBuf);
                InputId = NewClient.user.substr(0, NewClient.user.find("-"));
                InputPw = NewClient.user.substr(NewClient.user.find("-") + 1);
                NewClient.user = InputId;


                for (int i = 0; i < SocketList.size(); i++) { // 접속중인 아이디인지 확인
                    if (SocketList[i].user == NewClient.user) { SameId = true; }
                }

                if (!SameId) {         // 아이디가 접속중이 아닐때 로그인 정보 판별

                    stmt = con->createStatement();
                    res = stmt->executeQuery("SELECT id FROM user_info");

                    while (res->next() == true) {
                        std::string id = res->getString("id");
                        if (InputId == id) { ID = true; }
                    }

                    stmt = con->createStatement();
                    res = stmt->executeQuery("SELECT pw FROM user_info where id =\"" + InputId + "\"");
                    while (res->next() == true) {
                    std::string pw = res->getString("pw");
                    if (InputPw == pw) { PW = true; }
                    }
                    if (ID && PW) { // 로그인 성공!!
                        NewClient.user = InputId;
                        NewClient.ti = ti;
                        send(NewClient.sck, "true", strlen("true"), 0);
                    }
                    else {
                        send(NewClient.sck, "false", strlen("false"), 0);
                    }
                }
                else {                                     // 아이디 접속중일때!!
                    send(NewClient.sck, "false", strlen("false"), 0);
                }
                break;
            }
            recv(NewClient.sck, SecondMenu, 2, 0);        //메인메뉴 채팅방입장 받아오기
            if (strcmp(SecondMenu, "1") == 0) {            // 채팅방 입장!!!
                SocketList.push_back(NewClient);
                string msg = "[공지] " + InputId + " 님이 입장했습니다.";
                cout << msg << endl;
                std::thread th(RecvMsg, NewClient.user); // 다른 사람들로부터 오는 메시지를 계속해서 받을 수 있는 상태로 만들어 두기.
                ClientCount++;                            // client 수 증가.
                cout << "[공지] 현재 접속자 수 : " << ClientCount << "명" << endl;
                SendMsg(msg.c_str());
                th.join();
                break;
            }
            else if ((strcmp(SecondMenu, "3") == 0)) { // 회원탈퇴
                char YesOrNO[MAX_SIZE] = {};
                char UserInfo[MAX_SIZE] = {};
                while (1) {

                    bool ID = false;
                    bool PW = false;

                    recv(NewClient.sck, UserInfo, MAX_SIZE, 0);
                    NewClient.user = string(UserInfo);
                    InputId = NewClient.user.substr(0, NewClient.user.find("-"));
                    InputPw = NewClient.user.substr(NewClient.user.find("-") + 1);

                    stmt = con->createStatement();
                    res = stmt->executeQuery("SELECT id FROM user_info");

                    while (res->next() == true) {
                        std::string id = res->getString("id");
                        if (InputId == id) {ID = true; }
                    }
                    res = stmt->executeQuery("SELECT pw FROM user_info where id =\"" + InputId + "\"");
                    while (res->next() == true) {
                        std::string pw = res->getString("pw");
                        if (InputPw == pw) { PW = true; }
                    }
                    if (ID && PW) {
                        send(NewClient.sck, "true", strlen("true"), 0);
                        break;
                    }
                    else {
                        send(NewClient.sck, "false", strlen("false"), 0);
                    }
                }

                recv(NewClient.sck, YesOrNO, MAX_SIZE, 0);

                if (strcmp(YesOrNO, "yes") == 0) {
                    string NewId = "알수없음";

                    pstmt = con->prepareStatement("UPDATE chatting SET id = ? WHERE id = ?");
                    pstmt->setString(1, NewId);     // newId는 변경하려는 새로운 값
                    pstmt->setString(2, InputId); // input_id는 변경하려는 특정 id
                    pstmt->executeUpdate();

                    pstmt = con->prepareStatement("DELETE FROM user_info WHERE id = ?");
                    pstmt->setString(1, InputId);
                    pstmt->executeUpdate();

                    send(NewClient.sck, "true", strlen("true"), 0);
                }
                else if (strcmp(YesOrNO, "no") == 0) {
                    send(NewClient.sck, "false", strlen("false"), 0);
                }
                else {
                    send(NewClient.sck, "잘못입력", strlen("잘못입력"), 0);
                }

            }
            else if ((strcmp(SecondMenu, "4") == 0)) {  //정보수정
                char CheckIdPw[MAX_SIZE] = { };
                recv(NewClient.sck, CheckIdPw, MAX_SIZE, 0);// 비번 들고옴

                NewClient.user = string(CheckIdPw);
                InputId = NewClient.user.substr(0, NewClient.user.find("-"));
                InputPw = NewClient.user.substr(NewClient.user.find("-") + 1);
                NewClient.user = InputId;

                stmt = con->createStatement();
                res = stmt->executeQuery("SELECT id FROM user_info");

                while (res->next() == true) {
                    std::string id = res->getString("id");
                    if (InputId == id) { ID = true; }
                }

                stmt = con->createStatement();
                res = stmt->executeQuery("SELECT pw FROM user_info where id =\"" + InputId + "\"");
                while (res->next() == true) {
                    std::string pw = res->getString("pw");
                    if (InputPw == pw) { PW = true; }
                }

                if (ID && PW) { // 로그인 성공!!
                    NewClient.user = InputId;
                    NewClient.ti = ti;
                    send(NewClient.sck, "true", strlen("true"), 0);
                }
                else {
                    send(NewClient.sck, "false", strlen("false"), 0);
                }

                char UserInfo[MAX_SIZE] = { };
                recv(NewClient.sck, UserInfo, MAX_SIZE, 0);

                string ReviseNum, RevisInfo, ReviseCol;
                string UserInfoString = string(UserInfo);

                ReviseNum = UserInfoString.substr(0, UserInfoString.find("-"));
                RevisInfo = UserInfoString.substr(UserInfoString.find("-") + 1);

                if (ReviseNum == "1") {
                    ReviseCol = "name";
                }
                else if (ReviseNum == "2") {
                    ReviseCol = "pw";
                }
                else if (ReviseNum == "3") {
                    ReviseCol = "birth";
                }
                else if (ReviseNum == "4") {
                    ReviseCol = "num";
                }
                else if (ReviseNum == "5") {
                    ReviseCol = "email";
                }
                else if (ReviseNum == "6") {
                    ReviseCol = "address";
                }

                pstmt = con->prepareStatement("UPDATE user_info SET " + ReviseCol + " = ? WHERE id = ? ");
                pstmt->setString(1, RevisInfo);
                pstmt->setString(2, NewClient.user);
                pstmt->executeUpdate();
                send(NewClient.sck, "true", strlen("true"), 0);

            }
            else if (strcmp(SecondMenu, "5") == 0) { // 로그아웃
                string msg;
                msg = "[공지] " + NewClient.user + " 님이 로그아웃했습니다.";
                cout << msg << endl;
                LogoutSoket(NewClient.user);
            }
        }
        else if (strcmp(MenuBuf, "2") == 0) {
            std::vector<string> SignInfo = { "아이디","이름","비밀번호(영어,숫자,특수문자 조합)","birth(yyyy-mm-dd)","연락처 (010-xxxx-xxxx)","email","address" };

            for (int i = 0; i < 7; i++) {
                char SignAns[MAX_SIZE] = { };
                recv(NewClient.sck, SignAns, sizeof(SignAns), 0);
                if (i == 0) {                                               //아이디 중복확인
                    stmt = con->createStatement();
                    res = stmt->executeQuery("SELECT id FROM user_info");
                    bool NoSame = true;
                    while (res->next() == true) {
                        std::string id = res->getString("id");
                        if (SignAns == id) {                                    //중복 아이디일 때
                            send(NewClient.sck, "false", sizeof("false"), 0);
                            NoSame = false;
                            i--;
                            break;
                        }
                    }
                    if (NoSame) {
                        send(NewClient.sck, "true", sizeof("true"), 0);
                        SignInfo[0] = SignAns;
                    }
                }
                else {
                    SignInfo[i] = SignAns;
                }
            }
            pstmt = con->prepareStatement("INSERT INTO user_info(id,name,pw,birth,num,email,address) VALUES(?,?,?,?,?,?,?)"); // INSERT
            for (int i = 0; i < 7; i++) {
                pstmt->setString(i + 1, SignInfo[i]);
            }
            pstmt->execute();
            delete pstmt;
        }
    }
}

void SendMsg(const char* msg) {
    for (int i = 0; i < SocketList.size(); i++) { // 접속해 있는 모든 client에게 메시지 전송
        send(SocketList[i].sck, msg, MAX_SIZE, 0);
    }
}

void SendDmMsg(const char* msg, int i, string RecvDmUser, string SendDmUser) { //dm
    string DmMsg = "/dm" + SendDmUser + "의 귓속말:";
    for (int i = 0; i < SocketList.size(); i++) { // 접속해 있는 모든 client에게 메시지 전송
        if (SocketList[i].user == RecvDmUser) {
            send(SocketList[i].sck, (DmMsg + msg).c_str(), MAX_SIZE, 0);
        }
    }

}

string FindSubStr(string const& str, int n) {
    if (str.length() < n) {
        return str;
    }
    return str.substr(0, n);
}

void RecvMsg(string user) {
    char buf[MAX_SIZE] = { };
    string msg = "";
    SOCKET sck = GetSocket(user);
    string MsgTime = GetTime();

    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(server, username, password);
    }
    catch (sql::SQLException& e) {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        exit(1);
    }

    con->setSchema("kdt");

    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    while (1) {
        ZeroMemory(&buf, MAX_SIZE);
        if (recv(sck, buf, MAX_SIZE, 0) > 0) { // 오류가 발생하지 않으면 recv는 수신된 바이트 수를 반환. 0보다 크다는 것은 메시지가 왔다는 것
            string t = buf;
            if (FindSubStr(t, 3) == "/dm") {
                string a = buf;
                string dm = a.substr(4);
                string dm_message = dm.substr(dm.find(" ") + 1);
                string dm_id = dm.erase(dm.find(" "), dm.size());

                pstmt = con->prepareStatement("INSERT INTO dm_chatting(시간,보낸사람, 받는사람 ,내용) VALUES(?,?,?,?)"); // INSERT
                pstmt->setString(1, MsgTime);
                pstmt->setString(2, user);
                pstmt->setString(3, dm_id);
                pstmt->setString(4, dm_message);
                pstmt->execute();
                // MySQL Connector/C++ 정리
                delete pstmt;
                SendDmMsg(dm_message.c_str(), SocketList.size(), dm_id, user);
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
                cout << user << "가 " << dm_id << "에게 " << dm_message<<endl;
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
            }

            else {
                msg = user + " : " + buf;
                msg = MsgTime + " " + user + " : " + buf;
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
                cout << msg << endl;
                SendMsg(msg.c_str());

                pstmt = con->prepareStatement("INSERT INTO chatting(시간,id, 내용) VALUES(?,?,?)"); // INSERT
                pstmt->setString(1, MsgTime);
                pstmt->setString(2, user);
                pstmt->setString(3, buf);
                pstmt->execute();
                delete pstmt;
            }
        }
        else { //그렇지 않을 경우 퇴장에 대한 신호로 생각하여 퇴장 메시지 전송
            msg = "[공지] " + user + " 님이 퇴장했습니다.";
            cout << msg << endl;
            SendMsg(msg.c_str());
            int remove = RemoveSocket(user);
            if (remove > -1) {
                recreate = remove;
            }
            return;
        }

    }
}

void DelClient(int idx) {
    closesocket(SocketList[idx].sck);
    SocketList.erase(SocketList.begin() + idx); // 배열에서 클라이언트를 삭제하게 될 경우 index가 달라지면서 런타임 오류 발생....ㅎ
    ClientCount--;
}

SOCKET GetSocket(string user) {
    for (int i = 0; i < SocketList.size(); i++) {
        if (SocketList[i].user == user) {
            return SocketList[i].sck;
        }
    }
    return 0;
}

void RecreateThread() {
    while (1) {
        if (recreate > -1) {
            ClientThread[recreate].join();
            ClientThread[recreate] = std::thread(AddClient, recreate);
            recreate = -1;
        }
        if (recreate == -2) {
            return;
        }
        Sleep(1000);
    }
}

int RemoveSocket(string user) {
    int ti;
    for (int i = 0; i < SocketList.size(); i++) {
        if (SocketList[i].user == user) {
            ti = SocketList[i].ti;
            DelClient(i);
            return ti;
        }
    }
    return -1;
}

int LogoutSoket(string user) {
    int ti;
    for (int i = 0; i < SocketList.size(); i++) {
        if (SocketList[i].user == user) {
            ti = SocketList[i].ti;
            SocketList.erase(SocketList.begin() + i);
            return ti;
        }
    }
    return -1;
}

string GetTime()
{
    using namespace std::chrono;
    system_clock::time_point tp = system_clock::now();
    std::stringstream str;
    __time64_t t1 = system_clock::to_time_t(tp);
    system_clock::time_point t2 = system_clock::from_time_t(t1);
    if (t2 > tp) {
        t1 = system_clock::to_time_t(tp - seconds(1));
    }
    tm tm{};
    localtime_s(&tm, &t1);
    str << std::put_time(&tm, "[%Y-%m-%d %H:%M:%S]");

    return str.str();
}

