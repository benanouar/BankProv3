#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <ctime>

using namespace std;
namespace fs = filesystem;
const string DATA_DIR = "/data/data/com.termux/files/home/.bankprov3";
const string DATA_FILE = "/data/data/com.termux/files/home/.bankprov3/bank.txt";
const string HISTORY_FILE = "/data/data/com.termux/files/home/.bankprov3/history.txt";
const string BACKUP_FILE = "/data/data/com.termux/files/home/.bankprov3/bank_backup.txt";
const string LAST_STATE_FILE = "/data/data/com.termux/files/home/.bankprov3/last_state.txt";
class Account {
public:
    string owner;
    int accountNumber;
    long long balance;

    void show() {
        cout << "\n-----------------" << endl;
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
void writeLog(string action);
void createBackup()
{
    ifstream src(DATA_FILE);
    writeLog("AUTO BACKUP CREATED");
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
    saveLastState();
    accounts.push_back(a);
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

    if(username == "admin" &&
       password == "1234")
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
        cout << "14. Exit\n";
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
            cout << "Goodbye!" << endl;
            saveToFile(accounts);
            break; 
        default:
            cout << "Invalid choice!" << endl;
        }

    } while (choice != 14);

    return 0;
}
