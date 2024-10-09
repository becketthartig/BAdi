#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

std::string getSecondField(const std::string& line) {
    std::string field;
    bool insideQuotes = false;
    int commaCount = 0;

    for (char ch : line) {
        if (ch == '"') {
            insideQuotes = !insideQuotes;
        } else if (ch == ',' && !insideQuotes) {
            commaCount++;
            if (commaCount == 2) {
                return field;
            }
            field.clear();
            continue;
        }
        field += ch;
    }

    return ""; 
}

int main() {

    auto inittime = std::chrono::high_resolution_clock::now();

    std::ifstream file("2023-09-11.csv");
    if (!file.is_open()) {
        std::cout << "Error opening file!" << std::endl;
        return 0;
    }

    int count = 0;
    int linec = 0;
    std::string line;

    while (std::getline(file, line)) {
        linec++;

        std::string conds = getSecondField(line);
        if (conds.find("12") != std::string::npos) 
            count++;

        if (linec % 1000000 == 0)
            std::cout << "Trades analyzed: " << linec << std::endl;
    }

    std::cout << "----**** Done ****------------" << std::endl;
    std::cout << "Times Seen Condition: " << count << std::endl;

    auto ttime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - inittime);
    std::cout << "Time to execute (s): " << ttime.count() << std::endl;

    file.close();
    return 0;
}