#include <iostream>
#include <string>
#include <mysql/jdbc.h>
#include <windows.h>
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
    stmt->execute("CREATE TABLE user_info (id VARCHAR(50) PRIMARY key , name VARCHAR(50));"); // CREATE
    cout << "Finished creating table" << endl;*/
    while (is_there_same) {
        cout << "id :";
        getline(cin, in);
        res = stmt->executeQuery("SELECT id FROM user_info");
        while (res->next() == true) {
            std::string id = res->getString("id");
            if (in == id) {
                is_there_same = true; cout << "�̹� �����ϴ� ���̵� �Դϴ�." << endl;
                Sleep(500);
                break;
            }
            else
                is_there_same = false;
        }
        system("cls");
    }
    if (is_there_same) {}
    else {
        pstmt = con->prepareStatement("INSERT INTO user_info(id, name ) VALUES(?,?)"); // INSERT
        pstmt->setString(1, in);
        pstmt->setString(2, "john");
        pstmt->execute();
        cout << "���ԿϷ�" << endl;
        delete pstmt;
    }


    cout << "===========id list=============" << endl;
    
    while (res->next() == true) {
        std::string id = res->getString("id");
        if (in == id) { is_there_same = true; }
    }
    delete stmt;

    // MySQL Connector/C++ ����
    delete res;

    delete con;

    return 0;
}