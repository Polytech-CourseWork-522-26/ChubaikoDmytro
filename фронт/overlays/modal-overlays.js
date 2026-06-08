// overlays/modal-overlays.js

window.API = window.API || 'http://127.0.0.1:18080';
const baseFolder = '../overlays/';

// ─────────────────────────────────────────────
//  ТОКЕН
// ─────────────────────────────────────────────
function getToken() {
    return localStorage.getItem('token') || '';
}

// ─────────────────────────────────────────────
//  ЗАВАНТАЖЕННЯ МОДАЛЬНОГО ВІКНА
// ─────────────────────────────────────────────
window.loadModalContent = async function (fileName) {
    const modal        = document.getElementById('accountModal');
    const modalContent = document.getElementById('modalContent');
    if (!modal || !modalContent) return;

    try {
        if (fileName === 'login.html' && getToken()) {
            fileName = 'account.html';
        }

        const response = await fetch(baseFolder + fileName);
        if (!response.ok) throw new Error(`Не вдалося знайти файл: ${fileName}`);

        const html = await response.text();
        const doc  = new DOMParser().parseFromString(html, 'text/html');
        const card = doc.querySelector('.card') ||
                     doc.querySelector('.account-card') ||
                     doc.body.firstElementChild;

        if (!card) return;

        modalContent.innerHTML = '';
        modalContent.appendChild(card);
        modal.style.setProperty('display', 'flex', 'important');

        setupInternalLinks();
        setupFormHandlers();

        // ── Профіль ──
        if (fileName === 'account.html') {
            const nameEl   = document.getElementById('user-profile-name');
            const emailEl  = document.getElementById('user-profile-email');
            const adminBtn = document.getElementById('admin-panel-btn');
            const ordersBtn = document.getElementById('orders-btn');
            const logoutBtn = document.getElementById('logout-btn');

            if (nameEl)  nameEl.innerHTML   = `Вітаю,<br>${localStorage.getItem('userName') || 'Гість'}!`;
            if (emailEl) emailEl.textContent = localStorage.getItem('userEmail') || '';

            if (adminBtn && localStorage.getItem('userRole') === '1') {
                adminBtn.style.display = 'block';
                adminBtn.addEventListener('click', () => {
                    modal.style.display = 'none';
                    if (typeof window.loadAdminMenu === 'function') {
                        window.loadAdminMenu();
                    } else {
                        const script = document.createElement('script');
                        script.src = '../admin-panel/scripts/modal-admin.js';
                        script.onload = () => window.loadAdminMenu();
                        document.head.appendChild(script);
                    }
                });
            }

            if (ordersBtn) {
                ordersBtn.addEventListener('click', () => {
                    if (!getToken()) {
                        window.loadModalContent('login.html');
                        return;
                    }
                    modal.style.display = 'none';
                    if (localStorage.getItem('userRole') === '1') {
                        window.location.href = '../order/cart-admin.html';
                    } else {
                        window.location.href = '../order/order.html';
                    }
                });
            }

            if (logoutBtn) {
                logoutBtn.addEventListener('click', () => {
                    localStorage.removeItem('token');
                    localStorage.removeItem('userEmail');
                    localStorage.removeItem('userName');
                    localStorage.removeItem('userRole');
                    modal.style.display = 'none';
                    alert('Ви успішно вийшли з акаунту');
                });
            }
        }

    } catch (error) {
        console.error('Помилка завантаження модального вікна:', error);
    }
};

// ─────────────────────────────────────────────
//  ЗАВАНТАЖЕННЯ КОШИКА
// ─────────────────────────────────────────────
window.loadCartContent = async function () {
    if (document.getElementById('cartOverlay')) {
        if (typeof window.openCart === 'function') window.openCart();
        return;
    }

    try {
        const response = await fetch('../order/cart.html');
        if (!response.ok) throw new Error('Не вдалося знайти cart.html');

        const html = await response.text();
        const doc  = new DOMParser().parseFromString(html, 'text/html');
        const cartOverlay = doc.getElementById('cartOverlay');
        if (!cartOverlay) throw new Error('#cartOverlay не знайдено');

        document.body.appendChild(cartOverlay);

        if (!document.querySelector('link[href*="cart.css"]')) {
            const link = document.createElement('link');
            link.rel  = 'stylesheet';
            link.href = '../order/css/cart.css';
            document.head.appendChild(link);
        }

        if (!document.querySelector('script[src*="modal-order.js"]')) {
            const script = document.createElement('script');
            script.src = '../order/scripts/modal-order.js';
            script.onload = () => {
                if (typeof window.openCart === 'function') window.openCart();
            };
            document.head.appendChild(script);
        } else {
            if (typeof window.openCart === 'function') window.openCart();
        }

    } catch (error) {
        console.error('Помилка завантаження кошика:', error);
    }
};

// ─────────────────────────────────────────────
//  ДОДАТИ ТОВАР У КОШИК
// ─────────────────────────────────────────────
window.addToCartServer = async function (productId) {
    if (!productId) return;

    if (!getToken()) {
        window.loadModalContent('login.html');
        return;
    }

    try {
        const cartRes = await fetch(`${window.API}/api/cart`, {
            headers: { 'Authorization': `Bearer ${getToken()}` }
        });

        if (cartRes.ok) {
            const cartData = await cartRes.json();
            const already  = (cartData.items || []).some(
                item => Number(item.productId) === Number(productId)
            );
            if (already) {
                alert('Цей товар вже є у вашому кошику.');
                window.loadCartContent();
                return;
            }
        }

        const res = await fetch(`${window.API}/api/cart/${productId}`, {
            method:  'POST',
            headers: { 'Authorization': `Bearer ${getToken()}` }
        });

        if (res.ok) {
            window.loadCartContent();
        } else {
            const err = await res.json().catch(() => ({}));
            if (res.status === 401 || res.status === 403) {
                alert('Будь ласка, увійдіть до акаунту!');
                window.loadModalContent('login.html');
            } else {
                alert(err.error || 'Помилка додавання до кошика');
            }
        }
    } catch (err) {
        console.error('Помилка кошика:', err);
        alert("Не вдалося з'єднатися з сервером!");
    }
};

// ─────────────────────────────────────────────
//  ВНУТРІШНІ ПОСИЛАННЯ (data-modal-target)
// ─────────────────────────────────────────────
function setupInternalLinks() {
    document.querySelectorAll('[data-modal-target]').forEach(link => {
        link.addEventListener('click', (e) => {
            e.preventDefault();
            window.loadModalContent(link.getAttribute('data-modal-target'));
        });
    });
}

// ─────────────────────────────────────────────
//  ОБРОБНИКИ ФОРМ
// ─────────────────────────────────────────────
function setupFormHandlers() {

    // ── ЛОГІН ──
    const loginForm = document.getElementById('loginForm');
    if (loginForm) {
        loginForm.addEventListener('submit', async (e) => {
            e.preventDefault();
            const email    = document.getElementById('login-email').value.trim();
            const password = document.getElementById('login-password').value.trim();

            if (!email || !password) {
                alert('Введіть email і пароль');
                return;
            }

            try {
                const res = await fetch(`${window.API}/api/login`, {
                    method:  'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body:    JSON.stringify({ email, password })
                });

                if (res.ok) {
                    const data = await res.json();
                    localStorage.setItem('token',     data.token);
                    localStorage.setItem('userName',  data.name);
                    localStorage.setItem('userEmail', data.email);
                    localStorage.setItem('userRole',  String(data.role));
                    window.loadModalContent('account.html');
                } else {
                    const err = await res.json().catch(() => ({}));
                    alert(err.error || 'Невірний email або пароль!');
                }
            } catch (err) {
                console.error('Помилка авторизації:', err);
                alert("Не вдалося з'єднатися з сервером.");
            }
        });
    }

    // ── РЕЄСТРАЦІЯ ──
    const registerForm = document.getElementById('registerForm');
    if (registerForm) {
        registerForm.addEventListener('submit', async (e) => {
            e.preventDefault();
            const name     = document.getElementById('reg-name').value.trim();
            const email    = document.getElementById('reg-email').value.trim();
            const password = document.getElementById('reg-password').value.trim();

            if (name.length < 2) {
                alert("Ім'я має бути мінімум 2 символи!");
                return;
            }
            if (password.length < 4) {
                alert('Пароль має бути мінімум 4 символи!');
                return;
            }
            if (!email.includes('@')) {
                alert('Введіть коректний email!');
                return;
            }

            try {
                const res = await fetch(`${window.API}/api/register`, {
                    method:  'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body:    JSON.stringify({ name, email, password })
                });

                if (res.ok) {
                    const data = await res.json();
                    localStorage.setItem('token',     data.token);
                    localStorage.setItem('userName',  data.name);
                    localStorage.setItem('userEmail', data.email);
                    localStorage.setItem('userRole',  String(data.role));
                    window.loadModalContent('account.html');
                } else {
                    const err = await res.json().catch(() => ({}));
                    alert(err.error || 'Помилка реєстрації');
                }
            } catch (err) {
                console.error('Помилка реєстрації:', err);
                alert("Не вдалося з'єднатися з сервером.");
            }
        });
    }
}

// ─────────────────────────────────────────────
//  ІНІЦІАЛІЗАЦІЯ
// ─────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
    const profileBtn = document.querySelector('.ProfileButton');
    const cartBtn    = document.querySelector('.CartButton');
    const modal      = document.getElementById('accountModal');

    if (profileBtn) {
        profileBtn.addEventListener('click', (e) => {
            e.preventDefault();
            window.loadModalContent('login.html');
        });
    }

    if (cartBtn) {
        cartBtn.addEventListener('click', (e) => {
            e.preventDefault();
            if (!getToken()) {
                window.loadModalContent('login.html');
                return;
            }
            if (localStorage.getItem('userRole') === '1') {
                window.location.href = '../order/cart-admin.html';
            } else {
                window.loadCartContent();
            }
        });
    }

    if (modal) {
        modal.addEventListener('click', (e) => {
            if (e.target === modal) modal.style.display = 'none';
        });
    }

    // ── Кнопка замовлень у хедері ──
    const ordersBtn = document.querySelector('.OrdersButton');
    if (ordersBtn) {
        ordersBtn.addEventListener('click', (e) => {
            e.preventDefault();
            if (!getToken()) {
                window.loadModalContent('login.html');
                return;
            }
            if (localStorage.getItem('userRole') === '1') {
                window.location.href = '../order/cart-admin.html';
            } else {
                window.location.href = '../order/order.html';
            }
        });
    }
});