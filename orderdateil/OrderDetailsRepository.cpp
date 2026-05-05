#include "OrderDetailsRepository.h"
#include <fstream>
#include <cstdio> // Для remove та rename

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

// Функція для видалення товару з кошика
bool deleteOrderDetail(int productId, int orderId) {
    ifstream file(DETAILS_FILE, ios::binary);
    ofstream tempFile("temp.dat", ios::binary);
    if (!file || !tempFile) return false;

    OrderDetails d;
    bool found = false;
    while (file.read((char*)&d, sizeof(OrderDetails))) {
        // Якщо це не той товар, який ми видаляємо — записуємо його у тимчасовий файл
        if (d.productId == productId && d.orderId == orderId && !found) {
            found = true; // Пропускаємо запис (видаляємо)
            continue;
        }
        tempFile.write((char*)&d, sizeof(OrderDetails));
    }
    file.close();
    tempFile.close();

    remove(DETAILS_FILE);
    rename("temp.dat", DETAILS_FILE);
    return found;
}

// Функція для оновлення ID замовлення (переведення з кошика 0 в реальне ID)
void updateOrderDetailsId(int oldId, int newId) {
    fstream file(DETAILS_FILE, ios::binary | ios::in | ios::out);
    if (!file) return;

    OrderDetails d;
    while (file.read((char*)&d, sizeof(OrderDetails))) {
        if (d.orderId == oldId) {
            d.orderId = newId;
            // Повертаємо покажчик запису назад, щоб переписати саме цей об'єкт
            file.seekp((int)file.tellg() - sizeof(OrderDetails));
            file.write((char*)&d, sizeof(OrderDetails));
            // Скидаємо прапорець читання, щоб продовжити далі
            file.seekg(file.tellp()); 
        }
    }
    file.close();
}