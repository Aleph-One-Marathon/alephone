
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
typedef HANDLE PipeType;
#define NULLPIPE NULL
typedef unsigned __int8 uint8;
typedef __int32 int32;
typedef unsigned __int64 uint64;
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
typedef uint8_t uint8;
typedef int32_t int32;
typedef uint64_t uint64;
typedef int PipeType;
#define NULLPIPE -1
#endif

#include <sstream>
#include "steamshim_child.h"

#define DEBUGPIPE 1
#if DEBUGPIPE
#define dbgpipe printf
#else
static inline void dbgpipe(const char *fmt, ...) {}
#endif

static int writePipe(PipeType fd, const void *buf, const unsigned int _len);
static int readPipe(PipeType fd, void *buf, const unsigned int _len);
static void closePipe(PipeType fd);
static char *getEnvVar(const char *key, char *buf, const size_t buflen);
static int pipeReady(PipeType fd);


#ifdef _WIN32

static int pipeReady(PipeType fd)
{
    DWORD avail = 0;
    return (PeekNamedPipe(fd, NULL, 0, NULL, &avail, NULL) && (avail > 0));
} /* pipeReady */

static int writePipe(PipeType fd, const void *buf, const unsigned int _len)
{
    const DWORD len = (DWORD) _len;
    DWORD bw = 0;
    return ((WriteFile(fd, buf, len, &bw, NULL) != 0) && (bw == len));
} /* writePipe */

static int readPipe(PipeType fd, void *buf, const unsigned int _len)
{
    const DWORD len = (DWORD) _len;
    DWORD br = 0;
    return ReadFile(fd, buf, len, &br, NULL) ? (int) br : -1;
} /* readPipe */

static void closePipe(PipeType fd)
{
    CloseHandle(fd);
} /* closePipe */

static char *getEnvVar(const char *key, char *buf, const size_t _buflen)
{
    const DWORD buflen = (DWORD) _buflen;
    const DWORD rc = GetEnvironmentVariableA(key, buf, buflen);
    /* rc doesn't count null char, hence "<". */
    return ((rc > 0) && (rc < buflen)) ? buf : NULL;
} /* getEnvVar */

#else

static int pipeReady(PipeType fd)
{
    int rc;
    struct pollfd pfd = { fd, POLLIN | POLLERR | POLLHUP, 0 };
    while (((rc = poll(&pfd, 1, 0)) == -1) && (errno == EINTR)) { /*spin*/ }
    return (rc == 1);
} /* pipeReady */

static int writePipe(PipeType fd, const void *buf, const unsigned int _len)
{
    const ssize_t len = (ssize_t) _len;
    ssize_t bw;
    while (((bw = write(fd, buf, len)) == -1) && (errno == EINTR)) { /*spin*/ }
    return (bw == len);
} /* writePipe */

static int readPipe(PipeType fd, void *buf, const unsigned int _len)
{
    const ssize_t len = (ssize_t) _len;
    ssize_t br;
    while (((br = read(fd, buf, len)) == -1) && (errno == EINTR)) { /*spin*/ }
    return (int) br;
} /* readPipe */

static void closePipe(PipeType fd)
{
    close(fd);
} /* closePipe */

static char *getEnvVar(const char *key, char *buf, const size_t buflen)
{
    const char *envr = getenv(key);
    if (!envr || (strlen(envr) >= buflen))
        return NULL;
    strcpy(buf, envr);
    return buf;
} /* getEnvVar */

#endif


static PipeType GPipeRead = NULLPIPE;
static PipeType GPipeWrite = NULLPIPE;

enum ShimCmd
{
    SHIMCMD_BYE,
    SHIMCMD_PUMP,
    SHIMCMD_REQUESTSTATS,
    SHIMCMD_STORESTATS,
    SHIMCMD_SETACHIEVEMENT,
    SHIMCMD_GETACHIEVEMENT,
    SHIMCMD_RESETSTATS,
    SHIMCMD_SETSTATI,
    SHIMCMD_GETSTATI,
    SHIMCMD_SETSTATF,
    SHIMCMD_GETSTATF,
    SHIMCMD_WORKSHOP_UPLOAD,
    SHIMCMD_WORKSHOP_QUERY_ITEM_OWNED,
    SHIMCMD_WORKSHOP_QUERY_ITEM_MOD,
    SHIMCMD_WORKSHOP_QUERY_ITEM_SCENARIO
};

static int write1ByteCmd(const uint8 b1)
{
    uint16_t length = 1;
    uint8* length_bytes = (uint8*)&length;
    const uint8 buf[] = { length_bytes[0], length_bytes[1], b1};
    return writePipe(GPipeWrite, buf, sizeof (buf));
} /* write1ByteCmd */

static int write2ByteCmd(const uint8 b1, const uint8 b2)
{
    uint16_t length = 2;
    uint8* length_bytes = (uint8*)&length;
    const uint8 buf[] = { length_bytes[0], length_bytes[1], b1, b2};
    return writePipe(GPipeWrite, buf, sizeof (buf));
} /* write2ByteCmd */

static inline int writeBye(void)
{
    dbgpipe("Child sending SHIMCMD_BYE().\n");
    return write1ByteCmd(SHIMCMD_BYE);
} // writeBye

static int initPipes(void)
{
    char buf[64];
    unsigned long long val;

    if (!getEnvVar("STEAMSHIM_READHANDLE", buf, sizeof (buf)))
        return 0;
    else if (sscanf(buf, "%llu", &val) != 1)
        return 0;
    else
        GPipeRead = (PipeType) val;

    if (!getEnvVar("STEAMSHIM_WRITEHANDLE", buf, sizeof (buf)))
        return 0;
    else if (sscanf(buf, "%llu", &val) != 1)
        return 0;
    else
        GPipeWrite = (PipeType) val;
    
    return ((GPipeRead != NULLPIPE) && (GPipeWrite != NULLPIPE));
} /* initPipes */


int STEAMSHIM_init(void)
{
    dbgpipe("Child init start.\n");
    if (!initPipes())
    {
        dbgpipe("Child init failed.\n");
        return 0;
    } /* if */

//    signal(SIGPIPE, SIG_IGN);

    dbgpipe("Child init success!\n");
    return 1;
} /* STEAMSHIM_init */

void STEAMSHIM_deinit(void)
{
    dbgpipe("Child deinit.\n");
    if (GPipeWrite != NULLPIPE)
    {
        writeBye();
        closePipe(GPipeWrite);
    } /* if */

    if (GPipeRead != NULLPIPE)
        closePipe(GPipeRead);

    GPipeRead = GPipeWrite = NULLPIPE;

//    signal(SIGPIPE, SIG_DFL);
} /* STEAMSHIM_deinit */

static inline int isAlive(void)
{
    return ((GPipeRead != NULLPIPE) && (GPipeWrite != NULLPIPE));
} /* isAlive */

static inline int isDead(void)
{
    return !isAlive();
} /* isDead */

int STEAMSHIM_alive(void)
{
    return isAlive();
} /* STEAMSHIM_alive */

static const STEAMSHIM_Event *processEvent(const uint8 *buf, size_t buflen)
{
    static STEAMSHIM_Event event;
    const STEAMSHIM_EventType type = (STEAMSHIM_EventType) *(buf++);
    buflen--;

    memset(&event, '\0', sizeof (event));
    event.type = type;
    event.okay = 1;

    #if DEBUGPIPE
    if (0) {}
    #define PRINTGOTEVENT(x) else if (type == x) printf("Child got " #x ".\n")
    PRINTGOTEVENT(SHIMEVENT_BYE);
    PRINTGOTEVENT(SHIMEVENT_STATSRECEIVED);
    PRINTGOTEVENT(SHIMEVENT_STATSSTORED);
    PRINTGOTEVENT(SHIMEVENT_SETACHIEVEMENT);
    PRINTGOTEVENT(SHIMEVENT_GETACHIEVEMENT);
    PRINTGOTEVENT(SHIMEVENT_RESETSTATS);
    PRINTGOTEVENT(SHIMEVENT_SETSTATI);
    PRINTGOTEVENT(SHIMEVENT_GETSTATI);
    PRINTGOTEVENT(SHIMEVENT_SETSTATF);
    PRINTGOTEVENT(SHIMEVENT_GETSTATF);
    PRINTGOTEVENT(SHIMEVENT_IS_OVERLAY_ACTIVATED);
    PRINTGOTEVENT(SHIMEVENT_WORKSHOP_UPLOAD_RESULT);
    PRINTGOTEVENT(SHIMEVENT_WORKSHOP_UPLOAD_PROGRESS);
    PRINTGOTEVENT(SHIMEVENT_WORKSHOP_QUERY_ITEM_OWNED_RESULT);
    PRINTGOTEVENT(SHIMEVENT_WORKSHOP_QUERY_ITEM_SUBSCRIBED_RESULT);
    #undef PRINTGOTEVENT
    else printf("Child got unknown shimevent %d.\n", (int) type);
    #endif

    switch (type)
    {
        case SHIMEVENT_BYE:
            break;

        case SHIMEVENT_STATSRECEIVED:
        case SHIMEVENT_STATSSTORED:
        case SHIMEVENT_IS_OVERLAY_ACTIVATED:
            if (!buflen) return NULL;
            event.okay = *(buf++) ? 1 : 0;
            break;

        case SHIMEVENT_SETACHIEVEMENT:
            if (buflen < 3) return NULL;
            event.ivalue = *(buf++) ? 1 : 0;
            event.okay = *(buf++) ? 1 : 0;
            strcpy(event.name, (const char *) buf);
            break;

        case SHIMEVENT_GETACHIEVEMENT:
            if (buflen < 10) return NULL;
            event.ivalue = (int) *(buf++);
            if (event.ivalue == 2)
                event.ivalue = event.okay = 0;
            event.epochsecs = (long long unsigned) *((uint64 *) buf);
            buf += sizeof (uint64);
            strcpy(event.name, (const char *) buf);
            break;

        case SHIMEVENT_RESETSTATS:
            if (buflen != 2) return NULL;
            event.ivalue = *(buf++) ? 1 : 0;
            event.okay = *(buf++) ? 1 : 0;
            break;

        case SHIMEVENT_SETSTATI:
        case SHIMEVENT_GETSTATI:
            event.okay = *(buf++) ? 1 : 0;
            event.ivalue = (int) *((int32 *) buf);
            buf += sizeof (int32);
            strcpy(event.name, (const char *) buf);
            break;

        case SHIMEVENT_SETSTATF:
        case SHIMEVENT_GETSTATF:
            event.okay = *(buf++) ? 1 : 0;
            event.fvalue = (int) *((float *) buf);
            buf += sizeof (float);
            strcpy(event.name, (const char *) buf);
            break;

        case SHIMEVENT_WORKSHOP_UPLOAD_RESULT:
            event.ivalue = (int)*(buf++);
            event.okay = event.ivalue == 1;
            event.needs_to_accept_workshop_agreement = (bool)*(buf++);
            break;

        case SHIMEVENT_WORKSHOP_UPLOAD_PROGRESS:
            event.okay = (int)*(buf++);
            event.ivalue = (int)*(buf++);
            break;

        case SHIMEVENT_WORKSHOP_QUERY_ITEM_OWNED_RESULT:
        {
            event.items_owned = {};
            event.items_owned.shim_deserialize(buf, buflen);
            event.okay = event.items_owned.result_code == 1;
            break;
        }

        case SHIMEVENT_WORKSHOP_QUERY_ITEM_SUBSCRIBED_RESULT:
        {
            event.items_subscribed = {};
            event.items_subscribed.shim_deserialize(buf, buflen);
            event.okay = event.items_owned.result_code == 1;
            break;
        }

        default:  /* uh oh */
            return NULL;
    } /* switch */

    return &event;
} /* processEvent */

const STEAMSHIM_Event *STEAMSHIM_pump(void)
{
    static uint8 buf[65536];
    static int br = 0;
    int evlen = (br > 1) ? *reinterpret_cast<uint16_t*>(buf) : 0;

    if (isDead())
        return NULL;

    if (br - 2 < evlen)  /* we have an incomplete commmand. Try to read more. */
    {
        if (pipeReady(GPipeRead))
        {
            const int morebr = readPipe(GPipeRead, buf + br, sizeof (buf) - br);
            if (morebr > 0)
                br += morebr;
            else  /* uh oh */
            {
                dbgpipe("Child readPipe failed! Shutting down.\n");
                STEAMSHIM_deinit();   /* kill it all. */
            } /* else */
        } /* if */
    } /* if */

    if (evlen && (br - 2 >= evlen))
    {
        const STEAMSHIM_Event *retval = processEvent(buf+2, evlen);
        br -= evlen + 2;
        if (br > 0)
            memmove(buf, buf+evlen+2, br);
        return retval;
    } /* if */

    /* Run Steam event loop. */
    if (br == 0)
    {
        dbgpipe("Child sending SHIMCMD_PUMP().\n");
        write1ByteCmd(SHIMCMD_PUMP);
    } /* if */

    return NULL;
} /* STEAMSHIM_pump */

void STEAMSHIM_requestStats(void)
{
    if (isDead()) return;
    dbgpipe("Child sending SHIMCMD_REQUESTSTATS().\n");
    write1ByteCmd(SHIMCMD_REQUESTSTATS);
} /* STEAMSHIM_requestStats */

void STEAMSHIM_storeStats(void)
{
    if (isDead()) return;
    dbgpipe("Child sending SHIMCMD_STORESTATS().\n");
    write1ByteCmd(SHIMCMD_STORESTATS);
} /* STEAMSHIM_storeStats */

void STEAMSHIM_setAchievement(const char *name, const int enable)
{
    uint8 buf[256];
    uint8 *ptr = buf+2;
    if (isDead()) return;
    dbgpipe("Child sending SHIMCMD_SETACHIEVEMENT('%s', %senable).\n", name, enable ? "" : "!");
    *(ptr++) = (uint8) SHIMCMD_SETACHIEVEMENT;
    *(ptr++) = enable ? 1 : 0;
    strcpy((char *) ptr, name);
    ptr += strlen(name) + 1;
    uint16_t length = ((ptr - 1) - buf);
    uint8* length_bytes = (uint8*)&length;
    buf[0] = length_bytes[0];
    buf[1] = length_bytes[1];
    writePipe(GPipeWrite, buf, length + 2);
} /* STEAMSHIM_setAchievement */

void STEAMSHIM_getAchievement(const char *name)
{
    uint8 buf[256];
    uint8 *ptr = buf+2;
    if (isDead()) return;
    dbgpipe("Child sending SHIMCMD_GETACHIEVEMENT('%s').\n", name);
    *(ptr++) = (uint8) SHIMCMD_GETACHIEVEMENT;
    strcpy((char *) ptr, name);
    ptr += strlen(name) + 1;
    uint16_t length = ((ptr - 1) - buf);
    uint8* length_bytes = (uint8*)&length;
    buf[0] = length_bytes[0];
    buf[1] = length_bytes[1];
    writePipe(GPipeWrite, buf, length + 2);
} /* STEAMSHIM_getAchievement */

void STEAMSHIM_uploadWorkshopItem(const item_upload_data& item)
{
    if (isDead()) return;
    dbgpipe("Child sending SHIMCMD_UPLOADWORKSHOP.\n");

    auto data_stream = item.shim_serialize();
    data_stream = std::ostringstream() << (uint8)SHIMCMD_WORKSHOP_UPLOAD << data_stream.str();
    
    std::ostringstream data_stream_shim;

    uint16_t length = data_stream.str().length();
    data_stream_shim.write(reinterpret_cast<const char*>(&length), sizeof(length));
    data_stream_shim << data_stream.str();

    auto buffer = data_stream_shim.str();
    writePipe(GPipeWrite, buffer.data(), buffer.length());
}

void STEAMSHIM_queryWorkshopItemOwned(const std::string& scenario_name)
{
    if (isDead()) return;

    dbgpipe("Child sending SHIMCMD_WORKSHOP_QUERY_ITEM_OWNED.\n");

    std::ostringstream data_stream;
    data_stream << (uint8)SHIMCMD_WORKSHOP_QUERY_ITEM_OWNED << scenario_name << '\0';

    std::ostringstream data_stream_shim;

    uint16_t length = data_stream.str().length();
    data_stream_shim.write(reinterpret_cast<const char*>(&length), sizeof(length));
    data_stream_shim << data_stream.str();

    auto buffer = data_stream_shim.str();
    writePipe(GPipeWrite, buffer.data(), buffer.length());
}

void STEAMSHIM_queryWorkshopItemMod(const std::string& scenario_name)
{
    if (isDead()) return;

    dbgpipe("Child sending SHIMCMD_WORKSHOP_QUERY_ITEM_MOD.\n");

    std::ostringstream data_stream;
    data_stream << (uint8)SHIMCMD_WORKSHOP_QUERY_ITEM_MOD << scenario_name << '\0';

    std::ostringstream data_stream_shim;

    uint16_t length = data_stream.str().length();
    data_stream_shim.write(reinterpret_cast<const char*>(&length), sizeof(length));
    data_stream_shim << data_stream.str();

    auto buffer = data_stream_shim.str();
    writePipe(GPipeWrite, buffer.data(), buffer.length());
}

void STEAMSHIM_queryWorkshopItemScenario()
{
    if (isDead()) return;
    dbgpipe("Child sending SHIMCMD_WORKSHOP_QUERY_ITEM_SCENARIO.\n");
    write1ByteCmd(SHIMCMD_WORKSHOP_QUERY_ITEM_SCENARIO);
}

void STEAMSHIM_resetStats(const int bAlsoAchievements)
{
    if (isDead()) return;
    dbgpipe("Child sending SHIMCMD_RESETSTATS(%salsoAchievements).\n", bAlsoAchievements ? "" : "!");
    write2ByteCmd(SHIMCMD_RESETSTATS, bAlsoAchievements ? 1 : 0);
} /* STEAMSHIM_resetStats */

static void writeStatThing(const ShimCmd cmd, const char *name, const void *val, const size_t vallen)
{
    uint8 buf[256];
    uint8 *ptr = buf+2;
    if (isDead()) return;
    *(ptr++) = (uint8) cmd;
    if (vallen)
    {
        memcpy(ptr, val, vallen);
        ptr += vallen;
    } /* if */
    strcpy((char *) ptr, name);
    ptr += strlen(name) + 1;
    uint16_t length = ((ptr - 1) - buf);
    uint8* length_bytes = (uint8*)&length;
    buf[0] = length_bytes[0];
    buf[1] = length_bytes[1];
    writePipe(GPipeWrite, buf, length + 2);
} /* writeStatThing */

void STEAMSHIM_setStatI(const char *name, const int _val)
{
    const int32 val = (int32) _val;
    dbgpipe("Child sending SHIMCMD_SETSTATI('%s', val %d).\n", name, val);
    writeStatThing(SHIMCMD_SETSTATI, name, &val, sizeof (val));
} /* STEAMSHIM_setStatI */

void STEAMSHIM_getStatI(const char *name)
{
    dbgpipe("Child sending SHIMCMD_GETSTATI('%s').\n", name);
    writeStatThing(SHIMCMD_GETSTATI, name, NULL, 0);
} /* STEAMSHIM_getStatI */

void STEAMSHIM_setStatF(const char *name, const float val)
{
    dbgpipe("Child sending SHIMCMD_SETSTATF('%s', val %f).\n", name, val);
    writeStatThing(SHIMCMD_SETSTATF, name, &val, sizeof (val));
} /* STEAMSHIM_setStatF */

void STEAMSHIM_getStatF(const char *name)
{
    dbgpipe("Child sending SHIMCMD_GETSTATF('%s').\n", name);
    writeStatThing(SHIMCMD_GETSTATF, name, NULL, 0);
} /* STEAMSHIM_getStatF */

/* end of steamshim_child.cpp ... */

