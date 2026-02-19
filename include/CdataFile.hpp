#pragma once

#include <string>


class CdataFile
{
public:
        CdataFile();

inline  const wchar_t* Filename() const
        {
            return m_filename.c_str();
        }

inline  bool IsNewFile() const
        {
            return m_IsNewFile;
        }

private:
        void             BuildFilename();

        std::wstring     m_filename;
        bool             m_IsNewFile{ false };
};

