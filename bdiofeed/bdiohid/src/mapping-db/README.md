# mapping-db

This folder contains the mappings from `brltty` devices to HID layout that are installed with bdio. When you build `bdio`, `maptodb` reads settings and device definitions from 
the `InputFiles` directory and uses them to create `/etc/bdio/bdio.db` for the target system. 

## Running maptodb manually

maptodb takes three parameters: an optional flag specifying which data to clear from the database, the name of the database, and a list of files and directories
to read.

__Note:__ *Unless you are building a brand-new database file, the results of reading a device file without first clearing any information pertaining to that*
*device are undefined and probably not what you wanted.*

```
maptodb <flags> <filename>...
```

### Flags:

|Flag|Meaning|
|----|-------|
| __-c__ all | Clears all data from the database |
| __-c__ settings | Clears just the settings data from the database |
| __-c__ smart | Clears data associated with the devices read from the specified device definition files |
| __-c__ *ttydriver* | Clears all device data with the specified ttydriver, as specified in the device definition files, documented below. |
| __-c__ *ttydriver*/*ttymodel* | Clears all device data with the specified ttydriver and ttymodel, as specified in the device definition files. |
| __-o__ *dbfilename* | Specifies the database file 

__Note:__ *`-c smart` does not work with settings. If you need to clear settings, you must specify `-c settings`*.

### Examples:
```
# All examples look for or create `bdio.db` in the files/etc subdirectory of your `bdiohid` directory, assuming you run 
# it from this directory.

# clears the entire database and rebuilds it using the two specified settings files and everything in the Devices 
# directory. This is what the build process does.
maptodb -c all -o ../../files/etc/bdio.db InputFiles/Settings/Global.settings InputFiles/Settings/NanoPiNeo.settings InputFiles/Devices

# clears just the settings from the database, and then rebuilds them using the specified settings files
maptodb -c settings -o ../../files/etc/bdio.db InputFiles/Settings/Global.settings InputFiles/Settings/NanoPiNeo.settings

# clears the settings for all HIMS devices, then rebuilds them using all of the files in the Devices diretory starting 
# with "hm-"
maptodb -c HIMS -o ../../files/etc/bdio.db InputFiles/Devices/hm-*

# same result as the previous example, but gets the device names from the devices files.
maptodb -c smart -o ../../files/etc/bdio.db InputFiles/Devices/hm-*

# clears the settings for just the HIMS SyncBraille, then rebuilds it using the `hm-sync` device definition file.
maptodb -c HIMS/sync -o ../../files/etc/bdio.db InputFiles/Devices/hm-sync

# same result as the previous example, but gets the device name from the device file.
maptodb -c smart -o ../../files/etc/bdio.db InputFiles/Devices/hm-sync
```

## Settings Files

A settings file is an ASCII text file containing configuration data for `bdio`. The format of this file is 

```
    # comments are indicated with either pound signs as in shell scripts
    // or double slashes as in C++-like languages
    settings {
        # keys aren't quoted and must have no spaces. 
        # Values are quoted and may contain anything but a quote character.
        # There are no escape codes, so you really can't put quotes in values.

        key1 = "value 1";   
        key2 = "value #2";  // pound signs in quoted strings aren't comments.
        ...
    }
```

You can put any settings you want in a settings file, but at present there is a very small list of settings that have any effect, and all of them 
are defined in one or more of the files in the `InputFiles/Settings` folder. 

Generally, however, the default settings are the only settings that make sense. It's likely you'll only need to modify them if you are 
porting to new hardware that uses different USB drivers.

## Device Definition Files

Device definition files live in the `InputFiles/Devices` folder.

A device definition file is an ASCII text file containing information about a single Braille device. It contains a set of strings that 
identify the device, followed by structured markup that defines the HID application collection and all of its sub-collections and controls.

Below is the contents of the `hm-sync` device definition file, liberally marked up with comments in an attempt to explain everything. 
This file was chosen for its simplicity, and because it's the hardware I used in initial development.

The machine-generated device definition files may begin with comments about the hardware. These comments are derived from the `brltty`
input table definitions and may contain helpful hints as to the locations or meanings of the keys defined within. The definition file 
will always begin with a comment listing the machine-generated manufacturer and model information. These also come from the `brltty` input
table definitions.

```
    # Comments are the same as in settings files - pound signs as in shell scripts
    // or double slashes as in C++-like languages.
    # Quoted strings are also the same as in settings files - they can't contain " characters.

    # Display definition for HIMS : SyncBraille

    ttydriver    "HIMS"        // This is the name of the driver exposed by brlapi. You probably will not want to change it.
    ttymodel     "sync"        // This is the model name exposed by brlapi. You probably will not want to change it, either.
    ttycode      "hm"          // This is the driver code used by brltty when not using autodetection. Probably also unchanged.
    manufacturer "HIMS"        // This is the name of the hardware manufacturer. This, you might want to change.
    model        "SyncBraille" // This is the name of the specific Braille hardware. This, too, you might want to change.

    // The following line starts the HID report descriptor definition by creating a "Braille Display" (0x41 0x01) application 
    // collection. 

    display {                  

        // The following line creates a "Braille Row" logical collection. See below for the names of collections in the 
        // Braille usage page.

        row {
            // This row contains 8-dot Braille cells. (6 is also a valid value. No other value is valid.)
            // The number of cells (and routing keys) is not specified; it is read from brlapi at runtime.

            cells 8;

            // This row also contains a row of primary router keys. This line creates a logical collection for them.

            router1 {

                // The entirety of the brltty key group "1" is used for the router keys in this row.
                // This is a specialized case of the general case of defining key mappings, explained below.

                router   1;  # RoutingKey

            }  // a close brace marks the end of a logical collection.
        }

        // This line creates a "Braille Left" logical collection. Such a collection should contain all buttons on the left 
        // side of the display, per the HID Braille specification.

        left {

            // The next line demonstrates the most general form of a key mapping. 
            // A key mapping contains three components: a usage value, a brltty key, and an optional name. 
            // A key mapping is always followed by a semicolon.
            
            // In this line, "button 1" is the usage value. It consists of the usage page - "button" - and 
            // the usage ID within that page - "1".  If the usage page is the Braille usage page, 0x41, and the 
            // usage ID is one of the named IDs defined below, the page may be omitted. Thus, "router" defines 
            // both the usage page (0x41) and the usage ID (0x100).
            // Either the usage page or the usage ID may be a number. The following usages are all equivalent:
            // router
            // braille router
            // 0x41 router
            // 0x41 0x100
            // braille 0x100
            // braille 256
            // 65 256
            // See the following section for a list of the defined names.

            // "(0, 12)" is the brltty key. A brltty key consists of either a single number, which represents all keys
            // in the brltty key group with that number, or an ordered pair of the key group and the key id within that
            // group. Thus, (0, 12) is the key with id 12 in group 0. These values are generated by the brltty driver and
            // usually will not need be changed, though it's possible that the machine generation process misread the 
            // values for some driver. At present, bdio only understands what to do with entire groups when they are 
            // defined as router keys, so it's likely that anything that isn't a router will use the ordered pair 
            // representation.

            // Finally, "LeftScrollUp" is the name of the key. The default names are derived from the brltty driver source, 
            // and have no spaces. You'll probably want to make these human-readable by adding spaces or completely 
            // renaming them. 
            // Note that, currently, bdio does nothing with these names. Eventually, it is hoped that the names will
            // be attached to the HID report descriptor, but Linux currently has no support for application-defined 
            // string descriptors. It's still worthwhile to update the names, however, since they may eventually be 
            // exposed to the client.
            // The name is optional. Thus, the following line could just be 
            // button 1   (0, 12); 

            button 1   (0, 12) "LeftScrollUp"; 
            button 2   (0, 15) "LeftScrollDown"; 
        }

        right {
            button 3   (0, 13) "RightScrollUp"; 
            button 4   (0, 14) "RightScrollDown"; 
        }
    }
```

In addition to the above, you may occasionally see lines like the following in a machine-generated device definition file

```
  button 7   (0, 37) "B11";   # brltty has no binding for this key
  button 8   (0, 39) "F1";   # brltty has no binding for this key
  button 9   (0, 40) "F2";   # brltty has no binding for this key
  button 10   (0, 41) "F3";   # brltty has no binding for this key
  button 11   (0, 42) "F4";   # brltty has no binding for this key
```

These lines are from the `bm-ultra` file. The comments indicate that `brltty` does not use these keys. We still define the keys,
by default, but the fact that `brltty` does not use them may indicate that they do not actually exist on this hardware. Thus,
the machine generation process adds a comment to notify you of that fact.

In fact, the Baum VarioUltra does not have a "B11" key, but it does have the "F1" through "F4" keys. So, you might decide to 
delete the "B11" line or modify it to instead read

```
  # button 7   (0, 37) "B11";   # The VarioUltra has no B11 key.
```

Another thing you may see is something like the following

```
  # unrecognized group 2 named "NavrowKey"   # brltty has no binding for this key

  # unrecognized group 2 named "HorizontalSensor" 
  # unrecognized group 5 named "ScaledLeftSensor" 
  # unrecognized group 6 named "ScaledRightSensor" 
```

The first line is from the `fs-pacmate` definition file. It indicates that `brltty` exposes a group of keys similar to router 
keys, but that since it does not explicitly call them router keys, the machine didn't know how to map them. It also indicates
that `brltty` itself doesn't use those keys, as above.

The other three lines are from the `bm-inka` definition file. They indicate the same thing: `brltty` exposes three groups of
keys that aren't called routing keys, so they are unmapped by default. (But `brltty` does something with them, so they're 
probably real and meaningful.)

How you deal with these lines depends greatly on what the hardware actually does with the keys in those groups. Since I don't 
have access to any of the hardware that exposes such keys, I don't know what to do with them either. They may map as router 
keys, they may map as a bunch of individual buttons, or they may just not map at all.

It's also possible that some such key groups may want to map to an entire manufacturer-defined (0xFFxx) usage page. This would require 
slight changes to the bdio executable and to the device definition language, but might be the best disposition for them.

Finally, the machine-generation process attempts to identify keys that can be mapped to HID concepts based on their names, but
it doesn't always succeed. You might see this in the Freedom Scientific Focus 40 definition, where some keys that are on the
left are put into a `Left` collection, but others are not. You might also see it in the Baum VarioUltra definition, where the 
DPad is mapped to individual generic buttons. Thus, it's likely that you'll want to change the usages for some keys, or to move
keys around into different collections. You may even want to add whole new collections that were not generated by the machine.

## Named Usages

Device definition files accept names for all of the defined usages on the Braille usage page (0x41.) In addition, they also 
accept names for two defined usage pages.

### Usage pages

| name   | value | meaning |
|--------|-------|---------|
|button  | 0x09 | The generic buttons page. All usage IDs from 0x0001 to 0xffff are available, but none of them have names. |
|braille | 0x41 | The Braille usage page. Defined usages on this page all have names; see below. No other usage on this page should be used. |

The Braille usage page is defined by USB HID [HUTRR-78](https://www.usb.org/sites/default/files/hutrr78_-_creation_of_a_braille_display_usage_page_0.pdf)

The Button usage page is defined by the [USB HID Usage Tables Specification](https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf) starting on page 67.

The latter specification also defines many other usage pages. You can map Braille buttons to those usage pages by specifying their numeric values, but be aware that 
since the device identifies itself as a Braille display and not as a joystick, mappings to *e.g.* the Generic Desktop page probably will not result in being 
able to use the joystick on your Braille display as a game controller. (If you wanted to do that, you'd start by modifying the database to use a usage 
of 0x01 0x05 Game Pad for the application collection, and setting the mappings to the D-Pad mappings, 0x01 0x90 through 0x01 0x93. Hypothetically. But
then you wouldn't be able to use it as a Braille display.)

Mapping to the keyboard (or mouse) page and modifying `bdiohid` to identify as both a keypad and a Braille display might be an interesting hack, and even 
a useful one for some displays that have Control or Alt keys, but you can probably already do that with your screen reader.

### Usage IDs on the Braille usage page

| name   | value | meaning |
|--------|-------|---------|
 | display | 0x1 | display application **collection** - *this is only to be used as the top-level application collection* | 
 | row | 0x2 | row **collection** (contains cells, cell count, router collections, router buttons) | 
 | cell8 | 0x3 | 8-dot Braille cell (8 bits) - *do not use this mapping. Use the `cells` directive instead* | 
 | cell6 | 0x4 | 6-dot Braille cell (8 bits) - *do not use this mapping. Use the `cells` directive instead* | 
 | cellcount | 0x5 | cell count (8 bits) - *bdio currently does not support this usage* | 
 | router1 | 0xfa | router set 1 **collection** | 
 | router2 | 0xfb | router set 2 **collection** | 
 | router3 | 0xfc | router set 3 **collection** | 
 | router | 0x100 | router key (contained in router set, array) | 
 | buttons | 0x200 | generic Braille button **collection** | 
 | dot1 | 0x201 | keyboard dot 1 | 
 | dot2 | 0x202 | keyboard dot 2 | 
 | dot3 | 0x203 | keyboard dot 3 | 
 | dot4 | 0x204 | keyboard dot 4 | 
 | dot5 | 0x205 | keyboard dot 5 | 
 | dot6 | 0x206 | keyboard dot 6 | 
 | dot7 | 0x207 | keyboard dot 7 | 
 | dot8 | 0x208 | keyboard dot 8 | 
 | space | 0x209 | keyboard space | 
 | lspace | 0x20a | keyboard left space | 
 | rspace | 0x20b | keyboard right space | 
 | face | 0x20c | face controls **collection** | 
 | left | 0x20d | left controls **collection** | 
 | right | 0x20e | right controls **collection** | 
 | top | 0x20f | top controls **collection** | 
 | jcenter | 0x210 | joystick center (sometimes called joystick press) | 
 | jup | 0x211 | joystick up | 
 | jdown | 0x212 | joystick down | 
 | jleft | 0x213 | joystick left | 
 | jright | 0x214 | joystick right | 
 | dcenter | 0x215 | d-pad center (sometimes called d-pad press) | 
 | dup | 0x216 | d-pad up | 
 | ddown | 0x217 | d-pad down | 
 | dleft | 0x218 | d-pad left | 
 | dright | 0x219 | d-pad right | 
 | pleft | 0x21a | pan left | 
 | pright | 0x21b | pan right | 
 | rup | 0x21c | rocker up | 
 | rdown | 0x21d | rocker down | 
 | rpress | 0x21e | rocker press | 
