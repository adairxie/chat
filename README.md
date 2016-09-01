系统描述
--------
    使用C++实现， 运行在Linux下， 依赖mysqlclient库, 消息的编解码使用protobuf。
    客户端与服务器的通信采用TCP长连接。 服务器采用多线程异步模型，主线程接收新连接，
    子线程通过事件循环处理网络IO。新建TcpConnection时从event loop pool里挑选一个
    loop给TcpConnection用
体系结构
--------
  ![](http://images.cnitblog.com/blog/136188/201303/05095209-75f020e922c04c4695f43d2a7780577e.png)
详细说明
--------
    gate: 用来分配客户端的连接到不同的connector上，客户端先连接gate, gate告知客户端
          应该连接哪个connector, 并将connector的地址端口等信息发送给客户端，客户端
          据此连接指定的connector
    connector: 处理长连接的建立和维护，同时负责接收和发送信息
    chat :管理聊天室用户， 接收connector发送的消息， push消息到对应的connector
    mastter: master通过RPC调用来发送“启动/关闭/更新”进程的命令
    
编译
----
cd ./chat/excercise/chatServer<br>
make

运行服务器
---------
./server_threaded <port> <thread_nums> #启动服务器 port：服务器端口 thread_nums：线程池大小
运行客户端
---------
chatClient 需要在本地存储用户信息，user.info中的用户信息在用户首次注册时生成。
./chatClient <server_addr> <server_port> # server_addr —— 服务器地址 server_port —— 服务器端口
