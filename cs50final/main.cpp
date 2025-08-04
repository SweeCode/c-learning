#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <limits>
#include <iomanip>
#include "sqlite3.h"

const double TRANSACTION_FEE = 5.00;

struct StockItem {
    int id;
    std::string ticker;
    double price;
};

struct PortfolioItem {
    int stock_id;
    std::string ticker;
    int shares;
    double price;
    double purchase_price;
};

void execute_sql(sqlite3* db, const std::string& sql) {
    char* zErrMsg = 0;
    int rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
}

void initialize_database(sqlite3* db) {
    execute_sql(db, "CREATE TABLE IF NOT EXISTS stocks (id INTEGER PRIMARY KEY AUTOINCREMENT, ticker TEXT NOT NULL UNIQUE, price REAL NOT NULL);");
    execute_sql(db, "CREATE TABLE IF NOT EXISTS portfolio (stock_id INTEGER PRIMARY KEY, shares INTEGER NOT NULL, purchase_price REAL NOT NULL, FOREIGN KEY(stock_id) REFERENCES stocks(id));");
    execute_sql(db, "CREATE TABLE IF NOT EXISTS user (id INTEGER PRIMARY KEY, balance REAL NOT NULL);");
    execute_sql(db, "INSERT OR IGNORE INTO user (id, balance) VALUES (1, 5000.0);");
}

std::string generate_ticker() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 25);
    std::string ticker;
    for (int i = 0; i < 4; ++i) {
        ticker += (char)('A' + dis(gen));
    }
    return ticker;
}

std::vector<PortfolioItem> get_portfolio(sqlite3* db) {
    std::vector<PortfolioItem> portfolio;
    const char* sql = "SELECT s.id, s.ticker, p.shares, s.price, p.purchase_price FROM stocks s JOIN portfolio p ON s.id = p.stock_id;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            PortfolioItem item;
            item.stock_id = sqlite3_column_int(stmt, 0);
            item.ticker = (const char*)sqlite3_column_text(stmt, 1);
            item.shares = sqlite3_column_int(stmt, 2);
            item.price = sqlite3_column_double(stmt, 3);
            item.purchase_price = sqlite3_column_double(stmt, 4);
            portfolio.push_back(item);
        }
    }
    sqlite3_finalize(stmt);
    return portfolio;
}

std::vector<StockItem> get_all_stocks(sqlite3* db) {
    std::vector<StockItem> stocks;
    const char* sql = "SELECT id, ticker, price FROM stocks;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            StockItem item;
            item.id = sqlite3_column_int(stmt, 0);
            item.ticker = (const char*)sqlite3_column_text(stmt, 1);
            item.price = sqlite3_column_double(stmt, 2);
            stocks.push_back(item);
        }
    }
    sqlite3_finalize(stmt);
    return stocks;
}

void show_portfolio(sqlite3* db) {
    std::cout << "\n--- Your Portfolio ---\n";
    std::vector<PortfolioItem> portfolio = get_portfolio(db);
    if (portfolio.empty()) {
        std::cout << "Your portfolio is empty.\n";
    } else {
        for (size_t i = 0; i < portfolio.size(); ++i) {
            double gain_loss = (portfolio[i].price - portfolio[i].purchase_price) * portfolio[i].shares;
            std::cout << i + 1 << ". " << portfolio[i].ticker << " - " << portfolio[i].shares << " shares @ $" << std::fixed << std::setprecision(2) << portfolio[i].price
                      << " (Bought @ $" << portfolio[i].purchase_price << ") - Gain/Loss: " << (gain_loss >= 0 ? "+$" : "-$") << std::abs(gain_loss) << "\n";
        }
    }
    std::cout << "----------------------\n\n";
}

double get_balance(sqlite3* db) {
    double balance = 0;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, "SELECT balance FROM user WHERE id = 1;", -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            balance = sqlite3_column_double(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return balance;
}

void update_balance(sqlite3* db, double new_balance) {
    std::string sql = "UPDATE user SET balance = " + std::to_string(new_balance) + " WHERE id = 1;";
    execute_sql(db, sql);
}

void buy_stock(sqlite3* db) {
    std::cout << "Do you want to buy an (e)xisting stock or a (n)ew one? ";
    char choice;
    std::cin >> choice;

    std::string ticker;
    double price;
    int stock_id;

    if (choice == 'n') {
        ticker = generate_ticker();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(1.0, 1000.0);
        price = dis(gen);

        std::string sql = "INSERT INTO stocks (ticker, price) VALUES ('" + ticker + "', " + std::to_string(price) + ");";
        execute_sql(db, sql);
        stock_id = sqlite3_last_insert_rowid(db);

    } else if (choice == 'e') {
        std::vector<StockItem> all_stocks = get_all_stocks(db);
        if (all_stocks.empty()) {
            std::cout << "There are no stocks on the market yet." << std::endl;
            return;
        }

        std::cout << "\n--- Available Stocks ---\n";
        for (size_t i = 0; i < all_stocks.size(); ++i) {
            std::cout << i + 1 << ". " << all_stocks[i].ticker << " @ $" << std::fixed << std::setprecision(2) << all_stocks[i].price << "\n";
        }
        std::cout << "------------------------\n\n";

        std::cout << "Enter the number of the stock you want to buy: ";
        size_t index;
        while (!(std::cin >> index) || index < 1 || index > all_stocks.size()) {
            std::cout << "Invalid input. Please enter a number from 1 to " << all_stocks.size() << ": ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        StockItem& stock_to_buy = all_stocks[index - 1];
        ticker = stock_to_buy.ticker;
        price = stock_to_buy.price;
        stock_id = stock_to_buy.id;

    } else {
        std::cout << "Invalid choice." << std::endl;
        return;
    }

    std::cout << "How many shares of " << ticker << " would you like to buy? ";
    int shares;
    while (!(std::cin >> shares) || shares <= 0) {
        std::cout << "Invalid input. Please enter a positive number: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    double cost = (shares * price) + TRANSACTION_FEE;
    double balance = get_balance(db);

    if (cost > balance) {
        std::cout << "You do not have enough money to make this purchase (including a $" << TRANSACTION_FEE << " fee)." << std::endl;
        return;
    }

    update_balance(db, balance - cost);

    std::string sql = "INSERT INTO portfolio (stock_id, shares, purchase_price) VALUES (" + std::to_string(stock_id) + ", " + std::to_string(shares) + ", " + std::to_string(price) + ") ON CONFLICT(stock_id) DO UPDATE SET shares = shares + " + std::to_string(shares) + ", purchase_price = ((purchase_price * shares) + (" + std::to_string(cost) + ")) / (shares + " + std::to_string(shares) + ");";
    execute_sql(db, sql);

    std::cout << "Bought " << shares << " shares of " << ticker << " for $" << std::fixed << std::setprecision(2) << cost << " (including fee).\n";
}

void sell_stock(sqlite3* db) {
    std::vector<PortfolioItem> portfolio = get_portfolio(db);
    if (portfolio.empty()) {
        std::cout << "You have no stocks to sell.\n";
        return;
    }

    show_portfolio(db);

    std::cout << "Enter the number of the stock you want to sell: ";
    size_t index;
    while (!(std::cin >> index) || index < 1 || index > portfolio.size()) {
        std::cout << "Invalid input. Please enter a number from 1 to " << portfolio.size() << ": ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    PortfolioItem& item_to_sell = portfolio[index - 1];

    std::cout << "How many shares of " << item_to_sell.ticker << " do you want to sell? ";
    int shares_to_sell;
    while (!(std::cin >> shares_to_sell) || shares_to_sell <= 0 || shares_to_sell > item_to_sell.shares) {
        std::cout << "Invalid input. Please enter a number from 1 to " << item_to_sell.shares << ": ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    double earnings = (shares_to_sell * item_to_sell.price) - TRANSACTION_FEE;
    double balance = get_balance(db);
    update_balance(db, balance + earnings);

    if (shares_to_sell == item_to_sell.shares) {
        std::string sql = "DELETE FROM portfolio WHERE stock_id = " + std::to_string(item_to_sell.stock_id) + ";";
        execute_sql(db, sql);
    } else {
        std::string sql = "UPDATE portfolio SET shares = shares - " + std::to_string(shares_to_sell) + " WHERE stock_id = " + std::to_string(item_to_sell.stock_id) + ";";
        execute_sql(db, sql);
    }

    std::cout << "Sold " << shares_to_sell << " shares of " << item_to_sell.ticker << " for $" << std::fixed << std::setprecision(2) << earnings << " (after fee).\n";
}

void show_statistics(sqlite3* db) {
    double balance = get_balance(db);
    double portfolio_value = 0;
    double total_purchase_cost = 0;
    std::vector<PortfolioItem> portfolio = get_portfolio(db);
    for (const auto& item : portfolio) {
        portfolio_value += item.shares * item.price;
        total_purchase_cost += item.shares * item.purchase_price;
    }
    double unrealized_pl = portfolio_value - total_purchase_cost;

    std::cout << "\n--- Your Statistics ---\n";
    std::cout << "Cash Balance: $" << std::fixed << std::setprecision(2) << balance << "\n";
    std::cout << "Portfolio Value: $" << std::fixed << std::setprecision(2) << portfolio_value << "\n";
    std::cout << "Net Worth: $" << std::fixed << std::setprecision(2) << balance + portfolio_value << "\n";
    std::cout << "Unrealized P/L: " << (unrealized_pl >= 0 ? "+$" : "-$") << std::fixed << std::setprecision(2) << std::abs(unrealized_pl) << "\n";
    std::cout << "-----------------------\n\n";
}

void next_day(sqlite3* db) {
    std::vector<StockItem> all_stocks = get_all_stocks(db);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-0.1, 0.1);

    for (auto& stock : all_stocks) {
        double change_percent = dis(gen);
        stock.price += stock.price * change_percent;
        if (stock.price < 1.0) {
            stock.price = 1.0;
        }
        std::string sql = "UPDATE stocks SET price = " + std::to_string(stock.price) + " WHERE id = " + std::to_string(stock.id) + ";";
        execute_sql(db, sql);
    }
    std::cout << "A new day has begun. Stock prices have been updated." << std::endl;
}

void show_help() {
    std::cout << "\n--- Help ---\n";
    std::cout << "buy:       Buy a new or existing stock.\n";
    std::cout << "sell:      Sell a stock from your portfolio.\n";
    std::cout << "portfolio: View your current stock portfolio.\n";
    std::cout << "stats:     View your financial statistics.\n";
    std::cout << "next:      Advance to the next day.\n";
    std::cout << "reset:     Reset the game to its initial state.\n";
    std::cout << "quit:      Quit the game.\n";
    std::cout << "------------\n\n";
}

void reset_game(sqlite3* db) {
    std::cout << "Are you sure you want to reset the game? (y/n): ";
    char choice;
    std::cin >> choice;
    if (choice == 'y') {
        execute_sql(db, "DELETE FROM stocks;");
        execute_sql(db, "DELETE FROM portfolio;");
        execute_sql(db, "UPDATE user SET balance = 5000.0 WHERE id = 1;");
        std::cout << "Game has been reset." << std::endl;
    } else {
        std::cout << "Reset cancelled." << std::endl;
    }
}

int main() {
    sqlite3* db;
    if (sqlite3_open("financials.db", &db)) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }

    initialize_database(db);

    std::string command;
    while (true) {
        std::cout << "\nEnter command (buy, sell, portfolio, stats, next, help, reset, quit): ";
        std::cin >> command;

        if (command == "quit") {
            break;
        } else if (command == "buy") {
            buy_stock(db);
        } else if (command == "sell") {
            sell_stock(db);
        } else if (command == "portfolio") {
            show_portfolio(db);
        } else if (command == "stats") {
            show_statistics(db);
        } else if (command == "next") {
            next_day(db);
        } else if (command == "help") {
            show_help();
        } else if (command == "reset") {
            reset_game(db);
        } else {
            std::cout << "Unknown command.\n";
        }
    }

    sqlite3_close(db);
    return 0;
}
