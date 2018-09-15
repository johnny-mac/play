#include <stdexcept>
#include "Ps2_PsfDevice.h"
#include "PtrStream.h"
#include "stricmp.h"

using namespace PS2;

CPsfDevice::CPsfDevice()
{
}

CPsfDevice::~CPsfDevice()
{
}

void CPsfDevice::AppendArchive(const CPsfBase& archive)
{
	m_fileSystem.AppendArchive(archive);
}

Framework::CStream* CPsfDevice::GetFile(uint32 mode, const char* path)
{
	if(mode != CDevice::OPEN_FLAG_RDONLY)
	{
		printf("%s: Attempting to open a file in non read mode.\r\n", __FUNCTION__);
		return nullptr;
	}
	Framework::CStream* result = m_fileSystem.GetFile(path);
	return result;
}
