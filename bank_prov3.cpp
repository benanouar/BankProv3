#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <ctime>
#include <algorithm>
#include <cctype>
#include <termios.h>
#include <unistd.h>
#include <iomanip>
#include <cmath>
using namespace std;
namespace fs = filesystem;
const string DATA_DIR = "/data/data/com.termux/files/home/.bankprov3";
const string DATA_FILE = "/data/data/com.termux/files/home/.bankprov3/bank.txt";
const string HISTORY_FILE = "/data/data/com.termux/files/home/.bankprov3/history.txt";
const string BACKUP_FILE = "/data/data/com.termux/files/home/.bankprov3/bank_backup.txt";
const string LAST_STATE_FILE = "/data/data/com.termux/files/home/.bankprov3/last_state.txt";
const string FAVORITES_FILE = "/data/data/com.termux/files/home/.bankprov3/favorites.txt";
const string PASSWORD_FILE = "/data/data/com.termux/files/home/.bankprov3/passwords.txt";
const string CSV_FILE = "/data/data/com.termux/files/home/.bankprov3/monthly_report.txt";
const string REPORT_FILE = "/data/data/com.termux/files/home/.bankprov3/monthly_report.txt";
const string STATEMENT_DIR = "/data/data/com.termux/files/home/.bankprov3/";
const double EUR_TO_DZD = 148.69;
const double USD_TO_DZD = 133.94;
const double USD_TO_EUR = 0.90;
const double DZD_TO_EUR = 1.0 / EUR_TO_DZD;
const double DZD_TO_USD = 1.0 / USD_TO_DZD;
const double EUR_TO_USD = 1.0 / USD_TO_EUR;
bool isFavorite(int accountNumber);
bool checkPassword(int accountNumber);
bool isStrongPassword(string password);
string inputPassword();
class Account{
public:
    string owner;
    int accountNumber;
    double  balance;
    string password;
    string currency;
    bool favorite = false;
    void show() {
        cout << "\n-----------------" << endl;
        if (isFavorite(accountNumber))
        {
            cout << "⭐ Favorite Account" << endl;
        }
        cout << "Owner: " << owner << endl;
        cout << "Account Number: " << accountNumber << endl;
        cout << fixed << setprecision(2);
        cout << "Balance: " << balance << " " << currency << endl;
       
    }


    void showBalance() {
        cout << "Current Balance: " << balance << " " << currency << endl;
    }
};

// 💾 Save
void initializeSystem()
{
    if (!fs::exists(DATA_DIR))
    {
        fs::create_directory(DATA_DIR);
    }

    if (!fs::exists(DATA_FILE))
    {
        ofstream file(DATA_FILE);
        file.close();
    }
}
string currentTime();
void setPassword(vector<Account>& accounts);
void writeLog(string action);
void savePassword(int accountNumber, string password);
void changePassword(vector<Account>& accounts);
void deletePassword(int accountNumber);
void bankInterestCalculator(vector<Account>& accounts);
void exportToCSV(vector<Account>& accounts);
void monthlyReport(vector<Account>& accounts);
void createBackup()

{
    ifstream src(DATA_FILE);
    if (!src)
        return;

    ofstream dst(BACKUP_FILE);

    dst << src.rdbuf();

    src.close();
    dst.close();
}

void saveToFile(vector<Account>& accounts) {
    
    // Backup
    createBackup();
    ifstream src(DATA_FILE);
    ofstream backup(BACKUP_FILE);

    backup << src.rdbuf();

    src.close();
    backup.close();

    // Save
    
    ofstream file(DATA_FILE);
    file << fixed << setprecision(2);
    for (Account a : accounts) {
        file << a.owner << " "
             << a.accountNumber << " "
             << a.balance << " "  
             << a.currency << endl;         
    }

    file.close();
}
// 📂 Load
void loadFromFile(vector<Account>& accounts) {
    ifstream file(DATA_FILE);

    Account a;

    while (file >> a.owner >> a.accountNumber >> a.balance >> a.currency)
    {
        accounts.push_back(a);
    }

    file.close();
}
void restoreBackup(vector<Account>& accounts)
{
    ifstream src(BACKUP_FILE);
    writeLog("RESTORE BACKUP");
    if (!src)
    {
        cout << "No backup found!" << endl;
        return;
    }

    ofstream dst(DATA_FILE);

    dst << src.rdbuf();

    src.close();
    dst.close();
    
   accounts.clear();
    loadFromFile(accounts);

    writeLog("RESTORE BACKUP");

    cout << "Backup restored successfully!" << endl;
}
string currentTime()
{
    time_t now = time(nullptr);
    string s = ctime(&now);

    if (!s.empty() && s.back() == '\n')
        s.pop_back();

    return s;
}

void writeLog(string action)
{
    ofstream file(HISTORY_FILE, ios::app);

    file << currentTime() << " | " << action << endl;

    file.close();
}
void saveLastState()
{
    ifstream src(DATA_FILE);
    ofstream dst(LAST_STATE_FILE);

    dst << src.rdbuf();

    src.close();
    dst.close();
}
double convertCurrency(double amount, string from, string to)
{
    if (from == to)
        return amount;

    if (from == "EUR" && to == "DZD")
        return amount * EUR_TO_DZD;

    if (from == "DZD" && to == "EUR")
        return amount * DZD_TO_EUR;

    if (from == "USD" && to == "DZD")
        return amount * USD_TO_DZD;

    if (from == "DZD" && to == "USD")
        return amount * DZD_TO_USD;

    if (from == "USD" && to == "EUR")
        return amount * USD_TO_EUR;

    if (from == "EUR" && to == "USD")
        return amount * EUR_TO_USD;

    return amount;
}
// ➕ Add
void addAccount(vector<Account>& accounts) {
    Account a;

    cout << "Owner: ";
    cin >> a.owner;

    cout << "Account Number: ";
    cin >> a.accountNumber;

    cout << "Balance: ";
    cin >> a.balance;
    string confirmPassword;
    while (true)
    {
        cout << "Currency (DZD/USD/EUR): ";
        cin >> a.currency;
        transform(a.currency.begin(), a.currency.end(),
    a.currency.begin(), ::toupper);

        if (a.currency == "DZD" ||
            a.currency == "USD" ||
            a.currency == "EUR")
        {
            break;
        }

        cout << "Invalid currency! Please enter DZD, USD or EUR.\n";
    }
    cout << "Create Password: ";
    a.password = inputPassword();

    cout << "Confirm Password: ";
    confirmPassword = inputPassword();

    if (a.password != confirmPassword)
    {
        cout << "Passwords do not match!" << endl;
        return;
    }
if (!isStrongPassword(a.password))
{
    cout << "\nWeak password!" << endl;
    cout << "Password must contain:" << endl;
    cout << "- At least 8 characters" << endl;
    cout << "- One uppercase letter" << endl;
    cout << "- One lowercase letter" << endl;
    cout << "- One digit" << endl;
    return;
}
    saveLastState();
    accounts.push_back(a);
    savePassword(a.accountNumber, a.password);
    saveToFile(accounts);
    writeLog("ADD | " + a.owner +
         " | Account: " +
    to_string(a.accountNumber));
    cout << "Account Added!" << endl;

}

// 📋 Show All
void showAll(vector<Account>& accounts) {
    cout << "\n--- ALL ACCOUNTS ---\n";

    for (Account a : accounts) {
        a.show();
    }
}

// 🔍 Search
void searchAccount(vector<Account>& accounts) {
    int acc;
    cout << "Enter Account Number: ";
    cin >> acc;

    for (Account &a : accounts) {
        if (a.accountNumber == acc) {
            a.show();
            return;
        }
    }

    cout << "Account not found!" << endl;
}

// 💰 Deposit
void deposit(vector<Account>& accounts) {
    int acc;
    double amount;

    cout << "Account Number: ";
    cin >> acc;

    cout << "Amount: ";
    cin >> amount;

    for (Account &a : accounts) {
        if (a.accountNumber == acc) {
            saveLastState();
            a.balance += amount;
            cout << "\nDeposit successful!\n";
            cout << "Amount: +" << amount << " " << a.currency << endl;
            cout << fixed << setprecision(2);
            saveToFile(accounts);
            writeLog(
                "DEPOSIT | Account: " +
                to_string(a.accountNumber) +
                " | +" +
                to_string(amount) + " " + a.currency
            );
            return;
        }
    }

    cout << "Account not found!" << endl;
}

// 💸 Withdraw
void withdraw(vector<Account>& accounts) {
    int acc;
    double amount;

    cout << "Account Number: ";
    cin >> acc;

    cout << "Amount: ";
    cin >> amount;

    for (Account &a : accounts) {
        if (a.accountNumber == acc) {
            if (!checkPassword(a.accountNumber))
            {
                return;
            }
            if (amount <= a.balance) {
                saveLastState();
                a.balance -= amount;
                cout << "\nWithdrawal successful!\n";
                cout << "Amount: -" << amount << " " << a.currency << endl;
                cout << fixed << setprecision(2);
                saveToFile(accounts); 
                writeLog(
                    "WITHDRAW | Account: " +
                    to_string(a.accountNumber) +
                    " | -" +
                    to_string(amount) + " " + a.currency
                );
            } else {
                cout << "Not enough balance!" << endl;
            }

            return;
        }
    }

    cout << "Account not found!" << endl;
}

// 💵 Show Balance Only
void showBalance(vector<Account>& accounts) {
    int acc;

    cout << "Account Number: ";
    cin >> acc;

    for (Account &a : accounts) {
        if (a.accountNumber == acc) {
            if (!checkPassword(a.accountNumber))
            {
                return;
            }
            a.showBalance();
            return;
        }
    }

    cout << "Account not found!" << endl;
    cout << fixed << setprecision(2);
}
void transferMoney(vector<Account>& accounts) {
    int fromAcc, toAcc;
    double amount;

    cout << "From Account: ";
    cin >> fromAcc;

    cout << "To Account: ";
    cin >> toAcc;

    cout << "Amount: ";
    cin >> amount;

    int fromIndex = -1;
    int toIndex = -1;

    for(int i = 0; i < accounts.size(); i++) {
        if(accounts[i].accountNumber == fromAcc)
            fromIndex = i;

        if(accounts[i].accountNumber == toAcc)
            toIndex = i;
    }

    if(fromIndex == -1 || toIndex == -1) {
        cout << "Account not found!" << endl;
        return;
    }
    if (!checkPassword(accounts[fromIndex].accountNumber))
    {
        return;
    }
    if(accounts[fromIndex].balance < amount) {
        cout << "Not enough balance!" << endl;
        return;
    }
    if (accounts[fromIndex].currency != accounts[toIndex].currency)
    {
    cout << "Transfer between different currencies is not allowed!" << endl;
    return;
    }
    saveLastState();
    accounts[fromIndex].balance -= amount;
    accounts[toIndex].balance += amount;

    saveToFile(accounts);
writeLog(
    "TRANSFER | From " +
    to_string(fromAcc) +
    " To " +
    to_string(toAcc) +
    " | Amount: " +
    to_string(amount) + " " + accounts[fromIndex].currency 
);
    cout << "\nTransfer successful!\n";
    cout << "Amount: " << amount << " " << accounts[fromIndex].currency << endl;
    cout << "Sender Balance: "
     << accounts[fromIndex].balance << " "
     << accounts[fromIndex].currency << endl;
    cout << "Receiver Balance: "
     << accounts[toIndex].balance << " "
     << accounts[toIndex].currency << endl;
    cout << "Transfer successful!" << endl; 
    cout << fixed << setprecision(2);
}
void deleteAccount(vector<Account>& accounts) {

    int acc;

    cout << "Account Number: ";
    cin >> acc;

    for(int i = 0; i < accounts.size(); i++) {

        if(accounts[i].accountNumber == acc) {
            if (!checkPassword(accounts[i].accountNumber))
            {
                return;
            }
            saveLastState();
            accounts.erase(accounts.begin() + i);
            deletePassword(acc);
            saveToFile(accounts);
writeLog(
    "DELETE | Account: " +
    to_string(acc)
);
            cout << "Account deleted!" << endl;

            return;
        }
    }

    cout << "Account not found!" << endl;
}
void showLog()
{
    ifstream file(HISTORY_FILE);

    if (!file)
    {
        cout << "No history.\n";
        return;
    }

    string line;

    while (getline(file, line))
    {
        cout << line << endl;
    }

    file.close();
}
void saveHistory(string text) {

    ofstream file(HISTORY_FILE, ios::app);

    file << text << endl;

    file.close();
}
void showHistory() {

    ifstream file(HISTORY_FILE);

    string line;

    cout << "\n--- HISTORY ---\n";

    while(getline(file, line)) {
        cout << line << endl;
    }

    file.close();
}
bool login() {

    string username;
    string password;

    cout << "\n===== LOGIN =====\n";

    cout << "Username: ";
    cin >> username;

    cout << "Password: ";
    password = inputPassword();

    if(username == "Anouar" &&
       password == "4321")
    {
        return true;
    }

    return false;
}


void statistics(vector<Account>& accounts)
{
    int choice;

    cout << "\n===== STATISTICS =====\n";
    cout << "Total Accounts: " << accounts.size() << endl;

    cout << "\n1. Total Bank Balance (Convert)" << endl;
    cout << "2. Balance by Currency" << endl; 
    cout << "3. Delailed Statistics" << endl;
    cout << "Choice: ";
    cin >> choice;

    if (choice == 1)
    {
        string target;

        while (true)
        {
            cout << "\nConvert Total To (DZD/USD/EUR): ";
            cin >> target;

            transform(target.begin(),
                      target.end(),
                      target.begin(),
                      ::toupper);

            if (target == "DZD" ||
                target == "USD" ||
                target == "EUR")
                break;

            cout << "Invalid currency!" << endl;
        }

        double total = 0;

        for (Account a : accounts)
        {
            total += convertCurrency(
                        a.balance,
                        a.currency,
                        target);
        }

        cout << fixed << setprecision(2);

        cout << "\n===== TOTAL BANK BALANCE =====\n";
        cout << total << " " << target << endl;
    }

    else if (choice == 2)
    {
        double dzd = 0;
        double usd = 0;
        double eur = 0;

        for (Account a : accounts)
        {
            if (a.currency == "DZD")
                dzd += a.balance;

            else if (a.currency == "USD")
                usd += a.balance;

            else if (a.currency == "EUR")
                eur += a.balance;
        }

        cout << fixed << setprecision(2);

        cout << "\n===== BALANCE BY CURRENCY =====\n";

        cout << "DZD : " << dzd << endl;
        cout << "USD : " << usd << endl;
        cout << "EUR : " << eur << endl;
    }

    else
    {
        cout << "Invalid choice!" << endl;
    }  
if (choice == 3)
{
    if (accounts.empty())
    {
        cout << "No accounts found!" << endl;
        return;
    }

    int dzdCount = 0;
    int usdCount = 0;
    int eurCount = 0;

    double totalDZD = 0;

    int highest = 0;
    int lowest = 0;

    for (int i = 0; i < accounts.size(); i++)
    {
        if (accounts[i].currency == "DZD")
            dzdCount++;
        else if (accounts[i].currency == "USD")
            usdCount++;
        else if (accounts[i].currency == "EUR")
            eurCount++;

        double value = convertCurrency(
            accounts[i].balance,
            accounts[i].currency,
            "DZD");

        totalDZD += value;

        if (value >
            convertCurrency(accounts[highest].balance,
                            accounts[highest].currency,
                            "DZD"))
        {
            highest = i;
        }

        if (value <
            convertCurrency(accounts[lowest].balance,
                            accounts[lowest].currency,
                            "DZD"))
        {
            lowest = i;
        }
    }

    cout << fixed << setprecision(2);

    cout << "\n========== DETAILED STATISTICS ==========\n";

    cout << "Total Accounts : "
         << accounts.size() << endl;

    cout << "\nAccounts by Currency\n";

    cout << "DZD : " << dzdCount << endl;
    cout << "USD : " << usdCount << endl;
    cout << "EUR : " << eurCount << endl;

    cout << "\nHighest Balance\n";

    cout << "Owner : "
         << accounts[highest].owner << endl;

    cout << "Balance : "
         << accounts[highest].balance
         << " "
         << accounts[highest].currency
         << endl;

    cout << "\nLowest Balance\n";

    cout << "Owner : "
         << accounts[lowest].owner << endl;

    cout << "Balance : "
         << accounts[lowest].balance
         << " "
         << accounts[lowest].currency
         << endl;

    cout << "\nAverage Balance (DZD): "
         << totalDZD / accounts.size()
         << " DZD" << endl;
} 
       

}
void createBackupFolder() 
{
    if (!filesystem::exists("backups")) {
        filesystem::create_directory("backups");
    }

}
void undoLastOperation(vector<Account>& accounts)
{
    ifstream src(LAST_STATE_FILE);

    if (!src)
    {
        cout << "No operation to undo!" << endl;
        return;
    }

    ofstream dst(DATA_FILE);

    dst << src.rdbuf();

    src.close();
    dst.close();

    accounts.clear();
    loadFromFile(accounts);

    writeLog("UNDO LAST OPERATION");

    cout << "Last operation has been undone." << endl;
}
void addToFavorites(int accountNumber)
{
    ifstream check(FAVORITES_FILE);

    int number;

    while (check >> number)
    {
        if (number == accountNumber)
        {
            cout << "This account is already in favorites!" << endl;
            check.close();
            return;
        }
    }

    check.close();

    ofstream file(FAVORITES_FILE, ios::app);

    if (!file)
    {
        cout << "Cannot open favorites file!" << endl;
        return;
    }

    file << accountNumber << endl;
    file.close();

    cout << "Account added to favorites!" << endl;
}
void favoriteAccount(vector<Account>& accounts)
{
    int number;
    cout << "Enter account number: ";
    cin >> number;

    for (Account &a : accounts)
    {
        if (a.accountNumber == number)
        {
            addToFavorites(number);
            return;
        }
    }

    cout << "Account not found!" << endl;
}
void showFavorites(vector<Account>& accounts)
{
    ifstream file(FAVORITES_FILE);

    if (!file)
    {
        cout << "No favorite accounts found!" << endl;
        return;
    }

    int number;
    bool found = false;

    cout << "\n===== FAVORITE ACCOUNTS =====\n";

    while (file >> number)
    {
        for (Account &a : accounts)
        {
            if (a.accountNumber == number)
            {
                a.show();
                found = true;
            }
        }
    }

    file.close();

    if (!found)
    {
        cout << "No favorite accounts found!" << endl;
    }
}
void removeFromFavorites(vector<Account>& accounts)
{
    int accountNumber;
    cout << "Enter account number: ";
    cin >> accountNumber;

    ifstream input(FAVORITES_FILE);

    if (!input)
    {
        cout << "Favorites file not found!" << endl;
        return;
    }

    vector<int> favorites;
    int number;

    while (input >> number)
    {
        favorites.push_back(number);
    }

    input.close();
    
   bool removed = false;

   for (auto it = favorites.begin();
   it != favorites.end(); )
   {
       if (*it == accountNumber)
       {
           it = favorites.erase(it);
           removed = true;
       }
       else
       {
           ++it;
       }
   }
ofstream output(FAVORITES_FILE);

for (int number : favorites)
{
    output << number << endl;
}

output.close();

if (removed)
{
    cout << "Account removed from favorites!" << endl;
}
else
{
    cout << "Account is not in favorites!" << endl;
}
    cout << "Favorites loaded successfully." << endl;
}
bool isFavorite(int accountNumber)
{
    ifstream file(FAVORITES_FILE);

    int number;

    while (file >> number)
    {
        if (number == accountNumber)
        {
            file.close();
            return true;
        }
    }

    file.close();
    return false;
}
void savePassword(int accountNumber, string password)
{
    vector<pair<int, string>> passwords;

    ifstream input(PASSWORD_FILE);

    int number;
    string pass;
    bool found = false;

    while (input >> number >> pass)
    {
        if (number == accountNumber)
        {
            passwords.push_back({accountNumber, password});
            found = true;
        }
        else
        {
            passwords.push_back({number, pass});
        }
    }

    input.close();

    if (!found)
    {
        passwords.push_back({accountNumber, password});
    }

    ofstream output(PASSWORD_FILE);

    for (auto &p : passwords)
    {
        output << p.first << " " << p.second << endl;
    }

    output.close();
}
void setPassword(vector<Account>& accounts)
{
    int accountNumber;

    cout << "Account Number";
    cin >> accountNumber;

    for (Account &a : accounts)
    {
        if (a.accountNumber == accountNumber)
        {
            string Password;
            string confirmPassword;

            cout << "New Password: ";
            Password = inputPassword();

            cout << "Confirm Password: ";
            confirmPassword = inputPassword();
            if (!isStrongPassword(Password))
            {
                cout << "\nWeak password!" << endl;
                cout << "Password must contain:" << endl;
                cout << "- At least 8 characters" << endl;
                cout << "- One uppercase letter" << endl;
                cout << "- One lowercase letter" << endl;
                cout << "- One digit" << endl;
                return;
            }
            if (Password != confirmPassword)
            {
                cout << "Passwords do not match!" << endl;
                return;
            }

            savePassword(accountNumber, Password);

            cout << "Password saved successfully!" << endl;
            return;
        }
    }

    cout << "Account not found!" << endl;
}
bool checkPassword(int accountNumber)
{
    ifstream file(PASSWORD_FILE);

    if (!file)
    {
        cout << "Password file not found!" << endl;
        return false;
    }

    int number;
    string savedPassword;
    bool found = false;

    while (file >> number >> savedPassword)
    {
        if (number == accountNumber)
        {
            found = true;
            break;
        }
    }

    file.close();

    if (!found)
    {
        cout << "Password not found!" << endl;
        return false;
    }

    for (int attempts = 3; attempts > 0; attempts--)
    {
        string enteredPassword;

        cout << "Enter Password: "; 
        enteredPassword = inputPassword();

        if (enteredPassword == savedPassword)
        {
            return true;
        }

        if (attempts > 1)
        {
            cout << "Wrong password! "
                 << attempts - 1
                 << " attempt(s) remaining."
                 << endl;
        }
    }

    cout << "Too many failed attempts!" << endl;
    return false;
}
void changePassword(vector<Account>& accounts)
{
    int accountNumber;

    cout << "Account Number: ";
    cin >> accountNumber;

    bool found = false;

    for (Account &a : accounts)
    {
        if (a.accountNumber == accountNumber)
        {
            found = true;

            if (!checkPassword(accountNumber))
            {
                return;
            }

            string newPassword;
            string confirmPassword;

            cout << "New Password: ";
            newPassword = inputPassword();

            cout << "Confirm Password: ";
            confirmPassword = inputPassword();
if (!isStrongPassword(newPassword))
{
    cout << "\nWeak password!" << endl;
    cout << "Password must contain:" << endl;
    cout << "- At least 8 characters" << endl;
    cout << "- One uppercase letter" << endl;
    cout << "- One lowercase letter" << endl;
    cout << "- One digit" << endl;
    return;
}
            if (newPassword != confirmPassword)
            {
                cout << "Passwords do not match!" << endl;
                return;
            }

            savePassword(accountNumber, newPassword);

            cout << "Password changed successfully!" << endl;
            return;
        }
    }

    if (!found)
    {
        cout << "Account not found!" << endl;
    }
}
void deletePassword(int accountNumber)
{
    vector<pair<int, string>> passwords;

    ifstream input(PASSWORD_FILE);

    int number;
    string pass;

    while (input >> number >> pass)
    {
        if (number != accountNumber)
        {
            passwords.push_back({number, pass});
        }
    }

    input.close();

    ofstream output(PASSWORD_FILE);

    for (auto &p : passwords)
    {
        output << p.first << " " << p.second << endl;
    }

    output.close();
}
bool isStrongPassword(string password)
{
    if (password.length() < 8)
        return false;

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;

    for (char c : password)
    {
        if (isupper(c))
            hasUpper = true;

        if (islower(c))
            hasLower = true;

        if (isdigit(c))
            hasDigit = true;
    }

    return hasUpper && hasLower && hasDigit;
}
string inputPassword()
{
    termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);

    newt = oldt;
    newt.c_lflag &= ~ECHO;

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    string password;
    cin >> password;

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    cout << endl;

    return password;
}
void changeCurrency(vector<Account>& accounts)
{
    int acc;
    string newCurrency;

    cout << "\n===== CHANGE CURRENCY =====\n";

    cout << "Account Number: ";
    cin >> acc;

    for (Account &a : accounts)
    {
        if (a.accountNumber == acc)
        {
            if (!checkPassword(a.accountNumber))
                return;

            cout << "Current Currency: " << a.currency << endl;
            cout << "Current Balance : "
                 << fixed << setprecision(2)
                 << a.balance << " " << a.currency << endl;

            while (true)
            {
                cout << "\nNew Currency (DZD/USD/EUR): ";
                cin >> newCurrency;

                transform(newCurrency.begin(),
                          newCurrency.end(),
                          newCurrency.begin(),
                          ::toupper);

                if (newCurrency == "DZD" ||
                    newCurrency == "USD" ||
                    newCurrency == "EUR")
                    break;

                cout << "Invalid currency!\n";
            }

            if (newCurrency == a.currency)
            {
                cout << "Account is already in "
                     << newCurrency << endl;
                return;
            }

            double oldBalance = a.balance;
            string oldCurrency = a.currency;

            a.balance = convertCurrency(
                            a.balance,
                            a.currency,
                            newCurrency);

            a.currency = newCurrency;

            saveToFile(accounts);

            writeLog(
                "CHANGE CURRENCY | Account: " +
                to_string(a.accountNumber) +
                " | " +
                oldCurrency +
                " -> " +
                newCurrency
            );

            cout << "\nCurrency changed successfully!\n";

            cout << "Old Balance : "
                 << oldBalance
                 << " "
                 << oldCurrency
                 << endl;

            cout << "New Balance : "
                 << a.balance
                 << " "
                 << a.currency
                 << endl;

            return;
        }
    }

    cout << "Account not found!\n";
}
void bankInterestCalculator(vector<Account>& accounts)
{
    int choice;

    cout << "\n===== BANK INTEREST CALCULATOR =====\n";
    cout << "1. Simple Interest\n";
    cout << "2. Compound Interest\n";
    cout << "3. Back\n";
    cout << "Choice: ";
    cin >> choice;

    if (choice == 3)
        return;

    int acc;

    cout << "\nAccount Number: ";
    cin >> acc;

    for (Account &a : accounts)
    {
        if (a.accountNumber == acc)
        {
            if (!checkPassword(a.accountNumber))
                return;

            cout << fixed << setprecision(2);

            if (choice == 1)
            {
double rate;
int years;
char apply;

cout << "\nCurrent Balance : "
     << a.balance << " "
     << a.currency << endl;

cout << "Interest Rate (%): ";
cin >> rate;

cout << "Years: ";
cin >> years;

double interest =
    a.balance * (rate / 100.0) * years;

double finalBalance =
    a.balance + interest;

cout << "\n========== SIMPLE INTEREST ==========\n";

cout << "Interest     : "
     << interest << " "
     << a.currency << endl;

cout << "Final Balance: "
     << finalBalance << " "
     << a.currency << endl;

cout << "\nApply Interest? (Y/N): ";
cin >> apply;

if (toupper(apply) == 'Y')
{
    saveLastState();

    a.balance = finalBalance;

    saveToFile(accounts);

    writeLog(
        "SIMPLE INTEREST | Account: " +
        to_string(a.accountNumber) +
        " | +" +
        to_string(interest) +
        " " +
        a.currency
    );

    cout << "\nInterest applied successfully!\n";
}
else
{
    cout << "\nNo changes were made.\n";
}
}
else  if (choice == 2)
{
    double rate;
    int years;
    int compounds;
    char apply;

    cout << "\nCurrent Balance : "
         << a.balance << " "
         << a.currency << endl;

    cout << "Interest Rate (%): ";
    cin >> rate;

    cout << "Years: ";
    cin >> years;

    cout << "\nCompounds Per Year\n";
    cout << "1   - Yearly\n";
    cout << "2   - Semi-Annual\n";
    cout << "4   - Quarterly\n";
    cout << "12  - Monthly\n";
    cout << "365 - Daily\n";
    cout << "Choice: ";
    cin >> compounds;

    if (compounds <= 0)
    {
        cout << "Invalid value!" << endl;
        return;
    }

    double finalBalance =
        a.balance *
        pow(1 + (rate / 100.0) / compounds,
            compounds * years);

    double interest =
        finalBalance - a.balance;

    cout << "\n========== COMPOUND INTEREST ==========\n";

    cout << "Interest     : "
         << interest << " "
         << a.currency << endl;

    cout << "Final Balance: "
         << finalBalance << " "
         << a.currency << endl;

    cout << "\nApply Interest? (Y/N): ";
    cin >> apply;

    if (toupper(apply) == 'Y')
    {
        saveLastState();

        a.balance = finalBalance;

        saveToFile(accounts);

        writeLog(
            "COMPOUND INTEREST | Account: " +
            to_string(a.accountNumber) +
            " | +" +
            to_string(interest) +
            " " +
            a.currency
        );

        cout << "\nCompound interest applied successfully!\n";
    }
    else
    {
        cout << "\nNo changes were made.\n";
    }
}

return;
        }
    }

    cout << "Account not found!" << endl;
}
void exportToCSV(vector<Account>& accounts)
{
    int choice;

    cout << "\n===== EXPORT TO CSV =====\n";
    cout << "1. Export All Accounts\n";
    cout << "2. Export One Account\n";
    cout << "3. Export By Currency\n";
    cout << "4. Back\n";
    cout << "Choice: ";
    cin >> choice;

    if (choice == 4)
        return;

    cout << fixed << setprecision(2);

    if (choice == 1)
    {
        ofstream file(CSV_FILE);
        file << fixed << setprecision(2);
        if (accounts.empty())
        {
            cout << "No accounts to export!" << endl;
            return;
        }
        if (!file)
        {
            cout << "Cannot create CSV file!" << endl;
            return;
        }

        time_t now = time(nullptr);

        file << "Bank Pro v3 Export\n";
        file << "Date," << ctime(&now);
        file << "\n";

        file << "Owner,Account Number,Balance,Currency\n";

        int exported = 0;

        for (Account &a : accounts)
        {
            file << a.owner << ","
                 << a.accountNumber << ","
                 << a.balance << ","
                 << a.currency << "\n";

            exported++;
        }

        file.close();

        writeLog("EXPORT CSV | ALL");

        cout << "\nAccounts exported successfully!\n";
        cout << "Exported Accounts: "
             << exported
             << endl;

        cout << "File: "
             << CSV_FILE
             << endl;
}
else if (choice == 2)
{
    int acc;

    cout << "\nAccount Number: ";
    cin >> acc;

    bool found = false;

    for (Account &a : accounts)
    {
        if (a.accountNumber == acc)
        {
            found = true;

            string fileName =
                DATA_DIR + "/account_" +
                to_string(acc) +
                ".csv";

            ofstream file(fileName);
            file << fixed << setprecision(2);
            if (!file)
            {
                cout << "Cannot create CSV file!" << endl;
                return;
            }

            time_t now = time(nullptr);

            file << "Bank Pro v3 Export\n";
            file << "Date," << ctime(&now);
            file << "\n";

            file << "Owner,Account Number,Balance,Currency\n";

            file << fixed << setprecision(2);

            file << a.owner << ","
                 << a.accountNumber << ","
                 << a.balance << ","
                 << a.currency << "\n";

            file.close();

            writeLog(
                "EXPORT CSV | Account: " +
                to_string(acc)
            );

            cout << "\nAccount exported successfully!\n";
            cout << "File: " << fileName << endl;

            break;
        }
    }

    if (!found)
    {
        cout << "Account not found!" << endl;
    }
}    
else if (choice == 3)
{
    string currency;

    cout << "\nCurrency (DZD/USD/EUR): ";
    cin >> currency;
    
    transform(currency.begin(),
              currency.end(),
              currency.begin(),
              ::toupper);

    string fileName =
        DATA_DIR + "/accounts_" +
        currency +
        ".csv";

    ofstream file(fileName); 
    file << fixed << setprecision(2);
if (currency != "DZD" &&
    currency != "USD" &&
    currency != "EUR")
{
    cout << "Invalid currency!" << endl;
    return;
}
    if (!file)
    {
        cout << "Cannot create CSV file!" << endl;
        return;
    }

    time_t now = time(nullptr);

    file << "Bank Pro v3 Export\n";
    file << "Date," << ctime(&now);
    file << "\n";

    file << "Owner,Account Number,Balance,Currency\n";

    file << fixed << setprecision(2);

    int exported = 0;

    for (Account &a : accounts)
    {
        if (a.currency == currency)
        {
            file << a.owner << ","
                 << a.accountNumber << ","
                 << a.balance << ","
                 << a.currency << "\n";

            exported++;
        }
    }

    file.close();

    if (exported == 0)
    {
        cout << "No accounts found with this currency!" << endl;
        return;
    }

    writeLog(
        "EXPORT CSV | Currency: " + currency
    );

    cout << "\nAccounts exported successfully!\n";
    cout << "Exported Accounts: "
         << exported << endl;

    cout << "File: "
         << fileName << endl;
}
else
{
    cout << "Invalid choice!" << endl;
}
}
void monthlyReport(vector<Account>& accounts)
{
    if (accounts.empty())
    {
        cout << "No accounts found!" << endl;
        return;
    }

    double totalEUR = 0;
    double totalUSD = 0;
    double totalDZD = 0;

    int totalAccounts = accounts.size();

    Account highest = accounts[0];
    Account lowest = accounts[0];

    for (Account &a : accounts)
    {
        if (a.currency == "EUR")
            totalEUR += a.balance;
        else if (a.currency == "USD")
            totalUSD += a.balance;
        else if (a.currency == "DZD")
            totalDZD += a.balance;

        if (a.balance > highest.balance)
            highest = a;

        if (a.balance < lowest.balance)
            lowest = a;
    }
    // ===== Converted Totals =====

    double totalInDZD =
        totalDZD +
        (totalEUR * 148.69) +
        (totalUSD * 133.94);

    double totalInEUR =
        (totalDZD / 148.69) +
        totalEUR +
        (totalUSD * 0.90);

    double totalInUSD =
        (totalDZD / 133.94) +
        (totalEUR / 0.90) +
        totalUSD;
    cout << fixed << setprecision(2);

    cout << "\n========== MONTHLY REPORT ==========\n";

    cout << "Date: " << currentTime() << endl;

    cout << "\nTotal Accounts: "
         << totalAccounts << endl;

    cout << "\n----- BALANCES BY CURRENCY -----\n";

    cout << "EUR : " << totalEUR << endl;
    cout << "USD : " << totalUSD << endl;
    cout << "DZD : " << totalDZD << endl;

    cout << "\n----- CONVERTED TOTALS -----\n";

    cout << "EUR : " << totalInEUR << endl;
    cout << "USD : " << totalInUSD << endl;
    cout << "DZD : " << totalInDZD << endl;

    cout << "\n----- HIGHEST BALANCE -----\n";
    cout << "Owner   : " << highest.owner << endl;
    cout << "Account : " << highest.accountNumber << endl;
    cout << "Balance : "
         << highest.balance << " "
         << highest.currency << endl;

    cout << "\n----- LOWEST BALANCE -----\n";
    cout << "Owner   : " << lowest.owner << endl;
    cout << "Account : " << lowest.accountNumber << endl;
    cout << "Balance : "
         << lowest.balance << " "
         << lowest.currency << endl;
    ofstream file(REPORT_FILE);

    if (file)
    {
        file << fixed << setprecision(2);

        file << "========== MONTHLY REPORT ==========\n";
        file << "Date: " << currentTime() << "\n\n";

        file << "Total Accounts: "
             << totalAccounts << "\n\n";

        file << "----- BALANCES BY CURRENCY -----\n";
        file << "EUR : " << totalEUR << "\n";
        file << "USD : " << totalUSD << "\n";
        file << "DZD : " << totalDZD << "\n\n";

        file << "----- CONVERTED TOTALS -----\n";
        file << "EUR : " << totalInEUR << "\n";
        file << "USD : " << totalInUSD << "\n";
        file << "DZD : " << totalInDZD << "\n\n";

        file << "----- HIGHEST BALANCE -----\n";
        file << "Owner   : " << highest.owner << "\n";
        file << "Account : " << highest.accountNumber << "\n";
        file << "Balance : "
             << highest.balance << " "
             << highest.currency << "\n\n";

        file << "----- LOWEST BALANCE -----\n";
        file << "Owner   : " << lowest.owner << "\n";
        file << "Account : " << lowest.accountNumber << "\n";
        file << "Balance : "
             << lowest.balance << " "
             << lowest.currency << "\n";

        file.close();
    }

    writeLog("MONTHLY REPORT GENERATED");

    cout << "\nMonthly report generated successfully!\n";
    cout << "File: " << REPORT_FILE << endl;
         
}
void accountStatement(vector<Account>& accounts)
{
    int acc;

    cout << "\n===== ACCOUNT STATEMENT =====\n";
    cout << "Account Number: ";
    cin >> acc;

    for (Account &a : accounts)
    {
        if (a.accountNumber == acc)
        {
            if (!checkPassword(a.accountNumber))
                return;

            string fileName =
                STATEMENT_DIR +
                "statement_" +
                to_string(acc) +
                ".txt";

            ofstream file(fileName);

            if (!file)
            {
                cout << "Cannot create statement!" << endl;
                return;
            }

            cout << fixed << setprecision(2);
            file << fixed << setprecision(2);

            file << "========== ACCOUNT STATEMENT ==========\n\n";

file << "Date: "
     << currentTime()
     << "\n\n";

file << "Owner          : "
     << a.owner
     << "\n";

file << "Account Number : "
     << a.accountNumber
     << "\n";

file << "Currency       : "
     << a.currency
     << "\n";

file << "Current Balance: "
     << a.balance << " "
     << a.currency
     << "\n\n";

file << "=======================================\n";
file << "Transactions\n";
file << "=======================================\n\n";
ifstream history(HISTORY_FILE);

string line;
int transactionCount = 0;
while (getline(history, line))
{
    if (line.find("Account: " + to_string(acc)) != string::npos ||
        line.find("From " + to_string(acc)) != string::npos ||
        line.find("To " + to_string(acc)) != string::npos)
    {
        transactionCount++;

        file << transactionCount
             << ". "
             << line << endl;

        cout << transactionCount
             << ". "
             << line << endl;
    }
}
file << "\n---------------------------------------\n";
file << "Total Transactions: "
     << transactionCount
     << "\n";

cout << "\nTotal Transactions: "
     << transactionCount
     << endl;
history.close();
cout << "\n========== ACCOUNT STATEMENT ==========\n";

cout << "Owner          : "
     << a.owner << endl;

cout << "Account Number : "
     << a.accountNumber << endl;

cout << "Currency       : "
     << a.currency << endl;

cout << "Current Balance: "
     << a.balance << " "
     << a.currency << endl;

cout << "\nCollecting transactions...\n";
            file.close();

            writeLog("ACCOUNT STATEMENT | Account: " +
                     to_string(acc));

            cout << "\nStatement created successfully!\n";
            cout << "File: " << fileName << endl;

            return;
        }
    }

    cout << "Account not found!" << endl;
}
int main () {
cout << fixed << setprecision(2);
    if(!login()) {
        return 0;
    }
    initializeSystem();
    vector<Account> accounts;

    loadFromFile(accounts);

    int choice;

    do {
        cout << "\n===== BANK PRO v3 =====\n";
        cout << "1. Add Account\n";
        cout << "2. Show All Accounts\n";
        cout << "3. Search Account\n";
        cout << "4. Deposit\n";
        cout << "5. Withdraw\n";
        cout << "6. Show Balance\n";
        cout << "7. Transfer Money\n";
        cout << "8. Delete Account\n";
        cout << "9. Show History\n";
        cout << "10. Statistics\n";
        cout << "11. Restore Backup\n";
        cout << "12. Activity Log\n";
        cout << "13. Undo Last Operation\n";
        cout << "14. Add to Favorites\n";
        cout << "15. Show Favorites\n";
        cout << "16. Remove from Favorites\n";
        cout << "17. Set Password\n";
        cout << "18. Change Password\n";
        cout << "19. Change Currency\n";
        cout << "20. Bank Interest calculator\n";
        cout << "21. Export to CSV\n";  
        cout << "22. Monthly Report \n";
        cout << "23. Account Statement\n";
        cout << "24. Exit\n";
        cout << "Choice: ";

        cin >> choice;

        switch (choice) {

        case 1:
            addAccount(accounts);
            break;

        case 2:
            showAll(accounts);
            break;

        case 3:
            searchAccount(accounts);
            break;

        case 4:
            deposit(accounts);
            break;

        case 5:
            withdraw(accounts);
            break;

        case 6:
            showBalance(accounts);
            break;

        case 7:
            transferMoney(accounts);
            break;
        case 8:
            deleteAccount(accounts);
            break;
        case 9:
            showHistory();
            break;
        case 10:
            statistics(accounts);
            break;
        case 11:
            restoreBackup(accounts);
            break;
        case 12: 
            showLog();
            break;

        case 13:
            undoLastOperation(accounts);
            break;

        case 14:
            favoriteAccount(accounts);
            break;
        case 15:
            showFavorites(accounts);
            break;
        case 16:
            removeFromFavorites(accounts);
            break;
        case 17:
            setPassword(accounts);
            break;
        case 18:
            changePassword(accounts); 
            break;
        case 19:
            changeCurrency(accounts);
            break;
        case 20:
            bankInterestCalculator(accounts);
            break;
        case 21:
            exportToCSV(accounts);
            break;
        case 22:
            monthlyReport(accounts);
            break;
        case 23:
            accountStatement(accounts);
            break;
        case 24:
            cout << "Goodbye!" << endl;
            saveToFile(accounts);
            break; 
        default:
            cout << "Invalid choice!" << endl;
        }

    } while (choice != 24);

    return 0;
}
