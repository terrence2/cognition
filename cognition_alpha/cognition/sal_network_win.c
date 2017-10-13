// Cognition
// sv_network.c
// Created 1-27-03 by Terrence Cole

// Includes
/////////////
#include "cog_global.h"
#include <Winsock2.h>

// Definitions
////////////////
#define MAX_CONNECTIONS 128
#define TCP_MAX_FRAME_SIZE 1460
#define FRAME_SIZE 4 * TCP_MAX_FRAME_SIZE // 2 times the max data size for a tcp packet

// Global Prototypes
//////////////////////
/*
int net_Initialize(void);
void net_Terminate(void);
			
byte net_cl_Connect( char *address, char *port );
void net_cl_Disconnect();
int net_cl_GetServerPacket( byte *type, unsigned short int *size, byte **data );
byte net_cl_SendData( byte *data, unsigned short int size );
byte net_cl_Flush();

byte net_cl_SendByte( byte b );
byte net_cl_SendShort( unsigned short int us );
byte net_cl_SendLong( unsigned long int ul );
byte net_cl_SendString( char *str );

byte net_sv_OpenNetwork( char *port );
byte net_sv_CloseNetwork();
connection_t net_sv_Accept( char **address );
void net_sv_Disconnect( connection_t client );
int net_sv_GetClientPacket( connection_t client, byte *type, unsigned short int *size, byte **data );
byte net_sv_SendData( connection_t client, byte *data, unsigned short int size );
byte net_sv_Flush( connection_t client );

byte net_sv_SendByte( connection_t client, byte b );
byte net_sv_SendShort( connection_t client, unsigned short int us );
byte net_sv_SendLong( connection_t client, unsigned long int ul );
byte net_sv_SendString( connection_t client, char *str );

unsigned short int net_GetShort( byte *buf );
unsigned long int net_GetLong( byte *buf );
unsigned long int net_GetString( char *outBuf, byte *buf, int maxLen );
*/

// Local Prototypes
//////////////////
static byte net_pk_FrameHasAPacket( byte *frame, unsigned short int start, unsigned short int length, unsigned short int *size );

// Local Variables
////////////////////
// the connected client socket
static SOCKET clSocket = INVALID_SOCKET;  
static byte clSendFrame[FRAME_SIZE];
static byte clRecvFrame[FRAME_SIZE];
static unsigned short int clSdFrSt = 0;
static unsigned short int clRcFrSt = 0;
static unsigned short int clSdFrLen = 0;
static unsigned short int clRcFrLen = 0;
static byte clPacketData[TCP_MAX_FRAME_SIZE];

// the server listener socket
static SOCKET svSocket = INVALID_SOCKET; 

// the server side client sockets
static SOCKET svConnections[MAX_CONNECTIONS];
static struct sockaddr_in svAddresses[MAX_CONNECTIONS];
static int svNumConnections = 0;
static byte svSendFrames[MAX_CONNECTIONS][FRAME_SIZE]; // 750k total frame data, not a horrible cost
static byte svRecvFrames[MAX_CONNECTIONS][FRAME_SIZE];
static unsigned short int svSdFrSt[MAX_CONNECTIONS];
static unsigned short int svRcFrSt[MAX_CONNECTIONS];
static unsigned short int svSdFrLen[MAX_CONNECTIONS];
static unsigned short int svRcFrLen[MAX_CONNECTIONS];
static byte svPacketData[TCP_MAX_FRAME_SIZE];

// *********** FUNCTIONALITY ***********
/* ------------
net_Initialize
------------ */
int net_Initialize(void)
{
	WSADATA wsaData;
	int ret, a;

	con_Print( "\n<BLUE>Initializing</BLUE> network and WSA sockets." );
	eng_LoadingFrame();

	// call the special windows startup stuff
	ret = WSAStartup( MAKEWORD(2, 0), &wsaData);
	if( ret != 0 ) 
	{
		con_Print( "<RED>Network Init Failed.  WSAStartup failed.  Error:  %d</RED>", WSAGetLastError() );
		return 0;
	}

	// clear the server listener
	svSocket = INVALID_SOCKET;

	// clear the server connections
	for( a = 0 ; a < MAX_CONNECTIONS ; a++ )
	{
		svConnections[a] = INVALID_SOCKET;
		memset( &svAddresses[a], 0, sizeof(struct sockaddr) );
		memset( svSendFrames[a], 0, FRAME_SIZE );
		memset( svRecvFrames[a], 0, FRAME_SIZE );
		svSdFrSt[a] = 0;
		svRcFrSt[a] = 0;
		svSdFrLen[a] = 0;
		svRcFrLen[a] = 0;
	}
	svNumConnections = 0;
	memset( svPacketData, 0, TCP_MAX_FRAME_SIZE );

	// clear the client connection
	clSocket = INVALID_SOCKET;
	memset( clSendFrame, 0, FRAME_SIZE );
	memset( clRecvFrame, 0, FRAME_SIZE );
	memset( clPacketData, 0, TCP_MAX_FRAME_SIZE );
	clSdFrSt = 0;
	clRcFrSt = 0;
	clSdFrLen = 0;
	clRcFrLen = 0;

	con_Print( "\tDone." );
	eng_LoadingFrame();

	return 1;
}

/* ------------
net_Terminate
------------ */
void net_Terminate(void)
{
	int a;

	con_Print( "\n<RED>Terminating</RED> network system and WSA sockets." );
	
	// if we have a server active we'll need to close its connections
	if( clSocket != INVALID_SOCKET ) net_cl_Disconnect();
	if( svSocket != INVALID_SOCKET ) net_sv_CloseNetwork();

	// clear the server listener
	svSocket = INVALID_SOCKET;

	// clear the server connections
	for( a = 0 ; a < MAX_CONNECTIONS ; a++ )
	{
		svConnections[a] = INVALID_SOCKET;
		memset( &svAddresses[a], 0, sizeof(struct sockaddr) );
		memset( svSendFrames[a], 0, FRAME_SIZE );
		memset( svRecvFrames[a], 0, FRAME_SIZE );
		svSdFrSt[a] = 0;
		svRcFrSt[a] = 0;
		svSdFrLen[a] = 0;
		svRcFrLen[a] = 0;
	}
	svNumConnections = 0;
	memset( svPacketData, 0, TCP_MAX_FRAME_SIZE );

	// clear the client connection
	clSocket = INVALID_SOCKET;
	memset( clSendFrame, 0, FRAME_SIZE );
	memset( clRecvFrame, 0, FRAME_SIZE );
	memset( clPacketData, 0, TCP_MAX_FRAME_SIZE );
	clSdFrSt = 0;
	clRcFrSt = 0;
	clSdFrLen = 0;
	clRcFrLen = 0;
	
	// do windows cleanup
	WSACleanup();
}

/* ------------
net_cl_Connect
------------ */
byte net_cl_Connect( char *address, char *port )
{
	int ret;
	unsigned long on = 1;
	struct sockaddr_in destAddr;

	// assert
	if( address == NULL ) return 0;
	if( port == NULL ) return 0;
	if( tcstrlen(address) <= 0 ) return 0;
	if( tcstrlen(port) <= 0 ) return 0;
	if( clSocket != INVALID_SOCKET )
	{
		con_Print( "Network Client Connect Failed:  Client already connected." );
		return 0;
	}

	// construct a destination
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons( (unsigned short)tcatoi(port) );
	if( (destAddr.sin_addr.s_addr = inet_addr( address )) < 0 )
	{
		con_Print( "<RED>Connect Failed:  Unable to convert address to network byte-ordered long.</RED>" );
		return 0;
	}
	memset( destAddr.sin_zero, 0, 8 );

	// get the socket
	clSocket = socket( AF_INET, SOCK_STREAM, 0 );
	if( clSocket == INVALID_SOCKET )
	{
		con_Print( "<RED>Connect Failed:  Unable to create a socket for the connection.</RED>" );
		return 0;
	}

	con_Print( "\n<BLUE>Connecting</BLUE> to <ORANGE>\"%s:%s\"</ORANGE>...", address, port );
	
	// connect
	ret = connect( clSocket, (struct sockaddr *)&destAddr, sizeof(struct sockaddr) );
	if( ret == SOCKET_ERROR )
	{
		con_Print( "<RED>Connect Failed:  Unable to create a socket connection.  Error: %d</RED>", WSAGetLastError() );
		shutdown( clSocket, 2 );
		clSocket = INVALID_SOCKET;
		return 0;
	}

	con_Print( "\t<BLUE>Connected.</BLUE>" );

	// make this socket non-blocking
	ioctlsocket( clSocket, FIONBIO, &on );

	return 1;
}

/* ------------
net_cl_Disconnect
------------ */
void net_cl_Disconnect()
{
	// assert
	if( clSocket == INVALID_SOCKET )
	{
		con_Print( "Network System Assert Failed:  net_cl_Disconnect:  clSocket is INVALID_SOCKET." );
		return;
	}
	
	// tell the user about it
	con_Print( "<RED>Disconnecting</RED> Client." );

	// do the shutdown
	if( shutdown( clSocket, 2 ) == SOCKET_ERROR )
	{
		con_Print( "Network Client Error:  Shutdown Failed.  Error:  %d", WSAGetLastError() );
	}
	if( closesocket( clSocket ) == SOCKET_ERROR )
	{
		con_Print( "Network Client Error:  closesocket failed.  Error:  %d", WSAGetLastError() );
		return;
	}	
	clSocket = INVALID_SOCKET;

	// clear the buffers
	memset( clSendFrame, 0, FRAME_SIZE );
	memset( clRecvFrame, 0, FRAME_SIZE );
	memset( clPacketData, 0, TCP_MAX_FRAME_SIZE );
	clSdFrSt = 0;
	clRcFrSt = 0;
	clSdFrLen = 0;
	clRcFrLen = 0;
}

/* ------------
net_cl_GetServerPacket - attempts to read the next packet off of the client recieve frame, when empty calls recv
					- returns 0 if no more packets are available, returns -1 of the connection is closed
					- returns the number of bytes read from the buffer on success
------------ */
int net_cl_GetServerPacket( byte *type, unsigned short int *size, byte **data )
{
	int a;
	byte cnt;
	byte pkType;
	unsigned short int pkSize;
	unsigned short int index, length;

	// assert
	if( clSocket == INVALID_SOCKET )
	{
		con_Print( "<RED>Network Client Assert Failed:  clSocket is invalid socket." );
		return -1;
	}
	if( (clRcFrSt < 0) || (clRcFrSt >= FRAME_SIZE) )
	{
		con_Print( "<RED>Network Client Assert Failed:  Receive Frame start marker out of bounds:  %d", clRcFrSt );
		return -1;
	}
	if( (clRcFrLen < 0) || (clRcFrLen >= FRAME_SIZE) )
	{
		con_Print( "<RED>Network Client Assert Failed:  Receive Frame length marker out of bounds:  %d", clRcFrLen );
		return -1;
	}
	if( type == NULL ) return 0;
	if( size == NULL ) return 0;
	if( (data == NULL) ) return 0;

	// 2 cases: we have a whole packet in the buffer, ready to unpack and send
	// or we need to call recieve to get the next packet
	pkType = net_pk_FrameHasAPacket( clRecvFrame, clRcFrSt, clRcFrLen, &pkSize );

	// 1) we don't have a whole packet and need to recv
	cnt = 0;  // don't get stuck in a loop if the server dies badly
	while( !pkType && (cnt < 4) )
	{
		unsigned short int dataEnd;
		int iDataRecvd;

		// strategy:  pick a position to read to, do the recv, see if we have a full packet, repeat as needed

		// A) pick a read position and span
		// check to see if the current packet extends around the end of the buffer	
		dataEnd = clRcFrSt + clRcFrLen;
		if( dataEnd >= FRAME_SIZE )
		{
			index = dataEnd - FRAME_SIZE;
			length = clRcFrSt - index;
		}
		else
		{
			index = dataEnd;
			length = FRAME_SIZE - dataEnd;
		}
		
		// B) perform the recv operation
		iDataRecvd = recv( clSocket, (char*)&clRecvFrame[index], length, 0 );
		// normal socket shutdown
		if( iDataRecvd == 0 )
		{
			con_Print( "<RED>Network Client Warning:  The server has closed the connection." );
			return -1;
		}
		// the myriad of potentially dangerous errors mixed with normal operation conditions
		else if( iDataRecvd == SOCKET_ERROR )
		{
			// an error occured that may mean many things, we need to check WSAGetLastError's return
			switch( WSAGetLastError() )
			{
			// fatal errors
			case WSANOTINITIALISED: // A successful WSAStartup call must occur before using this function. 
			case WSAEFAULT: // The buf parameter is not completely contained in a valid part of the user address space. 
			case WSAENOTSOCK: // The descriptor is not a socket. 
				con_Print( "<RED>Network System Error:  A fatal error has occured in the network system." );
				con_Print( "<RED>The program will now exit.  Please report this error and help squash this bug." );
				eng_Stop( "34687" );
				return -1;

			// disconnect errors
			case WSAENETDOWN: // The network subsystem has failed. 
			case WSAENOTCONN: // The socket is not connected. 
			case WSAENETRESET: // The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress. 
			case WSAESHUTDOWN: // The socket has been shut down; it is not possible to receive on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH. 
			case WSAEINVAL: // The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative. 
			case WSAECONNABORTED: // The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable. 
			case WSAETIMEDOUT: // The connection has been dropped because of a network failure or because the peer system failed to respond. 
			case WSAECONNRESET: // The virtual circuit was reset by the remote side executing a hard or abortive close. The application should close the socket as it is no longer usable. On a UPD-datagram socket this error would indicate that a previous send operation resulted in an ICMP "Port Unreachable" message. 
				con_Print( "<RED>Network System Warning:  The connection with the server has ended abnormally." );
				return -1;
			
			// the socket's recv que is empty
			case WSAEWOULDBLOCK: // The socket is marked as nonblocking and the receive operation would block. 
				// we can't get any more information out of the socket at this time
				return 0;
				
			// handle circular buffer issues
			case WSAEMSGSIZE: // The message was too large to fit into the specified buffer and was truncated. 
				// set the frame parameters to match this condition, then continue with the operation
				clRcFrLen += length; 
				break;

			// meaningless errors
			case WSAEINTR: // The (blocking) call was canceled through WSACancelBlockingCall. 
			case WSAEINPROGRESS: // A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
			case WSAEOPNOTSUPP: // MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations. 
			default:
				con_Print( "Network Warning:  Client receive failed with a meaningless error." );
				break;
			}
		}
		// the normal response
		else
		{
			clRcFrLen += iDataRecvd;
		}

		// catch this potentially deadly error
		if( clRcFrLen > FRAME_SIZE )
		{
			con_Print( "<RED>Network System Error:   A network communication error has occured." );
			con_Print( "<RED>Please report this bug." );
			return -1;
		}

		// see if we have enough data to procede yet
		pkType =  net_pk_FrameHasAPacket( clRecvFrame, clRcFrSt, clRcFrLen, &pkSize );
	}

	// the operation timed out this round
	if( (cnt == 4) && (!pkType) ) return 0;  // try again next frame

	// return a pointer to the packet position in a local static memory buffer
	// this memory will be valid until the next call to this function
	// we need to copy by byte so we don't overrun our buffer
	memset( clPacketData, 0, TCP_MAX_FRAME_SIZE );
	index = clRcFrSt + 3;
	for( a = 0 ; a < pkSize ; a++ )
	{
		if( index >= FRAME_SIZE ) index -= FRAME_SIZE;
		clPacketData[a] = clRecvFrame[index];
		clRecvFrame[index] = 0;
		index++;
	}
	
	// adjust the frame reference to our current state
	clRcFrLen -= (pkSize + 3);
	clRcFrSt = index;
	if( clRcFrSt >= FRAME_SIZE ) clRcFrSt -= FRAME_SIZE;

	// return our valid data
	*type = pkType;
	*size = pkSize;
	*data = clPacketData;
	return 1;
}

/* ------------
net_cl_SendData - push size bytes from data to the send buffer - returns >0 on success, 0 on fail
				- if we return 0 a critical error has occurred and the caller needs to shutdown
------------ */
byte net_cl_SendData( byte *data, unsigned short int size )
{
	unsigned short int start, length;

	// assert
	if( (clSdFrSt < 0) || (clSdFrSt >= FRAME_SIZE) )
	{
		con_Print( "<RED>Network Client Assert Failed:	Client Send Frame Start Marker out of bounds in Send Data:  %d", clSdFrSt );
		return 0;
	}
	if( (clSdFrLen < 0) || (clSdFrLen >= FRAME_SIZE) )
	{
		con_Print( "<RED>Network Client Assert Failed:	Client Send Frame Length Marker out of bounds in Send Data:  %d", clSdFrLen );
		return 0;
	}

	// we may need to send off some data to add more to the que
	if( (clSdFrLen + size) >= FRAME_SIZE )
	{
		net_cl_Flush();
	}

	// set a starting position
	start = clSdFrSt + clSdFrLen;
	if( start >= FRAME_SIZE )
	{
		start -= FRAME_SIZE;
	}

	// adjust the overall frame length because we might need to adjust the transient length
	clSdFrLen += size;

	// push as much data as possible to the buffer
	length = size;
	if( (start + length) > FRAME_SIZE )
	{
		// we can't do the whole thing in one go, so do a pre write then adjust pointers so the next write always works
		length = FRAME_SIZE - start - 1;
		size -= length;
		memcpy( &clSendFrame[start], data, length );
		start = 0;
		length = size;
	}
	memcpy( &clSendFrame[start], data, length );

	return 1;
}

/* ------------
net_cl_Flush - pushes all data in the send buffer to the kernel tcp stack - returns >0 on success, 0 if failed, -1 on disconnect
------------ */
byte net_cl_Flush()
{
	int iThisSend;
	int iAmtToSend;
	byte cnt = 0;
	
	// assert
	if( (clSdFrSt < 0) || (clSdFrSt >= FRAME_SIZE) )
	{
		con_Print( "<RED>Network Client Assert Failed:  Client Send Frame Start Marker out of bounds in Flush:  %d", clSdFrSt );
		return 0;
	}
	if( (clSdFrLen < 0) || (clSdFrLen >= FRAME_SIZE) )
	{
		con_Print( "<RED>Network Client Assert Failed:  Client Send Frame Length Marker out of bounds in Flush:  %d", clSdFrLen );
		return 0;
	}
	
	// see if we have any work to do
	if( clSdFrLen <= 0 ) return 1;

	while( (clSdFrLen > 0) && (cnt < 4) )
	{
		// make sure we don't try to send something that's too big or not in our buffers
		iAmtToSend = clSdFrLen;
		if( iAmtToSend > TCP_MAX_FRAME_SIZE )
		{
			iAmtToSend = TCP_MAX_FRAME_SIZE - 1;
		}
		if( (clSdFrSt + iAmtToSend) >= FRAME_SIZE )
		{
			iAmtToSend = FRAME_SIZE - clSdFrSt;
		}

		// do the send
		iThisSend = send( clSocket, &clSendFrame[clSdFrSt], iAmtToSend, 0 );

		// always check the send return for errors
		if( iThisSend == SOCKET_ERROR )
		{
			switch( WSAGetLastError() )
			{
			// fatal errors
			case WSANOTINITIALISED: // A successful WSAStartup call must occur before using this function. 
			case WSAEACCES: // The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST parameter to allow the use of the broadcast address. 
			case WSAEFAULT: // The buf parameter is not completely contained in a valid part of the user address space. 
			case WSAENOTSOCK: // The descriptor is not a socket. 
			case WSAEMSGSIZE: // The socket is message oriented, and the message is larger than the maximum supported by the underlying transport. 
			case WSAEINVAL: // The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled. 
				con_Print( "<RED>Network System Error:	A fatal error has occured in the network system." );
				con_Print( "<RED>The program will now exit.  Please report this error and help squash this bug." );
				eng_Stop( "34698" );
				return -1;

			// disconnect errors
			case WSAENETDOWN: // The network subsystem has failed. 
			case WSAENETRESET: // The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress. 
			case WSAENOTCONN: // The socket is not connected. 
			case WSAESHUTDOWN: // The socket has been shut down; it is not possible to send on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH. 
			case WSAEHOSTUNREACH: // The remote host cannot be reached from this host at this time. 
			case WSAECONNABORTED: // The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable. 
			case WSAECONNRESET: // The virtual circuit was reset by the remote side executing a hard or abortive close. For UPD sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a "Port Unreachable" ICMP packet. The application should close the socket as it is no longer usable. 
			case WSAETIMEDOUT: // The connection has been dropped, because of a network failure or because the system on the other end went down without notice. 
				con_Print( "<RED>Network System Warning:  The connection with the server has ended abnormally." );
				con_Print( "<RED>The game will now end." );
				return -1;
				
			// we have flooded the kernel tcp stack; wait till the next frame to try sending
			case WSAENOBUFS: // No buffer space is available. 
			case WSAEWOULDBLOCK: // The socket is marked as nonblocking and the requested operation would block. 
				return 1;

			// useless errors
			case WSAEINTR: // A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall. 
			case WSAEINPROGRESS: // A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
			case WSAEOPNOTSUPP: // MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations. 
			default:
				con_Print( "Network Warning:  Client send failed with a meaningless error." );
				break;
			}
		}
		
		// adjust the frame length
		clSdFrLen -= iThisSend;

		// adjust the send start point but remember to make it circular
		clSdFrSt += iThisSend;
		if( clSdFrSt >= FRAME_SIZE ) clSdFrSt -= FRAME_SIZE;
	}

	// if we got stuck somewhere without generating an error condition
	if( cnt == 4 ) return 0;

	// reset the start and length for maximum packing on subsequent calls
	clSdFrSt = 0;
	clSdFrLen = 0;
	return 1;
}

/* ------------
net_cl_Send**** - calls send data with the adjusted network versions of the data
------------ */
byte net_cl_SendByte( byte b ) 
{ 
	return net_cl_SendData( &b, 1 ); 
}
byte net_cl_SendShort( unsigned short int us ) 
{
	unsigned short int usTmp = htons(us);
	return net_cl_SendData( (byte*)&usTmp, 2 ); 
}
byte net_cl_SendLong( unsigned long int ul ) 
{ 
	unsigned long int ulTmp = htonl(ul);
	return net_cl_SendData( (byte*)&ulTmp, 4 ); 
}
byte net_cl_SendString( char *str ) 
{ 
	return net_cl_SendData( str, (unsigned short int)tcstrlen(str) ); 
}


/* ------------
net_sv_OpenNetwork - inits the server network interface and prepares to accept connections on the given port
------------ */
byte net_sv_OpenNetwork( char *port )
{
	int ret;
	unsigned long on = 1;
	struct sockaddr_in thisAddr;

	//assert
	if( port == NULL ) return 0;
	if( tcstrlen(port) <= 0 ) return 0;
	if( svSocket != INVALID_SOCKET )
	{
		con_Print( "<RED>Net Server Error:  Open Network cannot continue, network already open." );
		return 0;
	}

	// set up the address
	thisAddr.sin_family = AF_INET;
	thisAddr.sin_port = htons( (unsigned short)tcatoi(port) );
	thisAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset( thisAddr.sin_zero, 0, 8 );

	// construct a socket
	svSocket = socket( AF_INET, SOCK_STREAM, 0 );
	if( svSocket == INVALID_SOCKET )
	{
		con_Print( "<RED>Open Network Failed:  Unable to construct a socket for listening.  Error:  %d", WSAGetLastError() );
		return 0;
	}

	// make this socket non-blocking
	ioctlsocket( svSocket, FIONBIO, &on );

	// bind ourselves to a port so that we can listen
	ret = bind( svSocket, (struct sockaddr*)&thisAddr, sizeof(struct sockaddr) );
	if( ret == SOCKET_ERROR ) 
	{
		con_Print( "<RED>Open Network Failed:  Unable to bind to port \"%s\".  Error:  %d", port, WSAGetLastError() );
		shutdown( svSocket, 2 );
		svSocket = INVALID_SOCKET;
		return 0;
	}

	ret = listen( svSocket, 10 );
	if( ret == SOCKET_ERROR )
	{
		con_Print( "<RED>Open Network Failed:  Unable to listen on new socket.  Error:  %d", WSAGetLastError() );
		shutdown( svSocket, 2 );
		svSocket = INVALID_SOCKET;
		return 0;
	}
	
	// turn off Nagel algorithm
//	setsockopt( TCP_NODELAY ); 
 
	return 1;
}

/* ------------
net_sv_CloseNetwork
------------ */
byte net_sv_CloseNetwork()
{
	int a;
	
	// disconnect all clients with a message
	for( a = 0 ; a < MAX_CONNECTIONS ; a++ )
	{
		if( svConnections[a] != -1 )
		{
			net_sv_Disconnect( a );
		}
	}

	// close down this server
	if( svSocket != INVALID_SOCKET )
	{
		if( closesocket( svSocket ) == SOCKET_ERROR )
		{
			con_Print( "<RED>Network Server:  Close Network Failed:  closesocket for server socket failed.  Error:  %d</RED>", WSAGetLastError()  );
			return 0;
		}
		svSocket = INVALID_SOCKET;
	}

	return 1;
}

/* ------------
net_sv_Accept - returns the index of the new socket or an invalid connection if none are pending
------------ */
connection_t net_sv_Accept( char **address )
{
	byte bDone;
	int index;
	int ret;
	SOCKET tSock;
	struct sockaddr_in tAddr;
	unsigned long on = 1;
	int size = sizeof(struct sockaddr);

	// assert
	if( svSocket == INVALID_SOCKET )
	{
		con_Print( "<RED>Network Server Assert Failed:  Accept:  Attempted accept on an invalid server socket." );
		return INVALID_CONNECTION;
	}
	
	// this is the main decision code in a per-frame function, so get it out of the way and cull the simple cases
	tSock = accept( svSocket, (struct sockaddr*)&tAddr, &size );
	if( tSock == INVALID_SOCKET ) 
	{
		// no connections pending
		return INVALID_CONNECTION;
	}

	// if we got here we need a new index
	index = 0;
	bDone = 0;
	while( (index < MAX_CONNECTIONS) && !bDone )
	{
		if( svConnections[index] == INVALID_SOCKET )
		{
			bDone = 1;
		}
		else
		{
			index++;
		}
	}

	// check for an error
	if( bDone == 0 ) // we ran out of open connections so turn down this connection
	{
		// print the error
		con_Print( "<VIOLET>Network Service Condition:  All available connection slots are full." );
		shutdown( tSock, 2 );
	}

	// copy over the connection to our new index
	svConnections[index] = tSock;
	memcpy( &svAddresses[index], &tAddr, size );

	// make this new connection non blocking
	ret = ioctlsocket( svConnections[index], FIONBIO, &on );
	if( ret )
	{
		con_Print( "<RED>Network Server Error:  unable to set new client as non-blocking.  Error:  %d",  WSAGetLastError() );
		shutdown( svConnections[index], 2 );
		svConnections[index] = INVALID_SOCKET;
		return 0;
	}

	// get the address for the server's perusal
	*address = inet_ntoa( svAddresses[index].sin_addr );

	con_Print( "Network Server:  Accepted Client on socket %d.", tSock );
	return index;
}

/* ------------
net_sv_Disconnect
------------ */
void net_sv_Disconnect( connection_t client )
{
	// assert
	if( (client < 0) || (client >= MAX_CONNECTIONS) )
	{
		con_Print( "<RED>Network Server Assert Failed:  Disconnect:  client out of bounds.  client:  %d", client );
		return;
	}
	if( svConnections[client] == INVALID_SOCKET )
	{
		con_Print( "<RED>Network Server Assert Failed:	Disconnect:  client socket not a valid socket.  client: %d", client );
		return;
	}
	
	// sending a closing message to the client is NOT the net systems responsibility
	// the client should have been previously informed of the closing status of the socket
	
	//do the shutdown
	if( shutdown( svConnections[client], 2 ) )
	{
		con_Print( "<RED>Network Server Disconnect Failed for client %d:  shutdown Failed.  Please Report this error.", client );
	}
	if( closesocket( svConnections[client] ) )
	{
		con_Print( "<RED>Network Server Disconnect Failed for client %d:  closesocket Failed.  Please Report this error.", client );
		return;
	}
	svConnections[client] = INVALID_SOCKET;
	svNumConnections--;

	// clear the frame memory
	memset( &svAddresses[client], 0, sizeof(struct sockaddr) );
	memset( svSendFrames[client], 0, FRAME_SIZE );
	memset( svRecvFrames[client], 0, FRAME_SIZE );
	svSdFrSt[client] = 0;
	svRcFrSt[client] = 0;
	svSdFrLen[client] = 0;
	svRcFrLen[client] = 0;
}

/* ------------
net_sv_GetClientPacket - attempts to read the next packet off of the client recieve frame, when empty calls recv
					- returns 0 if no more packets are available, returns -1 of the connection is closed
					- returns the number of bytes read from the buffer on success
------------ */
int net_sv_GetClientPacket( connection_t client, byte *type, unsigned short int *size, byte **data )
{
	int a;
	byte cnt;
	byte pkType;
	unsigned short int pkSize;
	unsigned short int index, length;

	// assert
	if( (client < 0) || (client >= MAX_CONNECTIONS) )
	{
		con_Print( "<RED>Network Server Assert Failed:  GetClientPacket:  client out of bounds.  client:  %d", client );
		return -1;
	}
	if( svConnections[client] == INVALID_SOCKET )
	{
		con_Print( "<RED>Network Server Assert Failed:  GetClientPacket:  Attempted use of an invalid socket.  client: %d", client );
		return -1;
	}
	if( (svRcFrSt[client] < 0) || (svRcFrSt[client] >= FRAME_SIZE) )
	{
		con_Print( "<RED>Network Server Assert Failed:  GetClientPacket:  Server Receive Frame Start out of bounds for client %d.", client );
		return -1;
	}
	if( (svRcFrLen[client] < 0) || (svRcFrLen[client] >= FRAME_SIZE) )
	{
		con_Print( "<RED>Network Server Assert Failed:	GetClientPacket:  Server Receive Frame Length out of bounds for client %d.", client );
		return -1;
	}
	if( type == NULL ) return 0;
	if( size == NULL ) return 0;
	if( (data == NULL) ) return 0;

	// 2 cases: we have a whole packet in the buffer, ready to unpack and send
	// or we need to call recieve to get the next packet
	pkType = net_pk_FrameHasAPacket( svRecvFrames[client], svRcFrSt[client], svRcFrLen[client], &pkSize );

	// 1) we don't have a whole packet and need to recv
	cnt = 0;  // don't get stuck in a loop if the server dies badly
	while( !pkType && (cnt < 4) )
	{
		unsigned short int dataEnd;
		int iDataRecvd;

		// strategy:  pick a position to read to, do the recv, see if we have a full packet, repeat as needed

		// A) pick a read position and span
		// check to see if the current packet extends around the end of the buffer	
		dataEnd = svRcFrSt[client] + svRcFrLen[client];
		if( dataEnd >= FRAME_SIZE )
		{
			index = dataEnd - FRAME_SIZE;
			length = svRcFrSt[client] - index;
		}
		else
		{
			index = dataEnd;
			length = FRAME_SIZE - dataEnd;
		}
		
		// B) perform the recv operation
		iDataRecvd = recv( svConnections[client], &svRecvFrames[client][index], length, 0 );
		// normal socket shutdown
		if( iDataRecvd == 0 )
		{
			con_Print( "<RED>Network System Warning:  The client has closed the connection." );
			return -1;
		}
		// the myriad of potentially dangerous errors mixed with normal operation conditions
		else if( iDataRecvd == SOCKET_ERROR )
		{
			// an error occured that may mean many things, we need to check WSAGetLastError's return
			switch( WSAGetLastError() )
			{

			// fatal errors
			case WSANOTINITIALISED: // A successful WSAStartup call must occur before using this function. 
			case WSAEFAULT: // The buf parameter is not completely contained in a valid part of the user address space. 
			case WSAENOTSOCK: // The descriptor is not a socket. 
				con_Print( "<RED>Network System Warning:  A fatal error has occured on the client socket at connection %d.", client );
				return -1;

			// disconnect errors
			case WSAENETDOWN: // The network subsystem has failed. 
			case WSAENOTCONN: // The socket is not connected. 
			case WSAENETRESET: // The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress. 
			case WSAESHUTDOWN: // The socket has been shut down; it is not possible to receive on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH. 
			case WSAEINVAL: // The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative. 
			case WSAECONNABORTED: // The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable. 
			case WSAETIMEDOUT: // The connection has been dropped because of a network failure or because the peer system failed to respond. 
			case WSAECONNRESET: // The virtual circuit was reset by the remote side executing a hard or abortive close. The application should close the socket as it is no longer usable. On a UPD-datagram socket this error would indicate that a previous send operation resulted in an ICMP "Port Unreachable" message. 
				con_Print( "<RED>Network System Warning:  The connection with the client has ended abnormally." );
				return -1;
			
			// the socket's recv que is empty
			case WSAEWOULDBLOCK: // The socket is marked as nonblocking and the receive operation would block. 
				// we can't get any more information out of the socket at this time
				return 0;
				
			// handle circular buffer issues
			case WSAEMSGSIZE: // The message was too large to fit into the specified buffer and was truncated. 
				// set the frame parameters to match this condition, then continue with the operation
				svRcFrLen[client] += length; 
				break;

			// meaningless errors
			case WSAEINTR: // The (blocking) call was canceled through WSACancelBlockingCall. 
			case WSAEINPROGRESS: // A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
			case WSAEOPNOTSUPP: // MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations. 
			default:
				con_Print( "Network Warning:  Server receive for client connection %d failed with a meaningless error.", client );
				break;
			}
		}
		// the normal response
		else
		{
			svRcFrLen[client] += iDataRecvd;
		}

		// catch this potentially deadly error
		if( svRcFrLen[client] >= FRAME_SIZE )
		{
			con_Print( "<RED>Network System Error:   A network communication error has occured." );
			con_Print( "<RED>Please report this bug." );
			return -1;
		}

		// see if we have enough data to procede yet
		pkType =  net_pk_FrameHasAPacket( svRecvFrames[client], svRcFrSt[client], svRcFrLen[client], &pkSize );
		
	}

	// the operation timed out this round
	if( (cnt == 4) && (!pkType) ) return 0;  // try again next frame

	// return a pointer to the packet position in a local static memory buffer
	// this memory will be valid until the next call to this function
	// we need to copy by byte so we don't overrun our buffer
	memset( svPacketData, 0, TCP_MAX_FRAME_SIZE );
	index = svRcFrSt[client] + 3;
	for( a = 0 ; a < pkSize ; a++ )
	{
		if( index >= FRAME_SIZE ) index -= FRAME_SIZE;
		svPacketData[a] = svRecvFrames[client][index];
		svRecvFrames[client][index] = 0;
		index++;
	}
	
	// adjust the frame reference to our current state
	svRcFrLen[client] -= (pkSize + 3);
	svRcFrSt[client] = index;
	if( svRcFrSt[client] >= FRAME_SIZE ) svRcFrSt[client] -= FRAME_SIZE;
	
	// return our valid data
	*type = pkType;
	*size = pkSize;
	*data = svPacketData;
	return 1;
}


/* ------------
net_sv_SendData - push size bytes from data to the send buffer - returns >0 on success, 0 on fail
				- if we return 0 a critical error has occurred and the caller needs to shutdown
------------ */
byte net_sv_SendData( connection_t client, byte *data, unsigned short int size )
{
	unsigned short int start, length;

	// assert
	if( (client < 0) || (client >= MAX_CONNECTIONS) )
	{
		con_Print( "<RED>Network Server Assert Failed:	SendData:  client out of bounds.  client:  %d", client );
		return 0;
	}
	if( (svSdFrSt[client] < 0) || (svSdFrSt[client] >= FRAME_SIZE) )
	{
		con_Print( "<RED>Network Server Assert Failed:	SendData:  Server Send Frame Start out of bounds for client %d.", client );
		return 0;
	}
	if( (svSdFrLen[client] < 0) || (svSdFrLen[client] >= FRAME_SIZE) )
	{
		con_Print( "<RED>Network Server Assert Failed:	SendData:  Server Send Frame Length out of bounds for client %d.  Length:  %d", client, svSdFrLen[client] );
		return 0;
	}

	// we may need to send off some data to add more to the que
	if( (svSdFrLen[client] + size) >= FRAME_SIZE )
	{
		net_sv_Flush(client);
	}

	// set a starting position
	start = svSdFrSt[client] + svSdFrLen[client];
	if( start >= FRAME_SIZE ) start -= FRAME_SIZE;

	// adjust the overall frame length because we might need to adjust the transient length
	svSdFrLen[client] += size;

	// push as much data as possible to the buffer
	length = size;
	if( (start + length) > FRAME_SIZE )
	{
		// we can't do the whole thing in one go, so do a pre write then adjust pointers so the next write always works
		length = FRAME_SIZE - start - 1;
		size -= length;
		memcpy( &svSendFrames[client][start], data, length );
		start = 0;
		length = size;
	}
	memcpy( &svSendFrames[client][start], data, length );

	return 1;
}

/* ------------
net_sv_Flush - pushes all data in the send buffer to the kernel tcp stack - returns >0 on success, 0 if failed, -1 on disconnect
------------ */
byte net_sv_Flush( connection_t client )
{
	int iThisSend;
	int iAmtToSend;
	byte cnt = 0;

	// assert
	if( (client < 0) || (client >= MAX_CONNECTIONS) )
	{
		con_Print( "<RED>Network Server Assert Failed:	Flush:  client out of bounds.  client:  %d", client );
		return 0;
	}
	if( (svSdFrSt[client] < 0) || (svSdFrSt[client] >= FRAME_SIZE) )
	{
		con_Print( "<RED>Network Server Assert Failed:	Flush:  Server Send Frame Start out of bounds for client %d.", client );
		return -1;
	}
	if( (svSdFrLen[client] < 0) || (svSdFrLen[client] >= FRAME_SIZE) )
	{
		con_Print( "<RED>Network Server Assert Failed:	Flush:  Server Send Frame Length out of bounds for client %d.", client );
		return -1;
	}

	// see if we have any work to do
	if( svSdFrLen[client] <= 0 ) return 1;

	while( (svSdFrLen[client] > 0) && (cnt < 4) )
	{
		// make sure we don't try to send something that's too big or not in our buffers
		iAmtToSend = svSdFrLen[client];
		if( iAmtToSend > TCP_MAX_FRAME_SIZE )
		{
			iAmtToSend = TCP_MAX_FRAME_SIZE - 1;
		}
		if( (svSdFrSt[client] + iAmtToSend) >= FRAME_SIZE )
		{
			iAmtToSend = FRAME_SIZE - svSdFrSt[client];
		}

		// do the send
		iThisSend = send( svConnections[client], &svSendFrames[client][svSdFrSt[client]], iAmtToSend, 0 );

		// always check the send return for errors
		if( iThisSend == SOCKET_ERROR )
		{
			switch( WSAGetLastError() )
			{
			// fatal errors
			case WSANOTINITIALISED: // A successful WSAStartup call must occur before using this function. 
			case WSAEACCES: // The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST parameter to allow the use of the broadcast address. 
			case WSAEFAULT: // The buf parameter is not completely contained in a valid part of the user address space. 
			case WSAENOTSOCK: // The descriptor is not a socket. 
			case WSAEMSGSIZE: // The socket is message oriented, and the message is larger than the maximum supported by the underlying transport. 
			case WSAEINVAL: // The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled. 
				con_Print( "<RED>Network System Error:	A fatal error has occured the the client socket at %d.", client );
				return -1;

			// disconnect errors
			case WSAENETDOWN: // The network subsystem has failed. 
			case WSAENETRESET: // The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress. 
			case WSAENOTCONN: // The socket is not connected. 
			case WSAESHUTDOWN: // The socket has been shut down; it is not possible to send on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH. 
			case WSAEHOSTUNREACH: // The remote host cannot be reached from this host at this time. 
			case WSAECONNABORTED: // The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable. 
			case WSAECONNRESET: // The virtual circuit was reset by the remote side executing a hard or abortive close. For UPD sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a "Port Unreachable" ICMP packet. The application should close the socket as it is no longer usable. 
			case WSAETIMEDOUT: // The connection has been dropped, because of a network failure or because the system on the other end went down without notice. 
				con_Print( "<RED>Network System Warning:  The connection with the client at %d has ended abnormally.", client );
				return -1;
				
			// we have flooded the kernel tcp stack; wait till the next frame to try sending
			case WSAENOBUFS: // No buffer space is available. 
			case WSAEWOULDBLOCK: // The socket is marked as nonblocking and the requested operation would block. 
				return 1;

			// useless errors
			case WSAEINTR: // A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall. 
			case WSAEINPROGRESS: // A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. 
			case WSAEOPNOTSUPP: // MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations. 
			default:
				con_Print( "Network Warning:  Server send failed with a meaningless error." );
				break;
			}
		}
		
		// adjust the frame length
		svSdFrLen[client] -= iThisSend;

		// adjust the send start point but remember to make it circular
		svSdFrSt[client] += iThisSend;
		if( svSdFrSt[client] >= FRAME_SIZE ) svSdFrSt[client] -= FRAME_SIZE;
	}

	// if we got stuck somewhere without generating an error condition
	if( cnt == 4 ) return 0;

	// reset our start and length for the flushed buffer to get the maximum packing out of sendData
	svSdFrSt[client] = 0;
	svSdFrLen[client] = 0;
	return 1;
}

/* ------------
net_sv_Send**** - calls send data with the adjusted network versions of the data
------------ */
byte net_sv_SendByte( connection_t client, byte b ) 
{ 
	return net_sv_SendData( client, &b, 1 ); 
}
byte net_sv_SendShort( connection_t client, unsigned short int us ) 
{
	unsigned short int usTmp = htons(us);
	return net_sv_SendData( client, (byte*)&usTmp, 2 ); 
}
byte net_sv_SendLong( connection_t client, unsigned long int ul ) 
{ 
	unsigned long int ulTmp = htonl(ul);
	return net_sv_SendData( client, (byte*)&ulTmp, 4 ); 
}
byte net_sv_SendString( connection_t client, char *str ) 
{ 
	return net_sv_SendData( client, str, (unsigned short int)tcstrlen(str) ); 
}

/* ------------
net_pk_FrameHasAPacket - returns > 0 if a complete packet resides in the indicated frame with the specified parameters
					 - returns 0 if a packet is present, if present, returns the packet type byte and sets *size to the data size
------------ */
static byte net_pk_FrameHasAPacket( byte *frame, unsigned short int start, unsigned short int length, unsigned short int *size )
{
	unsigned short int index;
	unsigned short int usTmp, usSize;
	
	// check the simple case
	if( length < 3 ) return 0;

	// if we have 3 or more bytes, decode the second two to find the size;  remember to decode in nbo
	index = start + 1;
	usTmp = 0;
	if( index >= FRAME_SIZE ) index -= FRAME_SIZE;
	usTmp += frame[index];
	index++;
	if( index >= FRAME_SIZE ) index -= FRAME_SIZE;
	usTmp += frame[index] << 8;
	usSize = ntohs( usTmp );

	// check for the hard error case
	if( usSize >= FRAME_SIZE )
	{
		con_Print( "<RED>Network Client Error:  The server sent a packet larger than the TCP frame buffer." );
		return 0;
	}

	// check the available length against the packet data size
	// if true we don't have enough of the packet to continue
	if( (usSize + 3) > length ) return 0;

	// if the entire packet is present, return the type and the size
	*size = usSize;
	return frame[start];
}


/* ------------
net_GetShort - converts the 2 byte unsigned integer from network order to host order and returns a machine int
------------ */
unsigned short int net_GetShort( byte *buf )
{
	unsigned short int usTmp;

	// assert
	if( buf == NULL ) return 0;
	
	memcpy( &usTmp, buf, 2 );
	return ntohs( usTmp );
}

/* ------------
net_GetLong - converts the 4 byte unsigned integer from network order to host order and returns a machine int
------------ */
unsigned long int net_GetLong( byte *buf )
{
	unsigned long int ulTmp;

	// assert
	if( buf == NULL ) return 0;

	memcpy( &ulTmp, buf, 4 );
	return ntohl( ulTmp );
}

/* ------------
net_GetString - copies a network bytes from buf to machine chars in outBuf until buf is 0 or maxLen is reached.
			- returns the number of bytes copied
------------ */
unsigned long int net_GetString( char *outBuf, byte *buf, int maxLen )
{
	int a;

	// assert
	if( outBuf == NULL ) return 0;
	if( buf == NULL ) return 0;
	if( maxLen <= 0 ) return 0;
	
	for( a = 0 ; a < maxLen ; a++ )
	{
		if( !buf ) return a;
		outBuf[a] = buf[a];
	}

	return a;
}

