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

    // 데이터베이스 선택
    con->setSchema("final");
    
    // db 한글 저장을 위한 셋팅 
    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    // 데이터베이스 쿼리 실행
    stmt = con->createStatement();
    /*stmt->execute("DROP TABLE IF EXISTS user_info");// 채팅 시작 할때 생성해야 됨
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
                is_there_same = true; cout << "이미 존재하는 아이디 입니다." << endl;
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
        cout << "가입완료" << endl;
        delete pstmt;
    }


    cout << "===========id list=============" << endl;
    
    while (res->next() == true) {
        std::string id = res->getString("id");
        if (in == id) { is_there_same = true; }
    }
    delete stmt;

    // MySQL Connector/C++ 정리
    delete res;

    delete con;

    return 0;
}