#pragma once

struct Product {
    int id;
    char modelName[100];
    double price;
    char description[256];
    int status; // 1 - активний, 0 - неактивний
};
