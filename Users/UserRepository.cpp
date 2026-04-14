#include "User.h"
#include "UserRepository.h"
#include <fstream>
#include <cstdio>


using namespace std;

const char* USER_FILE = "users.dat";

bool saveUser(User user) {
    ofstream file(USER_FILE, ios::binary | ios::app);
    if (!file) return false;
    file.write((char*)&user, sizeof(User));
    file.close();
    return true;
}

int getAllUsers(User outArray[], int maxCount) {
    ifstream file(USER_FILE, ios::binary);
    if (!file) return 0;
    int count = 0;
    while (count < maxCount && file.read((char*)&outArray[count], sizeof(User))) {
        count++;
    }
    file.close();
    return count;
}

User findUserById(int id) {
    ifstream file(USER_FILE, ios::binary);
    User temp; temp.id = -1;
    if (!file) return temp;
    while (file.read((char*)&temp, sizeof(User))) {
        if (temp.id == id) { file.close(); return temp; }
    }
    file.close();
    temp.id = -1;
    return temp;
}

bool updateUser(User user) {
    fstream file(USER_FILE, ios::binary | ios::in | ios::out);
    if (!file) return false;
    User temp;
    while (file.read((char*)&temp, sizeof(User))) {
        if (temp.id == user.id) {
            file.seekp((int)file.tellg() - sizeof(User));
            file.write((char*)&user, sizeof(User));
            file.close();
            return true;
        }
    }
    return false;
}

bool deleteUser(int id) {
    ifstream file(USER_FILE, ios::binary);
    ofstream tempFile("temp_u.dat", ios::binary);
    if (!file || !tempFile) return false;
    User temp; bool found = false;
    while (file.read((char*)&temp, sizeof(User))) {
        if (temp.id != id) tempFile.write((char*)&temp, sizeof(User));
        else found = true;
    }
    file.close(); tempFile.close();
    remove(USER_FILE); rename("temp_u.dat", USER_FILE);
    return found;
}