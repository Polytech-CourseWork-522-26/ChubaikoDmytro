#pragma once
#include "Users/User.h"

// Функції для роботи з інтерфейсом
void initializeSystem();
void showCatalogFlow();
void registerUserFlow();
void loginFlow();
void addProductFlow();

// Глобальна змінна для поточної сесії
extern User currentUser;