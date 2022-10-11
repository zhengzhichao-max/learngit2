
#ifndef __LOGGER_H__
#define __LOGGER_H__

#if (LOG_USING_LOGGER)

#define _printf_ _printf2

#if (LOG_USING_COLOR || LOG_USING_LEVEL)
#define fun_printf(lv, fmt, ...) \
    if (lv <= g_log_level) _printf_(fmt, ##__VA_ARGS__)
#else //#if (!LOG_USING_COLOR && !LOG_USING_LEVEL)
#define fun_printf(lv, fmt, ...) _printf_(fmt, ##__VA_ARGS__)
#endif

#if (LOG_USING_COLOR)
/**
 * CSI(Control Sequence Introducer/Initiator) sign
 * more information on https://en.wikipedia.org/wiki/ANSI_escape_code
 */
#define CSI_START "\033["
#define CSI_END "\033[0m"
#endif //#if (LOG_USING_COLOR)

#if (LOG_USING_COLOR || LOG_USING_LEVEL || 1 /*必须存在，原因，就算不使用，但编程上，仍有打印函数是这样填写的*/)
//启用日志模块等级功能
typedef enum {
    //All < Trace < Debug < Info < Warn < Error < Fatal < OFF
    log_level_off = 0,
    log_level_fatal,
    log_level_err,
    log_level_warn,
    log_level_info,
    log_level_debug,
    log_level_trace,
    log_level_all,
    log_level_unknown,
    log_level_cnt,
} log_level_enum;
#endif //#if (LOG_USING_LEVEL)

#if (LOG_USING_COLOR && LOG_USING_LEVEL)
typedef struct {
    const char ch;
    const char *color;
} logger_level_st;
#endif //#if (LOG_USING_COLOR && LOG_USING_LEVEL)

#if (LOG_USING_COLOR && !LOG_USING_LEVEL)
typedef struct {
    const char *color;
} logger_level_st;
#endif //#if (LOG_USING_COLOR && !LOG_USING_LEVEL)

#if (!LOG_USING_COLOR && LOG_USING_LEVEL)
typedef struct {
    const char ch;
} logger_level_st;
#endif //#if (!LOG_USING_COLOR && LOG_USING_LEVEL)

#if (LOG_USING_TIME_INFO || 1)
#define _tm() getLogTimeInfo(jiffies)
#endif

#if (LOG_USING_FILE_INFO)
#define _fi() getFileName(__FILE__)
#endif

//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
#pragma region //打印参数定义// _pattern_ // _parameter() // _parameter(val)
#if (LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
#define _fun_ln() __func__, __LINE__
#endif //#if (LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)

#if (LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
#define _fi_fun_ln() _fi(), _fun_ln()
#endif //#if (LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)

#if (LOG_USING_LEVEL)
#define _lv_letter(lv) logger_level[(/*lv >= 0 &&*/ lv < log_level_cnt) ? lv : log_level_unknown].ch
#endif //#if (LOG_USING_LEVEL)

#if (LOG_USING_COLOR)
#define _lv_color(lv) logger_level[(/*lv >= 0 &&*/ lv < log_level_cnt) ? lv : log_level_unknown].color
#define _start_color_ "%s%s"
#define _start_color(lv) CSI_START, _lv_color(lv)
#endif //#if (LOG_USING_COLOR)

#if (LOG_USING_LEVEL && !LOG_USING_TIME_INFO && !LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
#define _lv_fun_ln_ "[%c][%s, %d]: "
//log_level_enum
#define _lv_fun_ln(lv) _lv_letter(lv), _fun_ln()
#endif //#if (LOG_USING_LEVEL && !LOG_USING_TIME_INFO && !LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)

#if (LOG_USING_LEVEL && !LOG_USING_TIME_INFO && LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
#define _lv_fi_fun_ln_ "[%c][%s, %s, %d]: "
//log_level_enum
#define _lv_fi_fun_ln(lv) _lv_letter(lv), _fi_fun_ln()
#endif //#if (LOG_USING_LEVEL && !LOG_USING_TIME_INFO && LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)

#if (LOG_USING_LEVEL && LOG_USING_TIME_INFO && !LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
#define _lv_tm_fun_ln_ "[%c][%s][%s, %d]: "
//log_level_enum
#define _lv_tm_fun_ln(lv) _lv_letter(lv), _tm(), _fun_ln()
#endif //#if (LOG_USING_LEVEL && LOG_USING_TIME_INFO && !LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)

#if (LOG_USING_LEVEL && LOG_USING_TIME_INFO && LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
#define _lv_tm_fi_fun_ln_ "[%c][%s][%s, %s, %d]: "
//log_level_enum
#define _lv_tm_fi_fun_ln(lv) _lv_letter(lv), _tm(), _fi_fun_ln()
#endif //#if (LOG_USING_LEVEL && LOG_USING_TIME_INFO && LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)

#if (LOG_USING_COLOR && LOG_USING_LEVEL && LOG_USING_TIME_INFO && LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
//log_level_enum //sClr = start color
#define _sClr_lv_tm_fi_fun_ln_ _start_color_ "" _lv_tm_fi_fun_ln_
#define _sClr_lv_tm_fi_fun_ln(lv) _start_color(lv), _lv_tm_fi_fun_ln(lv)
#elif (LOG_USING_COLOR && LOG_USING_LEVEL && LOG_USING_TIME_INFO && !LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
//log_level_enum //sClr = start color
#define _sClr_lv_tm_fun_ln_ _start_color_ "" _lv_tm_fun_ln_
#define _sClr_lv_tm_fun_ln(lv) _start_color(lv), _lv_tm_fun_ln(lv)
#elif (LOG_USING_COLOR && LOG_USING_LEVEL && !LOG_USING_TIME_INFO && LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
//log_level_enum //sClr = start color
#define _sClr_lv_fi_fun_ln_ _start_color_ "" _lv_fi_fun_ln_
#define _sClr_lv_fi_fun_ln(lv) _start_color(lv), _lv_fi_fun_ln(lv)
#elif (LOG_USING_COLOR && LOG_USING_LEVEL && !LOG_USING_TIME_INFO && !LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
//log_level_enum //sClr = start color
#define _sClr_lv_fun_ln_ _start_color_ "" _lv_fun_ln_
#define _sClr_lv_fun_ln(lv) _start_color(lv), _lv_fun_ln(lv)
#endif
#pragma endregion //打印参数定义// _pattern_ // _parameter() // _parameter(val)
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
#define _ok_ "%s%sok%s"
#define _ok() _start_color(log_level_fatal), CSI_END

#define _done_ "%s%sdone%s"
#define _done() _start_color(log_level_fatal), CSI_END

#define _err_ "%s%serr%s"
#define _err() _start_color(log_level_err), CSI_END

#define _word_(a) "%s%s" a "%s"
#define _word_color(lv) _start_color(lv), CSI_END

#define _word2_() "%s%s%s%s%s%s"
#define _word2_color(lv, lv2, str) _start_color(lv), str, CSI_END, _start_color(lv2)
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
#pragma region //base printf
#if (LOG_USING_COLOR && (LOG_USING_LEVEL || 1 /*不管有没有等级字符，只要定义使用了颜色，就有这个*/))
//原始打印，无附加//等级
//log_level_enum
#define logb_empty(lv, fmt, ...) fun_printf(lv, fmt, ##__VA_ARGS__)
//颜色打印//开始//等级
//log_level_enum
#define logb_bgColor(lv, fmt, ...) fun_printf(lv, _start_color_ "" fmt, _start_color(lv), ##__VA_ARGS__)
//颜色打印//结束//等级
//log_level_enum
#define logb_endColor(lv, fmt, ...) fun_printf(lv, fmt "%s", ##__VA_ARGS__, CSI_END)
//颜色打印//开始+结束//等级
//log_level_enum
#define logb_bgEndColor(lv, fmt, ...) fun_printf(lv, _start_color_ "" fmt "%s", _start_color(lv), ##__VA_ARGS__, CSI_END)
//颜色打印//开始+结束+换行//等级
//log_level_enum
#define logb_bgEndColor_newLn(lv, fmt, ...) fun_printf(lv, _start_color_ "" fmt "%s\r\n", _start_color(lv), ##__VA_ARGS__, CSI_END)
//颜色打印//结束+换行//等级
//log_level_enum
#define logb_endColor_newLn(lv, fmt, ...) fun_printf(lv, fmt "%s\r\n", ##__VA_ARGS__, CSI_END)
#endif //#if (LOG_USING_COLOR && LOG_USING_LEVEL)

#if (!LOG_USING_COLOR && !LOG_USING_LEVEL)
//原始打印，无附加//等级
//log_level_enum
#define logb_empty(lv, fmt, ...) fun_printf(lv, fmt, ##__VA_ARGS__)
//颜色打印//结束//等级
//log_level_enum
#define logb_endColor logb_empty
//颜色打印//结束+换行//等级
//log_level_enum
#define logb_endColor_newLn(lv, fmt, ...) fun_printf(lv, fmt "\r\n", ##__VA_ARGS__)
#endif            //#if (!LOG_USING_COLOR && !LOG_USING_LEVEL)
#pragma endregion //base printf
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
#if (LOG_USING_COLOR && LOG_USING_LEVEL && LOG_USING_TIME_INFO && LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
//------
#define logb(lv, fmt, ...) fun_printf(lv, _sClr_lv_tm_fi_fun_ln_ "" fmt "%s\r\n", _sClr_lv_tm_fi_fun_ln(lv), ##__VA_ARGS__, CSI_END)
//--
#define logf(fmt, ...) fun_printf(log_level_fatal, _sClr_lv_tm_fi_fun_ln_ "" fmt "%s\r\n", _sClr_lv_tm_fi_fun_ln(log_level_fatal), ##__VA_ARGS__, CSI_END)
#define loge(fmt, ...) fun_printf(log_level_err, _sClr_lv_tm_fi_fun_ln_ "" fmt "%s\r\n", _sClr_lv_tm_fi_fun_ln(log_level_err), ##__VA_ARGS__, CSI_END)
#define logw(fmt, ...) fun_printf(log_level_warn, _sClr_lv_tm_fi_fun_ln_ "" fmt "%s\r\n", _sClr_lv_tm_fi_fun_ln(log_level_warn), ##__VA_ARGS__, CSI_END)
#define logi(fmt, ...) fun_printf(log_level_info, _sClr_lv_tm_fi_fun_ln_ "" fmt "%s\r\n", _sClr_lv_tm_fi_fun_ln(log_level_info), ##__VA_ARGS__, CSI_END)
#define logd(fmt, ...) fun_printf(log_level_debug, _sClr_lv_tm_fi_fun_ln_ "" fmt "%s\r\n", _sClr_lv_tm_fi_fun_ln(log_level_debug), ##__VA_ARGS__, CSI_END)
//------
#define logb_NoNewLn(lv, fmt, ...) fun_printf(lv, _sClr_lv_tm_fi_fun_ln_ "" fmt, _sClr_lv_tm_fi_fun_ln(lv), ##__VA_ARGS__)
//--
#define logf_NoNewLn(fmt, ...) fun_printf(log_level_fatal, _sClr_lv_tm_fi_fun_ln_ "" fmt, _sClr_lv_tm_fi_fun_ln(log_level_fatal), ##__VA_ARGS__)
#define loge_NoNewLn(fmt, ...) fun_printf(log_level_err, _sClr_lv_tm_fi_fun_ln_ "" fmt, _sClr_lv_tm_fi_fun_ln(log_level_err), ##__VA_ARGS__)
#define logw_NoNewLn(fmt, ...) fun_printf(log_level_warn, _sClr_lv_tm_fi_fun_ln_ "" fmt, _sClr_lv_tm_fi_fun_ln(log_level_warn), ##__VA_ARGS__)
#define logi_NoNewLn(fmt, ...) fun_printf(log_level_info, _sClr_lv_tm_fi_fun_ln_ "" fmt, _sClr_lv_tm_fi_fun_ln(log_level_info), ##__VA_ARGS__)
#define logd_NoNewLn(fmt, ...) fun_printf(log_level_debug, _sClr_lv_tm_fi_fun_ln_ "" fmt, _sClr_lv_tm_fi_fun_ln(log_level_debug), ##__VA_ARGS__)
//------
#define logbr(lv, fmt, ...) fun_printf(lv, _sClr_lv_tm_fi_fun_ln_ "" fmt "%s\r", _sClr_lv_tm_fi_fun_ln(lv), ##__VA_ARGS__, CSI_END)
//--
#define logfr(fmt, ...) fun_printf(log_level_fatal, _sClr_lv_tm_fi_fun_ln_ "" fmt "%s\r", _sClr_lv_tm_fi_fun_ln(log_level_fatal), ##__VA_ARGS__, CSI_END)
#define loger(fmt, ...) fun_printf(log_level_err, _sClr_lv_tm_fi_fun_ln_ "" fmt "%s\r", _sClr_lv_tm_fi_fun_ln(log_level_err), ##__VA_ARGS__, CSI_END)
#define logwr(fmt, ...) fun_printf(log_level_warn, _sClr_lv_tm_fi_fun_ln_ "" fmt "%s\r", _sClr_lv_tm_fi_fun_ln(log_level_warn), ##__VA_ARGS__, CSI_END)
#define logir(fmt, ...) fun_printf(log_level_info, _sClr_lv_tm_fi_fun_ln_ "" fmt "%s\r", _sClr_lv_tm_fi_fun_ln(log_level_info), ##__VA_ARGS__, CSI_END)
#define logdr(fmt, ...) fun_printf(log_level_debug, _sClr_lv_tm_fi_fun_ln_ "" fmt "%s\r", _sClr_lv_tm_fi_fun_ln(log_level_debug), ##__VA_ARGS__, CSI_END)
#elif (LOG_USING_COLOR && LOG_USING_LEVEL && LOG_USING_TIME_INFO && !LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
#define logb(lv, fmt, ...) fun_printf(lv, _sClr_lv_tm_fun_ln_ "" fmt "%s\r\n", _sClr_lv_tm_fun_ln(lv), ##__VA_ARGS__, CSI_END)
//--
#define logf(fmt, ...) fun_printf(log_level_fatal, _sClr_lv_tm_fun_ln_ "" fmt "%s\r\n", _sClr_lv_tm_fun_ln(log_level_fatal), ##__VA_ARGS__, CSI_END)
#define loge(fmt, ...) fun_printf(log_level_err, _sClr_lv_tm_fun_ln_ "" fmt "%s\r\n", _sClr_lv_tm_fun_ln(log_level_err), ##__VA_ARGS__, CSI_END)
#define logw(fmt, ...) fun_printf(log_level_warn, _sClr_lv_tm_fun_ln_ "" fmt "%s\r\n", _sClr_lv_tm_fun_ln(log_level_warn), ##__VA_ARGS__, CSI_END)
#define logi(fmt, ...) fun_printf(log_level_info, _sClr_lv_tm_fun_ln_ "" fmt "%s\r\n", _sClr_lv_tm_fun_ln(log_level_info), ##__VA_ARGS__, CSI_END)
#define logd(fmt, ...) fun_printf(log_level_debug, _sClr_lv_tm_fun_ln_ "" fmt "%s\r\n", _sClr_lv_tm_fun_ln(log_level_debug), ##__VA_ARGS__, CSI_END)
//------
#define logb_NoNewLn(lv, fmt, ...) fun_printf(lv, _sClr_lv_tm_fun_ln_ "" fmt, _sClr_lv_tm_fun_ln(log_level_fatal), ##__VA_ARGS__)
//--
#define logf_NoNewLn(fmt, ...) fun_printf(log_level_fatal, _sClr_lv_tm_fun_ln_ "" fmt, _sClr_lv_tm_fun_ln(log_level_fatal), ##__VA_ARGS__)
#define loge_NoNewLn(fmt, ...) fun_printf(log_level_err, _sClr_lv_tm_fun_ln_ "" fmt, _sClr_lv_tm_fun_ln(log_level_err), ##__VA_ARGS__)
#define logw_NoNewLn(fmt, ...) fun_printf(log_level_warn, _sClr_lv_tm_fun_ln_ "" fmt, _sClr_lv_tm_fun_ln(log_level_warn), ##__VA_ARGS__)
#define logi_NoNewLn(fmt, ...) fun_printf(log_level_info, _sClr_lv_tm_fun_ln_ "" fmt, _sClr_lv_tm_fun_ln(log_level_info), ##__VA_ARGS__)
#define logd_NoNewLn(fmt, ...) fun_printf(log_level_debug, _sClr_lv_tm_fun_ln_ "" fmt, _sClr_lv_tm_fun_ln(log_level_debug), ##__VA_ARGS__)
//------
#define logbr(lv, fmt, ...) fun_printf(lv, _sClr_lv_tm_fun_ln_ "" fmt "%s\r", _sClr_lv_tm_fun_ln(lv), ##__VA_ARGS__, CSI_END)
//--
#define logfr(fmt, ...) fun_printf(log_level_fatal, _sClr_lv_tm_fun_ln_ "" fmt "%s\r", _sClr_lv_tm_fun_ln(log_level_fatal), ##__VA_ARGS__, CSI_END)
#define loger(fmt, ...) fun_printf(log_level_err, _sClr_lv_tm_fun_ln_ "" fmt "%s\r", _sClr_lv_tm_fun_ln(log_level_err), ##__VA_ARGS__, CSI_END)
#define logwr(fmt, ...) fun_printf(log_level_warn, _sClr_lv_tm_fun_ln_ "" fmt "%s\r", _sClr_lv_tm_fun_ln(log_level_warn), ##__VA_ARGS__, CSI_END)
#define logir(fmt, ...) fun_printf(log_level_info, _sClr_lv_tm_fun_ln_ "" fmt "%s\r", _sClr_lv_tm_fun_ln(log_level_info), ##__VA_ARGS__, CSI_END)
#define logdr(fmt, ...) fun_printf(log_level_debug, _sClr_lv_tm_fun_ln_ "" fmt "%s\r", _sClr_lv_tm_fun_ln(log_level_debug), ##__VA_ARGS__, CSI_END)
#elif (LOG_USING_COLOR && LOG_USING_LEVEL && !LOG_USING_TIME_INFO && LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
#define logb(lv, fmt, ...) fun_printf(lv, _sClr_lv_fi_fun_ln_ "" fmt "%s\r\n", _sClr_lv_fi_fun_ln(lv), ##__VA_ARGS__, CSI_END)
//--
#define logf(fmt, ...) fun_printf(log_level_fatal, _sClr_lv_fi_fun_ln_ "" fmt "%s\r\n", _sClr_lv_fi_fun_ln(log_level_fatal), ##__VA_ARGS__, CSI_END)
#define loge(fmt, ...) fun_printf(log_level_err, _sClr_lv_fi_fun_ln_ "" fmt "%s\r\n", _sClr_lv_fi_fun_ln(log_level_err), ##__VA_ARGS__, CSI_END)
#define logw(fmt, ...) fun_printf(log_level_warn, _sClr_lv_fi_fun_ln_ "" fmt "%s\r\n", _sClr_lv_fi_fun_ln(log_level_warn), ##__VA_ARGS__, CSI_END)
#define logi(fmt, ...) fun_printf(log_level_info, _sClr_lv_fi_fun_ln_ "" fmt "%s\r\n", _sClr_lv_fi_fun_ln(log_level_info), ##__VA_ARGS__, CSI_END)
#define logd(fmt, ...) fun_printf(log_level_debug, _sClr_lv_fi_fun_ln_ "" fmt "%s\r\n", _sClr_lv_fi_fun_ln(log_level_debug), ##__VA_ARGS__, CSI_END)
//------
#define logb_NoNewLn(lv, fmt, ...) fun_printf(lv, _sClr_lv_fi_fun_ln_ "" fmt, _sClr_lv_fi_fun_ln(lv), ##__VA_ARGS__)
//--
#define logf_NoNewLn(fmt, ...) fun_printf(log_level_fatal, _sClr_lv_fi_fun_ln_ "" fmt, _sClr_lv_fi_fun_ln(log_level_fatal), ##__VA_ARGS__)
#define loge_NoNewLn(fmt, ...) fun_printf(log_level_err, _sClr_lv_fi_fun_ln_ "" fmt, _sClr_lv_fi_fun_ln(log_level_err), ##__VA_ARGS__)
#define logw_NoNewLn(fmt, ...) fun_printf(log_level_warn, _sClr_lv_fi_fun_ln_ "" fmt, _sClr_lv_fi_fun_ln(log_level_warn), ##__VA_ARGS__)
#define logi_NoNewLn(fmt, ...) fun_printf(log_level_info, _sClr_lv_fi_fun_ln_ "" fmt, _sClr_lv_fi_fun_ln(log_level_info), ##__VA_ARGS__)
#define logd_NoNewLn(fmt, ...) fun_printf(log_level_debug, _sClr_lv_fi_fun_ln_ "" fmt, _sClr_lv_fi_fun_ln(log_level_debug), ##__VA_ARGS__)
//------
#define logbr(lv, fmt, ...) fun_printf(lv, _sClr_lv_fi_fun_ln_ "" fmt "%s\r", _sClr_lv_fi_fun_ln(lv), ##__VA_ARGS__, CSI_END)
//--
#define logfr(fmt, ...) fun_printf(log_level_fatal, _sClr_lv_fi_fun_ln_ "" fmt "%s\r", _sClr_lv_fi_fun_ln(log_level_fatal), ##__VA_ARGS__, CSI_END)
#define loger(fmt, ...) fun_printf(log_level_err, _sClr_lv_fi_fun_ln_ "" fmt "%s\r", _sClr_lv_fi_fun_ln(log_level_err), ##__VA_ARGS__, CSI_END)
#define logwr(fmt, ...) fun_printf(log_level_warn, _sClr_lv_fi_fun_ln_ "" fmt "%s\r", _sClr_lv_fi_fun_ln(log_level_warn), ##__VA_ARGS__, CSI_END)
#define logir(fmt, ...) fun_printf(log_level_info, _sClr_lv_fi_fun_ln_ "" fmt "%s\r", _sClr_lv_fi_fun_ln(log_level_info), ##__VA_ARGS__, CSI_END)
#define logdr(fmt, ...) fun_printf(log_level_debug, _sClr_lv_fi_fun_ln_ "" fmt "%s\r", _sClr_lv_fi_fun_ln(log_level_debug), ##__VA_ARGS__, CSI_END)
#elif (LOG_USING_COLOR && LOG_USING_LEVEL && !LOG_USING_TIME_INFO && !LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
#define logb(lv, fmt, ...) fun_printf(lv, _sClr_lv_fun_ln_ "" fmt "%s\r\n", _sClr_lv_fun_ln(lv), ##__VA_ARGS__, CSI_END)
//--
#define logf(fmt, ...) fun_printf(log_level_fatal, _sClr_lv_fun_ln_ "" fmt "%s\r\n", _sClr_lv_fun_ln(log_level_fatal), ##__VA_ARGS__, CSI_END)
#define loge(fmt, ...) fun_printf(log_level_err, _sClr_lv_fun_ln_ "" fmt "%s\r\n", _sClr_lv_fun_ln(log_level_err), ##__VA_ARGS__, CSI_END)
#define logw(fmt, ...) fun_printf(log_level_warn, _sClr_lv_fun_ln_ "" fmt "%s\r\n", _sClr_lv_fun_ln(log_level_warn), ##__VA_ARGS__, CSI_END)
#define logi(fmt, ...) fun_printf(log_level_info, _sClr_lv_fun_ln_ "" fmt "%s\r\n", _sClr_lv_fun_ln(log_level_info), ##__VA_ARGS__, CSI_END)
#define logd(fmt, ...) fun_printf(log_level_debug, _sClr_lv_fun_ln_ "" fmt "%s\r\n", _sClr_lv_fun_ln(log_level_debug), ##__VA_ARGS__, CSI_END)
//------
#define logb_NoNewLn(lv, fmt, ...) fun_printf(lv, _sClr_lv_fun_ln_ "" fmt, _sClr_lv_fun_ln(lv), ##__VA_ARGS__)
//--
#define logf_NoNewLn(fmt, ...) fun_printf(log_level_fatal, _sClr_lv_fun_ln_ "" fmt, _sClr_lv_fun_ln(log_level_fatal), ##__VA_ARGS__)
#define loge_NoNewLn(fmt, ...) fun_printf(log_level_err, _sClr_lv_fun_ln_ "" fmt, _sClr_lv_fun_ln(log_level_err), ##__VA_ARGS__)
#define logw_NoNewLn(fmt, ...) fun_printf(log_level_warn, _sClr_lv_fun_ln_ "" fmt, _sClr_lv_fun_ln(log_level_warn), ##__VA_ARGS__)
#define logi_NoNewLn(fmt, ...) fun_printf(log_level_info, _sClr_lv_fun_ln_ "" fmt, _sClr_lv_fun_ln(log_level_info), ##__VA_ARGS__)
#define logd_NoNewLn(fmt, ...) fun_printf(log_level_debug, _sClr_lv_fun_ln_ "" fmt, _sClr_lv_fun_ln(log_level_debug), ##__VA_ARGS__)
//------
#define logbr(lv, fmt, ...) fun_printf(lv, _sClr_lv_fun_ln_ "" fmt "%s\r", _sClr_lv_fun_ln(lv), ##__VA_ARGS__, CSI_END)
//--
#define logfr(fmt, ...) fun_printf(log_level_fatal, _sClr_lv_fun_ln_ "" fmt "%s\r", _sClr_lv_fun_ln(log_level_fatal), ##__VA_ARGS__, CSI_END)
#define loger(fmt, ...) fun_printf(log_level_err, _sClr_lv_fun_ln_ "" fmt "%s\r", _sClr_lv_fun_ln(log_level_err), ##__VA_ARGS__, CSI_END)
#define logwr(fmt, ...) fun_printf(log_level_warn, _sClr_lv_fun_ln_ "" fmt "%s\r", _sClr_lv_fun_ln(log_level_warn), ##__VA_ARGS__, CSI_END)
#define logir(fmt, ...) fun_printf(log_level_info, _sClr_lv_fun_ln_ "" fmt "%s\r", _sClr_lv_fun_ln(log_level_info), ##__VA_ARGS__, CSI_END)
#define logdr(fmt, ...) fun_printf(log_level_debug, _sClr_lv_fun_ln_ "" fmt "%s\r", _sClr_lv_fun_ln(log_level_debug), ##__VA_ARGS__, CSI_END)
#elif (LOG_USING_COLOR && !LOG_USING_LEVEL && !LOG_USING_TIME_INFO && !LOG_USING_FILE_INFO && !LOG_USING_FUNC_INFO && !LOG_USING_LINE_INFO)
#define logb(lv, fmt, ...) fun_printf(lv, _start_color_ "" fmt "%s\r\n", _start_color(lv), ##__VA_ARGS__, CSI_END)
//--
#define logf(fmt, ...) fun_printf(log_level_fatal, _start_color_ "" fmt "%s\r\n", _start_color(log_level_fatal), ##__VA_ARGS__, CSI_END)
#define loge(fmt, ...) fun_printf(log_level_err, _start_color_ "" fmt "%s\r\n", _start_color(log_level_err), ##__VA_ARGS__, CSI_END)
#define logw(fmt, ...) fun_printf(log_level_warn, _start_color_ "" fmt "%s\r\n", _start_color(log_level_warn), ##__VA_ARGS__, CSI_END)
#define logi(fmt, ...) fun_printf(log_level_info, _start_color_ "" fmt "%s\r\n", _start_color(log_level_info), ##__VA_ARGS__, CSI_END)
#define logd(fmt, ...) fun_printf(log_level_debug, _start_color_ "" fmt "%s\r\n", _start_color(log_level_debug), ##__VA_ARGS__, CSI_END)
//------
#define logb_NoNewLn(lv, fmt, ...) fun_printf(lv, _start_color_ "" fmt, _start_color(lv), ##__VA_ARGS__)
//--
#define logf_NoNewLn(fmt, ...) fun_printf(log_level_fatal, _start_color_ "" fmt, _start_color(log_level_fatal), ##__VA_ARGS__)
#define loge_NoNewLn(fmt, ...) fun_printf(log_level_err, _start_color_ "" fmt, _start_color(log_level_err), ##__VA_ARGS__)
#define logw_NoNewLn(fmt, ...) fun_printf(log_level_warn, _start_color_ "" fmt, _start_color(log_level_warn), ##__VA_ARGS__)
#define logi_NoNewLn(fmt, ...) fun_printf(log_level_info, _start_color_ "" fmt, _start_color(log_level_info), ##__VA_ARGS__)
#define logd_NoNewLn(fmt, ...) fun_printf(log_level_debug, _start_color_ "" fmt, _start_color(log_level_debug), ##__VA_ARGS__)
//------
#define logbr(lv, fmt, ...) fun_printf(lv, _start_color_ "" fmt "%s\r", _start_color(lv), ##__VA_ARGS__, CSI_END)
//--
#define logfr(fmt, ...) fun_printf(log_level_fatal, _start_color_ "" fmt "%s\r", _start_color(log_level_fatal), ##__VA_ARGS__, CSI_END)
#define loger(fmt, ...) fun_printf(log_level_err, _start_color_ "" fmt "%s\r", _start_color(log_level_err), ##__VA_ARGS__, CSI_END)
#define logwr(fmt, ...) fun_printf(log_level_warn, _start_color_ "" fmt "%s\r", _start_color(log_level_warn), ##__VA_ARGS__, CSI_END)
#define logir(fmt, ...) fun_printf(log_level_info, _start_color_ "" fmt "%s\r", _start_color(log_level_info), ##__VA_ARGS__, CSI_END)
#define logdr(fmt, ...) fun_printf(log_level_debug, _start_color_ "" fmt "%s\r", _start_color(log_level_debug), ##__VA_ARGS__, CSI_END)
#elif (LOG_USING_COLOR && !LOG_USING_LEVEL && LOG_USING_TIME_INFO && !LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
#error can not define like these...
#elif (!LOG_USING_COLOR && LOG_USING_LEVEL && !LOG_USING_TIME_INFO && LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
#define logb(lv, fmt, ...) fun_printf(lv, _lv_fi_fun_ln_ "" fmt "\r\n", _lv_fi_fun_ln(lv), ##__VA_ARGS__)
//--
#define logf(fmt, ...) fun_printf(log_level_fatal, _lv_fi_fun_ln_ "" fmt "\r\n", _lv_fi_fun_ln(log_level_fatal), ##__VA_ARGS__)
#define loge(fmt, ...) fun_printf(log_level_err, _lv_fi_fun_ln_ "" fmt "\r\n", _lv_fi_fun_ln(log_level_err), ##__VA_ARGS__)
#define logw(fmt, ...) fun_printf(log_level_warn, _lv_fi_fun_ln_ "" fmt "\r\n", _lv_fi_fun_ln(log_level_warn), ##__VA_ARGS__)
#define logi(fmt, ...) fun_printf(log_level_info, _lv_fi_fun_ln_ "" fmt "\r\n", _lv_fi_fun_ln(log_level_info), ##__VA_ARGS__)
#define logd(fmt, ...) fun_printf(log_level_debug, _lv_fi_fun_ln_ "" fmt "\r\n", _lv_fi_fun_ln(log_level_debug), ##__VA_ARGS__)
//------
#define logb_NoNewLn(lv, fmt, ...) fun_printf(lv, _lv_fi_fun_ln_ "" fmt, _lv_fi_fun_ln(lv), ##__VA_ARGS__)
//--
#define logf_NoNewLn(fmt, ...) fun_printf(log_level_fatal, _lv_fi_fun_ln_ "" fmt, _lv_fi_fun_ln(log_level_fatal), ##__VA_ARGS__)
#define loge_NoNewLn(fmt, ...) fun_printf(log_level_err, _lv_fi_fun_ln_ "" fmt, _lv_fi_fun_ln(log_level_err), ##__VA_ARGS__)
#define logw_NoNewLn(fmt, ...) fun_printf(log_level_warn, _lv_fi_fun_ln_ "" fmt, _lv_fi_fun_ln(log_level_warn), ##__VA_ARGS__)
#define logi_NoNewLn(fmt, ...) fun_printf(log_level_info, _lv_fi_fun_ln_ "" fmt, _lv_fi_fun_ln(log_level_info), ##__VA_ARGS__)
#define logd_NoNewLn(fmt, ...) fun_printf(log_level_debug, _lv_fi_fun_ln_ "" fmt, _lv_fi_fun_ln(log_level_debug), ##__VA_ARGS__)
//------
#define logbr(lv, fmt, ...) fun_printf(lv, _lv_fi_fun_ln_ "" fmt "\r", _lv_fi_fun_ln(lv), ##__VA_ARGS__)
//--
#define logfr(fmt, ...) fun_printf(log_level_fatal, _lv_fi_fun_ln_ "" fmt "\r", _lv_fi_fun_ln(log_level_fatal), ##__VA_ARGS__)
#define loger(fmt, ...) fun_printf(log_level_err, _lv_fi_fun_ln_ "" fmt "\r", _lv_fi_fun_ln(log_level_err), ##__VA_ARGS__)
#define logwr(fmt, ...) fun_printf(log_level_warn, _lv_fi_fun_ln_ "" fmt "\r", _lv_fi_fun_ln(log_level_warn), ##__VA_ARGS__)
#define logir(fmt, ...) fun_printf(log_level_info, _lv_fi_fun_ln_ "" fmt "\r", _lv_fi_fun_ln(log_level_info), ##__VA_ARGS__)
#define logdr(fmt, ...) fun_printf(log_level_debug, _lv_fi_fun_ln_ "" fmt "\r", _lv_fi_fun_ln(log_level_debug), ##__VA_ARGS__)
#elif (!LOG_USING_COLOR && LOG_USING_LEVEL && !LOG_USING_TIME_INFO && !LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
#define logb(lv, fmt, ...) fun_printf(lv, _lv_fun_ln_ "" fmt "\r\n", _lv_fun_ln(lv), ##__VA_ARGS__)
//--
#define logf(fmt, ...) fun_printf(log_level_fatal, _lv_fun_ln_ "" fmt "\r\n", _lv_fun_ln(log_level_fatal), ##__VA_ARGS__)
#define loge(fmt, ...) fun_printf(log_level_err, _lv_fun_ln_ "" fmt "\r\n", _lv_fun_ln(log_level_err), ##__VA_ARGS__)
#define logw(fmt, ...) fun_printf(log_level_warn, _lv_fun_ln_ "" fmt "\r\n", _lv_fun_ln(log_level_warn), ##__VA_ARGS__)
#define logi(fmt, ...) fun_printf(log_level_info, _lv_fun_ln_ "" fmt "\r\n", _lv_fun_ln(log_level_info), ##__VA_ARGS__)
#define logd(fmt, ...) fun_printf(log_level_debug, _lv_fun_ln_ "" fmt "\r\n", _lv_fun_ln(log_level_debug), ##__VA_ARGS__)
//------
#define logb_NoNewLn(lv, fmt, ...) fun_printf(lv, _lv_fun_ln_ "" fmt, _lv_fun_ln(lv), ##__VA_ARGS__)
//--
#define logf_NoNewLn(fmt, ...) fun_printf(log_level_fatal, _lv_fun_ln_ "" fmt, _lv_fun_ln(log_level_fatal), ##__VA_ARGS__)
#define loge_NoNewLn(fmt, ...) fun_printf(log_level_err, _lv_fun_ln_ "" fmt, _lv_fun_ln(log_level_err), ##__VA_ARGS__)
#define logw_NoNewLn(fmt, ...) fun_printf(log_level_warn, _lv_fun_ln_ "" fmt, _lv_fun_ln(log_level_warn), ##__VA_ARGS__)
#define logi_NoNewLn(fmt, ...) fun_printf(log_level_info, _lv_fun_ln_ "" fmt, _lv_fun_ln(log_level_info), ##__VA_ARGS__)
#define logd_NoNewLn(fmt, ...) fun_printf(log_level_debug, _lv_fun_ln_ "" fmt, _lv_fun_ln(log_level_debug), ##__VA_ARGS__)
//------
#define logbr(lv, fmt, ...) fun_printf(lv, _lv_fun_ln_ "" fmt "\r", _lv_fun_ln(lv), ##__VA_ARGS__)
//--
#define logfr(fmt, ...) fun_printf(log_level_fatal, _lv_fun_ln_ "" fmt "\r", _lv_fun_ln(log_level_fatal), ##__VA_ARGS__)
#define loger(fmt, ...) fun_printf(log_level_err, _lv_fun_ln_ "" fmt "\r", _lv_fun_ln(log_level_err), ##__VA_ARGS__)
#define logwr(fmt, ...) fun_printf(log_level_warn, _lv_fun_ln_ "" fmt "\r", _lv_fun_ln(log_level_warn), ##__VA_ARGS__)
#define logir(fmt, ...) fun_printf(log_level_info, _lv_fun_ln_ "" fmt "\r", _lv_fun_ln(log_level_info), ##__VA_ARGS__)
#define logdr(fmt, ...) fun_printf(log_level_debug, _lv_fun_ln_ "" fmt "\r", _lv_fun_ln(log_level_debug), ##__VA_ARGS__)
#elif (!LOG_USING_COLOR && !LOG_USING_TIME_INFO && !LOG_USING_LEVEL && !LOG_USING_FILE_INFO && !LOG_USING_FUNC_INFO && !LOG_USING_LINE_INFO)
#define logb(lv, fmt, ...) fun_printf(lv, fmt "\r\n", ##__VA_ARGS__)
//--
#define logf(fmt, ...) logb(0, fmt, ##__VA_ARGS__)
#define logi logf
#define logw logf
#define loge logf
#define logf logf
//------
#define logb_NoNewLn(lv, fmt, ...) fun_printf(lv, fmt, ##__VA_ARGS__)
//--
#define logf_NoNewLn(fmt, ...) logb_NoNewLn(0, fmt, ##__VA_ARGS__)
#define logi_NoNewLn logf_NoNewLn
#define logw_NoNewLn logf_NoNewLn
#define loge_NoNewLn logf_NoNewLn
#define logf_NoNewLn logf_NoNewLn
//------
#define logbr(lv, fmt, ...) fun_printf(lv, fmt "\r", ##__VA_ARGS__)
//--
#define logfr(fmt, ...) logbr(0, fmt, ##__VA_ARGS__)
#define logir logfr
#define logwr logfr
#define loger logfr
#define logfr logfr
#endif
#endif //#if (LOG_USING_LOGGER)

//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
#if (LOG_USING_LOGGER)
#if (LOG_USING_COLOR || LOG_USING_LEVEL)
//log_level_debug//log_level_err//log_level_all//log_level_info
extern log_level_enum g_log_level;
extern const logger_level_st logger_level[];
#endif

//------------------------------------------------------------------------------------------------------------------
#if (LOG_USING_FILE_INFO)
char *getFileName(char *file);
#endif //#if (LOG_USING_FILE_INFO)

#if (LOG_USING_TIME_INFO || 1)
char *getLogTimeInfo(u32 time);
#endif //#if (LOG_USING_TIME_INFO)
#endif //#if (LOG_USING_LOGGER)

#if (!LOG_USING_LOGGER)
#define logb_empty nullFun
#define logb_endColor nullFun
#define logb_endColor_newLn nullFun
#define logb nullFun
//------
#define logf nullFun
#define loge nullFun
#define logw nullFun
#define logi nullFun
#define logd nullFun
//------
#define logfr nullFun
#define loger nullFun
#define logwr nullFun
#define logir nullFun
#define logdr nullFun
#endif //#if (!LOG_USING_LOGGER)

#endif /*__LOGGER_H__*/
