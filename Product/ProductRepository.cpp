#include "ProductRepository.h"
#include <fstream>
#include <cstdio> // для remove та rename

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
    while (count < maxCount && file.read((char*)&outArray[count], sizeof(Product))) {
        count++;
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