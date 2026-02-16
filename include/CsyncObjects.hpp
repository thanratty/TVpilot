#pragma once

#include <vector>





#define     E_SO_OK                     (0)
#define     E_SO_WAIT_FAIL              (-1)
#define     E_SO_RESET_FAIL_INDEX       (-2)
#define     E_SO_RESET_FAIL_UNLOCK      (-3)
#define     E_SO_RESET_FAIL             (-4)




class CMultiEvents
{
public:
    CMultiEvents(const std::vector<HANDLE>& handles);
    ~CMultiEvents();

    int     Wait();
    int     Reset(DWORD index);

private:
    std::vector<HANDLE>     m_handles;
    DWORD                   m_last_error{ 0 };

    bool                    CheckWaitResult(DWORD result) const;
};






class CslotsSem
{
public:
    static CslotsSem& getInstance();

    bool    Lock();
    bool    Unlock();

private:
    CslotsSem(CslotsSem const&) = delete;
    CslotsSem& operator=(CslotsSem const&) = delete;

    CslotsSem();
    ~CslotsSem();

private:
    const CString   m_name{ L"slotsSem" };
    HANDLE          m_hSem{ INVALID_HANDLE_VALUE };
    DWORD           m_last_error{ 0 };

    bool            CheckWaitResult(DWORD result);

};
