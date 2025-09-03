(function() {
  function insertDownloadLink() {
    try {
      var container = document.querySelector('.wy-side-nav-search') || document.querySelector('.wy-nav-top');
      if (!container) return;

      var link = document.createElement('a');
      link.href = '_static/sdk-docs.pdf';
      link.textContent = '下载 PDF';
      link.setAttribute('download', 'SDK_Documentation.pdf');
      link.style.display = 'inline-block';
      link.style.marginTop = '8px';
      link.style.fontWeight = '600';

      // 包个容器，风格更一致
      var wrapper = document.createElement('div');
      wrapper.style.marginTop = '6px';
      wrapper.appendChild(link);

      container.appendChild(wrapper);
    } catch (e) {
      console && console.warn && console.warn('PDF 下载链接注入失败:', e);
    }
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', insertDownloadLink);
  } else {
    insertDownloadLink();
  }
})();

