# WindCloud Linux 企业级云存储系统 (Enterprise Cloud Storage)
A high-performance cloud storage system on Linux featuring breakpoint resume, instant upload, and virtual file system.

> 一个基于 C 语言开发的高性能 Linux 云盘后端系统，支持秒传、断点续传及虚拟文件系统升级。

## 📅 项目开发进度 (Gantt Chart)

```mermaid
gantt
    title 企业上云项目 - 10日实战开发计划
    dateFormat  YYYY-MM-DD
    axisFormat  %m-%d

    section 开发阶段
    用户注册与登录验证 (核心鉴权)    :done,   task1, 2026-04-17, 3d
    日志记录与断点续传逻辑 (文件I/O) :active, task2, 2026-04-17, 4d
    服务器虚拟文件表升级 (存储结构)   :active, task3, 2026-04-17, 3d
    Hash秒传功能实现 (算法校验)      :         task4, 2026-04-17, 2d
    多点下载功能开发 (网络优化)      :         task5, 2026-04-17, 3d
    项目联调与演示准备 (结项)        :         task6, 2026-04-27, 2d

    section 团队任务认领
    组员 A :active, a1, 2026-04-17, 10d
    组员 B :active, b1, 2026-04-17, 10d
    组员 C :active, c1, 2026-04-17, 10d
    组员 D :active, d1, 2026-04-17, 10d
    组员 E :active, e1, 2026-04-17, 10d
    组员 F :active, f1, 2026-04-17, 10d

🚀 项目简介
本项目是针对企业级文件存储需求开发的 Linux 后端系统。核心目标是解决海量文件存储中的数据冗余（通过秒传）和传输稳定性（通过断点续传）问题。项目深度应用了 Linux 系统编程、多线程/进程协作及高并发网络通信技术。

🛠️ 技术栈
核心语言: C (C99/C11)

操作系统: Linux (Ubuntu / WSL2)

开发工具: VS Code (Remote-SSH), Vim, Git

关键技术:

网络通信: TCP Socket, Epoll 高并发 IO 多路复用

文件处理: mmap 内存映射, 引用计数算法

数据校验: MD5/SHA-256 摘要算法

数据库: MySQL

🌟 核心功能说明
用户系统: 负责用户注册、登录态保持及权限验证。

断点续传: 在传输中断后，通过记录已传输偏移量，支持从断点位置恢复。

秒传功能: 提取文件唯一 Hash 特征，若服务器已存在相同文件，则通过“逻辑指向”实现瞬间上传。

虚拟文件系统 (VFS): 升级服务器目录结构，实现用户视图路径与物理存储路径的解耦。

多点下载: 优化下载机制，支持多连接并行下载提升速率。

📂 目录结构预览（示例）
Plaintext
.
├── src/            # 源代码文件 (.c)
├── include/        # 头文件 (.h)
├── bin/            # 编译生成的二进制程序 (已过滤)
├── build/          # 构建中间产物 (已过滤)
├── storage/        # 用户文件物理存放区 (已过滤)
├── Makefile        # 项目自动化编译脚本
└── README.md       # 项目文档
👥 团队分工
余文嘉: 负责模块 1 - 待定。

胡洪谅: 负责模块 2 - 待定。

李文硕: 负责模块 3 - 待定。

陈昌杰: 负责模块 4 - 待定。

闫思宏: 负责模块 5 - 待定。

王子康: 负责模块 6 - 待定。

⚙️ 快速构建
Bash
# 1. 克隆代码
git clone [https://github.com/你的用户名/仓库名.git](https://github.com/你的用户名/仓库名.git)

# 2. 编译
make

# 3. 运行服务器
./bin/server
