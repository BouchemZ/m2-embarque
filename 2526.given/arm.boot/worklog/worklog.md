# Introduction

---

This is **your worklog** when discovering programming for embedded systems. 
It should contain what you will need to reopen this project in 6 months 
or a year and be able to understand and evolve it.

Also, as an Appendix, there should be the necessary details about what is
new to you, something like a cheat sheet in some sense. Something you can 
go back to when your memory will not server you well.

---

Bare-metal programming, C + Assembly directement sur hardware, pas d'OS ou de librairie lourde pour faire les choses à notre place, tout est fait maison

La cross compilation consiste compiler du code en binaire executable pour une autre machine.

linker script: divisé en section (habituellement text pour le code, data pour les variable initialisé, bss pour tout ce qui est d'initialisé à zéro et le stack dont ont débinit la taille et 'grandi' vers le bas) décrit comment on va load notre code sur la board.

Tout doit être compatible avec la board, là ou les section sont load, là où les sections sont compiler, la mémoire utilisé en tout, et la board memory map.

Reset exception en 0x0000.

phase linkin entre .o

.elf  executable linked file

Plutot que d'avoir à utilisé une board avec un serial line pour on va utilisé QEMU.

QEMU est un logiciel de virtualisation open source qui permet l'émulation et la virtualisation de divers systèmes d'exploitation et applications sur différentes plateformes matérielles.

![](/home/zak/snap/marktext/9/.config/marktext/images/2026-01-24-14-20-07-image.png)

---

# BUILD

**Question:** what should the configuration of the GNU toolchain and the configuration of the QEMU be correlated?

On cherche à compiler de manière à ce que le code puisse tourner sur une machine emuler sur  QEMU, c'est donc naturel qu'on s'attende à ce qu'il y est une correlation entre la toolchain utiliser et la configuration de notre QEMU

**Question:** how are they correlated? What options need to be the same or similar?

On s'attend à ce que le CPU soit similaire

An crucial part of the makefile is the linker script.
Look at the correct linker script for the board `versatile`.

**Question:** explain the different sections

text:

tout ce qui est code

data:

variable intialisé et globale

bss:

zone memoir qui dois être  mis à zero

stack:

0x1000 apres la fin de bss, soit 4Kb, le pointeur stack 'descend' vers la region bss avec l'utilisation.

**Question:** explain why the linker script is "versatile" specific? 

tout linker script est board spécifique car il y a besoin de savoir la memorymap soit là où on s'attend à ce que certaine mémoir soit. Par exemple le trap ou la est attendu à 0x0 dans notre cas et ne marcherai pas nécessairement avec d'autre carte et processeur

**Question:** make sure they are compatible with the memory map

tout est bien dans SRAM qui est n'est pas réservé donc compatible

Go check the memory map in the documentation:

- Versatile Application Baseboard for ARM926EJ-S User Guide
- Programmer’s Reference, section 4.1, « Memory Map » (page 140)

**Question:** is the amount of SDRAM memory in the makefile correct?

SDRAM 0x00000000 à 0x08000000 = 134217728 bytes soit 131072 KB

Dans le makefile on précise vouloir utilisé 16KB, ça rentre

**Question:** are the sections in the right place?

je suppose? ils sont dans le bonne ordre en tout cas ? tant que le text+data+bss+stack ne dépasse pas la capacité de la SDRAM

**Question:** why are the different flags given to the compiler?

Car on cherche à compiler pour une machine qui n'est pas la machine hôte dans on a besoin d'être spécifique.

In particular:

- what are these flags?
   -D$(CPU) on précise quel est le processeur cible, soit celui qu'on retrouvera sur la board
  -DMEMORY précise la mémoire disponnible pour le programmme sur la board, ici c'est 16KB

```make
-D$(CPU) -DMEMORY="($(MEMSIZE)*1024)
```

- what are these flags?
  -nostdlib indique aux compilateur de ne pas utilisé les librarie standard C et C++
  -ffreestanding (d'après internet) indique aux compilateur qu'on sera sur environnement freestanding donc il faudra ne pas supposer l'existence des librairie standard ou qu'elles ont leur definition habituelle
  
  ```make
  -nostdlib -ffreestanding
  ```

- what are these flags
  
  d'après le makefile lui meme, ça demande à gcc de génere des dependances accurates
  
  ```make
  -MT $@ -MMD -MP -MF $(BUILD)/$*.d
  ```

- what is the line?
  
  inclue tout les gcc-generated dependencies
  
  ```make
  -include $(wildcard $(BUILD)/*.d)
  ```

**Question:** why are the different flags given to the linker?

-nostdlib pour les même raison que le compilateur, il ne faut pas s'attendre à ce qu'il y est les librairies standardes C C++

-static tout est linké statiquement

```make
  -nostdlib -static
```

Pour partir de zero et en pas ajouté les librairie standard

**Question:** what is the "-g" flag given to both the compiler and linker?

-g produit des information de debuggage qui sont compatibles avec gdb

**Question:** is the "-g" flag necessary to execute the code?

Non

---

# EXECUTION

### Debug essentials

Dans un terminal 

```
make debug
```

ou

```
$(QEMU) -M $(MACHINE) -cpu $(QCPU) -m $(MEMORY) $(VGA) $(SERIAL) -device loader,file=$(BUILD)/kernel.elf -gdb tcp::1234 -S
```

Dans l'autre

```
gdb-multiarch build/versatile/kernel.elf
```

puis on se connecte à la session debug qemu

```
(gdb) target remote localhost:1234
```

**step permet de "plonger" dans les fonctions appeler pour les exécuter pas à pas alors que next "saute l'interieure de la fonction**

| Commande   | Signification                            |
| ---------- | ---------------------------------------- |
| break      | break function_name                      |
| next       | avance pas à pas C                       |
| nexti      | avance pas à pas Assembleur              |
| step       | avance pas à pas C                       |
| stepi      | avance pas à pas Assembleur              |
| finish     | retourne à l'appeleur                    |
| continue   | reprendre l'exectution (jusqu'au next b) |
| ctrl x + a | code + pointeur                          |

### BOOT SEQUENCE

**Question:** did you understand the exception vector?

    .equ    CPSR_USR_MODE,       0x10
    .equ    CPSR_FIQ_MODE,       0x11
    .equ    CPSR_IRQ_MODE,       0x12
    .equ    CPSR_SVC_MODE,       0x13
    .equ    CPSR_ABT_MODE,       0x17
    .equ    CPSR_UND_MODE,       0x1B
    .equ    CPSR_SYS_MODE,       0x1F
    
    .equ    CPSR_IRQ_FLAG,         0x80      /* when set, IRQs are disabled, at the core. */
    .equ    CPSR_FIQ_FLAG,         0x40      /* when set, FIQs are disabled, at the core. */

Définition de NOM, VALEUR

Il y a une partie pour les modes, par exemple le code symbolisant qu'on est en usermode est 0x10

Puis une partie pour les flags, par exemple pour verifier si les IRQs sont disabled on fait un & logique avec CPSR_IRQ_FLAG (c'est à dire 0x80)

**Question:** did you understand the BSS section initialization?

```
.clear:
    ldr    r4, =_bss_start
    ldr    r9, =_bss_end
    mov    r5, #0
1:
    stmia    r4!, {r5} 
    cmp    r4, r9
    blo    1b
```

entre autre, on recupere le debut et la fin du bss et on met le courrant (r4) au debut. On ecrit 0, et on increment, on verifie si on est à la fin, sinon on refait.

**Question:** did you understand why there is a stack and where it is and
how it grows?

dans le linkerfile il etait deja écrit qu'on avait une stack de 4 Kb qui grandit vers le bas, c'est à que le bottom de la stack et 4kb apres la fin de la bss et que le top de la stack elle se rapproche de la bss, entre autre.

### MAIN LOOP

```c
#undef ECHO_ZZZ
```

```c
 while (1) {
    uint8_t c;
    if (0==uart_receive(UART0,&c))
      continue;
    if (c == 13) {
      uart_send(UART0, '\r');
      uart_send(UART0, '\n');
    } else {
      uart_send(UART0, c);
    }
  }
```

Not looking at the implementation in the source file `uart.c`,
but looking at the header file `uart.h`, let's explain this loop.

**Question:** explain the above loop.

boucle infini,

on cherche à recevoir un byte depuis UART0 que l'on stock dans c, si pas de byte on reessaye (spin)

si on recois un byte, si c'est 13 alors on send '\r' puis '\n' dans UART0.

                                    sinon on send c 

**Question:** what are the bytes flowing back and forth through the UART0?

périfiphérique d'entrée de la carte qui est très probablement connecté aux touches sur lesquel on appui dans le clavier

```c
#define ECHO_ZZZ
```

```c
 while (1) {
    uint8_t c;
    while (0 == uart_receive(UART0, &c)) {
      count++;
      if (count > 50000000) {
        uart_send_string(UART0, "\n\rZzzz....\n\r");
        count = 0;
      }
    }
    if (c == 13) {
      uart_send(UART0, '\r');
      uart_send(UART0, '\n');
    } else {
      uart_send(UART0, c);
    }
  }
```

**Question:** explain the above loop.

boucle infini,

on boucle sur la reception à UART0, tant qu'on a pas reçu un byte on incremente count, toutes les 50000000 incrementation ça send ZZZZZ

si on recois un byte, si c'est 13 alors on send '\r' puis '\n' dans UART0.

                                    sinon on send c 

**Question:** what are the bytes flowing back and forth through the UART0?

Let's continue with `ECHO_ZZZ` as **defined**.

**Question:** what is the printing of "Zzzz...." telling you?

ça nous dis qu'il n'y a pas eu de reception de byte, mais surtout ça nous montre que la machine ne s'est pas 'endormi' qu'elle est tout le temps en execution

---

# DEBUGGING

- start a debug session

- set a breakpoint in `_start`

- step in the function `check_stacks`

- verify the addresses `max` and `addr`, printing them with `gdb`

```
make debu
```

```
gdb-multiarch build/versatile/kernel.elf
target remote localhost:1234
b _start
```

(gdb) print max
$2 = (void *) 0x4000

(gdb) print addr
$3 = (void *) 0x23b0

```
zak@zak-IdeaPad-1-15ALC7:~/College_Work/embarque/gruber/2526.given/arm.boot/build/versatile$ ls -l kernel.*
-rwxrwxr-x 1 zak zak  5025 Feb  2 14:16 kernel.bin
-rwxrwxr-x 1 zak zak 14668 Feb  2 14:16 kernel.elf
```

**Question:** explain why the `kernel.elf` is larger than the `kernel.bin`

elf : executable linkable format files

generer après la linking phase donc tout les symboles ont été résolu et assemble tout les .o

bin: binary files

juste du code

**Question:** explain the code in the function `check_memory`.

on recupère la taille utilisé qui été definie dans le makefile MEMORY soit 16kb donc 0x4000

on recupère là ou pointe stack_top, c'est à dire la fin de region memoire qui nous est reserve, qui lui été defini dans le linker file.

si stack_top > max, alors le top de notre stack dépasse les 16Kb qui nous était attribué de base, donc on panique

On veut rajouter kprintf, pour ça je change ma target dans le make file de cette manière

```
# Object files to build and link together
objs= exception.o startup.o main.o uart.o kprintf.o
```

```
zak@zak-IdeaPad-1-15ALC7:~/College_Work/embarque/gruber/2526.given/arm.boot$ ls -l build/versatile/kernel*
-rwxrwxr-x 1 zak zak 11675 Feb  2 14:48 build/versatile/kernel.bin
-rwxrwxr-x 1 zak zak 26736 Feb  2 14:48 build/versatile/kernel.elf
```

Le code ne run plus, à priori un problème de mémoire, on a va donc rajouter de l'espace en faisant MEMORY = 32 dans le makefile

Maintenant ça marche

```
(gdb) p max
$5 = (void *) 0x4000
(gdb) p addr 
$6 = (void *) 0x3da0
```

One of the main difference when debugging an embedded system
is with respect to faulty addresses, either manipulating data 
or executing code. Invalid memory accesses 
do raise ***hardware exceptions***, making the execution flow
go through the hardware exception vector at the address 0x0000-0000.

When does that happen?

- undefined instruction exception
  - the execution strays somewhere where it is not valid assembly code,
    but the address range is valid memory
  - the executed code has been trashed via faulty pointers
- prefetch abort
  - the execution strays in an invalid address range in memory
- data abort
  - a load or store operation tries to manipulate an invalid address in memory

On Linux, the operating system catches those exceptions and surface them through SEGV signal that GDB understands.

```
(gdb) disassemble _start
Dump of assembler code for function _start:
   0x000010e0 <+0>:    push    {r11, lr}
   0x000010e4 <+4>:    add    r11, sp, #4
   0x000010e8 <+8>:    sub    sp, sp, #24
   0x000010ec <+12>:    bl    0x10a0 <check_memory>
   0x000010f0 <+16>:    movw    r3, #48879    @ 0xbeef
   0x000010f4 <+20>:    movt    r3, #57005    @ 0xdead
   0x000010f8 <+24>:    str    r3, [r11, #-12]
   0x000010fc <+28>:    ldr    r3, [r11, #-12]
   0x00001100 <+32>:    ldr    r3, [r3]
   0x00001104 <+36>:    str    r3, [r11, #-16]
   0x00001108 <+40>:    movw    r1, #11508    @ 0x2cf4
   0x0000110c <+44>:    movt    r1, #0
   0x00001110 <+48>:    mov    r0, #4096    @ 0x1000
   0x00001114 <+52>:    movt    r0, #4127    @ 0x101f
   0x00001118 <+56>:    bl    0x12e0 <uart_send_string>
   0x0000111c <+60>:    movw    r1, #11528    @ 0x2d08
   0x00001120 <+64>:    movt    r1, #0
   0x00001124 <+68>:    mov    r0, #4096    @ 0x1000
   0x00001128 <+72>:    movt    r0, #4127    @ 0x101f
   0x0000112c <+76>:    bl    0x12e0 <uart_send_string>
   0x00001130 <+80>:    movw    r1, #11580    @ 0x2d3c
   0x00001134 <+84>:    movt    r1, #0
```

disassemble func donne la version assembleur de la function donné

---

# First Sprint

- [Setup](./setup.md)
- [Understanding the build](./build.md)
- [Understanding the execution](./execution.md)
- [Advanced debugging](./debugging.md)

---

# Second week

## Console

Je faisais (probablement mal) plusieurs appels de uart_receive lors du démarrage d'un esc sequence, ça mener plusieurs fois à des 'fuites' qui faiait que je devrais l'un des bytes de la sequence comme un byte classique printable et je le printer dans le terminal.

J'ai changé pour un suivi de l'état, il n'y a plus de uart_receive en dehors de celui appelé en avant le console_echo. chaque appel de console_echo est donc plus court dans le sens ou j'ai principale des switch et de if avec une profondeur basse.
