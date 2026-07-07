#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <ctime>
#include <algorithm>
using namespace std;
namespace fs = filesystem;
const string DATA_DIR = "/data/data/com.termux/files/home/.bankprov3";
const string DATA_FILE = "/data/data/com.termux/files/home/.bankprov3/bank.txt";
const string HISTORY_FILE = "/data/data/com.termux/files/home/.bankprov3/history.txt";
const string BACKUP_FILE = "/data/data/com.termux/files/home/.bankprov3/bank_backup.txt";
const string LAST_STATE_FILE = "/data/data/com.termux/files/home/.bankprov3/last_state.txt";
const string FAVORITES_FILE = "/data/data/com.termux/files/home/.bankprov3/favorites.txt";
const string PASSWORD_FILE = "/data/data/com.termux/files/home/.bankprov3/passwords.txt";
bool isFavorite(int accountNumber);
bool checkPassword(int accountNumber);
class Account{
public:
    string owner;
    int accountNumber;
    long long balance;
    string password;
    bool favorite = false;
    void show() {
        cout << "\n-----------------" << endl;
        if (isFavorite(accountNumber))
        {
            cout << "⭐ Favorite Account" << endl;
        }
        cout << "Owner: " << owner << endl;
        cout << "Account Number: " << accountNumber << endl;
        cout << "Balance: " << balance << endl;
       
    }


    void showBalance() {
        cout << "Current Balance: " << balance << endl;
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

    for (Account a : accounts) {
        file << a.owner << " "
             << a.accountNumber << " "
             << a.balance << endl; 
             
    }

    file.close();
}
// 📂 Load
void loadFromFile(vector<Account>& accounts) {
    ifstream file(DATA_FILE);

    Account a;

    while (file >> a.owner >> a.accountNumber >> a.balance)
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

    cout << "Create Password: ";
    cin >> a.password;

    cout << "Confirm Password: ";
    cin >> confirmPassword;

    if (a.password != confirmPassword)
    {
        cout << "Passwords do not match!" << endl;
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
            cout << "Deposit successful!" << endl;
            saveToFile(accounts);
            writeLog(
                "DEPOSIT | Account: " +
                to_string(a.accountNumber) +
                " | +" +
                to_string(amount)
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
                cout << "Withdrawal successful!" << endl;
                saveToFile(accounts); 
                writeLog(
                    "WITHDRAW | Account: " +
                    to_string(a.accountNumber) +
                    " | -" +
                    to_string(amount)
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

    for (Account a : accounts) {
        if (a.accountNumber == acc) {
            a.showBalance();
            return;
        }
    }

    cout << "Account not found!" << endl;
}
void transferMoney(vector<Account>& accounts) {
    int fromAcc, toAcc;
    long long amount;

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

    if(accounts[fromIndex].balance < amount) {
        cout << "Not enough balance!" << endl;
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
    to_string(amount)
);
    cout << "Transfer successful!" << endl;
}
void deleteAccount(vector<Account>& accounts) {

    int acc;

    cout << "Account Number: ";
    cin >> acc;

    for(int i = 0; i < accounts.size(); i++) {

        if(accounts[i].accountNumber == acc) {
            saveLastState();
            accounts.erase(accounts.begin() + i);

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
    cin >> password;

    if(username == "Anouar" &&
       password == "4321")
    {
        return true;
    }

    return false;
}


void statistics(vector<Account>& accounts) {

    cout << "\n===== STATISTICS =====\n";

    cout << "Total Accounts: "
         << accounts.size()
         << endl;

    long long totalMoney = 0;

    for(Account a : accounts) {
        totalMoney += a.balance;
    }

    cout << "Total Bank Money "
         << totalMoney
         << endl;
}
void createBackupFolder() {

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
    ofstream file(PASSWORD_FILE, ios::app);

    if (!file)
    {
        cout << "Cannot open password file!" << endl;
        return;
    }

    file << accountNumber << " " << password << endl;

    file.close();
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
    string enteredPassword;

    cout << "Enter Password: ";
    cin >> enteredPassword;

    while (file >> number >> savedPassword)
    {
        if (number == accountNumber)
        {
            file.close();

            if (enteredPassword == savedPassword)
                return true;

            cout << "Wrong password!" << endl;
            return false;
        }
    }

    file.close();

    cout << "Password not found!" << endl;
    return false;
}
void setPassword(vector<Account>& accounts)
{
    int accountNumber;

    cout << "Account Number: ";
    cin >> accountNumber;

    for (Account &a : accounts)
    {
        if (a.accountNumber == accountNumber)
        {
            string password;
            string confirm;

            cout << "New Password: ";
            cin >> password;

            cout << "Confirm Password: ";
            cin >> confirm;

            if (password != confirm)
            {
                cout << "Passwords do not match!" << endl;
                return;
            }

            savePassword(accountNumber, password);

            cout << "Password saved successfully!" << endl;
            return;
        }
    }

    cout << "Account not found!" << endl;
}
int main () {

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
        cout << "13.Undo Last Operation\n";
        cout << "14. Add to Favorites\n";
        cout << "15. Show Favorites\n";
        cout << "16. Remove from Favorites\n";
        cout << "17. Set Password\n";
        cout << "18. Exit\n";
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
            cout << "Goodbye!" << endl;
            saveToFile(accounts);
            break; 
        default:
            cout << "Invalid choice!" << endl;
        }

    } while (choice != 18);

    return 0;
}
