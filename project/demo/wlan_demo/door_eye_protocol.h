#ifndef I_DOOR_EYE_PROTOCOL_H
#define I_DOOR_EYE_PROTOCOL_H

#define START_CODE1 0xAB
#define START_CODE2 0xCD
#define END_CODE1   0x0A
#define END_CODE2   0x0D

#define PACKET_DATA_MAX_LEN    100

typedef enum
{
    E_STEP_START_CODE1 = 0,
    E_STEP_START_CODE2,
    E_STEP_PID,
    E_STEP_DATA_LEN_BYTE1,
    E_STEP_DATA_LEN_BYTE2,
    E_STEP_CMD_CODE,
    E_STEP_PACKET_DATA,
    E_STEP_CHECK_SUM1,
    E_STEP_CHECK_SUM2,
    E_STEP_END_CODE1,
    E_STEP_END_CODE2
}DOOR_EYE_PROTOCOL_STEP;

enum LockInstructTyp{
 DATA_PASSTHROUGH = 0xFF, //锁与模块透传     锁<--->模块
 GET_TIMESTAMP = 0xA5,  //获取时间
 VISUAL_DOORBELL_STATUS = 0xA9, //获取模块状态
 DISTRIBUTION_NETWORK_MODEL = 0xAF,//WIFI配网模式
 CALL_PHONE = 0xAC,//呼叫手机
 REMOTE_OPEN_LOCK = 0xAE,//远程开锁
 QUICK_POWER_ON_SELF_TEST = 0xB1,//可视模块自检-电压-电流
 WARNING_NOTICE = 0xCC, //报警事件推送
 VISUAL_DOORBELL_RESET = 0xC4,//可视模块恢复出厂设置
 PARAMETER_SETTING = 0xC8,//参数设置
 LOCLA_OPEN_LOCK = 0xD3,//本地开锁
 LOCLA_CLOSE_LOCK = 0xD4,//本地关锁
 LOCK_RESET = 0xD5,//锁恢复出厂设置
 WIFI_INFORMATION_SET_UP_NET = 0x02,//WIFI信息网络配置
 TGIRD_PARTYY_PASSTHROUGH = 0x07,//第三方服务透传 锁<-->后台
};

/*
状态（STATUS）
可视模块响应回复汇总
*/
enum Response{
 RESPONSE_STATUS = 0x00, /*正确（执行成功）*/
 RESPONSE_DATA_CHECKOUT = 0x01,/*数据校验错误 */
 CAPTURE_JPEG_PUSH_0_0_1 = 0x02,/*抓拍失败，录像失败，推送成功（上传图片超时）*/
 CAPTURE_JPEG_PUSH_0_0_0 = 0x03,/*抓拍失败，录像失败，推送失败（无网络无 SD 卡录像时）*/
 CAPTURE_JPEG_PUSH_0_1_0 = 0x04,/*抓拍失败，录像成功，推送失败（无网络 SD 卡录像时）*/
 CAPTURE_JPEG_PUSH_1_0_1 = 0x05,/*抓拍成功，录像失败，推送成功（无 TF 卡、卡坏或卡无法识别需要格式化）*/
 OPERATION_FREQUENTLY = 0x06,
 /*操作频繁，录像重复操作的间隔 60 秒，抓图重复的间隔 10 秒，不抓图/录像重复的间隔 2 秒 */
 WIFI_VOICE_PROMPT = 0x07,/*正在配网中，智锁端收到此回复后须播放提示语音“WiFi 连接中，请稍等”*/
 WIFI_SET_UP_NET_SUCCEED = 0x08,/* 连接 WiFi 路由成功 */
 WFIF_CONNECTION_TIMEOUT = 0x09,/* 连接 WiFi 路由超时失败（180 秒），如输入错的密码、SSID，或路由信号差 */
 CALL_CONNECTION_TIMEOUT = 0x0A,/* 呼叫 30 秒超时，无人接听 */
 LIVING_BROADCAST = 0x0B, /* 正在视频直播通话中... */
 APP_SLEEP = 0x0C, /* APP 结束通话/网络中断等挂机进入休眠 */
 CALL_CONNECTION = 0x0F, /* 呼叫转接中...（锁端可以通过此状态进行相关的显示或重新等待 30 秒超时） */
 RESPONSE_RESERVE = 0xF0, /* 没有定义、未能识别的操作 */
 RESPONSE_BUSY = 0xFC, /* 系统忙，执行失败 */
 EXECUTION_BUSY_10 = 0xFF, /* 执行中，可视模块会在 10 秒内再次返回执行的结果 */
};


typedef struct
{
    unsigned short start_code;//开始码
    unsigned char PID;//标识
    unsigned short data_len;//数据长度
    unsigned char cmd_code;//指令码
    unsigned char packet_data[PACKET_DATA_MAX_LEN];//包数据
    unsigned short packet_data_len;//包数据长度
    unsigned short check_sum;//校验和
    unsigned short end_code;//结束码
}DOOR_EYE_PROTOCOL_DATA;

typedef struct
{
    unsigned char second; //秒
    unsigned char minute; //分
    unsigned char hour;   //时
    unsigned char day;    //日
    unsigned char month;  //月
    unsigned char year;   //年
}DATE;

typedef struct
{
    unsigned char idle_state: 1U;//1=可以处理任何令；0=忙，不处理
    unsigned char wifi_connect_state: 1U;//1=已连接；0=未连接或连接中
    unsigned char login_service_state: 1U;//1=有网络服务；0=无网络服务
    unsigned char communication_mode: 1U;//1=可远程网络唤醒；0=仅本地唤醒
    unsigned char telecommunication: 1U;//1=网络通信中；0=无网络事件
    unsigned char Doorbell_call:1U;//1=呼叫中；0=呼叫结束
    unsigned char Video_state:1U;//1=录像中；0=无录像/录像已完成
    unsigned char reserved: 1U;//保留
}VISUAL_DOOR_STATE;

typedef struct
{
    unsigned char wifi_name[32U];
    unsigned char wifi_key[32U];
    unsigned char wifi_encryption_type;
    unsigned char user_key[10U];
}USER_WIFI_INFO;



extern void analys_door_eye_protocol(char *data);

#endif

