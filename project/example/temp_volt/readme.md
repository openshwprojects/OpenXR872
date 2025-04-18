# 温度电压检测示例工程

> 温度电压检测示例工程展示了温度电压检测模块接口使用方法。
>
> 本工程中提供以下模块接口使用的示例：
> 1. 单次获取温度电压值的接口使用；
> 2. 定期获取温度电压值的接口使用；
> 3. 温度电压值超阈值报警的接口使用；


---

## 适用平台

> 本工程适用以下芯片类型：
> 1. XR808系列芯片：XR808CT
> 2. XR872系列芯片：XR872AT、XR872ET

> 本工程适用以下评估板类型：
> 1. 底板：XR872_EVB_AI、XR872_EVB_IO、XR808_EVB_IO
> 2. 模组：XR872AT_MD01、XR808CT0_MD01、XR808CT0_MD02

> 本工程在基于"XR872AT_MD01"的“XR872_EVB_AI”板上测试通过。
> 若需要在其他适用芯片和评估板上运行本工程，请根据快速指南《XRadio_Quick_Start_Guide-CN》的提示进行相关配置修改。

> XRadio Wireless MCU芯片和评估板的更多信息可在以下地址获取：
> https://docs.xradiotech.com

## 工程配置

> localconfig.mk：
> * N/A
>
> Makefile：
> * PRJ_BOARD：必选项，选择板子的板级配置路径
>
> board_config.h
> * N/A
>
> board_config.c
> * N/A
>
> prj_config.h
>
> * N/A

## 模块依赖

> 必选项
> 1.libxrwireless.a：
> 2.libwlan.a

---

## 工程说明

> 本工程的实现默认注册温度电压上报的回调函数，通过命令操作实现以下功能：
> 1.单次获取温度电压值；
> 2.定期获取温度电压值；
> 3.温度电压值超阈值报警；

### 操作说明：

> 1. 编译工程，烧录镜像，启动即可
> 2. 系统启动后，需要先连接Wi-Fi
> 3. 用户根据需求，输入命令实现对应功能


> XRadio SDK的编译、烧写等操作方式的说明可在以下地址获取：
> https://docs.xradiotech.com

### 控制命令
> 1. 单次获取温度电压值：
>    net wlan get_temp_volt
>
> 2. 定期获取温度电压值：
>    net wlan set_tv_upload e=<enable> p=<period>
>    例如，使能该功能，并以1s间隔获取结果：
>    net wlan set_tv_upload e=1 p=10
>
> 3. 温度电压值超阈值报警：
>    net wlan set_tv_thresh THe=<enable> TH=<temp_high> TLe=<enable> TL=<temp_low> VHe=<enable> VH=<volt_high> VLe=<enable> VL=<volt_low>
>    例如，使能温度阈值上限为30°C，下限为25°C，电压上限为5V，下限为4.5V：
>    net wlan set_tv_thresh THe=1 TH=30 TLe=1 TL=25 VHe=1 VH=5 VLe=1 VL=4.5

### 代码结构
```
.
├── gcc
│   ├── localconfig.mk          # 本工程的配置规则，用于覆盖默认配置
│   └── Makefile                # 本工程的编译规则，可指定src、lib、ld、image.cfg、board_config等文件
├── image
│   └── xr872
│       └── image.cfg           # 本工程的镜像分区配置
├── command.c                   # 本工程的命令实现
├── command.h                   # 本工程的命令头文件
├── main.c                      # 本工程的入口，进行wdg三种工作模式的选择和执行
├── prj_config.h                # 本工程的配置选项，主要用于功能的选择。
└── readme.md                   # 本工程的说明文档

#本程用到XRadio SDK的其他配置文件
.
└── project
    └── common
        └── board
            └── xr872_evb_ai             #本工程在Makefile中指定使用xr872_evb_ai的板级配置
                ├── board_config.h     #本工程的板级配置，
                └── board_config.c     #本工程的板级pin mux的配置。
```
### 代码流程

> 1. main()入口：
> A）初始化系统
> B）注册处理温度电压上报信息的回调函数
> 2. 回调函数流程：
> A）打印出温度电压信息
> B）根据flag置起的位判断出本次消息上报的原因

---

## 常见问题

> 若出现上报时间不准确，或者长时间不上报信息，尝试关闭WLAN的PS功能或者low power功能

## 参考文档

> * N/A
