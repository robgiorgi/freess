# freess
RISC-V Superscalar Educational Simulator based on Tomasulo's Algorithm

# Tutorial/Paper
[This paper has been published at the Workshop on Computer Architecture Education 2025 (at ISCA-2025).](https://doi.org/10.48550/arXiv.2506.07665)
Please cite this work via this reference:
```
@InProceedings{Giorgi25-wcae,
  author = {Giorgi, Roberto},
  title = {{FREESS}: An Educational Simulator of a {RISC-V}-Inspired Superscalar Processor Based on Tomasulo’s Algorithm},
  booktitle = "ACM Workshop on Computer Architecture Education (WCAE-25)",
  address = "Tokyo, Japan",
  pages = "1-8",
  month = "jun",
  year = "2025"
}
```

![FREESS Processor Architecture](processor_diagram.png "Block diagram of the processor")

# pre-requisites (dependencies in Ubuntu)
```
sudo apt install build-essential git vim
```
# pre-requisites (dependencies in Fedora)
```
sudo dnf install @development-tools git vim
```
# to compile
```
make
```
# to check that it works
```
make test
```
It should print something like '* TEST1 PASSED ...'

# to run the examples
```
./run-ex1.sh
./run-ex2.sh
./run-ex3.sh
```

# to write your own RISC-V-like program
To write a program, students simply replace each mnemonic with its corresponding opcode, specify the register indices, and provide any required immediate values. The handling of branch instructions is particularly instructive: instead of using labels as in assembly, students must enter the immediate value representing the number of instructions to jump—positive for forward branches, negative for backward ones, effectively replacing the role of labels in `BEQ` and `BNE` instructions. For those unfamiliar with machine code, this exercise also provides a valuable opportunity to understand how binary files encode programs at the instruction level.

Table 1 reports the opcodes for the seven supported instructions.

**Table 1: FREESS instructions**

| Mnemonic | Operation Code (opcode) |
|----------|-------------------------|
| LW       | 1                       |
| SW       | 2                       |
| BEQ      | 3                       |
| BNE      | 4                       |
| ADD      | 5                       |
| ADDI     | 6                       |
| MUL      | 7                       |

Assuming the code:
```
loop: x3 <- mem(0+x4)   # load b(i)
      x7 <- mem(128+x5) # load c(i)
      x7 <- x7 * x3     # b(i) * c(i)
      x1 <- x1 - 1      # decr. counter
      mem(256+x6)<- x7  # store a(i)
      x2 <- x2 + 8      # bump index
      P <- loop; x1!=0  # close loop
```
the resulting code is the following (i.e., filename 'program1':

```
1 3 4 0
1 7 5 128
7 7 7 3
6 1 1 -1
2 7 6 256
6 2 2 8
4 1 0 -7
```

# running the program:
```
./freess -exe program1
```

# for help
```
./freess -h
```

![Main Processor Parameters](parameters.png "Main parameters of the FREESS processor")

# NEW (wasm branch)
In addition here is a WebAssembly version!

# Prerequistes: same as above and additionally (if you want to re-generate the WebAssembly via EMSCRIPTEN)
```
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
<follow instructions on the screen to setup your enviroment variables>
```

# to compile
```
cd freess
git checkout wasm
make wasm
```

# to run the examples
Copy index.html freess.js freess.wasm to your web server and just access it via a web browser.
or you can use it directly from here [https://robgiorgi.github.io/freess/](https://robgiorgi.github.io/freess/)

# WARNING
This is work in progress: if you have comments or would like to contribute, you are welcome!

