#ifndef _PSP_IODEVICE_H_
#define _PSP_IODEVICE_H_

#include <memory>
#include "Stream.h"

namespace Psp
{
	class CIoDevice
	{
	public:
		virtual ~CIoDevice()
		{
		}
		virtual Framework::CStream* GetFile(const char*, uint32) = 0;
	};

	typedef std::shared_ptr<CIoDevice> IoDevicePtr;
}

#endif
