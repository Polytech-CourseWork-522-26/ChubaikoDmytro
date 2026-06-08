
#pragma once

struct Product {
    int id;
    char modelName[100];
    double price;
    char shortDescription[1024]; 
    char technology[1024];       
    char careInstructions[1024];  
    int status;   
    char imagePath[256];  
    char modelPath[256]; 
};