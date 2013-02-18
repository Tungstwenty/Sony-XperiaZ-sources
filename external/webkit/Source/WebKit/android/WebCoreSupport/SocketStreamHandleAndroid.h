/*
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef SocketStreamHandleAndroid_h
#define SocketStreamHandleAndroid_h

#include "config.h"
#include "SocketStreamHandle.h"
#include "WebSocketStreamHandle.h"

#include <utils/Log.h>
//#define LOGSOCKETS(...) ((void)android_printLog(ANDROID_LOG_DEBUG, "WebSockets", __VA_ARGS__))
#define LOGSOCKETS(...)

namespace WebCore {
class SocketStreamHandleAndroid: public SocketStreamHandle {
public:
    SocketStreamHandleAndroid(const KURL& url, SocketStreamHandleClient* client);
    virtual ~SocketStreamHandleAndroid();

    virtual int platformSend(const char* data, int length);
    virtual void platformClose();

    void onConnected();
    void onClosed();
    void onSentData(int length);
    void onReceivedData(const char* data, int length);
    void onError(int error);

private:
    scoped_refptr<android::WebSocketStreamHandle> m_socketStream;
};
}
#endif
