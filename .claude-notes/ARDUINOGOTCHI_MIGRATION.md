# ArduinoGotchi → KidsBar 移植计划

## 📊 兼容性分析

### 硬件对比
| 组件 | ArduinoGotchi | KidsBar | 适配难度 |
|------|---------------|---------|----------|
| **MCU** | ESP32/ESP8266 | ESP32-S3 | ✅ 无需修改 |
| **显示器** | 128x64 OLED (I2C) | 296x128 E-ink (SPI) | ⚠️ 简单（替换库） |
| **输入** | 3 按钮 | 旋转编码器 | ⚠️ 简单（重新映射） |
| **存储** | EEPROM | NVS | ✅ 无需修改 |
| **LED** | 无 | WS2812B | ➕ 额外功能 |

---

## ✅ 可以直接复用（90%）

### 1. **核心游戏逻辑** ✅ 100% 复用
```cpp
TamaLib 模拟器核心：
- tamalib.h/cpp - Tamagotchi P1 完整模拟
- cpu.h/cpp - 虚拟 CPU
- rom_12bit.h - 原版 ROM 数据
- hal.h/cpp - 硬件抽象层
```

**这是真正的 Tamagotchi！** 包含所有原版游戏机制。

### 2. **状态管理** ✅ 100% 复用
```cpp
savestate.h/cpp：
- 自动保存（每 5 分钟）
- EEPROM/NVS 持久化
- 断电恢复
```

### 3. **游戏玩法** ✅ 传统 Tamagotchi
- 喂食（米饭 / 零食）
- 猜拳游戏
- 清洁
- 看医生
- 开关灯
- 纪律训练

---

## ⚠️ 需要适配（10%）

### 1. **显示驱动** - 主要工作

**原始代码（OLED）：**
```cpp
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDRESS);
}

void loop() {
  display.clearDisplay();
  display.drawBitmap(x, y, bitmap, w, h, WHITE);
  display.display();
}
```

**KidsBar 适配（E-ink）：**
```cpp
#include <GxEPD2_BW.h>
GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display(...);

void setup() {
  display.init(115200);
  display.setRotation(1);
}

void loop() {
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.drawBitmap(x, y, bitmap, w, h, GxEPD_BLACK);
  } while (display.nextPage());
}
```

**适配工作量：** 1-2 小时（只需要改显示函数）

---

### 2. **输入映射** - 非常简单

**ArduinoGotchi（3 按钮）：**
```cpp
#define BTN_LEFT   D5
#define BTN_MIDDLE D6
#define BTN_RIGHT  D7
```

**KidsBar 适配（编码器）：**
```cpp
旋转编码器 CW（顺时针） → BTN_RIGHT
旋转编码器 CCW（逆时针）→ BTN_LEFT
编码器按钮按下 → BTN_MIDDLE
```

**适配工作量：** 30 分钟

---

### 3. **分辨率缩放**

ArduinoGotchi 使用 32x32 像素位图（Tamagotchi 原始分辨率）。

**方案 1：直接居中显示**
```cpp
// 屏幕中心：(148, 64)
// Tamagotchi 屏幕：32x32
int offsetX = (296 - 32) / 2;  // = 132
int offsetY = (128 - 32) / 2;  // = 48
```

**方案 2：放大 2 倍**
```cpp
// 32x32 → 64x64（更容易看清）
drawScaledBitmap(bitmap, 32, 32, 2);
```

**适配工作量：** 1 小时

---

## 🎨 美工需求

**好消息：ArduinoGotchi 已经包含所有图形！**

ArduinoGotchi 使用原版 Tamagotchi P1 的位图：
- 12 种数码宠物形态
- 食物图标
- 游戏图标
- UI 元素

**你不需要美工！** 可以直接用原版图形。

如果想要**自定义美术风格**（更适合儿童）：
- 12 个宠物形态（32x32 像素）
- 6 个图标（食物、游戏、医生等）

---

## 📅 移植时间表

### Day 1: 克隆和测试（2 小时）
- [x] 克隆 ArduinoGotchi 到 KidsBar
- [ ] 测试原版代码在 ESP32 上运行
- [ ] 理解代码结构

### Day 2: 显示适配（4 小时）
- [ ] 替换 Adafruit_SSD1306 → GxEPD2
- [ ] 调整刷新逻辑（E-ink 较慢）
- [ ] 测试显示效果

### Day 3: 输入适配（2 小时）
- [ ] 实现编码器 → 按钮映射
- [ ] 测试所有游戏功能

### Day 4: 优化和测试（2 小时）
- [ ] 调整刷新速率（避免 E-ink 过度刷新）
- [ ] 添加 LED 状态指示
- [ ] 完整游戏测试

**总工作量：10-12 小时**（1-2 天）

---

## 🚀 立即开始

我现在就可以帮你：

1. **克隆 ArduinoGotchi 代码到 KidsBar**
2. **创建显示适配层**
3. **实现编码器输入**
4. **测试完整流程**

**优势：**
- ✅ 真实的 Tamagotchi 游戏逻辑
- ✅ 不需要美工（已有原版图形）
- ✅ 经过验证的稳定代码
- ✅ 完整的游戏体验
- ✅ 适合 4-10 岁儿童

要我现在开始移植吗？
