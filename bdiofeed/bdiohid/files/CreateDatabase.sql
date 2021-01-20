-- SQLite
drop table if exists settings;
create table settings(setting_name, setting_value);

insert into settings VALUES
    ("udc", "musb-hdrc.1.auto"),
    ("configdir", "/sys/kernel/config"),
    ("vid", "0x1209"),
    ("pid", "0xbd10"),
    ("serial", "bdio-hid"),
    ("devname", "/dev/hidg0");

drop table if exists displays;
create table displays(modelid, mfgr, model, mfgr_name, model_name);

insert into displays values
    (1, "HIMS", "sync", "HIMS", "SyncBraille"),
    (2, "TSI",  "pb40", "TeleSensory", "PowerBraille 40"),
    (3, "HumanWare", "BI40", "HumanWare", "Brailliant BI 40"),
    (4, "Baum", "ultra", "Baum", "VarioUltra");

drop table if exists collections;
create table collections(modelid, collection_id, parent_collection_id, usage_page, usage);

insert into collections values
    (1, 0, -1, "braille", "display"),
    (1, 1,  0, "braille", "row"),
    (1, 2,  1, "braille", "router1"),
    (1, 3,  0, "braille", "left"),
    (1, 4,  0, "braille", "right"),

    (2, 0, -1, "braille", "display"),
    (2, 1,  0, "braille", "row"),
    (2, 2,  1, "braille", "router1"),
    (2, 3,  0, "braille", "left"),
    (2, 4,  0, "braille", "right"),
    (2, 5,  0, "braille", "face"),
    (2, 6,  5, "braille", "left"),
    (2, 7,  5, "braille", "right"),

    (3, 0, -1, "braille", "display"),
    (3, 1,  0, "braille", "row"),
    (3, 2,  1, "braille", "router1"),
    (3, 3,  0, "braille", "left"),
    (3, 4,  0, "braille", "right"),
    (3, 5,  0, "braille", "face"),

    (4, 0, -1, "braille", "display"),
    (4, 1,  0, "braille", "row"),
    (4, 2,  1, "braille", "router1"),
    (4, 3,  0, "braille", "left"),
    (4, 4,  0, "braille", "right"),
    (4, 5,  0, "braille", "face");


drop table if exists mappings;
create table mappings(modelid, key_group, key_index, collection_id, usage_page, usage, name);

insert into mappings values
    (1, -1, -1, 1, "braille", "cell8",  "cells"),
    (1,  1, -1, 2, "braille", "router", "router"),
    (1,  0, 12, 3, "button",  "1",      "left scroll up"),
    (1,  0, 15, 3, "button",  "2",      "left scroll down"),
    (1,  0, 13, 4, "button",  "1",      "right scroll up"),
    (1,  0, 14, 4, "button",  "2",      "right scroll down"),

    (2, -1, -1, 1, "braille", "cell8",  "cells"),
    (2,  1, -1, 2, "braille", "router", "router"),
    (2,  0,  0, 3, "button",  "1",      "left button"),
    (2,  0,  2, 4, "button",  "2",      "right button"),
    (2,  0,  6, 6, "braille", "rup",    "left rocker up"),
    (2,  0,  8, 6, "braille", "rdown",  "left rocker down"),
    (2,  0,  9, 5, "button",  "1",      "convex"),
    (2,  0,  4, 5, "button",  "2",      "concave"),
    (2,  0,  1, 7, "braille", "rup",    "right rocker up"),
    (2,  0,  3, 7, "braille", "rdown",  "right rocker down"),

    (3, -1, -1, 1, "braille", "cell8",  "cells"),
    (3,  1, -1, 2, "braille", "router", "router"),
    (3,  0,  2, 0, "braille", "dot1",   "dot 1"),
    (3,  0,  3, 0, "braille", "dot2",   "dot 2"),
    (3,  0,  4, 0, "braille", "dot3",   "dot 3"),
    (3,  0,  5, 0, "braille", "dot4",   "dot 4"),
    (3,  0,  6, 0, "braille", "dot5",   "dot 5"),
    (3,  0,  7, 0, "braille", "dot6",   "dot 6"),
    (3,  0,  8, 0, "braille", "dot7",   "dot 7"),
    (3,  0,  9, 0, "braille", "dot8",   "dot 8"),
    (3,  0, 10, 0, "braille", "space",  "space"),
    (3,  0, 17, 5, "button",  "11",     "previous"),
    (3,  0, 18, 5, "button",  "12",     "back"),
    (3,  0, 19, 5, "button",  "13",     "advance"),
    (3,  0, 20, 5, "button",  "14",     "next"),
    (3,  0, 11, 3, "button",  "1",      "command 1"),
    (3,  0, 12, 3, "button",  "2",      "command 2"),
    (3,  0, 13, 3, "button",  "3",      "command 3"),
    (3,  0, 14, 4, "button",  "4",      "command 4"),
    (3,  0, 15, 4, "button",  "5",      "command 5"),
    (3,  0, 16, 4, "button",  "6",      "command 6"),

    (4, -1, -1, 1, "braille", "cell8",  "cells"),
    (4,  1, -1, 2, "braille", "router", "router"),
    (4,  0, 43, 0, "braille", "dot1",   "dot 1"),
    (4,  0, 44, 0, "braille", "dot2",   "dot 2"),
    (4,  0, 45, 0, "braille", "dot3",   "dot 3"),
    (4,  0, 46, 0, "braille", "dot4",   "dot 4"),
    (4,  0, 47, 0, "braille", "dot5",   "dot 5"),
    (4,  0, 48, 0, "braille", "dot6",   "dot 6"),
    (4,  0, 49, 0, "braille", "dot7",   "dot 7"),
    (4,  0, 50, 0, "braille", "dot8",   "dot 8"),
    (4,  0, 35, 0, "braille", "lspace", "B9/left space"),
    (4,  0, 36, 0 ,"braille" ,"rspace", "b10/right space"),
    (4,  0, 39, 5, "button",  "11",     "F1"),
    (4,  0, 40, 5, "button",  "12",     "F2"),
    (4,  0, 41, 5, "button",  "13",     "F3"),
    (4,  0, 42, 5, "button",  "14",     "F4"),
    (4,  0, 51, 5, "braille", "jup",    "joystick up"),
    (4,  0, 52, 5, "braille", "jleft",  "joystick left"),
    (4,  0, 53, 5, "braille", "jdown",  "joystick down"),
    (4,  0, 54, 5, "braille", "jright", "joystick right"),
    (4,  0, 55, 5, "braille", "jcenter","joystick press"),
    (4,  0,  0, 3, "button",  "1",      "command 1"),
    (4,  0,  1, 3, "button",  "2",      "command 2"),
    (4,  0,  2, 3, "button",  "3",      "command 3"),
    (4,  0,  3, 4, "button",  "4",      "command 4"),
    (4,  0,  4, 4, "button",  "5",      "command 5"),
    (4,  0,  5, 4, "button",  "6",      "command 6");    
select * from settings order by setting_name;
select * from displays order by modelid;
select * from collections order by modelid, collection_id;
select * from mappings order by modelid, key_group, key_index;
