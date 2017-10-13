// Cognition
// misc.h
// created by Terrence Cole 4-8-02

#ifndef _MISC_H
#define _MISC_H

// Constant Definitions
////////////////////

// screen absolutes
#define SCR_WIDTH 1024
#define SCR_HEIGHT 768
#define SCR_WIDTH_HALF 320
#define SCR_HEIGHT_HALF 240

// module system
#define API_VERSION 1
#define MENU_MODULE_NAME "menu.dll"
#define CLIENT_MODULE_NAME "client.dll"
#define SERVER_MODULE_NAME "server.dll"

// image system
#define IMG_ALPHA_SOLID 255
#define IMG_ALPHA_CLEAR 0

// camera processor
#define CAMERA_PROJECTION 1
#define CAMERA_ORTHAGONAL 2

// lighting system
#define LIGHT_NORMAL 0
#define LIGHT_SPOT 1
#define LIGHT_DIRECTIONAL 2

// networking definitions
#define DEF_SERVER_PORT "31492"
#define LOCALHOST "127.0.0.1"

// world definitions
#define WORLD_CUBE_SIZE 160.0f
#define WRLD_CUBE_MINE 28

// entity definitions
#define ENT_TYPE_PLAYER 1
#define PLAYER_MAX_NAME_SIZE 64
#define ENT_TRANFORM_MODE_EULER 0
#define ENT_TRANFORM_MODE_QUAT 1

// player states
#define PL_STATE_DISCONNECTED 1
#define PL_STATE_CONNECTING 2
#define PL_STATE_CONNECTED 4
#define INVALID_KEY -1

// text colors
#define CON_CS_BLACK 0
#define CON_CS_WHITE 1
#define CON_CS_GRAY 2
#define CON_CS_RED 3
#define CON_CS_ORANGE 4
#define CON_CS_YELLOW 5
#define CON_CS_GREEN 6
#define CON_CS_BLUE 7
#define CON_CS_VIOLET 8
#define CON_CS_AQUA 9
#define CON_CS_MAGENTA 10
#define CON_CS_CORNFLOWERBLUE 11
#define CON_CS_FOUR 12
#define CON_CS_FIVE 13
#define CON_CS_SIX 14

// defined background count
#define CON_NUM_BACKGROUNDS 5

// particle system
#define PS_FUNCT_CONST 2  // the unit step u(-(t-t0)) on [0,t] ;  A*u(t)
#define PS_FUNCT_IMPULSE 3  // delta(t), area 1 ; SUM( A*d(t-kB) ), k in N, C stores next spike time
#define PS_FUNCT_LINEAR 4  // [0,t], B=starting rate, A=rate change per millisecond ; |At+B|
#define PS_FUNCT_SIN 5  // [0,t], B=length of period, A=parts per millisecond multiplier ; |A*sin(2*PI*t/B)|
#define PS_FUNCT_EXP 7 // [0,t],AC=constant, B=coeficient ; A*e^(Bt)
#define PS_FUNCT_RING 8  // |C*e^At * sin( 2*PI*t/B )|  underdamped mass-spring or RLC

#define PS_SHAPE_SPARKS 2
#define PS_SHAPE_FIRE 4
#define PS_SHAPE_SMOKE 8

// game types
#define GAME_TYPE_NOGAME 0
#define GAME_TYPE_SINGLE 1
#define GAME_TYPE_MULTI 2

// server game states
#define SER_STATE_LOUNGE 1
#define SER_STATE_LAUNCH 2
#define SER_STATE_LAUNCH_TIME 4000
#define SER_STATE_PLAYING 3
#define SER_STATE_RESTART 4

// NETWORK COMMUNICATION
// client -> server
#define PK_CTS_PLAYER_DATA 2  // 		str (n+1), str (n)
		// second stage of handshake -> use as reply to STC_JOIN_ACCEPT after storing the key

#define PK_CTS_READY_TO_PLAY 3 //
		// fourth stage of handshake -> use as reply to world state after creating world, no args

#define PK_CTS_WORLD_EVENT	4 //		pos (3 * 4), newState (1)
#define PK_CTS_PLAYER_NAME 5 //		str (n)
#define PK_CTS_PLAYER_MODEL 6 // 	str (n)
#define PK_CTS_PLAYER_POS 7 //		pos (3 * 4)
#define PK_CTS_PLAYER_ANG 8 //		pos (2 * 2)
	// world change events

#define PK_CTS_PLAYER_DISCONNECT 9 // null
#define PK_CTS_MESSAGE 10 //			for (4), str (n)

// server -> client
#define PK_STC_JOIN_ACCEPT 11 // key (n), str (n)
#define PK_STC_JOIN_DENY 12 // str (n)
	// first phase of the handshake -> sent immediately after accept

#define PK_STC_WORLD_STATE_START 13 // time (4), maxClients (1), size (3 * 4)
#define PK_STC_WORLD_STATE_CUBE 26 // pos (3 * 4), state (2)
#define PK_STC_WORLD_STATE_END 27 // none
#define PK_STC_PLAYER_STATE 14 // key (4), name (n), break (1), model (n), break (1), pos (3 * 4), ang (2 * 2)
	// third stage of handshake

#define PK_STC_WORLD_PLAYER_POS 15 // pos (3 * 4) 

#define PK_STC_PLAYER_DISCONNECT 16 //	msg (n)
		// used to kick clients
	
#define PK_STC_MESSAGE 17 //			str (n)
		// used to retransmit whispers, etc

// server -> BroadCast
#define PK_SBC_PLAYER_STATE 18 // key (4), name (n), break (1), model (n), break (1), pos (3 * 4), ang (2 * 2)
	// fifth (last) stage of handshake, final positioning of new player, bcast to all players

#define PK_SBC_WORLD_EVENT 19 // pos (3 * 4), newState (1)
#define PK_SBC_PLAYER_NAME 20 // key (4), str (n)
#define PK_SBC_PLAYER_MODEL 21 // key (4), str (n)
#define PK_SBC_PLAYER_POS 22 // time (4), pos (3 * 4)
#define PK_SBC_PLAYER_ANG 23 // time (4), ang (2 * 2)
		// sent to all when the world changes

#define PK_SBC_PLAYER_DISCONNECT 24 //	key (4)
	// sent when a player disconnects

#define PK_SBC_MESSAGE 25 // str (n)
	// used to rebroadcast 'say' etc

#define PK_STC_WORLD_STATE_GAME 28 // state(4)
	// sent on connect to tell the client the current game state

// Index Definitions
//////////////////////
#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2
#define W_AXIS 3

#define S_AXIS 0
#define T_AXIS 1

#define R_AXIS 0
#define G_AXIS 1
#define B_AXIS 2
#define A_AXIS 3

#define RECT_LEFT 0
#define RECT_RIGHT 1
#define RECT_TOP 2
#define RECT_BOTTOM 3

#define PITCH 0
#define YAW 1
#define ROLL 2
#define R_SP 2

#define MAX_UINT 0xFFFFFFFF
#define MAX_INT 0x7FFFFFFF

#define COL_PRIORITY_NONE 1
#define COL_PRIORITY_LOW 2
#define COL_PRIORITY_MEDIUM 3
#define COL_PRIORITY_HIGH 4

// Data Type Definitions
//////////////////////////
typedef unsigned char byte;
typedef byte color[4];
typedef float vec2[2];
typedef float vec3[3];
typedef float vec4[4];
typedef float matrix44f[4][4];

// variable system
typedef void* varlatch;

// sound system
typedef int sndhandle;

// timer system
typedef int cog_timer_t;
#define NULL_TIMER -1

// command processor
typedef int command_t;
#define NULL_COMMAND -1

// network system
typedef int connection_t;
#define INVALID_CONNECTION -1

static vec3 vZero = { 0.0f, 0.0f, 0.0f };
static vec3 vUnity = { 1.0f, 1.0f, 1.0f };

// System dependent stuff
#ifdef WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif // stupid windows

#endif // _MISC_H
