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

extern User currentUser;