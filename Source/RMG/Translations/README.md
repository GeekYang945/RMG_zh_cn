# Translations

RMG is written in English, translations are done with Qt Linguist.

The translation files are compiled into the `RMG` executable as Qt resources
(`:/i18n/rmg_<locale>.qm`) and loaded at runtime, see
`MainWindow::configureLanguage()`.

The following texts are part of this catalog:

- RMG itself
- The configuration dialogs of RMG-Input, RMG-Input-GCA and RMG-Audio,
  because those dialogs are shown from within RMG

The configuration dialog of GLideN64 has its own translations which are
shipped with GLideN64 (`Source/3rdParty/mupen64plus-video-GLideN64/translations`).
They are embedded into RMG as well (see `Translations.qrc`) and loaded
when the selected language in RMG has a matching GLideN64 translation.

`rmg_extra_zh_CN.ts` is not managed by lupdate. It contains translations
which cannot be extracted from RMG's sources, such as Qt's standard dialog
buttons and strings of the GLideN64 configuration dialog which are missing
in GLideN64's own translation.

## Updating the source texts

After adding or changing translatable strings, run the following CMake target
to update the `.ts` files:

```sh
cmake --build <build-directory> --target update_translations
```

Then translate the new unfinished entries with Qt Linguist.

## Adding a new language

1. Add the `.ts` file to `qt_add_translations()` in `Source/RMG/CMakeLists.txt`.
2. Add the language to the `languageComboBox` in
   `Source/RMG/UserInterface/Dialog/SettingsDialog.ui`.
3. Update the index/locale mapping in `loadInterfaceGeneralSettings()` and
   `saveInterfaceGeneralSettings()` in
   `Source/RMG/UserInterface/Dialog/SettingsDialog.cpp`.
