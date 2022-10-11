
#include "include_all.h"

#pragma region //定义菜单带颜色
#if (LOG_USING_COLOR)
/**
 * CSI(Control Sequence Introducer/Initiator) sign
 * more information on https://en.wikipedia.org/wiki/ANSI_escape_code
 */
/* output log front color */
#define F_BLACK "30m"
#define F_RED "31m"
#define F_GREEN "32m"
#define F_YELLOW "33m"
#define F_BLUE "34m"
#define F_MAGENTA "35m"
#define F_CYAN "36m"
#define F_WHITE "37m"

/* output log default color definition */
#ifndef LOG_COLOR_OFF
#define LOG_COLOR_OFF (F_WHITE)
#endif
#ifndef LOG_COLOR_FATAL
#define LOG_COLOR_FATAL (F_MAGENTA)
#endif
#ifndef LOG_COLOR_ERROR
#define LOG_COLOR_ERROR (F_RED)
#endif
#ifndef LOG_COLOR_WARN
#define LOG_COLOR_WARN (F_YELLOW)
#endif
#ifndef LOG_COLOR_INFO
#define LOG_COLOR_INFO (F_GREEN)
#endif
#ifndef LOG_COLOR_DEBUG
#define LOG_COLOR_DEBUG F_WHITE
#endif
#ifndef LOG_COLOR_TRACE
#define LOG_COLOR_TRACE F_WHITE
#endif
#ifndef LOG_COLOR_ALL
#define LOG_COLOR_ALL F_WHITE
#endif
#ifndef LOG_COLOR_UNKNOWN
#define LOG_COLOR_UNKNOWN LOG_COLOR_FATAL
#endif
#endif            //#if (LOG_USING_COLOR)
#pragma endregion //定义菜单带颜色

#if (LOG_USING_COLOR && LOG_USING_LEVEL)
const logger_level_st logger_level[] = {
    {'o', LOG_COLOR_OFF},
    {'f', LOG_COLOR_FATAL},
    {'e', LOG_COLOR_ERROR},
    {'w', LOG_COLOR_WARN},
    {'i', LOG_COLOR_INFO},
    {'d', LOG_COLOR_DEBUG},
    {'t', LOG_COLOR_TRACE},
    {'a', LOG_COLOR_ALL},
    {'?', LOG_COLOR_UNKNOWN},
};
#endif //#if (LOG_USING_COLOR && LOG_USING_LEVEL)

#if (!LOG_USING_COLOR && LOG_USING_LEVEL)
const logger_level_st logger_level[] = {
    {'o'},
    {'f'},
    {'e'},
    {'w'},
    {'i'},
    {'d'},
    {'t'},
    {'a'},
    {'?'},
};
#endif //#if (!LOG_USING_COLOR && LOG_USING_LEVEL)

#if (LOG_USING_COLOR && !LOG_USING_LEVEL)
const logger_level_st logger_level[] = {
    {LOG_COLOR_OFF},
    {LOG_COLOR_FATAL},
    {LOG_COLOR_ERROR},
    {LOG_COLOR_WARN},
    {LOG_COLOR_INFO},
    {LOG_COLOR_DEBUG},
    {LOG_COLOR_TRACE},
    {LOG_COLOR_ALL},
    {LOG_COLOR_UNKNOWN},
};
#endif //#if (LOG_USING_COLOR && !LOG_USING_LEVEL)

#if (LOG_USING_COLOR || LOG_USING_LEVEL)
//log_level_debug//log_level_err//log_level_all//log_level_info//log_level_off
log_level_enum g_log_level = log_level_debug;
#endif //#if (LOG_USING_LEVEL)

#if (LOG_USING_LOGGER)
#if (LOG_USING_FILE_INFO)
char *getFileName(char *file) {
    char *ret = strrchr(file, '\\');
    if (ret == NULL) {
        return file;
    } else {
        return ret + 1;
    }
}
#endif /* #if (LOG_USING_FILE_INFO) */

#if (LOG_USING_TIME_INFO || 1)
char *getLogTimeInfo(u32 time) {
    static char ret[13] = {0};

    u8 hour = (time % (1000 * 86400)) / (1000 * 3600); //=INT(MOD(A29,1000*86400)/(1000*3600))
    u8 min = (time % (1000 * 3600)) / (1000 * 60);     //=INT(MOD(A27,1000*3600)/(1000*60))
    u8 sec = (time % (1000 * 60)) / 1000;              //=INT(MOD(A27,1000*60)/1000)
    u16 msec = time % 1000;                            //=MOD(A27,1000)

    sprintf(ret, "%02d:%02d:%02d:%03d", hour, min, sec, msec);

    return ret;
}
#endif //#if (LOG_USING_TIME_INFO)
#endif //#if (LOG_USING_LOGGER)
