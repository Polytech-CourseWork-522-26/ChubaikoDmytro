#pragma once

struct User {
    int id;
    char name[50];
    char email[64];
    char password[32];
    int role; 
    long registrationDate;
};