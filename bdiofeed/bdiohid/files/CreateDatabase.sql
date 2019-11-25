-- SQLite
drop table if exists settings;
create table settings(setting_name, setting_value);

insert into settings VALUES
    ("udc", "musb-hdrc.1.auto"),
    ("configdir", "/sys/kernel/config"),
    ("vid", "0x1209"),
    ("pid", "0x0001"),
    ("serial", "bdio-hid"),
    ("devname", "/dev/hidg0");

drop table if exists displays;
create table displays(modelid, mfgr, model, mfgr_name, model_name);

insert into displays values
    (1, "HIMS", "sync", "HIMS", "SyncBraille"),
    (2, "TSI",  "pb40", "TeleSensory", "PowerBraille 40");

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
    (2, 7,  5, "braille", "right");

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
    (2,  0,  3, 7, "braille", "rdown",  "right rocker down");

select * from settings order by setting_name;
select * from displays order by modelid;
select * from collections order by modelid, collection_id;
select * from mappings order by modelid, key_group, key_index;
