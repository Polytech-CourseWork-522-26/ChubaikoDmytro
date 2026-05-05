struct Product {
    int id;
    char modelName[100];
    double price;
    char shortDescription[256]; // Короткий опис для каталогу
    char technology[512];      // Поле "Технологія" зі скриншоту
    char careInstructions[512]; // Поле "Догляд" зі скриншоту
    int status;                 // 1 - In Stock, 0 - Out of Stock
};
