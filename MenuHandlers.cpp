#include <iostream>
#include <cstring>
#include "MenuHandlers.h"
#include "Users/UserRepository.h"
#include "Product/ProductRepository.h"

using namespace std;

User currentUser;

void initializeSystem() {
    currentUser.id = -1;
    currentUser.role = 0;
    strcpy(currentUser.name, "Guest");
}

void showCatalogFlow() {
    Product list[100];
    int count = getAllProducts(list, 100);
    
    cout << "\n--- CE WATCH CATALOG ---" << endl;
    if (count == 0) {
        cout << "Catalog is empty." << endl;
    } else {
        for (int i = 0; i < count; i++) {
            cout << i + 1 << ". " << list[i].modelName 
                 << " | Price: $" << list[i].price 
                 << " | Status: " << (list[i].status == 1 ? "In Stock" : "Out of Stock") << endl;
            cout << "   Description: " << list[i].description << endl;
            cout << "------------------------------------" << endl;
        }
    }
    cout << "\nPress Enter to continue...";
    cin.ignore(1000, '\n');
    cin.get();
}

void registerUserFlow() {
    User newUser;
    User existingUsers[100];
    int count = getAllUsers(existingUsers, 100);

    cout << "\n--- User Registration ---" << endl;
    cout << "Enter Name: "; cin.ignore(1000, '\n'); cin.getline(newUser.name, 50);
    cout << "Enter Email: "; cin.getline(newUser.email, 64);
    
    // Перевірка унікальності Email
    for (int i = 0; i < count; i++) {
        if (strcmp(existingUsers[i].email, newUser.email) == 0) {
            cout << "[Error] User with this email already exists!" << endl;
            return;
        }
    }

    cout << "Enter Password: "; cin.getline(newUser.password, 32);

    newUser.id = count + 1;
    
    /*
    cout << "Set role (0 - User, 1 - Admin): ";
    cin >> newUser.role; cin.ignore(1000, '\n'); 
    */
    
    newUser.role = 0; 

    if (saveUser(newUser)) {
        cout << "[Success] Registered! Your auto-generated ID is: " << newUser.id << endl;
    }
}

void loginFlow() {
    char email[64], pass[32];
    cout << "\n--- Login ---" << endl;
    cout << "Email: "; cin.ignore(1000, '\n'); cin.getline(email, 64);
    cout << "Password: "; cin.getline(pass, 32);

    User users[100];
    int count = getAllUsers(users, 100);
    bool found = false;

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].email, email) == 0 && strcmp(users[i].password, pass) == 0) {
            currentUser = users[i];
            found = true;
            cout << "[Welcome] Hello, " << currentUser.name << "!" << endl;
            break;
        }
    }
    if (!found) cout << "[Error] Invalid credentials." << endl;
}

void addProductFlow() {
    Product p;
    Product existingProds[100];
    int count = getAllProducts(existingProds, 100);

    cout << "\n--- Admin: Add New Watch ---" << endl;
    cout << "Model Name: "; cin.ignore(1000, '\n'); cin.getline(p.modelName, 100);

    // Перевірка унікальності назви товару
    for (int i = 0; i < count; i++) {
        if (strcmp(existingProds[i].modelName, p.modelName) == 0) {
            cout << "[Error] This model is already in the database!" << endl;
            return;
        }
    }

    cout << "Price: "; cin >> p.price; cin.ignore(1000, '\n');
    cout << "Description: "; cin.getline(p.description, 256);
    
    cout << "Is in stock? (1 - Yes, 0 - No): "; 
    cin >> p.status; cin.ignore(1000, '\n');

    p.id = count + 1; // Автоматичний ID

    if (saveProduct(p)) {
        cout << "[Success] Product added with ID: " << p.id << endl;
    }
}