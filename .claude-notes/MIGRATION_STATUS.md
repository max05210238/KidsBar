# KidsBar 迁移状态 - CryptoBar 到 KidsBar

## ✅ 已完成的底层驱动（100%）

### 核心硬件驱动
- ✅ `encoder_pcnt.cpp/h` - 旋转编码器 PCNT 驱动
  - ESP32-S3 PCNT 硬件支持
  - GPIO 1/2 优化配置
  - 防抖动、EMI 抑制

- ✅ `led_status.cpp/h` - WS2812B RGB LED 控制
  - 外部 LED (GPIO 15)
  - 板载 LED 强制关闭节能

- ✅ `ui.cpp/h` - E-ink 显示驱动
  - GxEPD2 库支持
  - 2.9" 显示器 (296x128)
  - 部分刷新优化

- ✅ `settings_store.cpp/h` - NVS 持久化存储
  - 保存宠物状态
  - 用户设置

### KidsBar 专属组件
- ✅ `app_state.cpp/h` - 宠物状态管理
  - 饥饿度、快乐度、健康度
  - 等级和经验值系统
  - 状态更新逻辑

- ✅ `config.h` - 硬件配置（简化版）
  - ESP32-S3 引脚定义
  - 宠物更新间隔
  - NVS 键定义

- ✅ `main.cpp` - 主程序
  - 硬件初始化
  - 事件循环
  - 编码器/LED 反馈

### 配置文件
- ✅ `platformio.ini` - PlatformIO 配置
- ✅ `partitions.csv` - 8MB Flash 分区表
- ✅ `README.md` - 项目文档

### 字体资源
- ✅ `SpaceGroteskBold24pt7b.h` - 显示字体（52KB）

---

## ❌ 从 CryptoBar **不需要**移植的

### Phase 1 (电子宠物) 不需要：
- ❌ WiFi 相关代码 (wifi_portal, network, app_wifi)
- ❌ 加密货币功能 (coins, day_avg)
- ❌ 图表功能 (chart)
- ❌ OTA 更新 (ota_guard, maint_mode)
- ❌ 时区管理 (app_time)
- ❌ 复杂的菜单系统 (ui_list, ui_menu 等)

### 原因：
KidsBar Phase 1 是**离线设备**，专注于本地虚拟宠物互动，不需要网络功能。

---

## 🎯 Phase 1 开发路线图（电子宠物）

### 当前状态：✅ 硬件层完成，可以开始开发！

### 第一步：基础 UI（1-2 天）
**目标：** 在 E-ink 上显示宠物和状态

需要创建：
```cpp
// include/pet_graphics.h
- 宠物表情图标（开心、难过、饥饿、睡觉）
- 状态条图形（心形、食物图标）

// src/pet_ui.cpp
- drawPetFace() - 显示宠物脸
- drawStatusBars() - 显示饥饿/快乐/健康条
- drawHomeScreen() - 主屏幕布局
```

**提示：**
- 用简单的几何图形开始（圆形脸、三角形耳朵）
- 先用 Adafruit GFX 基本绘图函数（fillCircle, drawRect）
- 后期可以用位图替换

### 第二步：交互逻辑（2-3 天）
**目标：** 编码器控制菜单

需要实现：
```cpp
// src/pet_actions.cpp
- feedPet() - 喂食（增加饥饿度）
- playWithPet() - 玩耍（增加快乐度）
- checkHealth() - 检查健康

// src/menu.cpp
- 简单 3 选项菜单：喂食 / 玩耍 / 状态
- 编码器旋转切换选项
- 按钮确认选择
```

### 第三步：动画效果（1-2 天）
**目标：** 让宠物"活"起来

```cpp
// src/pet_animation.cpp
- 喂食动画（嘴巴张开）
- 开心动画（跳跃、闪烁）
- 睡觉动画（眼睛闭合）
- 空闲动画（眨眼）
```

### 第四步：状态持久化（半天）
**目标：** 断电后恢复宠物状态

```cpp
// 已有 settings_store 可用
- 每分钟保存一次状态到 NVS
- 启动时读取上次状态
- 根据离线时间计算饥饿度变化
```

---

## 🔮 Phase 2/3 未来需求

### Phase 2（任务奖励）可能需要：
- ⚠️ 简单的 WiFi（可选，家长 app 同步）
- ⚠️ 时间管理（RTC 或 NTP）
- ⚠️ 任务列表 UI

### Phase 3（学习游戏）可能需要：
- ⚠️ WiFi（题库更新）
- ⚠️ 更多字体（适合儿童的大字体）

**建议：** 先完成 Phase 1，再考虑这些功能。

---

## 🚀 开始开发清单

### 立即可以开始的工作：

1. **测试编译**
   ```bash
   cd /home/user/KidsBar
   pio run
   ```

2. **创建第一个宠物脸**
   - 在 `src/main.cpp` 的 TODO 部分添加简单绘图
   - 用 `display.fillCircle()` 画一个笑脸

3. **测试编码器输入**
   - 已经有基础代码在 `loop()` 中
   - 旋转编码器会打印步数到串口

4. **测试 LED 反馈**
   - 已经有 LED 颜色变化代码
   - 可以添加更多状态指示

### 推荐的开发顺序：
1. ✅ 测试硬件（编译+上传）
2. 🎨 画一个简单的宠物脸
3. 📊 添加状态条显示
4. 🎮 实现 3 选项菜单
5. 🎬 添加简单动画
6. 💾 启用 NVS 保存

---

## 📚 相关文档链接

- **GxEPD2 库文档:** https://github.com/ZinggJM/GxEPD2
- **Adafruit GFX 教程:** https://learn.adafruit.com/adafruit-gfx-graphics-library
- **ESP32 PCNT 文档:** https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/pcnt.html

---

**状态：** ✅ 底层完成，随时可以开始上层开发！
**下一步：** 画出第一个宠物脸 🐱
