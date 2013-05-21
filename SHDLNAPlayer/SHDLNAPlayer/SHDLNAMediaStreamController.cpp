
#include "stdafx.h"
#include <assert.h>
#include "SHDLNAMediaStreamController.h"

//
bool CSHDLNAMediaStreamController::m_isReqHeader = true;

CHeaderInfo* CSHDLNAMediaStreamController::m_headerInfo = NULL;

NPT_String CSHDLNAMediaStreamController::m_lastFileName;

NPT_SET_LOCAL_LOGGER("shdlnaplayer.shdlnastreamctrl")
/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
CSHDLNAMediaStreamController::CSHDLNAMediaStreamController()
{
	m_fileSize = 0;
	m_available = 0;
	m_curPos = 0;
	m_status = 0;

	m_reqPos = 0;
	m_isSeek = false;
	m_isRun = true;

	m_handle = NULL;

	m_openFile = NULL;

	m_type = SHDLNAPLAYER_MEDIA_UNKNOWN;

	m_openFileName = "";
	m_playFileName = "";
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
CSHDLNAMediaStreamController::~CSHDLNAMediaStreamController()
{
	Close();
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::Open(const char* name)
{
	int ret = 0;

	m_openFileName = name;

	if ( m_openFileName.Find("vid") != -1 && m_openFileName != m_lastFileName )
	{
		m_isReqHeader = true;
		m_lastFileName = name;
	}

	NPT_LOG_FATAL_1("the url is %s\n",name);

	if ( m_openFileName.StartsWith("http:") || m_openFileName.StartsWith("https:") )
	{
		m_type = SHDLNAPLAYER_MEDIA_ONLINE;
	}
	else
	{
		m_type = SHDLNAPLAYER_MEDIA_LOCAL;
	}

	if ( m_type == SHDLNAPLAYER_MEDIA_LOCAL )
	{
		ret = _wfopen_s(&m_openFile,mbs2wcs(CP_UTF8,m_openFileName).c_str(),L"rb,ccs=UNICODE");
		if(!ret)
		{		
			fseek(m_openFile,0,SEEK_END);
			m_fileSize = ftell(m_openFile);
			m_available = m_fileSize;
			rewind(m_openFile);
		}
	}
	else if ( m_type == SHDLNAPLAYER_MEDIA_ONLINE )
	{
		if (m_isReqHeader)
		{
			m_isReqHeader = false;
			{
				NPT_AutoLock lock(m_headerMutex);
				if (m_headerInfo)
				{
					delete m_headerInfo;
					m_headerInfo = NULL;
				}
				m_headerInfo = new CHeaderInfo();
			}
			
			m_client.InitialRequest(&m_headerInfo->sequence,&m_headerInfo->info,&m_headerInfo->cdninfo);
			{
				NPT_AutoLock lock(m_statusMutex);
				m_status = 0;
			}

			int result = m_client.HeaderRequest(m_openFileName.GetChars(),m_playFileName,NULL,NULL,NULL);

			if (result < 0)
			{
				m_isReqHeader = true;
				NPT_LOG_FATAL_1("request the url %s header failed\n",name);
				return -1;
			}

			unsigned int filesize = 0;
			m_client.GetRequestFileInfo(&filesize);
			m_reqPos = 0;
			m_fileSize = filesize;

			NPT_LOG_FATAL_1("request the url %s header success\n",name);
		}
		else
		{
			if( !m_headerInfo->cdninfo.empty() && m_headerInfo->sequence.GetSectionCount() == m_headerInfo->cdninfo.size() )
			{
				m_client.InitialRequest(&m_headerInfo->sequence,&m_headerInfo->info,&m_headerInfo->cdninfo);

				unsigned int filesize = 0;
				m_client.GetRequestFileInfo(&filesize);
				
				m_reqPos = 0;
				m_fileSize = filesize;

				NPT_LOG_FATAL_1("Open success, filesize is %d\n.",filesize);
			}
			else
			{
				NPT_LOG_FATAL("Open failed, waiting for header.");
				return -1;
			}
		}
		m_handle = (HANDLE)_beginthreadex( NULL,          
			0,              
			ThreadEntryPoint,  
			this,             
			0,    
			NULL );  

	}
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::Close()
{
	int ret = 0;
	if (m_type == SHDLNAPLAYER_MEDIA_LOCAL)
	{
		if (m_openFile)
		{
			ret = fclose(m_openFile);
			m_openFile = NULL;
		}
	}
	else if(m_type == SHDLNAPLAYER_MEDIA_ONLINE)
	{
		{
			NPT_AutoLock lock(m_statusMutex);
			m_status = -1;
			NPT_LOG_FATAL("status set to -1");
		}
		
		m_isRun = false;
		m_reqPos = m_fileSize;
		WaitForSingleObject(m_handle,INFINITE);

		NPT_AutoLock lock(m_bufferMutex);
		while (m_bufferList.GetItemCount())
		{
			NPT_List<CDataBuffer>::Iterator it = m_bufferList.GetFirstItem();
			delete []it->buffer;
			it->buffer = NULL;
			m_bufferList.Erase(it);
		}
		m_bufferList.Clear();
		m_client.FlushRequest();
	}

	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::Read(void* buffer, unsigned int bytes_to_read, unsigned int* bytes_read)
{
	int ret = 0;
	if (m_type == SHDLNAPLAYER_MEDIA_LOCAL)
	{
		if (m_openFile)
			*bytes_read = fread(buffer,1,bytes_to_read,m_openFile);
	}
	else if (m_type == SHDLNAPLAYER_MEDIA_ONLINE)
	{
		if(!m_fileSize)
			return -1;

		unsigned char* ppBuffer = NULL;
		unsigned int pBufferSize(0);
		m_client.GetRequestFileHeader(&ppBuffer,&pBufferSize);
		if (pBufferSize == 0)
			return -1;

		if (m_curPos<pBufferSize)
		{
			unsigned int tmp_bytes = pBufferSize-static_cast<unsigned int>(m_curPos);
			*bytes_read = (bytes_to_read>tmp_bytes) ? tmp_bytes : bytes_to_read;
			memcpy(buffer,ppBuffer+m_curPos,*bytes_read);
			m_curPos+=*bytes_read;

			NPT_String strRange="file size is ";
			strRange += NPT_String::FromIntegerU(m_fileSize);
			strRange += ",Header size is ";
			strRange += NPT_String::FromIntegerU(pBufferSize);
			strRange += ",current size is ";
			strRange += NPT_String::FromIntegerU(m_curPos);
			
			NPT_LOG_FATAL(strRange);
		}
		else
		{
			if (m_bufferList.GetItemCount())
			{
				NPT_List<CDataBuffer>::Iterator it = m_bufferList.GetFirstItem();

				if(it->readpos+bytes_to_read <= it->size)
				{
					*bytes_read = bytes_to_read;
					memcpy(buffer,it->buffer+it->readpos,*bytes_read);

				}
				else
				{
					*bytes_read = it->size-it->readpos;
					memcpy(buffer,it->buffer+it->readpos,*bytes_read);

				}
				
				it->readpos += *bytes_read;
				m_curPos += *bytes_read;

				NPT_String strRange = "File size is ";
				strRange += NPT_String::FromIntegerU(m_fileSize);
				strRange += ",header size is ";
				strRange += NPT_String::FromIntegerU(pBufferSize);
				strRange += ",current size is ";
				strRange += NPT_String::FromIntegerU(m_curPos);

				NPT_LOG_FATAL(strRange);
			
				NPT_AutoLock lock(m_bufferMutex);
				if (it->readpos == it->size)
				{
					delete []it->buffer;
					it->buffer = NULL;
					m_bufferList.Erase(it);
				}
			}
			else
			{
				*bytes_read = 0;
			}
		}
	}
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::Seek(unsigned __int64 offset)
{
	int ret = 0;
	if (m_type == SHDLNAPLAYER_MEDIA_LOCAL)
	{
		if (m_openFile)
			ret = fseek(m_openFile,(long)offset,SEEK_CUR);
	}
	else if (m_type == SHDLNAPLAYER_MEDIA_ONLINE)
	{
		{
			NPT_AutoLock lock(m_statusMutex);
			m_status = -1;
		}

		if (m_fileSize)
		{
			m_curPos = offset;
			m_isSeek = true;
			NPT_LOG_FATAL_1("Seek position %u",m_curPos);
		}
		else
			ret = -1;
	}
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::Tell(unsigned __int64* offset)
{
	int ret = 0;
	if (m_type == SHDLNAPLAYER_MEDIA_LOCAL)
	{
		//On failure, -1L is returned
		if (m_openFile)
		{
			*offset = ftell(m_openFile);
		}
		return (int)*offset;
	}
	else if (m_type == SHDLNAPLAYER_MEDIA_ONLINE)
	{
		if (m_fileSize)
		{
			*offset = m_curPos;
			NPT_LOG_FATAL_1("Tell position %u\n",m_curPos);
		}
		else
			ret = -1;
	}
	return ret;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::GetSize(unsigned __int64* size)
{
	if (!m_fileSize)
	{
		unsigned int filesize = 0;
		m_client.GetRequestFileInfo(&filesize);
		m_fileSize = filesize;

		*size = filesize;

	}
	else
		*size = m_fileSize;
	return 0;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::GetAvailable(unsigned __int64* available)
{
	*available = m_available;
	return 0;
}

/*----------------------------------------------------------------------
|   
+---------------------------------------------------------------------*/
int CSHDLNAMediaStreamController::ProgressCallBack(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	int status(0);
	if( clientp == NULL )
		status = 0;
	else
	{
		CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)clientp);
		if (str)
		{
			CSHDLNAMediaStreamController* pThis = dynamic_cast<CSHDLNAMediaStreamController*>((CSHDLNAMediaStreamController *)str->m_customparam);
			if (pThis)
			{
				status = pThis->m_status;
			}
			else
				status = 0;
		}
		else
			status = 0;
	}

	//NPT_LOG_FATAL_1("process status is %d\n",status);
	return status;
}

size_t CSHDLNAMediaStreamController::RecvMediaDataCallBackFunc(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	if (buffer == NULL)
		return 0;

	CEasyRequest* str = dynamic_cast<CEasyRequest*>((CEasyRequest *)lpVoid);
	CSHDLNAMediaStreamController* pThis = NULL;
	if (str)
	{
		pThis = dynamic_cast<CSHDLNAMediaStreamController*>((CSHDLNAMediaStreamController *)str->m_customparam);
	}

	if(str && str->m_response_datasize)
	{
		size_t copysize = size * nmemb;
		if (str->m_response_size + copysize > str->m_response_datasize)
		{
			copysize = str->m_response_datasize - str->m_response_size;
		}

		str->m_response_size += copysize;

		CDataBuffer recv_data;
		recv_data.size = copysize;
		recv_data.buffer = new unsigned char[copysize]();
		memcpy(recv_data.buffer,buffer,copysize);

		NPT_LOG_FATAL_1("receive data is %u",copysize);

		if (pThis)
		{
			NPT_AutoLock lock(pThis->m_bufferMutex);
			pThis->GetBufferList().Add(recv_data);
		}

	}
	else
	{
		CDataBuffer recv_data;
		recv_data.size = nmemb;
		recv_data.buffer = new unsigned char[nmemb]();
		memcpy(recv_data.buffer,buffer,nmemb);

		NPT_LOG_FATAL_1("receive data is %u",nmemb);

		if (pThis)
		{
			NPT_AutoLock lock(pThis->m_bufferMutex);
			pThis->GetBufferList().Add(recv_data);
		}
	}

	return nmemb;
}


void CSHDLNAMediaStreamController::RequestMediaData()
{    
	//m_status=0;
	unsigned int pBufferSize;
	m_client.GetRequestFileHeader(NULL,&pBufferSize);

	unsigned int startPos(0);

	//播放到结尾停止
	while (m_isRun)
	{
// 		用户seek了视频
		if (m_isSeek)
		{
			NPT_String msg = "seek event, pos is ";
			msg += NPT_String::FromIntegerU(m_curPos);
			NPT_LOG_FATAL(msg);

			startPos = m_curPos;
			m_isSeek = false;
			if(startPos < pBufferSize)
				startPos = pBufferSize;

			{
				NPT_AutoLock lock(m_statusMutex);
				m_status = 0;
			}

			while(m_reqPos < m_fileSize)
			{
				if ( m_curPos>= m_reqPos )
				{
					int result ;
					{
						NPT_AutoLock lock(m_headerMutex);
						result = m_client.DataRequest(startPos, startPos + 1 * 1024 *1024, 
							(unsigned int*)&m_reqPos,RecvMediaDataCallBackFunc,ProgressCallBack,this);
					}
					if (result >=  0)
					{
						startPos = m_reqPos;
					}
					else
					{
						break;
					}
				}
				else
					Sleep(10);
			}
		}
		else
			Sleep(10);
	}
}

unsigned __stdcall CSHDLNAMediaStreamController::ThreadEntryPoint(void * pThis)  
{  
	CSHDLNAMediaStreamController * pthX = (CSHDLNAMediaStreamController*)pThis;    
	if (pthX->m_fileSize)
	{
		pthX->RequestMediaData();
	}
	else
		NPT_LOG_FATAL("can't RequestMediaData");

	return 0;												   
}
