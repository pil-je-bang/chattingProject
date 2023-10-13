#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h> //Winsock ������� include. WSADATA �������.
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

// MySQL Connector/C++ �ʱ�ȭ
sql::mysql::MySQL_Driver* driver;// ���� �������� �ʾƵ� Connector/C++�� �ڵ����� ������ ��
sql::Connection* con;
sql::Statement* stmt;
sql::PreparedStatement* pstmt;
sql::ResultSet* res = NULL;

const string server = "tcp://127.0.0.1:3306"; // �����ͺ��̽� �ּ�
const string username = "root"; // �����ͺ��̽� �����
const string password = "(()()&pj0907"; // �����ͺ��̽� ���� ��й�ȣ


int chat_recv() {
    char buf[MAX_SIZE] = { };
    string msg;

    while (1) {
        ZeroMemory(&buf, MAX_SIZE);
        if (recv(client_sock, buf, MAX_SIZE, 0) > 0) {
            msg = buf;
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
// �����ͺ��̽� ����
con->setSchema("kdt");

// db �ѱ� ������ ���� ���� 
stmt = con->createStatement();
stmt->execute("set names euckr");
if (stmt) { delete stmt; stmt = nullptr; }

vector<string> user_info = { "���̵�","�̸�","��й�ȣ(����,����,Ư������ ����)","birth(yyyy-mm-dd)","����ó (010-xxxx-xxxx)","email","address" };
int specialList[12] = { '!','@','#','$','%','^','&','*','(',')','-','+' };
bool numberCheck = false;  // ���� check
bool englishCheck = false; // ���� check
bool specialCheck = false; // Ư�� check
bool is_there_same = 1;
string in;



while (is_there_same) {
    in = "";
    cout << "============ȸ������==============" << endl;
    cout << "���̵� :";
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
                cout << "�̹� �����ϴ� ���̵� �Դϴ�." << endl;
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
        cout << "���ڿ� ������ �������� id�� �����ϼ���." << endl;
        numberCheck = false;  // ���� check
        englishCheck = false; // ���� check
        specialCheck = false; // Ư�� check
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
                cout << "��й�ȣ�� [����,����,Ư������]�� �̷������ �մϴ�." << endl;
                numberCheck = false;  // ���� check
                englishCheck = false; // ���� check
                specialCheck = false; // Ư�� check
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
                cout << "������ ���� �ʽ��ϴ�." << endl;
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
                cout << "������ ���� �ʽ��ϴ�." << endl;
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
                cout << "������ ���� �ʽ��ϴ�." << endl;
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
pstmt = con->prepareStatement("INSERT INTO user_info(id,name,pw,birth,num,email,address) VALUES(?,?,?,?,?,?,?)"); // INSERT
for (int i = 0; i < 7; i++) {
    pstmt->setString(i + 1, user_info[i]);
}
pstmt->execute();
delete pstmt;
cout << "���ԿϷ�" << endl;
   return in;
}

string login(string input_id, string input_pw) {
    /*string input_id = "";
    string input_pw = "";*/
    bool login = false;
    bool pass = false;

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

    // �����ͺ��̽� ���� ����
    stmt = con->createStatement();
    //cin.ignore();
    cout << "==========login===========" << endl;
    cout << "id :";
    if(input_id.length() == 0)
        getline(cin, input_id);
    cout << "pw :";
    if (input_pw.length() == 0)
        getline(cin, input_pw);

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
        cout << "�α��� ����";
    }
    else {
        cout << "�α��� ����";
    }
    delete stmt;
    // MySQL Connector/C++ ����
    delete res;
    delete con;
    return input_id;
}





int main(int argc, char *argv[])
{
    cout << "1�� �α���  2�� ȸ������";
    int a;
    //cin >> a;
    a = atoi(argv[1]);
    if (a == 1) {
        string id = argv[2];
        string pw = argv[3];
        my_nick = login(id, pw);
    }
    else if (a == 2) {
        my_nick = sign_up();
    }
    

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
        InetPton(AF_INET, TEXT("192.168.0.10"), &client_addr.sin_addr);

        while (1) {
            if (!connect(client_sock, (SOCKADDR*)&client_addr, sizeof(client_addr))) { // ���� ������ ������ �ش��ϴ� server�� ����!
                cout << "Server Connect" << endl;
                send(client_sock, my_nick.c_str(), my_nick.length(), 0); // ���ῡ �����ϸ� client �� �Է��� �г����� ������ ����
                break;
            }
            cout << "Connecting..." << endl;
        }
      


        std::thread th2(chat_recv);
        cout << "ä���� ���۵Ǿ����ϴ�.";
        cout << "\n";

        while (1) {
            string text;
            /*std::getline(cin, text);*/
            cin >> text;
            text += get_time();
            const char* buffer = text.c_str(); // string���� char* Ÿ������ ��ȯ
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