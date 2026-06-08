#pragma once
#include "Users/User.h"

void initializeSystem();
void showCatalogFlow();
void registerUserFlow();
void loginFlow();
void addProductFlow();

// Сценарії замовлень
void viewCartFlow();               // Кошик: перегляд, видалення, оформлення
void viewMyOrdersFlow();           // [User] Історія моїх замовлень
void manageOrdersFlow();           // [Admin] Керування статусами замовлень
void viewAllUsersFlow();           // [Admin] Список користувачів
// У MenuHandlers.h
bool saveUser(User newUser);
int getAllUsers(User users[], int maxCount);
bool isUsernameTaken(const char* name);

extern User currentUser;