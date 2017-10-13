// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

// Since winsock implements a posix style frontend, this isn't actually too 
// difficult.  The real troubles are data types and return constants.  Both of
// these we can generally handle with some smart adaptors.
#ifdef WIN32 
#include <Winsock2.h>
#define WOULD_HAVE_OVERRUN(a) ( \
    ((a) == WSAEMSGSIZE) ) /* The message was too large to fit into the specified buffer and was truncated. */
#define IS_FINISHED_FOR_NOW(a) ( \
    ((a) == WSAEWOULDBLOCK) ) /* The socket is marked as nonblocking and the receive operation would block. */
#define SEND_BUFFER_FULL(a) ( \
    ((a) == WSAENOBUFS) ) /* No buffer space is available. */ 
#define IS_FATAL_ERROR(a) ( \
    ((a) == WSANOTINITIALISED) || /* A successful WSAStartup call must occur before using this function. */ \
    ((a) == WSAEINVAL) || /* The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative. */ \
    ((a) == WSAEFAULT) || /* The buf parameter is not completely contained in a valid part of the user address space. */ \
    ((a) == WSAENOTSOCK) || /* The descriptor is not a socket. */ \
    ((a) == WSAEACCES) ) /* The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST parameter to allow the use of the broadcast address. */
#define IS_DISCONNECT_ERROR(a) ( \
    ((a) == WSAENETDOWN) || /* The network subsystem has failed. */ \
    ((a) == WSAENOTCONN) || /* The socket is not connected. */ \
    ((a) == WSAENETRESET) || /* The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress. */ \
    ((a) == WSAESHUTDOWN) || /* The socket has been shut down; it is not possible to receive on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH. */ \
    ((a) == WSAECONNABORTED) || /* The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable. */ \
    ((a) == WSAETIMEDOUT) || /* The connection has been dropped because of a network failure or because the peer system failed to respond. */ \
    ((a) == WSAECONNRESET) || /* The virtual circuit was reset by the remote side executing a hard or abortive close. The application should close the socket as it is no longer usable. On a UPD-datagram socket this error would indicate that a previous send operation resulted in an ICMP "Port Unreachable" message. */ \
    ((a) == WSAEHOSTUNREACH) ) /* The remote host cannot be reached from this host at this time. */
#define IS_MEANINGLESS_WARNING(a) ( \
    ((a) == WSAEINTR) || /* The (blocking) call was canceled through WSACancelBlockingCall. */ \
    ((a) == WSAEINPROGRESS) || /* A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. */ \
    ((a) == WSAEOPNOTSUPP) /* MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations. */
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
// special windows socket values
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
// special windows data types
#define WSADATA int
#define SOCKET int
// windows has no way to get a string error, so we use errno throughout
#define WSAGetLastError() errno
// windows uses closesocket instead of close
#define closesocket(a) close((a))

// linux version of the error selectors
#define WOULD_HAVE_OVERRUN(a) ( \
    ((a) == EMSGSIZE) ) /* The message was too large to fit into the specified buffer and was truncated. */
#define IS_FINISHED_FOR_NOW(a) ( \
    ((a) == EAGAIN) ) /* The socket is marked as nonblocking and the receive operation would block. */
#define SEND_BUFFER_FULL(a) ( \
    ((a) == ENOBUFS) ) /* No buffer space is available. */ 
#define IS_FATAL_ERROR(a) ( \
    ((a) == ENOMEM) || /* Out of memory */ \
    ((a) == EINVAL) || /* Invalid parameter was passed */ \
    ((a) == EFAULT) || /* The buf parameter is not completely contained in a valid part of the user address space. */ \
    ((a) == EBADF) || /* The descriptor is not a file descriptor. */ \
    ((a) == ENOTSOCK) || /* The descriptor is not a socket. */ \
    ((a) == EDESTADDRREQ) || /* The socket is not connection-mode, and no peer address is set. */ \
    ((a) == EISCONN) || /* Recipient specified on connected socket */ \
    ((a) == EOPNOTSUPP) || /* Flag is inappropriate for socket type */ \
    ((a) == EPIPE) || /* Local connection has terminated */ \
    ((a) == EACCES) ) /* The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST parameter to allow the use of the broadcast address. */
#define IS_DISCONNECT_ERROR(a) ( \
    ((a) == ECONNREFUSED) || /* The network subsystem has failed. */ \
    ((a) == ECONNRESET) || /* connection reset by peer */ \
    ((a) == ENOTCONN) ) /* The socket is not connected. */ 
#define IS_MEANINGLESS_WARNING(a) ( \
    ((a) == EINTR) ) /* The (blocking) call was canceled through WSACancelBlockingCall. */ 

#endif
