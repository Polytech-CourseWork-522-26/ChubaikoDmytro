#pragma once

struct Order {
    int id;
    int userId;
    long dateTime;
    int status; // 0 - нове, 1 - в обробці, 2 - виконано
    double totalCost;
};