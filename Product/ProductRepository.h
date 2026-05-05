#pragma once
#include "Product.h"

// Зберігає один товар у файл
bool saveProduct(Product product);

int getAllProducts(Product outArray[], int maxCount);

Product findProductById(int id);

bool updateProduct(Product product);

bool deleteProduct(int id);

bool updateProductStatus(int productId, int newStatus);

int getAllProductsFull(Product outArray[], int maxCount);

int filterProducts(Product input[], int inCount, Product output[], 
                   const char* name, double minP, double maxP, bool ascending);