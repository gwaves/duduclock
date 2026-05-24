import os

# Patch TFT_eSPI for ESP32-C3:
# ESP-IDF 5.x changed SPI2_HOST from 2 to 1, but REG_SPI_BASE requires >=2.
# This causes SPI register access to resolve to address 0x00000000,
# leading to a white screen or StoreProhibited crash.

PATCH_HEADER = ".pio/libdeps/{env}/TFT_eSPI/Processors/TFT_eSPI_ESP32_C3.h"
PATCH_SOURCE = ".pio/libdeps/{env}/TFT_eSPI/Processors/TFT_eSPI_ESP32_C3.c"

Import("env")

pioenv = env.get("PIOENV", "")
if not pioenv.startswith("esp32c3"):
    print(f"[patch_tft_espi_c3] Skipping for env: {pioenv}")
    env.Exit(0)

project_dir = env.get("PROJECT_DIR", ".")
patched_any = False

# --- Patch header: SPI_PORT and gpio_ll.h ---
header_path = os.path.join(project_dir, PATCH_HEADER.format(env=pioenv))
if os.path.exists(header_path):
    with open(header_path, "r") as f:
        content = f.read()

    if "SPI2_HOST changed to 1 in ESP-IDF 5.x" in content:
        print("[patch_tft_espi_c3] Header already patched, skipping.")
    else:
        # Fix SPI_PORT: original library uses SPI2_HOST which is 1 in ESP-IDF 5.x
        if "#define SPI_PORT SPI2_HOST" in content:
            content = content.replace(
                "#define SPI_PORT SPI2_HOST",
                "#define SPI_PORT 2 // SPI2_HOST changed to 1 in ESP-IDF 5.x, but TFT_eSPI needs >=2"
            )
            patched_any = True
        elif "#define SPI_PORT 2" in content:
            # Someone already changed it to 2 but without our comment
            content = content.replace(
                "#define SPI_PORT 2",
                "#define SPI_PORT 2 // SPI2_HOST changed to 1 in ESP-IDF 5.x, but TFT_eSPI needs >=2"
            )
            patched_any = True

        # Add hal/gpio_ll.h if missing (needed by begin_SDA_Read/end_SDA_Read)
        if '#include "hal/gpio_ll.h"' not in content:
            content = content.replace(
                '#include "driver/spi_master.h"',
                '#include "driver/spi_master.h"\n#include "hal/gpio_ll.h"'
            )
            patched_any = True

        with open(header_path, "w") as f:
            f.write(content)
        print(f"[patch_tft_espi_c3] Patched {header_path}")
else:
    print(f"[patch_tft_espi_c3] Warning: header not found: {header_path}")

# --- Patch source: _spi_user hardcoded address ---
source_path = os.path.join(project_dir, PATCH_SOURCE.format(env=pioenv))
if os.path.exists(source_path):
    with open(source_path, "r") as f:
        content = f.read()

    if "(volatile uint32_t*)(0x60024010)" in content:
        # Remove the commented-out original line if present
        content = content.replace(
            "  // volatile uint32_t* _spi_user      = (volatile uint32_t*)(SPI_USER_REG(SPI_PORT));\n",
            ""
        )
        content = content.replace(
            "volatile uint32_t* _spi_user      = (volatile uint32_t*)(0x60024010);",
            "volatile uint32_t* _spi_user      = (volatile uint32_t*)(SPI_USER_REG(SPI_PORT));"
        )
        with open(source_path, "w") as f:
            f.write(content)
        print(f"[patch_tft_espi_c3] Patched {source_path}")
        patched_any = True
    else:
        print("[patch_tft_espi_c3] Source already patched or no hardcoded address found, skipping.")
else:
    print(f"[patch_tft_espi_c3] Warning: source not found: {source_path}")

if not patched_any:
    print("[patch_tft_espi_c3] Nothing to patch, already up to date.")
