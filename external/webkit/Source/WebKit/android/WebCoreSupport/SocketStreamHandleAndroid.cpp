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
#include "SocketStreamHandleAndroid.h"

#include "NotImplemented.h"
#include "PlatformString.h"
#include "SocketStreamHandleClient.h"
#include "SocketStreamError.h"

#include <wtf/text/CString.h>

namespace WebCore {

PassRefPtr<SocketStreamHandle> SocketStreamHandle::create(const KURL& url, SocketStreamHandleClient* client)
{
    return adoptRef(new SocketStreamHandleAndroid(url, client));
}

SocketStreamHandleAndroid::SocketStreamHandleAndroid(const KURL& url, SocketStreamHandleClient* client)
    : SocketStreamHandle(url, client)
{
    if (!m_url.hasPath())
        m_url.setPath("/");
    const String& str = m_url.string();
    CString cstr = str.utf8();
    std::string urlStr(cstr.data());
    GURL gurl(urlStr);
    LOGSOCKETS("SocketStreamHandleAndroid::SocketStreamHandleAndroid(%s)", cstr.data());

    m_socketStream = new android::WebSocketStreamHandle(gurl);
    m_socketStream->setSocketStreamHandle(this);
    m_socketStream->connect();
}

SocketStreamHandleAndroid::~SocketStreamHandleAndroid()
{
    LOGSOCKETS("SocketStreamHandleAndroid::~SocketStreamHandleAndroid()");
    m_socketStream->setSocketStreamHandle(0);
}

int SocketStreamHandleAndroid::platformSend(const char* data, int length)
{
    LOGSOCKETS("SocketStreamHandleAndroid::platformSend(%d)", length);
    return m_socketStream->send(data, length);;
}

void SocketStreamHandleAndroid::platformClose()
{
    LOGSOCKETS("SocketStreamHandleAndroid::platformClose()");
    m_socketStream->close();
}

void SocketStreamHandleAndroid::onConnected()
{
    LOGSOCKETS("SocketStreamHandleAndroid::onConnected()");
    m_state = Open;
    m_client->didOpenSocketStream(this);
}

void SocketStreamHandleAndroid::onClosed()
{
    LOGSOCKETS("SocketStreamHandleAndroid::onClosed()");
    m_client->didCloseSocketStream(this);
}

void SocketStreamHandleAndroid::onSentData(int length)
{
    LOGSOCKETS("SocketStreamHandleAndroid::onSentData(%d)", length);
    if (!m_buffer.isEmpty()) {
        sendPendingData();
    }
}

void SocketStreamHandleAndroid::onReceivedData(const char* data, int length)
{
    LOGSOCKETS("SocketStreamHandleAndroid::onReceivedData(%d)", length);
    m_client->didReceiveSocketStreamData(this, data, length);
}

void SocketStreamHandleAndroid::onError(int error)
{
    LOGSOCKETS("SocketStreamHandleAndroid::onError(%d)", error);
    m_client->didFailSocketStream(this, SocketStreamError(error));
}

void SocketStreamHandle::didReceiveAuthenticationChallenge(const AuthenticationChallenge&)
{
    notImplemented();
}

void SocketStreamHandle::receivedCredential(const AuthenticationChallenge&, const Credential&)
{
    notImplemented();
}

void SocketStreamHandle::receivedRequestToContinueWithoutCredential(const AuthenticationChallenge&)
{
    notImplemented();
}

void SocketStreamHandle::receivedCancellation(const AuthenticationChallenge&)
{
    notImplemented();
}
}
#endif // ENABLE(WEB_SOCKETS)
