#include <iostream>
#include <string>
#include <mysql/jdbc.h>

using std::cout;
using std::cin;
using std::endl;
using std::string;

const string server = "tcp://127.0.0.1:3306"; // �����ͺ��̽� �ּ�
const string username = "root"; // �����ͺ��̽� �����
const string password = "(()()&pj0907"; // �����ͺ��̽� ���� ��й�ȣ

int main()
{
    // MySQL Connector/C++ �ʱ�ȭ
    sql::mysql::MySQL_Driver* driver; // ���� �������� �ʾƵ� Connector/C++�� �ڵ����� ������ ��
    sql::Connection* con;
    sql::Statement* stmt;
    sql::PreparedStatement* pstmt;

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
    stmt->execute("DROP TABLE IF EXISTS chatting"); // DROP
    cout << "Finished dropping table (if existed)" << endl;
    stmt->execute("CREATE TABLE chatting (id serial PRIMARY KEY, ���� VARCHAR(255));"); // CREATE
    cout << "Finished creating table" << endl;
    delete stmt;

    pstmt = con->prepareStatement("INSERT INTO chatting(����) VALUES(?)"); // INSERT
    cout << "ä���� ���۵Ǿ����ϴ�.";
    cout << "\n";
    string a;
    while (a != "c") {
        cin >> a;
        cout << "\n";
        pstmt->setString(1, a);
        pstmt->execute();
        //cout << "One row inserted." << endl;

    }


    // MySQL Connector/C++ ����
    delete pstmt;
    delete con;

    return 0;
}