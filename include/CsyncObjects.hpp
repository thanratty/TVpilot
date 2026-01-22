#pragma once

#include <vector>





#define     E_SO_OK                     (0)
#define     E_SO_WAIT_FAIL              (-1)
#define     E_SO_RESET_FAIL_INDEX       (-2)
#define     E_SO_RESET_FAIL_UNLOCK      (-3)
#define     E_SO_RESET_FAIL             (-4)



// TODO Use a singleton class ???
class CslotsSem
{
public:
    CslotsSem();
    ~CslotsSem();

    bool Lock();
    bool Unlock();

private:
    DWORD       m_last_error{ 0 };
    CString     m_name;

    // Only a single instance of these two vars shared between all objects
    inline static int    m_refcount{ 0 };
    inline static HANDLE m_hSem{ INVALID_HANDLE_VALUE };
};



class CMultiEvents
{
public:
    CMultiEvents::CMultiEvents(const HANDLE* handles, unsigned num_events );

    int     Wait();
    int     Reset(DWORD index);

    inline bool IsSignalled(DWORD index) const
    {
        return m_bIsSignalled[ index ];
    }

private:
    std::vector<HANDLE>     m_handles;
    std::vector<bool>       m_bIsSignalled;

    DWORD                   m_last_error{ 0 };
};

