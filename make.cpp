/*
jsonファイルを読み込み、C++ファイルをコンパイルし、実行する
何らかの引数があれば、コンパイル後に実行する
*/
/*
g++ make.cpp -o make.exe -I "C:\download_libraries\json-develop\json-develop\include"
./make.exe
*/

#include <iostream>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include <fstream>

using namespace std;
using json = nlohmann::json;

int main(int argc, char* argv[]) {
    ifstream file("make.json");
    if (!file.is_open()) {
        cerr << "Failed to open make.json" << endl;
        return 1;
    }
    json config;
    try {
        file >> config;
    } catch (const json::parse_error& e) {
        cerr << "JSON parse error: " << e.what() << endl;
        return 1;
    }
    string target = config.value("target", "");
    string output = config.value("output", "a.exe");

    cout << "compile " << target << endl;
    cout << "output " << output << endl;

    int result = system(config.value("command", "").c_str());
    if (result != 0) {
        cerr << "Compilation failed with error code: " << result << endl;
        return 1;
    }
    if(argc > 1){
        result = system(("./" + output).c_str());
        if (result != 0) {
            cerr << "Execution failed with error code: " << result << endl;
            return 1;
        }
    }
    return 0;
}