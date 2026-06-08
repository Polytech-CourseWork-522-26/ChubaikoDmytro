# Chrono Emporium

Вебзастосунок інтернет-магазину преміальних годинників.

## Стек
- **Бекенд:** C++ (Crow framework)
- **Фронтенд:** Vanilla JS, HTML5, CSS3
- **Зберігання:** бінарні файли (.dat)

## Запуск
```bash
cd бек
g++ -O3 main.cpp Users/UserRepository.cpp Product/ProductRepository.cpp order/OrderRepository.cpp orderdateil/OrderDetailsRepository.cpp -o app.exe -I "../libs" -I "../libs/asio-1.36.0/include" -lws2_32 -lmswsock
./app.exe
```
Відкрий `фронт/main/index.html` через Live Server.
