#include "ProductRepository.h"
#include <fstream>
#include <cstdio> // для remove та rename
#include <algorithm>
#include <cstring>

using namespace std;

const char* PRODUCT_FILE = "products.dat";

bool saveProduct(Product product) {
    ofstream file(PRODUCT_FILE, ios::binary | ios::app);
    if (!file) return false;
    
    file.write((char*)&product, sizeof(Product));
    file.close();
    return true;
}

int getAllProducts(Product outArray[], int maxCount) {
    ifstream file(PRODUCT_FILE, ios::binary);
    if (!file) return 0;

    int count = 0;
    Product temp;
    
    // Зчитуємо в тимчасову змінну, щоб перевірити статус
    while (count < maxCount && file.read((char*)&temp, sizeof(Product))) {
        if (temp.status == 1) { // 1 = In Stock
            outArray[count] = temp;
            count++;
        }
    }
    
    file.close();
    return count;
}

Product findProductById(int id) {
    ifstream file(PRODUCT_FILE, ios::binary);
    Product temp;
    temp.id = -1; 

    if (!file) return temp;

    while (file.read((char*)&temp, sizeof(Product))) {
        if (temp.id == id) {
            file.close();
            return temp;
        }
    }
    file.close();
    temp.id = -1;
    return temp;
}

bool updateProduct(Product product) {
    fstream file(PRODUCT_FILE, ios::binary | ios::in | ios::out);
    if (!file) return false;

    Product temp;
    while (file.read((char*)&temp, sizeof(Product))) {
        if (temp.id == product.id) {
            // Зсув назад на розмір однієї структури
            file.seekp((int)file.tellg() - sizeof(Product));
            file.write((char*)&product, sizeof(Product));
            file.close();
            return true;
        }
    }
    return false;
}

bool deleteProduct(int id) {
    // Для видалення без vector нам доведеться читати по одному елементу
    ifstream file(PRODUCT_FILE, ios::binary);
    ofstream tempFile("temp.dat", ios::binary);
    if (!file || !tempFile) return false;

    Product temp;
    bool found = false;
    while (file.read((char*)&temp, sizeof(Product))) {
        if (temp.id != id) {
            tempFile.write((char*)&temp, sizeof(Product));
        } else {
            found = true;
        }
    }

    file.close();
    tempFile.close();

    remove(PRODUCT_FILE);
    rename("temp.dat", PRODUCT_FILE);
    
    return found;
}
bool updateProductStatus(int productId, int newStatus) {
    fstream file("products.dat", ios::binary | ios::in | ios::out);
    if (!file) return false;

    Product temp;
    while (file.read((char*)&temp, sizeof(Product))) {
        if (temp.id == productId) {
            temp.status = newStatus; // Міняємо 1 (є) на 0 (немає)
            
            // Повертаємо курсор назад на початок цього запису
            int pos = (int)file.tellg() - sizeof(Product);
            file.seekp(pos);
            file.write((const char*)&temp, sizeof(Product));
            
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}
int getAllProductsFull(Product outArray[], int maxCount) {
    ifstream file(PRODUCT_FILE, ios::binary);
    if (!file) return 0;

    int count = 0;
    while (count < maxCount && file.read((char*)&outArray[count], sizeof(Product))) {
        count++;
    }
    
    file.close();
    return count;
}
int filterProducts(Product input[], int inCount, Product output[], 
                   const char* name, double minP, double maxP, bool ascending) {
    int outCount = 0;

    for (int i = 0; i < inCount; i++) {
        // 1. Пошук по назві (strstr повертає не NULL, якщо текст знайдено)
        // Якщо name порожній "", strstr завжди поверне адресу, що нам і треба
        bool nameMatch = (strlen(name) == 0) || (strstr(input[i].modelName, name) != nullptr);
        
        // 2. Перевірка діапазону ціни
        bool priceMatch = (input[i].price >= minP && input[i].price <= maxP);

        // 3. Тільки ті, що в наявності (status == 1)
        if (nameMatch && priceMatch && input[i].status == 1) {
            output[outCount] = input[i];
            outCount++;
        }
    }

    // 4. Сортування результату
    std::sort(output, output + outCount, [ascending](const Product& a, const Product& b) {
        return ascending ? (a.price < b.price) : (a.price > b.price);
    });

    return outCount;
}