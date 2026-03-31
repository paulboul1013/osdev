## 建立工作目錄與基本工具
```bash
sudo apt update
sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo \
                    grub-pc-bin xorriso qemu-system-x86 gdb mtools nasm git curl wget
```

### 檢查工具版本
```bash
qemu-system-i386 --version
gdb --version
xorriso --version
grub-mkrescue --version
```

### host

你**現在用來開發程式的那台電腦/環境**。
直覺上就是：**你人坐在哪台機器前面寫程式，哪台就是 host**。

例子：
你在 Windows 或 Linux 電腦上寫作業系統程式，這台電腦就是 host。

---

### target

你**要讓程式實際執行的目標平台**。
直覺上就是：**程式最後要跑去哪裡，那裡就是 target**。

例子：
你在 Windows 上編譯一個要跑在 x86 裸機上的小作業系統，
那個 x86 裸機就是 target。

---

### cross-compiler

**在 host 上執行，但產生給 target 用的機器碼的編譯器**。
直覺上就是：**人在甲地工作，做出給乙地使用的程式**。

例子：
你在 x86-64 Linux 上，用 `i686-elf-gcc` 編譯要跑在 32-bit x86 OS 上的核心。
這個 `i686-elf-gcc` 就是 cross-compiler。

---

### bootloader

**開機後最先執行、負責把作業系統載入記憶體並交出控制權的程式**。
直覺上就是：**它是作業系統的「開門員」或「搬運工」**。

它通常負責：

* 初始化最基本的執行環境
* 找到 kernel
* 把 kernel 載入記憶體
* 跳到 kernel 的入口點開始執行

常見例子：GRUB

---

### ELF

**Executable and Linkable Format**，是一種可執行檔/目標檔格式。
直覺上就是：**程式在磁碟上的一種標準包裝方式**。

它裡面通常會放：

* 程式碼（text section）
* 已初始化資料（data section）
* 未初始化資料（bss section）
* 符號表、重定位資訊等

在作業系統開發裡，kernel 常常先被編譯成 ELF，再由 bootloader 載入。

---

### 概念連接

你在 **host** 上，用 **cross-compiler** 編譯出給 **target** 執行的 ELF 格式 kernel，然後由 **bootloader** 在開機時把它載入並啟動。

要的話我也可以再幫你整理成一版 **「OS 開發術語對照表」**，用表格一次看懂。




## 建 i686-elf cross-compiler 的資料夾骨架
```bash
export PREFIX="$HOME/osdev/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

echo 'export PREFIX="$HOME/osdev/opt/cross"' >> ~/.bashrc
echo 'export TARGET=i686-elf' >> ~/.bashrc
echo 'export PATH="$PREFIX/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc

cd ~/osdev/src

curl -LO https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.xz
curl -LO https://ftp.gnu.org/gnu/gcc/gcc-14.1.0/gcc-14.1.0.tar.xz

tar xf binutils-2.42.tar.xz
tar xf gcc-14.1.0.tar.xz
```

### binutils

一組**底層二進位工具集合**，用來處理目標檔、可執行檔、符號表等。
直覺上就是：**幫你操作編譯結果的工具箱**。

常見工具有：

* `as`：assembler
* `ld`：linker
* `objdump`：反組譯、看 section
* `nm`：看符號
* `readelf`：看 ELF 結構

---

### gcc

GNU Compiler Collection，常被拿來指 **C 編譯器**。
直覺上就是：**把 C 程式一路變成機器碼的主工具**。

它實際上不只做一件事，常會幫你串起整條流程：

* 預處理
* 編譯
* 組譯
* 連結

所以你打 `gcc main.c -o main`，背後其實呼叫了很多步驟。

---

### assembler

**把組合語言轉成機器碼/目標檔** 的工具。
直覺上就是：**把人類較能讀的 asm，翻成 CPU 真正懂的格式**。

例子：

* 輸入：`mov eax, 1`
* 輸出：對應的機器碼與 `.o` 目標檔

GNU 的 assembler 通常是 `as`。

---

### linker

**把多個目標檔和函式庫接起來，變成一個完整可執行檔** 的工具。
直覺上就是：**把零件組裝成完整程式的人**。

它主要做：

* 合併不同 `.o` 檔
* 解決符號引用（例如 `main` 呼叫 `printf`）
* 安排程式在記憶體中的位置
* 產生最終 ELF 或 kernel 映像

GNU 的 linker 通常是 `ld`。

---

### freestanding

一種 C 執行環境，表示**程式不依賴一般作業系統提供的完整標準函式庫與啟動環境**。
直覺上就是：**你在一個「沒有作業系統幫忙」的世界自己站起來**。

在 OS 開發裡常見，因為：

* 沒有現成的 `main` 啟動流程
* 沒有完整 libc 可直接用
* 你要自己處理記憶體、輸出、啟動程式等

所以寫 kernel 時，通常是 **freestanding**，不是一般應用程式那種 **hosted** 環境。

---

### 概念連接

你用 `gcc` 編譯程式，過程中可能會用到 `binutils` 裡的 `assembler` 和 `linker`；而作業系統核心通常跑在 **freestanding** 環境，不是一般有完整 OS 支援的程式。



## 編譯 binutils
```bash
export PREFIX="$HOME/osdev/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

mkdir -p ~/osdev/build/binutils
cd ~/osdev/build/binutils

../../src/binutils-2.42/configure \
  --target=$TARGET \
  --prefix="$PREFIX" \
  --with-sysroot \
  --disable-nls \
  --disable-werror

make -j"$(nproc)"
make install

i686-elf-as --version
i686-elf-ld --version
```

繼續用 **「一句話定義 + 直覺理解」** 來精簡說明：

### object file

**編譯或組譯後產生的中間檔**，通常還不能直接執行。
直覺上就是：**程式的半成品零件**。

常見副檔名：

* `.o`（Linux / Unix）
* `.obj`（Windows）

它裡面通常有：

* 機器碼
* symbol
* section
* relocation 資訊

---

### symbol

程式裡有名字的東西，例如**函式名、全域變數名、標籤名**。
直覺上就是：**程式世界裡的「名字 → 對應位置/意義」**。

例子：

* `main`
* `printf`
* `kernel_main`

linker 會靠 symbol 來知道：
「這裡呼叫的函式，到底在哪裡？」

---

### section

object file 或 ELF 裡的**區塊分區**，把不同類型內容分開放。
直覺上就是：**把程式檔案切成不同用途的抽屜**。

常見 section：

* `.text`：程式碼
* `.data`：已初始化的全域/靜態變數
* `.bss`：未初始化的全域/靜態變數
* `.rodata`：唯讀資料，例如字串常數

---

### relocation

一種「**這個位址之後還要再修正**」的資訊。
直覺上就是：**先留一個空位，等 linker 最後補上正確地址**。

例子：
某個 `.o` 檔裡呼叫 `printf`，但現在還不知道 `printf` 最終位址。
那就先記一筆 relocation，之後 link 時再補。

---

### sysroot

交叉編譯時使用的一個**目標系統根目錄**，裡面放 target 的標頭檔、函式庫等。
直覺上就是：**模擬 target 系統檔案樹的資料夾**。

例如裡面可能有：

* `/usr/include`
* `/usr/lib`

這樣 compiler / linker 才知道要去找 **target 平台** 的 headers 和 libraries，
而不是誤用 host 的。

---

### 概念連接

原始碼先被編成 **object file**，裡面分成不同 **section**，包含各種 **symbol** 和需要之後修正地址的 **relocation**；交叉編譯時，工具鏈還可能透過 **sysroot** 去找 target 用的標頭與函式庫。

你要的話，我下一則可以直接幫你把目前所有術語整理成一份 **「作業系統開發術語總表（超精簡版）」**。

## 編譯 GCC
```bash
export PREFIX="$HOME/osdev/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

cd ~/osdev/src/gcc-14.1.0
./contrib/download_prerequisites

mkdir -p ~/osdev/build/gcc
cd ~/osdev/build/gcc

../../src/gcc-14.1.0/configure \
  --target=$TARGET \
  --prefix="$PREFIX" \
  --disable-nls \
  --enable-languages=c,c++ \
  --without-headers

make all-gcc -j"$(nproc)"
make all-target-libgcc -j"$(nproc)"
make install-gcc
make install-target-libgcc

i686-elf-gcc --version
i686-elf-g++ --version
```

### ABI

**Application Binary Interface**，是二進位層級的規則。
直覺上就是：**程式彼此怎麼在機器碼層面互相配合的約定**。

它會規定像是：

* 函式參數怎麼傳
* 回傳值放哪裡
* 哪些暫存器誰負責保存
* 資料型別大小與對齊方式
* 可執行檔格式怎麼配合

你可以把它想成：
**API 是原始碼層的規則，ABI 是編譯後二進位層的規則。**

---

### libgcc

GCC 附帶的一個**低階輔助函式庫**。
直覺上就是：**當 CPU 沒有直接支援某些操作時，GCC 幫你補上的工具包**。

例如可能提供：

* 大整數除法
* 乘法/除法輔助
* stack unwinding 的部分支援
* 某些編譯器內建運算的實作

在 OS 開發裡常會看到：
即使不用完整標準函式庫，還是可能需要連到 `libgcc`。

---

### runtime support

程式在**真正執行時所需要的底層支援**。
直覺上就是：**不是你業務邏輯本身，但程式要跑起來常常少不了的基礎設施**。

可能包含：

* 啟動程式碼
* stack / 記憶體初始化
* 編譯器需要的輔助函式
* 例外處理支援
* 全域建構/解構流程

簡單說：
**compile 是把程式做出來，runtime support 是讓它真的跑得動。**

---

### hosted

一種 C 執行環境，表示**程式跑在有作業系統與完整標準函式庫支援的環境**。
直覺上就是：**有大平台罩著你，你不是從零開始。**

通常代表：

* 可以用 `main()`
* 通常有完整 libc
* 可以用很多標準函式
* OS 幫你處理程序啟動等事情

一般使用者應用程式大多是 **hosted**。

---

### freestanding

一種 C 執行環境，表示**沒有保證完整作業系統與標準函式庫支援**。
直覺上就是：**你要自己搭很多地基。**

通常代表：

* 不一定有一般的 `main()`
* 不保證有完整 libc
* 很多初始化要自己做
* 常見於 kernel、bootloader、embedded

作業系統核心通常是 **freestanding**。

---

### hosted vs freestanding

最精簡地分：

* **hosted**：有作業系統幫你，像一般應用程式
* **freestanding**：幾乎要自己處理底層，像 kernel/bootloader

---

### 概念連接

**ABI** 決定二進位怎麼互相配合，`**libgcc**` 和其他 **runtime support** 提供執行時需要的底層支援；一般應用程式多半在 **hosted** 環境，而作業系統核心多半在 **freestanding** 環境。


## Bare Bones
```bash
mkdir -p ~/osdev/projects/barebones
cd ~/osdev/barebones

mkdir -p iso/boot/grub
touch boot.s kernel.c linker.ld grub.cfg Makefile
```

### multiboot header

給 **bootloader（像 GRUB）看的啟動資訊表**。
直覺上就是：**kernel 檔案開頭的一張「我是可開機核心」的身份證**。

它通常告訴 bootloader：

* 這是不是符合 Multiboot 規範的 kernel
* 需要哪些載入資訊
* 檢查用的 magic / flags / checksum

沒有它，GRUB 可能不知道怎麼正確載入你的 kernel。

---

### _start

程式真正開始執行的**入口點符號**。
直覺上就是：**CPU 進入你核心世界的第一個門口**。

在一般 C 程式中你常看到 `main`，
但在 OS / boot 階段，真正最先執行的通常不是 `main`，而是 `_start`。

它常做的事：

* 設 stack
* 做最早期初始化
* 呼叫 `kernel_main`

---

### stack

程式執行時用來暫存資料的一塊記憶體。
直覺上就是：**函式呼叫時臨時堆放東西的工作桌**。

stack 常拿來放：

* 函式返回位址
* 區域變數
* 暫存資料
* 傳遞參數的一部分

在 x86 中，stack 通常是**往低位址方向成長**。

---

### .bss

用來放**未初始化全域變數 / 靜態變數**的 section。
直覺上就是：**只先保留空間，但一開始不在檔案裡真的存內容的區域**。

例如：

* `int x;`
* 一大塊保留給 stack 的空間

特點：

* 可執行檔不需要真的把這些 0 全寫進去
* 載入時再把這塊區域設成 0 或保留空間

所以很適合拿來放：

* 大陣列
* kernel stack
* 未初始化資料

---

### linker script

告訴 **linker**「最終程式在記憶體和檔案裡要怎麼排」的腳本。
直覺上就是：**核心映像的施工藍圖**。

它會決定：

* `.text` 放哪裡
* `.data` 放哪裡
* `.bss` 放哪裡
* 入口點是誰（例如 `_start`）
* kernel 要被連結到哪個位址

在 OS 開發裡很重要，因為你不能只靠預設配置，
你通常必須精確控制 kernel 的記憶體布局。

---

### 概念連接

bootloader 先讀 **multiboot header** 確認這是可啟動的 kernel，接著從 **`_start`** 進入；`_start` 先設好 **stack**，而 stack 常常放在 **`.bss`** 裡保留空間；整個 kernel 各區段最後要放到哪裡，則由 **linker script** 決定。


## 第一次在 QEMU 看到字

```bash
make run
```

```bash
file mykernel.bin
readelf -h mykernel.bin
readelf -S mykernel.bin
nm mykernel.bin | head
```

### VGA text mode

一種老式螢幕顯示模式，直接用**文字格子**在畫面上顯示字元。
直覺上就是：**螢幕像一張固定大小的字表，每格放一個字和顏色**。

在 OS 教學裡常用它來做最早期輸出，因為很簡單：

* 不用圖形驅動
* 直接寫記憶體就能顯示文字
* 常見位置是 `0xB8000`

---

### entry point

程式開始執行的**起始位址/入口位置**。
直覺上就是：**CPU 一進程式時，先踩到的第一個點**。

在 kernel 裡常常是：

* `_start`

linker 會把這個入口資訊放進最終檔案裡，bootloader 或載入器再從那裡開始執行。

---

### ELF header

ELF 檔案最前面的**總表頭資訊**。
直覺上就是：**ELF 檔案的封面和目錄摘要**。

它會告訴工具或載入器：

* 這是不是 ELF
* 是 32-bit 還是 64-bit
* 給哪種 CPU 用
* 入口點在哪
* section / program header 在哪裡

---

### symbol table

記錄 **symbol 名字與相關資訊** 的表。
直覺上就是：**程式裡所有重要名字的通訊錄**。

裡面可能記：

* 函式名
* 全域變數名
* 位址
* 所屬 section
* 大小

工具像 `nm`、`objdump`、`readelf` 常會讀它。

---

### emulator

用軟體**模擬另一台機器/硬體環境** 的程式。
直覺上就是：**你電腦上的一台假電腦**。

在 OS 開發裡很常用，因為你可以：

* 不用每次都燒到真機
* 更容易除錯
* 快速測試 boot/kernel

常見例子：

* QEMU
* Bochs

---

### 概念連接

你的 kernel 會被包成 ELF，裡面有 **ELF header**、**symbol table** 和 **entry point**；啟動後可以先用 **VGA text mode** 輸出文字，而整個系統通常先放到 **emulator** 裡測試。


## 從最小範例走向可維護專案

### 整理目錄，開始 arch 與 include 分層

拆分成arch/i386，kernel，include，iso/boot/grub目錄

繼續用 **一句話定義 + 直覺理解** 來精簡說明：

### architecture-specific

**只適用某一種 CPU 架構的程式碼**。
直覺上就是：**這段程式只服務特定硬體，不是 everywhere 都能用。
例如：

* x86 的中斷設定
* ARM 的暫存器操作
* RISC-V 的啟動碼

這些都高度依賴 CPU 架構本身。

---

### generic code

**不依賴特定 CPU 架構、可重用的通用程式碼**。
直覺上就是：**能共用就共用，別綁死在某個硬體上。**

例如：

* 通用資料結構
* 排程器的一般邏輯
* 記憶體管理的高層抽象
* 字串處理工具

通常 OS 會把程式拆成：

* `arch/x86/...` 這種 architecture-specific
* `kernel/...`、`mm/...` 這種 generic code

---

### include path

編譯器在找 `#include` 標頭檔時會搜尋的路徑。
直覺上就是：**compiler 找 header 的搜尋路線圖**。

例如你寫：

```c
#include <stdio.h>
#include "kernel.h"
```

compiler 會去一串目錄裡找這些檔案，
那些目錄就是 include path。

常見設定方式：

* `-Iinclude`
* `-I/usr/local/include`

---

### translation unit

**一個 `.c` 檔加上它展開後所包含的所有 header 內容**。
直覺上就是：**編譯器一次真正看到並編譯的完整單位**。

例如：

```c
// main.c
#include "a.h"
#include "b.h"
int main() { return 0; }
```

經過前處理後，`main.c` + 展開的 `a.h`、`b.h`，
整包就是一個 translation unit。

簡單說：

* `.c` 檔不是單獨看的
* 是展開 include 後一起編譯

---

### kernel entry

**kernel 開始執行的入口位置**。
直覺上就是：**CPU 正式踏進 kernel 世界的第一步。**

它通常是一個符號，例如：

* `_start`

這裡常做的事：

* 設 stack
* 做最早期初始化
* 再跳去 `kernel_main`

它和一般應用程式的 `main` 不一樣，
因為 kernel entry 通常更底層、更早期。

---

### 概念連接

作業系統會把程式分成 **architecture-specific** 和 **generic code**；編譯時 compiler 會靠 **include path** 找 header，每個 `.c` 檔展開後形成一個 **translation unit**，最後從 **kernel entry** 開始執行。

## 把 Bare Bones 移植到新骨架

把barebones程式碼移植到meaty目錄中


### source tree

整個專案原始碼的**目錄結構**。
直覺上就是：**程式碼長成的一棵資料夾樹**。

例如 OS 專案常會長這樣：

```text
kernel/
mm/
drivers/
arch/x86/
include/
boot/
```

這整包目錄安排，就叫 source tree。

---

### separation of concerns

把不同責任的程式碼**拆開處理**，不要全部混在一起。
直覺上就是：**一個模組做一件主要的事**。

例如：

* boot 負責啟動
* mm 負責記憶體管理
* drivers 負責裝置
* scheduler 負責排程

這樣比較好讀、好改、好除錯。

---

### arch layer

作業系統中專門處理**特定 CPU/平台差異**的那一層。
直覺上就是：**把硬體相關細節集中隔離的一層**。

例如：

* x86 的中斷初始化
* ARM 的例外向量
* page table 格式差異

常見目錄像：

```text
arch/x86/
arch/arm64/
```

---

### interface

模組對外提供的**使用規則/入口**。
直覺上就是：**你可以怎麼用這個模組的門面**。

例如一個記憶體配置介面可能是：

```c
void *kmalloc(size_t size);
void kfree(void *p);
```

別的模組只需要知道怎麼呼叫，這就是 interface。

---

### implementation

真正把功能**做出來的內部程式碼**。
直覺上就是：**門面背後實際運作的內容**。

例如同樣是：

```c
void *kmalloc(size_t size);
```

interface 是這個函式宣告，
implementation 是它裡面到底怎麼分配記憶體。

---

### interface vs implementation

最精簡地分：

* **interface**：告訴你「怎麼用」
* **implementation**：告訴電腦「怎麼做」

---

### 概念連接

一個 OS 專案的 **source tree** 會按照 **separation of concerns** 來拆資料夾，其中 **arch layer** 專門放硬體相關部分；每個模組通常都分成 **interface** 和 **implementation**。

## meaty Makefile

可以，先給你一版 **OS 練習用的第一版 Makefile**，然後順手把今天這幾個術語一起記住。

---

### 第一版 Makefile

假設你的專案目前是這樣：

```text
boot.s
kernel.c
linker.ld
Makefile
```

那可以先寫成：

```makefile
# 工具鏈
CC = i686-elf-gcc
AS = i686-elf-as
LD = i686-elf-ld

# 參數
CFLAGS = -ffreestanding -m32 -Wall -Wextra -O2
ASFLAGS = --32
LDFLAGS = -T linker.ld -m elf_i386

# 目標檔
OBJS = boot.o kernel.o

# 最終輸出
TARGET = kernel.elf

all: $(TARGET)

boot.o: boot.s
	$(AS) $(ASFLAGS) boot.s -o boot.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET)

debug: $(TARGET)
	qemu-system-i386 -kernel $(TARGET) -s -S

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all debug clean
```

---

### 這份 Makefile 在做什麼

流程是：

1. `boot.s` 組譯成 `boot.o`
2. `kernel.c` 編譯成 `kernel.o`
3. `boot.o + kernel.o` 連結成 `kernel.elf`

也就是：

```text
boot.s   ----> boot.o
kernel.c ----> kernel.o
boot.o + kernel.o ----> kernel.elf
```

---


### dependency

**依賴項**。
就是：「我要做這個東西之前，先需要哪些檔案。」

例如：

```makefile
kernel.o: kernel.c
```

這裡的 `kernel.c` 就是 `kernel.o` 的 dependency。
意思是：

* `kernel.o` 依賴 `kernel.c`
* 如果 `kernel.c` 變了，就要重做 `kernel.o`

---

### target

**要生成的目標**。
也就是 rule 左邊那個東西。

例如：

```makefile
kernel.o: kernel.c
```

這裡的 `kernel.o` 就是 target。

再例如：

```makefile
all: $(TARGET)
```

這裡的 `all` 也是 target。

---

### rule

**一條建置規則**。
它通常長這樣：

```makefile
target: dependencies
	commands
```

例如：

```makefile
kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o
```

這整段就是一條 rule。

你可以把它理解成：

* 我要做出誰（target）
* 它依賴誰（dependencies）
* 要怎麼做（commands）

---

### debug target

專門拿來做**除錯用途**的 target。

例如：

```makefile
debug: $(TARGET)
	qemu-system-i386 -kernel $(TARGET) -s -S
```

這裡 `debug` 不是一個真正輸出的檔案，
而是一個「快捷指令 target」。

它的用途通常是：

* 啟動 QEMU
* 開 GDB stub
* 停在第一行等你 attach debugger

所以 `make debug` 就能快速進入除錯模式。

---

### reproducible build

**可重現建置**。
意思是：

* 同一份原始碼
* 同樣的工具鏈
* 同樣的參數
* 不管在哪台機器建置

都應該產生**相同或可預期一致**的結果。

直覺上就是：

**今天 build 一次，明天再 build，一樣的輸入就該有一樣的輸出。**

這很重要，因為它能幫你：

* 減少「我這台可以，你那台不行」
* 方便除錯
* 方便追蹤問題
* 讓 release 更可信

---

### 把 Makefile 跟術語對上去

這條：

```makefile
kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o
```

可以這樣拆：

* `kernel.o` → **target**
* `kernel.c` → **dependency**
* 整段三行 → **rule**

這條：

```makefile
debug: $(TARGET)
	qemu-system-i386 -kernel $(TARGET) -s -S
```

就是：

* `debug` → **debug target**

---

### 簡單記憶

你可以先背這句：

> Makefile 就是在寫：**我要做什麼(target)，它先需要什麼(dependency)，以及怎麼做(rule)。**

然後補一句：

> `debug target` 是方便除錯的快捷入口，`reproducible build` 是讓建置結果穩定可重現。

---


### 對照表

| 術語                 | 最短理解                                |
| ------------------ | ----------------------------------- |
| dependency         | 做某個 target 前先需要的東西                  |
| target             | 要生成的東西，或一個 make 指令入口                |
| rule               | target + dependency + command 的整套規則 |
| debug target       | 專門用來除錯的 make 目標                     |
| reproducible build | 同樣輸入能做出一致結果的建置                      |

---


## 補 Getting to C，理解從 assembly 進 C

在kernel.c中使用a=3,b=4,c=a+b; 來檢查stack是否正常運作看c的值是否為7


### calling convention

函式呼叫時，**參數怎麼傳、回傳值放哪、哪些暫存器誰負責保存** 的規則。
直覺上就是：**函式彼此溝通時共同遵守的交通規則**。

它通常規定：

* 參數放在 stack 還是暫存器
* 回傳值放哪個暫存器
* caller / callee 誰保存哪些暫存器

常見例子：

* cdecl
* stdcall
* System V ABI
* x86-64 calling convention

---

### prologue

函式一進來時先做的**開場準備動作**。
直覺上就是：**函式開始工作前，先把自己的桌面整理好**。

常見內容：

* 保存舊的 base pointer
* 建立新的 stack frame
* 挪出區域變數空間
* 保存需要保留的暫存器

x86 常見長這樣：

```asm
push %ebp
mov %esp, %ebp
sub $N, %esp
```

---

### epilogue

函式結束前做的**收尾還原動作**。
直覺上就是：**函式下班前，把借來的東西放回去**。

常見內容：

* 還原 stack
* 還原保存過的暫存器
* 恢復舊的 base pointer
* 返回呼叫者

x86 常見長這樣：

```asm
mov %ebp, %esp
pop %ebp
ret
```

---

### packed struct

一種要求編譯器**不要幫 struct 成員自動補 padding 對齊空間** 的結構。
直覺上就是：**把欄位緊緊排在一起，不留空隙**。

用途常見於：

* 磁碟格式
* 網路封包
* 硬體暫存器格式
* binary file format

例如：

```c
struct __attribute__((packed)) Header {
    char a;
    int b;
};
```

這樣 `a` 和 `b` 中間不會自動補齊。

但要注意：

* 存取可能變慢
* 某些架構上未對齊存取可能出錯

---

### stack frame

某一次函式呼叫在 stack 上用到的那一塊區域。
直覺上就是：**每個函式呼叫都有自己的一小塊工作區**。

裡面通常可能放：

* 返回位址
* 舊的 base pointer
* 區域變數
* 暫存資料
* 某些參數

可以把它想成：

* 呼叫一個函式，就在 stack 上開一個 frame
* 函式結束，就把這個 frame 收掉

---

### 這幾個怎麼串起來

當你呼叫一個函式時，會遵守 **calling convention**；
函式進來先做 **prologue**，建立自己的 **stack frame**；
如果 struct 要完全貼齊格式，可能會用 **packed struct**；
最後函式結束前再用 **epilogue** 收尾並返回。

---

### 最短記憶版

* **calling convention**：函式呼叫規則
* **prologue**：函式開場準備
* **epilogue**：函式結束收尾
* **packed struct**：不補 padding 的 struct
* **stack frame**：一次函式呼叫的 stack 工作區
