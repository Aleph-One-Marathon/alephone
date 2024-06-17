#ifndef _INCL_STEAMSHIM_CHILD_H_
#define _INCL_STEAMSHIM_CHILD_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum STEAMSHIM_EventType
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
    SHIMEVENT_ISOVERLAYACTIVATED
} STEAMSHIM_EventType;

/* not all of these fields make sense in a given event. */
typedef struct STEAMSHIM_Event
{
    STEAMSHIM_EventType type;
    int okay;
    int ivalue;
    float fvalue;
    unsigned long long epochsecs;
    char name[256];
} STEAMSHIM_Event;

int STEAMSHIM_init(void);  /* non-zero on success, zero on failure. */
void STEAMSHIM_deinit(void);
int STEAMSHIM_alive(void);
const STEAMSHIM_Event *STEAMSHIM_pump(void);
void STEAMSHIM_requestStats(void);
void STEAMSHIM_storeStats(void);
void STEAMSHIM_setAchievement(const char *name, const int enable);
void STEAMSHIM_getAchievement(const char *name);
void STEAMSHIM_resetStats(const int bAlsoAchievements);
void STEAMSHIM_setStatI(const char *name, const int _val);
void STEAMSHIM_getStatI(const char *name);
void STEAMSHIM_setStatF(const char *name, const float val);
void STEAMSHIM_getStatF(const char *name);

#ifdef __cplusplus
}
#endif

#endif  /* include-once blocker */

/* end of steamshim_child.h ... */

