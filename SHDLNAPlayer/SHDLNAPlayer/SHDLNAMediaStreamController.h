#ifndef _SH_DLNA_MEDIA_STREAM_CONTROLLER_H_
#define _SH_DLNA_MEDIA_STREAM_CONTROLLER_H_

#include "SHDLNAMediaStream.h"
#include "Mp4DownloadClient.h"

//�洢ͷ��Ϣ
struct CHeaderInfo 
{
	QtSequence sequence;
	SHVideoInfo info;
	vector<SHCDNInfo> cdninfo;
	~CHeaderInfo()
	{
		vector<SHCDNInfo>().swap(cdninfo);
	}
};

//�洢���ݵ�Ԫ
struct CDataBuffer
{
	unsigned char* buffer;
	unsigned int size;
	unsigned int readpos;
	CDataBuffer():buffer(NULL),size(0),readpos(0){};
};

//������������
typedef enum {
	SHDLNAPLAYER_MEDIA_UNKNOWN					= 0,		// δ֪ý��
	SHDLNAPLAYER_MEDIA_LOCAL					= 1,		// ����ý��
	SHDLNAPLAYER_MEDIA_ONLINE					= 2		    // ����ý��
} SHDLNAPlayer_MediaType;

/*----------------------------------------------------------------------
|   class CSHDLNAMediaStreamController
+---------------------------------------------------------------------*/
class CSHDLNAMediaStreamController : public IMediaStream
{
	friend class CMp4DownloadClient;

public:
	CSHDLNAMediaStreamController();
	virtual ~CSHDLNAMediaStreamController();

	int Open(const char* name);
	int Close();
	int Read(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read);
	int Seek(unsigned __int64 offset);
	int Tell(unsigned __int64* offset);
	int GetSize(unsigned __int64* size);
	int GetAvailable(unsigned __int64* available);

	static size_t RecvMediaDataCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid);
	static int ProgressCallBack(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
	static unsigned __stdcall ThreadEntryPoint(void * pThis);

public:
	//0 ִ�У�-1 ֹͣ
	int m_status;
	NPT_List<CDataBuffer>& GetBufferList(){		return m_bufferList;	}
	//�������������
	NPT_Mutex m_bufferMutex;
	NPT_Mutex m_statusMutex;
	NPT_Mutex m_headerMutex;

private:
	void RequestMediaData();

	unsigned __int64 m_fileSize;
	unsigned __int64 m_available;
	unsigned __int64 m_curPos;
	//��ǰ�����󵽵�����λ��
	unsigned __int64 m_reqPos;

	//�Ƿ���seek
	bool m_isSeek;
	//ֹͣ�߳�ִ��
	bool m_isRun;

	HANDLE m_handle;

	//�ļ���Ϣ
	FILE *m_openFile;
	NPT_String m_openFileName;
	CString m_playFileName;
	static NPT_String m_lastFileName;
	SHDLNAPlayer_MediaType m_type;

	//http����ͻ���
	CMp4DownloadClient m_client;

	//�����б�
	//��ֻ֤����һ��ͷ
	static bool m_isReqHeader;
	//�洢ͷ�ṹ
	static CHeaderInfo* m_headerInfo;
	//�洢���ݶ���
	NPT_List<CDataBuffer> m_bufferList;
};


#endif // _SH_DLNA_MEDIA_STREAM_CONTROLLER_H_