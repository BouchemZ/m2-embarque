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

# Execution

# First Sprint

- [Setup](./setup.md)
- [Understanding the build](./build.md)
- [Understanding the execution](./execution.md)
- [Advanced debugging](./debugging.md)
