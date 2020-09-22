# OneOS概述

OneOS 是中国移动针对物联网领域推出的轻量级操作系统，具有可裁剪、跨平台、低功耗、高安全等特点，支持ARM Cortex-M、MIPS、RISC-V等主流芯片架构，兼容POSIX、CMSIS等标准接口，支持MicroPython语言开发，提供图形化开发工具，能够有效提升开发效率并降低开发成本，帮助用户开发稳定可靠、安全易用的物联网应用。

---

# OneOS主要特点

### 灵活裁剪

抢占式实时多任务的RTOS。内存资源占用极小，支持多任务处理、软件定时器、信号量、互斥锁、消息队列、邮箱和实时调度等特性。可灵活裁剪，搭配丰富组件，适应不同客户需求。

### 跨芯片平台

应用程序可无缝移植，大幅提高软件复用率。支持的主流芯片架构有：ARM Cortex-M、MIPS、RISC-V等。支持几乎所有的MCU和主流的NB-IOT、4G、WIFI、蓝牙通信芯片。

### 丰富组件

提供丰富的组件功能，如互联互通、端云融合、远程升级、室内外定位、低功耗控制等。同时提供开放的第三方组件管理工具，支持添加各类第三方组件，以便扩展系统功能。

### 超低功耗设计

支持MCU和外围设备的功耗管理，用户可以根据业务场景选择相应低功耗方案，系统会自动采用相应功耗控制策略，进行休眠和调频调压，有效降低设备整体功耗。

### FOTA升级

提供免费的FOTA升级服务。支持加密、防篡改、断点续传等功能，同时支持智能还原和回溯机制，拥有完善的版本管理和灵活的升级策略配置机制。

### 全面彻底的安全设计

针对物联网设备资源受限、海量连接、网络异构等特点，参考等保2.0及《GB/T 36951-2018 信息安全技术 物联网感知终端应用安全技术要求》等规范，在系统安全、通信安全、数据安全等方面提供多维度安全防护能力。

### OpenCPU开发框架

支持通信SoC芯片OpenCPU开发模式，为开发者带来屏蔽复杂通信芯片差异的高效开发方式，提供统一开发体验。同时，在同样的业务功能下，减少了设备额外MCU开销和存储器的使用，大幅降低设备成本。

### 简易开发

一站式开发工具OneOS Studio可用于对内核和组件的功能进行配置，支持组件自由裁剪，让系统按需进行积木式构建，同时可帮助用户跟踪调试，快速定位问题。

---

# 许可协议

OneOS 代码遵循 Apache License 2.0 开源协议。

---
# OneOS架构

OneOS 总体架构采用分层设计，主体由内核层、组件层、安全框架组成。采用一个轻量级内核加多个系统组件的模式，使物联网操作系统具备极高的可伸缩性。

### 内核层

极简的硬实时内核，支持多任务管理调度、任务间同步的信号量和互斥量、任务间通信的消息队列和邮箱、以及内存管理等。

### 组件层

包括网络协议、OneNET接入、远程升级、虚拟文件系统、SHELL命令行工具、日志系统、测试框架等。采用模块化设计，使各个组件的功能独立，易于灵活裁剪。

# OneOS目录结构

| **目录** | **描述**                                                     |
| :----------------- | :----------------------------------------------------------- |
| arch               | 存放和 MCU（或 CPU ）架构体系相关的代码。                    |
| common             | 存放一些通用的没有具体业务指向的程序代码，所有模块都可以使用，不通过编译选项控制是否编译<br>，采用默认编译进工程的方式。 |
| components         | 存放组件代码，可进行裁剪。                                   |
| demos              | 存放内核或组件的对外接口如何使用的示例程序。                 |
| docs               | 存放一些文档，如编码规范、编程指南等。                       |
| drivers            | 存放驱动的抽象层代码和具体外设的驱动代码。                   |
| kernel             | 存放内核代码，如任务管理及调度、任务间同步以及通信、内存管理等代码。 |
| libc               | Libc 库部分硬件相关接口的底层适配。                          |
| osal               | OneOS操作系统接口抽象层，支持Posix接口、CMSIS接口、RT-Thread接口等 |
| projects           | 各种开发板的示例工程                                         |
| scripts            | 存放OneOS-Cube工具在编译构造时所需要的脚本文件。             |
| thirdparty         | 存放第三方开源社区或第三方厂家的程序，包括组件、工具、协议实现或对接平台的代码等。 |
| Kconfig            | Menuconfig配置文件，代码工程（如projects目录下的示例工程）中的Kconfig文件<br>会引用此文件 |
| SConscript         | OneOS操作系统使用Scons构建工具时的根编译脚本，该脚本会引用其它目录<br>的SConscript脚本，若在OneOS操作系统根目录增加新的代码目录，需要修改<br>此文件（参见“从零开始构建代码工程”章节）。 |
| LICENSE            | License 授权说明。                                           |

# 硬件支持

##### 目前系统已支持的SOC/MCU列表如下：

| **芯片制造商** | **SOC/MCU** |
| -------------- | ----------- |
| 瑞昱           | RTL8710BX   |
| GigaDevice     | gd32vf103   |
| ST             | STM32F030   |
| ST             | STM32F091   |
| ST             | STM32F103   |
| ST             | STM32F107   |
| ST             | STM32F302   |
| ST             | STM32F303   |
| ST             | STM32F334   |
| ST             | STM32F401   |
| ST             | STM32F405   |
| ST             | STM32F407   |
| ST             | STM32F410   |
| ST             | STM32F411   |
| ST             | STM32F413   |
| ST             | STM32F429   |
| ST             | STM32F446   |
| ST             | STM32F469   |
| ST             | STM32F722   |
| ST             | STM32F746   |
| ST             | STM32F767   |
| ST             | STM32F769   |
| ST             | STM32G071   |
| ST             | STM32H743   |
| ST             | STM32H747   |
| ST             | STM32H750   |
| ST             | STM32L4R5   |
| ST             | STM32L4R9   |
| ST             | STM32L053   |
| ST             | STM32L152   |
| ST             | STM32L432   |
| ST             | STM32L433   |
| ST             | STM32L452   |
| ST             | STM32L475   |
| ST             | STM32L496   |

##### 目前系统已支持的传感器列表如下：

| 厂商           | 传感器                                | 类型                     |
| -------------- | ------------------------------------- | ------------------------ |
| **ADI**        |                                       |                          |
|                | **ADXL345**                           | 加速度计                 |
| **AsahiKASEI** |                                       |                          |
|                | **AK8963**                            | 磁力计                   |
| **BOSCH**      |                                       |                          |
|                | **BMP180**                            | 压力计                   |
| **ST**         |                                       |                          |
|                | **lsm6dsl**                           | 加速度计、陀螺仪、计步计 |
| **invensense** |                                       |                          |
|                | **mpu6xxx(mpu6050/mpu9250/icm20608)** | 加速度计、陀螺仪         |
| **ASAIR**      |                                       |                          |
|                | **aht10**                             | 温度计、湿度计           |
| **ROHM**       |                                       |                          |
|                | **BH1750**                            | 环境光照强度             |
| **Sensirion**  |                                       |                          |
|                | **SHT20**                             | 温度计、湿度计           |

##### 目前系统已适配的开发板列表如下：

注：下列开发板均经过测试可支持 OneOS 系统，但是外设资源并不全面支持，需要用户一定程度二次开发，如果有好的意见和建议欢迎与 OneOS 工程师联系。

| **开发板名称**             | **资料链接**                                                 |
| :------------------------- | :----------------------------------------------------------- |
| amebaz                     | [瑞昱-RTL8710BX](https://www.realtek.com/zh-tw/)             |
| apollo                     |                                                              |
| gd32vf103-Longan-nano      | [Longan Nano](http://longan.sipeed.com/zh/)                  |
| stm32f030-vanviot-s5       |                                                              |
| stm32f091-nucleo-64        | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_F091RC_STM32Nucleo_64%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-860) |
| stm32f103-c8t6-demo        |                                                              |
| stm32f103-m5310-nbiot      | [OneNET论坛](https://open.iot.10086.cn/bbs/thread-19650-1-1.html) |
| stm32f103-rct6             |                                                              |
| stm32f107-vct6-100         | [微雪百科](http://www.waveshare.net/wiki/Open107V)           |
| stm32f302-nucleo           | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_F302R8_STM32Nucleo_64%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-786) |
| stm32f303-k8t6-nucleo      | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_F303K8_STM32Nucleo_32%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-943) |
| stm32f334-r8t6-nucleo      | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_F334R8_STM32Nucleo_64%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-864) |
| stm32f401-ret6-nucleo      | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_F401RE_STM32Nucleo_64%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-721) |
| stm32f405-lvsn-mini        |                                                              |
| stm32f407-atk-explorer     | [正点原子](http://www.openedv.com/docs/boards/stm32/zdyz_stm32f407_explorer.html) |
| stm32f410-rbt6-nucleo      | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_F410RB_STM32Nucleo_64%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-951) |
| stm32f411-ret6-nucleo      | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_F411RE_STM32Nucleo_64%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-867) |
| stm32f413-zht6-nucleo      | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_F413ZH_STM32Nucleo_144%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-1116) |
| stm32f429-atk-apollo       | [正点原子](http://www.openedv.com/docs/boards/stm32/zdyz_stm32f429_apollo.html) |
| stm32f429-st-disco         | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=32F429IDISCOVERY_%E6%9D%BF%E8%BD%BDSTM32F429ZIT6%E5%8D%95%E7%89%87%E6%9C%BA%E7%9A%84%E6%8E%A2%E7%B4%A2%E5%A5%97%E4%BB%B6%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-634) |
| stm32f446-zet6-nucleo      | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_F446RE_STM32Nucleo_64%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-946) |
| stm32f469-st-disco         | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=32F469IDISCOVERY_%E6%9D%BF%E8%BD%BDSTM32F469NIH6%E5%8D%95%E7%89%87%E6%9C%BA%E7%9A%84%E6%8E%A2%E7%B4%A2%E5%A5%97%E4%BB%B6%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-938) |
| stm32f722-st-nucleo        | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_F722ZE_STM32Nucleo_144%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-1112) |
| stm32f746-st-nucleo        | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_F746ZG_STM32Nucleo_144%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-979) |
| stm32f767-atk-apollo       | [正点原子](http://www.openedv.com/docs/boards/stm32/zdyz_stm32f767_apollo.html) |
| stm32f769-st-disco         | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=32F769IDISCOVERY_%E6%9D%BF%E8%BD%BDSTM32F769NIH6%E5%8D%95%E7%89%87%E6%9C%BA%E7%9A%84%E6%8E%A2%E7%B4%A2%E5%A5%97%E4%BB%B6%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-1019) |
| stm32g071-nucleo-64        | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_G071RB_STM32Nucleo_64%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=) |
| stm32h743-atk-apollo       | [正点原子](http://www.openedv.com/docs/boards/stm32/zdyz_stm32h743_shuixing.html) |
| stm32h747-st-disco         | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=STM32H747I_EVAL_%E6%9D%BF%E8%BD%BDSTM32H747XI%E5%8D%95%E7%89%87%E6%9C%BA%E7%9A%84%E8%AF%84%E4%BC%B0%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-1306) |
| stm32h750-h750x-pro        | [野火论坛](https://ebf-products.readthedocs.io/zh_CN/latest/stm32/ebf_stm32h750_pro.html) |
| stm32l4r5-nucleo-144       | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_L4R5ZI_STM32Nucleo_144%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=) |
| stm32l4r9-st-disco         | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=32L4R9IDISCOVERY_%E6%9D%BF%E8%BD%BDSTM32L4R9AI%E5%8D%95%E7%89%87%E6%9C%BA%E7%9A%84%E6%8E%A2%E7%B4%A2%E5%A5%97%E4%BB%B6%E6%9D%BF&lang=EN&ver=) |
| stm32l053-nucleo-64        | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_L053R8_STM32Nucleo_64%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-799) |
| stm32l152-nucleo-64        | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_L152RE_STM32Nucleo_64%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-636) |
| stm32l432-nucleo-32        | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_L432KC_STM32Nucleo_32%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-1031) |
| stm32l433-nucleo-64        | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_L433RC_P_STM32Nucleo_64%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-1142) |
| stm32l452-st-nucleo        | [ST官网](https://www.stmcu.com.cn/Designresource/design_resource_detail?file_name=NUCLEO_L452RE_STM32Nucleo_64%E5%BC%80%E5%8F%91%E6%9D%BF&lang=EN&ver=)        [STMCU社区](https://www.stmcu.org.cn/document/list/index/category-1100) |
| stm32l475-atk-pandora      | [正点原子](http://www.openedv.com/docs/boards/iot/zdyz_panduola.html) |
| stm32l475-cmcc-oneos       |                                                              |
| stm32l496-ali-developerkit | [阿里-云栖社区](https://yq.aliyun.com/articles/646543)       |

# 贡献代码

1.在OneOS官方开源仓库界面找到右上角的fork按钮，克隆出OneOS对应的您自己仓库；

2.根据您的需求在本地git clone一份fork出仓库的代码，进行代码更新；

3.将本地您的代码更新，push 到您fork的远程分支；

4.创建符合规范的pull request，向OneOS官方开源仓库的master分支提交合入请求；

5.OneOS开发团队会定期审查pull request，在通过专家审查后合入到OneOS官方代码中。

# 加入OneOS社区

直接我们的访问OneOS论坛：https://os.iot.10086.cn/forum/consumer/ ，有志同道合的伙伴与您一起学习~

关注我们的微信公众号:中移OneOS，有最新的前沿消息给您推送~

加入我们的QQ群（群名OneOS交流家园，群号：158631242），有专业的开发人员与您探讨~

