(function() {
  function fileExists(url) {
    // file:// 场景下 fetch 会被 CORS 限制，直接认为存在，交给浏览器下载失败再说
    if (location.protocol === 'file:') return Promise.resolve(true);
    return fetch(url, { method: 'HEAD' }).then(function(res) {
      return res.ok;
    }).catch(function() { return false; });
  }

  function getStaticBase() {
    // 通过当前脚本（download_pdf.js）已解析后的 src 反推出版本根路径，再拼接 _static/
    try {
      var scripts = document.getElementsByTagName('script');
      for (var i = 0; i < scripts.length; i++) {
        var src = scripts[i].getAttribute('src') || '';
        if (src.indexOf('download_pdf.js') !== -1) {
          var abs = new URL(src, window.location.href).href;
          // 去掉末尾 '_static/download_pdf.js' 或隔离语言资源目录
          //（如 '_static_en/download_pdf.js'），PDF 始终从共享 _static 读取。
          var base = abs.replace(/_static(?:_[a-z]{2})?\/download_pdf\.js.*$/i, '');
          return base + '_static/';
        }
      }
    } catch (e) {}
    // 兜底：相对当前页面（可能在子目录下，路径可能不正确）
    return '_static/';
  }

  function loadProjectInfo(staticBase) {
    // file:// 场景下尝试读取 window.projectInfo（由 project_info.js 提供）
    if (location.protocol === 'file:') {
      if (window.projectInfo) return Promise.resolve(window.projectInfo);
      // 注入一个 <script> 尝试加载 project_info.js
      return new Promise(function(resolve){
        var s = document.createElement('script');
        s.src = staticBase + 'project_info.js';
        s.onload = function(){ resolve(window.projectInfo || {}); };
        s.onerror = function(){ resolve({}); };
        document.head.appendChild(s);
      });
    }
    return fetch(staticBase + 'project_info.json', { cache: 'no-store' })
      .then(function(r){ return r.ok ? r.json() : {}; })
      .catch(function(){ return {}; });
  }

  function insertDownloadButton() {
    var container = document.querySelector('.wy-side-nav-search');
    if (!container) container = document.querySelector('.wy-nav-top');
    if (!container) return;

    var staticBase = getStaticBase();

    // 先渲染占位，避免闪烁
    var wrapper = document.createElement('div');
    wrapper.className = 'sdk-download-pdf-wrapper';
    wrapper.style.visibility = 'hidden';
    container.appendChild(wrapper);

    loadProjectInfo(staticBase).then(function(info){
      var displayName = (info && info.projectName) ? info.projectName : 'SDK 文档';
      // 检测当前语言
      var currentLanguage = detectCurrentLanguage();
      console.log('PDF下载UI检测到当前语言:', currentLanguage);
      var configuredFiles = (info && info.pdfFiles) ? info.pdfFiles : {};
      var fallbackBase = displayName.replace(/\s+/g, '_');
      var pdfName = configuredFiles[currentLanguage] || (
        currentLanguage === 'zh' ? fallbackBase + '.pdf' : fallbackBase + '_EN.pdf'
      );

      // 根据当前语言创建对应的按钮
      var button = null;
      if (currentLanguage === 'zh') {
        // 中文按钮
        button = document.createElement('a');
        button.href = staticBase + pdfName;
        button.setAttribute('download', pdfName);
        button.setAttribute('aria-label', '下载 ' + displayName + '下载 PDF');
        button.className = 'sdk-download-pdf-btn sdk-download-pdf-btn--zh';
        button.innerHTML = '\n        <span class="sdk-pdf-icon" aria-hidden="true">\u2B07\uFE0F</span>\n        <span class="sdk-pdf-text">下载 PDF</span>\n      ';
      } else {
        // 英文按钮
        button = document.createElement('a');
        button.href = staticBase + pdfName;
        button.setAttribute('download', pdfName);
        button.setAttribute('aria-label', 'Download ' + (displayName || 'SDK Docs') + 'Download PDF');
        button.className = 'sdk-download-pdf-btn sdk-download-pdf-btn--en';
        button.innerHTML = '\n        <span class="sdk-pdf-icon" aria-hidden="true">\u2B07\uFE0F</span>\n        <span class="sdk-pdf-text">Download PDF</span>\n      ';
      }

      // 将按钮添加到容器
      wrapper.appendChild(button);

      // 检查PDF文件是否存在
      fileExists(button.href).then(function(exists){
        wrapper.style.visibility = exists ? 'visible' : 'hidden';
        if (!exists) wrapper.remove();
      });
    });
  }

  /**
   * 检测当前页面的语言
   */
  function detectCurrentLanguage() {
    if (window.DOCS_LANGUAGE && window.DOCS_LANGUAGE.current) {
      return window.DOCS_LANGUAGE.current;
    }
    const htmlLang = document.documentElement.getAttribute('lang');
    const currentPath = window.location.pathname;
    
    // 根据HTML lang属性和URL路径判断语言
    if (htmlLang === 'zh-CN' || currentPath.includes('_zh.html')) {
      return 'zh';
    } else if (htmlLang === 'en' || currentPath.endsWith('.html')) {
      return 'en';
    }
    
    // 默认返回英文
    return 'en';
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', insertDownloadButton);
  } else {
    insertDownloadButton();
  }
})();

