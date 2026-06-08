// admin-panel/scripts/modal-admin.js
// Вся логіка адмін-панелі: меню-оверлей + сторінка користувачів

// ─────────────────────────────────────────────
//  ЗАВАНТАЖЕННЯ АДМІН-МЕНЮ (оверлей поверх акаунту)
// ─────────────────────────────────────────────
window.loadAdminMenu = async function () {
    const modal        = document.getElementById('accountModal');
    const modalContent = document.getElementById('modalContent');
    if (!modal || !modalContent) return;

    // Підключаємо CSS один раз
    if (!document.querySelector('link[href*="admin-menu.css"]')) {
        const link = document.createElement('link');
        link.rel  = 'stylesheet';
        link.href = '../admin-panel/admin-menu.css';
        document.head.appendChild(link);
    }

    try {
        const response = await fetch('../admin-panel/admin-menu.html');
        if (!response.ok) throw new Error('Не вдалося знайти admin-menu.html');

        const html = await response.text();
        const doc  = new DOMParser().parseFromString(html, 'text/html');
        const card = doc.querySelector('.admin-menu-card') || doc.body.firstElementChild;
        if (!card) return;

        modalContent.innerHTML = '';
        modalContent.appendChild(card);
        modal.style.setProperty('display', 'flex', 'important');

        // ── Кнопка закриття ──
        const closeBtn = document.getElementById('admin-menu-close-btn');
        if (closeBtn) closeBtn.addEventListener('click', () => {
            modal.style.display = 'none';
        });

        // ── Пункти навігації ──
        const ordersItem = document.getElementById('adminmenu-orders');
        if (ordersItem) ordersItem.addEventListener('click', (e) => {
            e.preventDefault();
            modal.style.display = 'none';
            window.location.href = '../order/cart-admin.html';
        });

        const usersItem = document.getElementById('adminmenu-users');
        if (usersItem) usersItem.addEventListener('click', (e) => {
            e.preventDefault();
            modal.style.display = 'none';
            window.location.href = '../admin-panel/users-admin.html';
        });

        const addProductItem = document.getElementById('adminmenu-add-product');
        if (addProductItem) addProductItem.addEventListener('click', (e) => {
            e.preventDefault();
            modal.style.display = 'none';
            window.location.href = '../catalog/add-product.html';
        });

        const catalogItem = document.getElementById('adminmenu-catalog');
        if (catalogItem) catalogItem.addEventListener('click', (e) => {
            e.preventDefault();
            modal.style.display = 'none';
            window.location.href = '../catalog/catalog.html';
        });

    } catch (error) {
        console.error('Помилка завантаження адмін меню:', error);
    }
};

// ─────────────────────────────────────────────
//  ЛОГІКА СТОРІНКИ КОРИСТУВАЧІВ (users-admin.html)
// ─────────────────────────────────────────────
(function initUsersAdminPage() {
    // Запускаємо лише якщо на сторінці є відповідні елементи
    const usersList = document.getElementById('usersList');
    if (!usersList) return;

    const API = window.API || 'http://127.0.0.1:18080';
    let allUsers = [];

    // Захист — тільки адмін
    document.addEventListener('DOMContentLoaded', () => {
        const token = localStorage.getItem('token');
        const role  = localStorage.getItem('userRole');
        if (!token || role !== '1') {
            window.location.href = '../main/index.html';
            return;
        }
        loadUsers();
    });

    async function loadUsers() {
        const token = localStorage.getItem('token');
        try {
            const res = await fetch(`${API}/api/users`, {
                headers: { 'Authorization': `Bearer ${token}` }
            });
            if (!res.ok) throw new Error('HTTP ' + res.status);
            allUsers = await res.json();
            renderUsers(allUsers);
            updateStats(allUsers);
        } catch (err) {
            console.error('Помилка завантаження:', err);
            usersList.innerHTML = '<p class="UsersEmpty">Помилка завантаження користувачів.</p>';
        }
    }

    function renderUsers(users) {
        if (users.length === 0) {
            usersList.innerHTML = '<p class="UsersEmpty">Користувачів не знайдено.</p>';
            return;
        }
        // Токен має формат "userId:role", беремо першу частину
        const currentUserId = parseInt((localStorage.getItem('token') || '').split(':')[0]);
        usersList.innerHTML = users.map(u => {
            const idStr   = '#' + String(u.id).padStart(3, '0');
            const roleCls = u.role === 1 ? 'admin' : 'user';
            const roleStr = u.role === 1 ? 'АДМІН' : 'КОРИСТУВАЧ';
            const dateStr = u.registrationDate
                ? new Date(u.registrationDate * 1000).toLocaleDateString('uk-UA', {
                    day: '2-digit', month: '2-digit', year: 'numeric'
                  })
                : '—';
            // Кнопку видалення не показуємо для самого себе
            const isSelf = u.id === currentUserId;
            const deleteBtn = isSelf
                ? ''
                : `<button class="AdminDeleteBtn" onclick="deleteUser(${u.id}, '${esc(u.name)}')">✕</button>`;
            return `
                <div class="AdminUserRow" id="user-row-${u.id}">
                    <div class="AdminUserId">${esc(idStr)}</div>
                    <div class="AdminUserName">${esc(u.name)}</div>
                    <div class="AdminUserEmail">${esc(u.email)}</div>
                    <div class="AdminUserDate">${dateStr}</div>
                    <div class="AdminUserRoleCell">
                        <span class="AdminRolePill ${roleCls}">${roleStr}</span>
                        ${deleteBtn}
                    </div>
                </div>
            `;
        }).join('');
    }

    window.deleteUser = async function (userId, userName) {
        if (!confirm(`Видалити користувача "${userName}"?\nЦю дію не можна скасувати.`)) return;

        const token = localStorage.getItem('token');
        try {
            const res = await fetch(`${API}/api/users/${userId}`, {
                method: 'DELETE',
                headers: { 'Authorization': `Bearer ${token}` }
            });
            if (res.ok) {
                // Видаляємо рядок з DOM без перезавантаження сторінки
                const row = document.getElementById(`user-row-${userId}`);
                if (row) row.remove();
                allUsers = allUsers.filter(u => u.id !== userId);
                updateStats(allUsers);
            } else {
                const err = await res.json().catch(() => ({}));
                alert(err.error || 'Помилка видалення');
            }
        } catch (err) {
            console.error('Помилка:', err);
            alert("Не вдалося з'єднатися з сервером.");
        }
    };

    function updateStats(users) {
        const admins  = users.filter(u => u.role === 1).length;
        const regular = users.filter(u => u.role === 0).length;
        const elTotal  = document.getElementById('statTotal');
        const elUsers  = document.getElementById('statUsers');
        const elAdmins = document.getElementById('statAdmins');
        if (elTotal)  elTotal.textContent  = users.length;
        if (elUsers)  elUsers.textContent  = regular;
        if (elAdmins) elAdmins.textContent = admins;
    }

    // Глобальна функція пошуку (викликається з oninput в HTML)
    window.searchUsers = function (val) {
        const q = val.toLowerCase().trim();
        const filtered = allUsers.filter(u =>
            !q ||
            u.name.toLowerCase().includes(q) ||
            u.email.toLowerCase().includes(q)
        );
        renderUsers(filtered);
    };

    function esc(str) {
        return String(str || '')
            .replace(/&/g, '&amp;').replace(/</g, '&lt;')
            .replace(/>/g, '&gt;').replace(/"/g, '&quot;');
    }
})();