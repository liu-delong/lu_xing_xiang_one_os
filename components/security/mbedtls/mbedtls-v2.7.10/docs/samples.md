# 示例程序

该示例程序提供了一个简单的 TLS client，与测试网站建立 TLS 连接并获取加密数据。

**示例文件**

| 示例程序路径               | 说明         |
| -------------------------- | ------------ |
| samples/mbedtls_app_test.c | TLS 测试例程 |

## 例程工作流程

本例程的测试网站是 `www.gitee.com`，使用 `mbedtls_client_write` 函数发送 HTTP 测试请求，成功后，该网站会返回文本数据，测试例程将解析后的数据输出到控制台。

- 例程使用的 HTTP 请求数据如下所示

```c
"GET /tangyao10/myTest/raw/master/test HTTP/1.1\r\n" \
"Host: gitee.com\r\n" \
"\r\n"
```

- mbedTLS 测试例程的基本工作流程如下所示

  - client 连接测试网站 `www.gitee.org`
  - client 和 server 握手成功
  - client 发送请求
  - server 回应请求
  - TLS 测试成功/失败

## 准备工作

### 配置软件

- menuconfig 配置软件包

  打开 OneOS-Cube 工具，使用 **menuconfig** 配置软件包。

  启用 mbedtls 软件包，并配置使能测试例程（Enable a mbedtls client example），如下所示：

```c
components --->
  security  --->
        Select Root Certificate  --->      # 选择证书文件
    [*] mbedtls: An portable and flexible SSL/TLS library # 打开 mbedtls 软件包
    [*]   Store the AES tables in ROM      # 将 AES 表存储在 ROM 中
    (2)   Maximum window size used         # 用于点乘的最大“窗口”大小（2-7）
    (3584) Maxium fragment length in bytes # 配置数据帧大小
    [*]   Enable a mbedtls client example  # 开启 mbedtls 测试例程
    [ ]   Enable Debug log output          # 开启调试 log 输出
          version (v2.7.10)  --->          # 选择软件包版本，默认为最新版本
```

### 同步设备时间

SSL/TLS 服务器进行证书校验的过程中，会对当前发起校验请求的时间进行认证，如果时间不满足服务器的要求，就会校验证书失败。因此，我们需要为设备同步本地时间。

- 方式一： 使用 NTP 同步网络时间

  该方式需要依赖 NTP 工具包，使用 `menuconfig` 配置获取，如下所示：

```c
TOP --->
    Thirdparty  --->
        -*- NTP  --->
            -*-   Enable NTP(Network Time Protocol) client
            (8)     Timezone for calculate local time
            (cn.ntp.org.cn) NTP server name
```

使用命令 **`ntp_sync`** 同步网络时间

```c
sh />ntp_sync
Get local time from NTP server: Thu Aug  2 14:31:30 2018
The system time is updated. Timezone is 8.
```

## 启动例程

在 shell 中使用命令 **`mbedtls_test`** 执行示例程序，成功建立 TLS 连接后，设备会从服务器拿到一组密码套件，设备 log 如下所示：

```c
sh />mbedtls_test
[20201] I/mbedtls: mbedtls_client_test_entry enter [mbedtls_client_test_entry][66]
[20202] I/mbedtls: Memory usage before the handshake connection is established: [mbedtls_client_test_entry][69]
total memory: 45688
used memory : 14444
maximum allocated memory: 40296
[20204] I/mbedtls: Start handshake tick:20204 [mbedtls_client_test_entry][103]
[20205] D/tls_client: mbedtls client struct init success... [mbedtls_client_init][73]
[20206] D/tls_client: Loading the CA root certificate success... [mbedtls_client_context][131]
[20207] D/tls_client: mbedtls client context init success... [mbedtls_client_context][166]
sh />
sh />[20212] D/tls_client: Connected www.gitee.com:443 success... [mbedtls_client_connect][183]
[20432] D/tls_client: Certificate verified success... [mbedtls_client_connect][206]
[20433] I/mbedtls: Finish handshake tick:20433 [mbedtls_client_test_entry][123]
[20434] I/mbedtls: MbedTLS connect success [mbedtls_client_test_entry][124]
[20435] I/mbedtls: Memory usage after the handshake connection is established: [mbedtls_client_test_entry][127]
total memory: 45688
used memory : 35496
maximum allocated memory: 40296
[20437] I/mbedtls: Writing HTTP request success [mbedtls_client_test_entry][139]
[20437] I/mbedtls: Getting HTTP response [mbedtls_client_test_entry][140]
HTTP/1.1 200 OK
Date: Fri, 12 Jun 2020 10:01:44 GMT
Content-Type: text/plain;charset=utf-8
Transfer-Encoding: chunked
Connection: keep-alive
Keep-Alive: timeout=60
Server: nginx
X-XSS-Protection: 1; mode=block
X-Content-Type-Options: nosniff
X-UA-Compatible: chrome=1
Expires: Sun, 1 Jan 2000 01:00:00 GMT
Pragma: must-revalidate, no-cache, private
ETag: "a3afe0574088075266e0152eabb252a7"
Content-Disposition: inline; filename="test"
Content-Transfer-Encoding: binary
Cache-Control: no-cache
Set-Cookie: oschina_new_user=false; path=/; expires=Tue, 12 Jun 2040 10:01:44 -0000
Set-Cookie: gitee-session-n=cXFKcHVPYUcvd1EzZHRYL2MwNk5VU2svVjFBdlVBS2N2dnk0VktaelRDbGZ2Z0RweUE5bFgvWnJHV2hyWjV1NUFiSWtKMDVBWTRBRXdCOU5JT0FBQ2c9PS0tbFNPeURhRmtXRG5EQi8zV3dwdjFqQT09--b37d45966a1eef5a56fdc376f87cf9a4040d09dd; domain=.gitee.com; path=/; HttpOnly
X-Request-Id: 7153501b-8d5f-4421-aa94-e6d0474ee1ea
X-Runtime: 0.030097

3f
This is a test for mbedtls sample.
Congratulations, it worked!
0


[20466] I/mbedtls: MbedTLS connection close success [mbedtls_client_test_entry][172]
```
