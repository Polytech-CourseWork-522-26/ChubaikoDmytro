#include <iostream>
#include <ctime>
#include <fstream>
#include <string>
#include <filesystem>
#include "crow_all.h"
#include <cstring>
#include <windows.h>
#include <vector>

#include "MenuHandlers.h"
#include "Product/ProductRepository.h"
#include "Users/UserRepository.h"
#include "order/OrderRepository.h"
#include "orderdateil/OrderDetailsRepository.h"

using namespace std;

// ==========================================
// СТРУКТУРА ДЛЯ АВТОРИЗАЦІЇ (ТОКЕНИ)
// ==========================================
struct TokenData
{
    int userId;
    int role;
    bool valid;
};

TokenData parseToken(const crow::request &req)
{
    TokenData td{-1, -1, false};
    auto auth = req.get_header_value("Authorization");
    if (auth.empty())
        return td;
    // Формат: "Bearer <userId>:<role>"
    if (auth.rfind("Bearer ", 0) != 0)
        return td;
    string token = auth.substr(7);
    auto sep = token.find(':');
    if (sep == string::npos)
        return td;
    try
    {
        td.userId = stoi(token.substr(0, sep));
        td.role = stoi(token.substr(sep + 1));
        td.valid = true;
    }
    catch (...)
    {
    }
    return td;
}

// ==========================================
// CORS MIDDLEWARE
// ==========================================
struct CORSMiddleware
{
    struct context
    {
    };

    void before_handle(crow::request &req, crow::response &res, context &)
    {
        // Відповідаємо на preflight OPTIONS одразу, до будь-якого маршруту
        if (req.method == crow::HTTPMethod::Options)
        {
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            res.code = 200;
            res.end();
        }
    }

    void after_handle(crow::request &, crow::response &res, context &)
    {
        // Додаємо CORS-заголовки до кожної звичайної відповіді
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    }
};

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    crow::App<CORSMiddleware> app;

    std::filesystem::create_directories("images/products");
    std::filesystem::create_directories("models/products");

    // ==========================================
    // 1. РЕЄСТРАЦІЯ
    // ==========================================
    CROW_ROUTE(app, "/api/register").methods(crow::HTTPMethod::Post)([](const crow::request &req)
                                                                     {
        crow::response res;
        auto x = crow::json::load(req.body);
        if (!x || !x.has("name") || !x.has("email") || !x.has("password")) {
            res.code = 400; res.body = "Некоректні дані форми!"; return res;
        }
        User newUser; memset(&newUser, 0, sizeof(User));
        string s_name = x["name"].s(), s_email = x["email"].s(), s_pass = x["password"].s();
        
        if (s_name.size() < 2) { res.code = 400; res.body = "Ім'я занадто коротке!"; return res; }
        if (s_pass.size() < 4) { res.code = 400; res.body = "Пароль має бути мінімум 4 символи!"; return res; }
        
        strncpy(newUser.name, s_name.c_str(), sizeof(newUser.name) - 1);
        strncpy(newUser.email, s_email.c_str(), sizeof(newUser.email) - 1);
        strncpy(newUser.password, s_pass.c_str(), sizeof(newUser.password) - 1);
        
        if (isUsernameTaken(newUser.name)) { res.code = 400; res.body = "Користувач з таким іменем вже існує!"; return res; }
        
        User users[200]; int count = getAllUsers(users, 200); int maxId = 0;
        for (int i = 0; i < count; i++) if (users[i].id > maxId) maxId = users[i].id;
        
        newUser.id = maxId + 1; newUser.role = 0;
        newUser.registrationDate = (long)time(nullptr);
        if (saveUser(newUser)) { 
            res.code = 200; 
            res.body = "Реєстрація пройшла успішно!"; 
        } else { 
            res.code = 500; res.body = "Помилка збереження користувача в базу даних."; 
        }
        return res; });

    // ==========================================
    // 2. ЛОГІН
    // ==========================================
    CROW_ROUTE(app, "/api/login").methods(crow::HTTPMethod::Post)([](const crow::request &req)
                                                                  {
        crow::response res;
        auto x = crow::json::load(req.body);
        if (!x || !x.has("email") || !x.has("password")) {
            res.code = 400; res.body = "{\"error\": \"Будь ласка, заповніть пошту та пароль!\"}"; return res;
        }
        string req_email = x["email"].s(), req_password = x["password"].s();
        User users[200]; int count = getAllUsers(users, 200);
        bool found = false; User loggedInUser;
        
        for (int i = 0; i < count; i++) {
            if (string(users[i].email) == req_email && string(users[i].password) == req_password) {
                found = true; loggedInUser = users[i]; break;
            }
        }
        
        if (found) {
            res.code = 200;
            crow::json::wvalue d;
            string token = to_string(loggedInUser.id) + ":" + to_string(loggedInUser.role);
            
            d["status"] = "success"; 
            d["name"] = string(loggedInUser.name);
            d["email"] = string(loggedInUser.email); 
            d["role"] = loggedInUser.role;
            d["token"] = token; // Додано токен
            res.body = d.dump();
        } else { res.code = 401; res.body = "{\"error\": \"Неправильний Email або пароль!\"}"; }
        return res; });

    // ==========================================
    // 3. ОТРИМАННЯ СПИСКУ ТОВАРІВ
    // ==========================================
    CROW_ROUTE(app, "/api/products").methods(crow::HTTPMethod::Get)([](const crow::request &req)
                                                                    {
    crow::response res;
    res.set_header("Content-Type", "application/json");

    // Читаємо токен — адмін бачить всі товари, юзер тільки status==1
    auto td = parseToken(req);
    bool admin = td.valid && td.role == 1;

    string name = req.url_params.get("name") ? req.url_params.get("name") : "";

    double minP = 0.0;
    double maxP = 999999.0;
    try {
        if (req.url_params.get("minP") && strlen(req.url_params.get("minP")) > 0)
            minP = stod(req.url_params.get("minP"));
        if (req.url_params.get("maxP") && strlen(req.url_params.get("maxP")) > 0)
            maxP = stod(req.url_params.get("maxP"));
    } catch (...) {}

    bool ascending = req.url_params.get("ascending")
        ? string(req.url_params.get("ascending")) != "false"
        : true;

    std::vector<Product> all(500);
    std::vector<Product> filtered(500);

    int total = getAllProductsFull(all.data(), 500);

    // Адмін — фільтруємо без перевірки статусу
    // Юзер  — filterProducts показує тільки status==1
    int count = 0;
    if (admin) {
        // Фільтруємо вручну без перевірки статусу
        for (int i = 0; i < total; i++) {
            if (all[i].id <= 0) continue;
            bool nameMatch = (name.empty()) || (strstr(all[i].modelName, name.c_str()) != nullptr);
            bool priceMatch = (all[i].price >= minP && all[i].price <= maxP);
            if (nameMatch && priceMatch) {
                filtered[count++] = all[i];
            }
        }
        // Сортування
        std::sort(filtered.begin(), filtered.begin() + count,
            [ascending](const Product& a, const Product& b) {
                return ascending ? (a.price < b.price) : (a.price > b.price);
            });
    } else {
        count = filterProducts(all.data(), total, filtered.data(),
                               name.c_str(), minP, maxP, ascending);
    }

    crow::json::wvalue result;
    crow::json::wvalue::list list;

    for (int i = 0; i < count; i++) {
        if (filtered[i].id <= 0) continue;
        crow::json::wvalue p;
        p["id"]               = filtered[i].id;
        p["modelName"]        = filtered[i].modelName[0] != '\0' ? string(filtered[i].modelName) : "Без назви";
        p["price"]            = filtered[i].price;
        p["shortDescription"] = filtered[i].shortDescription[0] != '\0' ? string(filtered[i].shortDescription) : "";
        p["technology"]       = filtered[i].technology[0] != '\0' ? string(filtered[i].technology) : "";
        p["careInstructions"] = filtered[i].careInstructions[0] != '\0' ? string(filtered[i].careInstructions) : "";
        p["status"]           = filtered[i].status;
        p["imagePath"]        = filtered[i].imagePath[0] != '\0' ? string(filtered[i].imagePath) : "";
        list.push_back(std::move(p));
    }

    result["products"] = std::move(list);
    result["count"]    = count;

    res.code = 200;
    res.body = result.dump();
    return res; });

    // ==========================================
    // 4. ДОДАВАННЯ ТОВАРУ (З ПЕРЕВІРКОЮ ТОКЕНА)
    // ==========================================
    CROW_ROUTE(app, "/api/products/add").methods(crow::HTTPMethod::Post)([](const crow::request &req)
                                                                         {
        crow::response res;
        
        auto td = parseToken(req);
        if (!td.valid || td.role != 1) {
            res.code = 403; res.body = "{\"error\":\"Доступ заборонено (лише для адмінів)\"}"; return res;
        }

        crow::multipart::message msg(req);

        string modelName, shortDescription, technology, careInstructions;
        double price = 0.0; int status = 1;
        string imagePath = "", modelPath = "";

        for (auto& part : msg.parts) {
            string partName = "";
            auto it = part.headers.find("Content-Disposition");
            if (it != part.headers.end()) {
                auto nameParam = it->second.params.find("name");
                if (nameParam != it->second.params.end()) partName = nameParam->second;
            }

            if      (partName == "modelName")   modelName        = part.body;
            else if (partName == "price")        price            = stod(part.body);
            else if (partName == "description")  shortDescription = part.body;
            else if (partName == "technology")   technology       = part.body;
            else if (partName == "care")         careInstructions = part.body;
            else if (partName == "status")       status           = stoi(part.body);
            else if (partName == "photo") {
                auto it2 = part.headers.find("Content-Disposition");
                if (it2 != part.headers.end()) {
                    auto fnParam = it2->second.params.find("filename");
                    if (fnParam != it2->second.params.end() && !fnParam->second.empty()) {
                        string ext = ""; size_t d = fnParam->second.rfind('.');
                        if (d != string::npos) ext = fnParam->second.substr(d);
                        string tempPath = "images/products/temp" + ext;
                        ofstream outFile(tempPath, ios::binary);
                        if (outFile) { outFile.write(part.body.data(), part.body.size()); outFile.close(); imagePath = tempPath; }
                    }
                }
            }
            else if (partName == "model") {
                auto it2 = part.headers.find("Content-Disposition");
                if (it2 != part.headers.end()) {
                    auto fnParam = it2->second.params.find("filename");
                    if (fnParam != it2->second.params.end() && !fnParam->second.empty()) {
                        string ext = ""; size_t d = fnParam->second.rfind('.');
                        if (d != string::npos) ext = fnParam->second.substr(d);
                        vector<string> validExts = {".glb", ".gltf", ".obj", ".fbx", ".stl"};
                        bool isValid = false;
                        for (auto& ve : validExts) if (ext == ve) { isValid = true; break; }
                        if (isValid) {
                            string tempPath = "models/products/temp" + ext;
                            ofstream outFile(tempPath, ios::binary);
                            if (outFile) { outFile.write(part.body.data(), part.body.size()); outFile.close(); modelPath = tempPath; }
                        }
                    }
                }
            }
        }

        if (modelName.empty()) { res.code = 400; res.body = "{\"error\": \"Назва товару обов'язкова!\"}"; return res; }

        Product products[500]; int count = getAllProductsFull(products, 500);
        int maxId = 0;
        for (int i = 0; i < count; i++) if (products[i].id > maxId) maxId = products[i].id;
        int newId = maxId + 1;

        if (!imagePath.empty()) {
            string ext = imagePath.substr(imagePath.rfind('.'));
            string finalPath = "images/products/" + to_string(newId) + ext;
            rename(imagePath.c_str(), finalPath.c_str());
            imagePath = finalPath;
        }
        if (!modelPath.empty()) {
            string ext = modelPath.substr(modelPath.rfind('.'));
            string finalPath = "models/products/" + to_string(newId) + ext;
            rename(modelPath.c_str(), finalPath.c_str());
            modelPath = finalPath;
        }

        Product newProduct; memset(&newProduct, 0, sizeof(Product));
        newProduct.id = newId; newProduct.price = price; newProduct.status = status;
        strncpy(newProduct.modelName,        modelName.c_str(),        sizeof(newProduct.modelName) - 1);
        strncpy(newProduct.shortDescription, shortDescription.c_str(), sizeof(newProduct.shortDescription) - 1);
        strncpy(newProduct.technology,       technology.c_str(),       sizeof(newProduct.technology) - 1);
        strncpy(newProduct.careInstructions, careInstructions.c_str(), sizeof(newProduct.careInstructions) - 1);
        strncpy(newProduct.imagePath,        imagePath.c_str(),        sizeof(newProduct.imagePath) - 1);
        strncpy(newProduct.modelPath,        modelPath.c_str(),        sizeof(newProduct.modelPath) - 1);

        if (saveProduct(newProduct)) {
            res.code = 200;
            crow::json::wvalue d;
            d["status"] = "success"; d["id"] = newProduct.id;
            d["imagePath"] = imagePath;
            res.body = d.dump();
        } else { res.code = 500; res.body = "{\"error\": \"Помилка збереження товару!\"}"; }
        return res; });

    // ==========================================
    // 5-6. ОТРИМАННЯ ФОТО ТА 3D МОДЕЛІ
    // ==========================================
    CROW_ROUTE(app, "/api/products/image/<int>")([](int id)
                                                 {
        crow::response res;
        vector<string> exts = {".jpg", ".jpeg", ".png", ".webp"};
        string foundPath = "";
        for (auto& ext : exts) {
            string path = "images/products/" + to_string(id) + ext;
            ifstream f(path);
            if (f.good()) { foundPath = path; break; }
        }
        if (foundPath.empty()) { res.code = 404; res.body = "Фото не знайдено"; return res; }
        ifstream file(foundPath, ios::binary);
        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        string ext = foundPath.substr(foundPath.rfind('.'));
        string contentType = "image/jpeg";
        if (ext == ".png") contentType = "image/png";
        if (ext == ".webp") contentType = "image/webp";
        res.code = 200; res.set_header("Content-Type", contentType); res.body = content;
        return res; });

    CROW_ROUTE(app, "/api/products/model/<int>")([](int id)
                                                 {
        crow::response res;
        vector<string> exts = {".glb", ".gltf", ".obj", ".fbx", ".stl"};
        string foundPath = "";
        for (auto& ext : exts) {
            string path = "models/products/" + to_string(id) + ext;
            ifstream f(path);
            if (f.good()) { foundPath = path; break; }
        }
        if (foundPath.empty()) { res.code = 404; res.body = "{\"error\": \"3D модель не знайдена\"}"; return res; }
        ifstream file(foundPath, ios::binary);
        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        string ext = foundPath.substr(foundPath.rfind('.'));
        string contentType = "application/octet-stream";
        if (ext == ".glb") contentType = "model/gltf-binary";
        else if (ext == ".gltf") contentType = "model/gltf+json";
        else if (ext == ".obj") contentType = "text/plain";
        res.code = 200; res.set_header("Content-Type", contentType);
        res.set_header("Cache-Control", "public, max-age=86400");
        res.body = content;
        return res; });

    // ==========================================
    // 7. ОТРИМАННЯ ОДНОГО ТОВАРУ ЗА ID
    // ==========================================
    CROW_ROUTE(app, "/api/products/<int>").methods(crow::HTTPMethod::Get)([](int id)
                                                                          {
        crow::response res;
        res.set_header("Content-Type", "application/json");
        
        Product p = findProductById(id);
        if (p.id == -1) {
            res.code = 404; res.body = "{\"error\": \"Товар не знайдено\"}"; return res;
        }
        
        crow::json::wvalue result;
        result["id"]               = p.id;
        result["modelName"]        = (p.modelName[0] != '\0') ? string(p.modelName) : "Без назви";
        result["price"]            = p.price;
        result["shortDescription"] = (p.shortDescription[0] != '\0') ? string(p.shortDescription) : "";
        result["technology"]       = (p.technology[0] != '\0') ? string(p.technology) : "";
        result["careInstructions"] = (p.careInstructions[0] != '\0') ? string(p.careInstructions) : "";
        result["status"]           = p.status;
        result["imagePath"]        = (p.imagePath[0] != '\0') ? string(p.imagePath) : "";
        result["modelPath"]        = (p.modelPath[0] != '\0') ? string(p.modelPath) : "";
        
        res.code = 200; 
        res.body = result.dump();
        return res; });
    // ==========================================
    // 8. РЕДАГУВАННЯ ТОВАРУ
    // ==========================================
    CROW_ROUTE(app, "/api/products/update/<int>").methods(crow::HTTPMethod::Post)([](const crow::request &req, int id)
                                                                                  {
        crow::response res;
        auto td = parseToken(req);
        if (!td.valid || td.role != 1) {
            res.code = 403; res.body = "{\"error\":\"Доступ заборонено\"}"; return res;
        }

        crow::multipart::message msg(req);
        Product updatedProduct = findProductById(id);
        if (updatedProduct.id == -1) {
            res.code = 404; res.body = "{\"error\": \"Товар для оновлення не знайдено\"}"; return res;
        }

        for (auto& part : msg.parts) {
            string partName = "";
            auto it = part.headers.find("Content-Disposition");
            if (it != part.headers.end()) {
                auto nameParam = it->second.params.find("name");
                if (nameParam != it->second.params.end()) partName = nameParam->second;
            }

            if      (partName == "modelName")   strncpy(updatedProduct.modelName, part.body.c_str(), sizeof(updatedProduct.modelName)-1);
            else if (partName == "price")        updatedProduct.price = stod(part.body);
            else if (partName == "description")  strncpy(updatedProduct.shortDescription, part.body.c_str(), sizeof(updatedProduct.shortDescription)-1);
            else if (partName == "technology")   strncpy(updatedProduct.technology, part.body.c_str(), sizeof(updatedProduct.technology)-1);
            else if (partName == "care")         strncpy(updatedProduct.careInstructions, part.body.c_str(), sizeof(updatedProduct.careInstructions)-1);
            else if (partName == "status")       updatedProduct.status = stoi(part.body);
            else if (partName == "photo" && !part.body.empty()) {
                auto it2 = part.headers.find("Content-Disposition");
                if (it2 != part.headers.end()) {
                    auto fnParam = it2->second.params.find("filename");
                    if (fnParam != it2->second.params.end() && !fnParam->second.empty()) {
                        string ext = fnParam->second.substr(fnParam->second.rfind('.'));
                        string finalPath = "images/products/" + to_string(id) + ext;
                        ofstream outFile(finalPath, ios::binary);
                        if (outFile) { outFile.write(part.body.data(), part.body.size()); outFile.close(); strncpy(updatedProduct.imagePath, finalPath.c_str(), sizeof(updatedProduct.imagePath)-1); }
                    }
                }
            }
            else if (partName == "model" && !part.body.empty()) {
                auto it2 = part.headers.find("Content-Disposition");
                if (it2 != part.headers.end()) {
                    auto fnParam = it2->second.params.find("filename");
                    if (fnParam != it2->second.params.end() && !fnParam->second.empty()) {
                        string ext = fnParam->second.substr(fnParam->second.rfind('.'));
                        vector<string> validExts = {".glb", ".gltf", ".obj", ".fbx", ".stl"};
                        bool isValid = false;
                        for (auto& ve : validExts) if (ext == ve) { isValid = true; break; }
                        if (isValid) {
                            string finalPath = "models/products/" + to_string(id) + ext;
                            ofstream outFile(finalPath, ios::binary);
                            if (outFile) { outFile.write(part.body.data(), part.body.size()); outFile.close(); strncpy(updatedProduct.modelPath, finalPath.c_str(), sizeof(updatedProduct.modelPath)-1); }
                        }
                    }
                }
            }
        }

        if (updateProductFull(updatedProduct)) { res.code = 200; res.body = "{\"status\":\"success\"}"; } 
        else { res.code = 500; res.body = "{\"error\": \"Не вдалося оновити товар\"}"; }
        return res; });

    // ==========================================
    // 9. ВИДАЛЕННЯ ТОВАРУ
    // ==========================================
    CROW_ROUTE(app, "/api/products/<int>").methods(crow::HTTPMethod::Delete)([](const crow::request &req, int id)
                                                                             {
        crow::response res;
        auto td = parseToken(req);
        if (!td.valid || td.role != 1) {
            res.code = 403; res.body = "{\"error\":\"Доступ заборонено\"}"; return res;
        }

        for (auto& ext : std::vector<std::string>{".jpg", ".jpeg", ".png", ".webp"}) {
            std::string path = "images/products/" + std::to_string(id) + ext;
            if (std::filesystem::exists(path)) std::filesystem::remove(path);
        }
        for (auto& ext : std::vector<std::string>{".glb", ".gltf", ".obj", ".fbx", ".stl"}) {
            std::string path = "models/products/" + std::to_string(id) + ext;
            if (std::filesystem::exists(path)) std::filesystem::remove(path);
        }
        if (deleteProduct(id)) { res.code = 200; res.body = "{\"status\": \"deleted\"}"; } 
        else { res.code = 404; res.body = "{\"error\": \"Товар не знайдено\"}"; }
        return res; });

    // ==========================================
    // МАРШРУТ: КОШИК
    // ==========================================
    CROW_ROUTE(app, "/api/cart").methods(crow::HTTPMethod::Get)([](const crow::request &req)
                                                                {
        crow::response res;
        auto td = parseToken(req);
        if (!td.valid) { res.code = 401; res.body = "{\"error\":\"Не авторизовано\"}"; return res; }

        OrderDetails cart[100];
        int count = getDetailsByOrderId(-td.userId, cart, 100);

        crow::json::wvalue::list items;
        for (int i = 0; i < count; i++) {
            Product p = findProductById(cart[i].productId);
            crow::json::wvalue item;
            item["productId"] = cart[i].productId;
            item["quantity"] = cart[i].quantity;
            item["priceAtPurchase"] = cart[i].priceAtPurchase;
            item["modelName"] = p.id != -1 ? string(p.modelName) : "Невідомо";
            item["shortDescription"] = p.id != -1 ? string(p.shortDescription) : "";
            item["status"] = p.id != -1 ? p.status : 0;
            items.push_back(std::move(item));
        }

        crow::json::wvalue result; result["items"] = std::move(items);
        res.code = 200; res.set_header("Content-Type", "application/json"); res.body = result.dump(); return res; });

    CROW_ROUTE(app, "/api/cart/<int>").methods(crow::HTTPMethod::Post)([](const crow::request &req, int productId)
                                                                       {
        crow::response res;
        auto td = parseToken(req);
        if (!td.valid) { res.code = 401; res.body = "{\"error\":\"Не авторизовано\"}"; return res; }

        Product p = findProductById(productId);
        if (p.id == -1) { res.code = 404; res.body = "{\"error\":\"Товар не знайдено\"}"; return res; }
        if (p.status != 1) { res.code = 400; res.body = "{\"error\":\"Товар недоступний\"}"; return res; }

        // Перевірка: чи товар вже є в кошику цього користувача
        OrderDetails cart[100];
        int cartCount = getDetailsByOrderId(-td.userId, cart, 100);
        for (int i = 0; i < cartCount; i++) {
            if (cart[i].productId == productId) {
                res.code = 409; res.body = "{\"error\":\"Цей товар вже є у вашому кошику\"}"; return res;
            }
        }

        OrderDetails d;
        d.orderId = -td.userId; d.productId = productId; d.quantity = 1; d.priceAtPurchase = p.price;

        if (!saveOrderDetail(d)) { res.code = 500; res.body = "{\"error\":\"Помилка додавання\"}"; } 
        else { res.code = 201; res.body = "{\"success\":true}"; }
        return res; });

    CROW_ROUTE(app, "/api/cart/<int>").methods(crow::HTTPMethod::Delete)([](const crow::request &req, int productId)
                                                                         {
        crow::response res;
        auto td = parseToken(req);
        if (!td.valid) { res.code = 401; res.body = "{\"error\":\"Не авторизовано\"}"; return res; }

        if (!deleteOrderDetail(productId, -td.userId)) { res.code = 404; res.body = "{\"error\":\"Товар не знайдено в кошику\"}"; } 
        else { res.code = 200; res.body = "{\"success\":true}"; }
        return res; });

    // ==========================================
    // МАРШРУТИ: ЗАМОВЛЕННЯ ТА КОРИСТУВАЧІ
    // ==========================================
    CROW_ROUTE(app, "/api/orders").methods(crow::HTTPMethod::Post)([](const crow::request &req)
                                                                   {
        crow::response res;
        auto td = parseToken(req);
        if (!td.valid) { res.code = 401; res.body = "{\"error\":\"Не авторизовано\"}"; return res; }

        OrderDetails cart[100];
        int cartCount = getDetailsByOrderId(-td.userId, cart, 100);
        if (cartCount == 0) { res.code = 400; res.body = "{\"error\":\"Кошик порожній\"}"; return res; }

        Order allOrders[200]; int orderCount = getAllOrders(allOrders, 200);
        int maxOrderId = 0;
        for (int i = 0; i < orderCount; i++)
            if (allOrders[i].id > maxOrderId) maxOrderId = allOrders[i].id;

        Order newOrder;
        newOrder.id = maxOrderId + 1; newOrder.userId = td.userId;
        newOrder.status = 1; newOrder.dateTime = (long)time(nullptr); newOrder.totalCost = 0;

        for (int i = 0; i < cartCount; i++) newOrder.totalCost += cart[i].priceAtPurchase;

        if (!saveOrder(newOrder)) { res.code = 500; res.body = "{\"error\":\"Помилка збереження замовлення\"}"; return res; }

        updateOrderDetailsId(-td.userId, newOrder.id);
        for (int i = 0; i < cartCount; i++) updateProductStatus(cart[i].productId, 0);

        crow::json::wvalue result;
        result["id"] = newOrder.id; result["userId"] = newOrder.userId; result["status"] = newOrder.status; result["totalCost"] = newOrder.totalCost;
        res.code = 201; res.set_header("Content-Type", "application/json"); res.body = result.dump(); return res; });

    // ==========================================
    // GET ЗАМОВЛЕНЬ ПОТОЧНОГО КОРИСТУВАЧА
    // ==========================================
    CROW_ROUTE(app, "/api/orders").methods(crow::HTTPMethod::Get)([](const crow::request &req)
    {
        crow::response res;
        auto td = parseToken(req);
        if (!td.valid) { res.code = 401; res.body = "{\"error\":\"Не авторизовано\"}"; return res; }

        Order allOrders[200];
        int count = getAllOrders(allOrders, 200);

        crow::json::wvalue::list list;
        for (int i = 0; i < count; i++) {
            if (allOrders[i].userId != td.userId) continue;
            crow::json::wvalue o;
            o["id"]        = allOrders[i].id;
            o["status"]    = allOrders[i].status;
            o["totalCost"] = allOrders[i].totalCost;
            o["dateTime"]  = (long long)allOrders[i].dateTime;
            list.push_back(std::move(o));
        }
        crow::json::wvalue result;
        result["orders"] = std::move(list);
        res.code = 200; res.set_header("Content-Type", "application/json");
        res.body = result.dump(); return res;
    });

    // ==========================================
    // GET ТОВАРІВ КОНКРЕТНОГО ЗАМОВЛЕННЯ
    // ==========================================
    CROW_ROUTE(app, "/api/orders/<int>/details").methods(crow::HTTPMethod::Get)([](const crow::request &req, int orderId)
    {
        crow::response res;
        auto td = parseToken(req);
        if (!td.valid) { res.code = 401; res.body = "{\"error\":\"Не авторизовано\"}"; return res; }

        // Адмін може переглядати будь-яке замовлення, звичайний юзер — тільки своє
        Order allOrders[200];
        int orderCount = getAllOrders(allOrders, 200);
        bool owned = false;
        for (int i = 0; i < orderCount; i++) {
            if (allOrders[i].id == orderId) {
                if (td.role == 1 || allOrders[i].userId == td.userId) { owned = true; }
                break;
            }
        }
        if (!owned) { res.code = 403; res.body = "{\"error\":\"Доступ заборонено\"}"; return res; }

        OrderDetails details[100];
        int count = getDetailsByOrderId(orderId, details, 100);

        crow::json::wvalue::list list;
        for (int i = 0; i < count; i++) {
            Product p = findProductById(details[i].productId);
            crow::json::wvalue d;
            d["productId"]        = details[i].productId;
            d["priceAtPurchase"]  = details[i].priceAtPurchase;
            d["modelName"]        = p.id != -1 ? string(p.modelName)        : "Невідомо";
            d["shortDescription"] = p.id != -1 ? string(p.shortDescription) : "";
            list.push_back(std::move(d));
        }
        crow::json::wvalue result;
        result["details"] = std::move(list);
        res.code = 200; res.set_header("Content-Type", "application/json");
        res.body = result.dump(); return res;
    });

    CROW_ROUTE(app, "/api/users").methods(crow::HTTPMethod::Get)([](const crow::request &req)
                                                                 {
        crow::response res;
        auto td = parseToken(req);
        if (!td.valid || td.role != 1) { res.code = 403; res.body = "{\"error\":\"Доступ заборонено\"}"; return res; }

        User users[200]; int count = getAllUsers(users, 200);
        crow::json::wvalue::list list;
        for (int i = 0; i < count; i++) {
            crow::json::wvalue u;
            u["id"]               = users[i].id;
            u["name"]             = string(users[i].name);
            u["email"]            = string(users[i].email);
            u["role"]             = users[i].role;
            u["registrationDate"] = (long long)users[i].registrationDate;
            list.push_back(std::move(u));
        }
        crow::json::wvalue result = std::move(list);
        res.code = 200; res.set_header("Content-Type", "application/json"); res.body = result.dump(); return res; });

    // ==========================================
    // АДМІН: ВИДАЛЕННЯ КОРИСТУВАЧА
    // ==========================================
    CROW_ROUTE(app, "/api/users/<int>").methods(crow::HTTPMethod::Delete)([](const crow::request &req, int userId)
    {
        crow::response res;
        auto td = parseToken(req);
        if (!td.valid || td.role != 1) {
            res.code = 403; res.body = "{\"error\":\"Доступ заборонено\"}"; return res;
        }
        // Не дозволяємо адміну видалити самого себе
        if (td.userId == userId) {
            res.code = 400; res.body = "{\"error\":\"Не можна видалити власний акаунт\"}"; return res;
        }
        User target = findUserById(userId);
        if (target.id == -1) {
            res.code = 404; res.body = "{\"error\":\"Користувача не знайдено\"}"; return res;
        }
        if (deleteUser(userId)) {
            res.code = 200; res.body = "{\"status\":\"deleted\"}";
        } else {
            res.code = 500; res.body = "{\"error\":\"Помилка видалення\"}";
        }
        return res;
    });

    // ==========================================
    // АДМІН: GET ВСІХ ЗАМОВЛЕНЬ
    // ==========================================
    CROW_ROUTE(app, "/api/admin/orders").methods(crow::HTTPMethod::Get)([](const crow::request &req)
    {
        crow::response res;
        auto td = parseToken(req);
        if (!td.valid || td.role != 1) {
            res.code = 403; res.body = "{\"error\":\"Доступ заборонено\"}"; return res;
        }

        Order allOrders[200];
        int count = getAllOrders(allOrders, 200);

        User users[200];
        int userCount = getAllUsers(users, 200);

        crow::json::wvalue::list list;
        for (int i = 0; i < count; i++) {
            crow::json::wvalue o;
            o["id"]        = allOrders[i].id;
            o["userId"]    = allOrders[i].userId;
            o["status"]    = allOrders[i].status;
            o["totalCost"] = allOrders[i].totalCost;
            o["createdAt"] = (long long)allOrders[i].dateTime * 1000;

            // Підтягуємо ім'я та email юзера
            string userName = "", userEmail = "";
            for (int j = 0; j < userCount; j++) {
                if (users[j].id == allOrders[i].userId) {
                    userName  = string(users[j].name);
                    userEmail = string(users[j].email);
                    break;
                }
            }
            o["userName"]  = userName;
            o["userEmail"] = userEmail;

            list.push_back(std::move(o));
        }
        crow::json::wvalue result;
        result["orders"] = std::move(list);
        res.code = 200; res.set_header("Content-Type", "application/json");
        res.body = result.dump(); return res;
    });

    // ==========================================
    // АДМІН: ЗМІНА СТАТУСУ ЗАМОВЛЕННЯ
    // ==========================================
    CROW_ROUTE(app, "/api/admin/orders/<int>/status").methods(crow::HTTPMethod::Patch)([](const crow::request &req, int orderId)
    {
        crow::response res;
        auto td = parseToken(req);
        if (!td.valid || td.role != 1) {
            res.code = 403; res.body = "{\"error\":\"Доступ заборонено\"}"; return res;
        }

        auto x = crow::json::load(req.body);
        if (!x || !x.has("status")) {
            res.code = 400; res.body = "{\"error\":\"Поле status обов'язкове\"}"; return res;
        }

        int newStatus = (int)x["status"].i();
        if (newStatus < 1 || newStatus > 3) {
            res.code = 400; res.body = "{\"error\":\"Невалідний статус (1=очікує, 2=підтверджено, 3=відхилено)\"}"; return res;
        }

        // Знаходимо замовлення і оновлюємо статус
        Order target = findOrderById(orderId);
        if (target.id == -1) {
            res.code = 404; res.body = "{\"error\":\"Замовлення не знайдено\"}"; return res;
        }

        target.status = newStatus;
        if (!updateOrder(target)) {
            res.code = 500; res.body = "{\"error\":\"Помилка оновлення статусу\"}"; return res;
        }


        crow::json::wvalue result;
        result["id"]     = orderId;
        result["status"] = newStatus;
        res.code = 200; res.set_header("Content-Type", "application/json");
        res.body = result.dump(); return res;
    });

    // ==========================================
    // АДМІН: ДЕТАЛІ БУДЬ-ЯКОГО ЗАМОВЛЕННЯ
    // ==========================================
    CROW_ROUTE(app, "/api/admin/orders/<int>/details").methods(crow::HTTPMethod::Get)([](const crow::request &req, int orderId)
    {
        crow::response res;
        auto td = parseToken(req);
        if (!td.valid || td.role != 1) {
            res.code = 403; res.body = "{\"error\":\"Доступ заборонено\"}"; return res;
        }

        OrderDetails details[100];
        int count = getDetailsByOrderId(orderId, details, 100);

        crow::json::wvalue::list list;
        for (int i = 0; i < count; i++) {
            Product p = findProductById(details[i].productId);
            crow::json::wvalue d;
            d["productId"]        = details[i].productId;
            d["priceAtPurchase"]  = details[i].priceAtPurchase;
            d["modelName"]        = p.id != -1 ? string(p.modelName)        : "Невідомо";
            d["shortDescription"] = p.id != -1 ? string(p.shortDescription) : "";
            list.push_back(std::move(d));
        }
        crow::json::wvalue result;
        result["details"] = std::move(list);
        res.code = 200; res.set_header("Content-Type", "application/json");
        res.body = result.dump(); return res;
    });

    // ==========================================
    // ВИМКНЕННЯ СЕРВЕРА
    // ==========================================
    CROW_ROUTE(app, "/api/shutdown")([&app]()
                                     {
        app.stop();
        return "Сервер C++ успішно зупинено. Консоль можна закривати."; });

    cout << "\n==========================================" << endl;
    cout << "Сервер Crow запущено на http://127.0.0.1:18080" << endl;
    cout << "Для вимкнення перейдіть на: http://127.0.0.1:18080/api/shutdown" << endl;
    cout << "==========================================" << endl;

    app.port(18080).bindaddr("127.0.0.1").multithreaded().run();
    return 0;
}