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
//wifi��Ϣ����ָ��
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
                            //��������ظ���Ӧ
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
        case GET_TIMESTAMP://��ȡʱ�䣨������������ʱ�������
        {
            tx_protocol.data_len = 0x09;
            //MEMCPY(&tx_protocol.packet_data[0], (unsigned char*)&current_date, 6);
        }
        break;

        case VISUAL_DOORBELL_STATUS://��ȡ����ģ�鵱ǰ״̬ 
        {
            tx_protocol.data_len = 0x04;
            //MEMCPY(&tx_protocol.packet_data[0], (unsigned char*)&door_State, 1);

        }
        break;

        case DISTRIBUTION_NETWORK_MODEL://����ģ�� WiFi ����ģʽ
        {
            tx_protocol.data_len = 0x04;
            tx_protocol.packet_data[0] = RESPONSE_STATUS;//״̬��STATUS��
            /*
            ״̬��STATUS��
            0x00����ȷ
            0x01������У�����
            0xF0��û�ж��塢δ��ʶ��Ĳ���
            0x07�����������У��������յ��˻ظ����벥����ʾ������WiFi �����У����Եȡ�
            0x08������ WiFi ·�ɳɹ�
            0x09������ WiFi ·�ɳ�ʱʧ�ܣ�180 �룩�������������롢SSID����·���źŲ
            0xFC��ϵͳæ��ִ��ʧ��
            */
        }
        break;

        case CALL_PHONE://�����ֻ�ָ��
        {
            /*
            ����=0x00���������
            ����=0x01��SOS ����
            */
            if ((0x00 == protocol_data->packet_data[0]) || (0x01 == protocol_data->packet_data[0]))
            {
                tx_protocol.data_len = 0x04;
                tx_protocol.packet_data[0] = RESPONSE_STATUS;//״̬��STATUS��
                /*
                ״̬��STATUS��
                0x00����ȷ�����ں�����...)
                0x01������У�����
                0xF0��û�ж��塢δ��ʶ��Ĳ���
                0x0A��30 �볬ʱ�����˽���
                0x0B������ͨ����...
                0x0C��APP ����ͨ��/�����жϵȹһ���������
                0x0F: ����ת����...�����˿���ͨ����״̬������ص���ʾ�����µȴ� 30 �볬ʱ��
                0xFC��ϵͳæ��ִ��ʧ��
                0xFF��ִ���У�����ģ����� 10 �����ٴη���ִ�еĽ��
                */
            }
            else if (0x10 == protocol_data->packet_data[0])//����=0x10������ͨ��
            {
                tx_protocol.data_len = 0x04;
                tx_protocol.packet_data[0] = 0x0C;//״̬��STATUS��
                /*
                ״̬��STATUS��
                0x01������У�����
                0xF0��û�ж��塢δ��ʶ��Ĳ���
                0x0C������ͨ���һ���������
                0xFC��ϵͳæ��ִ��ʧ��
                */
            }
        }
        break;

        case REMOTE_OPEN_LOCK://Ӧ��Զ�̿���ָ��
        {
            /*
            ״̬��STATUS��
            0x00����ȷ�������ɹ���
            0x01������У�����
            0xF0��û�ж��塢δ��ʶ��Ĳ���
            0xFC��ϵͳæ��ִ��ʧ��
            0xFD�������ϣ�����ʧ��
            0xFE���������󣬿���ʧ��
            */
        }
        break;

        case QUICK_POWER_ON_SELF_TEST://����ģ���Լ����������ģʽ
        {

        }
        break;

        case WARNING_NOTICE://�����¼�����֪ͨ
        {

        }
        break;

        case VISUAL_DOORBELL_RESET://�ָ�����ģ��������á����� 
        {
            /*
            ����=0x00���ָ�����ģ��������ã���������ʻ���������������������£�
            ����=0x01���ָ�����·����Ϣ���������ӣ�����������õ�Ĭ��ֵ
            ����=0x02������ģ���������
            */
            tx_protocol.data_len = 0x04;
            tx_protocol.packet_data[0] = RESPONSE_STATUS;//״̬��STATUS��
            /*
            ״̬��STATUS��
            0x00����ȷ
            0x01������У�����
            0xF0��û�ж��塢δ��ʶ��Ĳ���
            0xFC��ϵͳæ��ִ��ʧ��
            */
        }
        break;

        case PARAMETER_SETTING://���ò���
        {
            /*
            ����=0x01��9600 bps��
            ����=0x02��115200 bps��
            ����=0x10��﮵�أ���/˫���Զ�ʶ��
            ����=0x11��AAA ��أ�4 �ڼ��Ե�� 6V��
            ��ע�����������ú�������Ч�����õ������������Ч���ָ��������û�����Ӱ�죬��Ȼ���ֵ�
            ǰ���趨

            */
            tx_protocol.data_len = 0x04;
            tx_protocol.packet_data[0] = RESPONSE_STATUS;//״̬��STATUS��
            /*
            ״̬��STATUS��
            0x00����ȷ
            0x01������У�����
            0xF0��û�ж��塢δ��ʶ��Ĳ���
            0xFC��ϵͳæ��ִ��ʧ��

            */
        }
        break;

        case LOCLA_OPEN_LOCK://����ģ�鱾��������ָ��
        {

        }
        break;

        case LOCLA_CLOSE_LOCK://����ģ�鱾����������ָ��
        {

        }
        break;

        case LOCK_RESET://����ģ������ �ָ����˳������� 
        {

        }
        break;
        
        case WIFI_INFORMATION_SET_UP_NET://wifi ��Ϣ����ָ��
        {
            /*
            ˵����
            ���ݸ�ʽ�����ȣ�1 �ֽڣ�+���ͣ�1 �ֽڣ�+���ݣ�����-1��...����+����+����
            ���ͣ�type����
            1 WIFI ���ƣ����
            2 WIFI ���루���
            3 �������ͣ����� ��0 �����룬1 WEP��2 WPA��4 WPA2��
            4 �û���Կ����ѡ ��APP ͨ�� api ��ȡ���ɣ����ڰ��豸��
            */
            wifi_info_distribution_network_instruction(&protocol_data->packet_data[0U], protocol_data->packet_data_len);
        }
        break;

        case TGIRD_PARTYY_PASSTHROUGH://����������͸��ָ��
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

//�������
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

//����Զ�̿���ָ��
void send_remote_unlock_command(void)
{
    tx_protocol.data_len = 0x09;
    //����룺������ģ���ϵ����ӶԽ�ʱ��ȡ������ʱ���ô���������ȷ�Ͽ���
    tx_protocol.packet_data[0] = 0x12;//�����
    tx_protocol.packet_data[1] = 0x34;//�����
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

//wifi��Ϣ����ָ��
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



