// Charset.h

#pragma once

namespace winapi
{
	class charset_t2
	{
	protected:
		charset_t2()
			: nCodePage_(CP_ACP)
            , bOwn_(FALSE)
			, lpStr_(NULL)
			, nSize_(0)
			, bOwnW_(FALSE)
			, lpStrW_(NULL)
			, nSizeW_(0)
			, ec_(0)
		{
		}

	protected:
        UINT nCodePage_;
		BOOL bOwn_;
		LPSTR lpStr_;
		int nSize_;
		BOOL bOwnW_;
		LPWSTR lpStrW_;
		int nSizeW_;
		int ec_;
	};

	class charset_t
		: charset_t2
	{
	public:
		charset_t(
			LPCSTR lpStr)
		{
			if (lpStr == NULL) {
				return;
			}
			int nSize = strlen(lpStr) + 1;
			a2w(lpStr, nSize);
		}

		charset_t(
			LPCSTR lpStr, 
			int nSize)
		{
			a2w(lpStr, nSize);
		}

		charset_t(
			LPSTR lpStr, 
			int nSize)
		{
			a2w(lpStr, nSize);
		}

		charset_t(
            UINT nCodePage, 
			LPCSTR lpStr)
		{
			if (lpStr == NULL) {
				return;
			}
            nCodePage_ = nCodePage;
			int nSize = strlen(lpStr) + 1;
			a2w(lpStr, nSize);
		}

		charset_t(
            UINT nCodePage, 
			LPCSTR lpStr, 
			int nSize)
		{
            nCodePage_ = nCodePage;
			a2w(lpStr, nSize);
		}

		charset_t(
            UINT nCodePage, 
			LPSTR lpStr, 
			int nSize)
		{
            nCodePage_ = nCodePage;
			a2w(lpStr, nSize);
		}

		~charset_t()
		{
			if (bOwnW_) {
				delete [] lpStrW_;
			}
			if (ec_) {
				SetLastError(ec_);
			}
		}

	public:
		void a2w(
			LPCSTR lpStr, 
			int nSize)
		{
			nSizeW_ = nSize;
			lpStrW_ = (LPWSTR)new WCHAR[nSizeW_];
			if (lpStrW_ == NULL) {
				ec_ = ERROR_NOT_ENOUGH_MEMORY;
				return;
			}
			if ((nSizeW_ = a2w(nCodePage_, lpStr, nSize, lpStrW_, nSizeW_)) == 0) {
				delete [] lpStrW_;
				lpStrW_ = NULL;
			}
			bOwnW_ = TRUE;
		}

        void a2w(
            LPSTR lpStr, 
			int nSize)
        {
 			lpStr_ = lpStr;
			nSize_ = nSize;
			nSizeW_ = nSize;
            if (lpStr_ != NULL && nSize_ != 0) {
			    lpStrW_ = (LPWSTR)new WCHAR[nSizeW_];
			    if (lpStrW_ == NULL) {
				    ec_ = ERROR_NOT_ENOUGH_MEMORY;
				    return;
			    }
                bOwnW_ = TRUE;
            }
       }

		void w2a()
		{
            if (lpStr_ != NULL && nSize_ != 0)
			    nSize_ = w2a(nCodePage_, lpStrW_, nSizeW_, lpStr_, nSize_);
            else
                nSize_ = nSizeW_;
		}

        void copy()
        {
            if (lpStr_ != NULL && nSize_ != 0) {
                if (nSizeW_ < nSize_)
                    nSize_ = nSizeW_;
			    memcpy(lpStr_, lpStrW_, nSize_);
            } else {
                nSize_ = nSizeW_;
            }
        }

	public:
		LPWSTR wstr() const
		{
			return lpStrW_;
		}

		int wlen() const
		{
			return nSizeW_;
		}

		void wlen(
			int nSizeW)
		{
			nSizeW_ = nSizeW;
		}

		int len() const
		{
			return nSize_;
		}

		int ec() const
		{
			return ec_;
		}

	public:
		static int w2a(
            _In_       UINT nCodePage, 
			_In_       LPCWSTR lpWideCharStr,
			_In_       int cchWideChar,
			_Out_opt_  LPSTR lpMultiByteStr,
			_In_       int cbMultiByte)
		{
			return ::WideCharToMultiByte(
				nCodePage, 
				0, 
				lpWideCharStr, 
				cchWideChar, 
				lpMultiByteStr, 
				cbMultiByte, 
				NULL, NULL);
		}

		static int a2w(
            _In_       UINT nCodePage, 
			_In_       LPCSTR lpMultiByteStr,
			_In_       int cbMultiByte,
			_Out_opt_  LPWSTR lpWideCharStr,
			_In_       int cchWideChar)
		{
			return ::MultiByteToWideChar(
				nCodePage, 
				0, 
				lpMultiByteStr, 
				cbMultiByte, 
				lpWideCharStr, 
				cchWideChar);
		}

		static int w2a(
			_In_       LPCWSTR lpWideCharStr,
			_In_       int cchWideChar,
			_Out_opt_  LPSTR lpMultiByteStr,
			_In_       int cbMultiByte)
		{
			return ::WideCharToMultiByte(
				CP_ACP, 
				0, 
				lpWideCharStr, 
				cchWideChar, 
				lpMultiByteStr, 
				cbMultiByte, 
				NULL, NULL);
		}

		static int a2w(
			_In_       LPCSTR lpMultiByteStr,
			_In_       int cbMultiByte,
			_Out_opt_  LPWSTR lpWideCharStr,
			_In_       int cchWideChar)
		{
			return ::MultiByteToWideChar(
				CP_ACP, 
				0, 
				lpMultiByteStr, 
				cbMultiByte, 
				lpWideCharStr, 
				cchWideChar);
		}
	};

}
