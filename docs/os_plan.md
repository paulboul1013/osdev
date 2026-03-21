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


