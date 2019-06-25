# V-2406C system package

## base-system
Base system for V-2406C.
It contains the necessary tools for setting up V-2406C

## modules
V-2406C standard kernel modules.
The Linux kernel modules for use on V-2406C

## firmware
V-2406C platform firmware files.
The Linux firmare files for use on V-2406C
### ath10k for wifi module
1. copy from debian pakcage firmware-atheros (20161130-5) under stretch dist
2. change the name from firmware-4.bin to firmware-5.bin in `ath10k/QCA6174/hw3.0` Folder
3. copy `ath10k/QCA6174/hw3.0/board.bin` --> `ath10k/pre-cal-pci-0000:05:000.bin`
4. copy `ath10k/QCA6174/hw3.0/board-2.bin` --> `ath10k/cal-pci-0000:05:000.bin`

### qca for BT module
1. copy from debian pakcage firmware-atheros (20161130-5) under stretch dist
2. copy `qca/nvm_usb_00000302.bin`, `qca/rampatch_00130302.bin`, `qca/rampatch_usb_00000302.bin` to `/lib/firmware/qca`

## source
V-2406C platform driver source files.
including watchdog, misc drivers
