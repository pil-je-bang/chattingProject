#include <iostream>
#include <string>
#include <mysql/jdbc.h>
#include <windows.h>
#include <vector>
#define ENABLE_TRACE 

using std::cout;
using std::endl;
using std::string;

using namespace std;

const string server = "tcp://127.0.0.1:3306"; // �����ͺ��̽� �ּ�
const string username = "root"; // �����ͺ��̽� �����
const string password = "admin"; // �����ͺ��̽� ���� ��й�ȣ

int main()
{

    // MySQL Connector/C++ �ʱ�ȭ

    sql::mysql::MySQL_Driver* driver; // ���� �������� �ʾƵ� Connector/C++�� �ڵ����� ������ ��
    sql::Connection* con;
    sql::Statement* stmt;
    sql::PreparedStatement* pstmt;
    sql::ResultSet* res = NULL;
    vector<string> user_info = { "id","name","pw","birth","num","email","address" };
    int choice = 0;
    string in = "";
    string pw = "";
    bool is_there_same = 1;

    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(server, username, password);
    }
    catch (sql::SQLException& e) {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        exit(1);
    }

    // �����ͺ��̽� ����
    con->setSchema("final");

    // db �ѱ� ������ ���� ���� 
    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    // �����ͺ��̽� ���� ����
    stmt = con->createStatement();
    /*stmt->execute("DROP TABLE IF EXISTS user_info");// ä�� ���� �Ҷ� �����ؾ� ��
    cout << "Finished dropping table (if existed)" << endl;// DROP
    stmt->execute("CREATE TABLE user_info (id VARCHAR(50), name VARCHAR(50), pw VARCHAR(50), birth VARCHAR(50), num VARCHAR(50), email VARCHAR(50), address VARCHAR(50));"); // CREATE
    cout << "Finished creating table" << endl;
    */

    while (is_there_same) {
        cout << "============join==============" << endl;
        cout << "id :";
        getline(cin, in);
        res = stmt->executeQuery("SELECT id FROM user_info");
        while (res->next()) {
            std::string id = res->getString("id");
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
        system("cls");
    }

    user_info[0] = in;
    string input = "";

    for (int i = 1; i < 7; i++) {
        cout << user_info[i] << ": ";
        cin >> input;
        user_info[i] = input;
    }
    pstmt = con->prepareStatement("INSERT INTO user_info(id,name,pw,birth,num,email,address) VALUES(?,?,?,?,?,?,?)"); // INSERT
    for (int i = 0; i < 7; i++) {
        pstmt->setString(i+1, user_info[i]);
    }
    pstmt->execute();
    delete pstmt;
    cout << "���ԿϷ�" << endl;

    while (res->next() == true) {
        std::string id = res->getString("id");
        if (in == id) { is_there_same = true; }
    }
    delete stmt;
    delete res;
    delete con;

    return 0;
}