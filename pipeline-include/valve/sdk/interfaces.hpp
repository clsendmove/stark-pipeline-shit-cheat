

#pragma once

#include "shared.hpp"
#include "memory.hpp"

#include "math.hpp"

class i_client_entity;
class i_client_networkable;
class i_client_unknown;

using c_base_handle = unsigned int;

typedef enum {
    DPT_Int = 0,
    DPT_Float,
    DPT_Vector,
    DPT_VectorXY,
    DPT_String,
    DPT_Array,
    DPT_DataTable,
    DPT_NUMSendPropTypes
} SendPropType;

class DVariant {
public:
    DVariant() { m_Type = DPT_Float; }
    DVariant(float val) {
        m_Type = DPT_Float;
        m_Float = val;
    }

    union {
        float       m_Float;
        int         m_Int;
        const char* m_pString;
        void* m_pData; // For DataTables.
        float       m_Vector[3];
    };
    SendPropType m_Type;
};

class RecvTable;
class RecvProp;
class CRecvProxyData {
public:
    const RecvProp* m_pRecvProp; // The property it's receiving.
    DVariant        m_Value;     // The value given to you to store.
    int             m_iElement;  // Which array element you're getting.
    int             m_ObjectID;  // The object being referred to.
};

class CRecvDecoder;

typedef void (*RecvVarProxyFn)(const CRecvProxyData* pData, void* pStruct, void* pOut);
typedef void (*ArrayLengthRecvProxyFn)(void* pStruct, int objectID, int currentArrayLength);
typedef void (*DataTableRecvVarProxyFn)(const RecvProp* pProp, void** pOut, void* pData, int objectID);

class SendProp;
class CSendTablePrecalc;
class send_table {
public:
    typedef SendProp   PropType;
    SendProp* Props;
    int                PropsCount;
    const char* NetTableName;
    CSendTablePrecalc* Precalc;

protected:
    bool Initialized : 1;
    bool HasBeenWritten : 1;
    bool HasPropsEncodedAgainstCurrentTickCount : 1;
};

class RecvProp {
public:
    const char* m_pVarName;
    SendPropType            m_RecvType;
    int                     m_Flags;
    int                     m_StringBufferSize;
    bool                    m_bInsideArray;

    const void* m_pExtraData;

    RecvProp* m_pArrayProp;
    ArrayLengthRecvProxyFn  m_ArrayLengthProxy;

    RecvVarProxyFn          m_ProxyFn;
    DataTableRecvVarProxyFn m_DataTableProxyFn;

    RecvTable* m_pDataTable;
    int                     m_Offset;

    int                     m_ElementStride;
    int                     m_nElements;

    const char* m_pParentArrayPropName;
};

class RecvTable {
public:
    typedef RecvProp PropType;

    RecvProp* m_pProps;
    int              m_nProps;
    CRecvDecoder* m_pDecoder;
    const char* m_pNetTableName;

    bool             m_bInitialized;
    bool             m_bInMainList;
};

//-----------------------------------------------------------------------------
// Purpose: This data structure is filled in by the engine when the client .dll requests information about
//  other players that the engine knows about
//-----------------------------------------------------------------------------

// Engine player info, no game related infos here
// If you change this, change the two byteswap defintions:
// cdll_client_int.cpp and cdll_engine_int.cpp
typedef struct player_info_s {
    // scoreboard information
    char name[MAX_PLAYER_NAME_LENGTH];
    // local server user ID, unique while server is running
    int  userID;
    // global unique player identifer
    char guid[SIGNED_GUID_LEN + 1];
    // friends identification number
    unsigned int  friendsID;
    // friends name
    char friendsName[MAX_PLAYER_NAME_LENGTH];
    // true, if player is a bot controlled by game.dll
    bool fakeplayer;
    // true if player is the HLTV proxy
    bool ishltv;
    bool isreplay;
    unsigned int  customFiles[4];
    unsigned char   filesDownloaded;
} player_info_t;

//-----------------------------------------------------------------------------
// Purpose: The engine reports to the client DLL what stage it's entering so the DLL can latch events
//  and make sure that certain operations only happen during the right stages.
// The value for each stage goes up as you move through the frame so you can check ranges of values
//  and if new stages get added in-between, the range is still valid.
//-----------------------------------------------------------------------------
enum frame_stage {
    FRAME_UNDEFINED = -1, // (haven't run any frames yet)
    FRAME_START,
    // A network packet is being received
    FRAME_NET_UPDATE_START,
    // Data has been received and we're going to start calling PostDataUpdate
    FRAME_NET_UPDATE_POSTDATAUPDATE_START,
    // Data has been received and we've called PostDataUpdate on all data recipients
    FRAME_NET_UPDATE_POSTDATAUPDATE_END,
    // We've received all packets, we can now do interpolation, prediction, etc..
    FRAME_NET_UPDATE_END,

    // We're about to start rendering the scene
    FRAME_RENDER_START,
    // We've finished rendering the scene.
    FRAME_RENDER_END
};

typedef void* (*CreateClientClassFn)(int entnum, int serialNum);
typedef void* (*CreateEventFn)();
class client_class {
public:
    CreateClientClassFn m_pCreateFn;
    CreateEventFn       m_pCreateEventFn; // Only called for event objects.
    const char* m_pNetworkName;
    RecvTable* m_pRecvTable;
    client_class* m_pNext;
    int                 m_ClassID; // Managed by the engine.

    e_class_ids         get_class() { return static_cast<e_class_ids>(m_ClassID); }
};

class bf_write;
class c_hl_client {
public:
    client_class* get_all_classes();
    void          create_move(int sequence_number, float input_sample_frametime, bool active);
    bool          write_cmd_to_buffer(bf_write* buf, int from, int to, bool is_new_cmd);
    bool          get_player_view(c_view_setup& player_view);
};

class c_engine_client {
public:
    void             get_screen_size(int& w, int& h);
    void             client_cmd_unrestricted(const char* input);
    bool             get_player_info(int entidx, player_info_t* info);
    int              get_local_player();
    float            plat_float_time();
    void             get_view_angles(QAngle& va);
    vector get_view_angles();
    void             set_view_angles(QAngle& va);
    //59



    int              get_max_clients();
    // is not loading anymore but is connected
    bool             is_in_game();
    // is connected but still loading
    bool             is_connected();
    // returns the game's root directory
    const char* get_game_directory();
    const matrix4x4& world_to_screen_matrix();
};



class CBaseHudChat
{
public:
    void chatprintf(int pIndex, const char* fmt, ...)
    {
        memory::find_vfunc<void(_cdecl*)(void*, int, int, const char*, ...)>(this, 19)(this, pIndex, 0, fmt);
    }

    void startmessagemode(int iMessageModeType)
    {
        CALL_VFUNC(void, 20, int)(this, iMessageModeType);
    }
};



// ripped from the hl2src leak
class i_handle_entity {
public:
    virtual ~i_handle_entity() {}
    virtual void                 set_ref_e_handle(const c_base_handle& handle) = 0;
    virtual const c_base_handle& get_ref_e_handle() const = 0;
};

class i_client_entity_list {
public:
    virtual i_client_networkable* get_client_networkable(int ent_num) = 0;
    virtual i_client_networkable* get_client_networkable_from_handle(c_base_handle ent) = 0;
    virtual i_client_unknown* get_client_unknown_from_handle(c_base_handle ent) = 0;
    virtual i_client_entity* get_client_entity(int ent_num) = 0;
    virtual i_client_entity* get_client_entity_from_handle(c_base_handle ent) = 0;
    virtual int                   number_of_entities(bool include_non_networked) = 0;
    virtual int                   get_highest_ent_index(void) = 0;
    virtual void                  get_max_entities(int max_ents) = 0;
    virtual int                   get_max_entities() = 0;
};

inline i_client_entity_list* entity_list = nullptr;



class client_mode_shared {
private:
    char szPad[28];
public:
    CBaseHudChat* chat_element;
};


inline c_engine_client* engine_client = nullptr;
inline client_mode_shared* client_mode = nullptr;
inline c_hl_client* client = nullptr;

class CSaveRestoreData;

//-----------------------------------------------------------------------------
// Purpose: Global variables used by shared code
//-----------------------------------------------------------------------------
class c_global_vars_base {
public:
    // Absolute time (per frame still - Use Plat_FloatTime() for a high precision real time
    //  perf clock, but not that it doesn't obey host_timescale/host_framerate)
    float             realtime;
    // Absolute frame counter
    int               framecount;
    // Non-paused frametime
    float             absoluteframetime;
    // Current time
    //
    // On the client, this (along with tickcount) takes a different meaning based on what
    // piece of code you're in:
    //
    //   - While receiving network packets (like in PreDataUpdate/PostDataUpdate and proxies),
    //     this is set to the SERVER TICKCOUNT for that packet. There is no interval between
    //     the server ticks.
    //     [server_current_Tick * tick_interval]
    //
    //   - While rendering, this is the exact client clock
    //     [client_current_tick * tick_interval + interpolation_amount]
    //
    //   - During prediction, this is based on the client's current tick:
    //     [client_current_tick * tick_interval]
    float             cur_time;
    // Time spent on last server or client frame (has nothing to do with think intervals)
    float             frame_time;
    // current maxplayers setting
    int               max_clients;
    // Simulation ticks
    int               tick_count;
    // Simulation tick interval
    float             interval_per_tick;
    // interpolation amount ( client-only ) based on fraction of next tick which has elapsed
    float             interpolation_amount;
    int               sim_ticks_this_frame;
    int               network_protocol;
    // current saverestore data
    CSaveRestoreData* save_data;

public:
    // Set to true in client code.
    bool is_client;
    // 100 (i.e., tickcount is rounded down to this base and then the "delta" from this base is networked
    int  time_stamp_networking_base;
    // 32 (entindex() % nTimestampRandomizeWindow ) is subtracted from gpGlobals->tickcount to set the
    // networking basis, prevents
    //  all of the entities from forcing a new PackedEntity on the same tick (i.e., prevents them from getting
    //  lockstepped on this)
    int  time_stamp_randomize_window;
};

enum {
    // This controls how much it smoothes out the samples from the server.
    NUM_CLOCKDRIFT_SAMPLES = 16
};

class c_clock_drift_manager {
public:
    // This holds how many ticks the client is ahead each time we get a server tick.
    // We average these together to get our estimate of how far ahead we are.
    float clock_offsets[NUM_CLOCKDRIFT_SAMPLES];
    int   cur_clock_offset;

    int   server_tick; // Last-received tick from the server.
    int   client_tick; // The client's own tick counter (specifically, for interpolation during rendering).
    // The server may be on a slightly different tick and the client will drift towards it.
};

struct AddAngle {
    float total;
    float starttime;
};

class CEventInfo
{
public:
    enum
    {
        EVENT_INDEX_BITS = 8,
        EVENT_DATA_LEN_BITS = 11,
        MAX_EVENT_DATA = 192,  // ( 1<<8 bits == 256, but only using 192 below )
    };

    ~CEventInfo()
    {
        if (pData)
        {
            delete pData;
        }
    }

    // 0 implies not in use
    short classID;

    // If non-zero, the delay time when the event should be fired ( fixed up on the client )
    float fire_delay;

    // send table pointer or NULL if send as full update
    const send_table* pSendTable;
    const client_class* pClientClass;

    // Length of data bits
    int		bits;
    // Raw event data
    byte* pData;
    // CLIENT ONLY Reliable or not, etc.
    int		flags;
private:
    char pad[24];
public:
    CEventInfo* next;
};

// a client can have up to 4 customization files (logo, sounds, models, txt).
#define MAX_CUSTOM_FILES            4 // max 4 files
#define MAX_AREA_STATE_BYTES        32
#define MAX_AREA_PORTAL_STATE_BYTES 24

typedef unsigned int CRC32_t;
typedef struct CustomFile_s {
    CRC32_t      crc;   // file CRC
    unsigned int reqID; // download request ID
} CustomFile_t;

class c_client_state {
public:
private:
    char pad_0000[8]; // 0x0000
public:
    void* thisptr = nullptr; // 0x0008
private:
    char pad_000C[4]; // 0x000C
public:
    CNetChannel* m_NetChannel = nullptr; // 0x0010
private:
    char pad_0014[316]; // 0x0014
public:
    int                   m_nCurrentSequence = 0; // 0x0150
    c_clock_drift_manager m_ClockDriftMgr = {};   // 0x0154
    int                   m_nDeltaTick = 0;       // 0x01A0
private:
    char pad_01A4[16]; // 0x01A4
public:
    char m_szLevelFileName[128]; // 0x01B4
    char m_szLevelBaseName[128]; // 0x0234
    int  m_nMaxClients = 0;      // 0x02B4
private:
    char pad_02B8[18528]; // 0x02B8
public:
    int                  oldtickcount = 0;        // 0x4B18
    float                m_tickRemainder = 0.f;   // 0x4B1C
    float                m_frameTime = 0.f;       // 0x4B20
    int                  lastoutgoingcommand = 0; // 0x4B24
    int                  chokedcommands = 0;      // 0x4B28
    int                  last_command_ack = 0;    // 0x4B2C
    int                  command_ack = 0;         // 0x4B30
    int                  m_nSoundSequence = 0;
    bool                 ishltv;
    MD5Value_t           serverMD5;
    unsigned char        m_chAreaBits[MAX_AREA_STATE_BYTES];
    unsigned char        m_chAreaPortalBits[MAX_AREA_PORTAL_STATE_BYTES];
    bool                 m_bAreaBitsValid;
    QAngle               viewangles;
    CUtlVector<AddAngle> addangle;
    float                addangletotal;
    float                prevaddangletotal;
    int                  cdtrack;
    CustomFile_t         m_nCustomFiles[MAX_CUSTOM_FILES];
    unsigned int         m_nFriendsID;
    char                 m_FriendsName[MAX_PLAYER_NAME_LENGTH];
    CEventInfo* events;
};

inline c_client_state* client_state = nullptr;
inline c_global_vars_base* globalsvarsbase = nullptr;

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class i_con_var;
class c_command;

//-----------------------------------------------------------------------------
// ConVar flags
//-----------------------------------------------------------------------------
// The default, no flags at all
#define FCVAR_NONE         0

// Command to ConVars and ConCommands
// ConVar Systems
#define FCVAR_UNREGISTERED (1 << 0) // If this is set, don't add to linked list, etc.
#define FCVAR_DEVELOPMENTONLY \
  (1 << 1) // Hidden in released products. Flag is removed automatically if ALLOW_DEVELOPMENT_CVARS is
           // defined.
#define FCVAR_GAMEDLL   (1 << 2) // defined by the game DLL
#define FCVAR_CLIENTDLL (1 << 3) // defined by the client DLL
#define FCVAR_HIDDEN \
  (1 << 4) // Hidden. Doesn't appear in find or auto complete. Like DEVELOPMENTONLY, but can't be compiled
           // out.

// ConVar only
#define FCVAR_PROTECTED \
  (1 << 5) // It's a server cvar, but we don't send the data since it's a password, etc.  Sends 1 if it's not
           // bland/zero, 0 otherwise as value
#define FCVAR_SPONLY   (1 << 6) // This cvar cannot be changed by clients connected to a multiplayer server.
#define FCVAR_ARCHIVE  (1 << 7) // set to cause it to be saved to vars.rc
#define FCVAR_NOTIFY   (1 << 8) // notifies players when changed
#define FCVAR_USERINFO (1 << 9) // changes the client's info string

#define FCVAR_PRINTABLEONLY \
  (1 << 10) // This cvar's string cannot contain unprintable characters ( e.g., used for player name etc ).
#define FCVAR_UNLOGGED \
  (1 << 11) // If this is a FCVAR_SERVER, don't log changes to the log file / console if we are creating a log
#define FCVAR_NEVER_AS_STRING (1 << 12) // never try to print that cvar

// It's a ConVar that's shared between the client and the server.
// At signon, the values of all such ConVars are sent from the server to the client (skipped for local
//  client, of course )
// If a change is requested it must come from the console (i.e., no remote client changes)
// If a value is changed while a server is active, it's replicated to all connected clients
#define FCVAR_REPLICATED \
  (1 << 13)                   // server setting enforced on clients, TODO rename to FCAR_SERVER at some time
#define FCVAR_CHEAT (1 << 14) // Only useable in singleplayer / debug / multiplayer & sv_cheats
#define FCVAR_SS \
  (1 << 15) // causes varnameN where N == 2 through max splitscreen slots for mod to be autogenerated
#define FCVAR_DEMO                   (1 << 16) // record this cvar when starting a demo file
#define FCVAR_DONTRECORD             (1 << 17) // don't record these command in demofiles
#define FCVAR_SS_ADDED               (1 << 18) // This is one of the "added" FCVAR_SS variables for the splitscreen players
#define FCVAR_RELEASE                (1 << 19) // Cvars tagged with this are the only cvars avaliable to customers
#define FCVAR_RELOAD_MATERIALS       (1 << 20) // If this cvar changes, it forces a material reload
#define FCVAR_RELOAD_TEXTURES        (1 << 21) // If this cvar changes, if forces a texture reload

#define FCVAR_NOT_CONNECTED          (1 << 22) // cvar cannot be changed by a client that is connected to a server
#define FCVAR_MATERIAL_SYSTEM_THREAD (1 << 23) // Indicates this cvar is read from the material system thread
#define FCVAR_ARCHIVE_GAMECONSOLE    (1 << 24) // cvar written to config.cfg on the Xbox

#define FCVAR_SERVER_CAN_EXECUTE \
  (1 << 28) // the server is allowed to execute this command on clients via
            // ClientCommand/NET_StringCmd/CBaseClientState::ProcessStringCmd.
#define FCVAR_SERVER_CANNOT_QUERY \
  (1 << 29) // If this is set, then the server is not allowed to query this cvar's value (via
            // IServerPluginHelpers::StartQueryCvarValue).
#define FCVAR_CLIENTCMD_CAN_EXECUTE \
  (1 << 30) // IVEngineClient::ClientCmd is allowed to execute this command.
            // Note: IVEngineClient::ClientCmd_Unrestricted can run any client command.

#define FCVAR_ACCESSIBLE_FROM_THREADS \
  (1 << 25) // used as a debugging tool necessary to check material system thread convars
// #define FCVAR_AVAILABLE			(1<<26)
// #define FCVAR_AVAILABLE			(1<<27)
// #define FCVAR_AVAILABLE			(1<<31)

#define FCVAR_MATERIAL_THREAD_MASK \
  (FCVAR_RELOAD_MATERIALS | FCVAR_RELOAD_TEXTURES | FCVAR_MATERIAL_SYSTEM_THREAD)

//-----------------------------------------------------------------------------
// Called when a ConVar changes value
// NOTE: For FCVAR_NEVER_AS_STRING ConVars, pOldValue == NULL
//-----------------------------------------------------------------------------
typedef void (*fn_change_callback)(i_con_var* var, const char* old_value, float float_old_value);

//-----------------------------------------------------------------------------
// Abstract interface for ConVars
//-----------------------------------------------------------------------------
class _color; // we have our own colors separate to what the engine has
class i_con_var {
public:
    virtual void set_value(const char* value) = 0;
    virtual void set_value(float value) = 0;
    virtual void set_value(int value) = 0;
    virtual const char* get_name(void) const = 0;
    virtual bool is_flag_set(int flag) const = 0;
};

//-----------------------------------------------------------------------------
// Uncomment me to test for threading issues for material system convars
// NOTE: You want to disable all threading when you do this
// +host_thread_mode 0 +r_threaded_particles 0 +sv_parallel_packentities 0 +sv_disable_querycache 0
//-----------------------------------------------------------------------------
// #define CONVAR_TEST_MATERIAL_THREAD_CONVARS 1

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class convar;
class c_command;
class con_command;
class con_command_base;
struct characterset_t;

//-----------------------------------------------------------------------------
// Any executable that wants to use ConVars need to implement one of
// these to hook up access to console variables.
//-----------------------------------------------------------------------------
class con_command_base_accessor {
public:
    virtual bool register_con_command_base(con_command_base* var) = 0;
};

//-----------------------------------------------------------------------------
// Called when a ConCommand needs to execute
//-----------------------------------------------------------------------------
typedef void (*fn_command_callback_v1)(void);
typedef void (*fn_command_callback)(const c_command& command);

#define COMMAND_COMPLETION_MAXITEMS    64
#define COMMAND_COMPLETION_ITEM_LENGTH 64

//-----------------------------------------------------------------------------
// Returns 0 to COMMAND_COMPLETION_MAXITEMS worth of completion strings
//-----------------------------------------------------------------------------
typedef int (*fn_command_completion_callback)(
    const char* partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]);

//-----------------------------------------------------------------------------
// Interface version
//-----------------------------------------------------------------------------
class i_command_callback {
public:
    virtual void command_callback(const c_command& command) = 0;
};

class i_command_completion_callback {
public:
    virtual int command_completion_comeback(const char* partial, void* commands) = 0;
};

struct cvar_dll_identifier;

//-----------------------------------------------------------------------------
// Purpose: The base console invoked command/cvar interface
//-----------------------------------------------------------------------------
class con_command_base {
public:
    virtual ~con_command_base(void);
    virtual bool is_command(void) const;
    virtual bool is_flag_set(int flag) const;
    virtual void add_flags(int flags);
    virtual const char* get_name(void) const;
    virtual const char* get_help_text(void) const;
    virtual bool is_registered(void) const;
    virtual cvar_dll_identifier get_dll_identifier() const;
    virtual void create_base(const char* name, const char* help_string = 0, int flags = 0);

    con_command_base* next;
    bool _is_registered;
    const char* name;
    const char* help_string;
    int flags;

protected:
    static con_command_base* con_command_bases;
    static con_command_base_accessor* accessor;
};

//-----------------------------------------------------------------------------
// Command tokenizer
//-----------------------------------------------------------------------------
class c_command {
public:
    enum {
        COMMAND_MAX_ARGC = 64,
        COMMAND_MAX_LENGTH = 512,
    };

    int         argc;
    int         argv0size;
    char        argsbuffer[COMMAND_MAX_LENGTH];
    char        argvbuffer[COMMAND_MAX_LENGTH];
    const char* argv[COMMAND_MAX_ARGC];
};

//-----------------------------------------------------------------------------
// Purpose: The console invoked command
//-----------------------------------------------------------------------------
class con_command : public con_command_base {
public:
    typedef con_command_base base_class;

    virtual ~con_command(void);

    virtual bool is_command(void) const;

    virtual int  auto_complete_suggest(const char* partial, void*& commands);

    virtual bool can_auto_complete(void);

    // Invoke the function
    virtual void dispatch(const c_command& command);

private:
    // NOTE: To maintain backward compat, we have to be very careful:
    // All public virtual methods must appear in the same order always
    // since engine code will be calling into this code, which *does not match*
    // in the mod code; it's using slightly different, but compatible versions
    // of this class. Also: Be very careful about adding new fields to this class.
    // Those fields will not exist in the version of this class that is instanced
    // in mod code.

    // Call this function when executing the command
    union {
        fn_command_callback_v1 fn_command_callback_v1;
        fn_command_callback   fn_command_callback;
        i_command_callback* command_callback;
    };

    union {
        fn_command_completion_callback fn_completion_callback;
        i_command_completion_callback* command_completion_callback;
    };

    bool has_completion_callback : 1;
    bool using_new_command_completion_callback : 1;
    bool using_command_callback_interface : 1;
};

//-----------------------------------------------------------------------------
// Purpose: A console variable
//-----------------------------------------------------------------------------
class convar : public con_command_base, public i_con_var {
public:
    virtual ~convar(void);
    virtual bool is_flag_set(int flag) const;
    virtual const char* get_help_text(void) const;
    virtual bool is_registered(void) const;
    virtual const char* get_name(void) const;
    virtual void add_flags(int flags);
    virtual	bool is_command(void) const;
    virtual void set_value(const char* value);
    virtual void set_value(float value);
    virtual void set_value(int value);
    virtual void internal_set_value(const char* value);
    virtual void internal_set_float_value(float new_value, bool force = false);
    virtual void internal_set_int_value(int value);
    virtual bool clamp_value(float& value);
    virtual void change_string_value(const char* temp_val, float old_value);
    virtual void init();

    inline float get_float(void) const;
    inline int get_int(void) const;
    inline bool	get_bool() const { return !!get_int(); }
    inline char const* get_string(void) const;

private:
    convar* parent;
    const char* default_value;
    char* string;
    int string_length;
    float float_value;
    int int_value;
    bool has_min;
    float min_value;
    bool has_max;
    float max_value;
    bool has_comp_min;
    float comp_min_value;
    bool has_comp_max;
    float comp_max_value;
    bool competitive_restrictions;
    fn_change_callback m_fn_change_callback;
};

//-----------------------------------------------------------------------------
// Purpose: Return ConVar value as a float
// Output : float
//-----------------------------------------------------------------------------
inline float convar::get_float(void) const {
    return parent->float_value;
}

//-----------------------------------------------------------------------------
// Purpose: Return ConVar value as an int
// Output : int
//-----------------------------------------------------------------------------
inline int convar::get_int(void) const {
    return parent->int_value;
}

//-----------------------------------------------------------------------------
// Purpose: Return ConVar value as a string, return "" for bogus string pointer, etc.
// Output : const char *
//-----------------------------------------------------------------------------
inline const char* convar::get_string(void) const {
    if (flags & FCVAR_NEVER_AS_STRING)
        return "FCVAR_NEVER_AS_STRING";

    char const* str = parent->string;
    return str ? str : "";
}

//-----------------------------------------------------------------------------
// Client systems are singleton objects in the client codebase responsible for
// various tasks
// The order in which the client systems appear in this list are the
// order in which they are initialized and updated. They are shut down in
// reverse order from which they are initialized.
//-----------------------------------------------------------------------------
enum init_return_val {
    INIT_FAILED = 0,
    INIT_OK,

    INIT_LAST_VAL,
};
typedef void* (*create_interface_fn)(const char* name, int* return_code);
class i_app_system {
public:
    // Here's where the app systems get to learn about each other
    virtual bool            connect(create_interface_fn factory) = 0;
    virtual void            disconnect() = 0;

    // Here's where systems can access other interfaces implemented by this object
    // Returns NULL if it doesn't implement the requested interface
    virtual void* query_interface(const char* interface_name) = 0;

    // Init, shutdown
    virtual init_return_val init() = 0;
    virtual void            shutdown() = 0;
};

//-----------------------------------------------------------------------------
// Used to display console messages
//-----------------------------------------------------------------------------
class i_console_display_func {
public:
    virtual void color_print(const _color& clr, const char* message) = 0;
    virtual void print(const char* message) = 0;
    virtual void dprint(const char* message) = 0;
};

class i_cvar_query : public i_app_system {
public:
    virtual bool are_convars_linked(const convar* child, const convar* parent) = 0;
};

//-----------------------------------------------------------------------------
// Purpose: DLL interface to ConVars/ConCommands
//-----------------------------------------------------------------------------
class i_cvar : public i_app_system {
public:
    virtual cvar_dll_identifier allocate_dll_identifier() = 0;
    virtual void register_con_command(con_command_base* command_base) = 0;
    virtual void unregister_con_command(con_command_base* command_base) = 0;
    virtual void unregister_con_commands(cvar_dll_identifier id) = 0;
    virtual const char* get_command_line_value(const char* variable_name) = 0;
    virtual con_command_base* find_command_base(const char* name) = 0;
    virtual const con_command_base* find_command_base(const char* name) const = 0;
    virtual convar* find_var(const char* var_name) = 0;
    virtual const convar* find_var(const char* var_name) const = 0;
    virtual con_command* find_command(const char* name) = 0;
    virtual const con_command* find_command(const char* name) const = 0;
    virtual con_command_base* get_commands(void) = 0;
    virtual const con_command_base* get_commands(void) const = 0;
    virtual void install_global_change_callback(fn_change_callback callback) = 0;
    virtual void remove_global_change_callback(fn_change_callback callback) = 0;
    virtual void call_global_change_callbacks(convar* var, const char* old_string, float old_value) = 0;
    virtual void install_console_display_func(fn_change_callback* display_func) = 0;
    virtual void remove_console_display_func(fn_change_callback* display_func) = 0;
    virtual void console_color_printf(const _color& clr, const char* format, ...) const = 0;
    virtual void console_printf(const char* format, ...) const = 0;
    virtual void console_d_printf(const char* format, ...) const = 0;
    virtual void revert_flagged_ConVars(int flag) = 0;
    virtual void install_c_var_query(i_cvar_query* query) = 0;
    virtual bool is_material_thread_set_allowed() const = 0;
    virtual void queue_material_thread_set_value(convar* convar, const char* value) = 0;
    virtual void queue_material_thread_set_value(convar* convar, int value) = 0;
    virtual void queue_material_thread_set_value(convar* convar, float value) = 0;
    virtual bool has_queued_material_thread_ConVar_sets() const = 0;
    virtual int process_queued_material_thread_ConVar_sets() = 0;
};


inline i_cvar* cvar = nullptr;

class OverlayText_t;

class ivdebugoverlay {
public:

    virtual void AddEntityTextOverlay(int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;
    virtual void AddBoxOverlay(const vector& origin, const vector& mins, const vector& max, QAngle const& orientation, int r, int g, int b, int a, float duration) = 0;
    virtual void AddTriangleOverlay(const vector& p1, const vector& p2, const vector& p3, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
    virtual void AddLineOverlay(const vector& origin, const vector& dest, int r, int g, int b, bool noDepthTest, float duration) = 0;
    virtual void AddTextOverlay(const vector& origin, float duration, const char* format, ...) = 0;
    virtual void AddTextOverlay(const vector& origin, int line_offset, float duration, const char* format, ...) = 0;
    virtual void AddScreenTextOverlay(float flXPos, float flYPos, float flDuration, int r, int g, int b, int a, const char* text) = 0;
    virtual void AddSweptBoxOverlay(const vector& start, const vector& end, const vector& mins, const vector& max, const QAngle& angles, int r, int g, int b, int a, float flDuration) = 0;
    virtual void AddGridOverlay(const vector& origin) = 0;
    virtual int ScreenPosition(const vector& point, vector& screen) = 0;
    virtual int ScreenPosition(float flXPos, float flYPos, vector& screen) = 0;

    virtual OverlayText_t* GetFirst(void) = 0;
    virtual OverlayText_t* GetNext(OverlayText_t* current) = 0;
    virtual void ClearDeadOverlays(void) = 0;
    virtual void ClearAllOverlays() = 0;

    virtual void AddTextOverlayRGB(const vector& origin, int line_offset, float duration, float r, float g, float b, float alpha, const char* format, ...) = 0;
    virtual void AddTextOverlayRGB(const vector& origin, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;

    virtual void AddLineOverlayAlpha(const vector& origin, const vector& dest, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
    virtual void AddBoxOverlay2(const vector& origin, const vector& mins, const vector& max, QAngle const& orientation, const color& faceColor, const color& edgeColor, float duration) = 0;

private:
    inline void AddTextOverlay(const vector& origin, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) {} /* catch improper use of bad interface. Needed because '0' duration can be resolved by compiler to NULL format string (i.e., compiles but calls wrong function) */
};


inline ivdebugoverlay* debugoverlay = nullptr;

class c_game_event {
public:
    virtual ~c_game_event();
    const char* get_name() const;
    bool        is_empty(const char* keyName = NULL);
    bool        is_local() const;
    bool        is_reliable() const;
    bool        get_bool(const char* keyName = NULL, bool defaultValue = false);
    int         get_int(const char* keyName = NULL, int defaultValue = 0);
    float       get_float(const char* keyName = NULL, float defaultValue = 0.0f);
    const char* get_string(const char* keyName = NULL, const char* defaultValue = "");
    void        set_bool(const char* keyName, bool value);
    void        set_bnt(const char* keyName, int value);
    void        set_float(const char* keyName, float value);
    void        set_string(const char* keyName, const char* value);
};

// A safer method than inheriting straight from IGameEventListener2.
// Avoids requiring the user to remove themselves as listeners in
// their destructor, and sets the serverside variable based on
// our dll location.
class c_game_event_listener {
public:
    virtual ~c_game_event_listener() {}

    // Intentionally abstract
    virtual void fire_game_event(c_game_event* event) = 0;
    virtual int  get_event_debug_id() { return m_debug; }

private:
    // Have we registered for any events?
    int m_debug;
};

class CSVCMsg_GameEvent;
class c_game_event_manager {
public: // IGameEventManager functions
    virtual ~c_game_event_manager();
    virtual int   load_events_from_file(const char* filename) = 0;
    virtual void  reset() = 0;
    virtual bool  add_listener(c_game_event_listener* listener, const char* name, bool bServerSide) = 0;
    virtual bool  find_listener(c_game_event_listener* listener, const char* name) = 0;
    virtual void  remove_listener(c_game_event_listener* listener) = 0;
    virtual void  add_listener_global(c_game_event_listener* listener, bool server_side) = 0;
    c_game_event* create_new_event(const char* name, bool force = false) {
        return memory::find_vfunc<c_game_event * (__thiscall*)(void*, const char*, bool)>(this, 6u)(this, name,
            force);
    }
    virtual bool          fire_event(c_game_event* pEvent, bool bDontBroadcast = false) = 0;
    virtual bool          fire_event_client_side(c_game_event* pEvent) = 0;
    virtual c_game_event* duplicate_event(c_game_event* pEvent) = 0;
    virtual void          free_event(c_game_event* pEvent) = 0;
    virtual bool          serialize_event(c_game_event* pEvent, CSVCMsg_GameEvent* pEventMsg) = 0;
    virtual c_game_event* unserialize_event(const CSVCMsg_GameEvent& eventMsg) = 0;
    virtual void* get_event_data_types(c_game_event* pEvent) = 0;
};

inline c_game_event_manager* game_event_manager = nullptr;

class i_input {
public:
    void cam_to_third_person();
    void cam_to_first_person();
};

inline i_input* input = nullptr;
class c_input_system {
public:
    void reset_input_state() {
        memory::find_vfunc<void(__thiscall*)(void*)>(this, 25u)(this);
    }
};

inline c_input_system* input_system = nullptr;

class i_move_helper {
public:

};

class tf_entity;
class c_user_cmd;
class c_prediction {
public:
    unsigned char pad[8];
    bool in_prediction;
    bool first_time_predicted;
    void run_command(tf_entity* ent, c_user_cmd* cmd, i_move_helper* move_helper);
};

inline c_prediction* prediction = nullptr;

inline i_move_helper* move_helper = nullptr;

#pragma once
#include <string_view>
#include "../../math.hpp"
#include "Windows.h"
#include <d3d9.h>

struct studiohwdata_t;
struct color_mesh_info_t;
using studio_decal_handle = void*;

constexpr std::string_view TEXTURE_GROUP_OTHER = "Other textures";

enum light_type {
    MATERIAL_LIGHT_DISABLE = 0,
    MATERIAL_LIGHT_POINT,
    MATERIAL_LIGHT_DIRECTIONAL,
    MATERIAL_LIGHT_SPOT,
};

// intersection boxes
struct mstudiobbox_t {
    int         bone;
    int         group; // intersection group
    vector      bbmin; // bounding box
    vector      bbmax;
    int         szhitboxnameindex; // offset to the name of the hitbox.
    int         unused[8];
    const char* pszHitboxName() {
        if (szhitboxnameindex == 0)
            return "";

        return ((const char*)this) + szhitboxnameindex;
    }
    mstudiobbox_t() {}

private:
    // No copy constructors allowed
    mstudiobbox_t(const mstudiobbox_t& vOther);
};

struct mstudiohitboxset_t {
    int                   sznameindex;
    inline char* const    pszName(void) const { return ((char*)this) + sznameindex; }
    int                   numhitboxes;
    int                   hitboxindex;
    inline mstudiobbox_t* pHitbox(int i) const { return (mstudiobbox_t*)(((byte*)this) + hitboxindex) + i; };
};

struct mstudiobone_t {
    int                sznameindex;
    inline char* const pszName(void) const { return ((char*)this) + sznameindex; }
    int                parent;            // parent bone
    int                bonecontroller[6]; // bone controller index, -1 == none
    // default values
    vector             pos;
    void* quat;
    void* rot;
    // compression scale
    vector             posscale;
    vector             rotscale;
    matrix3x4          poseToBone;
    void* qAlignment;
    int                flags;
    int                proctype;
    int                procindex;   // procedural rule
    mutable int        physicsbone; // index into physically simulated bone
    inline void* pProcedure() const {
        if (procindex == 0)
            return NULL;
        else
            return (void*)(((byte*)this) + procindex);
    };
    int                surfacepropidx; // index into string tablefor property name
    inline char* const pszSurfaceProp(void) const { return ((char*)this) + surfacepropidx; }
    int                contents;  // See BSPFlags.h for the contents flags
    int                unused[8]; // remove as appropriate

    mstudiobone_t() {}

private:
    mstudiobone_t(const mstudiobone_t& other);
};
struct studiohdr_t {
    int                   id;
    int                   version;
    int                   checksum; // this has to be the same in the phy and vtx files to load!
    char                  name[64];
    int                   length;
    vector                eyeposition;   // ideal eye position
    vector                illumposition; // illumination center
    vector                hull_min;      // ideal movement hull size
    vector                hull_max;
    vector                view_bbmin; // clipping bounding box
    vector                view_bbmax;
    int                   flags;
    int                   numbones; // bones
    int                   boneindex;
    inline mstudiobone_t* get_bone(int i) const { return (mstudiobone_t*)(((byte*)this) + boneindex) + i; };
    int                   numbonecontrollers; // bone controllers
    int                   bonecontrollerindex;
    int                   numhitboxsets;
    int                   hitboxsetindex;
    inline mstudiohitboxset_t* get_hitbox_set(int i) const {
        return (mstudiohitboxset_t*)(((byte*)this) + hitboxsetindex) + i;
    }
    inline mstudiobbox_t* get_hitbox(int i, int set) const {
        mstudiohitboxset_t const* s = get_hitbox_set(set);
        if (!s)
            return NULL;
        return s->pHitbox(i);
    }
    inline int get_hitbox_count(int set) const {
        mstudiohitboxset_t const* s = get_hitbox_set(set);
        if (!s)
            return 0;
        return s->numhitboxes;
    }
    // file local animations? and sequences
    // private:
    int           numlocalanim;   // animations/poses
    int           localanimindex; // animation descriptions
    int           numlocalseq;    // sequences
    int           localseqindex;
    // private:
    mutable int   activitylistversion; // initialization flag - have the sequences been indexed?
    mutable int   eventsindexed;
    // raw textures
    int           numtextures;
    int           textureindex;
    // raw textures search paths
    int           numcdtextures;
    int           cdtextureindex;
    // replaceable textures tables
    int           numskinref;
    int           numskinfamilies;
    int           skinindex;
    int           numbodyparts;
    int           bodypartindex;
    // queryable attachable points
    // private:
    int           numlocalattachments;
    int           localattachmentindex;
    // animation node to animation node transition graph
    // private:
    int           numlocalnodes;
    int           localnodeindex;
    int           localnodenameindex;
    int           numflexdesc;
    int           flexdescindex;
    int           numflexcontrollers;
    int           flexcontrollerindex;
    int           numflexrules;
    int           flexruleindex;
    int           numikchains;
    int           ikchainindex;
    int           nummouths;
    int           mouthindex;
    // private:
    int           numlocalposeparameters;
    int           localposeparamindex;
    int           surfacepropindex;
    // Key values
    int           keyvalueindex;
    int           keyvaluesize;
    int           numlocalikautoplaylocks;
    int           localikautoplaylockindex;
    // The collision model mass that jay wanted
    float         mass;
    int           contents;
    // external animations, models, etc.
    int           numincludemodels;
    int           includemodelindex;
    // implementation specific back pointer to virtual data
    mutable void* virtualModel;
    // for demand loaded animation blocks
    int           szanimblocknameindex;
    int           numanimblocks;
    int           animblockindex;
    mutable void* animblockModel;
    int           bonetablebynameindex;
    // used by tools only that don't cache, but persist mdl's peer data
    // engine uses virtualModel to back link to cache pointers
    void* pVertexBase;
    void* pIndexBase;
    // if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
    // this value is used to calculate directional components of lighting
    // on static props
    byte          constdirectionallightdot;
    // set during load of mdl data to track *desired* lod configuration (not actual)
    // the *actual* clamped root lod is found in studiohwdata
    // this is stored here as a global store to ensure the staged loading matches the rendering
    byte          rootLOD;
    // set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
    // to be set as root LOD:
    //	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
    //	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
    byte          numAllowedRootLODs;
    byte          unused[1];
    int           unused4; // zero out if version < 47
    int           numflexcontrollerui;
    int           flexcontrolleruiindex;
    float         flVertAnimFixedPointScale;
    int           unused3[1];
    // FIXME: Remove when we up the model version. Move all fields of studiohdr2_t into studiohdr_t.
    int           studiohdr2index;
    // NOTE: No room to add stuff? Up the .mdl file format version
    // [and move all fields in studiohdr2_t into studiohdr_t and kill studiohdr2_t],
    // or add your stuff to studiohdr2_t. See NumSrcBoneTransforms/SrcBoneTransform for the pattern to use.
    int           unused2[1];
};

struct light_desc_t {
    light_type   m_Type;
    vector       m_Color;
    vector       m_Position;
    vector       m_Direction;
    float        m_Range;
    float        m_Falloff;
    float        m_Attenuation0;
    float        m_Attenuation1;
    float        m_Attenuation2;
    float        m_Theta;
    float        m_Phi;
    float        m_ThetaDot;
    float        m_PhiDot;
    unsigned int m_Flags;
    float        OneOver_ThetaDot_Minus_PhiDot;
    float        m_RangeSquared;
};

struct draw_model_info_t {
    studiohdr_t* m_pStudioHdr;
    studiohwdata_t* m_pHardwareData;
    studio_decal_handle m_Decals;
    int                 m_Skin;
    int                 m_Body;
    int                 m_HitboxSet;
    void* m_pClientEntity;
    int                 m_Lod;
    color_mesh_info_t* m_pColorMeshes;
    bool                m_bStaticLighting;
    vector              m_vecAmbientCube[6];
    int                 m_nLocalLightCount;
    light_desc_t        m_LocalLightDescs[4];
    uint8_t             m_pad_00[1024];
};

struct draw_model_results_t {
    int m_ActualTriCount;
    int m_TextureMemoryBytes;
    int m_NumHardwareBones;
    int m_NumBatches;
    int m_NumMaterials;
    int m_nLODUsed;
    int m_flLODMetric;
};

//-----------------------------------------------------------------------------
// What kind of material override is it?
//-----------------------------------------------------------------------------
enum override_type_t {
    OVERRIDE_NORMAL = 0,
    OVERRIDE_BUILD_SHADOWS,
    OVERRIDE_DEPTH_WRITE,
    OVERRIDE_SSAO_DEPTH_WRITE,
};

//-----------------------------------------------------------------------------
// Shader state flags can be read from the FLAGS materialvar
// Also can be read or written to with the Set/GetMaterialVarFlags() call
// Also make sure you add/remove a string associated with each flag below to CShaderSystem::ShaderStateString
// in ShaderSystem.cpp
//-----------------------------------------------------------------------------
enum material_var_flags_t {
    MATERIAL_VAR_DEBUG = (1 << 0),
    MATERIAL_VAR_NO_DEBUG_OVERRIDE = (1 << 1),
    MATERIAL_VAR_NO_DRAW = (1 << 2),
    MATERIAL_VAR_USE_IN_FILLRATE_MODE = (1 << 3),
    MATERIAL_VAR_VERTEXCOLOR = (1 << 4),
    MATERIAL_VAR_VERTEXALPHA = (1 << 5),
    MATERIAL_VAR_SELFILLUM = (1 << 6),
    MATERIAL_VAR_ADDITIVE = (1 << 7),
    MATERIAL_VAR_ALPHATEST = (1 << 8),
    MATERIAL_VAR_MULTIPASS = (1 << 9),
    MATERIAL_VAR_ZNEARER = (1 << 10),
    MATERIAL_VAR_MODEL = (1 << 11),
    MATERIAL_VAR_FLAT = (1 << 12),
    MATERIAL_VAR_NOCULL = (1 << 13),
    MATERIAL_VAR_NOFOG = (1 << 14),
    MATERIAL_VAR_IGNOREZ = (1 << 15),
    MATERIAL_VAR_DECAL = (1 << 16),
    MATERIAL_VAR_ENVMAPSPHERE = (1 << 17),
    MATERIAL_VAR_NOALPHAMOD = (1 << 18),
    MATERIAL_VAR_ENVMAPCAMERASPACE = (1 << 19),
    MATERIAL_VAR_BASEALPHAENVMAPMASK = (1 << 20),
    MATERIAL_VAR_TRANSLUCENT = (1 << 21),
    MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = (1 << 22),
    MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING = (1 << 23),
    MATERIAL_VAR_OPAQUETEXTURE = (1 << 24),
    MATERIAL_VAR_ENVMAPMODE = (1 << 25),
    MATERIAL_VAR_SUPPRESS_DECALS = (1 << 26),
    MATERIAL_VAR_HALFLAMBERT = (1 << 27),
    MATERIAL_VAR_WIREFRAME = (1 << 28),
    MATERIAL_VAR_ALLOWALPHATOCOVERAGE = (1 << 29),
    MATERIAL_VAR_IGNORE_ALPHA_MODULATION = (1 << 30),

    // NOTE: Only add flags here that either should be read from
    // .vmts or can be set directly from client code. Other, internal
    // flags should to into the flag enum in imaterialinternal.h
};

class i_material {
public:
    void set_material_var_flag(material_var_flags_t flag, bool state);
};

class c_material_system {
public:
    i_material* find_material(std::string_view name, std::string_view group, bool complain = true,
        const char* complain_prefix = nullptr);
};

// 29, 28
class c_studio_render {
public:
    void set_color_modulation(const float* color);
    void set_alpha_modulation(float alpha);
    void forced_material_override(i_material* material, override_type_t override_type = OVERRIDE_NORMAL);
};

struct model_t {
    void* fnHandle;
    char* strName;

    int   nLoadFlags;   // mark loaded/not loaded
    int   nServerCount; // marked at load
    i_material**
        ppMaterials; // null-terminated runtime material cache; ((intptr_t*)(ppMaterials))[-1] == nMaterials

    void* type;
    int    flags; // MODELFLAG_???

    // volume occupied by the model graphics
    vector mins, maxs;
    float  radius;

    union {
        void* brush;
        void* studio;
        void* sprite;
    };
};

class c_model_info_client {
public:
    model_t* get_model(int entidx);
    studiohdr_t* get_studio_model(const model_t* model);
    const char* get_model_name(const model_t* model);

};

class c_render_view {
public:
    void get_matrices_for_view(const c_view_setup* view, D3DMATRIX* world_to_view, D3DMATRIX* view_to_projection, D3DMATRIX* world_to_projection, D3DMATRIX* world_to_pixels);
};

inline c_model_info_client* model_info_client = nullptr;
inline c_studio_render* studio_render = nullptr;
inline c_material_system* material_system = nullptr;
inline c_render_view* render_view = nullptr;

enum vguipanel_t
{
    PANEL_ROOT = 0,
    PANEL_GAMEUIDLL,
    PANEL_CLIENTDLL,
    PANEL_TOOLS,
    PANEL_INGAMESCREENS,
    PANEL_GAMEDLL,
    PANEL_CLIENTDLL_TOOLS
};

enum paintmode_t
{
    PAINT_UIPANELS = (1 << 0),
    PAINT_INGAMEPANELS = (1 << 1),
    PAINT_CURSOR = (1 << 2),
};

enum FontDrawType_t
{
    // Use the "additive" value from the scheme file
    FONT_DRAW_DEFAULT = 0,

    // Overrides
    FONT_DRAW_NONADDITIVE,
    FONT_DRAW_ADDITIVE,

    FONT_DRAW_TYPE_COUNT = 2,
};

//fazer um macro rapidao ai eu pesquiso sua coisa
typedef unsigned long hFont;

class c_surface {
public:
    void unlock_cursor() {
        memory::find_vfunc<void(__thiscall*)(void*)>(this, 61u)(this);
    }

    void play_sound(const char* name) {
        memory::find_vfunc<void(__thiscall*)(void*, const char*)>(this, 78u)(this, name);
    }

    //ok time for draw calls and shit

    void filled_rect(int x, int y, int w, int h)
    {
        CALL_VFUNC(void, 12, int, int, int, int)(this, x, y, w, h);
    }

    void outlined_rect(int x, int y, int w, int h)
    {
        CALL_VFUNC(void, 14, int, int, int, int)(this, x, y, w, h);
    }

    void line(int x0, int y0, int x1, int y1)
    {
        CALL_VFUNC(void, 15, int, int, int, int)(this, x0, y0, x1, y1);
    }

    void set_text_font(hFont font) {
        CALL_VFUNC(void, 18, hFont)(this, font);
    }

    void set_text_color(int r, int g, int b, int a)
    {
        CALL_VFUNC(void, 19, int, int, int, int)(this, r, g, b, a);
    }

    void set_text_pos(int x, int y)
    {
        CALL_VFUNC(void, 20, int, int)(this, x, y);
    }

    void print_text(const wchar_t* text, int textlen, FontDrawType_t drawtype = FONT_DRAW_DEFAULT) {
        CALL_VFUNC(void, 22, const wchar_t*, int, FontDrawType_t)(this, text, textlen, drawtype);
    }

    void set_texture_file(int id, const char* filename, int hardwareFilter, bool forceReload)
    {
        CALL_VFUNC(void, 30, int, const char*, int, bool)(this, id, filename, hardwareFilter, forceReload);
    }

    void set_texture(int id)
    {
        CALL_VFUNC(void, 32, int)(this, id);
    }

    void textured_rect(int x, int y, int w, int h)
    {
        CALL_VFUNC(void, 34, int, int, int, int)(this, x, y, w, h);
    }

    int create_new_texture_id(bool procedural)
    {
        return CALL_VFUNC(int, 37, bool)(this, procedural);
    }

    void set_cursor_always_visible(bool visible)
    {
        CALL_VFUNC(void, 52, bool)(this, visible);
    }

    bool is_cursor_visible()
    {
        return CALL_VFUNC(bool, 53)(this);
    }
    void lock_cursor()
    {
        CALL_VFUNC(void, 62)(this);
    }

    unsigned long create_font()
    {
        return CALL_VFUNC(unsigned long, 66)(this);
    }

    bool set_font_glyph_set(unsigned long font, const char* windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin = 0, int nRangeMax = 0)
    {
        return CALL_VFUNC(bool, 67, unsigned long, const char*, int, int, int, int, int, int, int)(this, font, windowsFontName, tall, weight, blur, scanlines, flags, nRangeMin, nRangeMax);
    }

    void get_text_size(unsigned long font, const wchar_t* text, int& wide, int& tall)
    {
        CALL_VFUNC(void, 75, unsigned long, const wchar_t*, int&, int&)(this, font, text, wide, tall);
    }

    void outlined_circle(int x, int y, int radius, int segments)
    {
        CALL_VFUNC(void, 99, int, int, int, int)(this, x, y, radius, segments);
    }

    void textured_sub_rect(int x, int y, int w, int h, float texs0, float text0, float texs1, float text1)
    {
        CALL_VFUNC(void, 101, int, int, int, int, float, float, float, float)(this, x, y, w, h, texs0, text0, texs1, text1);
    }

    void set_alpha_multiplier(float alpha)
    {
        CALL_VFUNC(void, 108, float)(this, alpha);
    }

    float get_alpha_multiplier()
    {
        return CALL_VFUNC(float, 109)(this);
    }

    void filled_rect_fade(int x, int y, int w, int h, unsigned int alpha0, unsigned int alpha1, bool bHorizontal)
    {
        CALL_VFUNC(void, 118, int, int, int, int, unsigned int, unsigned int, bool)(this, x, y, w, h, alpha0, alpha1, bHorizontal);
    }

    void set_color(color clr) {
        CALL_VFUNC(void, 13, color)(this, clr);
    }

    void start_drawing() {
        memory::find_pattern("vguimatsurface.dll", "40 53 56 57 48 83 EC ? 48 8B F9 80 3D");
    }
    void finish_drawing() {
        memory::find_pattern("vguimatsurface.dll", "40 53 48 83 EC ? 33 C9");
    }
};

class c_engine_vgui {
public:
    // returns true if the pause menu is visible
    bool is_game_ui_visible() {
        return memory::find_vfunc<bool(__thiscall*)(void*)>(this, 21u)(this);
    }
};

class c_panel {
public:
    const char* get_name(unsigned int panel) {
        return memory::find_vfunc<const char* (__thiscall*)(void*, unsigned int)>(this, 36u)(this, panel);
    }
};

inline c_surface* surface = nullptr;
inline c_engine_vgui* engine_vgui = nullptr;
inline c_panel* panel = nullptr;


#include "../../memory.hpp"
#include "../../math.hpp"
#include "entities.hpp"

// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
// FIXME: does the asm code even exist anymore?
struct cplane_t {
    vector normal;
    float  dist;
    byte   type;     // for fast side tests
    byte   signbits; // signx + (signy<<1) + (signz<<1)
    byte   pad[2];
};

//-----------------------------------------------------------------------------
// A ray...
//-----------------------------------------------------------------------------

struct ray_t {
    vectorAligned m_Start;       // starting point, centered within the extents
    vectorAligned m_Delta;       // direction + length of the ray
    vectorAligned m_StartOffset; // Add this to m_Start to get the actual ray start
    vectorAligned m_Extents;     // Describes an axis aligned box extruded along a ray
    bool          m_IsRay;       // are the extents zero?
    bool          m_IsSwept;     // is delta != 0?

    void          init(const vector& start, const vector& end) {
        m_Delta = end - start;
        m_IsSwept = (m_Delta.length() != 0);
        m_Extents = {};
        m_IsRay = true;
        m_StartOffset = {};
        m_Start = start;
    }

    void init(const vector& start, const vector& end, const vector& mins, const vector& maxs) {
        m_Delta = end - start;
        m_IsSwept = (m_Delta.length() != 0);
        m_Extents = maxs - mins;
        m_Extents *= 0.5f;
        m_IsRay = (m_Extents.length() < 1e-6);
        m_StartOffset = mins + maxs;
        m_StartOffset *= 0.5f;
        {
            // fuck glm
            auto start_temp = (vector)m_StartOffset;
            m_Start = start + start_temp;
        }
        m_StartOffset *= -1.0f;
    }

    vector InvDelta() const {
        vector vecInvDelta;

        for (int iAxis = 0; iAxis < 3; ++iAxis) {
            if (m_Delta[iAxis] != 0.0f)
                vecInvDelta[iAxis] = 1.0f / m_Delta[iAxis];

            else
                vecInvDelta[iAxis] = FLT_MAX;
        }

        return vecInvDelta;
    }
};

//=============================================================================
// Base Trace Structure
// - shared between engine/game dlls and tools (vrad)
//=============================================================================

class c_base_trace {
public:
    // Displacement flags tests.
    bool           IsDispSurface(void) { return ((dispFlags & DISPSURF_FLAG_SURFACE) != 0); }
    bool           IsDispSurfaceWalkable(void) { return ((dispFlags & DISPSURF_FLAG_WALKABLE) != 0); }
    bool           IsDispSurfaceBuildable(void) { return ((dispFlags & DISPSURF_FLAG_BUILDABLE) != 0); }
    bool           IsDispSurfaceProp1(void) { return ((dispFlags & DISPSURF_FLAG_SURFPROP1) != 0); }
    bool           IsDispSurfaceProp2(void) { return ((dispFlags & DISPSURF_FLAG_SURFPROP2) != 0); }

    // these members are aligned!!
    vector         startpos; // start position
    vector         endpos;   // final position
    cplane_t       plane;    // surface normal at impact

    float          fraction; // time completed, 1.0 = didn't hit anything

    int            contents;  // contents on other side of surface hit
    unsigned short dispFlags; // displacement flags for marking surfaces with data

    bool           allsolid;   // if true, plane is not valid
    bool           startsolid; // if true, the initial point was in a solid area

    c_base_trace() {}
};

struct surface_t {
    const char* name;
    short          surfaceProps;
    unsigned short flags;      // BUGBUG: These are declared per surface, not per material, but this database is
    // per-material now
    bool           allsolid;   // if true, plane is not valid
    bool           startsolid; // if true, the initial point was in a solid area
};

class c_entity;
class i_client_entity;
class c_base_trace;

//-----------------------------------------------------------------------------
// Purpose: A trace is returned when a box is swept through the world
// NOTE: eventually more of this class should be moved up into the base class!!
//-----------------------------------------------------------------------------
class c_game_trace : public c_base_trace {
public:
    // Returns true if hEnt points at the world entity.
    // If this returns true, then you can't use GetHitBoxIndex().
    bool      did_hit_world() const { return ent == nullptr; }

    // Returns true if we hit something and it wasn't the world.
    bool      did_hit_non_world_entity() const { return ent != nullptr && !did_hit_world(); }

    // Gets the entity's network index if the trace has hit an entity.
    // If not, returns -1.
    // WARNING: NOT AN ACTUAL FUNCTION
    int       get_entity_index() const { return 0; }

    // Returns true if there was any kind of impact at all
    bool      did_hit() const { return fraction < 1 || allsolid || startsolid; }

    float     fractionleftsolid; // time we left a solid, only valid if we started in solid
    surface_t surface;           // surface hit (impact surface)

    int       hitgroup;    // 0 == generic, non-zero is specific body part
    short     physicsbone; // physics bone hit by trace in studio

    tf_player* ent; //tigrinho aposta nele vai, daqui a pouco ta zerado kkkkkkkkkkk
    int       hitbox;
};

//-----------------------------------------------------------------------------
// The standard trace filter... NOTE: Most normal traces inherit from CTraceFilter!!!
//-----------------------------------------------------------------------------
enum trace_type {
    TRACE_EVERYTHING = 0,
    TRACE_WORLD_ONLY,              // NOTE: This does *not* test static props!!!
    TRACE_ENTITIES_ONLY,           // NOTE: This version will *not* test static props
    TRACE_EVERYTHING_FILTER_PROPS, // NOTE: This version will pass the IHandleEntity for props through the
    // filter, unlike all other filters
};

class i_trace_filter {
public:
    virtual bool       should_hit_entity(void* pEntity, int contentsMask) = 0;
    virtual trace_type get_trace_type() const = 0;
};

//-----------------------------------------------------------------------------
// Classes are expected to inherit these + implement the ShouldHitEntity method
//-----------------------------------------------------------------------------

// This is the one most normal traces will inherit from
class c_trace_filter : public i_trace_filter {
public:
    virtual trace_type get_trace_type() const { return TRACE_EVERYTHING; }
};

class c_trace_filter_entities_only : public i_trace_filter {
public:
    virtual trace_type get_trace_type() const { return TRACE_ENTITIES_ONLY; }
};

//-----------------------------------------------------------------------------
// Classes need not inherit from these
//-----------------------------------------------------------------------------
class c_trace_filter_world_only : public i_trace_filter {
public:
    bool               should_hit_entity(void* pServerEntity, int contentsMask) { return false; }
    virtual trace_type get_trace_type() const { return TRACE_WORLD_ONLY; }
};

class c_trace_filter_world_and_props_only : public i_trace_filter {
public:
    bool               should_hit_entity(void* pServerEntity, int contentsMask) { return false; }
    virtual trace_type get_trace_type() const { return TRACE_EVERYTHING; }
};

class c_trace_filter_hit_all : public c_trace_filter {
public:
    virtual bool should_hit_entity(void* pServerEntity, int contentsMask) { return true; }
};

class c_trace_filter_hitscan : public c_trace_filter {
public:
    bool       should_hit_entity(void* ent, int contents) override { return true; }

    virtual trace_type get_trace_type() const override { return TRACE_EVERYTHING; }

    i_client_entity* skip = nullptr;
};

class i_engine_trace {
public:
    // A version that simply accepts a ray (can work as a traceline or tracehull)
    void trace_ray(const ray_t& ray, unsigned int mask, i_trace_filter* filter, c_game_trace* trace) {
        memory::find_vfunc<void(__thiscall*)(void*, const ray_t&, unsigned int, i_trace_filter*, c_game_trace*)>(
            this, 4u)(this, ray, mask, filter, trace);
    }
};


inline i_engine_trace* engine_trace = nullptr;


#include "../../math.hpp"
class VMatrix {
public:
    // The matrix.
    matrix4x4 m[4][4];
};

//-----------------------------------------------------------------------------
// Flags passed in with view setup
//-----------------------------------------------------------------------------
enum ClearFlags_t {
    VIEW_CLEAR_COLOR = 0x1,
    VIEW_CLEAR_DEPTH = 0x2,
    VIEW_CLEAR_FULL_TARGET = 0x4,
    VIEW_NO_DRAW = 0x8,
    VIEW_CLEAR_OBEY_STENCIL = 0x10, // Draws a quad allowing stencil test to clear through portals
    VIEW_CLEAR_STENCIL = 0x20,
};

enum StereoEye_t {
    STEREO_EYE_MONO = 0,
    STEREO_EYE_LEFT = 1,
    STEREO_EYE_RIGHT = 2,
    STEREO_EYE_MAX = 3,
};

//-----------------------------------------------------------------------------
// Purpose: Renderer setup data.
//-----------------------------------------------------------------------------
class c_view_setup {
public:
    c_view_setup() {
        m_flAspectRatio = 0.0f;
        m_bRenderToSubrectOfLargerScreen = false;
        m_bDoBloomAndToneMapping = true;
        m_bOrtho = false;
        m_bOffCenter = false;
        m_bCacheFullSceneState = false;
        //		m_bUseExplicitViewVector = false;
        m_bViewToProjectionOverride = false;
        m_eStereoEye = STEREO_EYE_MONO;
    }

    // shared by 2D & 3D views

    // left side of view window
    int         x;
    int         m_nUnscaledX;
    // top side of view window
    int         y;
    int         m_nUnscaledY;
    // width of view window
    int         width;
    int         m_nUnscaledWidth;
    // height of view window
    int         height;
    // which eye are we rendering?
    StereoEye_t m_eStereoEye;
    int         m_nUnscaledHeight;

    // the rest are only used by 3D views

    // Orthographic projection?
    bool        m_bOrtho;
    // View-space rectangle for ortho projection.
    float       m_OrthoLeft;
    float       m_OrthoTop;
    float       m_OrthoRight;
    float       m_OrthoBottom;

    // horizontal FOV in degrees
    float       fov;
    // horizontal FOV in degrees for in-view model
    float       fovViewmodel;

    // 3D origin of camera
    vector      origin;

    // heading of camera (pitch, yaw, roll)
    QAngle      angles;
    // local Z coordinate of near plane of camera
    float       zNear;
    // local Z coordinate of far plane of camera
    float       zFar;

    // local Z coordinate of near plane of camera ( when rendering view model )
    float       zNearViewmodel;
    // local Z coordinate of far plane of camera ( when rendering view model )
    float       zFarViewmodel;

    // set to true if this is to draw into a subrect of the larger screen
    // this really is a hack, but no more than the rest of the way this class is used
    bool        m_bRenderToSubrectOfLargerScreen;

    // The aspect ratio to use for computing the perspective projection matrix
    // (0.0f means use the viewport)
    float       m_flAspectRatio;

    // Controls for off-center projection (needed for poster rendering)
    bool        m_bOffCenter;
    float       m_flOffCenterTop;
    float       m_flOffCenterBottom;
    float       m_flOffCenterLeft;
    float       m_flOffCenterRight;

    // Control that the SFM needs to tell the engine not to do certain post-processing steps
    bool        m_bDoBloomAndToneMapping;

    // Cached mode for certain full-scene per-frame varying state such as sun entity coverage
    bool        m_bCacheFullSceneState;

    // If using VR, the headset calibration will feed you a projection matrix per-eye.
    // This does NOT override the Z range - that will be set up as normal (i.e. the values in this matrix will
    // be ignored).
    bool        m_bViewToProjectionOverride;
    VMatrix     m_ViewToProjection;

    const c_view_setup* get_view_setup();
};


inline c_view_setup* view_render = nullptr;
