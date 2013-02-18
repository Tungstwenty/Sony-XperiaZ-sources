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
#include "config.h"

#if ENABLE(WEB_SOCKETS)
#include "WebSocketStreamHandle.h"

#include "SocketStreamHandleAndroid.h"

#include <wtf/MainThread.h>

#include <net/proxy/proxy_service.h>
#include <net/base/ssl_config_service_defaults.h>

namespace android {

base::Thread* WebSocketStreamHandle::ioThread()
{
    static base::Thread* networkThread = 0;
    static base::Lock networkThreadLock;

    base::AutoLock lock(networkThreadLock);

    if (!networkThread)
        networkThread = new base::Thread("WebSockets");

    if (!networkThread)
        return 0;

    if (networkThread->IsRunning())
        return networkThread;

    base::Thread::Options options;
    options.message_loop_type = MessageLoop::TYPE_IO;
    if (!networkThread->StartWithOptions(options)) {
        delete networkThread;
        networkThread = 0;
    }

    return networkThread;
}

WebSocketStreamHandle::WebSocketStreamHandle(const GURL& url)
    : m_url(url)
    , m_handle(0)
    , m_maxPendingSendAllowed(0)
    , m_pendingSendData(0)
    , m_dataSent(0)
    , m_sendState(SEND_WAITING)
{
    LOGSOCKETS("WebSocketStreamHandle::WebSocketStreamHandle()");
}

WebSocketStreamHandle::~WebSocketStreamHandle()
{
    LOGSOCKETS("WebSocketStreamHandle::~WebSocketStreamHandle()");
}

void WebSocketStreamHandle::connect()
{
    LOGSOCKETS("WebSocketStreamHandle::connect()");
    base::Thread* thread = ioThread();
    if (thread) {
        thread->message_loop()->PostTask(FROM_HERE,
                                         NewRunnableMethod(this,
                                                           &WebSocketStreamHandle::doConnect));
    }
}

void WebSocketStreamHandle::close()
{
    LOGSOCKETS("WebSocketStreamHandle::close()");
    base::Thread* thread = ioThread();
    if (thread) {
        thread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(this, &WebSocketStreamHandle::doClose));
    }
}

int WebSocketStreamHandle::send(const char* data, int length)
{
    LOGSOCKETS("WebSocketStreamHandle::send()");
    MutexLocker lock(m_mutex);
    m_sendState = SEND_WAITING;
    base::Thread* thread = ioThread();
    if (thread && data && length > 0) {
        thread->message_loop()->PostTask(FROM_HERE, NewRunnableMethod(this, &WebSocketStreamHandle::doSend,
                                                                      data, length));
        while (m_sendState == SEND_WAITING)
            m_condition.wait(m_mutex);
    }

    return m_dataSent;
}

void WebSocketStreamHandle::doStart()
{
    LOGSOCKETS("WebSocketStreamHandle::doStart()");
}

void WebSocketStreamHandle::doConnect()
{
    LOGSOCKETS("WebSocketStreamHandle::doConnect()");
    m_stream = new net::SocketStream(m_url, this);
    m_webRequestContext = new WebRequestContext(false);
    // WebRequestContext takes ownership of the ProxyService.
    m_webRequestContext->set_proxy_service(
        net::ProxyService::CreateUsingSystemProxyResolver(new net::ProxyConfigServiceAndroid(), 1, 0));
    m_webRequestContext->set_cert_verifier(new net::CertVerifier());
    m_webRequestContext->set_ssl_config_service(net::SSLConfigService::CreateSystemSSLConfigService());
    m_stream->set_context(m_webRequestContext.get());
    m_stream->Connect();
}

void WebSocketStreamHandle::doClose()
{
    LOGSOCKETS("WebSocketStreamHandle::doClose()");
    // set proxy service to 0 to make sure it's destroyed now.
    m_webRequestContext->set_proxy_service(0);
    m_stream->Close();
}

void WebSocketStreamHandle::doSend(const char* data, int length)
{
    LOGSOCKETS("WebSocketStreamHandle::doSend()");
    MutexLocker lock(m_mutex);
    if (m_pendingSendData + length >= m_maxPendingSendAllowed)
        length = m_maxPendingSendAllowed - m_pendingSendData - 1;

    if (length <= 0 || !m_stream->SendData(data, length)) {
        m_dataSent = 0;
        m_sendState = SEND_FAILED;
    }
    else {
        m_pendingSendData += length;
        m_dataSent = length;
        m_sendState = SEND_SUCCEEDED;
    }
    m_condition.broadcast();
}

namespace {
static void RunTask(void* v) {
    OwnPtr<Task> task(static_cast<Task*>(v));
    task->Run();
}
}

void WebSocketStreamHandle::runTaskOnMainThread(Task* task)
{
    callOnMainThread(RunTask, task);
}

void WebSocketStreamHandle::OnConnected(net::SocketStream* socket, int max_pending_send_allowed)
{
    LOGSOCKETS("WebSocketStreamHandle::OnConnected()");
    m_maxPendingSendAllowed = max_pending_send_allowed;
    runTaskOnMainThread(NewRunnableMethod(this, &android::WebSocketStreamHandle::mainOnConnected));
}

void WebSocketStreamHandle::OnClose(net::SocketStream* socket)
{
    LOGSOCKETS("WebSocketStreamHandle::OnClose()");
    runTaskOnMainThread(NewRunnableMethod(this, &android::WebSocketStreamHandle::mainOnClosed));
}

void WebSocketStreamHandle::OnSentData(net::SocketStream* socket, int amount_sent)
{
    LOGSOCKETS("WebSocketStreamHandle::OnSentData(%d)", amount_sent);
    if (amount_sent > 0) {
        m_pendingSendData -= amount_sent;
        runTaskOnMainThread(NewRunnableMethod(this, &android::WebSocketStreamHandle::mainOnSentData, amount_sent));
    }
}

void WebSocketStreamHandle::OnReceivedData(net::SocketStream* socket, const char* data, int length)
{
    LOGSOCKETS("WebSocketStreamHandle::OnReceivedData()");
    if (data && length > 0) {
        char* buf = new char[length];
        memcpy(buf, data, length);
        runTaskOnMainThread(NewRunnableMethod(this, &android::WebSocketStreamHandle::mainOnReceivedData, buf, length));
    }
}

void WebSocketStreamHandle::OnError(const net::SocketStream* socket, int error)
{
    LOGSOCKETS("WebSocketStreamHandle::OnError()");
    runTaskOnMainThread(NewRunnableMethod(this, &android::WebSocketStreamHandle::mainOnError, error));
}

void WebSocketStreamHandle::mainOnConnected()
{
    LOGSOCKETS("WebSocketStreamHandle::mainOnConnected()");
    if (m_handle)
        m_handle->onConnected();
}

void WebSocketStreamHandle::mainOnClosed()
{
    LOGSOCKETS("WebSocketStreamHandle::mainOnClosed()");
    if (m_handle)
        m_handle->onClosed();
}

void WebSocketStreamHandle::mainOnSentData(int amount_sent)
{
    LOGSOCKETS("WebSocketStreamHandle::mainOnSentData(%d)", amount_sent);
    if (m_handle)
        m_handle->onSentData(amount_sent);
}

void WebSocketStreamHandle::mainOnReceivedData(char* data, int length)
{
    LOGSOCKETS("WebSocketStreamHandle::mainOnReceivedData()");
    if (m_handle) {
        m_handle->onReceivedData(data, length);
        delete data;
    }
}

void WebSocketStreamHandle::mainOnError(int error)
{
    LOGSOCKETS("WebSocketStreamHandle::mainOnError()");
    if (m_handle)
        m_handle->onError(error);
}
} // namespace android
#endif // ENABLE(WEB_SOCKETS)
