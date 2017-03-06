// TlsPointer.h

#pragma once

#include "SocketEmulation.h"
#include "ThreadEmulation.h"

namespace winapi
{

	template<typename T>
	class tls_pointer
	{
	public:
		tls_pointer()
		{
			tls_index_ = TlsAlloc();
		}

		~tls_pointer()
		{
			TlsFree(tls_index_);
		}

	public:
		T * get()
		{
			T * d = (T *)TlsGetValue(tls_index_);
			if (d == NULL) {
				d = new T;
				TlsSetValue(tls_index_, d);
			}
			return d;
		}

		T * operator->()
		{
			return get();
		}

	private:
		DWORD tls_index_;
	};

}
