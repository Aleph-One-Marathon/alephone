#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
typedef PROCESS_INFORMATION ProcessType;
typedef HANDLE PipeType;
#define NULLPIPE NULL
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
typedef pid_t ProcessType;
typedef int PipeType;
#define NULLPIPE -1
#endif

#include <string>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

namespace fs = boost::filesystem;

#include "steam/steam_api.h"
#include <sstream>

#define DEBUGPIPE 1
#if DEBUGPIPE
#define dbgpipe printf
#else
static inline void dbgpipe(const char *fmt, ...) {}
#endif

/* platform-specific mainline calls this. */
static int mainline(void);

/* Windows and Unix implementations of this stuff below. */
static void fail(const char *err);
static bool writePipe(PipeType fd, const void *buf, const unsigned int _len);
static int readPipe(PipeType fd, void *buf, const unsigned int _len);
static bool createPipes(PipeType *pPipeParentRead, PipeType *pPipeParentWrite,
                        PipeType *pPipeChildRead, PipeType *pPipeChildWrite);
static void closePipe(PipeType fd);
static bool setEnvVar(const char *key, const char *val);
static bool launchChild(ProcessType *pid);
static int closeProcess(ProcessType *pid);

static fs::path findExe(const boost::regex& regex);
static fs::path findApp(const boost::regex& regex);

#ifdef _WIN32

static LPWSTR LpCmdLine = NULL;

static void fail(const char *err)
{
    MessageBoxA(NULL, err, "ERROR", MB_ICONERROR | MB_OK);
    ExitProcess(1);
} // fail

static bool writePipe(PipeType fd, const void *buf, const unsigned int _len)
{
    const DWORD len = (DWORD) _len;
    DWORD bw = 0;
    return ((WriteFile(fd, buf, len, &bw, NULL) != 0) && (bw == len));
} // writePipe

static int readPipe(PipeType fd, void *buf, const unsigned int _len)
{
    const DWORD len = (DWORD) _len;
    DWORD br = 0;
    return ReadFile(fd, buf, len, &br, NULL) ? (int) br : -1;
} // readPipe

static bool createPipes(PipeType *pPipeParentRead, PipeType *pPipeParentWrite,
                        PipeType *pPipeChildRead, PipeType *pPipeChildWrite)
{
    SECURITY_ATTRIBUTES pipeAttr;

    pipeAttr.nLength = sizeof (pipeAttr);
    pipeAttr.lpSecurityDescriptor = NULL;
    pipeAttr.bInheritHandle = TRUE;
    if (!CreatePipe(pPipeParentRead, pPipeChildWrite, &pipeAttr, 0))
        return 0;

    pipeAttr.nLength = sizeof (pipeAttr);
    pipeAttr.lpSecurityDescriptor = NULL;
    pipeAttr.bInheritHandle = TRUE;
    if (!CreatePipe(pPipeChildRead, pPipeParentWrite, &pipeAttr, 0))
    {
        CloseHandle(*pPipeParentRead);
        CloseHandle(*pPipeChildWrite);
        return 0;
    } // if

    return 1;
} // createPipes

static void closePipe(PipeType fd)
{
    CloseHandle(fd);
} // closePipe

static bool setEnvVar(const char *key, const char *val)
{
    return (SetEnvironmentVariableA(key, val) != 0);
} // setEnvVar

static bool launchChild(ProcessType *pid)
{
    STARTUPINFOW si;
    memset(&si, 0, sizeof(si));
    auto exe = findExe(boost::regex("Classic Marathon.*\\.exe"));

    std::wstring args = L"\"" + exe.wstring() + L"\" " + (LpCmdLine ? LpCmdLine : L""); //should never be null but just in case

    return (CreateProcessW(exe.wstring().c_str(),
        args.data(), NULL, NULL, TRUE, 0, NULL,
        NULL, &si, pid) != 0);
} // launchChild

static int closeProcess(ProcessType *pid)
{
    CloseHandle(pid->hProcess);
    CloseHandle(pid->hThread);
    return 0;
} // closeProcess

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    LpCmdLine = lpCmdLine;
    mainline();
    ExitProcess(0);
    return 0;  // just in case.
} // WinMain


#else  // everyone else that isn't Windows.

static void fail(const char *err)
{
    // !!! FIXME: zenity or something.
    fprintf(stderr, "%s\n", err);
    _exit(1);
} // fail

static bool writePipe(PipeType fd, const void *buf, const unsigned int _len)
{
    const ssize_t len = (ssize_t) _len;
    ssize_t bw;
    while (((bw = write(fd, buf, len)) == -1) && (errno == EINTR)) { /*spin*/ }
    return (bw == len);
} // writePipe

static int readPipe(PipeType fd, void *buf, const unsigned int _len)
{
    const ssize_t len = (ssize_t) _len;
    ssize_t br;
    while (((br = read(fd, buf, len)) == -1) && (errno == EINTR)) { /*spin*/ }
    return (int) br;
} // readPipe

static bool createPipes(PipeType *pPipeParentRead, PipeType *pPipeParentWrite,
                        PipeType *pPipeChildRead, PipeType *pPipeChildWrite)
{
    int fds[2];
    if (pipe(fds) == -1)
        return 0;
    *pPipeParentRead = fds[0];
    *pPipeChildWrite = fds[1];

    if (pipe(fds) == -1)
    {
        close(*pPipeParentRead);
        close(*pPipeChildWrite);
        return 0;
    } // if

    *pPipeChildRead = fds[0];
    *pPipeParentWrite = fds[1];

    return 1;
} // createPipes

static void closePipe(PipeType fd)
{
    close(fd);
} // closePipe

static bool setEnvVar(const char *key, const char *val)
{
    return (setenv(key, val, 1) != -1);
} // setEnvVar

static int GArgc = 0;
static char **GArgv = NULL;

static bool launchChild(ProcessType *pid)
{
    *pid = fork();
    if (*pid == -1)   // failed
        return false;
    else if (*pid != 0)  // we're the parent
        return true;  // we'll let the pipe fail if this didn't work.

    // we're the child.
#ifdef __APPLE__
    auto app = findApp(boost::regex("Classic Marathon.*\\.app"));
    auto macos = app / "Contents" / "MacOS";
    auto bin = boost::filesystem::directory_iterator(macos)->path().string();
#else
    auto bin = findExe(boost::regex("Classic Marathon.*"));
#endif

    GArgv[0] = strdup(bin.c_str());
    execvp(GArgv[0], GArgv);
    // still here? It failed! Terminate, closing child's ends of the pipes.
    _exit(1);
} // launchChild

static int closeProcess(ProcessType *pid)
{
    int rc = 0;
    while ((waitpid(*pid, &rc, 0) == -1) && (errno == EINTR)) { /*spin*/ }
    if (!WIFEXITED(rc))
        return 1;  // oh well.
    return WEXITSTATUS(rc);
} // closeProcess

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    GArgc = argc;
    GArgv = argv;
    return mainline();
} // main

#endif

fs::path findExe(const boost::regex& regex)
{
    auto this_exe = boost::dll::program_location();

    fs::directory_iterator end;
    for (fs::directory_iterator it(this_exe.parent_path()); it != end; ++it) {
        if (it->path() == this_exe) {
            continue;
        }

        auto filename = it->path().filename().string();
        if (boost::regex_match(filename, regex)) {
            return it->path();
        }
    }

    return fs::path();
}

fs::path findApp(const boost::regex& regex)
{
    auto this_exe = boost::dll::program_location();

    fs::directory_iterator end;
    for (fs::directory_iterator it(this_exe.parent_path()); it != end; ++it) {
        auto filename = it->path().filename().string();
        if (boost::regex_match(filename, regex)) {
            return it->path();
        }
    }

    return fs::path();
}

// THE ACTUAL PROGRAM.

class SteamBridge;

static ISteamUserStats *GSteamStats = NULL;
static ISteamUtils *GSteamUtils = NULL;
static ISteamUser *GSteamUser = NULL;
static ISteamUGC *GSteamUGC = NULL;
static ISteamFriends* GSteamFriends = NULL;
static AppId_t GAppID = 0;
static uint64 GUserID = 0;
static SteamBridge *GSteamBridge = NULL;

namespace SteamItemTags
{
    constexpr const char* ItemType = "ItemType";
    constexpr const char* ContentType = "ContentType";
    constexpr const char* RequiredScenario = "RequiredScenario";
}

enum class ItemType {
    Scenario,
    Plugin,
    Map,
    Physics,
    Script,
    Sounds,
    Shapes
};

enum class ContentType {
    START_SCENARIO = 0,
    None = 0,

    START_PLUGIN = 16,
    Graphics = 16,
    HUD,
    Music,
    Script,
    Theme,

    START_OTHER = 64,
    SoloAndNet = 64,
    Solo,
    Net
};

struct item_subscribed_query_result
{
    struct item
    {
        uint64_t id;
        ItemType item_type;
        ContentType content_type;
        std::string install_folder_path;
    };

    EResult result_code;
    std::vector<item> items;

    std::ostringstream shim_serialize() const
    {
        std::ostringstream data_stream;
        data_stream.write(reinterpret_cast<const char*>(&result_code), sizeof(result_code));

        int number_items = items.size();
        data_stream.write(reinterpret_cast<const char*>(&number_items), sizeof(number_items));

        for (const auto& item : items)
        {
            data_stream.write(reinterpret_cast<const char*>(&item.id), sizeof(item.id));
            data_stream.write(reinterpret_cast<const char*>(&item.item_type), sizeof(item.item_type));
            data_stream.write(reinterpret_cast<const char*>(&item.content_type), sizeof(item.content_type));
            data_stream << item.install_folder_path << '\0';
        }

        return data_stream;
    }
};

struct item_owned_query_result
{
    struct item
    {
        uint64_t id;
        ItemType item_type;
        ContentType content_type;
        bool is_scenarios_compatible;
        std::string title;
    };

    EResult result_code;
    std::vector<item> items;

    std::ostringstream shim_serialize() const
    {
        std::ostringstream data_stream;
        data_stream.write(reinterpret_cast<const char*>(&result_code), sizeof(result_code));

        int number_items = items.size();
        data_stream.write(reinterpret_cast<const char*>(&number_items), sizeof(number_items));

        for (const auto& item : items)
        {
            data_stream.write(reinterpret_cast<const char*>(&item.id), sizeof(item.id));
            data_stream.write(reinterpret_cast<const char*>(&item.item_type), sizeof(item.item_type));
            data_stream.write(reinterpret_cast<const char*>(&item.content_type), sizeof(item.content_type));
            data_stream.write(reinterpret_cast<const char*>(&item.is_scenarios_compatible), sizeof(item.is_scenarios_compatible));
            data_stream << item.title << '\0';
        }

        return data_stream;
    }
};

struct item_upload_data {

    uint64_t id;
    ItemType item_type;
    ContentType content_type;
    std::string directory_path;
    std::string thumbnail_path;
    std::string required_scenario;

    void shim_deserialize(const uint8* buf, unsigned int buflen)
    {
        std::istringstream iss(std::string((const char*)buf, buflen));
        iss.read(reinterpret_cast<char*>(&id), sizeof(id));
        iss.read(reinterpret_cast<char*>(&item_type), sizeof(item_type));
        iss.read(reinterpret_cast<char*>(&content_type), sizeof(content_type));
        std::getline(iss, directory_path, '\0');
        std::getline(iss, thumbnail_path, '\0');
        std::getline(iss, required_scenario, '\0');
    }
};

class SteamBridge
{
public:
    SteamBridge(PipeType _fd);
	STEAM_CALLBACK(SteamBridge, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived);
	STEAM_CALLBACK(SteamBridge, OnUserStatsStored, UserStatsStored_t, m_CallbackUserStatsStored);
    STEAM_CALLBACK(SteamBridge, OnOverlayActivated, GameOverlayActivated_t, m_CallbackOverlayActivated);
    void set_item_created_callback(SteamAPICall_t api_call, const item_upload_data& item_data);
    void set_item_updated_callback(SteamAPICall_t api_call, UGCUpdateHandle_t handle, const item_upload_data& item_data);
    void set_item_owned_queried_callback(SteamAPICall_t api_call, const std::string& scenario_name);
    void set_item_mod_queried_callback(SteamAPICall_t api_call, const std::string& scenario_name);
    void set_item_scenario_queried_callback(SteamAPICall_t api_call);
    void idle();
private:
    PipeType fd;
    CCallResult<SteamBridge, CreateItemResult_t> m_CallbackItemCreatedResult;
    CCallResult<SteamBridge, SubmitItemUpdateResult_t> m_CallbackItemUpdatedResult;
    CCallResult<SteamBridge, SteamUGCQueryCompleted_t> m_CallbackItemQueryCompleted;
    void OnItemCreated(CreateItemResult_t* pCallback, bool bIOFailure);
    void OnItemUpdated(SubmitItemUpdateResult_t* pCallback, bool bIOFailure);
    void OnItemOwnedQueried(SteamUGCQueryCompleted_t* pCallback, bool bIOFailure);
    void OnItemScenarioQueried(SteamUGCQueryCompleted_t* pCallback, bool bIOFailure);
    void OnItemModQueried(SteamUGCQueryCompleted_t* pCallback, bool bIOFailure);
    item_upload_data m_upload_item_data;
    UGCUpdateHandle_t m_update_handle = 0xffffffffffffffffull;
    std::string m_scenario_name;
};

typedef enum ShimCmd
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
} ShimCmd;

typedef enum ShimEvent
{
    SHIMEVENT_BYE,
    SHIMEVENT_STATSRECEIVED,
    SHIMEVENT_STATSSTORED,
    SHIMEVENT_SETACHIEVEMENT,
    SHIMEVENT_GETACHIEVEMENT,
    SHIMEVENT_RESETSTATS,
    SHIMEVENT_SETSTATI,
    SHIMEVENT_GETSTATI,
    SHIMEVENT_SETSTATF,
    SHIMEVENT_GETSTATF,
    SHIMEVENT_IS_OVERLAY_ACTIVATED,
    SHIMEVENT_WORKSHOP_UPLOAD_RESULT,
    SHIMEVENT_WORKSHOP_UPLOAD_PROGRESS,
    SHIMEVENT_WORKSHOP_QUERY_OWNED_ITEM_RESULT,
    SHIMEVENT_WORKSHOP_QUERY_SUBSCRIBED_ITEM_RESULT
} ShimEvent;

static bool write1ByteCmd(PipeType fd, const uint8 b1)
{
    uint16_t length = 1;
    uint8* length_bytes = (uint8*)&length;
    const uint8 buf[] = { length_bytes[0], length_bytes[1], b1 };
    return writePipe(fd, buf, sizeof (buf));
} // write1ByteCmd

static bool write2ByteCmd(PipeType fd, const uint8 b1, const uint8 b2)
{
    uint16_t length = 2;
    uint8* length_bytes = (uint8*)&length;
    const uint8 buf[] = { length_bytes[0], length_bytes[1], b1, b2 };
    return writePipe(fd, buf, sizeof (buf));
} // write2ByteCmd

static bool write3ByteCmd(PipeType fd, const uint8 b1, const uint8 b2, const uint8 b3)
{
    uint16_t length = 3;
    uint8* length_bytes = (uint8*)&length;
    const uint8 buf[] = { length_bytes[0], length_bytes[1], b1, b2, b3 };
    return writePipe(fd, buf, sizeof (buf));
} // write3ByteCmd


static inline bool writeBye(PipeType fd)
{
    dbgpipe("Parent sending SHIMEVENT_BYE().\n");
    return write1ByteCmd(fd, SHIMEVENT_BYE);
} // writeBye

static inline bool writeStatsReceived(PipeType fd, const bool okay)
{
    dbgpipe("Parent sending SHIMEVENT_STATSRECEIVED(%sokay).\n", okay ? "" : "!");
    return write2ByteCmd(fd, SHIMEVENT_STATSRECEIVED, okay ? 1 : 0);
} // writeStatsReceived

static inline bool writeStatsStored(PipeType fd, const bool okay)
{
    dbgpipe("Parent sending SHIMEVENT_STATSSTORED(%sokay).\n", okay ? "" : "!");
    return write2ByteCmd(fd, SHIMEVENT_STATSSTORED, okay ? 1 : 0);
} // writeStatsStored

static inline bool writeOverlayActivated(PipeType fd, const bool okay)
{
    dbgpipe("Parent sending SHIMEVENT_IS_OVERLAY_ACTIVATED(%sokay).\n", okay ? "" : "!");
    return write2ByteCmd(fd, SHIMEVENT_IS_OVERLAY_ACTIVATED, okay ? 1 : 0);
} // writeOverlayActivated

static inline bool writeWorkshopUploadResult(PipeType fd, const EResult result, bool needs_accept_workshop_agreement)
{
    dbgpipe("Parent sending SHIMEVENT_WORKSHOP_UPLOAD_RESULT(%d result %d workshop agreement).\n", result, needs_accept_workshop_agreement);
    return write3ByteCmd(fd, SHIMEVENT_WORKSHOP_UPLOAD_RESULT, result, needs_accept_workshop_agreement);
} // writeOverlayActivated

static bool writeWorkshopItemOwnedQueriedResult(PipeType fd, const item_owned_query_result& query_result)
{
    dbgpipe("Parent sending SHIMEVENT_WORKSHOP_QUERY_OWNED_ITEM_RESULT(%d result).\n", query_result.result_code);
    auto data_stream = query_result.shim_serialize();

    data_stream = std::ostringstream() << (uint8)SHIMEVENT_WORKSHOP_QUERY_OWNED_ITEM_RESULT << data_stream.str();
    
    std::ostringstream data_stream_shim;

    uint16_t length = data_stream.str().length();
    data_stream_shim.write(reinterpret_cast<const char*>(&length), sizeof(length));
    data_stream_shim << data_stream.str();

    auto buffer = data_stream_shim.str();
    return writePipe(fd, buffer.data(), buffer.length());
} // writeOverlayActivated

static bool writeWorkshopItemQueriedResult(PipeType fd, const item_subscribed_query_result& query_result)
{
    dbgpipe("Parent sending SHIMEVENT_WORKSHOP_QUERY_SUBSCRIBED_ITEM_RESULT(%d result).\n", query_result.result_code);
    auto data_stream = query_result.shim_serialize();

    data_stream = std::ostringstream() << (uint8)SHIMEVENT_WORKSHOP_QUERY_SUBSCRIBED_ITEM_RESULT << data_stream.str();

    std::ostringstream data_stream_shim;

    uint16_t length = data_stream.str().length();
    data_stream_shim.write(reinterpret_cast<const char*>(&length), sizeof(length));
    data_stream_shim << data_stream.str();

    auto buffer = data_stream_shim.str();
    return writePipe(fd, buffer.data(), buffer.length());
} // writeOverlayActivated

static bool writeAchievementSet(PipeType fd, const char *name, const bool enable, const bool okay)
{
    uint8 buf[256];
    uint8 *ptr = buf+2;
    dbgpipe("Parent sending SHIMEVENT_SETACHIEVEMENT('%s', %senable, %sokay).\n", name, enable ? "" : "!", okay ? "" : "!");
    *(ptr++) = (uint8) SHIMEVENT_SETACHIEVEMENT;
    *(ptr++) = enable ? 1 : 0;
    *(ptr++) = okay ? 1 : 0;
    strcpy((char *) ptr, name);
    ptr += strlen(name) + 1;
    uint16_t length = ((ptr - 1) - buf);
    uint8* length_bytes = (uint8*)&length;
    buf[0] = length_bytes[0];
    buf[1] = length_bytes[1];
    return writePipe(fd, buf, length + 2);
} // writeAchievementSet

static bool writeAchievementGet(PipeType fd, const char *name, const int status, const uint64 time)
{
    uint8 buf[256];
    uint8 *ptr = buf+2;
    dbgpipe("Parent sending SHIMEVENT_GETACHIEVEMENT('%s', status %d, time %llu).\n", name, status, (unsigned long long) time);
    *(ptr++) = (uint8) SHIMEVENT_GETACHIEVEMENT;
    *(ptr++) = (uint8) status;
    memcpy(ptr, &time, sizeof (time));
    ptr += sizeof (time);
    strcpy((char *) ptr, name);
    ptr += strlen(name) + 1;
    uint16_t length = ((ptr - 1) - buf);
    uint8* length_bytes = (uint8*)&length;
    buf[0] = length_bytes[0];
    buf[1] = length_bytes[1];
    return writePipe(fd, buf, length + 2);
} // writeAchievementGet

static inline bool writeResetStats(PipeType fd, const bool alsoAch, const bool okay)
{
    dbgpipe("Parent sending SHIMEVENT_RESETSTATS(%salsoAchievements, %sokay).\n", alsoAch ? "" : "!", okay ? "" : "!");
    return write3ByteCmd(fd, SHIMEVENT_RESETSTATS, alsoAch ? 1 : 0, okay ? 1 : 0);
} // writeResetStats

static bool writeStatThing(PipeType fd, const ShimEvent ev, const char *name, const void *val, const size_t vallen, const bool okay)
{
    uint8 buf[256];
    uint8 *ptr = buf+2;
    *(ptr++) = (uint8) ev;
    *(ptr++) = okay ? 1 : 0;
    memcpy(ptr, val, vallen);
    ptr += vallen;
    strcpy((char *) ptr, name);
    ptr += strlen(name) + 1;
    uint16_t length = ((ptr - 1) - buf);
    uint8* length_bytes = (uint8*)&length;
    buf[0] = length_bytes[0];
    buf[1] = length_bytes[1];
    return writePipe(fd, buf, length + 2);
} // writeStatThing

static inline bool writeSetStatI(PipeType fd, const char *name, const int32 val, const bool okay)
{
    dbgpipe("Parent sending SHIMEVENT_SETSTATI('%s', val %d, %sokay).\n", name, (int) val, okay ? "" : "!");
    return writeStatThing(fd, SHIMEVENT_SETSTATI, name, &val, sizeof (val), okay);
} // writeSetStatI

static inline bool writeSetStatF(PipeType fd, const char *name, const float val, const bool okay)
{
    dbgpipe("Parent sending SHIMEVENT_SETSTATF('%s', val %f, %sokay).\n", name, val, okay ? "" : "!");
    return writeStatThing(fd, SHIMEVENT_SETSTATF, name, &val, sizeof (val), okay);
} // writeSetStatF

static inline bool writeGetStatI(PipeType fd, const char *name, const int32 val, const bool okay)
{
    dbgpipe("Parent sending SHIMEVENT_GETSTATI('%s', val %d, %sokay).\n", name, (int) val, okay ? "" : "!");
    return writeStatThing(fd, SHIMEVENT_GETSTATI, name, &val, sizeof (val), okay);
} // writeGetStatI

static inline bool writeGetStatF(PipeType fd, const char *name, const float val, const bool okay)
{
    dbgpipe("Parent sending SHIMEVENT_GETSTATF('%s', val %f, %sokay).\n", name, val, okay ? "" : "!");
    return writeStatThing(fd, SHIMEVENT_GETSTATF, name, &val, sizeof (val), okay);
} // writeGetStatF

static std::vector<std::string> GetTagsForItemType(ItemType item_type, ContentType content_type)
{
    std::vector<std::string> tags;

    switch (item_type)
    {
        case ItemType::Scenario:
            tags.push_back("Scenario");
            break;
        case ItemType::Plugin:
            tags.push_back("Plugin");
            switch (content_type)
            {
                case ContentType::Graphics:
                    tags.push_back("Graphics");
                    break;
                case ContentType::HUD:
                    tags.push_back("HUD");
                    break;
                case ContentType::Music:
                    tags.push_back("Music");
                    break;
                case ContentType::Script:
                    tags.push_back("Script");
                    break;
                case ContentType::Theme:
                    tags.push_back("Theme");
                    break;
            }
            break;
        case ItemType::Map:
            switch (content_type)
            {
                case ContentType::Solo:
                    tags.push_back("Solo Map");
                    break;
                case ContentType::Net:
                    tags.push_back("Net Map");
                    break;
                case ContentType::SoloAndNet:
                    tags.push_back("Solo Map");
                    tags.push_back("Net Map");
                    break;
            }
            break;
        case ItemType::Physics:
            tags.push_back("Physics");
            break;
        case ItemType::Script:
            switch (content_type)
            {
                case ContentType::Solo:
                    tags.push_back("Solo Script");
                    break;
                case ContentType::Net:
                    tags.push_back("Net Script");
                    break;
                case ContentType::SoloAndNet:
                    tags.push_back("Solo Script");
                    tags.push_back("Net Script");
                    break;
            }
            break;
        case ItemType::Sounds:
            tags.push_back("Sounds");
            break;
        case ItemType::Shapes:
            tags.push_back("Shapes");
            break;
        default:
            break;
    }

    return tags;
}

static void UpdateItem(PublishedFileId_t item_id, const item_upload_data& item_data)
{
    UGCUpdateHandle_t updateHandle = GSteamUGC->StartItemUpdate(GAppID, item_id);

    if (item_data.id) //existing item
    {
        GSteamUGC->RemoveItemKeyValueTags(updateHandle, SteamItemTags::RequiredScenario);
        GSteamUGC->RemoveItemKeyValueTags(updateHandle, SteamItemTags::ContentType); //let at least users change solo/net stuff
    }
    else //new item
    {
        std::string title = "New Item " + std::to_string(item_id);
        std::string item_type = std::to_string(static_cast<int>(item_data.item_type));
        GSteamUGC->SetItemTitle(updateHandle, title.c_str());
        GSteamUGC->AddItemKeyValueTag(updateHandle, SteamItemTags::ItemType, item_type.c_str());
        GSteamUGC->SetItemVisibility(updateHandle, k_ERemoteStoragePublishedFileVisibilityPrivate);
    }

    std::string content_type = std::to_string(static_cast<int>(item_data.content_type));
    GSteamUGC->AddItemKeyValueTag(updateHandle, SteamItemTags::ContentType, content_type.c_str());

    auto tags = GetTagsForItemType(item_data.item_type, item_data.content_type);
    const char* tag_array[16];

    for (auto i = 0; i < tags.size(); i++)
    {
        tag_array[i] = tags[i].c_str();
    }

    SteamParamStringArray_t steam_tags = { tag_array, tags.size() };
    GSteamUGC->SetItemTags(updateHandle, &steam_tags);

    if (!item_data.directory_path.empty())
    {
        GSteamUGC->SetItemContent(updateHandle, item_data.directory_path.c_str());
    }

    if (!item_data.thumbnail_path.empty())
    {
        GSteamUGC->SetItemPreview(updateHandle, item_data.thumbnail_path.c_str());
    }

    if (!item_data.required_scenario.empty())
    {
        GSteamUGC->AddItemKeyValueTag(updateHandle, SteamItemTags::RequiredScenario, item_data.required_scenario.c_str());
    }

    auto steam_api_call = GSteamUGC->SubmitItemUpdate(updateHandle, nullptr);
    GSteamBridge->set_item_updated_callback(steam_api_call, updateHandle, item_data);
}

SteamBridge::SteamBridge(PipeType _fd)
    : m_CallbackUserStatsReceived( this, &SteamBridge::OnUserStatsReceived )
	, m_CallbackUserStatsStored( this, &SteamBridge::OnUserStatsStored )
    , m_CallbackOverlayActivated( this, &SteamBridge::OnOverlayActivated )
	, fd(_fd)
{
} // SteamBridge::SteamBridge

void SteamBridge::OnUserStatsReceived(UserStatsReceived_t *pCallback)
{
	if (GAppID != pCallback->m_nGameID) return;
	if (GUserID != pCallback->m_steamIDUser.ConvertToUint64()) return;
    writeStatsReceived(fd, pCallback->m_eResult == k_EResultOK);
} // SteamBridge::OnUserStatsReceived

void SteamBridge::OnUserStatsStored(UserStatsStored_t *pCallback)
{
	if (GAppID != pCallback->m_nGameID) return;
    writeStatsStored(fd, pCallback->m_eResult == k_EResultOK);
} // SteamBridge::OnUserStatsStored

void SteamBridge::OnOverlayActivated(GameOverlayActivated_t *pCallback)
{
    if (GAppID != pCallback->m_nAppID) return;
    writeOverlayActivated(fd, pCallback->m_bActive);
} // SteamBridge::OnOverlayActivated

void SteamBridge::set_item_created_callback(SteamAPICall_t api_call, const item_upload_data& item_data)
{
    m_upload_item_data = item_data;
    m_CallbackItemCreatedResult.Set(api_call, this, &SteamBridge::OnItemCreated);
}

void SteamBridge::set_item_updated_callback(SteamAPICall_t api_call, UGCUpdateHandle_t handle, const item_upload_data& item_data)
{
    m_upload_item_data = item_data;
    m_update_handle = handle;
    m_CallbackItemUpdatedResult.Set(api_call, this, &SteamBridge::OnItemUpdated);
}

void SteamBridge::set_item_owned_queried_callback(SteamAPICall_t api_call, const std::string& scenario_name)
{
    m_scenario_name = scenario_name;
    m_CallbackItemQueryCompleted.Set(api_call, this, &SteamBridge::OnItemOwnedQueried);
}

void SteamBridge::set_item_scenario_queried_callback(SteamAPICall_t api_call)
{
    m_CallbackItemQueryCompleted.Set(api_call, this, &SteamBridge::OnItemScenarioQueried);
}

void SteamBridge::set_item_mod_queried_callback(SteamAPICall_t api_call, const std::string& scenario_name)
{
    m_scenario_name = scenario_name;
    m_CallbackItemQueryCompleted.Set(api_call, this, &SteamBridge::OnItemModQueried);
}

void SteamBridge::idle()
{
    if (m_update_handle != 0xffffffffffffffffull)
    {
        uint64 total_bytes, uploaded_bytes;
        auto status = GSteamUGC->GetItemUpdateProgress(m_update_handle, &uploaded_bytes, &total_bytes);
        if (status != k_EItemUpdateStatusInvalid && total_bytes)
        {
            write3ByteCmd(fd, SHIMEVENT_WORKSHOP_UPLOAD_PROGRESS, status, (uploaded_bytes * 1.f / total_bytes) * 100);
        }
    }
}

void SteamBridge::OnItemCreated(CreateItemResult_t* pCallback, bool bIOFailure)
{
    if (bIOFailure || pCallback->m_eResult != k_EResultOK)
        writeWorkshopUploadResult(fd, pCallback->m_eResult, pCallback->m_bUserNeedsToAcceptWorkshopLegalAgreement);
    else
        UpdateItem(pCallback->m_nPublishedFileId, m_upload_item_data);
}

void SteamBridge::OnItemUpdated(SubmitItemUpdateResult_t* pCallback, bool bIOFailure)
{
    if (bIOFailure || pCallback->m_eResult != k_EResultOK)
    {
        if (!m_upload_item_data.id)
        {
            GSteamUGC->DeleteItem(pCallback->m_nPublishedFileId); //won't care about callback and return value here
        }
    }
    else
    {
        if (!m_upload_item_data.id)
        {
            std::string url = "https://steamcommunity.com/sharedfiles/filedetails/?id=" + std::to_string(pCallback->m_nPublishedFileId);
            GSteamFriends->ActivateGameOverlayToWebPage(url.c_str());
        }
    }

    m_update_handle = 0xffffffffffffffffull;
    writeWorkshopUploadResult(fd, pCallback->m_eResult, pCallback->m_bUserNeedsToAcceptWorkshopLegalAgreement);
}

void SteamBridge::OnItemOwnedQueried(SteamUGCQueryCompleted_t* pCallback, bool bIOFailure)
{
    item_owned_query_result items_result;

    if (!bIOFailure && pCallback->m_eResult == k_EResultOK)
    {
        for (int i = 0; i < pCallback->m_unNumResultsReturned; i++)
        {
            SteamUGCDetails_t item_details;

            if (!GSteamUGC->GetQueryUGCResult(pCallback->m_handle, i, &item_details) && !item_details.m_bBanned)
            {
                continue;
            }

            item_owned_query_result::item item;
            item.is_scenarios_compatible = true;

            char item_kv_tag[256];

            if (!GSteamUGC->GetQueryUGCKeyValueTag(pCallback->m_handle, i, SteamItemTags::ItemType, item_kv_tag, sizeof(item_kv_tag)))
            {
                continue; //ignore item if we can't retrieve its item type tag
            }

            item.item_type = static_cast<ItemType>(std::stoi(item_kv_tag)); //we should probably check better here

            if (!GSteamUGC->GetQueryUGCKeyValueTag(pCallback->m_handle, i, SteamItemTags::ContentType, item_kv_tag, sizeof(item_kv_tag)))
            {
                continue; //ignore item if we can't retrieve its content type tag
            }

            item.content_type = static_cast<ContentType>(std::stoi(item_kv_tag)); //we should probably check better here

            char item_scenario_tag[256];
            if (GSteamUGC->GetQueryUGCKeyValueTag(pCallback->m_handle, i, SteamItemTags::RequiredScenario, item_scenario_tag, sizeof(item_scenario_tag)))
            {
                if (std::strlen(item_scenario_tag) && m_scenario_name != item_scenario_tag)
                {
                    continue; //not compatible item
                }

                item.is_scenarios_compatible = false;
            }

            item.title = item_details.m_rgchTitle;
            item.id = item_details.m_nPublishedFileId;
            items_result.items.push_back(item);
        }
    }

    items_result.result_code = pCallback->m_eResult;
    GSteamUGC->ReleaseQueryUGCRequest(pCallback->m_handle);
    writeWorkshopItemOwnedQueriedResult(fd, items_result);
}

void SteamBridge::OnItemModQueried(SteamUGCQueryCompleted_t* pCallback, bool bIOFailure)
{
    item_subscribed_query_result items_result;

    if (!bIOFailure && pCallback->m_eResult == k_EResultOK)
    {
        for (int i = 0; i < pCallback->m_unNumResultsReturned; i++)
        {
            SteamUGCDetails_t item_details;

            if (!GSteamUGC->GetQueryUGCResult(pCallback->m_handle, i, &item_details))
            {
                continue;
            }

            char item_type_tag[256];
            if (!GSteamUGC->GetQueryUGCKeyValueTag(pCallback->m_handle, i, SteamItemTags::ItemType, item_type_tag, sizeof(item_type_tag)))
            {
                continue; //ignore item if we can't retrieve its type tag
            }

            char content_type_tag[256];
            if (!GSteamUGC->GetQueryUGCKeyValueTag(pCallback->m_handle, i, SteamItemTags::ContentType, content_type_tag, sizeof(content_type_tag)))
            {
                continue; //ignore item if we can't retrieve its content type tag
            }

            item_subscribed_query_result::item item;
            item.item_type = static_cast<ItemType>(std::stoi(item_type_tag)); //we should probably check better here
            item.content_type = static_cast<ContentType>(std::stoi(content_type_tag)); //we should probably check better here

            char item_scenario_tag[256];
            if (GSteamUGC->GetQueryUGCKeyValueTag(pCallback->m_handle, i, SteamItemTags::RequiredScenario, item_scenario_tag, sizeof(item_scenario_tag)))
            {
                if (std::strlen(item_scenario_tag) && m_scenario_name != item_scenario_tag)
                {
                    continue; //not compatible item
                }
            }

            uint64 item_size;
            char folder_path[256];
            uint32 time_stamp;

            if (!GSteamUGC->GetItemInstallInfo(item_details.m_nPublishedFileId, &item_size, folder_path, sizeof(folder_path), &time_stamp))
            {
                continue;
            }

            item.id = item_details.m_nPublishedFileId;
            item.install_folder_path = folder_path;
            items_result.items.push_back(item);
        }
    }

    items_result.result_code = pCallback->m_eResult;
    GSteamUGC->ReleaseQueryUGCRequest(pCallback->m_handle);
    writeWorkshopItemQueriedResult(fd, items_result);
}

void SteamBridge::OnItemScenarioQueried(SteamUGCQueryCompleted_t* pCallback, bool bIOFailure)
{
    item_subscribed_query_result items_result;

    if (!bIOFailure && pCallback->m_eResult == k_EResultOK)
    {
        for (int i = 0; i < pCallback->m_unNumResultsReturned; i++)
        {
            SteamUGCDetails_t item_details;

            if (!GSteamUGC->GetQueryUGCResult(pCallback->m_handle, i, &item_details))
            {
                continue;
            }

            uint64 item_size;
            char folder_path[256];
            uint32 time_stamp;

            if (!GSteamUGC->GetItemInstallInfo(item_details.m_nPublishedFileId, &item_size, folder_path, sizeof(folder_path), &time_stamp))
            {
                continue;
            }

            item_subscribed_query_result::item item;
            item.id = item_details.m_nPublishedFileId;
            item.item_type = ItemType::Scenario;
            item.content_type = ContentType::None;
            item.install_folder_path = folder_path;
            items_result.items.push_back(item);
        }
    }

    items_result.result_code = pCallback->m_eResult;
    GSteamUGC->ReleaseQueryUGCRequest(pCallback->m_handle);
    writeWorkshopItemQueriedResult(fd, items_result);
}

static bool processCommand(const uint8 *buf, unsigned int buflen, PipeType fd)
{
    if (buflen == 0)
        return true;

    const ShimCmd cmd = (ShimCmd) *(buf++);
    buflen--;

    #if DEBUGPIPE
    if (false) {}
    #define PRINTGOTCMD(x) else if (cmd == x) printf("Parent got " #x ".\n")
    PRINTGOTCMD(SHIMCMD_BYE);
    PRINTGOTCMD(SHIMCMD_PUMP);
    PRINTGOTCMD(SHIMCMD_REQUESTSTATS);
    PRINTGOTCMD(SHIMCMD_STORESTATS);
    PRINTGOTCMD(SHIMCMD_SETACHIEVEMENT);
    PRINTGOTCMD(SHIMCMD_GETACHIEVEMENT);
    PRINTGOTCMD(SHIMCMD_RESETSTATS);
    PRINTGOTCMD(SHIMCMD_SETSTATI);
    PRINTGOTCMD(SHIMCMD_GETSTATI);
    PRINTGOTCMD(SHIMCMD_SETSTATF);
    PRINTGOTCMD(SHIMCMD_GETSTATF);
    PRINTGOTCMD(SHIMCMD_WORKSHOP_UPLOAD);
    PRINTGOTCMD(SHIMCMD_WORKSHOP_QUERY_ITEM_OWNED);
    PRINTGOTCMD(SHIMCMD_WORKSHOP_QUERY_ITEM_SCENARIO);
    #undef PRINTGOTCMD
    else printf("Parent got unknown shimcmd %d.\n", (int) cmd);
    #endif

    switch (cmd)
    {
        case SHIMCMD_PUMP:
            SteamAPI_RunCallbacks();
            GSteamBridge->idle();
            break;

        case SHIMCMD_BYE:
            writeBye(fd);
            return false;

        case SHIMCMD_REQUESTSTATS:
            if ((!GSteamStats) || (!GSteamStats->RequestCurrentStats()))
                writeStatsReceived(fd, false);
            // callback later.
            break;

        case SHIMCMD_STORESTATS:
            if ((!GSteamStats) || (!GSteamStats->StoreStats()))
                writeStatsStored(fd, false);
            // callback later.
            break;

        case SHIMCMD_SETACHIEVEMENT:
            if (buflen >= 2)
            {
                const bool enable = (*(buf++) != 0);
                const char *name = (const char *) buf;   // !!! FIXME: buffer overflow possible.
                if (!GSteamStats)
                    writeAchievementSet(fd, name, enable, false);
                else if (enable && !GSteamStats->SetAchievement(name))
                    writeAchievementSet(fd, name, enable, false);
                else if (!enable && !GSteamStats->ClearAchievement(name))
                    writeAchievementSet(fd, name, enable, false);
                else
                    writeAchievementSet(fd, name, enable, true);
            } // if
            break;

        case SHIMCMD_GETACHIEVEMENT:
            if (buflen)
            {
                const char *name = (const char *) buf;   // !!! FIXME: buffer overflow possible.
                bool ach = false;
	            uint32 t = 0;
                if ((GSteamStats) && (GSteamStats->GetAchievementAndUnlockTime(name, &ach, &t)))
                    writeAchievementGet(fd, name, ach ? 1 : 0, t);
	            else
                    writeAchievementGet(fd, name, 2, 0);
            } // if
            break;

        case SHIMCMD_RESETSTATS:
            if (buflen)
            {
                const bool alsoAch = (*(buf++) != 0);
                writeResetStats(fd, alsoAch, (GSteamStats) && (GSteamStats->ResetAllStats(alsoAch)));
            } // if
            break;

        case SHIMCMD_SETSTATI:
            if (buflen >= 5)
            {
                const int32 val = *((int32 *) buf);
                buf += sizeof (int32);
                const char *name = (const char *) buf;   // !!! FIXME: buffer overflow possible.
                writeSetStatI(fd, name, val, (GSteamStats) && (GSteamStats->SetStat(name, val)));
            } // if
            break;

        case SHIMCMD_GETSTATI:
            if (buflen)
            {
                const char *name = (const char *) buf;   // !!! FIXME: buffer overflow possible.
                int32 val = 0;
                if ((GSteamStats) && (GSteamStats->GetStat(name, &val)))
                    writeGetStatI(fd, name, val, true);
                else
                    writeGetStatI(fd, name, 0, false);
            } // if
            break;

        case SHIMCMD_SETSTATF:
            if (buflen >= 5)
            {
                const float val = *((float *) buf);
                buf += sizeof (float);
                const char *name = (const char *) buf;   // !!! FIXME: buffer overflow possible.
                writeSetStatF(fd, name, val, (GSteamStats) && (GSteamStats->SetStat(name, val)));
            } // if
            break;

        case SHIMCMD_GETSTATF:
            if (buflen)
            {
                const char *name = (const char *) buf;   // !!! FIXME: buffer overflow possible.
                float val = 0;
                if ((GSteamStats) && (GSteamStats->GetStat(name, &val)))
                    writeGetStatF(fd, name, val, true);
                else
                    writeGetStatF(fd, name, 0.0f, false);
            } // if
            break;

        case SHIMCMD_WORKSHOP_UPLOAD:
            if (buflen)
            {
                item_upload_data item = {};
                item.shim_deserialize(buf, buflen);

                if (item.id)
                    UpdateItem(item.id, item);
                else
                {
                    auto steam_api_call = GSteamUGC->CreateItem(GAppID, k_EWorkshopFileTypeCommunity);
                    GSteamBridge->set_item_created_callback(steam_api_call, item);
                }
            } // if
            break;

        case SHIMCMD_WORKSHOP_QUERY_ITEM_OWNED:
            if (buflen)
            {
                auto scenario = std::string((const char*)buf);
                auto handle = GSteamUGC->CreateQueryUserUGCRequest(GUserID, k_EUserUGCList_Published, k_EUGCMatchingUGCType_Items, k_EUserUGCListSortOrder_TitleAsc, GAppID, 0, 1);
                GSteamUGC->SetReturnKeyValueTags(handle, true);
                auto steam_api_call = GSteamUGC->SendQueryUGCRequest(handle);
                GSteamBridge->set_item_owned_queried_callback(steam_api_call, scenario);
            }
            break;

        case SHIMCMD_WORKSHOP_QUERY_ITEM_MOD:
            if (buflen)
            {
                auto scenario = std::string((const char*)buf);
                auto handle = GSteamUGC->CreateQueryUserUGCRequest(GUserID, k_EUserUGCList_Subscribed, k_EUGCMatchingUGCType_Items, k_EUserUGCListSortOrder_CreationOrderDesc, 0, GAppID, 1);
                GSteamUGC->SetReturnKeyValueTags(handle, true);
                GSteamUGC->AddExcludedTag(handle, "Scenario");
                auto steam_api_call = GSteamUGC->SendQueryUGCRequest(handle);
                GSteamBridge->set_item_mod_queried_callback(steam_api_call, scenario);
            }
            break;

        case SHIMCMD_WORKSHOP_QUERY_ITEM_SCENARIO:
        {
            auto scenario_tag = std::to_string(static_cast<int>(ItemType::Scenario));
            auto handle = GSteamUGC->CreateQueryUserUGCRequest(GUserID, k_EUserUGCList_Subscribed, k_EUGCMatchingUGCType_Items, k_EUserUGCListSortOrder_CreationOrderDesc, 0, GAppID, 1);
            GSteamUGC->SetReturnOnlyIDs(handle, true);
            GSteamUGC->AddRequiredKeyValueTag(handle, SteamItemTags::ItemType, scenario_tag.c_str());
            auto steam_api_call = GSteamUGC->SendQueryUGCRequest(handle);
            GSteamBridge->set_item_scenario_queried_callback(steam_api_call);
        }
        break;
    } // switch

    return true;  // keep going.
} // processCommand

static void processCommands(PipeType pipeParentRead, PipeType pipeParentWrite)
{
    bool quit = false;
    static uint8 buf[65536];
    int br;

    // this read blocks.
    while (!quit && ((br = readPipe(pipeParentRead, buf, sizeof (buf))) > 0))
    {
        while (br > 0)
        {
            int cmdlen;
            const int rawdatalength = br - 2;

            if (rawdatalength >= 0 && rawdatalength >= (cmdlen = *reinterpret_cast<uint16_t*>(buf)))
            {
                if (!processCommand(buf+2, cmdlen, pipeParentWrite))
                {
                    quit = true;
                    break;
                } // if

                br -= cmdlen + 2;
                if (br > 0)
                    memmove(buf, buf+cmdlen+2, br);
            } // if
            else  // get more data.
            {
                const int morebr = readPipe(pipeParentRead, buf+br, sizeof (buf) - br);
                if (morebr <= 0)
                {
                    quit = true;  // uhoh.
                    break;
                } // if
                br += morebr;
            } // else
        } // while
    } // while
} // processCommands

static bool setEnvironmentVars(PipeType pipeChildRead, PipeType pipeChildWrite)
{
    char buf[64];
    snprintf(buf, sizeof (buf), "%llu", (unsigned long long) pipeChildRead);
    if (!setEnvVar("STEAMSHIM_READHANDLE", buf))
        return false;

    snprintf(buf, sizeof (buf), "%llu", (unsigned long long) pipeChildWrite);
    if (!setEnvVar("STEAMSHIM_WRITEHANDLE", buf))
        return false;

    return true;
} // setEnvironmentVars

static bool initSteamworks(PipeType fd, ESteamAPIInitResult* resultCode, SteamErrMsg* errorMessage)
{
    // this can fail for many reasons:
    //  - you forgot a steam_appid.txt in the current working directory.
    //  - you don't have Steam running
    //  - you don't own the game listed in steam_appid.txt
    *resultCode = SteamAPI_InitEx(errorMessage);
    if (*resultCode != k_ESteamAPIInitResult_OK) return 0;

    GSteamStats = SteamUserStats();
    GSteamUtils = SteamUtils();
    GSteamUser = SteamUser();
    GSteamUGC = SteamUGC();
    GSteamFriends = SteamFriends();

    GAppID = GSteamUtils ? GSteamUtils->GetAppID() : 0;
	GUserID = GSteamUser ? GSteamUser->GetSteamID().ConvertToUint64() : 0;
    GSteamBridge = new SteamBridge(fd);

    return 1;
} // initSteamworks

static void deinitSteamworks(void)
{
    SteamAPI_Shutdown();
    delete GSteamBridge;
    GSteamBridge = NULL;
    GSteamStats = NULL;
    GSteamUtils= NULL;
    GSteamUser = NULL;
    GSteamUGC = NULL;
    GSteamFriends = NULL;
} // deinitSteamworks

static int mainline(void)
{
    PipeType pipeParentRead = NULLPIPE;
    PipeType pipeParentWrite = NULLPIPE;
    PipeType pipeChildRead = NULLPIPE;
    PipeType pipeChildWrite = NULLPIPE;
    ProcessType childPid;

    dbgpipe("Parent starting mainline.\n");
    ESteamAPIInitResult initResultCode;
    SteamErrMsg initErrorMessage;

    if (!createPipes(&pipeParentRead, &pipeParentWrite, &pipeChildRead, &pipeChildWrite))
        fail("Failed to create application pipes");
    else if (!initSteamworks(pipeParentWrite, &initResultCode, &initErrorMessage))
    {
        char str[1200];
        sprintf(str, "Failed to initialize Steamworks: %s (error %d)", initErrorMessage, initResultCode);
        fail(str);
    }
    else if (!setEnvironmentVars(pipeChildRead, pipeChildWrite))
        fail("Failed to set environment variables");
    else if (!launchChild(&childPid)){
#ifdef _WIN32
        int error = GetLastError();
#else
        int error = errno;
#endif
        char str[100];
        sprintf(str, "Failed to launch application (%d)", error);
       // fail("Failed to launch application");
        fail(str);
    }

    // Close the ends of the pipes that the child will use; we don't need them.
    closePipe(pipeChildRead);
    closePipe(pipeChildWrite);
    pipeChildRead = pipeChildWrite = NULLPIPE;

    dbgpipe("Parent in command processing loop.\n");

    // Now, we block for instructions until the pipe fails (child closed it or
    //  terminated/crashed).
    processCommands(pipeParentRead, pipeParentWrite);

    dbgpipe("Parent shutting down.\n");

    // Close our ends of the pipes.
    writeBye(pipeParentWrite);
    closePipe(pipeParentRead);
    closePipe(pipeParentWrite);

    deinitSteamworks();

    dbgpipe("Parent waiting on child process.\n");

    // Wait for the child to terminate, close the child process handles.
    const int retval = closeProcess(&childPid);

    dbgpipe("Parent exiting mainline (child exit code %d).\n", retval);

    return retval;
} // mainline

// end of steamshim_parent.cpp ...

