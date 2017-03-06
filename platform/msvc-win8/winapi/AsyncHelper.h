// AsyncHelper.h

#pragma once

#include "SocketEmulation.h"
#include "ThreadEmulation.h"

namespace winapi
{

    int inline wait_action(
        Windows::Foundation::IAsyncAction ^ action)
    {
        HANDLE hEvent = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
        action->Completed = ref new Windows::Foundation::AsyncActionCompletedHandler([hEvent](
            Windows::Foundation::IAsyncAction^, Windows::Foundation::AsyncStatus) {
                SetEvent(hEvent);
        });
        WaitForSingleObjectEx(hEvent, INFINITE, FALSE);
        CloseHandle(hEvent);
        return action->Status == Windows::Foundation::AsyncStatus::Completed 
            ? 0 : SCODE_CODE(action->ErrorCode.Value);
    }

    template <typename TResult>
    int inline wait_operation(
        Windows::Foundation::IAsyncOperation<TResult>  ^ operation, 
        TResult & result)
    {
        HANDLE hEvent = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
        operation->Completed = ref new Windows::Foundation::AsyncOperationCompletedHandler<TResult>([hEvent](
            Windows::Foundation::IAsyncOperation<TResult>  ^, Windows::Foundation::AsyncStatus) {
                SetEvent(hEvent);
        });
        WaitForSingleObjectEx(hEvent, INFINITE, FALSE);
        CloseHandle(hEvent);
        if (operation->Status == Windows::Foundation::AsyncStatus::Completed) {
            result = operation->GetResults();
            return 0;
        } else {
            return SCODE_CODE(operation->ErrorCode.Value);
        }
    }

    template <typename TResult, typename TProgress>
    int inline wait_operation2(
        Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>  ^ operation, 
        TResult & result)
    {
        HANDLE hEvent = CreateEventExW(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
        operation->Completed = ref new Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>([hEvent](
            Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>  ^, Windows::Foundation::AsyncStatus) {
                SetEvent(hEvent);
        });
        WaitForSingleObjectEx(hEvent, INFINITE, FALSE);
        CloseHandle(hEvent);
        if (operation->Status == Windows::Foundation::AsyncStatus::Completed) {
            result = operation->GetResults();
            return 0;
        } else {
            return SCODE_CODE(operation->ErrorCode.Value);
        }
    }

}
