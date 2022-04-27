# BabyServer
simple cpp http server



## 目前支持的功能

参照更新日志



## Todo

- 分发函数`BabyHttp::DispatchProcess`
    - 解析http报文（message）的请求行（request line）

- URL分发处理
    - 阶段一：用一个map（可能经过hash）注册响应的函数
    - 阶段二：用一个树状map注册响应的函数
- 将默认页面改为文件而不是硬编码（在写完URL分发阶段一以后）
- 多线程处理（每个TCP连接开一个线程）



## 更新日志

- 2022-04-27
    - IPV4，单线程（同时只允许一个TCP连接），非持续连接
    - 响应任意URL的GET请求，默认返回`hello wolrd`，可自定义
    - 响应任意URL的非GET请求，返回一句话`Please use GET`




## 警告

- 有很多`size_t`到`int`的转换，因为对网络编程不是太熟悉，所以只是简单强制转换，有可能产生bug
- 有很多地方用了ascii，而不是unicode（既后缀为`A`而不是`W`的函数）

