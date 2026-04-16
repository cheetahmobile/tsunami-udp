# 安装与部署指南

## 编译环境准备
```bash
# 安装依赖
brew install autoconf automake  # macOS
sudo apt-get install autoconf automake libtool # Linux

# Windows 需要 MSYS2 或 Cygwin
```

## 编译步骤
```bash
# 自动编译
./recompile.sh

# 手动编译
aclocal
automake --add-missing
autoconf
./configure
make
```