/** Build-time configured Chinese/English document switcher. */
(function() {
    'use strict';

    function getLanguageConfig() {
        return window.DOCS_LANGUAGE || { enabled: false };
    }

    function switchLanguage(targetLanguage) {
        const config = getLanguageConfig();
        if (targetLanguage === config.current || !config.targetUrl) return;
        const switcher = document.getElementById('docs-language-switch');
        if (switcher) switcher.classList.add('loading');
        window.location.href = config.targetUrl;
    }

    function createLanguageSwitch() {
        const config = getLanguageConfig();
        if (!config.enabled || !Array.isArray(config.available) || config.available.length !== 2) {
            return;
        }
        if (document.getElementById('docs-language-switch')) return;
        const sidebar = document.querySelector('.wy-nav-side');
        if (!sidebar) return;

        sidebar.classList.add('has-docs-language-switch');
        sidebar.insertAdjacentHTML('beforeend', `
            <div class="docs-language-switch-container">
                <div class="docs-language-switch" id="docs-language-switch">
                    <div class="docs-language-switch__container">
                        <button class="docs-language-switch__option" data-lang="zh" aria-label="切换到中文">中文</button>
                        <span class="docs-language-switch__separator">|</span>
                        <button class="docs-language-switch__option" data-lang="en" aria-label="Switch to English">English</button>
                    </div>
                </div>
            </div>
        `);

        document.querySelectorAll('.docs-language-switch__option').forEach((option) => {
            const language = option.getAttribute('data-lang');
            const isCurrent = language === config.current;
            option.classList.toggle('active', isCurrent);
            option.setAttribute('aria-pressed', String(isCurrent));
            option.addEventListener('click', () => switchLanguage(language));
        });
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', createLanguageSwitch);
    } else {
        createLanguageSwitch();
    }

    window.LanguageSwitch = {
        switchLanguage,
        getCurrentLanguage: () => getLanguageConfig().current
    };
})();
