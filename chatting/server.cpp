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

#define MAX_SIZE 1024
#define MAX_CLIENT 3
using std::cout;
using std::cin;
using std::endl;
using std::string;

const string server = "tcp://127.0.0.1:3306"; // �����ͺ��̽� �ּ�
const string username = "root"; // �����ͺ��̽� �����
const string password = "(()()&pj0907"; // �����ͺ��̽� ���� ��й�ȣ



struct SOCKET_INFO { // ����� ���� ������ ���� Ʋ ����
    SOCKET sck;
    string user;
};



// MySQL Connector/C++ �ʱ�ȭ
sql::mysql::MySQL_Driver* driver; // ���� �������� �ʾƵ� Connector/C++�� �ڵ����� ������ ��
sql::Connection* con;
sql::Statement* stmt;
sql::PreparedStatement* pstmt;
sql::ResultSet* res;

std::vector<SOCKET_INFO> sck_list; // ����� Ŭ���̾�Ʈ ���ϵ��� ������ �迭 ����.
SOCKET_INFO server_sock; // ���� ���Ͽ� ���� ������ ������ ���� ����.
SOCKET_INFO server_sock1;
int client_count = 0; // ���� ������ �ִ� Ŭ���̾�Ʈ�� count �� ���� ����.

void server_init(); // socket �ʱ�ȭ �Լ�. socket(), bind(), listen() �Լ� �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
void add_client(); // ���Ͽ� ������ �õ��ϴ� client�� �߰�(accept)�ϴ� �Լ�. client accept() �Լ� �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
void send_msg(const char* msg); // send() �Լ� �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
void recv_msg(int idx); // recv() �Լ� �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
void del_client(int idx); // ���Ͽ� ����Ǿ� �ִ� client�� �����ϴ� �Լ�. closesocket() �����. �ڼ��� ������ �Լ� �����ο��� Ȯ��.
string get_time(); //�ð�

int main() {
    WSADATA wsa;

    // Winsock�� �ʱ�ȭ�ϴ� �Լ�. MAKEWORD(2, 2)�� Winsock�� 2.2 ������ ����ϰڴٴ� �ǹ�.
    // ���࿡ �����ϸ� 0��, �����ϸ� �� �̿��� ���� ��ȯ.
    // 0�� ��ȯ�ߴٴ� ���� Winsock�� ����� �غ� �Ǿ��ٴ� �ǹ�.
    int code = WSAStartup(MAKEWORD(2, 2), &wsa);

    if (!code) {
        server_init();

        std::thread th1[MAX_CLIENT];
        for (int i = 0; i < MAX_CLIENT; i++) {
            // �ο� �� ��ŭ thread �����ؼ� ������ Ŭ���̾�Ʈ�� ���ÿ� ������ �� �ֵ��� ��.
            th1[i] = std::thread(add_client);
        }
        //std::thread th1(add_client); // �̷��� �ϸ� �ϳ��� client�� �޾���...


        while (1) { // ���� �ݺ����� ����Ͽ� ������ ����ؼ� ä�� ���� �� �ִ� ���¸� ����� ��. �ݺ����� ������� ������ �� ���� ���� �� ����.
            string text, msg = "";
            std::getline(cin, text);
            const char* buf = text.c_str();
            msg = server_sock.user + " : " + buf;
            send_msg(msg.c_str());
        }

        for (int i = 0; i < MAX_CLIENT; i++) {
            th1[i].join();
            //join : �ش��ϴ� thread ���� ������ �����ϸ� �����ϴ� �Լ�.
            //join �Լ��� ������ main �Լ��� ���� ����Ǿ thread�� �Ҹ��ϰ� ��.
            //thread �۾��� ���� ������ main �Լ��� ������ �ʵ��� ����.
        }
        //th1.join();

        closesocket(server_sock.sck);
    }
    else {
        cout << "���α׷� ����. (Error code : " << code << ")";
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

    // Internet�� Stream ������� ���� ����
   // SOCKET_INFO�� ���� ��ü�� socket �Լ� ��ȯ��(��ũ���� ����)
   // ���ͳ� �ּ�ü��, ��������, TCP �������� �� ��. 

    SOCKADDR_IN server_addr = {}; // ���� �ּ� ���� ����
    // ���ͳ� ���� �ּ�ü�� server_addr

    server_addr.sin_family = AF_INET; // ������ Internet Ÿ�� 
    server_addr.sin_port = htons(7777); // ���� ��Ʈ ����
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // �����̱� ������ local �����Ѵ�. 
    //Any�� ���� ȣ��Ʈ�� 127.0.0.1�� ��Ƶ� �ǰ� localhost�� ��Ƶ� �ǰ� ���� �� ����ϰ� �� �� �ֵ�. �װ��� INADDR_ANY�̴�.
    //ip �ּҸ� ������ �� server_addr.sin_addr.s_addr -- ������ ���?

    bind(server_sock.sck, (sockaddr*)&server_addr, sizeof(server_addr)); // ������ ���� ������ ���Ͽ� ���ε��Ѵ�.
    listen(server_sock.sck, SOMAXCONN); // ������ ��� ���·� ��ٸ���.
    server_sock.user = "server";

    cout << "Server On" << endl;

}


void add_client() {
    
    string input_id, input_pw;
    SOCKET_INFO new_client = {};
    SOCKADDR_IN addr = {};
    int addrsize = sizeof(addr);
    new_client.sck = accept(server_sock.sck, (sockaddr*)&addr, &addrsize);
    ZeroMemory(&addr, addrsize); // addr�� �޸� ������ 0���� �ʱ�ȭ

    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(server, username, password);
    }
    catch (sql::SQLException& e) {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        exit(1);
    }
    // �����ͺ��̽� ����
    con->setSchema("kdt");

    // db �ѱ� ������ ���� ���� 
    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    while (1) {
        bool ID = false;
        bool PW = false;
        char buf[MAX_SIZE] = { };
        char buf2[MAX_SIZE] = { };
        recv(new_client.sck, buf, MAX_SIZE, 0);

        if (strcmp(buf, "1") == 0) { // �α���
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
            recv(new_client.sck, buf2, sizeof("1"), 0); //ä�ý���
            if (strcmp(buf2, "1") == 0) {
                string msg = "[����] " + input_id + " ���� �����߽��ϴ�.";
                cout << msg << endl;


                std::thread th(recv_msg, client_count);
                // �ٸ� �����κ��� ���� �޽����� ����ؼ� ���� �� �ִ� ���·� ����� �α�.

                client_count++; // client �� ����.
                cout << "[����] ���� ������ �� : " << client_count << "��" << endl;
                send_msg(msg.c_str()); // c_str : string Ÿ���� const chqr* Ÿ������ �ٲ���.
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

                    // �����ͺ��̽� ���� ����
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
                    string newId = "�˼�����";

                    pstmt = con->prepareStatement("UPDATE chatting SET id = ? WHERE id = ?");
                    pstmt->setString(1, newId);     // newId�� �����Ϸ��� ���ο� ��
                    pstmt->setString(2, input_id); // input_id�� �����Ϸ��� Ư�� id
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
                    send(new_client.sck, "�߸��Է�", strlen("�߸��Է�"), 0);
                }

            }
            else if ((strcmp(buf2, "4") == 0)) {  //��������
                char buf3[MAX_SIZE] = { };
                recv(new_client.sck, buf3, MAX_SIZE, 0);// ��� ����
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
            std::vector<string> user_info = { "���̵�","�̸�","��й�ȣ(����,����,Ư������ ����)","birth(yyyy-mm-dd)","����ó (010-xxxx-xxxx)","email","address" };


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
    for (int i = 0; i < client_count; i++) { // ������ �ִ� ��� client���� �޽��� ����
        send(sck_list[i].sck, msg, MAX_SIZE, 0);
    }
}
void send_dm_msg(const char* msg, int i, int index) { //dm
    string q = "/dm" + sck_list[index].user + "�� �ӼӸ�:";
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

    // �����ͺ��̽� ����
    con->setSchema("kdt");

    // db �ѱ� ������ ���� ���� 
    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    ////cout << sck_list[idx].user << endl;

    while (1) {
        ZeroMemory(&buf, MAX_SIZE);
        if (recv(sck_list[idx].sck, buf, MAX_SIZE, 0) > 0) { // ������ �߻����� ������ recv�� ���ŵ� ����Ʈ ���� ��ȯ. 0���� ũ�ٴ� ���� �޽����� �Դٴ� ��
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
                        pstmt = con->prepareStatement("INSERT INTO dm_chatting(�ð�, �������, �޴»��, ����) VALUES(?,?,?,?)"); // INSERT
                        pstmt->setString(1, msg_time);
                        pstmt->setString(2, sck_list[idx].user);
                        pstmt->setString(3, dm_id);
                        pstmt->setString(4, dm_message);
                        pstmt->execute();
                        // MySQL Connector/C++ ����
                        delete pstmt;

                        send_dm_msg(dm_message.c_str(), i, idx);
                        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
                        cout << sck_list[idx].user << "�� " << dm_id<< "���� " << dm_message;
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

                pstmt = con->prepareStatement("INSERT INTO chatting(�ð�,id, ����) VALUES(?,?,?)"); // INSERT
                pstmt->setString(1, msg_time);
                pstmt->setString(2, sck_list[idx].user);
                pstmt->setString(3, buf);
                pstmt->execute();
                // MySQL Connector/C++ ����
                delete pstmt;
            }
        }
        else { //�׷��� ���� ��� ���忡 ���� ��ȣ�� �����Ͽ� ���� �޽��� ����
            msg = "[����] " + sck_list[idx].user + " ���� �����߽��ϴ�.";
            cout << msg << endl;
            send_msg(msg.c_str());
            del_client(idx); // Ŭ���̾�Ʈ ����
            return;
        }

    }
}



void del_client(int idx) {
    closesocket(sck_list[idx].sck);
    sck_list.erase(sck_list.begin() + idx); // �迭���� Ŭ���̾�Ʈ�� �����ϰ� �� ��� index�� �޶����鼭 ��Ÿ�� ���� �߻�....��
    client_count--;
}