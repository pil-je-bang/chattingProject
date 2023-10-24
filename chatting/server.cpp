#pragma comment(lib, "ws2_32.lib") //������� ���̺귯���� ��ũ. ���� ���̺귯�� ����

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

#define MAX_SIZE 1024 // ���� ũ��
#define MAX_CLIENT 3  // ä�ù� ����

using std::cout;
using std::cin;
using std::endl;
using std::string;

const string server = "tcp://127.0.0.1:3306"; // �����ͺ��̽� �ּ�
const string username = "root";               // �����ͺ��̽� �����
const string password = "(()()&pj0907";       // �����ͺ��̽� ���� ��й�ȣ

struct SOCKET_INFO {                          // ����� ���� ������ ���� Ʋ ����
    SOCKET sck;
    string user;
    int ti;
};

// MySQL Connector/C++ �ʱ�ȭ
sql::mysql::MySQL_Driver* driver; // ���� �������� �ʾƵ� Connector/C++�� �ڵ����� ������ ��
sql::Connection* con;
sql::Statement* stmt;
sql::PreparedStatement* pstmt;
sql::ResultSet* res;

std::vector<SOCKET_INFO> SocketList; // ����� Ŭ���̾�Ʈ ���ϵ��� ������ �迭 ����.
SOCKET_INFO ServerSocket;           // ���� ���Ͽ� ���� ������ ������ ���� ����.
int ClientCount = 0;              // ���� ������ �ִ� Ŭ���̾�Ʈ�� count �� ���� ����.
int recreate = -1;
string GetTime();                 //ä�ýð�

void ServerInit();             // socket �ʱ�ȭ �Լ�. socket(), bind(), listen() �Լ� �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
void AddClient(int ti);        // ���Ͽ� ������ �õ��ϴ� client�� �߰�(accept)�ϴ� �Լ�. client accept() �Լ� �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
void SendMsg(const char* msg); // send() �Լ� �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
void RecvMsg(string user);     // recv() �Լ� �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
void DelClient(int idx);       // ���Ͽ� ����Ǿ� �ִ� client�� �����ϴ� �Լ�. closesocket() �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
int RemoveSocket(string user);  // user �������� ���� ����  
int LogoutSoket(string user);   // �α׾ƿ��� ���� ����
void RecreateThread();          // ������ ���� �� �����
SOCKET GetSocket(string user);

std::thread ClientThread[MAX_CLIENT];    // ä�ù� ���� ��ŭ ������迭 ����





int main() {
    WSADATA wsa;
    int code = WSAStartup(MAKEWORD(2, 2), &wsa);    // Winsock�� �ʱ�ȭ�ϴ� �Լ�. MAKEWORD(2, 2)�� Winsock�� 2.2 ������ ����ϰڴٴ� �ǹ�.
    // ���࿡ �����ϸ� 0��, �����ϸ� �� �̿��� ���� ��ȯ.
    // 0�� ��ȯ�ߴٴ� ���� Winsock�� ����� �غ� �Ǿ��ٴ� �ǹ�.
    if (!code) {
        ServerInit();
        for (int i = 0; i < MAX_CLIENT; i++) {      // �ο� �� ��ŭ thread ���� �� �迭�� ����
            ClientThread[i] = std::thread(AddClient, i);
        }
        std::thread th2(RecreateThread);            // ������ ����� ������
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
        cout << "���α׷� ����. (Error code : " << code << ")";
    }
    WSACleanup();

    return 0;
}

void ServerInit() {
    ServerSocket.sck = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Internet�� Stream ������� ���� ����
    // SOCKET_INFO�� ���� ��ü�� socket �Լ� ��ȯ��(��ũ���� ����)
    // ���ͳ� �ּ�ü��, ��������, TCP �������� �� ��. 

    SOCKADDR_IN ServerAddr = {};                     // ���� �ּ� ���� ����
    // ���ͳ� ���� �ּ�ü�� server_addr
    ServerAddr.sin_family = AF_INET;                 // ������ Internet Ÿ�� 
    ServerAddr.sin_port = htons(7777);               // ���� ��Ʈ ����
    ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // �����̱� ������ local �����Ѵ�. 
    bind(ServerSocket.sck, (sockaddr*)&ServerAddr, sizeof(ServerAddr)); // ������ ���� ������ ���Ͽ� ���ε��Ѵ�.
    listen(ServerSocket.sck, SOMAXCONN);                                  // ������ ��� ���·� ��ٸ���.
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

        recv(NewClient.sck, MenuBuf, MAX_SIZE, 0); // ���۸޴� ���� ���� �޾ƿ�


        if (strcmp(MenuBuf, "1") == 0) { // �α���
            while (1) {
                char LoginBuf[MAX_SIZE] = { };
                bool SameId = false;

                recv(NewClient.sck, LoginBuf, MAX_SIZE, 0);

                NewClient.user = string(LoginBuf);
                InputId = NewClient.user.substr(0, NewClient.user.find("-"));
                InputPw = NewClient.user.substr(NewClient.user.find("-") + 1);
                NewClient.user = InputId;


                for (int i = 0; i < SocketList.size(); i++) { // �������� ���̵����� Ȯ��
                    if (SocketList[i].user == NewClient.user) { SameId = true; }
                }

                if (!SameId) {         // ���̵� �������� �ƴҶ� �α��� ���� �Ǻ�

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
                    if (ID && PW) { // �α��� ����!!
                        NewClient.user = InputId;
                        NewClient.ti = ti;
                        send(NewClient.sck, "true", strlen("true"), 0);
                    }
                    else {
                        send(NewClient.sck, "false", strlen("false"), 0);
                    }
                }
                else {                                     // ���̵� �������϶�!!
                    send(NewClient.sck, "false", strlen("false"), 0);
                }
                break;
            }
            recv(NewClient.sck, SecondMenu, 2, 0);        //���θ޴� ä�ù����� �޾ƿ���
            if (strcmp(SecondMenu, "1") == 0) {            // ä�ù� ����!!!
                SocketList.push_back(NewClient);
                string msg = "[����] " + InputId + " ���� �����߽��ϴ�.";
                cout << msg << endl;
                std::thread th(RecvMsg, NewClient.user); // �ٸ� �����κ��� ���� �޽����� ����ؼ� ���� �� �ִ� ���·� ����� �α�.
                ClientCount++;                            // client �� ����.
                cout << "[����] ���� ������ �� : " << ClientCount << "��" << endl;
                SendMsg(msg.c_str());
                th.join();
                break;
            }
            else if ((strcmp(SecondMenu, "3") == 0)) { // ȸ��Ż��
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
                    string NewId = "�˼�����";

                    pstmt = con->prepareStatement("UPDATE chatting SET id = ? WHERE id = ?");
                    pstmt->setString(1, NewId);     // newId�� �����Ϸ��� ���ο� ��
                    pstmt->setString(2, InputId); // input_id�� �����Ϸ��� Ư�� id
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
                    send(NewClient.sck, "�߸��Է�", strlen("�߸��Է�"), 0);
                }

            }
            else if ((strcmp(SecondMenu, "4") == 0)) {  //��������
                char CheckIdPw[MAX_SIZE] = { };
                recv(NewClient.sck, CheckIdPw, MAX_SIZE, 0);// ��� ����

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

                if (ID && PW) { // �α��� ����!!
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
            else if (strcmp(SecondMenu, "5") == 0) { // �α׾ƿ�
                string msg;
                msg = "[����] " + NewClient.user + " ���� �α׾ƿ��߽��ϴ�.";
                cout << msg << endl;
                LogoutSoket(NewClient.user);
            }
        }
        else if (strcmp(MenuBuf, "2") == 0) {
            std::vector<string> SignInfo = { "���̵�","�̸�","��й�ȣ(����,����,Ư������ ����)","birth(yyyy-mm-dd)","����ó (010-xxxx-xxxx)","email","address" };

            for (int i = 0; i < 7; i++) {
                char SignAns[MAX_SIZE] = { };
                recv(NewClient.sck, SignAns, sizeof(SignAns), 0);
                if (i == 0) {                                               //���̵� �ߺ�Ȯ��
                    stmt = con->createStatement();
                    res = stmt->executeQuery("SELECT id FROM user_info");
                    bool NoSame = true;
                    while (res->next() == true) {
                        std::string id = res->getString("id");
                        if (SignAns == id) {                                    //�ߺ� ���̵��� ��
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
    for (int i = 0; i < SocketList.size(); i++) { // ������ �ִ� ��� client���� �޽��� ����
        send(SocketList[i].sck, msg, MAX_SIZE, 0);
    }
}

void SendDmMsg(const char* msg, int i, string RecvDmUser, string SendDmUser) { //dm
    string DmMsg = "/dm" + SendDmUser + "�� �ӼӸ�:";
    for (int i = 0; i < SocketList.size(); i++) { // ������ �ִ� ��� client���� �޽��� ����
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
        if (recv(sck, buf, MAX_SIZE, 0) > 0) { // ������ �߻����� ������ recv�� ���ŵ� ����Ʈ ���� ��ȯ. 0���� ũ�ٴ� ���� �޽����� �Դٴ� ��
            string t = buf;
            if (FindSubStr(t, 3) == "/dm") {
                string a = buf;
                string dm = a.substr(4);
                string dm_message = dm.substr(dm.find(" ") + 1);
                string dm_id = dm.erase(dm.find(" "), dm.size());

                pstmt = con->prepareStatement("INSERT INTO dm_chatting(�ð�,�������, �޴»�� ,����) VALUES(?,?,?,?)"); // INSERT
                pstmt->setString(1, MsgTime);
                pstmt->setString(2, user);
                pstmt->setString(3, dm_id);
                pstmt->setString(4, dm_message);
                pstmt->execute();
                // MySQL Connector/C++ ����
                delete pstmt;
                SendDmMsg(dm_message.c_str(), SocketList.size(), dm_id, user);
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
                cout << user << "�� " << dm_id << "���� " << dm_message<<endl;
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
            }

            else {
                msg = user + " : " + buf;
                msg = MsgTime + " " + user + " : " + buf;
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
                cout << msg << endl;
                SendMsg(msg.c_str());

                pstmt = con->prepareStatement("INSERT INTO chatting(�ð�,id, ����) VALUES(?,?,?)"); // INSERT
                pstmt->setString(1, MsgTime);
                pstmt->setString(2, user);
                pstmt->setString(3, buf);
                pstmt->execute();
                delete pstmt;
            }
        }
        else { //�׷��� ���� ��� ���忡 ���� ��ȣ�� �����Ͽ� ���� �޽��� ����
            msg = "[����] " + user + " ���� �����߽��ϴ�.";
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
    SocketList.erase(SocketList.begin() + idx); // �迭���� Ŭ���̾�Ʈ�� �����ϰ� �� ��� index�� �޶����鼭 ��Ÿ�� ���� �߻�....��
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

