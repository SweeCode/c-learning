# Stock Market Simulator

#### Video Demo:  <(https://www.youtube.com/watch?v=8F5zsI7KUMA)>

#### Description:

This project is a command-line based stock market simulation game written in C++. It provides a dynamic and engaging experience where you can buy and sell stocks, track your portfolio's performance, and navigate a fluctuating market. The goal is to grow your net worth by making smart investment decisions. The simulation is designed to be both entertaining and educational, offering a simplified yet realistic introduction to the world of stock trading.

### Core Features

*   **Dynamic Market:** The stock market is constantly changing. With every new day, stock prices fluctuate randomly, creating a challenging environment where you must adapt your strategy to succeed.
*   **User-Friendly Interface:** The game is controlled through a simple and intuitive command-line interface. Commands are straightforward and easy to learn, and a built-in `help` command is available to guide you.
*   **Portfolio Management:** You can buy and sell stocks to build and manage your portfolio. The game tracks your holdings, including the number of shares and the price at which you purchased them.
*   **Performance Tracking:** The simulation provides detailed statistics on your financial performance, including your cash balance, the total value of your portfolio, and your overall net worth. It also calculates and displays your unrealized profit or loss, giving you a clear picture of how your investments are doing.
*   **Realistic Transactions:** To add a layer of realism, the game includes a flat transaction fee for every buy and sell order. This encourages you to make thoughtful trades and consider the costs of trading.

### How to Play

When you start the game, you are given a balance of $5,000. From there, you can use the following commands to interact with the simulation:

*   `buy`: Purchase shares of a stock. You can choose to discover a new, randomly generated stock or buy shares of a stock that is already on the market.
*   `sell`: Sell shares of a stock from your portfolio. The game will display a numbered list of your holdings, and you can choose which one to sell by its index.
*   `portfolio`: View your current stock portfolio. This will show you a detailed breakdown of your holdings, including the number of shares, the average purchase price, the current market price, and your gain or loss for each stock.
*   `stats`: Display your financial statistics. This includes your cash balance, portfolio value, net worth, and total unrealized profit or loss.
*   `next`: Advance the simulation to the next day. This will cause the stock prices to fluctuate, so be sure to check your portfolio to see how the market has changed.
*   `help`: Display a list of all available commands and what they do.
*   `reset`: Start a new game. This will wipe all existing data and reset your balance to the initial $5,000.
*   `quit`: Exit the game.

### Design and Implementation

The project is written in C++ and uses the SQLite library to manage the game's data. The decision to use SQLite was based on its simplicity and efficiency. It allows for easy storage and retrieval of stock data, portfolio holdings, and user information without the need for a complex database setup.

The core of the simulation is the `main.cpp` file, which contains all the game logic. This includes functions for initializing the database, managing user commands, and handling the game's state. The code is organized into several functions, each responsible for a specific task, such as buying or selling stocks, displaying the portfolio, or updating stock prices.

One of the key design choices was to make the stock market dynamic. This was achieved by implementing the `next_day` function, which randomly adjusts the price of each stock on the market. This adds an element of unpredictability and makes the simulation more engaging. Another important feature is the detailed performance tracking, which was implemented by storing the purchase price of each stock and calculating the profit or loss based on the current market price.

Overall, the project is a well-rounded and feature-complete stock market simulation that provides a fun and educational experience. It is a great example of a semi-beginner C++ project that demonstrates a solid understanding of programming concepts and software design principles.
