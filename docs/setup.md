# Toolchain Setup

這份文件說明如何重建本專案使用的 cross-compilation toolchain，並重新產生被 `.gitignore` 排除的 `src/`、`build/`、`opt/` 目錄。

目標是安裝出一套 `i686-elf` 工具鏈，讓 [meaty/Makefile](/home/paulboul/osdev/meaty/Makefile) 可以正常使用：

- `i686-elf-as`
- `i686-elf-gcc`
- `i686-elf-ld`

## 目錄用途

- `src/`: 上游原始碼與壓縮檔
- `build/`: binutils / gcc 建置中介目錄
- `opt/`: 安裝完成的 cross toolchain

這三個目錄都不納入版控，因為它們屬於可下載、可重建的內容。

## 1. 安裝系統依賴

以下以 Debian / Ubuntu 為例：

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  bison \
  flex \
  libgmp3-dev \
  libmpc-dev \
  libmpfr-dev \
  texinfo \
  xorriso \
  grub-pc-bin \
  qemu-system-x86
```

如果缺少這些套件，常見症狀會是：

- `configure` 失敗
- GCC 無法啟用必要數學函式庫
- `grub-mkrescue` 找不到
- `qemu-system-i386` 找不到

## 2. 建立工作目錄

在 repo 根目錄執行：

```bash
mkdir -p src build opt/cross
```

專案預設把 toolchain 安裝到：

```bash
$HOME/osdev/opt/cross
```

這是因為 [meaty/Makefile](/home/paulboul/osdev/meaty/Makefile) 目前寫死了：

```make
override PREFIX := $(HOME)/osdev/opt/cross/bin/i686-elf
```

如果你的 repo 不在 `$HOME/osdev`，有兩種做法：

1. 把 repo 放到 `$HOME/osdev`
2. 修改 [meaty/Makefile](/home/paulboul/osdev/meaty/Makefile) 的 `PREFIX`

## 3. 下載原始碼

進入 `src/`：

```bash
cd /home/paulboul/osdev/src
```

下載 binutils 2.42 與 GCC 14.1.0：

```bash
curl -LO https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.xz
curl -LO https://ftp.gnu.org/gnu/gcc/gcc-14.1.0/gcc-14.1.0.tar.xz
```

解壓縮：

```bash
tar -xf binutils-2.42.tar.xz
tar -xf gcc-14.1.0.tar.xz
```

完成後你會得到：

- `src/binutils-2.42/`
- `src/gcc-14.1.0/`

## 4. 取得 GCC 依賴套件

GCC 原始碼目錄通常需要先下載內含依賴：

```bash
cd /home/paulboul/osdev/src/gcc-14.1.0
contrib/download_prerequisites
```

這一步會把 GCC 所需的 GMP / MPFR / MPC / ISL 依賴準備好。

如果你已經透過系統套件提供這些函式庫，這一步仍然是常見且穩定的做法。

## 5. 建置 binutils

建立獨立建置目錄：

```bash
mkdir -p /home/paulboul/osdev/build/binutils
cd /home/paulboul/osdev/build/binutils
```

設定環境變數：

```bash
export TARGET=i686-elf
export PREFIX="$HOME/osdev/opt/cross"
export PATH="$PREFIX/bin:$PATH"
```

執行 configure / make / install：

```bash
/home/paulboul/osdev/src/binutils-2.42/configure \
  --target=$TARGET \
  --prefix="$PREFIX" \
  --with-sysroot \
  --disable-nls \
  --disable-werror

make
make install
```

完成後應該會看到：

- `opt/cross/bin/i686-elf-as`
- `opt/cross/bin/i686-elf-ld`
- 其他 binutils 工具

## 6. 建置 GCC

建立 GCC 建置目錄：

```bash
mkdir -p /home/paulboul/osdev/build/gcc
cd /home/paulboul/osdev/build/gcc
```

保留前面的環境變數：

```bash
export TARGET=i686-elf
export PREFIX="$HOME/osdev/opt/cross"
export PATH="$PREFIX/bin:$PATH"
```

執行 configure：

```bash
/home/paulboul/osdev/src/gcc-14.1.0/configure \
  --target=$TARGET \
  --prefix="$PREFIX" \
  --disable-nls \
  --enable-languages=c,c++ \
  --without-headers
```

建置並安裝 GCC 本體與 target `libgcc`：

```bash
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
```

完成後應該會有：

- `opt/cross/bin/i686-elf-gcc`
- `opt/cross/bin/i686-elf-g++`
- `opt/cross/lib/gcc/i686-elf/.../libgcc.a`

## 7. 驗證 toolchain

檢查工具是否存在：

```bash
$HOME/osdev/opt/cross/bin/i686-elf-gcc --version
$HOME/osdev/opt/cross/bin/i686-elf-as --version
$HOME/osdev/opt/cross/bin/i686-elf-ld --version
```

也可以確認 target：

```bash
$HOME/osdev/opt/cross/bin/i686-elf-gcc -dumpmachine
```

預期輸出：

```text
i686-elf
```

## 8. 建置並啟動 kernel

toolchain 準備好後：

```bash
cd /home/paulboul/osdev/meaty
make
make run
```

這會產生：

- `meaty/arch/i386/boot.o`
- `meaty/kernel/kernel.o`
- `meaty/mykernel.bin`
- `meaty/myos.iso`

然後用 QEMU 啟動 ISO。

## 9. 清理方式

如果要重做 `meaty` 的建置產物：

```bash
cd /home/paulboul/osdev/meaty
make clean
```

如果要整個重建 toolchain，可以直接刪除忽略目錄後從頭再跑：

```bash
rm -rf /home/paulboul/osdev/build
rm -rf /home/paulboul/osdev/opt
rm -rf /home/paulboul/osdev/src/binutils-2.42
rm -rf /home/paulboul/osdev/src/gcc-14.1.0
```

壓縮檔要不要保留可自行決定。

## 10. 常見問題

`grub-mkrescue: command not found`

安裝 `grub-pc-bin` 與 `xorriso`。

`i686-elf-gcc: command not found`

通常代表：

- toolchain 尚未安裝完成
- `PREFIX` 路徑不對
- repo 不在 `$HOME/osdev`

`ld: cannot find -lgcc`

代表 GCC 雖然可能建好了，但 `libgcc` 沒有正確安裝。重新執行：

```bash
cd /home/paulboul/osdev/build/gcc
make all-target-libgcc
make install-target-libgcc
```

`make run` 無法啟動

先確認：

- `qemu-system-i386` 已安裝
- `meaty/myos.iso` 已成功產生
- [meaty/iso/boot/grub/grub.cfg](/home/paulboul/osdev/meaty/iso/boot/grub/grub.cfg) 存在

## 11. 建議

如果你之後要讓別人更容易重建環境，下一步建議補一份自動化腳本，例如：

- `scripts/build-toolchain.sh`
- `Makefile` 的 `toolchain` target

這樣就能把這份文件中的手動步驟收斂成一條命令。
