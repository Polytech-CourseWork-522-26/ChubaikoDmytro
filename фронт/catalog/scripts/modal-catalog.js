// ==========================================
// СПІЛЬНІ УТИЛІТИ (доступні на всіх сторінках)
// ==========================================
function getTokenData() {
    const token = localStorage.getItem('token');
    if (!token) return { userId: null, role: null, valid: false };
    const parts = token.split(':');
    if (parts.length !== 2) return { userId: null, role: null, valid: false };
    const userId = parseInt(parts[0]);
    const role = parseInt(parts[1]);
    if (isNaN(userId) || isNaN(role)) return { userId: null, role: null, valid: false };
    return { userId, role, valid: true };
}

function isAdmin() {
    const td = getTokenData();
    return td.valid && td.role === 1;
}

function getAuthHeaders() {
    const token = localStorage.getItem('token');
    return token ? { 'Authorization': 'Bearer ' + token } : {};
}

// ==========================================
// ВИЗНАЧЕННЯ ПОТОЧНОЇ СТОРІНКИ
// ==========================================
document.addEventListener('DOMContentLoaded', () => {
    const page = window.location.pathname;

    if (page.includes('add-product')) {
        initAddProductPage();
    } else if (page.includes('catalog')) {
        initCatalogPage();
    }
});

// ============================================================
//  СТОРІНКА КАТАЛОГУ
// ============================================================
function initCatalogPage() {
    const catalogGrid = document.getElementById('catalog-grid');
    const filterApplyBtn = document.getElementById('filter-apply');
    const filterCountEl = document.getElementById('filterCount');
    const addProductBtn = document.getElementById('addProductBtn');

    // Показуємо кнопку "Додати товар" лише адміну
    if (addProductBtn && isAdmin()) {
        addProductBtn.removeAttribute('hidden');
    }

    // ==========================================
    // ЗАВАНТАЖЕННЯ ТОВАРІВ
    // ==========================================
    async function loadProducts(filters = {}) {
        try {
            const url = new URL(window.API + '/api/products');
            if (filters.name) url.searchParams.append('name', filters.name);
            if (filters.minP) url.searchParams.append('minP', filters.minP);
            if (filters.maxP) url.searchParams.append('maxP', filters.maxP);
            if (filters.ascending) url.searchParams.append('ascending', filters.ascending);

            const response = await fetch(url, { headers: getAuthHeaders() });
            if (!response.ok) throw new Error('Помилка завантаження товарів');

            const data = await response.json();
            const products = data.products || [];

            if (filterCountEl) filterCountEl.textContent = `${products.length} товарів`;

            renderCatalog(products);
        } catch (error) {
            console.error('Не вдалося завантажити каталог:', error);
        }
    }

    // ==========================================
    // ВІДМАЛЬОВУВАННЯ КАРТОК
    // ==========================================
    function renderCatalog(products) {
        if (!catalogGrid) return;

        const addBtn = catalogGrid.querySelector('#addProductBtn');
        catalogGrid.innerHTML = '';
        if (addBtn) {
            catalogGrid.appendChild(addBtn);
            if (!isAdmin()) {
                addBtn.setAttribute('hidden', '');
            }
        }

        if (products.length === 0) {
            const empty = document.createElement('p');
            empty.className = 'no-products';
            empty.textContent = 'Товарів не знайдено.';
            catalogGrid.appendChild(empty);
            return;
        }

        const admin = isAdmin();

        products.forEach(product => {
            const card = document.createElement('div');
            card.className = 'CatalogCard';

            card.addEventListener('click', () => {
                if (admin) {
                    window.location.href = `add-product.html?id=${product.id}`;
                } else {
                    window.location.href = `../product/product1.html?id=${product.id}`;
                }
            });

            const imageDiv = document.createElement('div');
            imageDiv.className = 'CardImage';
            imageDiv.style.backgroundImage = product.imagePath
                ? `url(${window.API}/api/products/image/${product.id})`
                : 'linear-gradient(135deg, #f5f5f5 25%, #ececec 50%, #f5f5f5 75%)';
            imageDiv.style.backgroundSize = 'cover';
            imageDiv.style.backgroundPosition = 'center';

            const infoDiv = document.createElement('div');
            infoDiv.className = 'CardInfo';
            infoDiv.innerHTML = `
                <span class="CardName">${product.modelName}</span>
                <span class="CardPrice">$${Number(product.price).toFixed(2)}</span>
            `;

            if (admin) {
                const editBadge = document.createElement('div');
                editBadge.className = 'CardEditBadge';
                editBadge.title = 'Редагувати товар';
                editBadge.textContent = '✎';
                card.appendChild(editBadge);

                if (product.status !== 1) {
                    card.style.opacity = '0.5';
                    const badge = document.createElement('div');
                    badge.className = 'CardOutOfStockBadge';
                    badge.textContent = 'НЕМАЄ В НАЯВНОСТІ';
                    card.appendChild(badge);
                }
            }

            card.appendChild(imageDiv);
            card.appendChild(infoDiv);
            catalogGrid.appendChild(card);
        });
    }

    // ==========================================
    // ФІЛЬТРАЦІЯ
    // ==========================================
    if (filterApplyBtn) {
        filterApplyBtn.addEventListener('click', () => {
            loadProducts({
                name: document.getElementById('filter-name')?.value.trim() || '',
                minP: document.getElementById('filter-min')?.value || '',
                maxP: document.getElementById('filter-max')?.value || '',
                ascending: document.getElementById('filter-sort')?.value ?? 'true'
            });
        });
    }

    loadProducts();
}

// ============================================================
//  СТОРІНКА ДОДАВАННЯ / РЕДАГУВАННЯ ТОВАРУ
// ============================================================
function initAddProductPage() {

    // Захист: лише адмін
    if (!isAdmin()) {
        window.location.href = 'catalog.html';
        return;
    }

    const params = new URLSearchParams(window.location.search);
    const productId = params.get('id');

    const form = document.getElementById('addProductForm');
    const pageTitle = document.querySelector('.PageTitle');
    const pageSubtitle = document.querySelector('.PageSubtitle');
    const saveBtn = document.querySelector('.BtnSave .text');
    const deleteBtn = document.getElementById('btnDeleteProduct');
    const breadcrumb = document.querySelector('.MapElement3');

    // ── РЕЖИМ ДОДАВАННЯ (немає ?id) ──────────────────────────
    if (!productId) {
        if (deleteBtn) deleteBtn.style.display = 'none';
        setupPhotoPreview();
        setupModelPreview();
        setupFormAdd();
        return;
    }

    // ── РЕЖИМ РЕДАГУВАННЯ (є ?id) ────────────────────────────
    if (pageTitle) pageTitle.textContent = 'Редагувати товар';
    if (pageSubtitle) pageSubtitle.textContent = 'Змініть потрібні поля — решта залишиться без змін';
    if (saveBtn) saveBtn.textContent = 'Зберегти зміни';
    if (breadcrumb) breadcrumb.textContent = 'Редагувати товар';
    if (deleteBtn) deleteBtn.style.display = 'inline-flex';

    // Підтягуємо дані товару і заповнюємо форму
    loadProductForEdit(productId);

    setupPhotoPreview();
    setupModelPreview();
    setupDeleteModal(productId);
    setupFormSubmit(productId);
}

// ==========================================
// ЗАВАНТАЖЕННЯ ТОВАРУ ДЛЯ РЕДАГУВАННЯ
// ==========================================
async function loadProductForEdit(productId) {
    try {
        const res = await fetch(`${window.API}/api/products/${productId}`);
        if (!res.ok) throw new Error('Товар не знайдено');
        const p = await res.json();

        const set = (id, val) => { const el = document.getElementById(id); if (el) el.value = val || ''; };
        set('field-name', p.modelName);
        set('field-price', p.price);
        set('field-desc', p.shortDescription);
        set('field-tech', p.technology);
        set('field-care', p.careInstructions);

        // Статус
        const statusVal = String(p.status ?? 1);
        document.querySelectorAll('input[name="status"]').forEach(radio => {
            radio.checked = radio.value === statusVal;
        });

        // Поточне фото як прев'ю
        if (p.imagePath) {
            const preview = document.getElementById('photoPreview');
            const label = document.querySelector('.PhotoLabel');
            if (preview) { preview.src = `${window.API}/api/products/image/${productId}`; preview.style.display = 'block'; }
            if (label) label.style.display = 'none';
        }

        // Назва поточної 3D-моделі
        if (p.modelPath) {
            const fileNameBlock = document.getElementById('modelFileName');
            const fileNameText = document.getElementById('modelFileNameText');
            const modelLabel = document.querySelector('.ModelLabel');
            if (fileNameBlock) fileNameBlock.style.display = 'flex';
            if (fileNameText) fileNameText.textContent = p.modelPath.split('/').pop();
            if (modelLabel) modelLabel.style.display = 'none';
        }

    } catch (err) {
        console.error('Помилка завантаження товару:', err);
        alert('Не вдалося завантажити дані товару. Повернення до каталогу.');
        window.location.href = 'catalog.html';
    }
}

// ==========================================
// ПРЕВ'Ю ФОТО
// ==========================================
function setupPhotoPreview() {
    const input = document.getElementById('photoInput');
    const preview = document.getElementById('photoPreview');
    const label = document.querySelector('.PhotoLabel');
    if (!input || !preview) return;

    input.addEventListener('change', () => {
        const file = input.files[0];
        if (!file) return;
        const reader = new FileReader();
        reader.onload = e => {
            preview.src = e.target.result;
            preview.style.display = 'block';
            if (label) label.style.display = 'none';
        };
        reader.readAsDataURL(file);
    });
}

// ==========================================
// НАЗВА 3D МОДЕЛІ
// ==========================================
function setupModelPreview() {
    const input = document.getElementById('modelInput');
    const fileNameBlock = document.getElementById('modelFileName');
    const fileNameText = document.getElementById('modelFileNameText');
    const modelLabel = document.querySelector('.ModelLabel');
    if (!input) return;

    input.addEventListener('change', () => {
        const file = input.files[0];
        if (!file) return;
        if (fileNameBlock) fileNameBlock.style.display = 'flex';
        if (fileNameText) fileNameText.textContent = file.name;
        if (modelLabel) modelLabel.style.display = 'none';
    });
}

// ==========================================
// МОДАЛЬНЕ ВІКНО ВИДАЛЕННЯ
// ==========================================
function setupDeleteModal(productId) {
    const deleteBtn = document.getElementById('btnDeleteProduct');
    const modal = document.getElementById('deleteConfirmModal');
    const cancelBtn = document.getElementById('btnCancelDelete');
    const confirmBtn = document.getElementById('btnConfirmDelete');
    if (!deleteBtn || !modal) return;

    deleteBtn.addEventListener('click', () => { modal.style.display = 'flex'; });
    cancelBtn?.addEventListener('click', () => { modal.style.display = 'none'; });

    confirmBtn?.addEventListener('click', async () => {
        try {
            const res = await fetch(`${window.API}/api/products/${productId}`, {
                method: 'DELETE',
                headers: getAuthHeaders()
            });
            if (res.ok) {
                window.location.href = 'catalog.html';
            } else {
                const err = await res.json();
                alert('Помилка: ' + (err.error || 'Не вдалося видалити товар'));
                modal.style.display = 'none';
            }
        } catch {
            alert('Помилка мережі при видаленні');
            modal.style.display = 'none';
        }
    });
}

// ==========================================
// САБМІТ ФОРМИ РЕДАГУВАННЯ З ТОКЕНОМ
// ==========================================
function setupFormSubmit(productId) {
    const form = document.getElementById('addProductForm');
    if (!form) return;

    form.addEventListener('submit', async (e) => {
        e.preventDefault();
        try {
            const res = await fetch(`${window.API}/api/products/update/${productId}`, {
                method: 'POST',
                headers: getAuthHeaders(),
                body: new FormData(form)
            });
            if (res.ok) {
                window.location.href = 'catalog.html';
            } else {
                const err = await res.json();
                alert('Помилка: ' + (err.error || 'Не вдалося зберегти зміни'));
            }
        } catch {
            alert('Помилка мережі при збереженні');
        }
    });
}
// ==========================================
// САБМІТ ФОРМИ ДОДАВАННЯ З ТОКЕНОМ
// ==========================================
function setupFormAdd() {
    const form = document.getElementById('addProductForm');
    if (!form) return;

    form.addEventListener('submit', async (e) => {
        e.preventDefault();
        try {
            const res = await fetch(`${window.API}/api/products/add`, {
                method: 'POST',
                headers: getAuthHeaders(),
                body: new FormData(form)
            });
            if (res.ok) {
                window.location.href = 'catalog.html';
            } else {
                const err = await res.json();
                alert('Помилка: ' + (err.error || 'Не вдалося додати товар'));
            }
        } catch {
            alert('Помилка мережі при додаванні товару');
        }
    });
}