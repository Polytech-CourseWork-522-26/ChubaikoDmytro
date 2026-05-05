#include "OrderRepository.h"
#include <fstream>

using namespace std;

const char* ORDER_FILE = "orders.dat";

bool saveOrder(Order order) {
    ofstream file(ORDER_FILE, ios::binary | ios::app);
    if (!file) return false;
    file.write((char*)&order, sizeof(Order));
    file.close();
    return true;
}

int getAllOrders(Order outArray[], int maxCount) {
    ifstream file(ORDER_FILE, ios::binary);
    if (!file) return 0;
    int count = 0;
    while (count < maxCount && file.read((char*)&outArray[count], sizeof(Order))) {
        count++;
    }
    file.close();
    return count;
}

Order findOrderById(int id) {
    ifstream file(ORDER_FILE, ios::binary);
    Order temp; temp.id = -1;
    if (!file) return temp;
    while (file.read((char*)&temp, sizeof(Order))) {
        if (temp.id == id) { file.close(); return temp; }
    }
    file.close();
    temp.id = -1;
    return temp;
}

bool updateOrder(Order order) {
    // Відкриваємо файл у режимі читання та запису (binary | in | out)
    fstream file(ORDER_FILE, ios::binary | ios::in | ios::out);
    if (!file) return false;

    Order temp;
    bool found = false;
    
    // Шукаємо замовлення за ID
    while (file.read((char*)&temp, sizeof(Order))) {
        if (temp.id == order.id) {
            // Переміщуємо покажчик запису на початок знайденої структури
            file.seekp((int)file.tellg() - sizeof(Order));
            // Переписуємо дані новими значеннями
            file.write((char*)&order, sizeof(Order));
            found = true;
            break; // Виходимо з циклу після успішного оновлення
        }
    }

    file.close(); // Закриваємо файл тільки після закінчення всіх дій
    return found;
}