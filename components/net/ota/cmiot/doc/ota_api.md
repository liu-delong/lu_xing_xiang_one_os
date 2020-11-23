# OTA组件

------

## 简介

OneOS OTA组件提供了高效的差分算法，同时对应用进行了优化，应用程序仅需要调用少数API即可实现远程升级功能。本文列举了用户可能涉及的API，具备功能和描述如下。

------

## API列表

| **接口**               | **说明**                                                     |
| :--------------------- | :----------------------------------------------------------- |
| cmiot_get_network_type | OTA组件使用的通信网络类型，用于在服务端显示，目前可保持默认值 |
| cmiot_get_try_timer    | OTA组件socket的接收超时时间，单位毫秒                        |
| cmiot_get_try_count    | OTA组件在解码服务器的数据失败时，或端侧处理失败时，重新向服务器请求的次数 |
| cmiot_get_utc_time     | 当前的UTC时间戳，暂时未用到，目前可保持默认值                |
| cmiot_get_uniqueid     | 端侧的MID，最大30字节                                        |
| cmiot_app_name         | APP分区的名字                                                |
| cmiot_download_name    | DOWNLOAD分区的名字                                           |
| cmiot_printf           | OTA组件日志输出接口，当前默认用ONEOS内核的日志输出接口，用户可更改 |
| cmiot_delay            | OTA延时接口，当前默认用ONEOS内核的延时接口，用户可更改       |
| cmiot_upgrade          | 开始检测下载固件包                                           |
| cmiot_report_upgrade   | 上报升级结果                                                 |

## API说明

### cmiot_get_network_type

该函数用于设置网络类型，其函数原型如下：

```c
cmiot_char *cmiot_get_network_type(void);
```

| **参数**     | **说明**                       |
| :----------- | :----------------------------- |
| 无           | 无                             |
| **返回**     | **说明**                       |
| cmiot_char * | 网络类型，用户自定义，不能为空 |

### cmiot_get_try_timer

该函数用于设置socket的接收超时时间，单位毫秒，其函数原型如下：

```c
cmiot_uint cmiot_get_try_timer(void);
```

| **参数**   | **说明**           |
| :--------- | :----------------- |
| 无         | 无                 |
| **返回**   | **说明**           |
| cmiot_uint | socket接收超时时间 |

## cmiot_get_try_count

该函数用于设置在解码服务器的数据失败时，或端侧处理失败时，重新向服务器请求的次数 ，函数原型如下：

```c
cmiot_int8 cmiot_get_try_count(void);
```

| **参数**   | **说明** |
| :--------- | :------- |
| 无         | 无       |
| **返回**   | **说明** |
| cmiot_int8 | 重试次数 |

## cmiot_get_utc_time

该函数用于设置当前的UTC时间戳，函数原型如下：

```c
cmiot_int32 cmiot_get_utc_time(void);
```

| **参数**    | **说明**      |
| :---------- | :------------ |
| 无          | 无            |
| **返回**    | **说明**      |
| cmiot_int32 | 当前UTC时间戳 |

## cmiot_get_uniqueid

该函数用于设置端侧的MID，其函数原型如下：

```c
void cmiot_get_uniqueid(cmiot_uint8 *uid);
```

| **参数**      | **说明**                          |
| :------------ | :-------------------------------- |
| cmiot_uint8 * | 用于设置MID的内存地址，最大30字节 |
| **返回**      | **说明**                          |
| 无            | 无                                |

### cmiot_app_name

该函数用于设置APP分区的名字，其函数原型如下：

```c
cmiot_char *cmiot_app_name(void);
```

| **参数**     | **说明**              |
| :----------- | :-------------------- |
| 无           | 无                    |
| **返回**     | **说明**              |
| cmiot_char * | APP分区名字，不能为空 |

### cmiot_download_name

该函数用于设置DOWNLOAD分区的名字，该函数原型如下：

```c
cmiot_char *cmiot_download_name(void);
```

| **参数**     | **说明**                   |
| :----------- | :------------------------- |
| 无           | 无                         |
| **返回**     | **说明**                   |
| cmiot_char * | DOWNLOAD分区名字，不能为空 |

### cmiot_printf

该函数用于日志输出，该函数原型如下：

```c
void cmiot_printf(cmiot_char *data, cmiot_int len);
```

| **参数** | **说明**         |
| :------- | :--------------- |
| data     | 需要输出的字符串 |
| len      | 字符串长度       |
| **返回** | **说明**         |
| 无       | 无               |

### cmiot_delay

该函数用于延时，该函数原型如下：

```c
void cmiot_delay(cmiot_uint32 delay);
```

| **参数** | **说明**    |
| :------- | :---------- |
| delay    | 延时的ticks |
| **返回** | **说明**    |
| 无       | 无          |

### cmiot_upgrade

该函数开始检测下载固件包，其函数原型如下：

```c
cmiot_int cmiot_upgrade(void);
```

| **参数**             | **说明**                          |
| :------------------- | :-------------------------------- |
| 无                   | 无                                |
| **返回**             | **说明**                          |
| E_CMIOT_SUCCESS      | 有包且下载成功                    |
| E_CMIOT_FAILURE      | 下载失败                          |
| E_CMIOT_NOT_INITTED  | OTA组件初始化失败，一般是内存不够 |
| E_CMIOT_LAST_VERSION | 没有新的固件包                    |

### cmiot_report_upgrade

该函数用于上报升级结果，其函数原型如下：

```c
cmiot_int cmiot_report_upgrade(void);
```

| **参数**            | **说明**                          |
| :------------------ | :-------------------------------- |
| 无                  | 无                                |
| **返回**            | **说明**                          |
| E_CMIOT_SUCCESS     | 有升级结果且上报成功              |
| E_CMIOT_FAILURE     | 有升级结果但上报失败              |
| E_CMIOT_NO_UPGRADE  | 没有升级结果                      |
| E_CMIOT_NOT_INITTED | OTA组件初始化失败，一般是内存不够 |
