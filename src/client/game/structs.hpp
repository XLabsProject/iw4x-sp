#pragma once

#pragma warning(push)
#pragma warning(disable : 4324)

namespace game {
typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];

struct scr_entref_t {
  unsigned __int16 entnum;
  unsigned __int16 classnum;
};

typedef void(__cdecl* BuiltinMethod)(scr_entref_t);

typedef void(__cdecl* BuiltinFunction)();

struct BuiltinMethodDef {
  const char* actionString;
  void(__cdecl* actionFunc)(scr_entref_t);
  int type;
};

struct BuiltinFunctionDef {
  const char* actionString;
  void(__cdecl* actionFunc)();
  int type;
};

enum {
  CON_CHANNEL_DONT_FILTER = 0x0,
  CON_CHANNEL_ERROR = 0x1,
  CON_CHANNEL_GAMENOTIFY = 0x2,
  CON_CHANNEL_BOLDGAME = 0x3,
  CON_CHANNEL_SUBTITLE = 0x4,
  CON_CHANNEL_OBITUARY = 0x5,
  CON_CHANNEL_LOGFILEONLY = 0x6,
  CON_CHANNEL_CONSOLEONLY = 0x7,
  CON_CHANNEL_GFX = 0x8,
  CON_CHANNEL_SOUND = 0x9,
  CON_CHANNEL_FILES = 0xA,
  CON_CHANNEL_DEVGUI = 0xB,
  CON_CHANNEL_PROFILE = 0xC,
  CON_CHANNEL_UI = 0xD,
  CON_CHANNEL_CLIENT = 0xE,
  CON_CHANNEL_SERVER = 0xF,
  CON_CHANNEL_SYSTEM = 0x10,
  CON_CHANNEL_PLAYERWEAP = 0x11,
  CON_CHANNEL_AI = 0x12,
  CON_CHANNEL_ANIM = 0x13,
  CON_CHANNEL_PHYS = 0x14,
  CON_CHANNEL_FX = 0x15,
  CON_CHANNEL_LEADERBOARDS = 0x16,
  CON_CHANNEL_PARSERSCRIPT = 0x17,
  CON_CHANNEL_SCRIPT = 0x18,
  CON_CHANNEL_NETWORK = 0x19,

  CON_BUILTIN_CHANNEL_COUNT = 0x1A,
};

enum XAssetType {
  ASSET_TYPE_PHYSPRESET = 0,
  ASSET_TYPE_PHYSCOLLMAP = 1,
  ASSET_TYPE_XANIMPARTS = 2,
  ASSET_TYPE_XMODEL_SURFS = 3,
  ASSET_TYPE_XMODEL = 4,
  ASSET_TYPE_MATERIAL = 5,
  ASSET_TYPE_PIXELSHADER = 6,
  ASSET_TYPE_VERTEXSHADER = 7,
  ASSET_TYPE_VERTEXDECL = 8,
  ASSET_TYPE_TECHNIQUE_SET = 9,
  ASSET_TYPE_IMAGE = 10,
  ASSET_TYPE_SOUND = 11,
  ASSET_TYPE_SOUND_CURVE = 12,
  ASSET_TYPE_LOADED_SOUND = 13,
  ASSET_TYPE_CLIPMAP_SP = 14,
  ASSET_TYPE_CLIPMAP_MP = 15,
  ASSET_TYPE_COMWORLD = 16,
  ASSET_TYPE_GAMEWORLD_SP = 17,
  ASSET_TYPE_GAMEWORLD_MP = 18,
  ASSET_TYPE_MAP_ENTS = 19,
  ASSET_TYPE_FXWORLD = 20,
  ASSET_TYPE_GFXWORLD = 21,
  ASSET_TYPE_LIGHT_DEF = 22,
  ASSET_TYPE_UI_MAP = 23,
  ASSET_TYPE_FONT = 24,
  ASSET_TYPE_MENULIST = 25,
  ASSET_TYPE_MENU = 26,
  ASSET_TYPE_LOCALIZE_ENTRY = 27,
  ASSET_TYPE_WEAPON = 28,
  ASSET_TYPE_SNDDRIVER_GLOBALS = 29,
  ASSET_TYPE_FX = 30,
  ASSET_TYPE_IMPACT_FX = 31,
  ASSET_TYPE_AITYPE = 32,
  ASSET_TYPE_MPTYPE = 33,
  ASSET_TYPE_CHARACTER = 34,
  ASSET_TYPE_XMODELALIAS = 35,
  ASSET_TYPE_RAWFILE = 36,
  ASSET_TYPE_STRINGTABLE = 37,
  ASSET_TYPE_LEADERBOARD = 38,
  ASSET_TYPE_STRUCTURED_DATA_DEF = 39,
  ASSET_TYPE_TRACER = 40,
  ASSET_TYPE_VEHICLE = 41,
  ASSET_TYPE_ADDON_MAP_ENTS = 42,

  ASSET_TYPE_COUNT = 43,
};

enum FsThread {
  FS_THREAD_MAIN = 0x0,
  FS_THREAD_STREAM = 0x1,
  FS_THREAD_DATABASE = 0x2,
  FS_THREAD_BACKEND = 0x3,
  FS_THREAD_SERVER = 0x4,
  FS_THREAD_COUNT = 0x5,
  FS_THREAD_INVALID = 0x6,
};

enum FsListBehavior_e {
  FS_LIST_PURE_ONLY = 0x0,
  FS_LIST_ALL = 0x1,
};

enum fsMode_t {
  FS_READ = 0x0,
  FS_WRITE = 0x1,
  FS_APPEND = 0x2,
  FS_APPEND_SYNC = 0x3,
};

struct token_s {
  char string[1024];
  int type;
  int subtype;
  unsigned int intvalue;
  long double floatvalue;
  char* whitespace_p;
  char* endwhitespace_p;
  int line;
  int linescrossed;
  token_s* next;
};

static_assert(sizeof(token_s) == 0x430);

struct define_s {
  char* name;
  int flags;
  int builtin;
  int numparms;
  token_s* parms;
  token_s* tokens;
  define_s* next;
  define_s* hashnext;
};

struct punctuation_s {
  const char* p;
  int n;
  punctuation_s* next;
};

struct script_s {
  char filename[64];
  char* buffer;
  char* script_p;
  char* end_p;
  char* lastscript_p;
  char* whitespace_p;
  char* endwhitespace_p;
  int length;
  int line;
  int lastline;
  int tokenavailable;
  int flags;
  punctuation_s* punctuations;
  punctuation_s** punctuationtable;
  token_s token;
  script_s* next;
};

enum parseSkip_t {
  SKIP_NO = 0x0,
  SKIP_YES = 0x1,
  SKIP_ALL_ELIFS = 0x2,
};

struct indent_s {
  int type;
  parseSkip_t skip;
  script_s* script;
  indent_s* next;
};

struct source_s {
  char filename[64];
  char includepath[64];
  punctuation_s* punctuations;
  script_s* scriptstack;
  token_s* tokens;
  define_s* defines;
  define_s** definehash;
  indent_s* indentstack;
  int skip;
  token_s token;
};

struct pc_token_s {
  int type;
  int subtype;
  int intvalue;
  float floatvalue;
  char string[1024];
};

struct cmd_function_s {
  cmd_function_s* next;
  const char* name;
  const char* autoCompleteDir;
  const char* autoCompleteExt;
  void(__cdecl* function)();
  int flags;
};

struct CmdArgs {
  int nesting;
  int localClientNum[8];
  int controllerIndex[8];
  int argc[8];
  const char** argv[8];
};

struct msg_t {
  int overflowed;
  int readOnly;
  unsigned char* data;
  unsigned char* splitData;
  int maxsize;
  int cursize;
  int splitSize;
  int readcount;
  int bit;
  int lastEntityRef;
};

static_assert(sizeof(msg_t) == 0x28);

typedef enum {
  ERR_FATAL = 0x0,
  ERR_DROP = 0x1,
  ERR_SERVERDISCONNECT = 0x2,
  ERR_DISCONNECT = 0x3,
  ERR_SCRIPT = 0x4,
  ERR_SCRIPT_DROP = 0x5,
  ERR_LOCALIZATION = 0x6,
  ERR_MAPLOADERRORSUMMARY = 0x7,
} errorParm_t;

enum DvarFlags : std::uint16_t {
  DVAR_NONE = 0,
  DVAR_ARCHIVE = 1 << 0,
  DVAR_LATCH = 1 << 1,
  DVAR_CHEAT = 1 << 2,
  DVAR_CODINFO = 1 << 3,
  DVAR_SCRIPTINFO = 1 << 4,
  DVAR_UNK1 = 1 << 7,
  DVAR_USERINFO = 1 << 9,
  DVAR_SERVERINFO = 1 << 10,
  DVAR_INIT = 1 << 11,
  DVAR_ROM = 1 << 13,
  DVAR_CHANGEABLE_RESET = 1 << 14,
  DVAR_AUTOEXEC = 1 << 15,
};

enum : std::int8_t {
  DVAR_TYPE_BOOL = 0x0,
  DVAR_TYPE_FLOAT = 0x1,
  DVAR_TYPE_FLOAT_2 = 0x2,
  DVAR_TYPE_FLOAT_3 = 0x3,
  DVAR_TYPE_FLOAT_4 = 0x4,
  DVAR_TYPE_INT = 0x5,
  DVAR_TYPE_ENUM = 0x6,
  DVAR_TYPE_STRING = 0x7,
  DVAR_TYPE_COLOR = 0x8,
  DVAR_TYPE_FLOAT_3_COLOR = 0x9,
};

enum DvarSetSource {
  DVAR_SOURCE_INTERNAL,
  DVAR_SOURCE_EXTERNAL,
  DVAR_SOURCE_SCRIPT,
  DVAR_SOURCE_DEVGUI,
};

union DvarValue {
  bool enabled;
  int integer;
  unsigned int unsignedInt;
  float value;
  float vector[4];
  const char* string;
  unsigned char color[4];
};

struct enum_limit {
  int stringCount;
  const char** strings;
};

struct int_limit {
  int min;
  int max;
};

struct float_limit {
  float min;
  float max;
};

union DvarLimits {
  enum_limit enumeration;
  int_limit integer;
  float_limit value;
  float_limit vector;
};

struct dvar_t {
  const char* name;
  const char* description;
  unsigned int flags;
  char type;
  bool modified;
  DvarValue current;
  DvarValue latched;
  DvarValue reset;
  DvarLimits domain;
  bool(__cdecl* domainFunc)(dvar_t*, DvarValue);
  dvar_t* hashNext;
};

enum netsrc_t {
  NS_CLIENT1 = 0x0,
  NS_SERVER = 0x1,
  NS_MAXCLIENTS = 0x1,
  NS_PACKET = 0x2,
};

enum netadrtype_t {
  NA_BOT = 0x0,
  NA_BAD = 0x1,
  NA_LOOPBACK = 0x2,
  NA_BROADCAST = 0x3,
  NA_IP = 0x4,
  NA_IPX = 0x5,
  NA_BROADCAST_IPX = 0x6,
};

struct netadr_t {
  netadrtype_t type;
  unsigned char ip[4];
  unsigned __int16 port;
  char ipx[10];
};

static_assert(sizeof(netadr_t) == 0x14);

struct MapEnts {
  const char* name;
  char* entityString;
  int numEntityChars;
}; // Incomplete

struct WeaponCompleteDef {
  const char* szInternalName;
}; // Incomplete

struct Font_s {
  const char* fontName;
  int pixelHeight;
  int glyphCount;
  void* material;
  void* glowMaterial;
  void* glyphs;
};

struct RawFile {
  const char* name;
  int compressedLen;
  int len;
  const char* buffer;
};

struct G_GlassPiece {
  unsigned __int16 damageTaken;
  unsigned __int16 collapseTime;
  int lastStateChangeTime;
  char impactDir;
  char impactPos[2];
};

static_assert(sizeof(G_GlassPiece) == 0xC);

struct G_GlassName {
  char* nameStr;
  unsigned __int16 name;
  unsigned __int16 pieceCount;
  unsigned __int16* pieceIndices;
};

static_assert(sizeof(G_GlassName) == 0xC);

struct G_GlassData {
  G_GlassPiece* glassPieces;
  unsigned int pieceCount;
  unsigned __int16 damageToWeaken;
  unsigned __int16 damageToDestroy;
  unsigned int glassNameCount;
  G_GlassName* glassNames;
  unsigned __int8 pad[108];
};

static_assert(sizeof(G_GlassData) == 0x80);

struct GameWorldMp {
  const char* name;
  G_GlassData* g_glassData;
};

struct GameWorldSp {
  const char* name;
  unsigned char __pad0[0x30];
  G_GlassData* g_glassData;
};

static_assert(sizeof(GameWorldSp) == 0x38);

struct MenuList {
  const char* name;
  int menuCount;
  void** menus;
};

union XAssetHeader {
  void* data;
  GameWorldSp* gameWorldSp;
  GameWorldMp* gameWorldMp;
  Font_s* font;
  WeaponCompleteDef* weapon;
  RawFile* rawfile;
  MenuList* menuList;
};

struct XAsset {
  XAssetType type;
  XAssetHeader header;
};

struct XAssetEntry {
  XAsset asset;
  unsigned char zoneIndex;
  volatile unsigned char inuseMask;
  bool printedMissingAsset;
  unsigned __int16 nextHash;
  unsigned __int16 nextOverride;
};

static_assert(sizeof(XAssetEntry) == 0x10);

struct XZoneInfo {
  const char* name;
  int allocFlags;
  int freeFlags;
};

struct playerState_s {
  int commandTime;
  int pm_type;
  int pm_time;
  int pm_flags;
  int otherFlags;
  int linkFlags;
  int bobCycle;
  float origin[3];
  float velocity[3];
  unsigned char __pad0[0xAC70];
};

static_assert(sizeof(playerState_s) == 0xACA4);

struct gclient_s {
  playerState_s ps;
  unsigned char __pad0[0xB0];
  int flags; // 0xAD54
  unsigned char __pad1[0x27C];
};

static_assert(sizeof(gclient_s) == 0xAFD4);

struct entityState_s {
  unsigned char eType; // 0
  unsigned char __pad0[0x7D];
  unsigned __int16 number; // 0x7E
  unsigned char __pad1[0x2C];
};

static_assert(sizeof(entityState_s) == 0xAC);

struct entityShared_t {
  unsigned char __pad0[0x5C];
};

static_assert(sizeof(entityShared_t) == 0x5C);

struct gentity_s {
  entityState_s s;
  entityShared_t r;
  gclient_s* client;
  unsigned char __pad0[0x164];
};

static_assert(sizeof(gentity_s) == 0x270);

struct client_t {
  unsigned char __pad0[0x1674];
};

static_assert(sizeof(client_t) == 0x1674);

struct level_locals_t {
  unsigned char __pad0[0x34];
  int time;
  unsigned char __pad1[0x4748];
};

static_assert(sizeof(level_locals_t) == 0x4780);

struct field_t {
  int cursor;
  int scroll;
  int drawWidth;
  int widthInPixels;
  float charHeight;
  int fixedSize;
  char buffer[256];
};

struct ConDrawInputGlob {
  char autoCompleteChoice[64];
  int matchIndex;
  int matchCount;
  const char* inputText;
  int inputTextLen;
  bool hasExactMatch;
  bool mayAutoComplete;
  float x;
  float y;
  float leftX;
  float fontHeight;
};

struct MessageLine {
  int messageIndex;
  int textBufPos;
  int textBufSize;
  int typingStartTime;
  int lastTypingSoundTime;
  int flags;
};

struct Message {
  int startTime;
  int endTime;
};

struct MessageWindow {
  MessageLine* lines;
  Message* messages;
  char* circularTextBuffer;
  int textBufSize;
  int lineCount;
  int padding;
  int scrollTime;
  int fadeIn;
  int fadeOut;
  int textBufPos;
  int firstLineIndex;
  int activeLineCount;
  int messageIndex;
};

struct MessageBuffer {
  char gamemsgText[4][2048];
  MessageWindow gamemsgWindows[4];
  MessageLine gamemsgLines[4][12];
  Message gamemsgMessages[4][12];
  char miniconText[4096];
  MessageWindow miniconWindow;
  MessageLine miniconLines[100];
  Message miniconMessages[100];
  char errorText[1024];
  MessageWindow errorWindow;
  MessageLine errorLines[5];
  Message errorMessages[5];
};

struct Console {
  MessageWindow consoleWindow;
  MessageLine consoleLines[1024];
  Message consoleMessages[1024];
  char consoleText[65536];
  char textTempLine[512];
  unsigned int lineOffset;
  int displayLineOffset;
  int prevChannel;
  bool outputVisible;
  int fontHeight;
  int visibleLineCount;
  int visiblePixelWidth;
  float screenMin[2];
  float screenMax[2];
  MessageBuffer messageBuffer[1];
  float color[4];
};

enum {
  KEYCATCH_CONSOLE = 0x1,
};

enum ScreenPlacementMode {
  SCRMODE_FULL = 0x0,
  SCRMODE_DISPLAY = 0x1,
  SCRMODE_INVALID = 0x2,
  SCRMODE_COUNT = 0x3,
};

struct ScreenPlacement {
  float scaleVirtualToReal[2];
  float scaleVirtualToFull[2];
  float scaleRealToVirtual[2];
  float realViewportPosition[2];
  float realViewportSize[2];
  float virtualViewableMin[2];
  float virtualViewableMax[2];
  float realViewableMin[2];
  float realViewableMax[2];
  float virtualAdjustableMin[2];
  float virtualAdjustableMax[2];
  float realAdjustableMin[2];
  float realAdjustableMax[2];
  float subScreenLeft;
};

enum usercmdButtonBits {
  CMD_BUTTON_LEAN_LEFT = 0x40,
  CMD_BUTTON_LEAN_RIGHT = 0x80,
};

struct kbutton_t {
  int down[2];
  unsigned int downtime;
  unsigned int msec;
  bool active;
  bool wasPressed;
};

struct usercmd_s {
  int serverTime;
  int buttons;
  int angles[3];
  unsigned __int16 weapon;
  unsigned __int16 primaryWeaponForAltMode;
  unsigned __int16 offHandIndex;
  char forwardmove;
  char rightmove;
  unsigned char upmove;
  unsigned char downmove;
  char pitchmove;
  char yawmove;
  float gunPitch;
  float gunYaw;
  float gunXOfs;
  float gunYOfs;
  float gunZOfs;
  float meleeChargeYaw;
  unsigned char meleeChargeDist;
  char selectedLoc[2];
  unsigned char selectedLocAngle;
  char remoteControlAngles[2];
};

struct weaponParms {
  float forward[3];
  float right[3];
  float up[3];
  float muzzleTrace[3];
  float gunForward[3];
  unsigned int weaponIndex;
  const void* weapDef;
  const void* weapCompleteDef;
};

struct lockonFireParms {
  bool lockon;
  gentity_s* target;
  float targetPosOrOffset[3];
  bool topFire;
};

struct Bounds {
  float midPoint[3];
  float halfSize[3];
};

enum TraceHitType {
  TRACE_HITTYPE_NONE = 0x0,
  TRACE_HITTYPE_ENTITY = 0x1,
  TRACE_HITTYPE_DYNENT_MODEL = 0x2,
  TRACE_HITTYPE_DYNENT_BRUSH = 0x3,
  TRACE_HITTYPE_GLASS = 0x4,
};

struct trace_t {
  float fraction;
  float normal[3];
  int surfaceFlags;
  int contents;
  const char* material;
  TraceHitType hitType;
  unsigned __int16 hitId;
  unsigned __int16 modelIndex;
  unsigned __int16 partName;
  unsigned __int16 partGroup;
  bool allsolid;
  bool startsolid;
  bool walkable;
};

static_assert(sizeof(trace_t) == 0x2C);

struct pml_t {
  float forward[3];
  float right[3];
  float up[3];
  float frametime;
  int msec;
  int walking;
  int groundPlane;
  trace_t groundTrace;
  float impactSpeed;
  float previous_origin[3];
  float previous_velocity[3];
  unsigned int holdrand;
};

static_assert(sizeof(pml_t) == 0x80);

struct pmove_t {
  playerState_s* ps;
  usercmd_s cmd;
  usercmd_s oldcmd;
  int tracemask;
  int numtouch;
  int touchents[32];
  Bounds bounds;
  float xyspeed;
  int proneChange;
  int viewChangeTime;
  float viewChange;
  float fTorsoPitch;
  float fWaistPitch;
  unsigned char handler;
};

static_assert(sizeof(pmove_t) == 0x140);

enum ScopedCriticalSectionType {
  SCOPED_CRITSECT_NORMAL = 0x0,
  SCOPED_CRITSECT_DISABLED = 0x1,
  SCOPED_CRITSECT_RELEASE = 0x2,
  SCOPED_CRITSECT_TRY = 0x3,
};

enum CriticalSection {
  CRITSECT_CONSOLE = 0x0,
  CRITSECT_LSP = 0x1A,
  CRITSECT_CBUF = 0x21,
  CRITSECT_COUNT = 0x28,
};

struct TempPriority {
  void* threadHandle;
  int oldPriority;
};

struct FastCriticalSection {
  volatile long readCount;
  volatile long writeCount;
  TempPriority tempPriority;
};
} // namespace game

#pragma warning(pop)
