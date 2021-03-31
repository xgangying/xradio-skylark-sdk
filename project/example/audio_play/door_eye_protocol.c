#include "door_eye_protocol.h"
#include "common/cmd/cmd_util.h"
#include "common/cmd/cmd.h"


unsigned char analys_step = E_STEP_START_CODE1;
DOOR_EYE_PROTOCOL_DATA rx_protocol = {0};
DOOR_EYE_PROTOCOL_DATA tx_protocol = {0};
unsigned char tx_buff[100] = {0};
DATE current_date = {0};
VISUAL_DOOR_STATE door_State = {0};
USER_WIFI_INFO user_wifi_info;


void analys_data_packet(DOOR_EYE_PROTOCOL_DATA *protocol_data);
void data_package(unsigned char PID);
void data_package(unsigned char PID);
void send_remote_unlock_command(void);
void send_wake_up_response(void);
unsigned short count_checksum(unsigned char *buff, unsigned char len);
void send_serial_data(unsigned char *data);
//wifi信息配网指令
void wifi_info_distribution_network_instruction(unsigned char *data, unsigned char data_len);
unsigned char string_conversion_number(unsigned char *str, unsigned char *number);

void analys_door_eye_protocol(char *data)
{
    static unsigned short temp;
    unsigned char len = 0;
    unsigned char data_buff[100] = {0U};
    unsigned char index = 0;

#if 0
    len = string_conversion_number((unsigned char*)data, (unsigned char*)data_buff);

    printf("len=%d\n", len);
    for (index = 0; index < len; index++)
    {
        //printf("%d ",data_buff[index]);
    }
    //printf("\n");
    index = 0;
#else
while ('\0' != *data)
{
    data_buff[len] = data[len];
    printf("rx:%d\n",data_buff[len]);
    len++;
}
#endif

    for (index = 0; index < len; index++)
    {
        switch (analys_step)
        {
            case E_STEP_START_CODE1:
            {
                if (START_CODE1 == data_buff[index])
                {
                    temp = 0;
                    analys_step = E_STEP_START_CODE2;
                }
                else
                {
                    if (0x80 == data_buff[index])
                    {
                        if (temp++ > 5)
                        {
                            //可视门铃回复响应
                            send_wake_up_response();
                        }
                    }
                }
                printf("E_STEP_START_CODE1:%d\n", data_buff[index]);
            }
            break;

            case E_STEP_START_CODE2:
            {
                if (START_CODE2 == data_buff[index])
                {
                    analys_step = E_STEP_PID;
                    printf("START_CODE2:%d\n", data_buff[index]);
                }
                else
                {
                    analys_step = E_STEP_START_CODE1;
                }
            }
            break;

            case E_STEP_PID:
            {
                rx_protocol.PID = data_buff[index];
                rx_protocol.check_sum = data_buff[index];
                analys_step = E_STEP_DATA_LEN_BYTE1;
                printf("E_STEP_PID:%d\n", data_buff[index]);
            }
            break;

            case E_STEP_DATA_LEN_BYTE1:
            {
                rx_protocol.data_len = data_buff[index];
                rx_protocol.check_sum += data_buff[index];
                analys_step = E_STEP_DATA_LEN_BYTE2;
                printf("E_STEP_DATA_LEN_BYTE1:%d\n", data_buff[index]);
            }
            break;

            case E_STEP_DATA_LEN_BYTE2:
            {
                temp = data_buff[index];
                temp <<= 8;
                rx_protocol.data_len += temp;
                rx_protocol.check_sum += data_buff[index];
                analys_step = E_STEP_CMD_CODE;
                printf("E_STEP_DATA_LEN_BYTE2:%d\n", data_buff[index]);
            }
            break;

            case E_STEP_CMD_CODE:
            {
                temp = 0;
                rx_protocol.cmd_code = data_buff[index];
                rx_protocol.packet_data_len = rx_protocol.data_len - 3;
                rx_protocol.check_sum += data_buff[index];
                
                if (rx_protocol.packet_data_len > 0)
                {
                    analys_step = E_STEP_PACKET_DATA;
                }
                else
                {
                    analys_step = E_STEP_CHECK_SUM1;
                }
                printf("E_STEP_CMD_CODE:%d\n", data_buff[index]);
            }
            break;

            case E_STEP_PACKET_DATA:
            {
                if (temp < rx_protocol.packet_data_len)
                {
                    rx_protocol.packet_data[temp] = data_buff[index];
                    rx_protocol.check_sum += data_buff[index];
                    temp++;
                }

                if (temp >= rx_protocol.packet_data_len)
                {
                    analys_step = E_STEP_CHECK_SUM1;
                }
                printf("E_STEP_PACKET_DATA:%d\n", data_buff[index]);
            }
            break;

            case E_STEP_CHECK_SUM1:
            {
                if (data_buff[index] == ((unsigned char)rx_protocol.check_sum))
                {
                    analys_step = E_STEP_CHECK_SUM2;
                }
                else
                {
                    analys_step = E_STEP_START_CODE1;
                }
                printf("E_STEP_CHECK_SUM1:%d\n", data_buff[index]);
            }
            break;

            case E_STEP_CHECK_SUM2:
            {
                if (data_buff[index] == (unsigned char)(rx_protocol.check_sum >> 8))
                {
                    analys_step = E_STEP_END_CODE1;
                }
                else
                {
                    analys_step = E_STEP_START_CODE1;
                }
                printf("E_STEP_CHECK_SUM2:%d\n", data_buff[index]);
            }
            break;

            case E_STEP_END_CODE1:
            {
                if (END_CODE1 == data_buff[index])
                {
                    analys_step = E_STEP_END_CODE2;
                }
                else
                {
                    analys_step = E_STEP_START_CODE1;
                }
                printf("E_STEP_END_CODE1:%d\n", data_buff[index]);
            }
            break;

            case E_STEP_END_CODE2:
            {
                printf("E_STEP_END_CODE2:%d\n", data_buff[index]);
                if (END_CODE2 == data_buff[index])
                {
                    printf("check_sum_ok\n");
                    analys_data_packet(&rx_protocol);
                }
                else
                {
                    printf("check_sum_err\n");
                }

                analys_step = E_STEP_START_CODE1;
                
            }
            break;

            default:
            {

            }
            break;
        }
    }
}

void analys_data_packet(DOOR_EYE_PROTOCOL_DATA *protocol_data)
{
    switch (protocol_data->cmd_code)
    {
        case GET_TIMESTAMP://获取时间（用于锁的日期时间调整）
        {
            tx_protocol.data_len = 0x09;
            //MEMCPY(&tx_protocol.packet_data[0], (unsigned char*)&current_date, 6);
        }
        break;

        case VISUAL_DOORBELL_STATUS://获取可视模块当前状态 
        {
            tx_protocol.data_len = 0x04;
            //MEMCPY(&tx_protocol.packet_data[0], (unsigned char*)&door_State, 1);

        }
        break;

        case DISTRIBUTION_NETWORK_MODEL://可视模块 WiFi 配网模式
        {
            tx_protocol.data_len = 0x04;
            tx_protocol.packet_data[0] = RESPONSE_STATUS;//状态（STATUS）
            /*
            状态（STATUS）
            0x00：正确
            0x01：数据校验错误
            0xF0：没有定义、未能识别的操作
            0x07：正在配网中，智锁端收到此回复后须播放提示语音“WiFi 连接中，请稍等”
            0x08：连接 WiFi 路由成功
            0x09：连接 WiFi 路由超时失败（180 秒），如输入错的密码、SSID，或路由信号差。
            0xFC：系统忙，执行失败
            */
        }
        break;

        case CALL_PHONE://呼叫手机指令
        {
            /*
            数据=0x00，门铃呼叫
            数据=0x01，SOS 呼叫
            */
            if ((0x00 == protocol_data->packet_data[0]) || (0x01 == protocol_data->packet_data[0]))
            {
                tx_protocol.data_len = 0x04;
                tx_protocol.packet_data[0] = RESPONSE_STATUS;//状态（STATUS）
                /*
                状态（STATUS）
                0x00：正确（正在呼叫中...)
                0x01：数据校验错误
                0xF0：没有定义、未能识别的操作
                0x0A：30 秒超时，无人接听
                0x0B：正在通话中...
                0x0C：APP 结束通话/网络中断等挂机进入休眠
                0x0F: 呼叫转接中...（锁端可以通过此状态进行相关的显示或重新等待 30 秒超时）
                0xFC：系统忙，执行失败
                0xFF：执行中，可视模块会在 10 秒内再次返回执行的结果
                */
            }
            else if (0x10 == protocol_data->packet_data[0])//数据=0x10，结束通话
            {
                tx_protocol.data_len = 0x04;
                tx_protocol.packet_data[0] = 0x0C;//状态（STATUS）
                /*
                状态（STATUS）
                0x01：数据校验错误
                0xF0：没有定义、未能识别的操作
                0x0C：结束通话挂机进入休眠
                0xFC：系统忙，执行失败
                */
            }
        }
        break;

        case REMOTE_OPEN_LOCK://应答远程开锁指令
        {
            /*
            状态（STATUS）
            0x00：正确（开锁成功）
            0x01：数据校验错误
            0xF0：没有定义、未能识别的操作
            0xFC：系统忙，执行失败
            0xFD：锁故障，开锁失败
            0xFE：随机码错误，开锁失败
            */
        }
        break;

        case QUICK_POWER_ON_SELF_TEST://可视模块自检与电流测量模式
        {

        }
        break;

        case WARNING_NOTICE://报警事件推送通知
        {

        }
        break;

        case VISUAL_DOORBELL_RESET://恢复可视模块出厂设置、重启 
        {
            /*
            数据=0x00，恢复可视模块出厂设置（包括解绑帐户，必须在有网络的条件下）
            数据=0x01，恢复除了路由信息（网络连接）外的所有设置到默认值
            数据=0x02，可视模块软件重启
            */
            tx_protocol.data_len = 0x04;
            tx_protocol.packet_data[0] = RESPONSE_STATUS;//状态（STATUS）
            /*
            状态（STATUS）
            0x00：正确
            0x01：数据校验错误
            0xF0：没有定义、未能识别的操作
            0xFC：系统忙，执行失败
            */
        }
        break;

        case PARAMETER_SETTING://设置参数
        {
            /*
            数据=0x01（9600 bps）
            数据=0x02（115200 bps）
            数据=0x10（锂电池，单/双节自动识别）
            数据=0x11（AAA 电池，4 节碱性电池 6V）
            备注：波特率设置后立即生效，设置电池需重启后生效，恢复出厂设置或掉电后不影响，仍然保持当
            前的设定

            */
            tx_protocol.data_len = 0x04;
            tx_protocol.packet_data[0] = RESPONSE_STATUS;//状态（STATUS）
            /*
            状态（STATUS）
            0x00：正确
            0x01：数据校验错误
            0xF0：没有定义、未能识别的操作
            0xFC：系统忙，执行失败

            */
        }
        break;

        case LOCLA_OPEN_LOCK://可视模块本地请求开锁指令
        {

        }
        break;

        case LOCLA_CLOSE_LOCK://可视模块本地请求上锁指令
        {

        }
        break;

        case LOCK_RESET://可视模块请求 恢复锁端出厂设置 
        {

        }
        break;
        
        case WIFI_INFORMATION_SET_UP_NET://wifi 信息配网指令
        {
            /*
            说明：
            数据格式：长度（1 字节）+类型（1 字节）+数据（长度-1）...长度+类型+数据
            类型（type）：
            1 WIFI 名称（必填）
            2 WIFI 密码（必填）
            3 加密类型（必填 ，0 空密码，1 WEP，2 WPA，4 WPA2）
            4 用户密钥（可选 ，APP 通过 api 获取生成，用于绑定设备）
            */
            wifi_info_distribution_network_instruction(&protocol_data->packet_data[0U], protocol_data->packet_data_len);
        }
        break;

        case TGIRD_PARTYY_PASSTHROUGH://第三方服务透传指令
        {

        }
        break;

        default:
        {

        }
        break;
    }

    tx_protocol.cmd_code = protocol_data->cmd_code;
    data_package(0x07);
}

//数据组包
void data_package(unsigned char PID)
{
    tx_protocol.start_code = 0xCDAB;
    tx_protocol.PID = PID;
    tx_protocol.end_code = 0x0D0A;
    tx_protocol.packet_data_len = tx_protocol.data_len - 3;
    //MEMCPY(tx_buff, (unsigned char *)&tx_protocol, tx_protocol.packet_data_len + 6);
    tx_protocol.check_sum = count_checksum(&tx_buff[1], tx_protocol.packet_data_len + 4);
    tx_buff[tx_protocol.packet_data_len + 6] = (unsigned char)tx_protocol.check_sum;
    tx_buff[tx_protocol.packet_data_len + 7] = (unsigned char)(tx_protocol.check_sum >> 8);
    tx_buff[tx_protocol.packet_data_len + 8] = (unsigned char)tx_protocol.end_code;
    tx_buff[tx_protocol.packet_data_len + 9] = (unsigned char)(tx_protocol.end_code >> 8);
    tx_buff[tx_protocol.packet_data_len + 9] = '\0';
    //send_serial_data(tx_buff);
}

//发送远程开锁指令
void send_remote_unlock_command(void)
{
    tx_protocol.data_len = 0x09;
    //随机码：当可视模块上电或可视对讲时获取，开锁时采用此随机码进行确认开锁
    tx_protocol.packet_data[0] = 0x12;//随机码
    tx_protocol.packet_data[1] = 0x34;//随机码
    tx_protocol.cmd_code = 0xAE;
    data_package(0x01);
}

void send_wake_up_response(void)
{
    tx_buff[0] = 0xAB;
    tx_buff[1] = 0xCD;
    tx_buff[2] = 0x0A;
    tx_buff[3] = 0x0D;
    tx_buff[4] = '\0';
    send_serial_data(tx_buff);
}

unsigned short count_checksum(unsigned char *buff, unsigned char len)
{
    unsigned char index;
    unsigned short check_sum = 0;

    for (index = 0; index < len; index++)
    {
        check_sum += buff[index];
    }

    return check_sum;
}

void send_serial_data(unsigned char *data)
{
    //printf("%s", data);
}

//wifi信息配网指令
void wifi_info_distribution_network_instruction(unsigned char *data, unsigned char data_len)
{
    unsigned char len  = 0U;
    unsigned char type = 0U;
    unsigned char index = 0U;
    
    len = data[0U];
    type = data[1U];

    for (index = 0U; index < data_len;)
    {
        len = data[index];
        type = data[index + 1U];
        
        if (1U == type)
        {
            data_copy(&user_wifi_info.wifi_name[0U], &data[index + 2U], len);
            user_wifi_info.wifi_name[len] = '\0';
        }
        
        if (2U == type)
        {
            data_copy(&user_wifi_info.wifi_key[0U], &data[index + 2U], len);
            user_wifi_info.wifi_key[len] = '\0';
        }
        
        if (3U == type)
        {
            user_wifi_info.wifi_encryption_type = data[index + 2U];
        }
        
        if (4U == type)
        {
            data_copy(&user_wifi_info.user_key[0U], &data[index + 2U], len);
            user_wifi_info.user_key[len] = '\0';
        }

        index += (len + 1U);
    }
}

unsigned char string_conversion_number(unsigned char *str, unsigned char *number)
{
    unsigned char data = 0;
    unsigned char count = 0;
    unsigned char valid_flag = 0;
    
    while ('\0' != *str)
    {
        data = 0;
        while ((0x30U <= *str)&&(*str <= 0x39U))
        {
            data = data*10;
            data += (*str - 0x30);
            str++;
            valid_flag = 1U;
        }

        if (1U == valid_flag)
        {
            valid_flag = 0U;
            *number = data;        
            number++;
            count++;
        }

        str++;
    }

    return count;
}

void data_copy(unsigned char * data_to, unsigned char * data_from, unsigned short length)
{
    if((NULL != data_to) && (NULL != data_from))
    {    
        while(length > 0)
        {
            * data_to = * data_from;
            data_to++;
            data_from++;
            length--;
        }
    }
}



