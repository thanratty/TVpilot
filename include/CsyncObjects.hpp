#pragma once

#include <vector>





#define     E_SO_OK                     (0)
#define     E_SO_WAIT_FAIL              (-1)
#define     E_SO_RESET_FAIL_INDEX       (-2)
#define     E_SO_RESET_FAIL_UNLOCK      (-3)
#define     E_SO_RESET_FAIL             (-4)





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
    int                     m_refcount{ 0 };

    inline static HANDLE    m_hSem{ INVALID_HANDLE_VALUE };
};



class CMultiEvents
{
public:
    //CMultiEvents::CMultiEvents(const HANDLE* handles, unsigned num_events );
    CMultiEvents::CMultiEvents(const std::vector<HANDLE>& handles);

    int     Wait();
    int     Reset(DWORD index);

private:
    std::vector<HANDLE>     m_handles;
    DWORD                   m_last_error{ 0 };
};






class Sing
{
public:

    static Sing& getInstance();

    bool    Lock();
    bool    Unlock();

private:

    Sing(Sing const&) = delete;
    Sing& operator=(Sing const&) = delete;

    Sing();
    ~Sing();

private:

    inline static int   m_refcount{ 0 };
    HANDLE              m_hSem{ INVALID_HANDLE_VALUE };
    CString             m_name;
    DWORD               m_last_error{ 0 };

};

