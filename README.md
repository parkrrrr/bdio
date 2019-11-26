# BDIOHID

## Introduction

BDIOHID is a set of programs that allow older Refreshable Braille Display hardware to work with newer software and services 
designed to support the USB HID Braille standard. 

BDIOHID runs on a small Linux Single-Board Computer and leverages brltty to communicate with legacy hardware, converting HID 
output to brltty output and brltty input to HID input on the fly. 

At present, BDIOHID is a proof of concept. It currently supports only one Braille display, the HIMS SyncBraille. However, 
support for additional hardware that brltty can autodetect should be straightforward to add by merely adding records to `/etc/bdio.db`.

**Important!** Currently, BDIOHID uses a **test** USB Product ID from [pid.codes](http://pid.codes). It is only suitable for development and testing work. Do not distribute hardware based on this VID/PID. 

## What You Need

The provided files are designed to work with a FriendlyArm NanoPi NEO. To build, you will need the following:

1. A Unix-based computer configured to build [OpenWRT](https://openwrt.org/).
2. [My branch of the OpenWRT project](https://github.com/parkrrrr/openwrt/tree/enable-udc), which has been modified to support 
   the NanoPi NEO USB host function. I should contribute this back to the OpenWRT project, but it is currently not very specific 
   to the NEO and I'm afraid it might break configurations for other SunXi-based hardware.
3. This repository, which should be cloned into a folder next to the above OpenWRT project (i.e. `bdiofeed` and `openwrt` should 
   be in the same folder.)
4. At least one Micro SD card for the firmware. If you order your NanoPi from FriendlyElec, you have the option to add one to 
   your purchase, but they are likely available for far cheaper elsewhere. 
5. A USB A to USB Micro-B cable. These are plentiful in all shapes and sizes in the form of charging cables for older Android 
   phones. Find one that suits your personal sense of style. 
6. Some way to write to a Micro SD card from the computer on which you're building BDIOHID. This means software **and** hardware.
   Personally, my build machine is an Ubuntu VM running in Oracle VirtualBox on top of a Windows 10 host, so I use a cheap 
   glittery purple USB-to-MicroSD adapter I picked up at Office Depot for hardware, and [Balena Etcher](https://www.balena.io/etcher/) 
   for software. Use what you're comfortable with. (I would probably use Unix `dd`, but the Windows host autodetects the FAT 
   boot partition and tries to mount the drive, interfering with `dd`.)
7. A compatible Braille display and the necessary hardware to plug it into a USB A port.
8. A NanoPi NEO, of course. I recommend the 
   [NanoPi NEO Metal Basic Kit](https://www.friendlyarm.com/index.php?route=product/product&path=85&product_id=260). 
   This is a nice, compact implementation of the hardware that has everything you need for a clean, professional installation. 

If you intend to do development work, you may instead want to purchase the 
[NEO Core Starter Kit](https://www.friendlyarm.com/index.php?route=product/product&path=85&product_id=215), which comes with 
the debugging module, additional USB ports, and support for an M.2 2242 SSD. Note, however, that the Micro USB connector on the 
shield board does not support the required USB host functionality, so you will need to use the one on the NanoPi NEO Core board. 
Depending on the USB Micro-B cable you use, the GPIO pins on the shield board may interfere somewhat with using that connector. 
You may need to lift the Core board off of its mating pins somewhat, or desolder and remove the GPIO connector from the shield 
board, as this project does not use it.

As an alternative to the Starter Kit, you may also do development work with the NanoPi NEO Metal Basic Kit and a 
[USB to TTL Serial Cable](https://www.friendlyarm.com/index.php?route=product/product&path=70&product_id=178). Just remove the 
screws from one end of the metal kit, extract the NEO, and plug the serial cable into the appropriate pins on the NEO's GPIO header. 
I recommend switching the switch on the USB to TTL cable to **not** provide power to the NEO. This allows you to more easily 
cycle power to the NEO by unplugging and replugging the USB Micro-B cable. 

## How To Build

Now that you've got all of the required bits, it's time to build. (Note: I have not yet tested this process on a completely new system. If you do, and it doesn't work, please let me know so we can figure out what needs to change.)

1. Run `bdiofeed/bdiohid/files/CreateDatabase.sql` using whatever tools you use to create an SQLite database. I use an 
   SQLite extension to Visual Studio Code to do this. This step is optional, because the repository contains a copy of
   `bdio.db`, but if you modify `CreateDatabase.sql`, you will need to do it.
2. Copy the files from `bdiofeed/openwrt-configs` to the `openwrt` folder in which my OpenWRT branch resides. 
   (There are better ways to do this, using OpenWRT's `make diffconfig`, but setting that up is a future project.)
3. `cd openwrt`    \# set the working directory to the OpenWRT folder from step 1
4. `./scripts/feeds update bdio`  \# Copy the package files and index information for the packages used by BDIOHID. 
    Currently, these are a stripped-down version of brltty version 6.0 and the bdiohid executable itself.
5. `./scripts/feeds install -a -p bdio` \# Create the symbolic links the OpenWRT build system needs to build the new packages.
6. Optionally, `make menuconfig` to change the OpenWRT configuration. As currently configured, the firmware image built 
   by this configuration is very limited and contains almost no tools (including lsusb - if you need that you should turn it on
   in the BusyBox config.)
7. `make`. If you have never built OpenWRT before, this will build the entire OpenWRT system, including the cross-compilation 
   toolchain, the Linux kernel, and assorted other utilities. This will take a while; go catch a movie or binge a season of 
   your favorite TV show while you wait.
8. If everything went well, you will have an `.img.gz` file in some subdirectory of `openwrt/bin/targets`. Find it, decompress 
   it, and burn the image to your MicroSD card using whatever means appeals to you.
9. Put the MicroSD card in the NanoPI NEO, plug your Braille display into the NEO's USB A port, plug the USB Micro-B cable 
   into the NEO's "OTG" port, and plug the other end of that cable into your computer.
10. If everything continued to go well, you should now have a new HID device. 

## Where We're Going

The current short-term road map for BDIOHID consists of the following (These are all added as github issues, too):

- Make input less sluggish. I don't know how to do raw Linux file I/O, and it shows. Right now it can take a while for an input event
  from brltty to show up as a HID input report. Change the file I/O to use `boost::asio` because I know how to use that to do what 
  I'm trying to accomplish here.
- Obtain and use a real PID from pid.codes.
- Do whatever's necessary to make the process of configuring OpenWRT for the build easier and more flexible.
- Create a developer configuration for OpenWRT including networking and `sshd` so that a developer can use the Ethernet port instead
  of the serial debugger interface.
- Add support for more Braille hardware. At present, this means adding entries to `bdiofeed/bdiohid/files/CreateDatabase.sql` but 
  I would also like to find a way to make that process easier. Some sort of configuration-building tool that could edit the control 
  hierarchy in a visual form and then make the required entries in the SQL database would be ideal.
- Add support for configurability. I envision this as a built-in http server which is accessed through a simulated USB Ethernet 
  adapter profile added to the existing USB Composite Gadget. At the very least, this must allow the user to choose brltty settings 
  to support hardware that brltty cannot autodetect. The NanoPi has more than enough power to also run DNS and DHCP servers on a 
  tiny pocket network, meaning that after plugging it in the user will be able to simply open their favorite web browser and navigate
  to, e.g. `http://bdiohid/`. Because of some of the stuff on the next list, I would likely use `boost::beast`, built directly
  into the bdiohid executable, for the HTTP server.
- Add support for additional hardware. At minimum, I would like to support some newer NanoPi hardware, especially the new NanoPi R1S,
  but I would also like to see what it takes to support other openwrt-compatiable Linux SBCs with USB OTG ports such as the OrangePi 
  Zero Plus.
  
## Where We're Going After That
  
The long-term road map for BDIOHID contains even more pie-in-the-sky stuff:
  
- Add support for the HID Braille *Screen Reader Control* functionality. This requires feature reports, which are currently 
  not supported by the Linux `f_hid` gadget, so the first task would be adding that functionality in a way that would be 
  acceptable to the maintainer of the Linux USB gadget framework.
- Add support for string descriptions of controls, so that, for example, client software could know that "left button 1" on 
  the SyncBraille is the "left up button." This also requires modifications to the Linux `f_hid` gadget, which currently has no 
  support for custom string descriptors. Note that there's already a field in `bdio.db` for these strings, though they are not 
  currently used in building the HID descriptors.
- Add support for HID over Bluetooth Low Energy. This would let you use a system based on the NanoPi Duo2 or the Raspberry Pi 
  Zero W, for example, to convert your older Braille display to connect wirelessly to mobile devices or computers.
- Add a web-based "Braille display simulator." This could be used by software and hardware developers to configure and operate 
  a display with any conceivable combination of rows and buttons in a way that it appears to the OS and to their software to 
  be a real piece of hardware. This could be used for testing odd configurations or for prototyping new functionality.
- Add support for automated testing using the simulation technology from the above feature. This, too, would be for software 
  developers.
  
## How You Can Help

I welcome contributions of all kinds, but I am particularly enthusiastic about crowdsourcing:

- Creation of new display profiles.
- Porting to other base hardware besides the NanoPi.
- Debugging of the build process.
- Documentation in general. This is a long readme, but it really doesn't even scratch the surface of what you need to know.
- Marketing. If you know someone who needs this, tell them about it. Tell them to tell their friends about it. Spread the word.
- Feature suggestions. Do you wish it did something that's not already on one of the lists above? Do you wish one of the things
  on the long-term road map were on the shorter-term road map? Tell me!
- Bug reports. Did your Braille display wake up in the middle of the night and go on a midnight rampage throughout the neighborhood, 
  frightening small animals and children and etching an eldritch rune into your mailbox? I need to know, for the sake of everyone 
  else's mailboxes. (It's too late for yours.)
