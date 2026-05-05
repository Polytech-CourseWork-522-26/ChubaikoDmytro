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

        if (currentUser.id != -1) {
            if (currentUser.role == 1) {
                cout << "4. [Admin] Add Product" << endl;
                cout << "5. [Admin] View All Orders" << endl;
                cout << "6. [Admin] View All Users" << endl;
            } else {
                cout << "4. My Cart & Checkout" << endl;
                cout << "5. My Order History" << endl;
            }
        }

        cout << "0. Exit" << endl;
        cout << "Choice: ";
        
        if (!(cin >> choice)) {
            cin.clear(); cin.ignore(1000, '\n'); continue;
        }

        if (choice == 0) break;

        switch (choice) {
            case 1: showCatalogFlow(); break;
            case 2: registerUserFlow(); break;
            case 3: loginFlow(); break;
            case 4: 
                if (currentUser.role == 1) addProductFlow();
                else if (currentUser.id != -1) viewCartFlow(); // Кошик та оформлення
                break;
            case 5: 
                if (currentUser.role == 1) manageOrdersFlow(); // Адмін: одобрення/відхилення
                else if (currentUser.id != -1) viewMyOrdersFlow(); // Юзер: його історія
                break;
            case 6: 
                if (currentUser.role == 1) viewAllUsersFlow(); 
                break;
            default: cout << "Invalid choice." << endl;
        }
    }
    return 0;
}