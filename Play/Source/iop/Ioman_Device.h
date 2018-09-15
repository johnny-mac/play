#ifndef _IOMAN_DEVICE_H_
#define _IOMAN_DEVICE_H_

#include "Stream.h"

namespace Iop
{
	namespace Ioman
	{
		class CDevice
		{
		public:
			enum OPEN_FLAGS
			{
				OPEN_FLAG_RDONLY = 0x00000001,
				OPEN_FLAG_WRONLY = 0x00000002,
				OPEN_FLAG_RDWR = 0x00000003,
				OPEN_FLAG_ACCMODE = 0x00000003,
				OPEN_FLAG_CREAT = 0x00000200,
				OPEN_FLAG_TRUNC = 0x00000400,
				OPEN_FLAG_NOWAIT = 0x00008000,
			};

			virtual ~CDevice()
			{
			}
			virtual Framework::CStream* GetFile(uint32, const char*) = 0;
		};
	}
}

#endif
