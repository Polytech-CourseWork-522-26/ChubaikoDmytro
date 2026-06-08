// main/scripts/modal-main.js

// ── Ticker ──
(function () {
    var text  = 'Exclusive product Amber Light';
    var track = document.getElementById('tickerTrack');
    if (!track) return;
    var html = '';
    for (var i = 0; i < 16; i++) {
        html += '<span class="TickerItem">' + text + '</span>'
              + '<span class="TickerItem TickerDot"> • </span>';
    }
    track.innerHTML = html + html; // дублюємо для безшовного loop
})();

// ── Smooth scroll ──
document.querySelectorAll('a[href^="#"]').forEach(function (a) {
    a.addEventListener('click', function (e) {
        var target = document.querySelector(this.getAttribute('href'));
        if (target) {
            e.preventDefault();
            target.scrollIntoView({ behavior: 'smooth', block: 'start' });
        }
    });
});

// ── Активний пункт меню при скролі ──
var navMap = {
    home:     document.querySelector('.HomeButton'),
    about:    document.querySelector('.AboutButton'),
    catalog:  document.querySelector('.CatalogButton'),
    contacts: document.querySelector('.ContactsButton')
};

function clearActive() {
    Object.values(navMap).forEach(function (l) {
        if (!l) return;
        l.style.borderBottom  = 'none';
        l.style.paddingBottom = '0';
    });
}

window.addEventListener('scroll', function () {
    var y = window.scrollY + 120;
    ['home', 'about', 'catalog', 'contacts'].forEach(function (id) {
        var el = document.getElementById(id);
        if (!el || !navMap[id]) return;
        if (el.offsetTop <= y && el.offsetTop + el.offsetHeight > y) {
            clearActive();
            navMap[id].style.borderBottom  = '2px solid #000';
            navMap[id].style.paddingBottom = '2px';
        }
    });
});


// ══════════════════════════════════════════
// РЕКОМЕНДАЦІЇ ТИЖНЯ — динамічне завантаження
// ══════════════════════════════════════════

document.addEventListener('DOMContentLoaded', function () {
    loadCatalogPreview();
});

async function loadCatalogPreview() {
    const API   = window.API || 'http://127.0.0.1:18080';
    const grid  = document.getElementById('catalogPreviewGrid');
    if (!grid) return;

    try {
        const res = await fetch(`${API}/api/products`);
        if (!res.ok) throw new Error('HTTP ' + res.status);

        const data     = await res.json();
        const products = (data.products || [])
            .filter(p => p.status === 1)   // тільки товари в наявності
            .slice(0, 5);                  // максимум 5 карток

        if (products.length === 0) {
            grid.innerHTML = '<p style="color:#fff;font-family:\'Roboto Mono\',monospace;font-size:13px;">Товари відсутні.</p>';
            return;
        }

        grid.innerHTML = '';

        products.forEach(function (p, index) {
            const card = document.createElement('a');
            card.className = 'CatalogPreviewCard';
            card.href = `../product/product1.html?id=${p.id}`;

            // Визначаємо шлях до зображення
            const imgSrc = `${API}/api/products/image/${p.id}`;

            // Форматуємо ціну: 1670 → "$1 670"
            const price = '$' + Number(p.price).toLocaleString('uk-UA');

            card.innerHTML = `
                <img
                    class="CatalogPreviewImg"
                    src="${imgSrc}"
                    alt="${escHtml(p.modelName)}"
                    onerror="this.style.background='#222';this.removeAttribute('src')"
                >
                <div class="CatalogPreviewInfo">
                    <div class="CatalogPreviewName">${escHtml(p.modelName)}</div>
                    <div class="CatalogPreviewPrice">${price}</div>
                </div>
            `;

            grid.appendChild(card);
        });

    } catch (err) {
        console.error('Помилка завантаження рекомендацій:', err);
        // При помилці — залишаємо статичні картки (не очищаємо grid)
    }
}

function escHtml(str) {
    if (!str) return '';
    return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;');
}