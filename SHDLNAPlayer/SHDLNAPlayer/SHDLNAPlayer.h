#ifndef _SH_DLNAPLAYER_H_
#define _SH_DLNAPLAYER_H_

#ifdef SH_DLNAPLAYER_EXPORTS
#define SH_DLNAPLAYER_API __declspec(dllexport)
#else
#define SH_DLNAPLAYER_API __declspec(dllimport)
#endif // SH_DLNAPLAYER_EXPORTS

/*----------------------------------------------------------------------
|   DLNA��UI�����Ϣ֪ͨ
+---------------------------------------------------------------------*/

/*
*	�ص�������DLNA��Ϣ֪ͨ
*/
typedef void (*SH_DLNAPlayer_MessageNotifyUI)(int msg, void* wParam, void* lParam);


/*
*	DLNA֪ͨ��Ϣ
*/
typedef enum {
	SH_DLNAPLAYER_UI_MESSAGE_NONE = 0,
	SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_SUCCEEDED,						// ��ý��ɹ�
	SH_DLNAPLAYER_UI_MESSAGE_OPEN_MEDIA_FAILED,							// ��ý��ʧ��

	SH_DLNAPLAYER_UI_MESSAGE_SEEK_SUCCEEDED,							// seek�ɹ�
	SH_DLNAPLAYER_UI_MESSAGE_SEEK_FAILED,								// seekʧ��

	SH_DLNAPLAYER_UI_MESSAGE_GET_MEDIA_INFO_SUCCEEDED,					// ��ȡý����Ϣ�ɹ�
	SH_DLNAPLAYER_UI_MESSAGE_GET_MEDIA_INFO_FAILED,						// ��ȡý����Ϣʧ��

	SH_DLNAPLAYER_UI_MESSAGE_MEDIA_TOTAL_DURATION,						// ������Ƶʱ�� ms��wParam����ʱ��(long)
	SH_DLNAPLAYER_UI_MESSAGE_MEDIA_CURRENT_POS,							// ��ǰ����λ�� ms��wParam���ص�ǰλ��(long)

	SH_DLNAPLAYER_UI_MESSAGE_DEVICE_LIST_UPDATED,						// �豸�б���£�wParam�����豸�б�(SH_DLNAPlayer_DeviceList*)

	SH_DLNAPLAYER_UI_MESSAGE_DEVICE_CURRENT_VOLUME,						// �豸��ǰ����������wParam���ص�ǰ������int)

} SH_DLNAPlayer_UI_Message;

/*----------------------------------------------------------------------
|   DLNA�豸��Ϣ���
+---------------------------------------------------------------------*/
#define SH_DLNAPLAYER_DEVICE_COUNT_MAX				64					// �豸��Ŀ���ֵ
#define SH_DLNAPLAYER_DEVICE_NAME_LENGTH_MAX		256					// �豸�����ַ�����󳤶�
#define SH_DLNAPLAYER_DEVICE_UUID_LENGTH_MAX		64					// �豸UUID�ַ�����󳤶�

// �豸��Ϣ
typedef struct {
	char device_uuid[SH_DLNAPLAYER_DEVICE_UUID_LENGTH_MAX];				// �豸UUID
	char device_name[SH_DLNAPLAYER_DEVICE_NAME_LENGTH_MAX];				// �豸����
} SH_DLNAPlayer_DeviceInfo;

// �豸�б�
typedef struct {
	SH_DLNAPlayer_DeviceInfo	device[SH_DLNAPLAYER_DEVICE_COUNT_MAX];	// �豸��Ϣ
	int							count;									// �豸����
} SH_DLNAPlayer_DeviceList;

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


	/*----------------------------------------------------------------------
	|   DLNA �ӿ�
	+---------------------------------------------------------------------*/

	/*
	*	SH_DLNAPlayer_Init
	*	������			ģ���ʼ��������DLNA���ܣ��Զ���ѯͬ�����µ�DLNA�豸
	*	������			[in] message_to_notify - �ص�������DLNAģ����UI�㷢��֪ͨ��Ϣ
	*	����ֵ��		0 - �ɹ�����0 - ʧ��
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_Init(SH_DLNAPlayer_MessageNotifyUI message_to_notify);

	/*
	*	SH_DLNAPlayer_Uninit
	*	������			ģ�鷴��ʼ����ֹͣDLNA����
	*	������			��
	*	����ֵ��		0 - �ɹ�����0 - ʧ��
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_Uninit(void);

	/*
	*	SH_DLNAPlayer_ChooseDevice
	*	������			ѡ��DLNA�����豸
	*	������			[in] device_uuid - ��ѡ�豸��UUID
	*	����ֵ��		0 - �ɹ�����0 - ʧ��
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_ChooseDevice(const char* device_uuid);

	/*
	*	SH_DLNAPlayer_Open
	*	������			���ļ�
	*	������			[in] path_or_url - �����ļ�·����������ƵURL, ��utf-8���룩
	*	����ֵ��		0 - �ɹ�����0 - ʧ��
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_Open(const char* url_utf8);

	/*
	*	SH_DLNAPlayer_Close
	*	������			�ر��ļ�
	*	������			��
	*	����ֵ��		0 - �ɹ�����0 - ʧ��
	*/
	SH_DLNAPLAYER_API	int		SH_DLNAPlayer_Close(void);

	/*
	*	SH_DLNAPlayer_Play
	*	������			��ʼ����
	*	������			��
	*	����ֵ��		0 - �ɹ�����0 - ʧ��
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_Play(void);

	/*
	*	SH_DLNAPlayer_Seek
	*	������			Seek
	*	������			[in] pos_to_play - ָ������λ�ã���λΪ����
	*	����ֵ��		0 - �ɹ�����0 - ʧ��
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_Seek(long pos_to_play); // ms

	/*
	*	SH_DLNAPlayer_Pause
	*	������			��ͣ����
	*	������			��
	*	����ֵ��		0 - �ɹ�����0 - ʧ��
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_Pause(void);

	/*
	*	SH_DLNAPlayer_Stop
	*	������			ֹͣ����
	*	������			��
	*	����ֵ��		0 - �ɹ�����0 - ʧ��
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_Stop(void);

	/*
	*	SH_DLNAPlayer_SetVolume
	*	������			���ò�������
	*	������			[in] volume - ��������ΧΪ0-100
	*	����ֵ��		0 - �ɹ�����0 - ʧ��
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_SetVolume(int volume); // 0-100


	/*
	*	SH_DLNAPlayer_GetMediaDuration
	*	������			��ȡ��ǰý�岥��ʱ��������Ϣ֪ͨ��ʽ���ؽ��
	*	������			��
	*	����ֵ��		0 - �ɹ�����0 - ʧ��
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_GetMediaDuration(void);

	/*
	*	SH_DLNAPlayer_GetCurPlayPos
	*	������			��ȡ��ǰý�岥��λ�ã�����Ϣ֪ͨ��ʽ���ؽ��
	*	������			��
	*	����ֵ��		0 - �ɹ�����0 - ʧ��
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_GetCurPlayPos(void);

	/*
	*	SH_DLNAPlayer_GetVolume
	*	������			��ȡ��ǰ��������������Ϣ֪ͨ��ʽ���ؽ��
	*	������			��
	*	����ֵ��		0 - �ɹ�����0 - ʧ��
	*/
	SH_DLNAPLAYER_API int		SH_DLNAPlayer_GetVolume(void);


#ifdef __cplusplus
};
#endif // __cplusplus

#endif // _SH_DLNAPLAYER_H_
