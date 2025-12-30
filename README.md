# KidsBar - 儿童互动电子宠物设备

基于 ESP32-S3 的儿童互动设备，通过电子宠物引导 4-10 岁儿童完成学习任务。

## 🎯 项目愿景

**三阶段成长路线：**

### 第一阶段：电子宠物（已启动）
- 🐾 虚拟宠物系统：喂食、玩耍、照顾
- 📊 状态追踪：饥饿度、快乐度、健康度
- 🎨 E-ink 显示：低功耗动画展示
- 🎮 旋转编码器交互：直觉化操作

### 第二阶段：任务奖励系统（规划中）
- ✅ 家长设置任务：刷牙、整理房间、作业
- 🏆 完成任务获得虚拟货币
- 🛍️ 用货币购买宠物用品和装饰
- 📈 培养责任感和计划能力

### 第三阶段：学习游戏化（未来）
- 🧮 数学、识字、逻辑训练小游戏
- 🌟 学习成果转化为宠物成长
- 📚 寓教于乐，主动学习动力
- 🏅 成就系统激励进步

## 🔧 硬件平台

- **MCU**: ESP32-S3 (双核 240MHz, 8MB Flash)
- **显示**: 2.9" E-ink 显示器 (296x128, 黑白)
- **输入**: 旋转编码器（带按钮）
- **反馈**: WS2812B RGB LED
- **电源**: USB-C 供电或锂电池

## 🏗️ 底层驱动架构

```
KidsBar/
├── platformio.ini          # ESP32-S3 配置
├── partitions.csv          # 8MB OTA 分区表
├── include/
│   ├── config.h            # 硬件引脚定义
│   ├── app_state.h         # 全局状态管理
│   ├── encoder_pcnt.h      # 旋转编码器驱动
│   ├── led_status.h        # RGB LED 控制
│   ├── ui.h                # E-ink 显示驱动
│   └── settings_store.h    # NVS 持久化存储
└── src/
    ├── main.cpp            # 主程序入口
    ├── app_state.cpp       # 状态实现
    ├── encoder_pcnt.cpp    # 编码器实现
    ├── led_status.cpp      # LED 实现
    ├── ui.cpp              # 显示实现
    └── settings_store.cpp  # 存储实现
```

## 🚀 快速开始

### 环境准备
```bash
# 安装 PlatformIO
pip install platformio

# 克隆项目
git clone https://github.com/max05210238/KidsBar.git
cd KidsBar
```

### 编译上传
```bash
# 编译
pio run

# 上传到 ESP32-S3
pio run --target upload

# 串口监视
pio device monitor
```

## 📖 从 CryptoBar 移植

本项目底层硬件驱动移植自 [CryptoBar](https://github.com/max05210238/CryptoBar)：
- ✅ 旋转编码器 PCNT 驱动（高性能硬件计数）
- ✅ WS2812B LED 状态指示
- ✅ E-ink 显示优化（减少刷新延迟）
- ✅ NVS 设置持久化

在此基础上专为儿童设备定制：
- 🎨 简化 UI，适合儿童认知
- 🔋 低功耗优化（E-ink + 深度睡眠）
- 🛡️ 家长控制和内容过滤

## 📋 当前开发状态

- [x] 底层硬件驱动移植
- [x] E-ink 显示基础框架
- [ ] 电子宠物核心逻辑
- [ ] 动画和图形资源
- [ ] 任务系统设计
- [ ] 家长控制面板

## 🤝 贡献

欢迎对儿童教育和嵌入式开发感兴趣的开发者参与！

## 📄 许可证

MIT License - 详见 [LICENSE](LICENSE)

---

**为孩子们打造有温度的学习伙伴** ❤️
