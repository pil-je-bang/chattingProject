#include <iostream>
#include <string>
#include <mysql/jdbc.h>
#include <windows.h>
#define ENABLE_TRACE 

using std::cout;
using std::endl;
using std::string;

using namespace std;

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "admin"; // 데이터베이스 접속 비밀번호

int main()
{

    // MySQL Connector/C++ 초기화
    sql::mysql::MySQL_Driver* driver; // 추후 해제하지 않아도 Connector/C++가 자동으로 해제해 줌
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

    // 데이터베이스 선택
    con->setSchema("final");

    // db 한글 저장을 위한 셋팅 
    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    // 데이터베이스 쿼리 실행
    stmt = con->createStatement();


    cout << "id :";
    string in = "";
    string pw = "";
    getline(cin, in);
    cout << "pw :";
    getline(cin, pw);

    res = stmt->executeQuery("SELECT id FROM user_info");

    cout << "===========id list=============" << endl;
    while (res->next() == true) {
        std::string id = res->getString("id");
        if (in == id) { cout << "!!"; }
        cout << "id :" << id.c_str() << endl;
    }

    delete stmt;






    // MySQL Connector/C++ 정리
    delete res;
    delete con;

    return 0;
}