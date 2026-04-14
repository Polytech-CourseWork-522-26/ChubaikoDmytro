#pragma once
#include "User.h"

bool saveUser(User user);
int getAllUsers(User outArray[], int maxCount);
User findUserById(int id);
bool updateUser(User user);
bool deleteUser(int id);