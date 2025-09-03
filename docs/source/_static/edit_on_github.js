(function(){
  function getMetaEditBase(){
    // 从 Sphinx html_context 注入的值读取
    try {
      var metas = document.getElementsByTagName('meta');
      for (var i=0;i<metas.length;i++){
        if (metas[i].getAttribute('name') === 'edit-base-url'){
          return (metas[i].getAttribute('content') || '').replace(/\/+$/, '/')
        }
      }
    } catch(e) {}
    return '';
  }

  function injectMetaFromContext(){
    // conf.py 会把 edit_base_url 放到 html_context；我们在模板层插一条 meta
    if (window && window.SPHINX_EDIT_BASE_URL && !document.querySelector('meta[name="edit-base-url"]')){
      var m = document.createElement('meta');
      m.setAttribute('name','edit-base-url');
      m.setAttribute('content', window.SPHINX_EDIT_BASE_URL);
      document.head.appendChild(m);
    }
  }

  function buildEditUrl(){
    var base = getMetaEditBase();
    // 规范化 base，确保为绝对 URL，避免生成相对路径
    function normalizeBase(s){
      if (!s) return '';
      s = (s + '').trim();
      if (/^https?:\/\//i.test(s)) return s.replace(/\/+$/, '/')
      if (/github\.com\//i.test(s)) return ('https://' + s).replace(/\/+$/, '/');
      return '';
    }
    base = normalizeBase(base);
    if (!base) {
      // 在 file:// 环境下没有配置 base，则不显示按钮
      if (location && location.protocol === 'file:') return '';
    }
    if (!base) return '';
    var branch = (window.versionInfo && window.versionInfo.branch) ? window.versionInfo.branch : 'master';
    // 当前页面在版本目录后的相对路径
    var path = location.pathname;
    var urlPath = (window.versionInfo && window.versionInfo.url_path) ? window.versionInfo.url_path : 'latest';
    // 仅截取版本目录后的相对路径（不含域名与构建根）
    var marker = '/' + urlPath + '/';
    var idx = path.indexOf(marker);
    if (idx >= 0){
      path = path.substring(idx + marker.length);
    }

    // 将 docs 拷贝后的路径映射回真实项目源路径
    // 示例：driver/Titan_driver_hyperram/README_zh.html -> project/Titan_driver_hyperram/README_zh.md
    var projectsDir = (window.versionInfo && window.versionInfo.projectsDir) ? window.versionInfo.projectsDir : '';
    var copyFiles = (window.versionInfo && window.versionInfo.copyFiles) ? window.versionInfo.copyFiles : [];
    var mapped = path;
    // 剥离 docs 分类前缀（start/basic/driver/component/...）后取剩余相对路径
    var firstSlash = mapped.indexOf('/');
    if (firstSlash >= 0) {
      mapped = mapped.substring(firstSlash + 1);
    }
    // 去掉 .html 后缀
    var htmlName = mapped.split('/').pop();
    var base = htmlName.replace(/\.html$/i, '');
    // 依据 copy_files 中的候选名推断源文件名优先级（README_zh.md、README.md 等）
    // 回退规则：
    // 1) 如果 base 匹配 README_zh/README 等，按列表顺序尝试；
    // 2) 否则将 .html 改为 .md
    var candidates = [];
    if (/^readme(_zh)?$/i.test(base)) {
      var normalized = copyFiles.map(function(n){ return String(n || '').toLowerCase(); });
      // 常见文件名优先
      ['readme_zh.md','readme.md'].forEach(function(name){ if (normalized.indexOf(name) >= 0 && candidates.indexOf(name) < 0) candidates.push(name); });
      // 追加其余列表中文件名（保持顺序）
      copyFiles.forEach(function(n){ var ln = String(n||'').toLowerCase(); if (candidates.indexOf(ln) < 0) candidates.push(ln); });
    }
    if (candidates.length === 0) {
      candidates = [base + '.md'];
    }
    // 组装目录 + 源文件名
    var dir = mapped.substring(0, mapped.lastIndexOf('/'));
    mapped = dir + '/' + candidates[0];
    // 拼接为仓库中的项目路径
    if (projectsDir) {
      mapped = projectsDir.replace(/\/+$/,'') + '/' + mapped.replace(/^\//,'');
    }
    // 组装最终编辑链接
    var editUrl = base + encodeURIComponent(branch) + '/' + mapped;
    return editUrl;
  }

  function insertButton(){
    var url = buildEditUrl();
    if (!url) return;

    // 优先插入到面包屑，保持与导航同高；使用带边框+图标样式
    var breadcrumbs = document.querySelector('.wy-breadcrumbs');
    if (breadcrumbs) {
      var oldAside = breadcrumbs.querySelector('.wy-breadcrumbs-aside');
      if (oldAside && oldAside.parentNode) oldAside.parentNode.removeChild(oldAside);
      var li = document.createElement('li');
      li.className = 'wy-breadcrumbs-aside';
      var a = document.createElement('a');
      a.href = url;
      a.target = '_blank';
      a.rel = 'noopener';
      a.className = 'sdk-edit-on-github__btn';
      a.innerHTML = '<svg class="sdk-edit-on-github__icon" viewBox="0 0 16 16" width="14" height="14" aria-hidden="true" focusable="false"><path fill="currentColor" d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0 0 16 8c0-4.42-3.58-8-8-8Z"></path></svg><span>在 GitHub 上编辑</span>';
      li.appendChild(a);
      breadcrumbs.appendChild(li);
      return;
    }

    // 回退：插入到正文容器顶部右侧
    var content = document.querySelector('.wy-nav-content');
    if (!content) return;
    if (document.querySelector('.sdk-edit-on-github')) return;
    var container = document.createElement('div');
    container.className = 'sdk-edit-on-github';
    var a2 = document.createElement('a');
    a2.className = 'sdk-edit-on-github__btn';
    a2.target = '_blank';
    a2.rel = 'noopener';
    a2.href = url;
    a2.innerHTML = '<svg class="sdk-edit-on-github__icon" viewBox="0 0 16 16" width="14" height="14" aria-hidden="true" focusable="false"><path fill="currentColor" d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0 0 16 8c0-4.42-3.58-8-8-8Z"></path></svg><span>在 GitHub 上编辑</span>';
    container.appendChild(a2);
    var first = content.firstElementChild;
    if (first) content.insertBefore(container, first); else content.appendChild(container);
  }

  function init(){
    injectMetaFromContext();
    if (document.readyState === 'loading'){
      document.addEventListener('DOMContentLoaded', insertButton);
    } else {
      insertButton();
    }
  }

  init();
})();

