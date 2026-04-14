#include <iostream>
#include <windows.h>
#include "MenuHandlers.h"

using namespace std;

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    initializeSystem();

    int choice;
    while (true) {
        cout << "\n========== CE WATCH STORE ==========" << endl;
        cout << "Current User: " << currentUser.name << (currentUser.role == 1 ? " [ADMIN]" : "") << endl;
        cout << "------------------------------------" << endl;
        cout << "1. View Catalog" << endl;
        cout << "2. Register New User" << endl;
        cout << "3. Login" << endl;

        if (currentUser.role == 1) {
            cout << "4. [Admin] Add Product" << endl;
        }

        cout << "0. Exit" << endl;
        cout << "Choice: ";
        
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        if (choice == 0) break;

        switch (choice) {
            case 1: showCatalogFlow(); break;
            case 2: registerUserFlow(); break;
            case 3: loginFlow(); break;
            case 4: 
                if (currentUser.role == 1) addProductFlow();
                else cout << "[Access Denied]" << endl;
                break;
            default: cout << "Invalid choice." << endl;
        }
    }
    return 0;
}