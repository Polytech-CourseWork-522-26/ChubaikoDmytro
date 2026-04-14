#pragma once
#include "Product.h"

// Зберігає один товар у файл
bool saveProduct(Product product);

int getAllProducts(Product outArray[], int maxCount);

// Пошук за ID
Product findProductById(int id);

// Оновлення
bool updateProduct(Product product);

// Видалення
bool deleteProduct(int id);