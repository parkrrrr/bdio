# Unsupported Braille Devices
The following devices are unlikely to be supported, because `brltty` doesn't use its modern raw-key-based driver architecture for them:

## Hardware Devices

I would like to support these devices, but many of them have little to no information available online, and since it would
require significant work on the brltty drivers they would also need to be tested extensively. That seems impractical given 
the age of the hardware.

- BrailComm - [III] can't find any evidence of this hardware's existence? The organization still exists, though.
- BrailleLite - this one might actually be important? But it's old hardware. [Blazie->HJ/FS->Vispero]
- Braudi - [Pro] can't find this one either
- EcoBraille - old Spanish display, little information available.
- LogText - Tactilog LogText - Danish, old, most links are to a museum in Denmark
- MiniBraille - Tieman MiniBraille - unmaintained driver [Tieman->Optelec->Vispero]
- MultiBraille - Tieman MultiBraille/Braillenotebook [Tieman->Optelec->Vispero]
- TechniBraille - [Manager 40] - French, possibly dead for 20 years?
- VideoBraille - Tiflosoft Videobraille VB40 - Italian
- VisioBraille - [TVB, 20/40] French - 30 years old? Name reused by new owner of Baum IP, so difficult to research.

## Software Devices

I will likely never support these devices, opting instead to create similar functionality through `bdiohid` directly where it makes
sense. (For example, my long-term wishlist includes an HTML/Javascript-based emulator that would fill XWindow's niche, and it would
be simple to write a network forwarder for HID config and reports to replace BrlAPI or Virtual.)

- BrlAPI - software (remote brltty)
- Libbraille - software - support for a number of displays, but apparently not touched since 2005
- TTY - software (text forwarder, doesn't handle dots)
- Virtual - software, network forwarder
- XWindow - software, graphical virtual display
