#include <iostream>
#include <string>
#include <mysql/jdbc.h>

using std::cout;
using std::cin;
using std::endl;
using std::string;

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "(()()&pj0907"; // 데이터베이스 접속 비밀번호

int main()
{
    // MySQL Connector/C++ 초기화
    sql::mysql::MySQL_Driver* driver; // 추후 해제하지 않아도 Connector/C++가 자동으로 해제해 줌
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

    // 데이터베이스 선택
    con->setSchema("kdt");

    // db 한글 저장을 위한 셋팅 
    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    // 데이터베이스 쿼리 실행
    stmt = con->createStatement();
    stmt->execute("DROP TABLE IF EXISTS chatting"); // DROP
    cout << "Finished dropping table (if existed)" << endl;
    stmt->execute("CREATE TABLE chatting (id serial PRIMARY KEY, 내용 VARCHAR(255));"); // CREATE
    cout << "Finished creating table" << endl;
    delete stmt;

    pstmt = con->prepareStatement("INSERT INTO chatting(내용) VALUES(?)"); // INSERT
    cout << "채팅이 시작되었습니다.";
    cout << "\n";
    string a;
    while (a != "c") {
        cin >> a;
        cout << "\n";
        pstmt->setString(1, a);
        pstmt->execute();
        //cout << "One row inserted." << endl;

    }


    // MySQL Connector/C++ 정리
    delete pstmt;
    delete con;

    return 0;
}