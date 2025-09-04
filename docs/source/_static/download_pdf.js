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
          // 去掉末尾 '_static/download_pdf.js'
          var base = abs.replace(/_static\/download_pdf\.js.*$/, '');
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
      // 以 config.yaml 的 project.name 为基准名；若后端未写入，则从已生成的中文名回退
      var baseName = displayName;
      // 后端也会写入 pdfFileName（中文文件名），若存在则以其去掉扩展名作为备选
      if (info && info.pdfFileName && /\.pdf$/i.test(info.pdfFileName)) {
        baseName = info.pdfFileName.replace(/\.pdf$/i, '');
      }
      var zhPdfName = baseName + '.pdf';
      var enPdfName = baseName.replace(/\s+/g, '_') + '_EN.pdf';

      // 中文按钮
      var btnZh = document.createElement('a');
      btnZh.href = staticBase + zhPdfName;
      btnZh.setAttribute('download', displayName + '.pdf');
      btnZh.setAttribute('aria-label', '下载 ' + displayName + ' 中文 PDF');
      btnZh.className = 'sdk-download-pdf-btn sdk-download-pdf-btn--zh';
      btnZh.innerHTML = '\n        <span class="sdk-pdf-icon" aria-hidden="true">\u2B07\uFE0F</span>\n        <span class="sdk-pdf-text">中文 PDF</span>\n      ';

      // 英文按钮
      var btnEn = document.createElement('a');
      btnEn.href = staticBase + enPdfName;
      btnEn.setAttribute('download', (displayName || 'SDK Docs') + '_EN.pdf');
      btnEn.setAttribute('aria-label', 'Download ' + (displayName || 'SDK Docs') + ' English PDF');
      btnEn.className = 'sdk-download-pdf-btn sdk-download-pdf-btn--en';
      btnEn.innerHTML = '\n        <span class="sdk-pdf-icon" aria-hidden="true">\u2B07\uFE0F</span>\n        <span class="sdk-pdf-text">English PDF</span>\n      ';

      // 将按钮添加到容器
      wrapper.appendChild(btnZh);
      wrapper.appendChild(btnEn);

      // 仅当至少一个PDF存在时显示容器
      Promise.all([fileExists(btnZh.href), fileExists(btnEn.href)]).then(function(results){
        var anyExists = results.some(function(x){ return !!x; });
        wrapper.style.visibility = anyExists ? 'visible' : 'hidden';
        if (!anyExists) wrapper.remove();
        // 若单个不存在，隐藏对应按钮
        if (!results[0]) btnZh.style.display = 'none';
        if (!results[1]) btnEn.style.display = 'none';
      });
    });
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', insertDownloadButton);
  } else {
    insertDownloadButton();
  }
})();

