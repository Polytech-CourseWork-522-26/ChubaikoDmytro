#pragma once
#include "OrderDetails.h"

bool saveOrderDetail(OrderDetails detail);
// Повертає всі товари, що належать конкретному замовленню
int getDetailsByOrderId(int orderId, OrderDetails outArray[], int maxCount);