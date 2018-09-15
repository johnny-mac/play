#pragma once

#include "Types.h"
#include "Convertible.h"
#include "../uint128.h"
#include "../Profiler.h"
#include "zip/ZipArchiveWriter.h"
#include "zip/ZipArchiveReader.h"

//#define DELAYED_MSCAL

class CVpu;
class CINTC;

class CVif
{
public:
	enum
	{
		REGS0_START = 0x10003800,
		VIF0_STAT = 0x10003800,
		VIF0_FBRST = 0x10003810,
		VIF0_MARK = 0x10003830,
		VIF0_CYCLE = 0x10003840,
		VIF0_MODE = 0x10003850,
		VIF0_NUM = 0x10003860,
		VIF0_CODE = 0x10003880,
		VIF0_R0 = 0x10003900,
		VIF0_R1 = 0x10003910,
		VIF0_R2 = 0x10003920,
		VIF0_R3 = 0x10003930,
		REGS0_END = 0x10003A00,

		REGS1_START = 0x10003C00,
		VIF1_STAT = 0x10003C00,
		VIF1_FBRST = 0x10003C10,
		VIF1_MARK = 0x10003C30,
		VIF1_CYCLE = 0x10003C40,
		VIF1_MODE = 0x10003C50,
		VIF1_NUM = 0x10003C60,
		VIF1_CODE = 0x10003C80,
		VIF1_R0 = 0x10003D00,
		VIF1_R1 = 0x10003D10,
		VIF1_R2 = 0x10003D20,
		VIF1_R3 = 0x10003D30,
		REGS1_END = 0x10003E00,

		VIF0_FIFO_START = 0x10004000,
		VIF0_FIFO_END = 0x10004FFF,
		VIF1_FIFO_START = 0x10005000,
		VIF1_FIFO_END = 0x10005FFF,
	};

	CVif(unsigned int, CVpu&, CINTC&, uint8*, uint8*);
	virtual ~CVif() = default;

	virtual void Reset();
	uint32 GetRegister(uint32);
	void SetRegister(uint32, uint32);
	virtual void SaveState(Framework::CZipArchiveWriter&);
	virtual void LoadState(Framework::CZipArchiveReader&);

	virtual uint32 GetTOP() const;
	virtual uint32 GetITOP() const;

	virtual uint32 ReceiveDMA(uint32, uint32, uint32, bool);

	bool IsWaitingForProgramEnd() const;

protected:
	enum
	{
		FBRST_RST = 0x01,
		FBRST_STC = 0x08
	};

	enum
	{
		STAT_FDR = 0x00800000,
	};

	enum
	{
		FIFO_SIZE = 0x100
	};

	class CFifoStream
	{
	public:
		CFifoStream(uint8*, uint8*);
		virtual ~CFifoStream() = default;

		void Reset();

		uint32 GetAvailableReadBytes() const;
		uint32 GetRemainingDmaTransferSize() const;
		void Read(void*, uint32);
		void Flush();
		void Align32();
		void SetDmaParams(uint32, uint32, bool);
		void SetFifoParams(uint8*, uint32);

		uint8* GetDirectPointer() const;
		void Advance(uint32);

	private:
		void SyncBuffer();

		enum
		{
			BUFFERSIZE = 0x10
		};

		uint8* m_ram = nullptr;
		uint8* m_spr = nullptr;

		uint128 m_buffer;
		uint32 m_bufferPosition = BUFFERSIZE;
		uint32 m_startAddress = 0;
		uint32 m_nextAddress = 0;
		uint32 m_endAddress = 0;
		bool m_tagIncluded = false;
		uint8* m_source = nullptr;
	};

	typedef CFifoStream StreamType;

	struct STAT : public convertible<uint32>
	{
		unsigned int nVPS : 2;
		unsigned int nVEW : 1;
		unsigned int nReserved0 : 3;
		unsigned int nMRK : 1;
		unsigned int nDBF : 1;
		unsigned int nVSS : 1;
		unsigned int nVFS : 1;
		unsigned int nVIS : 1;
		unsigned int nINT : 1;
		unsigned int nER0 : 1;
		unsigned int nER1 : 1;
		unsigned int nReserved2 : 9;
		unsigned int nFDR : 1; //VIF1 only
		unsigned int nFQC : 4;
		unsigned int nReserved3 : 4;
	};
	static_assert(sizeof(STAT) == sizeof(uint32), "Size of STAT struct must be 4 bytes.");

	struct CYCLE : public convertible<uint32>
	{
		unsigned int nCL : 8;
		unsigned int nWL : 8;
		unsigned int reserved : 16;
	};
	static_assert(sizeof(CYCLE) == sizeof(uint32), "Size of CYCLE struct must be 4 bytes.");

	struct CODE : public convertible<uint32>
	{
		unsigned int nIMM : 16;
		unsigned int nNUM : 8;
		unsigned int nCMD : 7;
		unsigned int nI : 1;
	};
	static_assert(sizeof(CODE) == sizeof(uint32), "Size of CODE struct must be 4 bytes.");

	enum ADDMODE
	{
		MODE_NORMAL = 0,
		MODE_OFFSET = 1,
		MODE_DIFFERENCE = 2
	};

	enum MASKOP
	{
		MASK_DATA = 0,
		MASK_ROW = 1,
		MASK_COL = 2,
		MASK_MASK = 3
	};

	void ProcessFifoWrite(uint32, uint32);

	void ProcessPacket(StreamType&);
	virtual void ExecuteCommand(StreamType&, CODE);
	virtual void Cmd_UNPACK(StreamType&, CODE, uint32);

	void Cmd_MPG(StreamType&, CODE);
	void Cmd_STROW(StreamType&, CODE);
	void Cmd_STCOL(StreamType&, CODE);
	void Cmd_STMASK(StreamType&, CODE);

	bool Unpack_ReadValue(const CODE&, StreamType&, uint128&, bool);
	bool Unpack_S32(StreamType&, uint128&);
	bool Unpack_S16(StreamType&, uint128&, bool);
	bool Unpack_S8(StreamType&, uint128&, bool);
	bool Unpack_V16(StreamType&, uint128&, unsigned int, bool);
	bool Unpack_V8(StreamType&, uint128&, unsigned int, bool);
	bool Unpack_V32(StreamType&, uint128&, unsigned int);
	bool Unpack_V45(StreamType&, uint128&);

	uint32 GetMaskOp(unsigned int, unsigned int) const;

	virtual void PrepareMicroProgram();
	void StartMicroProgram(uint32);
#ifdef DELAYED_MSCAL
	void StartDelayedMicroProgram(uint32);
	bool ResumeDelayedMicroProgram();
#endif

	void DisassembleCommand(CODE);
	void DisassembleGet(uint32);
	void DisassembleSet(uint32, uint32);

	unsigned int m_number = 0;
	CVpu& m_vpu;
	CINTC& m_intc;
	uint8* m_ram = nullptr;
	uint8* m_spr = nullptr;
	CFifoStream m_stream;

	uint8 m_fifoBuffer[FIFO_SIZE];
	uint32 m_fifoIndex = 0;

	STAT m_STAT;
	CYCLE m_CYCLE;
	CODE m_CODE;
	uint8 m_NUM;
	uint32 m_MODE;
	uint32 m_R[4];
	uint32 m_C[4];
	uint32 m_MASK;
	uint32 m_MARK;
	uint32 m_ITOP;
	uint32 m_ITOPS;
	uint32 m_readTick;
	uint32 m_writeTick;
#ifdef DELAYED_MSCAL
	uint32 m_pendingMicroProgram;
	CODE m_previousCODE;
#endif

	CProfiler::ZoneHandle m_vifProfilerZone = 0;
};
