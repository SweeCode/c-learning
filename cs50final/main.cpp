#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>
#include <random>
#include <string>
#include <array>
#include "sqlite3.h"

struct Stock {
    float price;
    std::string ticker;
    std::string owners;
    int shares;
};

Stock stock {};

int main() {
    
    // open db
    sqlite3* db;

    int fin;

    fin = sqlite3_open("financials.db", &db);

    if (fin)
    {
        std::cerr << "Could not open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    } else {
        std::cout << "Database opened Successfully" << std::endl;
    }

    std::random_device rd; 
    std::mt19937 gen(rd()); // Random Number Generator
    std::uniform_int_distribution<int> dist(1, 1000);

    std::array<int, 100> rando {};
    
    rando[1] = 1;
    sqlite3_close(db);
    return 0;
}
