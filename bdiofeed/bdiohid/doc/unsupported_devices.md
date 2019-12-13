# Unsupported Braille Devices
The following devices are unlikely to be supported, because `brltty` doesn't use its modern raw-key-based driver architecture for them. 

## Hardware Devices

I would like to support these devices, but many of them have little to no information available online, and since it would
require significant work on the `brltty` drivers they would also need to be tested extensively. That seems impractical for me 
to do, given the age of the hardware, without the collaboration of someone who has working hardware for testing purposes.

There is some benefit to other users of `brltty` in updating these drivers - it appears on casual inspection that these displays
also would not support user-defined key bindings as currently written. (And, indeed, if you look in `brltty/Tables/Input/xx` for 
any of the drivers listed below, all you'll find is `.txt` files describing the hard-coded command mappings.)

As an alternative to rewriting the drivers to output raw keypresses, it might be possible to set a flag in the database specifying 
that a device should instead connect in cooked mode, and then map the `brltty` commands to buttons as appropriate. However, this 
gets interesting when chords are pressed, because we have to somehow reverse the chord-to-command mapping. And I'm not clear on 
what happens when the user presses a chord that the driver doesn't map; it probably gets eaten by the driver or output as the 
command mapped to some subset of the pressed keys.

- BrailComm - [III] In a cursory search, I can't find any evidence of this hardware's existence. The organization still exists, 
  but their current website makes no mention of the hardware that I can find.
- BrailleLite - This IP is currently owned by Vispero. It was a popular note-taker for its time, and it seems like it would be
  worthwhile to support it if we can. I have hardware for this one, but I don't know if it's operational because I lack the power
  supply and the proprietery serial cable. Still, of everything on this list, this is the one I'm most likely to be able to make 
  work.
- Braudi - [Pro] In a cursory search, I was unable to find anything about this hardware other than what's in the `brltty` sources 
  and documentation.
- EcoBraille - Mailing list archives indicate that this was an older Spanish display, apparently of a similar form factor to older
  Alva and TSI displays, designed to sit under a keyboard, and that it may still be (or have been, at the time of the mail thread) 
  possible for Spanish citizens to acquire one at an attractive price. 
- LogText - Tactilog LogText - Tactilog apparently made a large number of displays for the Danish market, but most of the references 
  to their hardware now on the Internet are on the website of a museum in Denmark who has a selection of their hardware.
- MiniBraille - Tieman MiniBraille - The `README` for this driver indicates that it is unmaintained due to the lack of availability 
  of the hardware. The last update to that file was before 2008 when the source tree was reorganized, so it's been unmaintained for 
  at least that long. This IP is also currently owned by Vispero.
- MultiBraille - Tieman MultiBraille/Braillenotebook - A very cursory search turned up little information beyond what I already knew
  about the MiniBraille. This IP is also currently owned by Vispero.
- TechniBraille - [Manager 40] - I was unable to find much information on this. The hardware was created for the French market. 
  The website is still alive, but it contains a forwarding link from 20 years ago that forwards to another website which is now 
  a parked domain.
- VideoBraille - Tiflosoft Videobraille VB40 - I don't know much about this hardware except that it was created for the Italian 
  market.
- VisioBraille - [TVB, 20/40] This was a French display apparently created about 30 years ago. It's hard to find information 
  about this hardware because the VisioBraille name has also been used by a German company who owns or owned the Baum IP.
  
## Software Devices

I will likely never support these devices, opting instead to create similar functionality through `bdiohid` directly where it makes
sense. (For example, my long-term wishlist includes an HTML/Javascript-based emulator that would fill XWindow's niche, and it would
be simple to write a network forwarder for HID config and reports to replace BrlAPI or Virtual.)

- BrlAPI - This is a forwarding driver for `brltty` running on a remote machine.
- Libbraille - This driver encapsulates `libbraille`, which has support for a number of displays but appears to have been inactive
  since 2005. If `libbraille` supports hardware that `brltty` does not support natively, there might be an argument for making this 
  work, if it can be done. There might also be an argument for working with `libbraille` directly from `bdio`, without the `brltty`
  middleman. 
- TTY - This is a software driver which outputs to a tty. Among other things, it also doesn't support raw dots, and raw dots are all
  we get from HID, so supporting this would require back-translation via `liblouis` or some other mechanism. While it's likely that
  such back-translation will eventually happen somewhere in `bdio`, it'll probably be through a web interface rather than a tty.
- Virtual - This is a network forwarder, mainly intended for testing and development purposes, with a simplified protocol.
- XWindow - This driver supports an X-based graphical virtual display. While I could theoretically support this driver with a remote
  X server (my hardware lacks a display adapter) I instead plan to cut `brltty` out of the loop for software-emulated displays, as 
  noted above.
