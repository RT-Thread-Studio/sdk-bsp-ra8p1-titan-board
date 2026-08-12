#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Documentation utility package with dependency-safe lazy imports."""

from importlib import import_module


_EXPORTS = {
    "ConfigLoader": (".config_loader", "ConfigLoader"),
    "DocumentCatalog": (".document_catalog", "DocumentCatalog"),
    "DocumentEntry": (".document_catalog", "DocumentEntry"),
    "ProjectScanner": (".project_scanner", "ProjectScanner"),
    "FileProcessor": (".file_processor", "FileProcessor"),
    "IndexGenerator": (".index_generator", "IndexGenerator"),
}

__all__ = list(_EXPORTS)


def __getattr__(name):
    if name not in _EXPORTS:
        raise AttributeError(f"module {__name__!r} has no attribute {name!r}")
    module_name, attribute_name = _EXPORTS[name]
    value = getattr(import_module(module_name, __name__), attribute_name)
    globals()[name] = value
    return value
