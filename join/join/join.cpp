#include <iostream>
#include <string>
#include <mysql/jdbc.h>
#include <windows.h>
#include <vector>
#include <regex>
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
    int specialList[12] = { '!','@','#','$','%','^','&','*','(',')','-','+' };
    // MySQL Connector/C++ 초기화
    string in = "";
    sql::mysql::MySQL_Driver* driver; // 추후 해제하지 않아도 Connector/C++가 자동으로 해제해 줌
    sql::Connection* con;
    sql::Statement* stmt;
    sql::PreparedStatement* pstmt;
    sql::ResultSet* res = NULL;
    vector<string> user_info = { "아이디","이름","비밀번호(영어,숫자,특수문자 조합)","birth(yyyy-mm-dd)","연락처 (010-xxxx-xxxx)","email","address" };
    int choice = 0;
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
    stmt->execute("CREATE TABLE user_info (id VARCHAR(50), name VARCHAR(50), pw VARCHAR(50), birth date, num VARCHAR(50), email VARCHAR(50), address VARCHAR(50));"); // CREATE
    cout << "Finished creating table" << endl;
    pstmt = con->prepareStatement("INSERT INTO user_info(id,name,pw,birth,num,email,address) VALUES(?,?,?,?,?,?,?)"); // INSERT
        pstmt->setString(1, user_info[0]); // 초기화
        pstmt->setString(2, user_info[1]);
        pstmt->setString(3, user_info[2]);
        pstmt->setString(4, "1999-12-31");
        pstmt->setString(5, user_info[0]);
        pstmt->setString(6, user_info[0]);
        pstmt->setString(7, user_info[0]);
    pstmt->execute();
    */
    bool numberCheck = false;  // 숫자 check
    bool englishCheck = false; // 영어 check
    bool specialCheck = false; // 특수 check
    
    while (is_there_same) {
        in = "";
        cout << "============회원가입==============" << endl;
        cout << "아이디 :";
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
            res = stmt->executeQuery("SELECT id FROM user_info");
            while (res->next()||res->isLast()) {
                std::string id = res->getString("id");
                if (in == id) {
                    cout << "이미 존재하는 아이디 입니다." << endl;
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
            cout << "숫자와 영어의 조합으로 id를 생성하세요." << endl;
            numberCheck = false;  // 숫자 check
            englishCheck = false; // 영어 check
            specialCheck = false; // 특수 check
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
                    cout << "비밀번호는 [영어,숫자,특수문자]로 이루어져야 합니다." << endl;
                    numberCheck = false;  // 숫자 check
                    englishCheck = false; // 영어 check
                    specialCheck = false; // 특수 check
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
                    cout << "형식이 맞지 않습니다." << endl;
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
                if (!(regex_match(input,phone))) {
                    cout << "형식이 맞지 않습니다." << endl;
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
                    cout << "형식이 맞지 않습니다." << endl;
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
        pstmt->setString(i+1, user_info[i]);
    }
    pstmt->execute();
    delete pstmt;
    cout << "가입완료" << endl;
    delete stmt;
    delete res;
    delete con;

    return 0;
}