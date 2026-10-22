# Translations

Phase 1 has no translation files: the bar only contains a handful of `qsTr()`
strings (the system menu entries) and no `.ts` was generated yet.

To add translations later, run `lupdate` over `src/` and `src/qml/` into this
directory:

```bash
lupdate -recursive src -ts translations/lingmo-statusbar_zh_CN.ts
```

The `.ts` files are then compiled with `qt6_add_translations()` in
`CMakeLists.txt`.
