
class MemoryLock
{
protected:

    DWORD OldProtect;

    void * Address;
    size_t Size;

public:

    const char * Error;

    inline MemoryLock(void * Address, size_t Size)
    {
        Error = 0;

        this->Address = Address;
        this->Size = Size;

        if(!VirtualProtectEx(GetCurrentProcess(), Address, Size, PAGE_EXECUTE_READWRITE, &OldProtect))
        {
            Error = "Failed to change memory protection";
            return;
        }
    }

    inline ~MemoryLock()
    {
        if(Error)
            return;

        DWORD Unused;
        VirtualProtectEx(GetCurrentProcess(), Address, Size, OldProtect, &Unused);
    }

    
};

class Patch
{
public:

    const char * Error;

    inline Patch(void * Address, const char * Old, const char * New, size_t Size)
    {
        Error = 0;

        {   MemoryLock Lock(Address, Size);

            if(Lock.Error)
            {
                Error = Lock.Error;
                return;
            }

            if(memcmp(Address, Old, Size))
            {
                Error = "Old memory isn't as expected - wrong version?";
                return;
            }

            memcpy(Address, New, Size);
        }
    }

};

