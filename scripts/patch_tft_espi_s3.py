import os

# Patch TFT_eSPI for ESP32-S3:
# ESP-IDF 5.x changed SPI2_HOST from 2 to 1, but REG_SPI_BASE requires >=2.
# This causes a StoreProhibited crash when TFT_eSPI tries to write to SPI registers.

PATCH_TARGET = ".pio/libdeps/{env}/TFT_eSPI/Processors/TFT_eSPI_ESP32_S3.h"
OLD_LINE = "    #define SPI_PORT FSPI"
NEW_LINE = "    #define SPI_PORT 2 // FSPI on ESP32-S3 is SPI2, REG_SPI_BASE needs >=2"

Import("env")

pioenv = env.get("PIOENV", "")
if not pioenv.startswith("esp32s3"):
    print(f"[patch_tft_espi_s3] Skipping for env: {pioenv}")
    env.Exit(0)

project_dir = env.get("PROJECT_DIR", ".")
target = os.path.join(project_dir, PATCH_TARGET.format(env=pioenv))

if not os.path.exists(target):
    print(f"[patch_tft_espi_s3] Target not found: {target}")
    env.Exit(0)

with open(target, "r") as f:
    content = f.read()

if NEW_LINE in content:
    print("[patch_tft_espi_s3] Already patched, skipping.")
    env.Exit(0)

if OLD_LINE not in content:
    print(f"[patch_tft_espi_s3] Warning: expected line not found: {OLD_LINE}")
    env.Exit(0)

content = content.replace(OLD_LINE, NEW_LINE)
with open(target, "w") as f:
    f.write(content)

print(f"[patch_tft_espi_s3] Patched {target}")
