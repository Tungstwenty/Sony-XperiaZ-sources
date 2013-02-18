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
#ifndef WebSocketStreamHandle_h
#define WebSocketStreamHandle_h

#include "config.h"
#include "ChromiumIncludes.h"
#include "WebRequestContext.h"

#include <wtf/Threading.h>

#include <net/socket_stream/socket_stream.h>

namespace WebCore {
class SocketStreamHandleAndroid;
}

namespace android {

class WebSocketStreamHandle : public base::RefCountedThreadSafe<WebSocketStreamHandle>,
                              public net::SocketStream::Delegate {
public:
    static base::Thread* ioThread();

    WebSocketStreamHandle(const GURL& url);
    virtual ~WebSocketStreamHandle();

    void setSocketStreamHandle(WebCore::SocketStreamHandleAndroid* handle) { m_handle = handle; }

    void connect();
    int send(const char*, int);
    void close();

    // These are the SocketStream::Delegate callbacks
    virtual void OnConnected(net::SocketStream* socket, int max_pending_send_allowed);
    virtual void OnSentData(net::SocketStream* socket, int amount_sent);
    virtual void OnReceivedData(net::SocketStream* socket, const char* data, int length);
    virtual void OnClose(net::SocketStream* socket);
    virtual void OnError(const net::SocketStream* socket, int error);

private:
    enum SendState {
        SEND_WAITING,
        SEND_SUCCEEDED,
        SEND_FAILED
    };
    void runTaskOnMainThread(Task*);
    void doStart();
    void doConnect();
    void doSend(const char* , int);
    void doClose();

    // These callbacks are executed on the main thread
    void mainOnConnected();
    void mainOnClosed();
    void mainOnSentData(int amount_sent);
    void mainOnReceivedData(char* data, int length);
    void mainOnError(int error);

    GURL m_url;
    scoped_refptr<net::SocketStream> m_stream;
    scoped_refptr<WebRequestContext> m_webRequestContext;
    WebCore::SocketStreamHandleAndroid* m_handle;
    WTF::Mutex m_mutex;
    WTF::ThreadCondition m_condition;
    int m_maxPendingSendAllowed;
    int m_pendingSendData;
    int m_dataSent;
    SendState m_sendState;
};
} // namespace android
#endif
