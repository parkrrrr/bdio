# Key table extraction

This directory contains files that ideally should only need to be used once. They were used to extract all of the raw key definitions
from the `brltty` drivers and convert them to display definition files. Since those display definition files are intended to be edited 
to add collection information, anything short of a brand-new driver is probably better handled by manual editing of the current display
definitions.

But, in case any of this is of any use to anyone else, here's what you'll find here.

- `alltables` is the output of the key table extraction. It contains, for each driver, the driver code and driver name followed by 
all of the tables defined for that driver.
- `dummy-for-keytable-extraction.zip` is a folder full of magic that extracts just the key table definitions from the `braille.c` file in 
each driver directory and compiles them into a program that outputs that folder's portion of `alltables`. Note that there are a handful
of drivers that don't fit the pattern - in particular, the Papenmeier and EuroBraille drivers. Those drivers were hand-modified to fit the
pattern before the scripts were run for the final time. Those modifications are not presented here.
- `compile-tables.pl` converts the easy-to-generate syntax of `alltables` into the more powerful syntax of a display definition file, making
certain assumptions about things like routing keys and cell sizes along the way.
- `mg-definitions` is a folder containing all of the original machine-generated definition files output by `compile-tables.pl`. The actual 
definitions used to build the mapping database are in the `src/mapping-db` directory.

