#ifndef _SH_DLNA_MEDIA_PLAYER_H_
#define _SH_DLNA_MEDIA_PLAYER_H_

#include "SHDLNAMediaController.h"

typedef NPT_Reference<PLT_UPnP>						PLT_UPnPReference;

/*----------------------------------------------------------------------
|   class CSHDLNAMediaPlayer
+---------------------------------------------------------------------*/
class CSHDLNAMediaPlayer
{
public:
	CSHDLNAMediaPlayer();
	~CSHDLNAMediaPlayer();

	NPT_Result		Init(SH_DLNAPlayer_MessageNotifyUI message_to_notify);
	NPT_Result		Uninit(void);
	NPT_Result		ChooseDevice(const char* device_uuid);
	NPT_Result		Open(const char* url_utf8);
	NPT_Result		Close(void);
	NPT_Result		Play(void);
	NPT_Result		Seek(long pos_to_play); // ms
	NPT_Result		Pause(void);
	NPT_Result		Stop(void);
	NPT_Result		SetVolume(int volume);	// 0-100

	NPT_Result		GetMediaDuration(void); // ms
	NPT_Result		GetCurPlayPos(void);	// ms
	NPT_Result		GetVolume(void);		// 0-100

	//static NPT_Result GetInputStream(NPT_InputStreamReference& stream);

protected:
	NPT_Result		BuildMediaInfo(const char* url_from_ui_utf8);
	NPT_Result		ParseMediaTitle(const char* url_from_ui_utf8, NPT_String& title);

protected:
	/*
	 *	UPnP manager
	 */
	PLT_UPnPReference					m_UPnP;

	/*
	 *	internal media server
	 */
	PLT_DeviceHostReference				m_MediaServer;

	/*
	 *	media controller
	 */
	NPT_Reference<CSHDLNAMediaController>	m_MediaController;

	/*
	 *	message to notify UI
	 */
	SH_DLNAPlayer_MessageNotifyUI		m_MessageNotifyUI;

	/*
	 *	input stream
	 */
	//static	NPT_InputStreamReference	m_InputStream;

	/*
	 *	media info
	 */
	SHDLNAMediaInfo_t					m_MediaInfo;
};

#endif // _SH_DLNA_MEDIA_PLAYER_H_
