#pragma once

#include "maptodb.h"

class Reader
{
public:
    static std::vector<Display> s_displays;
    static std::vector<Setting> s_settings;

    static bool ReadSettingsFile(const std::string& filename);
    static bool ReadDeviceFile(const std::string& filename);

private:
    static std::string ReadFile(const std::string& filename);
    static std::string StripComments(std::string& text);
};
