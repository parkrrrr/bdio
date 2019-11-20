-- SQLite
drop table if exists settings;
create table settings(setting_name, setting_value);

insert into settings VALUES
    ("udc", "musb-hdrc.1.auto"),
    ("configdir", "/sys/kernel/config/usb_gadget"),
    ("vid", "0x1209"),
    ("pid", "0x0001"),
    ("serial", "bdio-hid"),
    ("devname", "/dev/hidg0");

drop table if exists displays;
create table displays(modelid, mfgr, model, mfgr_name, model_name);

insert into displays values
    (1, "HIMS", "sync", "HIMS", "SyncBraille");

drop table if exists collections;
create table collections(modelid, collection_id, parent_collection_id, usage_page, usage);

insert into collections values
    (1, 0, -1, "braille", "display"),
    (1, 1, 0, "braille", "row"),
    (1, 2, 1, "braille", "router1"),
    (1, 3, 0, "braille", "left"),
    (1, 4, 0, "braille", "right");

drop table if exists mappings;
create table mappings(modelid, key_group, key_index, collection_id, usage_page, usage, name);

insert into mappings values
    (1, -1, -1, 1, "braille", "cell8", "cells"),
    (1, 1, -1, 2, "braille", "router", "router"),
    (1, 0, 12, 3, "button", "1", "left scroll up"),
    (1, 0, 15, 3, "button", "2", "left scroll down"),
    (1, 0, 13, 4, "button", "1", "right scroll up"),
    (1, 0, 14, 4, "button", "2", "right scroll down");

select * from settings order by setting_name;
select * from displays order by modelid;
select * from collections order by modelid, collection_id;
select * from mappings order by modelid, key_group, key_index;
