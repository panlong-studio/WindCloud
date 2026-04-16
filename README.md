
# WindCloud - Linux 企业级云存储系统

> 一个基于 C 语言开发的高性能 Linux 云盘后端系统。本项目采用企业级协作规范，支持秒传、断点续传及虚拟文件系统升级。

## 📅 项目实时进度 (Gantt Chart)

```mermaid
gantt
    title 企业上云项目 - 10日实战开发计划
    dateFormat  YYYY-MM-DD
    axisFormat  %m-%d

    section 开发阶段
    用户注册与登录验证 (核心鉴权)    :done,   task1, 2026-04-17, 3d
    日志记录与断点续传逻辑 (文件I/O) :active, task2, 2026-04-19, 3d
    服务器虚拟文件表升级 (存储结构)   :active, task3, 2026-04-21, 3d
    Hash秒传功能实现 (算法校验)      :         task4, 2026-04-22, 2d
    多点下载功能开发 (网络优化)      :         task5, 2026-04-23, 3d
    项目联调与演示准备 (结项)        :         task6, 2026-04-24, 2d

    section 核心组员进度
    组员 A :active, a1, 2026-04-17, 3d
    组员 B :active, b1, 2026-04-17, 5d
    组员 C :active, c1, 2026-04-17, 6d
    组员 D :active, d1, 2026-04-17, 8d
    组员 E :active, e1, 2026-04-17, 9d
    组员 F :active, f1, 2026-04-17, 10d
```


------

## 🚀 项目简介

`WindCloud` 旨在模拟高性能云存储解决方案。通过 C 语言底层编程，实现文件上传下载、目录管理及高效的存储策略。本项目重点解决了海量存储中的**数据去重（秒传）**和不稳定网络环境下的**传输可靠性（断点续传）**。

## 🛠️ 技术栈

- **语言**: C (C99/C11)
- **环境**: Linux (Ubuntu)
- **网络**: 高并发 Epoll 模型, TCP Socket
- **数据库**: MySQL


## 🤝 团队协作与贡献规范 (PR-Only)

为保证代码质量，本项目强制开启 **Branch Rulesets** 保护。

### 1. 开发流程

1. **Sync**: 切换至 `main` 保持最新 `git pull origin main`。
2. **Branch**: 创建特性分支 `git checkout -b feature/your-task`。
3. **Push**: 推送至远程 `git push origin feature/your-task`。
4. **Pull Request**: 发起 PR，并邀请至少 **2位** 组员进行 Review。

### 2. 合并规则 (Squash Merge)

- 本仓库统一采用 **Squash and Merge** 模式。
- 合并时，特性分支的所有 commit 将会被压缩为一个整洁的记录。
- 请确保 PR 描述清晰，合并后的信息将直接作为 `main` 分支的正式变更日志。

## 📂 目录结构



```bash
.
├── src/            # 逻辑代码
├── include/        # 头文件
├── config/         # 配置文件
├── scripts/        # 自动化脚本
├── Makefile        # 构建脚本
└── .gitignore      # 环境过滤配置
```

## ⚙️ 快速上手



```bash
git clone [https://github.com/panlong-studio/WindCloud.git](https://github.com/panlong-studio/WindCloud.git)
make
./bin/server
```
