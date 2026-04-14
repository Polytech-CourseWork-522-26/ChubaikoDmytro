#include "OrderDetailsRepository.h"
#include <fstream>

using namespace std;

const char* DETAILS_FILE = "order_details.dat";

bool saveOrderDetail(OrderDetails detail) {
    ofstream file(DETAILS_FILE, ios::binary | ios::app);
    if (!file) return false;
    file.write((char*)&detail, sizeof(OrderDetails));
    file.close();
    return true;
}

int getDetailsByOrderId(int orderId, OrderDetails outArray[], int maxCount) {
    ifstream file(DETAILS_FILE, ios::binary);
    if (!file) return 0;
    int count = 0;
    OrderDetails temp;
    while (count < maxCount && file.read((char*)&temp, sizeof(OrderDetails))) {
        if (temp.orderId == orderId) {
            outArray[count] = temp;
            count++;
        }
    }
    file.close();
    return count;
}