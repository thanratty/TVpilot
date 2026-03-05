#pragma once

#include <string>


class CdataFile
{
public:
        CdataFile();

const wchar_t* Filename() const {
    return m_filename.c_str();
}

bool IsNewFile() const {
    return m_IsNewFile;
}

private:
        void             BuildFilename();

        std::wstring     m_filename;
        bool             m_IsNewFile{ false };
};

