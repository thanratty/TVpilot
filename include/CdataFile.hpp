#pragma once

#include "config.h"

#include "pch.h"

#include "common.hpp"

//--

#include <string>


class CdataFile
{
public:
    CdataFile();

    inline const wchar_t* Filename() const
    {
        return m_filename.c_str();
    }

    inline bool IsNewFile() const
    {
        return new_data_file;
    }

private:
    void            BuildFilename();

    std::wstring     m_filename;
    bool             new_data_file{ false };
};

