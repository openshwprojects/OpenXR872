# nghttp2  示例工程

> nghttp2示例工程展示了XRadio SDK中使用HTTP/2.0 Client获取(get)、更新(put)数据的方法。
>

---

## 适用平台

> 本工程适用以下芯片类型：
>
> 1. XR808系列芯片：XR808CT
> 2. XR872系列芯片：XR872AT、XR872ET

> 本工程适用以下评估板类型：
> 1. 底板：XR808_EVB_IO、XR872_EVB_IO、XR872_EVB_AI
> 2. 模组：XR808CT0_MD01、XR808CT0_MD02、XR872AT_MD01

> 本工程在基于"XR872AT_MD01"的“XR872_EVB_AI”板上测试通过。
> 若需要在其他适用芯片和评估板上运行本工程，请根据快速指南《XRadio_Quick_Start_Guide-CN》的提示进行相关配置修改。

> XRadio Wireless MCU芯片和评估板的更多信息可在以下地址获取：
> https://docs.xradiotech.com

## 工程配置

> localconfig.mk：
> * __CONFIG_NGHTTP2：必选项，配置使能 nghttp2 模块
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
> * PRJCONF_NET_EN：必选项，配置使能网络功能

## 模块依赖

> 必选项
>
> 1. wlan模块：网络连接需要依赖的库

---

## 工程说明

> 本工程启动后，通过配网命令连接AP后，会自动向链接 “https://http2.golang.org/ECHO” 发送put请求，并接受服务器响应，
> 等待 5 秒后，再向 “https://http2.golang.org/clockstream” 发送get请求获取数据，10秒后自动断开连接。

### 操作说明

> 1. 编译工程，烧录镜像，启动
> 2. 往串口输入配网命令进行网络连接
> 3. 成功联网后，等待自动获取链接的网页数据

> XRadio SDK的编译、烧写等操作方式的说明可在以下地址获取：
> https://docs.xradiotech.com

### 控制命令

> 1. 网络连接
>
>    $ net sta config <ssid> <passphrase>
>
>    $ net sta enable

### 代码结构
```
#本工程
.
├── gcc
│   ├── localconfig.mk          # 本工程的配置选项，主要用于覆盖默认全局配置
│   └── Makefile                # 本工程的编译规则，如ld文件、image.cfg、等文件指定，可覆盖默认配置
├── main.c                      # 本工程的代码实现
├── prj_config.h                # 本工程的配置选项，主要用于功能的选择。
└── readme.md                   # 本工程的说明文档

#本程用到XRadio SDK的其他配置文件
.
└── project
    └── common
        └── board
            └── xr872_evb_ai           #本工程在Makefile中指定使用xr872_evb_ai的板级配置
                ├── board_config.h     #本工程的板级配置，
                └── board_config.c     #本工程的板级pin mux的配置。
```
### 代码流程

> 1. main()入口：创建并注册网络状态监视器，网络一旦连接成功后，便会调用注册的回调函数net_cb
> 2. http2_demo_loop()函数流程：
>   A）新建连接结构体struct http_conn
>   B）设置ssl证书参数(不设置也可正常运行)
>   C）设置连接超时时间
>   D）发起连接
>   E）启动执行线程
>   F）发起GET、PUT请求，设置相应的回调函数...从回调函数中接收响应信息
>   G）结束执行线程
>   H）释放连接(自动关闭连接)
>

---

## 常见问题

> 问：有时响应速度长或建立连接超时？

   答：取决于网络状态，服务器状态，由于示例代码中的链接是国外的链接，容易访问失败，可以使用PC验证当前环境与服务器直接网络是否通畅，再进行尝试。

## 参考文档

无
