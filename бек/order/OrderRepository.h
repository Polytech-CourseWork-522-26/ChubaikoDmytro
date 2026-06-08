#pragma once
#include "Order.h"

bool saveOrder(Order order);
int getAllOrders(Order outArray[], int maxCount);
Order findOrderById(int id);
bool updateOrder(Order order);