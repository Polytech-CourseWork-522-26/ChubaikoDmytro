// order/scripts/modal-order.js

window.API = window.API || 'http://127.0.0.1:18080';

window.openCart = function(isEmbedded = false) {
    if (!isEmbedded) {
        const overlay = document.getElementById('cartOverlay');
        if (overlay) overlay.style.display = 'flex';
    }

    const token = localStorage.getItem('token');
    if (!token) {
        window.setCartState('empty');
        return;
    }
    window.loadCart();
};

window.closeCart = function() {
    const overlay = document.getElementById('cartOverlay');
    if (overlay) overlay.style.display = 'none';

    const mainModal = document.getElementById('accountModal');
    if (mainModal) mainModal.style.display = 'none';
};

window.handleBg = function(event) {
    if (event.target === document.getElementById('cartOverlay')) {
        window.closeCart();
    }
};

window.setCartState = function(state) {
    const filledBody = document.getElementById('filledBody');
    const emptyBody = document.getElementById('emptyBody');
    const successBody = document.getElementById('successBody');
    const footer = document.getElementById('cartFooter');

    if (filledBody) filledBody.style.display = 'none';
    if (emptyBody) emptyBody.style.display = 'none';
    if (successBody) successBody.style.display = 'none';
    if (footer) footer.style.display = 'none';

    if (state === 'filled') {
        if (filledBody) filledBody.style.display = 'block';
        if (footer) footer.style.display = 'block';
    } else if (state === 'empty') {
        if (emptyBody) emptyBody.style.display = 'flex';
    } else if (state === 'success') {
        if (successBody) successBody.style.display = 'flex';
    }
};

window.loadCart = async function() {
    const token = localStorage.getItem('token');
    if (!token) return;

    try {
        const res = await fetch(`${window.API}/api/cart`, {
            headers: { 'Authorization': `Bearer ${token}` }
        });

        if (!res.ok) {
            window.setCartState('empty');
            return;
        }

        const data = await res.json();
        const items = data.items || [];

        if (items.length === 0) {
            window.setCartState('empty');
            return;
        }

        const filledBody = document.getElementById('filledBody');
        if (filledBody) {
            filledBody.innerHTML = '';
            let total = 0;

            items.forEach(item => {
                total += Number(item.priceAtPurchase) || 0;
                const div = document.createElement('div');
                div.className = 'cart-item';
                div.innerHTML = `
                    <img src="${window.API}/api/products/image/${item.productId}" class="cart-item-img" alt="${item.modelName}" onerror="this.onerror=null; this.src='../images/logo.png'; this.style.objectFit='contain'; this.style.padding='10px';">
                    <div class="cart-item-info">
                        <h4>${item.modelName}</h4>
                        <div class="cart-item-price">$${Number(item.priceAtPurchase).toFixed(2)}</div>
                    </div>
                    <button class="cart-item-remove" onclick="window.deleteCartItem(${item.productId})">Видалити</button>
                `;
                filledBody.appendChild(div);
            });

            const totalEl = document.getElementById('totalPrice');
            if (totalEl) totalEl.textContent = `$${total.toFixed(2)}`;
            window.setCartState('filled');
        }
    } catch (error) {
        console.error("Помилка завантаження кошика:", error);
    }
};

window.deleteCartItem = async function(productId) {
    const token = localStorage.getItem('token');
    try {
        const res = await fetch(`${window.API}/api/cart/${productId}`, {
            method: 'DELETE',
            headers: { 'Authorization': `Bearer ${token}` }
        });
        if (res.ok) {
            window.loadCart();
        } else {
            alert("Не вдалося видалити товар");
        }
    } catch (e) {
        console.error("Помилка видалення:", e);
    }
};

window.checkout = async function() {
    const token = localStorage.getItem('token');
    if (!token) {
        alert('Увійдіть в систему для оформлення замовлення');
        return;
    }

    try {
        const res = await fetch(`${window.API}/api/orders`, {
            method: 'POST',
            headers: { 'Authorization': `Bearer ${token}` }
        });

        if (res.ok) {
            window.setCartState('success');
        } else {
            const err = await res.json().catch(() => ({}));
            alert(err.error || 'Помилка при оформленні замовлення');
        }
    } catch (error) {
        console.error('Помилка оформлення:', error);
        alert('Не вдалося з\'єднатися з сервером');
    }
};

window.goToOrders = function() {
    window.closeCart();
    const path = window.location.pathname;
    if (path.includes('/order/')) {
        window.location.href = 'order.html';
    } else if (path.includes('/product/') || path.includes('/catalog/') || path.includes('/overlays/')) {
        window.location.href = '../order/order.html';
    } else {
        window.location.href = 'order/order.html';
    }
};


// ==========================================
// ІСТОРІЯ ЗАМОВЛЕНЬ КОРИСТУВАЧА
// ==========================================

document.addEventListener('DOMContentLoaded', function() {
    if (!window.location.pathname.includes('order.html')) return;
    window.initOrdersPage();
});

window.initOrdersPage = async function() {
    const token = localStorage.getItem('token');
    const listEl = document.getElementById('ordersList');
    const countEl = document.getElementById('ordersCount');
    const metaEl = document.querySelector('.OrdersMeta');

   if (!token) {
    if (listEl) listEl.innerHTML = `
        <p class="orders-empty">Увійдіть в систему для перегляду замовлень.</p>
        <button class="BtnBlack" onclick="document.getElementById('profileBtn') && document.getElementById('profileBtn').click()">
            Увійти
        </button>
    `;
    return;
}

    // Показуємо дані користувача в заголовку
    const parts = token.split(':');
    if (metaEl && parts.length === 2) {
        metaEl.textContent = `ID користувача: #${parts[0]}`;
    }

    try {
        const res = await fetch(`${window.API}/api/orders`, {
            headers: { 'Authorization': `Bearer ${token}` }
        });

        if (!res.ok) {
            if (listEl) listEl.innerHTML = '<p class="orders-empty">Не вдалося завантажити замовлення.</p>';
            return;
        }

        const data = await res.json();
        const orders = data.orders || [];

        if (countEl) countEl.textContent = `${orders.length} ${window.ordersWord(orders.length)}`;

        if (orders.length === 0) {
            if (listEl) listEl.innerHTML = '<p class="orders-empty">У вас ще немає замовлень.</p>';
            return;
        }

        if (listEl) listEl.innerHTML = '';

        for (const order of orders) {
            const row = await window.buildOrderRow(order, token);
            if (listEl) listEl.appendChild(row);
        }

    } catch (err) {
        console.error('Помилка завантаження замовлень:', err);
        if (listEl) listEl.innerHTML = '<p class="orders-empty">Помилка з\'єднання з сервером.</p>';
    }
};

window.buildOrderRow = async function(order, token) {
    const statusMap = {
        1: { label: 'ОЧІКУЄ',       cls: 'pending',  msg: 'Ваше замовлення прийнято. Менеджер обробить ваше замовлення найближчим часом.' },
        2: { label: 'ПІДТВЕРДЖЕНО', cls: 'approved', msg: 'Замовлення підтверджено. З вами зв\'яжуться для уточнення деталей доставки.' },
        3: { label: 'ВІДХИЛЕНО',    cls: 'rejected', msg: 'На жаль, це замовлення було відхилено. Зверніться до підтримки.' }
    };
    const s = statusMap[order.status] || statusMap[1];

    // Завантажуємо деталі замовлення
    let items = [];
    try {
        const res = await fetch(`${window.API}/api/orders/${order.id}/details`, {
            headers: { 'Authorization': `Bearer ${token}` }
        });
        if (res.ok) {
            const d = await res.json();
            items = d.details || [];
        }
    } catch (e) {
        console.error('Помилка деталей замовлення:', e);
    }

    // Будуємо HTML товарів
    const itemsHTML = items.map(item => `
        <div class="OrderItemRow">
            <div class="OrderItemImg">
                <img src="${window.API}/api/products/image/${item.productId}"
                     alt="${item.modelName}"
                     onerror="this.onerror=null;this.style.display='none';this.parentElement.textContent='⌚';">
            </div>
            <div style="flex:1">
                <div class="OrderItemName">${item.modelName}</div>
                <div class="OrderItemDesc">${item.shortDescription}</div>
            </div>
            <div class="OrderItemPrice">$${Number(item.priceAtPurchase).toFixed(2)}</div>
        </div>
    `).join('');

    const row = document.createElement('div');
    row.className = 'OrderRow';
    row.dataset.status = s.cls;
    row.innerHTML = `
        <div class="OrderRowTop" onclick="toggleOrder(this.parentElement)">
            <span class="OrderId">#${String(order.id).padStart(3, '0')}</span>
            <span class="OrderDate">—</span>
            <span class="OrderAmount">$${Number(order.totalCost).toFixed(2)}</span>
            <span class="OrderStatusPill ${s.cls}">${s.label}</span>
            <span class="OrderChevron">&#8964;</span>
        </div>
        <div class="OrderDetail">
            <div class="OrderDetailGrid">
                <div>${itemsHTML}</div>
                <div class="OrderSideInfo">
                    <div class="OrderInfoLabel">ДЕТАЛІ ЗАМОВЛЕННЯ</div>
                    <div class="OrderInfoRow"><span>Замовлення</span><span>#${String(order.id).padStart(3, '0')}</span></div>
                    <div class="OrderInfoRow"><span>Товарів</span><span>${items.length} шт</span></div>
                    <div class="OrderInfoRow total"><span>Сума</span><span>$${Number(order.totalCost).toFixed(2)}</span></div>
                    <div class="OrderStatusMsg ${s.cls}">${s.msg}</div>
                </div>
            </div>
        </div>
    `;
    return row;
};

window.ordersWord = function(n) {
    if (n === 1) return 'замовлення';
    if (n >= 2 && n <= 4) return 'замовлення';
    return 'замовлень';
};

window.toggleOrder = function(row) {
    const wasOpen = row.classList.contains('open');
    document.querySelectorAll('.OrderRow.open').forEach(r => r.classList.remove('open'));
    if (!wasOpen) row.classList.add('open');
};


// ==========================================
// АДМІН — ПАНЕЛЬ ЗАМОВЛЕНЬ
// ==========================================

const ADMIN_STATUS_MAP = {
    1: { label: 'ОЧІКУЄ',       cls: 'pending'  },
    2: { label: 'ПІДТВЕРДЖЕНО', cls: 'approved' },
    3: { label: 'ВІДХИЛЕНО',    cls: 'rejected' }
};
const ADMIN_STATUS_BY_CLS = { pending: 1, approved: 2, rejected: 3 };

document.addEventListener('DOMContentLoaded', function () {
    const token = localStorage.getItem('token');
    const role  = localStorage.getItem('userRole');

    // Захист сторінки адміна
    if (window.location.pathname.includes('cart-admin')) {
        if (!token || role !== '1') {
            window.location.href = '../main/index.html';
            return;
        }
        window.initAdminPage();
    }

    // Захист сторінки замовлень користувача
    if (window.location.pathname.includes('order.html')) {
        if (!token) {
            window.location.href = '../main/index.html';
            return;
        }
    }
});

window.initAdminPage = async function () {
    const token  = localStorage.getItem('token');
    const listEl = document.getElementById('adminList');

    if (!token) {
        if (listEl) listEl.innerHTML = window.adminMsg('Увійдіть в систему як адміністратор.');
        return;
    }

    try {
        const res = await fetch(`${window.API}/api/admin/orders`, {
            headers: { 'Authorization': `Bearer ${token}` }
        });

        if (res.status === 403) {
            if (listEl) listEl.innerHTML = window.adminMsg('Недостатньо прав доступу.');
            return;
        }
        if (!res.ok) throw new Error('HTTP ' + res.status);

        const data   = await res.json();
        const orders = data.orders || [];

        if (listEl) listEl.innerHTML = '';

        if (orders.length === 0) {
            if (listEl) listEl.innerHTML = window.adminMsg('Замовлень ще немає.');
            window.updateAdminStats();
            return;
        }

        for (const order of orders) {
            if (listEl) listEl.appendChild(window.buildAdminOrderBlock(order));
        }

        window.updateAdminStats();

    } catch (err) {
        console.error('Помилка завантаження:', err);
        if (listEl) listEl.innerHTML = window.adminMsg("Помилка з'єднання з сервером.");
    }
};

window.adminMsg = function (text) {
    return `<p style="font-family:'Roboto Mono',monospace;font-size:13px;color:rgba(145,145,145,1);padding:40px 0;">${text}</p>`;
};

window.buildAdminOrderBlock = function (order) {
    const s         = ADMIN_STATUS_MAP[order.status] || ADMIN_STATUS_MAP[1];
    const dateStr   = order.createdAt ? window.adminFormatDate(order.createdAt) : '—';
    const amt       = `$${Number(order.totalCost).toFixed(2)}`;
    const idStr     = '#' + String(order.id).padStart(3, '0');
    const userName  = order.userName || order.userLogin || order.userEmail || ('Користувач #' + order.userId);
    const userEmail = order.userEmail || '';
    const userId    = order.userId || '';
    const searchKey = [order.id, userName, userEmail].join(' ').toLowerCase();

    const block = document.createElement('div');
    block.className       = 'AdminOrderBlock';
    block.dataset.status  = s.cls;
    block.dataset.search  = searchKey;
    block.dataset.orderId = order.id;
    block.dataset.loaded  = 'false';
    block.dataset.userName  = userName;
    block.dataset.userEmail = userEmail;
    block.dataset.userId    = userId;
    block.dataset.date      = dateStr;
    block.dataset.amt       = Number(order.totalCost).toFixed(2);

    block.innerHTML = `
        <div class="AdminOrderRow" onclick="toggleAdmin(this.parentElement)">
            <div class="AdminOrderId">${idStr}</div>
            <div class="AdminUserInfo">
                <div class="AdminUserName">${window.adminEsc(userName)}</div>
                <div class="AdminUserEmail">${window.adminEsc(userEmail)}${userId ? ' · ID: ' + userId : ''}</div>
            </div>
            <div class="AdminOrderDate">${dateStr}</div>
            <div class="AdminOrderAmt">${amt}</div>
            <div><span class="AdminStatusPill ${s.cls}" id="apill-${order.id}">${s.label}</span></div>
            <div class="AdminQuickActions" id="aqact-${order.id}" onclick="event.stopPropagation()">
                ${window.adminQuickActionsHTML(order.id, s.cls)}
            </div>
            <div class="AdminOrderChevron">&#8964;</div>
        </div>
        <div class="AdminOrderDetail" id="adetail-${order.id}"></div>
    `;

    return block;
};

window.toggleAdmin = async function (block) {
    const wasOpen = block.classList.contains('open');
    document.querySelectorAll('.AdminOrderBlock.open').forEach(b => b.classList.remove('open'));
    if (wasOpen) return;

    block.classList.add('open');

    if (block.dataset.loaded === 'false') {
        block.dataset.loaded = 'loading';
        await window.loadAdminOrderDetails(block);
    }
};

window.loadAdminOrderDetails = async function (block) {
    const orderId   = block.dataset.orderId;
    const detailEl  = document.getElementById('adetail-' + orderId);
    const token     = localStorage.getItem('token');
    const statusCls = block.dataset.status;

    const userName  = block.dataset.userName  || '—';
    const userEmail = block.dataset.userEmail || '—';
    const userId    = block.dataset.userId    || '—';
    const dateText  = block.dataset.date      || '—';
    const amtRaw    = block.dataset.amt       || '0';

    // Показуємо лоадер поки завантажуємо
    detailEl.innerHTML = `<p style="font-family:'Roboto Mono',monospace;font-size:12px;color:rgba(145,145,145,1);padding:20px 0;">Завантаження деталей...</p>`;

    try {
        const res = await fetch(`${window.API}/api/orders/${orderId}/details`, {
            headers: { 'Authorization': `Bearer ${token}` }
        });

        let items = [];
        if (res.ok) {
            const d = await res.json();
            items = d.details || [];
        }

        const itemsHTML = items.length > 0
            ? items.map(item => `
                <div class="AdminItemRow">
                    <div class="AdminItemImg">
                        <img src="${window.API}/api/products/image/${item.productId}"
                             alt="${window.adminEsc(item.modelName)}"
                             style="width:100%;height:100%;object-fit:cover;border-radius:8px;"
                             onerror="this.onerror=null;this.style.display='none';this.parentElement.textContent='⌚';">
                    </div>
                    <div style="flex:1">
                        <div class="AdminItemName">${window.adminEsc(item.modelName)}</div>
                        <div class="AdminItemMeta">${window.adminEsc(item.shortDescription || '')}</div>
                    </div>
                    <div class="AdminItemPrice">$${Number(item.priceAtPurchase).toFixed(2)}</div>
                </div>
            `).join('')
            : `<p style="font-family:'Roboto Mono',monospace;font-size:12px;color:rgba(145,145,145,1);">Товари не знайдено</p>`;

        detailEl.innerHTML = `
            <div class="AdminDetailGrid">
                <div>${itemsHTML}</div>
                <div class="AdminSidePanel">
                    <div class="AdminSidePanelLabel">ДЕТАЛІ ЗАМОВЛЕННЯ</div>
                    <div class="AdminSideRow"><span>Замовлення</span><span>#${String(orderId).padStart(3, '0')}</span></div>
                    <div class="AdminSideRow"><span>Дата</span><span>${dateText}</span></div>
                    <div class="AdminSideRow"><span>Покупець</span><span>${window.adminEsc(userName)}</span></div>
                    <div class="AdminSideRow"><span>Email</span><span>${window.adminEsc(userEmail)}</span></div>
                    <div class="AdminSideRow"><span>User ID</span><span>#${userId}</span></div>
                    <div class="AdminSideRow"><span>Товарів</span><span>${items.length} шт</span></div>
                    <div class="AdminSideRow total"><span>Сума</span><span>$${Number(amtRaw).toFixed(2)}</span></div>
                    <div class="AdminActionBtns" id="aact-${orderId}">
                        ${window.adminDetailActionsHTML(orderId, statusCls)}
                    </div>
                </div>
            </div>
        `;

        block.dataset.loaded = 'true';

    } catch (err) {
        console.error('Помилка деталей:', err);
        detailEl.innerHTML = `<p style="font-family:'Roboto Mono',monospace;font-size:12px;color:rgba(145,145,145,1);padding:20px 0;">Помилка завантаження деталей.</p>`;
        block.dataset.loaded = 'false';
    }
};

window.adminStatus = async function (orderId, newCls) {
    const token   = localStorage.getItem('token');
    const prevCls = window.adminGetPrevCls(orderId);

    window.applyAdminStatusUI(orderId, newCls);
    window.updateAdminStats();

    try {
        const res = await fetch(`${window.API}/api/admin/orders/${orderId}/status`, {
            method: 'PATCH',
            headers: {
                'Authorization': `Bearer ${token}`,
                'Content-Type':  'application/json'
            },
            body: JSON.stringify({ status: ADMIN_STATUS_BY_CLS[newCls] })
        });

        if (!res.ok) {
            const err = await res.json().catch(() => ({}));
            alert('Помилка: ' + (err.error || res.status));
            window.applyAdminStatusUI(orderId, prevCls);
            window.updateAdminStats();
        }

    } catch (e) {
        console.error('Помилка зміни статусу:', e);
        alert("Не вдалося з'єднатися з сервером");
        window.applyAdminStatusUI(orderId, prevCls);
        window.updateAdminStats();
    }
};

window.adminGetPrevCls = function (orderId) {
    const pill = document.getElementById('apill-' + orderId);
    if (!pill) return 'pending';
    const block = pill.closest('.AdminOrderBlock');
    return block ? block.dataset.status : 'pending';
};

window.applyAdminStatusUI = function (orderId, newCls) {
    const s = Object.values(ADMIN_STATUS_MAP).find(x => x.cls === newCls);
    if (!s) return;

    const pill  = document.getElementById('apill-'  + orderId);
    const qact  = document.getElementById('aqact-'  + orderId);
    const aact  = document.getElementById('aact-'   + orderId);
    const block = pill ? pill.closest('.AdminOrderBlock') : null;

    if (block) block.dataset.status = newCls;
    if (pill)  { pill.className = `AdminStatusPill ${newCls}`; pill.textContent = s.label; }
    if (qact)  qact.innerHTML = window.adminQuickActionsHTML(orderId, newCls);
    if (aact)  aact.innerHTML = window.adminDetailActionsHTML(orderId, newCls);
};

window.adminQuickActionsHTML = function (id, cls) {
    if (cls === 'pending') return `
        <button class="AdminQuickBtn approve" onclick="adminStatus(${id},'approved')">Підтвердити</button>
        <button class="AdminQuickBtn reject"  onclick="adminStatus(${id},'rejected')">Відхилити</button>
    `;
    if (cls === 'approved') return `<span class="AdminDoneLabel">Підтверджено</span>`;
    if (cls === 'rejected') return `<button class="AdminQuickBtn restore" onclick="adminStatus(${id},'pending')">Відновити</button>`;
    return '';
};

window.adminDetailActionsHTML = function (id, cls) {
    if (cls === 'pending') return `
        <button class="AdminActionBtn" onclick="adminStatus(${id},'approved')">Підтвердити замовлення</button>
        <button class="AdminActionBtn outline" onclick="adminStatus(${id},'rejected')">Відхилити</button>
    `;
    if (cls === 'approved') return `
        <button class="AdminActionBtn outline" onclick="adminStatus(${id},'rejected')">Скасувати підтвердження</button>
    `;
    if (cls === 'rejected') return `
        <button class="AdminActionBtn" onclick="adminStatus(${id},'pending')">Відновити замовлення</button>
    `;
    return '';
};

window.updateAdminStats = function () {
    let p = 0, a = 0, r = 0, revenue = 0;
    document.querySelectorAll('.AdminOrderBlock').forEach(b => {
        const st  = b.dataset.status;
        const amt = parseFloat(b.querySelector('.AdminOrderAmt').textContent.replace('$', '').replace(/\s/g, '')) || 0;
        if (st === 'pending')  p++;
        if (st === 'approved') { a++; revenue += amt; }
        if (st === 'rejected') r++;
    });
    const elP = document.getElementById('statPending');
    const elA = document.getElementById('statApproved');
    const elR = document.getElementById('statRejected');
    const elV = document.getElementById('statRevenue');
    if (elP) elP.textContent = p;
    if (elA) elA.textContent = a;
    if (elR) elR.textContent = r;
    if (elV) elV.textContent = '$' + revenue.toFixed(2);
};

window.filterAdmin = function (status, btn) {
    document.querySelectorAll('.AdminTab').forEach(t => t.classList.remove('active'));
    btn.classList.add('active');
    document.querySelectorAll('.AdminOrderBlock').forEach(b => {
        b.style.display = (status === 'all' || b.dataset.status === status) ? 'block' : 'none';
    });
};

window.searchAdmin = function (val) {
    const q = val.toLowerCase().trim();
    document.querySelectorAll('.AdminOrderBlock').forEach(b => {
        b.style.display = (!q || b.dataset.search.includes(q)) ? 'block' : 'none';
    });
};

window.adminFormatDate = function (isoStr) {
    try {
        const d = new Date(isoStr);
        return d.toLocaleDateString('uk-UA', { day: '2-digit', month: '2-digit', year: 'numeric' });
    } catch { return '—'; }
};

window.adminEsc = function (str) {
    if (!str) return '';
    return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;');
};