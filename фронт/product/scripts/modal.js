// product/scripts/modal.js

window.API = window.API || "http://127.0.0.1:18080";

async function initProductPage() {
    const params = new URLSearchParams(window.location.search);
    const id = params.get('id');
    
    if (!id) {
        document.getElementById('productText').textContent = "Товар не знайдено";
        return;
    }

    const buyBtn = document.getElementById('buyBtn');
    if (buyBtn) {
        buyBtn.addEventListener('click', (e) => {
            e.preventDefault();
            if (typeof window.addToCartServer === 'function') {
                window.addToCartServer(id);
            } else {
                console.error("Глобальний скрипт оверлеїв не підключено!");
            }
        });
    }

    // Перевіряємо чи товар вже є в кошику — якщо так, змінюємо кнопку
    async function checkIfInCart() {
        const token = localStorage.getItem('token');
        if (!token || !buyBtn) return;
        try {
            const res = await fetch(`${window.API}/api/cart`, {
                headers: { 'Authorization': `Bearer ${token}` }
            });
            if (!res.ok) return;
            const data = await res.json();
            const items = data.items || [];
            const inCart = items.some(item => Number(item.productId) === Number(id));
            if (inCart) {
                const textSpan = buyBtn.querySelector('.text');
                if (textSpan) textSpan.textContent = 'В кошику';
                buyBtn.style.opacity = '0.5';
                buyBtn.style.pointerEvents = 'none';
                buyBtn.title = 'Цей товар вже є у вашому кошику';
            }
        } catch (e) { /* тихо ігноруємо */ }
    }
    checkIfInCart();

    try {
        const res = await fetch(`${window.API}/api/products/${id}`);
        if (!res.ok) throw new Error("Товар не знайдено");
        const product = await res.json();

        // Фото
        const imageDiv = document.getElementById('productImage');
        if (imageDiv) {
            const img = document.createElement('img');
            img.src = `${window.API}/api/products/image/${product.id}`;
            img.style.width = '100%';
            img.style.height = '100%';
            img.style.objectFit = 'cover';
            imageDiv.appendChild(img);
        }

        // Мініатюра фото зліва
        const thumbPhoto = document.getElementById('thumb_photo');
        if (thumbPhoto) {
            thumbPhoto.style.backgroundImage = `url('${window.API}/api/products/image/${product.id}')`;
            thumbPhoto.style.backgroundSize = 'cover';
            thumbPhoto.style.backgroundPosition = 'center';
            thumbPhoto.onclick = () => switchTab('photo');
        }

        // 3D Модель — перевіряємо чи є modelPath у відповіді API
        const modelViewer = document.getElementById('productModelViewer');
        const modelViewerWrap = document.getElementById('modelViewerWrap');
        const viewTabs = document.getElementById('viewTabs');
        const thumb3d = document.getElementById('thumb_3d');

        if (product.modelPath && product.modelPath.trim() !== '') {
            // Є 3D модель — показуємо таби і мініатюру
            const modelSrc = `${window.API}/api/products/model/${product.id}`;
            if (modelViewer) modelViewer.setAttribute('src', modelSrc);

            if (viewTabs) viewTabs.classList.add('has-both');
            if (thumb3d) {
                thumb3d.style.display = 'block';
                thumb3d.onclick = () => switchTab('3d');
            }
        } else {
            // Немає 3D — ховаємо таби і viewer повністю
            if (viewTabs) viewTabs.classList.remove('has-both');
            if (modelViewerWrap) modelViewerWrap.style.display = 'none';
            if (thumb3d) thumb3d.style.display = 'none';
        }

        // Текстові дані
        const titleEl = document.getElementById('productText');
        if (titleEl) titleEl.textContent = `${product.modelName} — $${Number(product.price).toFixed(2)}`;

        const descEl = document.getElementById('productDesc');
        if (descEl) {
            descEl.innerHTML = `
                <strong>Опис:</strong> ${product.shortDescription}<br><br>
                <strong>Технології:</strong> ${product.technology}<br><br>
                <strong>Догляд:</strong> ${product.careInstructions}
            `;
        }

        // Статус
        const statusBadge = document.getElementById('productStatus');
        if (statusBadge) {
            if (product.status === 1) {
                statusBadge.textContent = "В наявності";
                statusBadge.className = "P_status in-stock";
            } else {
                statusBadge.textContent = "Немає в наявності";
                statusBadge.className = "P_status out-of-stock";
                if (buyBtn) buyBtn.style.display = 'none';
            }
        }

        loadRecommendations();

    } catch (err) {
        console.error('Помилка ініціалізації сторінки:', err);
        document.getElementById('productText').textContent = "Помилка завантаження";
    }
}

async function loadRecommendations() {
    try {
        const res = await fetch(`${window.API}/api/products`);
        if (!res.ok) return;
        const data = await res.json();
        
        const params = new URLSearchParams(window.location.search);
        const currentId = params.get('id');

        const wrap = document.getElementById('recommendations');
        if (!wrap) return;

        const others = (data.products || [])
            .filter(p => Number(p.id) !== Number(currentId))
            .slice(0, 3);

        wrap.innerHTML = '';
        others.forEach(p => {
            const link = document.createElement('a');
            link.className = 'prop';
            link.href = `product1.html?id=${p.id}`;
            link.style.display = 'block';
            link.style.textDecoration = 'none';
            link.style.color = 'inherit';

            const imgDiv = document.createElement('div');
            imgDiv.style.width = '100%';
            imgDiv.style.height = '200px';
            imgDiv.style.backgroundImage = `url('${window.API}/api/products/image/${p.id}')`;
            imgDiv.style.backgroundSize = 'cover';
            imgDiv.style.backgroundPosition = 'center';
            imgDiv.style.marginBottom = '10px';

            const txt = document.createElement('div');
            txt.innerHTML = `<strong>${p.modelName}</strong><br>$${Number(p.price).toFixed(2)}`;

            link.appendChild(imgDiv);
            link.appendChild(txt);
            wrap.appendChild(link);
        });
    } catch (e) {
        console.error("Помилка рекомендацій:", e);
    }
}

// Функція для перемикання табів фото/3D (якщо вони є в HTML)
window.switchTab = function(tabName) {
    const photoTab = document.getElementById('tabPhoto');
    const modelTab = document.getElementById('tab3d');
    const photoView = document.getElementById('productImage');
    const modelView = document.getElementById('modelViewerWrap');

    if (tabName === 'photo') {
        photoTab.classList.add('active');
        modelTab.classList.remove('active');
        photoView.classList.remove('hidden-tab');
        modelView.classList.add('hidden-tab');
    } else {
        modelTab.classList.add('active');
        photoTab.classList.remove('active');
        modelView.classList.remove('hidden-tab');
        photoView.classList.add('hidden-tab');
    }
};

document.addEventListener('DOMContentLoaded', initProductPage);