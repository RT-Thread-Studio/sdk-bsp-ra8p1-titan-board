#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""PDF-only heading normalization for Sphinx LaTeX builds."""

import re


_MANUAL_NUMBER_RE = re.compile(
    r"^\s*(?:第\s*)?(\d+(?:\.\d+)*)(?:\s*章)?(?:\s*[.、:：)）]\s*|\s+)"
)


def strip_manual_heading_number(text: str) -> str:
    """Remove a leading manual section number without treating versions as numbers."""
    value = str(text or "")
    match = _MANUAL_NUMBER_RE.match(value)
    if not match:
        return value

    parts = match.group(1).split(".")
    if len(parts) > 1 and any(part == "0" or part.startswith("0") for part in parts[1:]):
        return value
    return value[match.end():].lstrip()


def normalize_latex_heading_numbers(app, doctree, docname):
    """Let LaTeX number section headings while leaving HTML/source text unchanged."""
    del docname
    if getattr(app.builder, "format", "") != "latex":
        return

    from docutils import nodes

    for section in doctree.findall(nodes.section):
        title = next(
            (child for child in section.children if isinstance(child, nodes.title)),
            None,
        )
        if title is None:
            continue
        first_text = next(
            (child for child in title.children if isinstance(child, nodes.Text)),
            None,
        )
        if first_text is None:
            continue
        normalized = strip_manual_heading_number(str(first_text))
        if normalized != str(first_text):
            first_text.parent.replace(first_text, nodes.Text(normalized))
