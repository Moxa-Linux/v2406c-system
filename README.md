# V-2406C system package

## base-system
Base system for V-2406C.
It contains the necessary tools for setting up V-2406C

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
### Intel "Kabylake" DMC firmware
1. copy from debian pakcage firmware-misc-nonfree (20190114-1~bpo9+2) non-free under stretch-backports
2. copy `i915/kbl_dmc_ver1_01.bin` to `/lib/firmware/i915`

## moxa-configs
The platform-related configuration items are recoreded in this file,
these items will be referred to other platform packages.
