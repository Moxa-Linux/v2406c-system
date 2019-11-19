# moxa-audio-retask

For soundcards retasking audio jack (e.g. ALC262)

## Usage
```bash
Usage:
        mx-audio-retask [Options]

Operations:
        -v,--version
                Show utility version
        -p,--port <port_index>
                Select Jack Index (1, 2, ...)
        -m,--mode <mode_index>
                Select Jack Mode Index (1 to 5)

Mode List:
        LINE_OUT=1
        SPEAKER=2
        HEADPHONE=3
        LINE_IN=4
        MICROPHONE=5

Example:
        Get audio jack 1 mode
        # mx-audio-retask -p 1
        Switch audio jack 2 to HEADPHONE mode
        # mx-audio-retask -p 2 -m 3
```

## Early Patching

### hdajackretask

```bash
$ apt-get install alsa-tools-gui
$ hdajackretask
```
From here, you'll then need to re-assign each required pin. Note that this tool, depending on your sound card, 
will most likely detect them by the color panel layout (see the back of your card and confirm 
if its' pins are color coded) or by the jack designator.

Either way, when you're done and you select "Apply", you'll need to reboot and the settings will apply on the next startup.

### Get pin configuration

If you select "Apply", The tool generates a firmware patch (under `/lib/firmware/hda-jack-retask.fw`)
entry that's also called up by a module configuration file (under `/etc/modprobe.d/hda-jack-retask.conf` or similar), 
whose settings are applied on every boot. That's what the "boot override" option does, 
overriding the sound card's pin assignments on every boot. To undo this in the case the configuration is no 
longer needed, just delete both files after purging hdajackretask.

### Example for contents of the /lib/firmware/hda-jack-retask.fw

```bash
[codec]
0x10ec0262 0x00000000 0

[pincfg]
0x14 0x02211010
0x18 0x02811020
```

[codec]
The file needs to have a line `[codec]`.  The next line should contain three numbers indicating the codec vendor-id 
(0x10ec0262 in the ALC262 example), the codec subsystem-id (0x00000000) and the address (2) ofthe codec. 
The rest patch entries are applied to this specified codec until another codec entry is given. 
Passing 0 or a negative number to the first or the second value will make the check of the corresponding 
field be skipped.  It'll be useful for really broken devices that don't initialize SSID properly.

ALC262 (vendor id=10EC; device id=0262)

[pincfg]
After the `[pincfg]` line, the contents are parsed as the initial
default pin-configurations just like `user_pin_configs` sysfs above.
The values can be shown in user_pin_configs sysfs file, too.

### Example for ALC262 pin configuration

| Mode       | Configuration |
| ---------- | ------------- |
| Line out   | 0x01014010    |
| Headphone  | 0x02211010    |
| Speaker    | 0x90170150    |
| Line in    | 0x02811020    |
| Microphone | 0x03a19020    |

### Example for Contents of the /etc/modprobe.d/hda-jack-retask.conf

```bash
# This file was added by the program 'hda-jack-retask'.
# If you want to revert the changes made by this program, you can simply erase this file and reboot your computer.
options snd-hda-intel patch=hda-jack-retask.fw,hda-jack-retask.fw,hda-jack-retask.fw,hda-jack-retask.fw
```

### Reboot

After reboot, you can check soundcard status by `dmesg`

```bash
root@Moxa:/home/moxa# dmesg | grep hda
[    5.006609] snd_hda_intel 0000:00:1f.3: Applying patch firmware 'hda-jack-retask.fw'
[    5.007676] snd_hda_intel 0000:00:1f.3: firmware: direct-loading firmware hda-jack-retask.fw
[    5.029772] snd_hda_intel 0000:00:1f.3: bound 0000:00:02.0 (ops i915_audio_component_bind_ops [i915])
[    5.070354] snd_hda_codec_realtek hdaudioC0D0: autoconfig for ALC262: line_outs=1 (0x14/0x0/0x0/0x0/0x0) type:hp
[    5.070355] snd_hda_codec_realtek hdaudioC0D0:    speaker_outs=0 (0x0/0x0/0x0/0x0/0x0)
[    5.070356] snd_hda_codec_realtek hdaudioC0D0:    hp_outs=0 (0x0/0x0/0x0/0x0/0x0)
[    5.070357] snd_hda_codec_realtek hdaudioC0D0:    mono: mono_out=0x0
[    5.070357] snd_hda_codec_realtek hdaudioC0D0:    inputs:
[    5.070358] snd_hda_codec_realtek hdaudioC0D0:      Line=0x18
```

## Reference

[More Notes on HD-Audio Driver](https://www.kernel.org/doc/html/v4.17/sound/hd-audio/notes.html)

[High Definition Audio Specification](https://www.intel.com/content/dam/www/public/us/en/documents/product-specifications/high-definition-audio-specification.pdf)

[Setup on Linux with a Realtek ALC898 sound card](https://gist.github.com/Brainiarc7/8ff198a5ac3f0050f68795233c4866d0)

## Config Example

### Path

```
/etc/moxa-configs/moxa-audio-retask.conf
```

### Description

* `CONFIG_VERSION`: The version of config file
* `NUM_OF_JACK_PORTS`: The number of audio jack ports on this device
* `JACKS_PIN_ID`: The audio jack ports pin IDs
* `CODEC`: The audio codec value (value according to audio chip)
* `LINE_OUT`: The pin config for line out mode
* `SPEAKER`: The pin config for speaker mode
* `HEADPHONE`: The pin config for headphone mode
* `LINE_IN`: The pin config for line in mode
* `MICROPHONE`: The pin config for microphone mode

### Example: For V-2406C Platform

```
#
# Config file version
#
CONFIG_VERSION=1.0.0

#
# Platform configuration for audio jacks
#
NUM_OF_JACK_PORTS=2
JACKS_PIN_ID=("0x14" "0x18")

#
# Mode Pin Configuation Value
#

# Codec
CODEC="0x10ec0262 0x00000000 0"

# output modes
LINE_OUT="0x01014010"
SPEAKER="0x90170150"
HEADPHONE="0x02211010"

# input modes
LINE_IN="0x02811020"
MICROPHONE="0x03a19020"
```
