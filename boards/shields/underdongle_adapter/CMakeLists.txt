if(CONFIG_SHIELD_UNDERDONGLE_ADAPTER)
  zephyr_library()
  file(GLOB font_sources src/fonts/*.c)
  zephyr_library_sources(${ZEPHYR_BASE}/misc/empty_file.c)
  zephyr_library_include_directories(${ZEPHYR_LVGL_MODULE_DIR})
  zephyr_library_include_directories(${ZEPHYR_BASE}/lib/gui/lvgl/)
  zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)
  zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)
  zephyr_library_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/include)
  zephyr_library_include_directories(${ZEPHYR_CURRENT_CMAKE_DIR}/include)
  zephyr_library_include_directories(include)
  # --- New SCADA/CRT dongle UI (UI layer fully separated from ZMK data) ---
  zephyr_library_sources(src/ui/dongle_ui.c)
  zephyr_library_sources(src/ui/main_screen.c)
  zephyr_library_sources(src/ui/boot_screen.c)

  zephyr_library_sources(src/display_init.c)
  if(CONFIG_RAW_HID)
    zephyr_library_sources(src/hid.c)
  endif()

  # Body pixel font (VT323). Lives under src/widgets/ but is a pure LVGL font
  # asset; compiled directly so the UI can reference vt323_16.
  zephyr_library_sources(src/widgets/vt323_16.c)

  zephyr_library_sources(${font_sources})
endif()