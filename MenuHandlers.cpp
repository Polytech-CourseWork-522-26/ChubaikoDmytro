#include <iostream>
#include <cstring>
#include "MenuHandlers.h"
#include "Users/UserRepository.h"
#include "Product/ProductRepository.h"
#include "order/OrderRepository.h"
#include "orderdateil/OrderDetailsRepository.h"

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
                 << " | [" << (list[i].status == 1 ? "In Stock" : "Out of Stock") << "]" << endl;
            cout << "   Description: " << list[i].shortDescription << endl;
            cout << "------------------------------------" << endl;
        }

        if (currentUser.id != -1 && currentUser.role == 0) {
            int choice;
            cout << "\nEnter product number to add to cart (0 to skip): ";
            cin >> choice;
            if (choice > 0 && choice <= count) {
                if (list[choice-1].status == 1) {
                    OrderDetails detail;
                    detail.orderId = 0; // 0 означає, що товар у кошику
                    detail.productId = list[choice-1].id;
                    detail.quantity = 1;
                    detail.priceAtPurchase = list[choice-1].price;
                    
                    if (saveOrderDetail(detail)) {
                        cout << "[Success] Added to your cart!" << endl;
                    }
                } else {
                    cout << "[Error] This item is currently out of stock." << endl;
                }
            }
        }
    }
    cout << "\nPress Enter to continue...";
    cin.ignore(1000, '\n'); cin.get();
}

void registerUserFlow() {
    User newUser;
    User existingUsers[100];
    // Отримуємо поточну кількість користувачів для генерації наступного ID
    int count = getAllUsers(existingUsers, 100);

    cout << "\n--- РЕЄСТРАЦІЯ ---" << endl;
    
    while (true) {
        cout << "Введіть ім'я: ";
        // Використовуємо cin.ignore(), якщо перед цим було введення через cin >>
        cin.ignore(1000, '\n'); 
        cin.getline(newUser.name, 50);

        // Валідація: мінімальна довжина імені
        if (strlen(newUser.name) < 2) {
            cout << "[Помилка] Ім'я занадто коротке (мінімум 2 символи)!" << endl;
            continue;
        }

        // Перевірка на унікальність імені
        if (isUsernameTaken(newUser.name)) {
            cout << "[Помилка] Користувач із таким ім'ям вже існує!" << endl;
            continue;
        }
        break; 
    }

    cout << "Введіть Email: ";
    cin.getline(newUser.email, 64);

    cout << "Введіть пароль: ";
    cin.getline(newUser.password, 32);
    
    // Валідація пароля
    if (strlen(newUser.password) < 4) {
        cout << "[Помилка] Пароль має бути не менше 4 символів!" << endl;
        return;
    }

    newUser.id = count + 1;
    newUser.role = 0; 

    if (saveUser(newUser)) {
        cout << "[Успіх] Реєстрація завершена! Ваш ID: " << newUser.id << endl;
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
    Product allProducts[100];
    int count = getAllProductsFull(allProducts, 100);

    cout << "\n--- ДОДАВАННЯ НОВОГО ТОВАРУ ---" << endl;
    p.id = count + 1;

    cout << "Назва моделі: ";
    cin.ignore(1000, '\n');
    cin.getline(p.modelName, 100);

    cout << "Ціна: ";
    cin >> p.price;

    cout << "Короткий опис: ";
    cin.ignore(1000, '\n');
    cin.getline(p.shortDescription, 256);

    cout << "Технологія: ";
    cin.getline(p.technology, 512);

    cout << "Догляд: ";
    cin.getline(p.careInstructions, 512);

    p.status = 1; // За замовчуванням в наявності

    if (saveProduct(p)) {
        cout << "[Успіх] Товар збережено для фронтенду!" << endl;
    }
}

void viewAllUsersFlow() {
    User list[100];
    int count = getAllUsers(list, 100);
    cout << "\n--- SYSTEM USERS ---" << endl;
    for (int i = 0; i < count; i++) {
        cout << "ID: " << list[i].id << " | Name: " << list[i].name 
             << " | Role: " << (list[i].role == 1 ? "Admin" : "User") << endl;
    }
    cout << "\nPress Enter...";
    cin.ignore(1000, '\n'); cin.get();
}

void manageOrdersFlow() {
    Order list[100];
    int count = getAllOrders(list, 100);
    
    cout << "\n--- КЕРУВАННЯ ЗАМОВЛЕННЯМИ (АДМІН) ---" << endl;
    if (count == 0) {
        cout << "Замовлень немає." << endl;
    } else {
        for (int i = 0; i < count; i++) {
            const char* statusStr = (list[i].status == 1) ? "Очікує" : 
                                   (list[i].status == 2) ? "Одобрено" : "Відхилено";
            cout << i + 1 << ". Замовлення #" << list[i].id 
                 << " | Юзер ID: " << list[i].userId 
                 << " | Статус: " << statusStr << endl;
        }

        int choice;
        cout << "\nОберіть номер замовлення для зміни статусу (0 - вихід): ";
        cin >> choice;

        if (choice > 0 && choice <= count) {
            cout << "Новий статус (2 - Одобрити, 3 - Відхилити): ";
            cin >> list[choice - 1].status;
            if (updateOrder(list[choice - 1])) {
                cout << "[Успіх] Статус оновлено!" << endl;
            }
        }
    }
    cout << "\nНатисніть Enter...";
    cin.ignore(1000, '\n'); cin.get();
}

void viewCartFlow() {
    OrderDetails cart[100];
    int count = getDetailsByOrderId(0, cart, 100); 
    
    cout << "\n--- МІЙ КОШИК ---" << endl;
    if (count == 0) {
        cout << "Кошик порожній." << endl;
    } else {
        double total = 0;
        for (int i = 0; i < count; i++) {
            cout << i + 1 << ". ID Товару: " << cart[i].productId 
                 << " | Ціна: $" << cart[i].priceAtPurchase << endl;
            total += cart[i].priceAtPurchase;
        }
        cout << "-----------------------" << endl;
        cout << "ЗАГАЛЬНА СУМА: $" << total << endl;
        
        cout << "\n1. Оформити замовлення\n2. Видалити товар з кошика\n0. Назад\nВибір: ";
        int action; cin >> action;

        if (action == 1) {
            Order newOrder;
            Order allOrders[100];
            int orderCount = getAllOrders(allOrders, 100);

            newOrder.id = orderCount + 1;
            newOrder.userId = currentUser.id;
            newOrder.status = 1; // 1 - Очікує

            if (saveOrder(newOrder)) {
                // 1. Оновлюємо зв'язок товарів: змінюємо orderId з 0 на новий ID
                updateOrderDetailsId(0, newOrder.id);

                // 2. ОНОВЛЕННЯ СКЛАДУ: Проходимо по кожному товару, який був у кошику,
                // і змінюємо його статус у products.dat на 0 (продано)
                for (int i = 0; i < count; i++) {
                    updateProductStatus(cart[i].productId, 0);
                }

                cout << "[Успіх] Замовлення #" << newOrder.id << " оформлено!" << endl;
                cout << "[Інфо] Куплені товари знято з вітрини магазину." << endl;
            }
        } else if (action == 2) {
            int num;
            cout << "Введіть номер товару зі списку для видалення: ";
            cin >> num;
            if (num > 0 && num <= count) {
                if (deleteOrderDetail(cart[num-1].productId, 0)) {
                    cout << "[Успіх] Товар видалено з кошика." << endl;
                }
            }
        }
    }
    cout << "\nНатисніть Enter...";
    cin.ignore(1000, '\n'); cin.get();
}

void viewMyOrdersFlow() {
    Order list[100];
    int count = getAllOrders(list, 100);
    bool found = false;

    cout << "\n--- МОЯ ІСТОРІЯ ЗАМОВЛЕНЬ ---" << endl;
    for (int i = 0; i < count; i++) {
        if (list[i].userId == currentUser.id) {
            found = true;
            const char* statusStr = (list[i].status == 1) ? "Очікує" : 
                                   (list[i].status == 2) ? "Одобрено" : "Відхилено";
            cout << "Замовлення #" << list[i].id << " | Статус: " << statusStr << endl;
        }
    }
    if (!found) cout << "У вас ще немає замовлень." << endl;
    cout << "\nНатисніть Enter...";
    cin.ignore(1000, '\n'); cin.get();
}