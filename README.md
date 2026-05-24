# DuduClock (TaoTao 天气时钟)

基于 ESP32 的桌面智能天气时钟，支持实时天气、空气质量、7日预报、计时器、本地温湿度等功能。

## 功能特性

- **实时天气**：当前温度、天气状况、体感温度、风向风力、能见度、湿度
- **空气质量**：PM10、PM2.5、NO2、SO2、CO、O3 六项指标
- **7日天气预报**：未来一周天气趋势
- **本地温湿度**：DHT11 传感器实时监测
- **NTP 自动对时**：多 NTP 服务器冗余，每 58 分钟自动校准
- **太空人动画**：桌面待机动画
- **计时器**：支持开始/暂停/归零
- **主题切换**：黑白双色主题
- **WiFi 配网**：首次启动自动进入 AP 配网模式，手机扫码即可配置

## 硬件清单

| 组件 | 规格 | 说明 |
|------|------|------|
| 主控 | ESP32-C3 / ESP32-S3 | 二选一，见下方兼容性说明 |
| 显示屏 | ST7789 240x320 TFT | 2.0 寸 IPS 屏 |
| 温湿度传感器 | DHT11 | GPIO6 |
| 按键 | 轻触开关 | GPIO8，支持单击/双击/长按 |
| LED | 板载 LED | GPIO12 |

### 显示屏接线

| TFT 引脚 | ESP32 GPIO |
|----------|-----------|
| MOSI | 3 |
| SCLK | 2 |
| CS   | 7 |
| DC   | 4 |
| RST  | 5 |

## 支持的开发板

本项目已适配两款开发板，通过 PlatformIO 环境切换：

| 环境 | 开发板 | 串口 |
|------|--------|------|
| `esp32c3` (默认) | AirM2M CORE ESP32C3 | `/dev/ttyACM0` |
| `esp32s3` | 立创 ESP32-S3R8N8 | `/dev/ttyUSB0` |

### ESP32-S3 兼容性说明

ESP32-S3 使用 ESP-IDF 5.x，TFT_eSPI 库需要自动补丁修复 SPI 寄存器映射问题。构建时 `scripts/patch_tft_espi_s3.py` 会自动处理，无需手动干预。

## 软件依赖

- [PlatformIO](https://platformio.org/)
- 库依赖（PlatformIO 自动安装）：
  - `bblanchon/ArduinoJson@6.21.5`
  - `mathertel/OneButton@^2.6.1`
  - `bodmer/TFT_eSPI@2.5.43`
  - `arkhipenko/TaskScheduler@3.6.0`
  - `arduinozlib`

## 构建与烧录

```bash
# ESP32-C3（默认）
pio run -e esp32c3 -t upload

# ESP32-S3
pio run -e esp32s3 -t upload

# 查看串口日志
pio device monitor -e esp32c3
```

首次烧录后需进行 WiFi 配网。

## 使用说明

### 首次配网

1. 设备首次启动自动进入配网模式，屏幕显示二维码
2. 手机连接 WiFi `TaoTaoClock`
3. 浏览器访问 `192.168.1.1`，选择 WiFi 并输入密码
4. 配置成功后设备自动重启并进入时钟页面

### 按键操作

| 操作 | 当前页面 | 功能 |
|------|---------|------|
| 单击 | 计时器 | 开始/暂停计时 |
| 双击 | 任意页面 | 切换页面（天气 -> 空气质量 -> 7日预报 -> 主题 -> 计时器 -> 重置 -> 天气） |
| 长按 3s | 重置页面 | 恢复出厂设置 |
| 长按 3s | 主题页面 | 切换黑白主题 |
| 长按 3s | 计时器 | 计时归零 |

### 页面说明

- **天气页面**：显示当前时间、日期、天气图标、温度、体感温度、风向、能见度、湿度，底部轮播天气信息，左上角太空人动画
- **空气质量页面**：PM10、PM2.5、NO2、SO2、CO、O3 六项指标
- **7日预报页面**：未来一周每日天气概况
- **主题页面**：长按切换黑白主题
- **计时器页面**：单击开始/暂停，长按归零
- **重置页面**：长按 3 秒恢复出厂设置（清除 WiFi 和城市配置）

## 项目结构

```
├── include/
│   └── TFT_eSPI_Setup.h      # TFT 显示配置
├── scripts/
│   └── patch_tft_espi_s3.py  # ESP32-S3 自动补丁
├── src/
│   ├── DuduClock_2.0.ino     # 主程序入口
│   ├── net.cpp/.h            # 网络相关：WiFi、HTTP、NTP、天气 API
│   ├── task.cpp/.h           # 多任务调度、定时器、按键处理
│   ├── tftUtil.cpp/.h        # TFT 显示工具函数
│   ├── preferencesUtil.cpp/.h # NVS 持久化存储
│   ├── common.h              # 全局定义、数据结构、HTML 配网页面
│   ├── font/                 # 自定义字体
│   └── img/                  # 天气图标、太空人动画帧
├── platformio.ini            # PlatformIO 配置
└── README.md
```

## 天气数据源

- [和风天气](https://www.qweather.com/) API
- 自动定位：通过公网 IP 获取所在城市
- 数据更新频率：实况天气每 8 小时，预报每 24 小时

## 许可证

MIT License
