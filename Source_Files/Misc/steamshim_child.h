#ifndef _INCL_STEAMSHIM_CHILD_H_
#define _INCL_STEAMSHIM_CHILD_H_

#include <string>
#include <vector>

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

struct item_upload_data {

    uint64_t id;
    ItemType item_type;
    ContentType content_type;
    std::string directory_path;
    std::string thumbnail_path;
    std::string required_scenario;

    std::ostringstream shim_serialize() const
    {
        std::ostringstream data_stream;
        data_stream.write(reinterpret_cast<const char*>(&id), sizeof(id));
        data_stream.write(reinterpret_cast<const char*>(&item_type), sizeof(item_type));
        data_stream.write(reinterpret_cast<const char*>(&content_type), sizeof(content_type));

        data_stream << directory_path << '\0';
        data_stream << thumbnail_path << '\0';
        data_stream << required_scenario << '\0';

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

    int result_code; //steam code (EResult) where 1 is success
    std::vector<item> items;

    void shim_deserialize(const uint8* buf, unsigned int buflen)
    {
        std::istringstream iss(std::string((const char*)buf, buflen));

        iss.read(reinterpret_cast<char*>(&result_code), sizeof(result_code));

        int number_items;
        iss.read(reinterpret_cast<char*>(&number_items), sizeof(number_items));

        for (int i = 0; i < number_items; i++)
        {
            item deserialized_item = {};
            
            iss.read(reinterpret_cast<char*>(&deserialized_item.id), sizeof(deserialized_item.id));

            int item_type;
            iss.read(reinterpret_cast<char*>(&item_type), sizeof(item_type));
            deserialized_item.item_type = static_cast<ItemType>(item_type);

            int content_type;
            iss.read(reinterpret_cast<char*>(&content_type), sizeof(content_type));
            deserialized_item.content_type = static_cast<ContentType>(content_type);

            iss.read(reinterpret_cast<char*>(&deserialized_item.is_scenarios_compatible), sizeof(deserialized_item.is_scenarios_compatible));
            std::getline(iss, deserialized_item.title, '\0');

            items.push_back(deserialized_item);
        }
    }
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

    int result_code; //steam code (EResult) where 1 is success
    std::vector<item> items;

    void shim_deserialize(const uint8* buf, unsigned int buflen)
    {
        std::istringstream iss(std::string((const char*)buf, buflen));

        iss.read(reinterpret_cast<char*>(&result_code), sizeof(result_code));

        int number_items;
        iss.read(reinterpret_cast<char*>(&number_items), sizeof(number_items));

        for (int i = 0; i < number_items; i++)
        {
            item deserialized_item = {};
            iss.read(reinterpret_cast<char*>(&deserialized_item.id), sizeof(deserialized_item.id));

            int item_type;
            iss.read(reinterpret_cast<char*>(&item_type), sizeof(item_type));
            deserialized_item.item_type = static_cast<ItemType>(item_type);

            int content_type;
            iss.read(reinterpret_cast<char*>(&content_type), sizeof(content_type));
            deserialized_item.content_type = static_cast<ContentType>(content_type);

            std::getline(iss, deserialized_item.install_folder_path, '\0');

            items.push_back(deserialized_item);
        }
    }
};

enum STEAMSHIM_EventType
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
    SHIMEVENT_WORKSHOP_QUERY_ITEM_OWNED_RESULT,
    SHIMEVENT_WORKSHOP_QUERY_ITEM_SUBSCRIBED_RESULT
};

enum STEAM_EItemUpdateStatus //from steam API
{
    k_EItemUpdateStatusInvalid = 0, // The item update handle was invalid, job might be finished, listen too SubmitItemUpdateResult_t
    k_EItemUpdateStatusPreparingConfig = 1, // The item update is processing configuration data
    k_EItemUpdateStatusPreparingContent = 2, // The item update is reading and processing content files
    k_EItemUpdateStatusUploadingContent = 3, // The item update is uploading content changes to Steam
    k_EItemUpdateStatusUploadingPreviewFile = 4, // The item update is uploading new preview file image
    k_EItemUpdateStatusCommittingChanges = 5  // The item update is committing all changes
};

/* not all of these fields make sense in a given event. */
struct STEAMSHIM_Event
{
    STEAMSHIM_EventType type;
    int okay;
    int ivalue;
    float fvalue;
    unsigned long long epochsecs;
    char name[256];
    bool needs_to_accept_workshop_agreement;
    item_owned_query_result items_owned;
    item_subscribed_query_result items_subscribed;
};

int STEAMSHIM_init(void);  /* non-zero on success, zero on failure. */
void STEAMSHIM_deinit(void);
int STEAMSHIM_alive(void);
const STEAMSHIM_Event *STEAMSHIM_pump(void);
void STEAMSHIM_requestStats(void);
void STEAMSHIM_storeStats(void);
void STEAMSHIM_setAchievement(const char *name, const int enable);
void STEAMSHIM_getAchievement(const char *name);
void STEAMSHIM_uploadWorkshopItem(const item_upload_data& item);
void STEAMSHIM_resetStats(const int bAlsoAchievements);
void STEAMSHIM_setStatI(const char *name, const int _val);
void STEAMSHIM_getStatI(const char *name);
void STEAMSHIM_setStatF(const char *name, const float val);
void STEAMSHIM_getStatF(const char *name);
void STEAMSHIM_queryWorkshopItemOwned(const std::string& scenario_name);
void STEAMSHIM_queryWorkshopItemMod(const std::string& scenario_name);
void STEAMSHIM_queryWorkshopItemScenario();
#endif  /* include-once blocker */

/* end of steamshim_child.h ... */

