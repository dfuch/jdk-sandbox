/*
 * Copyright (c) 2001, 2020, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stddef.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <limits.h>

#include "jni.h"
#include "java_props.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"
#include "sun_nio_ch_Net.h"
#include "net_util.h"
#include "net_util_md.h"
#include "nio_util.h"
#include "nio.h"

#ifdef _AIX
#include <stdlib.h>
#include <sys/utsname.h>
#endif


/* Subtle platform differences in how unnamed sockets (empty path)
 * are returned from getsockname()
 */
#ifdef MACOSX
  #define ZERO_PATHLEN(len) (JNI_FALSE)
#else
  #define ZERO_PATHLEN(len) (len == offsetof(struct sockaddr_un, sun_path))
#endif

JNIEXPORT jstring JNICALL
NET_SockaddrToUnixAddressString(JNIEnv *env, struct sockaddr_un *sa, socklen_t len) {

    if (sa->sun_family == AF_UNIX) {
        char *name;

        if (ZERO_PATHLEN(len)) {
            name = "";
        } else {
            name = sa->sun_path;
        }
        return JNU_NewStringPlatform(env, name);
    }
    return NULL;
}

JNIEXPORT jint JNICALL
NET_UnixSocketAddressToSockaddr(JNIEnv *env, jbyteArray path, struct sockaddr_un *sa, int *len)
{
    memset(sa, 0, sizeof(struct sockaddr_un));
    sa->sun_family = AF_UNIX;
    int ret;
    const char* pname = (const char *)(*env)->GetByteArrayElements(env, path, NULL);
    size_t name_len = (*env)->GetArrayLength(env, path);
    if (name_len > MAX_UNIX_DOMAIN_PATH_LEN) {
        JNU_ThrowByName(env, JNU_JAVANETPKG "SocketException", "Unix domain path too long");
        ret = 1;
        goto finish;
    }
    memcpy(sa->sun_path, pname, name_len);
    *len = (int)(offsetof(struct sockaddr_un, sun_path) + name_len + 1);
    ret = 0;
  finish:
    (*env)->ReleaseByteArrayElements(env, path, (jbyte *)pname, 0);
    return ret;
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_UnixDomainNet_maxNameLen0(JNIEnv *env, jclass cl)
{
    return MAX_UNIX_DOMAIN_PATH_LEN;
}

JNIEXPORT jboolean JNICALL
Java_sun_nio_ch_UnixDomainNet_socketSupported(JNIEnv *env, jclass cl)
{
    return JNI_TRUE;
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_UnixDomainNet_socket0(JNIEnv *env, jclass cl)
{
    int fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        return handleSocketError(env, errno);
    }
    return fd;
}

JNIEXPORT void JNICALL
Java_sun_nio_ch_UnixDomainNet_bind0(JNIEnv *env, jclass clazz, jobject fdo, jbyteArray path)
{
    struct sockaddr_un sa;
    int sa_len = 0;
    int rv = 0;

    if (path == NULL)
        return; /* Rely on implicit bind: Unix */

    if (NET_UnixSocketAddressToSockaddr(env, path, &sa, &sa_len) != 0)
        return;

    int fd = fdval(env, fdo);

    rv = bind(fdval(env, fdo), (struct sockaddr *)&sa, sa_len);
    if (rv != 0) {
        handleSocketError(env, errno);
    }
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_UnixDomainNet_connect0(JNIEnv *env, jclass clazz, jobject fdo, jbyteArray path)
{
    struct sockaddr_un sa;
    int sa_len = 0;
    int rv;

    if (NET_UnixSocketAddressToSockaddr(env, path, &sa, &sa_len) != 0) {
        return IOS_THROWN;
    }

    rv = connect(fdval(env, fdo), (struct sockaddr *)&sa, sa_len);
    if (rv != 0) {
        if (errno == EINPROGRESS) {
            return IOS_UNAVAILABLE;
        } else if (errno == EINTR) {
            return IOS_INTERRUPTED;
        }
        return handleSocketError(env, errno);
    }
    return 1;
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_UnixDomainNet_accept(JNIEnv *env, jclass clazz, jobject fdo, jobject newfdo,
                           jobjectArray usaa)
{
    jint fd = fdval(env, fdo);
    jint newfd;
    struct sockaddr_un sa;
    socklen_t sa_len = sizeof(struct sockaddr_un);
    jstring usa;

    /* accept connection but ignore ECONNABORTED */
    for (;;) {
        newfd = accept(fd, (struct sockaddr *)&sa, &sa_len);
        if (newfd >= 0) {
            break;
        }
        if (errno != ECONNABORTED) {
            break;
        }
        /* ECONNABORTED => restart accept */
    }

    if (newfd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return IOS_UNAVAILABLE;
        if (errno == EINTR)
            return IOS_INTERRUPTED;
        JNU_ThrowIOExceptionWithLastError(env, "Accept failed");
        return IOS_THROWN;
    }

    setfdval(env, newfdo, newfd);

    usa = NET_SockaddrToUnixAddressString(env, &sa, sa_len);
    CHECK_NULL_RETURN(usa, IOS_THROWN);

    (*env)->SetObjectArrayElement(env, usaa, 0, usa);

    return 1;
}

JNIEXPORT jstring JNICALL
Java_sun_nio_ch_UnixDomainNet_localAddress0(JNIEnv *env, jclass clazz, jobject fdo)
{
    SOCKETADDRESS sa;
    socklen_t sa_len = sizeof(SOCKETADDRESS);
    int port;
    if (getsockname(fdval(env, fdo), &sa.sa, &sa_len) < 0) {
        handleSocketError(env, errno);
        return NULL;
    }
    return NET_SockaddrToUnixAddressString(env, &sa.saun, sa_len);
}

JNIEXPORT jstring JNICALL
Java_sun_nio_ch_UnixDomainNet_remoteAddress0(JNIEnv *env, jclass clazz, jobject fdo)
{
    SOCKETADDRESS sa;
    socklen_t sa_len = sizeof(sa);

    if (getpeername(fdval(env, fdo), &sa.sa, &sa_len) < 0) {
        handleSocketError(env, errno);
        return NULL;
    }
    return NET_SockaddrToUnixAddressString(env, &sa.saun, sa_len);
}
