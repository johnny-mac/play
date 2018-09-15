#include <assert.h>
#include "../Log.h"
#include "../Ps2Const.h"
#include "Iop_Cdvdfsv.h"
#include "Iop_Cdvdman.h"
#include "Iop_SifManPs2.h"

using namespace Iop;

#define LOG_NAME "iop_cdvdfsv"

#define STATE_FILENAME ("iop_cdvdfsv/state.xml")

#define STATE_PENDINGCOMMAND ("PendingCommand")
#define STATE_PENDINGREADSECTOR ("PendingReadSector")
#define STATE_PENDINGREADCOUNT ("PendingReadCount")
#define STATE_PENDINGREADADDR ("PendingReadAddr")

#define STATE_STREAMING ("Streaming")
#define STATE_STREAMPOS ("StreamPos")
#define STATE_STREAMBUFFERSIZE ("StreamBufferSize")

CCdvdfsv::CCdvdfsv(CSifMan& sif, CCdvdman& cdvdman, uint8* iopRam)
    : m_cdvdman(cdvdman)
    , m_iopRam(iopRam)
{
	m_module592 = CSifModuleAdapter(std::bind(&CCdvdfsv::Invoke592, this,
	                                          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
	m_module593 = CSifModuleAdapter(std::bind(&CCdvdfsv::Invoke593, this,
	                                          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
	m_module595 = CSifModuleAdapter(std::bind(&CCdvdfsv::Invoke595, this,
	                                          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
	m_module596 = CSifModuleAdapter(std::bind(&CCdvdfsv::Invoke596, this,
	                                          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
	m_module597 = CSifModuleAdapter(std::bind(&CCdvdfsv::Invoke597, this,
	                                          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
	m_module59A = CSifModuleAdapter(std::bind(&CCdvdfsv::Invoke59A, this,
	                                          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
	m_module59C = CSifModuleAdapter(std::bind(&CCdvdfsv::Invoke59C, this,
	                                          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));

	sif.RegisterModule(MODULE_ID_1, &m_module592);
	sif.RegisterModule(MODULE_ID_2, &m_module593);
	sif.RegisterModule(MODULE_ID_4, &m_module595);
	sif.RegisterModule(MODULE_ID_5, &m_module596);
	sif.RegisterModule(MODULE_ID_6, &m_module597);
	sif.RegisterModule(MODULE_ID_7, &m_module59A);
	sif.RegisterModule(MODULE_ID_8, &m_module59C);
}

std::string CCdvdfsv::GetId() const
{
	return "cdvdfsv";
}

std::string CCdvdfsv::GetFunctionName(unsigned int) const
{
	return "unknown";
}

void CCdvdfsv::ProcessCommands(CSifMan* sifMan)
{
	if(m_pendingCommand != COMMAND_NONE)
	{
		static const uint32 sectorSize = 0x800;

		uint8* eeRam = nullptr;
		if(auto sifManPs2 = dynamic_cast<CSifManPs2*>(sifMan))
		{
			eeRam = sifManPs2->GetEeRam();
		}

		if(m_pendingCommand == COMMAND_READ)
		{
			if(m_opticalMedia != nullptr)
			{
				auto fileSystem = m_opticalMedia->GetFileSystem();
				for(unsigned int i = 0; i < m_pendingReadCount; i++)
				{
					fileSystem->ReadBlock(m_pendingReadSector + i, eeRam + (m_pendingReadAddr + (i * sectorSize)));
				}
			}
		}
		else if(m_pendingCommand == COMMAND_READIOP)
		{
			if(m_opticalMedia != nullptr)
			{
				auto fileSystem = m_opticalMedia->GetFileSystem();
				for(unsigned int i = 0; i < m_pendingReadCount; i++)
				{
					fileSystem->ReadBlock(m_pendingReadSector + i, m_iopRam + (m_pendingReadAddr + (i * sectorSize)));
				}
			}
		}
		else if(m_pendingCommand == COMMAND_STREAM_READ)
		{
			if(m_opticalMedia != nullptr)
			{
				auto fileSystem = m_opticalMedia->GetFileSystem();
				for(unsigned int i = 0; i < m_pendingReadCount; i++)
				{
					fileSystem->ReadBlock(m_streamPos, eeRam + (m_pendingReadAddr + (i * sectorSize)));
					m_streamPos++;
				}
			}
		}

		m_pendingCommand = COMMAND_NONE;
		sifMan->SendCallReply(MODULE_ID_4, nullptr);
	}
}

void CCdvdfsv::SetOpticalMedia(COpticalMedia* opticalMedia)
{
	m_opticalMedia = opticalMedia;
}

void CCdvdfsv::LoadState(Framework::CZipArchiveReader& archive)
{
	auto registerFile = CRegisterStateFile(*archive.BeginReadFile(STATE_FILENAME));

	m_pendingCommand = static_cast<COMMAND>(registerFile.GetRegister32(STATE_PENDINGCOMMAND));
	m_pendingReadSector = registerFile.GetRegister32(STATE_PENDINGREADSECTOR);
	m_pendingReadCount = registerFile.GetRegister32(STATE_PENDINGREADCOUNT);
	m_pendingReadAddr = registerFile.GetRegister32(STATE_PENDINGREADADDR);

	m_streaming = registerFile.GetRegister32(STATE_STREAMING) != 0;
	m_streamPos = registerFile.GetRegister32(STATE_STREAMPOS);
	m_streamBufferSize = registerFile.GetRegister32(STATE_STREAMBUFFERSIZE);
}

void CCdvdfsv::SaveState(Framework::CZipArchiveWriter& archive)
{
	auto registerFile = new CRegisterStateFile(STATE_FILENAME);

	registerFile->SetRegister32(STATE_PENDINGCOMMAND, m_pendingCommand);
	registerFile->SetRegister32(STATE_PENDINGREADSECTOR, m_pendingReadSector);
	registerFile->SetRegister32(STATE_PENDINGREADCOUNT, m_pendingReadCount);
	registerFile->SetRegister32(STATE_PENDINGREADADDR, m_pendingReadAddr);

	registerFile->SetRegister32(STATE_STREAMING, m_streaming);
	registerFile->SetRegister32(STATE_STREAMPOS, m_streamPos);
	registerFile->SetRegister32(STATE_STREAMBUFFERSIZE, m_streamBufferSize);

	archive.InsertFile(registerFile);
}

void CCdvdfsv::Invoke(CMIPS& context, unsigned int functionId)
{
	throw std::runtime_error("Not implemented.");
}

bool CCdvdfsv::Invoke592(uint32 method, uint32* args, uint32 argsSize, uint32* ret, uint32 retSize, uint8* ram)
{
	switch(method)
	{
	case 0:
	{
		//Init
		assert(argsSize >= 4);
		uint32 mode = args[0x00];
		if(retSize != 0)
		{
			assert(retSize >= 0x10);
			ret[0x03] = 0xFF;
		}
		CLog::GetInstance().Print(LOG_NAME, "Init(mode = %d);\r\n", mode);
	}
	break;
	default:
		CLog::GetInstance().Warn(LOG_NAME, "Unknown method invoked (0x%08X, 0x%08X).\r\n", 0x592, method);
		break;
	}
	return true;
}

bool CCdvdfsv::Invoke593(uint32 method, uint32* args, uint32 argsSize, uint32* ret, uint32 retSize, uint8* ram)
{
	switch(method)
	{
	case 0x01:
	{
		assert(retSize >= 0xC);
		CLog::GetInstance().Print(LOG_NAME, "ReadClock();\r\n");

		auto clockBuffer = reinterpret_cast<uint8*>(ret + 1);
		(*ret) = m_cdvdman.CdReadClockDirect(clockBuffer);
	}
	break;

	case 0x03:
		assert(retSize >= 4);
		CLog::GetInstance().Print(LOG_NAME, "GetDiskType();\r\n");
		ret[0x00] = m_cdvdman.CdGetDiskTypeDirect(m_opticalMedia);
		break;

	case 0x04:
		assert(retSize >= 4);
		CLog::GetInstance().Print(LOG_NAME, "GetError();\r\n");
		ret[0x00] = 0x00;
		break;

	case 0x05:
	{
		assert(argsSize >= 4);
		assert(retSize >= 8);
		uint32 mode = args[0x00];
		CLog::GetInstance().Print(LOG_NAME, "TrayReq(mode = %d);\r\n", mode);
		ret[0x00] = 0x01; //Result
		ret[0x01] = 0x00; //Tray check
	}
	break;

	case 0x0C:
		//Status
		assert(retSize >= 4);
		CLog::GetInstance().Print(LOG_NAME, "Status();\r\n");
		ret[0x00] = m_streaming ? CCdvdman::CDVD_STATUS_SEEK : CCdvdman::CDVD_STATUS_PAUSED;
		break;

	case 0x16:
		//Break
		{
			CLog::GetInstance().Print(LOG_NAME, "Break();\r\n");
			ret[0x00] = 1;
		}
		break;

	case 0x22:
	{
		//Set Media Mode (1 - CDROM, 2 - DVDROM)
		assert(argsSize >= 4);
		assert(retSize >= 4);
		uint32 mode = args[0x00];
		CLog::GetInstance().Print(LOG_NAME, "SetMediaMode(mode = %i);\r\n", mode);
		ret[0x00] = 1;
	}
	break;

	default:
		CLog::GetInstance().Print(LOG_NAME, "Unknown method invoked (0x%08X, 0x%08X).\r\n", 0x593, method);
		break;
	}
	return true;
}

bool CCdvdfsv::Invoke595(uint32 method, uint32* args, uint32 argsSize, uint32* ret, uint32 retSize, uint8* ram)
{
	switch(method)
	{
	case 0x01:
		Read(args, argsSize, ret, retSize, ram);
		return false;
		break;

	case 0x04:
	{
		//GetToc
		assert(argsSize >= 4);
		assert(retSize >= 4);
		uint32 nBuffer = args[0x00];
		CLog::GetInstance().Print(LOG_NAME, "GetToc(buffer = 0x%08X);\r\n", nBuffer);
		ret[0x00] = 1;
	}
	break;

	case 0x05:
	{
		assert(argsSize >= 4);
		uint32 seekSector = args[0];
		CLog::GetInstance().Print(LOG_NAME, "Seek(sector = 0x%08X);\r\n", seekSector);
	}
	break;

	case 0x09:
		return StreamCmd(args, argsSize, ret, retSize, ram);
		break;

	case 0x0D:
		ReadIopMem(args, argsSize, ret, retSize, ram);
		return false;
		break;

	case 0x0E:
		//DiskReady (returns 2 if ready, 6 if not ready)
		assert(retSize >= 4);
		CLog::GetInstance().Print(LOG_NAME, "NDiskReady();\r\n");
		if(m_pendingCommand != COMMAND_NONE)
		{
			ret[0x00] = 6;
		}
		else
		{
			ret[0x00] = 2;
		}
		break;

	default:
		CLog::GetInstance().Warn(LOG_NAME, "Unknown method invoked (0x%08X, 0x%08X).\r\n", 0x595, method);
		break;
	}
	return true;
}

bool CCdvdfsv::Invoke596(uint32 method, uint32* args, uint32 argsSize, uint32* ret, uint32 retSize, uint8* ram)
{
	switch(method)
	{
	default:
		CLog::GetInstance().Warn(LOG_NAME, "Unknown method invoked (0x%08X, 0x%08X).\r\n", 0x596, method);
		break;
	}
	return true;
}

bool CCdvdfsv::Invoke597(uint32 method, uint32* args, uint32 argsSize, uint32* ret, uint32 retSize, uint8* ram)
{
	switch(method)
	{
	case 0:
		SearchFile(args, argsSize, ret, retSize, ram);
		break;
	default:
		CLog::GetInstance().Warn(LOG_NAME, "Unknown method invoked (0x%08X, 0x%08X).\r\n", 0x597, method);
		break;
	}
	return true;
}

bool CCdvdfsv::Invoke59A(uint32 method, uint32* args, uint32 argsSize, uint32* ret, uint32 retSize, uint8* ram)
{
	return Invoke59C(method, args, argsSize, ret, retSize, ram);
}

bool CCdvdfsv::Invoke59C(uint32 method, uint32* args, uint32 argsSize, uint32* ret, uint32 retSize, uint8* ram)
{
	switch(method)
	{
	case 0:
	{
		//DiskReady (returns 2 if ready, 6 if not ready)
		assert(retSize >= 4);
		assert(argsSize >= 4);
		uint32 mode = args[0x00];
		CLog::GetInstance().Print(LOG_NAME, "DiskReady(mode = %i);\r\n", mode);
		ret[0x00] = 2;
	}
	break;
	default:
		CLog::GetInstance().Print(LOG_NAME, "Unknown method invoked (0x%08X, 0x%08X).\r\n", 0x59C, method);
		break;
	}
	return true;
}

void CCdvdfsv::Read(uint32* args, uint32 argsSize, uint32* ret, uint32 retSize, uint8* ram)
{
	uint32 sector = args[0x00];
	uint32 count = args[0x01];
	uint32 dstAddr = args[0x02];
	uint32 mode = args[0x03];

	CLog::GetInstance().Print(LOG_NAME, "Read(sector = 0x%08X, count = 0x%08X, addr = 0x%08X, mode = 0x%08X);\r\n",
	                          sector, count, dstAddr, mode);

	//We write the result now, but ideally should be only written
	//when pending read is completed
	if(retSize >= 4)
	{
		ret[0] = 0;
	}

	assert(m_pendingCommand == COMMAND_NONE);
	m_pendingCommand = COMMAND_READ;
	m_pendingReadSector = sector;
	m_pendingReadCount = count;
	m_pendingReadAddr = dstAddr & 0x1FFFFFFF;
}

void CCdvdfsv::ReadIopMem(uint32* args, uint32 argsSize, uint32* ret, uint32 retSize, uint8* ram)
{
	uint32 sector = args[0x00];
	uint32 count = args[0x01];
	uint32 dstAddr = args[0x02];
	uint32 mode = args[0x03];

	CLog::GetInstance().Print(LOG_NAME, "ReadIopMem(sector = 0x%08X, count = 0x%08X, addr = 0x%08X, mode = 0x%08X);\r\n",
	                          sector, count, dstAddr, mode);

	if(retSize >= 4)
	{
		ret[0] = 0;
	}

	assert(m_pendingCommand == COMMAND_NONE);
	m_pendingCommand = COMMAND_READIOP;
	m_pendingReadSector = sector;
	m_pendingReadCount = count;
	m_pendingReadAddr = dstAddr & 0x1FFFFFFF;
}

bool CCdvdfsv::StreamCmd(uint32* args, uint32 argsSize, uint32* ret, uint32 retSize, uint8* ram)
{
	bool immediateReply = true;

	uint32 sector = args[0x00];
	uint32 count = args[0x01];
	uint32 dstAddr = args[0x02];
	uint32 cmd = args[0x03];
	uint32 mode = args[0x04];

	CLog::GetInstance().Print(LOG_NAME, "StreamCmd(sector = 0x%08X, count = 0x%08X, addr = 0x%08X, cmd = 0x%08X, mode = 0x%08X);\r\n",
	                          sector, count, dstAddr, cmd, mode);

	assert(m_pendingCommand == COMMAND_NONE);

	switch(cmd)
	{
	case 1:
		//Start
		m_streamPos = sector;
		ret[0] = 1;
		CLog::GetInstance().Print(LOG_NAME, "StreamStart(pos = 0x%08X);\r\n", sector);
		m_streaming = true;
		break;
	case 2:
		//Read
		m_pendingCommand = COMMAND_STREAM_READ;
		m_pendingReadSector = 0;
		m_pendingReadCount = count;
		m_pendingReadAddr = dstAddr & (PS2::EE_RAM_SIZE - 1);
		ret[0] = count;
		immediateReply = false;
		CLog::GetInstance().Print(LOG_NAME, "StreamRead(count = 0x%08X, dest = 0x%08X);\r\n",
		                          count, dstAddr);
		break;
	case 3:
		//Stop
		ret[0] = 1;
		CLog::GetInstance().Print(LOG_NAME, "StreamStop();\r\n");
		m_streaming = false;
		break;
	case 5:
		//Init
		ret[0] = 1;
		CLog::GetInstance().Print(LOG_NAME, "StreamInit(bufsize = 0x%08X, numbuf = 0x%08X, buf = 0x%08X);\r\n",
		                          sector, count, dstAddr);
		m_streamBufferSize = sector;
		break;
	case 6:
		//Status
		ret[0] = m_streamBufferSize;
		CLog::GetInstance().Print(LOG_NAME, "StreamStat();\r\n");
		break;
	case 4:
	case 9:
		//Seek
		m_streamPos = sector;
		ret[0] = 1;
		CLog::GetInstance().Print(LOG_NAME, "StreamSeek(pos = 0x%08X);\r\n", sector);
		break;
	default:
		CLog::GetInstance().Warn(LOG_NAME, "Unknown stream command used.\r\n");
		break;
	}

	return immediateReply;
}

void CCdvdfsv::SearchFile(uint32* args, uint32 argsSize, uint32* ret, uint32 retSize, uint8* ram)
{
	uint32 pathOffset = 0x24;
	if(argsSize == 0x128)
	{
		pathOffset = 0x24;
	}
	else if(argsSize == 0x124)
	{
		pathOffset = 0x20;
	}
	else
	{
		CLog::GetInstance().Warn(LOG_NAME, "Warning: Using unknown structure size (%d bytes);\r\n", argsSize);
	}

	assert(retSize == 4);

	if(!m_opticalMedia)
	{
		ret[0] = 0;
		return;
	}

	//0x12C structure
	//00 - Block Num
	//04 - Size
	//08
	//0C
	//10
	//14
	//18
	//1C
	//20 - Unknown
	//24 - Path

	const char* path = reinterpret_cast<const char*>(args) + pathOffset;
	CLog::GetInstance().Print(LOG_NAME, "SearchFile(path = %s);\r\n", path);

	//Fix all slashes
	std::string fixedPath(path);
	{
		auto slashPos = fixedPath.find('\\');
		while(slashPos != std::string::npos)
		{
			fixedPath[slashPos] = '/';
			slashPos = fixedPath.find('\\', slashPos + 1);
		}
	}

	//Hack to remove any superfluous version extensions (ie.: ;1) that might be present in the path
	//Don't know if this is valid behavior but shouldn't hurt compatibility. This was done for Sengoku Musou 2.
	while(1)
	{
		auto semColCount = std::count(fixedPath.begin(), fixedPath.end(), ';');
		if(semColCount <= 1) break;
		auto semColPos = fixedPath.rfind(';');
		assert(semColPos != std::string::npos);
		fixedPath = std::string(fixedPath.begin(), fixedPath.begin() + semColPos);
	}

	ISO9660::CDirectoryRecord record;
	auto fileSystem = m_opticalMedia->GetFileSystem();
	if(!fileSystem->GetFileRecord(&record, fixedPath.c_str()))
	{
		ret[0] = 0;
		return;
	}

	args[0x00] = record.GetPosition();
	args[0x01] = record.GetDataLength();

	ret[0] = 1;
}
