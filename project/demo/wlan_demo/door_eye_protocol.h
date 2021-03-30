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
 DATA_PASSTHROUGH = 0xFF, //����ģ��͸��     ��<--->ģ��
 GET_TIMESTAMP = 0xA5,  //��ȡʱ��
 VISUAL_DOORBELL_STATUS = 0xA9, //��ȡģ��״̬
 DISTRIBUTION_NETWORK_MODEL = 0xAF,//WIFI����ģʽ
 CALL_PHONE = 0xAC,//�����ֻ�
 REMOTE_OPEN_LOCK = 0xAE,//Զ�̿���
 QUICK_POWER_ON_SELF_TEST = 0xB1,//����ģ���Լ�-��ѹ-����
 WARNING_NOTICE = 0xCC, //�����¼�����
 VISUAL_DOORBELL_RESET = 0xC4,//����ģ��ָ���������
 PARAMETER_SETTING = 0xC8,//��������
 LOCLA_OPEN_LOCK = 0xD3,//���ؿ���
 LOCLA_CLOSE_LOCK = 0xD4,//���ع���
 LOCK_RESET = 0xD5,//���ָ���������
 WIFI_INFORMATION_SET_UP_NET = 0x02,//WIFI��Ϣ��������
 TGIRD_PARTYY_PASSTHROUGH = 0x07,//����������͸�� ��<-->��̨
};

/*
״̬��STATUS��
����ģ����Ӧ�ظ�����
*/
enum Response{
 RESPONSE_STATUS = 0x00, /*��ȷ��ִ�гɹ���*/
 RESPONSE_DATA_CHECKOUT = 0x01,/*����У����� */
 CAPTURE_JPEG_PUSH_0_0_1 = 0x02,/*ץ��ʧ�ܣ�¼��ʧ�ܣ����ͳɹ����ϴ�ͼƬ��ʱ��*/
 CAPTURE_JPEG_PUSH_0_0_0 = 0x03,/*ץ��ʧ�ܣ�¼��ʧ�ܣ�����ʧ�ܣ��������� SD ��¼��ʱ��*/
 CAPTURE_JPEG_PUSH_0_1_0 = 0x04,/*ץ��ʧ�ܣ�¼��ɹ�������ʧ�ܣ������� SD ��¼��ʱ��*/
 CAPTURE_JPEG_PUSH_1_0_1 = 0x05,/*ץ�ĳɹ���¼��ʧ�ܣ����ͳɹ����� TF �����������޷�ʶ����Ҫ��ʽ����*/
 OPERATION_FREQUENTLY = 0x06,
 /*����Ƶ����¼���ظ������ļ�� 60 �룬ץͼ�ظ��ļ�� 10 �룬��ץͼ/¼���ظ��ļ�� 2 �� */
 WIFI_VOICE_PROMPT = 0x07,/*���������У��������յ��˻ظ����벥����ʾ������WiFi �����У����Եȡ�*/
 WIFI_SET_UP_NET_SUCCEED = 0x08,/* ���� WiFi ·�ɳɹ� */
 WFIF_CONNECTION_TIMEOUT = 0x09,/* ���� WiFi ·�ɳ�ʱʧ�ܣ�180 �룩�������������롢SSID����·���źŲ� */
 CALL_CONNECTION_TIMEOUT = 0x0A,/* ���� 30 �볬ʱ�����˽��� */
 LIVING_BROADCAST = 0x0B, /* ������Ƶֱ��ͨ����... */
 APP_SLEEP = 0x0C, /* APP ����ͨ��/�����жϵȹһ��������� */
 CALL_CONNECTION = 0x0F, /* ����ת����...�����˿���ͨ����״̬������ص���ʾ�����µȴ� 30 �볬ʱ�� */
 RESPONSE_RESERVE = 0xF0, /* û�ж��塢δ��ʶ��Ĳ��� */
 RESPONSE_BUSY = 0xFC, /* ϵͳæ��ִ��ʧ�� */
 EXECUTION_BUSY_10 = 0xFF, /* ִ���У�����ģ����� 10 �����ٴη���ִ�еĽ�� */
};


typedef struct
{
    unsigned short start_code;//��ʼ��
    unsigned char PID;//��ʶ
    unsigned short data_len;//���ݳ���
    unsigned char cmd_code;//ָ����
    unsigned char packet_data[PACKET_DATA_MAX_LEN];//������
    unsigned short packet_data_len;//�����ݳ���
    unsigned short check_sum;//У���
    unsigned short end_code;//������
}DOOR_EYE_PROTOCOL_DATA;

typedef struct
{
    unsigned char second; //��
    unsigned char minute; //��
    unsigned char hour;   //ʱ
    unsigned char day;    //��
    unsigned char month;  //��
    unsigned char year;   //��
}DATE;

typedef struct
{
    unsigned char idle_state: 1U;//1=���Դ����κ��0=æ��������
    unsigned char wifi_connect_state: 1U;//1=�����ӣ�0=δ���ӻ�������
    unsigned char login_service_state: 1U;//1=���������0=���������
    unsigned char communication_mode: 1U;//1=��Զ�����绽�ѣ�0=�����ػ���
    unsigned char telecommunication: 1U;//1=����ͨ���У�0=�������¼�
    unsigned char Doorbell_call:1U;//1=�����У�0=���н���
    unsigned char Video_state:1U;//1=¼���У�0=��¼��/¼�������
    unsigned char reserved: 1U;//����
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

