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

    
    cout << "id :";
    string in = "";
    string pw = "";
    getline(cin, in);
    cout << "pw :";
    getline(cin, pw);

    pstmt = con->prepareStatement("INSERT INTO user_info(id, name ) VALUES(?,?)"); // INSERT
    pstmt->setString(1, in);
    pstmt->setString(2, "john");
    pstmt->execute();
    cout << "One row inserted." << endl;

    res = stmt->executeQuery("SELECT id FROM user_info");

    cout << "===========id list=============" <<  endl;
    while (res->next() == true) {
        std::string id = res->getString("id");
        if (id == id.c_str()) {

        }
    }
    
    delete stmt;

 
  


    
    // MySQL Connector/C++ ����
    delete res;
    delete pstmt;
    delete con;

    return 0;
}