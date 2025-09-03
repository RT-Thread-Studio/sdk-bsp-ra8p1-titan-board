(function() {
  function fileExists(url) {
    // file:// 场景下 fetch 会被 CORS 限制，直接认为存在，交给浏览器下载失败再说
    if (location.protocol === 'file:') return Promise.resolve(true);
    return fetch(url, { method: 'HEAD' }).then(function(res) {
      return res.ok;
    }).catch(function() { return false; });
  }

  function loadProjectInfo() {
    if (location.protocol === 'file:') return Promise.resolve({});
    return fetch('_static/project_info.json', { cache: 'no-store' })
      .then(function(r){ return r.ok ? r.json() : {}; })
      .catch(function(){ return {}; });
  }

  function insertDownloadButton() {
    var container = document.querySelector('.wy-side-nav-search');
    if (!container) container = document.querySelector('.wy-nav-top');
    if (!container) return;

    // 先渲染占位，避免闪烁
    var wrapper = document.createElement('div');
    wrapper.className = 'sdk-download-pdf-wrapper';
    wrapper.style.visibility = 'hidden';
    container.appendChild(wrapper);

    loadProjectInfo().then(function(info){
      var pdfName = (info && info.pdfFileName) ? info.pdfFileName : 'sdk-docs.pdf';
      var displayName = (info && info.projectName) ? info.projectName : 'SDK 文档';

      var btn = document.createElement('a');
      btn.href = '_static/' + pdfName;
      btn.setAttribute('download', displayName + '.pdf');
      btn.setAttribute('aria-label', '下载 ' + displayName + ' PDF');
      btn.className = 'sdk-download-pdf-btn';
      btn.innerHTML = '\n        <span class="sdk-pdf-icon" aria-hidden="true">\u2B07\uFE0F</span>\n        <span class="sdk-pdf-text">下载 PDF</span>\n      ';

      wrapper.appendChild(btn);

      // 若 PDF 不存在则隐藏；存在则显示
      fileExists(btn.href).then(function(exists) {
        wrapper.style.visibility = exists ? 'visible' : 'hidden';
        if (!exists) wrapper.remove();
      });
    });
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', insertDownloadButton);
  } else {
    insertDownloadButton();
  }
})();

