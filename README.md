## 说明

---

一个基于安信可Rd04雷达模块的一个警戒助手。

有效探测距离为8m左右



## 安装

---

1. sdk

> 请参考[Ai-Thinker-WB2](https://github.com/WildboarG/Ai-Thinker-WB2)

1.  安装

   ```SHELL
   git clone https://github.com/WildboarG/Ai-Thinker-WB2.git
   ```

   

2. 指定sdk路径

   > 修改makefile中，指定自己的sdk路径
   >
   > BL60X_SDK_PATH ?= /home/wildboarg/github/Ai-Thinker-WB2

3. 编译

   ```shell
   make -j32
   ```

4. 烧录

   ```shell
   make flash
   ```

   



## 材料

- AI-wb2-12f wifi芯片8RMB
- rd04雷达模组 



## 原理

---

用WB2的IIC配置好雷达模组之后，不断读取雷达模组的输出，判断是否有物体移动，通过wifi联网发送到个人消息服务器通知（这里采用[gotify](https://gotify.net/)）。一个网页按钮来决定是否启用通知。


