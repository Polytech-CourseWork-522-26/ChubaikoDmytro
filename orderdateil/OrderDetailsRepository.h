#pragma once
#include "OrderDetails.h"

bool saveOrderDetail(OrderDetails detail);
int getDetailsByOrderId(int orderId, OrderDetails outArray[], int maxCount);

// ДОДАЙТЕ ЦІ ДВА РЯДКИ:
bool deleteOrderDetail(int productId, int orderId);
void updateOrderDetailsId(int oldId, int newId);