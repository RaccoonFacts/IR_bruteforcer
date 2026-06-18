#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/icon.h>
#include <input/input.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <storage/storage.h>
#include <lib/infrared/encoder_decoder/infrared.h>
#include <lib/infrared/worker/infrared_transmit.h>
#include <string.h>
#include <stdio.h>

extern const Icon I_arrow_updown_11x11;
extern const Icon I_btn_ok_11x11;
extern const Icon I_btn_right_6x11;

// ── Timing ────────────────────────────────────────────────────────────────────
#define SEND_DELAY_MS     300
#define POST_TX_DELAY_MS  100
#define LOG_PATH          APP_DATA_PATH("ir_hits.txt")
#define RECENT_MAX        5

// ── Scan lists ────────────────────────────────────────────────────────────────
typedef struct { uint8_t addr; uint8_t cmd; } Pair;

static const Pair TV_LIST[] = {
    {0x04,0x08},{0x04,0x0B},{0x04,0x02},{0x04,0x01},{0x04,0x00},{0x04,0x10},{0x04,0x11},{0x04,0x12},
    {0x04,0x13},{0x04,0x14},{0x04,0x15},{0x04,0x16},{0x04,0x17},{0x04,0x18},{0x04,0x19},{0x04,0x1A},
    {0x04,0x1B},{0x04,0x1C},{0x04,0x1D},{0x04,0x1E},{0x04,0x1F},{0x04,0x20},{0x04,0x21},{0x04,0x22},
    {0x07,0x02},{0x07,0x01},{0x07,0x08},{0x07,0x22},{0x07,0x00},{0x07,0x10},{0x07,0x11},{0x07,0x12},
    {0x07,0x13},{0x07,0x14},{0x07,0x15},{0x07,0x16},{0x07,0x17},{0x07,0x18},{0x07,0x19},{0x07,0x1A},
    {0x10,0x02},{0x10,0x01},{0x10,0x08},{0x10,0x40},{0x10,0x00},{0x10,0x10},{0x10,0x11},{0x10,0x12},
    {0x10,0x13},{0x10,0x14},{0x10,0x15},{0x10,0x16},{0x10,0x17},{0x10,0x18},{0x10,0x19},{0x10,0x1A},
    {0x40,0x08},{0x40,0x0C},{0x40,0x02},{0x40,0x01},{0x40,0x00},{0x40,0x10},{0x40,0x11},{0x40,0x12},
    {0x40,0x13},{0x40,0x14},{0x40,0x15},{0x40,0x16},{0x40,0x17},{0x40,0x18},{0x40,0x19},{0x40,0x1A},
    {0x00,0x08},{0x00,0x02},{0x00,0x01},{0x00,0x40},{0x00,0x00},{0x00,0x10},{0x00,0x11},{0x00,0x12},
    {0x00,0x13},{0x00,0x14},{0x00,0x15},{0x00,0x16},{0x00,0x17},{0x00,0x18},{0x00,0x19},{0x00,0x1A},
    {0x20,0x02},{0x20,0x01},{0x20,0x08},{0x20,0x00},{0x20,0x10},{0x20,0x11},{0x20,0x40},{0x20,0x12},
    {0x20,0x13},{0x20,0x14},{0x20,0x15},{0x20,0x16},{0x20,0x17},{0x20,0x18},{0x20,0x19},{0x20,0x1A},
    {0x08,0x02},{0x08,0x01},{0x08,0x08},{0x08,0x00},{0x08,0x10},{0x08,0x40},{0x08,0x11},{0x08,0x12},
    {0x08,0x13},{0x08,0x14},{0x08,0x15},{0x08,0x16},{0x08,0x17},{0x08,0x18},{0x08,0x19},{0x08,0x1A},
    {0x01,0x08},{0x01,0x02},{0x01,0x01},{0x01,0x00},{0x01,0x10},{0x01,0x40},{0x01,0x11},{0x01,0x12},
    {0x01,0x13},{0x01,0x14},{0x01,0x15},{0x01,0x16},{0x01,0x17},{0x01,0x18},{0x01,0x19},{0x01,0x1A},
    {0xA8,0x08},{0xA8,0x02},{0xA8,0x01},{0xA8,0x00},{0xA8,0x10},{0xA8,0x11},{0xA8,0x12},{0xA8,0x40},
    {0xA8,0x13},{0xA8,0x14},{0xA8,0x15},{0xA8,0x16},{0xA8,0x17},{0xA8,0x18},{0xA8,0x19},{0xA8,0x1A},
    {0xE0,0x08},{0xE0,0x02},{0xE0,0x01},{0xE0,0x00},{0xE0,0x10},{0xE0,0x11},{0xE0,0x12},{0xE0,0x40},
    {0xE0,0x13},{0xE0,0x14},{0xE0,0x15},{0xE0,0x16},{0xE0,0x17},{0xE0,0x18},{0xE0,0x19},{0xE0,0x1A},
    {0x18,0x08},{0x18,0x02},{0x18,0x01},{0x18,0x00},{0x18,0x10},{0x18,0x11},{0x18,0x12},{0x18,0x40},
    {0x18,0x13},{0x18,0x14},{0x18,0x15},{0x18,0x16},{0x18,0x17},{0x18,0x18},{0x18,0x19},{0x18,0x1A},
    {0x50,0x08},{0x50,0x02},{0x50,0x01},{0x50,0x00},{0x50,0x10},{0x50,0x11},{0x50,0x12},{0x50,0x40},
    {0x50,0x13},{0x50,0x14},{0x50,0x15},{0x50,0x16},{0x50,0x17},{0x50,0x18},{0x50,0x19},{0x50,0x1A},
    {0x28,0x08},{0x28,0x02},{0x28,0x01},{0x28,0x00},{0x28,0x10},{0x28,0x11},{0x28,0x12},{0x28,0x40},
    {0x28,0x13},{0x28,0x14},{0x28,0x15},{0x28,0x16},{0x28,0x17},{0x28,0x18},{0x28,0x19},{0x28,0x1A},
    {0x30,0x08},{0x30,0x02},{0x30,0x01},{0x30,0x00},{0x30,0x10},{0x30,0x11},{0x30,0x12},{0x30,0x40},
    {0x30,0x13},{0x30,0x14},{0x30,0x15},{0x30,0x16},{0x30,0x17},{0x30,0x18},{0x30,0x19},{0x30,0x1A},
    {0x58,0x08},{0x58,0x02},{0x58,0x01},{0x58,0x00},{0x58,0x10},{0x58,0x11},{0x58,0x12},{0x58,0x40},
    {0x60,0x08},{0x60,0x02},{0x60,0x01},{0x60,0x00},{0x60,0x10},{0x60,0x11},{0x60,0x12},{0x60,0x40},
};
#define TV_COUNT (sizeof(TV_LIST)/sizeof(TV_LIST[0]))

static const Pair LED_LIST[] = {
    {0x00,0x00},{0x00,0x01},{0x00,0x02},{0x00,0x03},{0x00,0x04},{0x00,0x05},{0x00,0x06},{0x00,0x07},
    {0x00,0x08},{0x00,0x09},{0x00,0x0A},{0x00,0x0B},{0x00,0x0C},{0x00,0x0D},{0x00,0x0E},{0x00,0x0F},
    {0x00,0x10},{0x00,0x11},{0x00,0x12},{0x00,0x13},{0x00,0x14},{0x00,0x15},{0x00,0x16},{0x00,0x17},
    {0x00,0x18},{0x00,0x19},{0x00,0x1A},{0x00,0x1B},{0x00,0x1C},{0x00,0x1D},{0x00,0x1E},{0x00,0x1F},
    {0xFF,0x00},{0xFF,0x01},{0xFF,0x02},{0xFF,0x03},{0xFF,0x04},{0xFF,0x05},{0xFF,0x06},{0xFF,0x07},
    {0xFF,0x08},{0xFF,0x09},{0xFF,0x0A},{0xFF,0x0B},{0xFF,0x0C},{0xFF,0x0D},{0xFF,0x0E},{0xFF,0x0F},
    {0xFF,0x10},{0xFF,0x11},{0xFF,0x12},{0xFF,0x13},{0xFF,0x14},{0xFF,0x15},{0xFF,0x16},{0xFF,0x17},
    {0xEF,0x00},{0xEF,0x01},{0xEF,0x02},{0xEF,0x03},{0xEF,0x04},{0xEF,0x05},{0xEF,0x06},{0xEF,0x07},
    {0xEF,0x08},{0xEF,0x09},{0xEF,0x0A},{0xEF,0x0B},{0xEF,0x0C},{0xEF,0x0D},{0xEF,0x0E},{0xEF,0x0F},
    {0x44,0x00},{0x44,0x01},{0x44,0x02},{0x44,0x03},{0x44,0x04},{0x44,0x05},{0x44,0x06},{0x44,0x07},
    {0x44,0x08},{0x44,0x09},{0x44,0x0A},{0x44,0x0B},{0x44,0x0C},{0x44,0x0D},{0x44,0x0E},{0x44,0x0F},
    {0x74,0x00},{0x74,0x01},{0x74,0x02},{0x74,0x03},{0x74,0x04},{0x74,0x05},{0x74,0x06},{0x74,0x07},
    {0x74,0x08},{0x74,0x09},{0x74,0x0A},{0x74,0x0B},{0x74,0x0C},{0x74,0x0D},{0x74,0x0E},{0x74,0x0F},
};
#define LED_COUNT (sizeof(LED_LIST)/sizeof(LED_LIST[0]))

static const Pair TOY_LIST[] = {
    {0x00,0x01},{0x00,0x02},{0x00,0x04},{0x00,0x08},{0x00,0x10},{0x00,0x20},{0x00,0x40},{0x00,0x80},
    {0x00,0x03},{0x00,0x05},{0x00,0x06},{0x00,0x09},{0x00,0x0A},{0x00,0x0C},{0x00,0x11},{0x00,0x12},
    {0x00,0x18},{0x00,0x24},{0x00,0x30},{0x00,0x48},{0x00,0x60},{0x00,0x90},{0x00,0xC0},{0x00,0xFF},
    {0x01,0x01},{0x01,0x02},{0x01,0x04},{0x01,0x08},{0x01,0x10},{0x01,0x20},{0x01,0x40},{0x01,0x80},
    {0x01,0x03},{0x01,0x05},{0x01,0x06},{0x01,0x09},{0x01,0x0A},{0x01,0x0C},{0x01,0x11},{0x01,0x12},
    {0x02,0x01},{0x02,0x02},{0x02,0x04},{0x02,0x08},{0x02,0x10},{0x02,0x20},{0x02,0x40},{0x02,0x80},
    {0x02,0x03},{0x02,0x05},{0x02,0x06},{0x02,0x09},{0x02,0x0A},{0x02,0x0C},{0x02,0x11},{0x02,0x12},
    {0x03,0x01},{0x03,0x02},{0x03,0x04},{0x03,0x08},{0x03,0x10},{0x03,0x20},{0x03,0x40},{0x03,0x80},
    {0x04,0x01},{0x04,0x02},{0x04,0x04},{0x04,0x08},{0x04,0x10},{0x04,0x20},{0x04,0x40},{0x04,0x80},
    {0xAA,0x01},{0xAA,0x02},{0xAA,0x03},{0xAA,0x04},{0xAA,0x05},{0xAA,0x06},{0xAA,0x08},{0xAA,0x10},
    {0xAA,0x20},{0xAA,0x40},{0xAA,0x80},{0xAA,0x11},{0xAA,0x22},{0xAA,0x44},{0xAA,0x88},{0xAA,0xFF},
    {0x55,0x01},{0x55,0x02},{0x55,0x03},{0x55,0x04},{0x55,0x05},{0x55,0x06},{0x55,0x08},{0x55,0x10},
    {0x55,0x20},{0x55,0x40},{0x55,0x80},{0x55,0x11},{0x55,0x22},{0x55,0x44},{0x55,0x88},{0x55,0xFF},
    {0xFE,0x01},{0xFE,0x02},{0xFE,0x04},{0xFE,0x08},{0xFE,0x10},{0xFE,0x20},{0xFE,0x40},{0xFE,0x80},
};
#define TOY_COUNT (sizeof(TOY_LIST)/sizeof(TOY_LIST[0]))

static const Pair RC_LIST[] = {
    {0x00,0x01},{0x00,0x02},{0x00,0x03},{0x00,0x04},{0x00,0x05},{0x00,0x06},{0x00,0x07},{0x00,0x08},
    {0x00,0x09},{0x00,0x0A},{0x00,0x0B},{0x00,0x0C},{0x00,0x10},{0x00,0x20},{0x00,0x40},{0x00,0x80},
    {0x08,0x01},{0x08,0x02},{0x08,0x03},{0x08,0x04},{0x08,0x05},{0x08,0x06},{0x08,0x07},{0x08,0x08},
    {0x08,0x09},{0x08,0x0A},{0x08,0x0B},{0x08,0x0C},{0x08,0x10},{0x08,0x20},{0x08,0x40},{0x08,0x80},
    {0x10,0x01},{0x10,0x02},{0x10,0x03},{0x10,0x04},{0x10,0x05},{0x10,0x06},{0x10,0x07},{0x10,0x08},
    {0x10,0x09},{0x10,0x0A},{0x10,0x0B},{0x10,0x0C},{0x10,0x10},{0x10,0x20},{0x10,0x40},{0x10,0x80},
    {0x18,0x01},{0x18,0x02},{0x18,0x03},{0x18,0x04},{0x18,0x05},{0x18,0x06},{0x18,0x07},{0x18,0x08},
    {0x88,0x01},{0x88,0x02},{0x88,0x03},{0x88,0x04},{0x88,0x05},{0x88,0x06},{0x88,0x07},{0x88,0x08},
    {0xCC,0x01},{0xCC,0x02},{0xCC,0x03},{0xCC,0x04},{0xCC,0x05},{0xCC,0x06},{0xCC,0x07},{0xCC,0x08},
    {0x90,0x01},{0x90,0x02},{0x90,0x03},{0x90,0x04},{0x90,0x05},{0x90,0x06},{0x90,0x07},{0x90,0x08},
    {0x98,0x01},{0x98,0x02},{0x98,0x03},{0x98,0x04},{0x98,0x05},{0x98,0x06},{0x98,0x07},{0x98,0x08},
    {0xA0,0x01},{0xA0,0x02},{0xA0,0x03},{0xA0,0x04},{0xA0,0x05},{0xA0,0x06},{0xA0,0x07},{0xA0,0x08},
};
#define RC_COUNT (sizeof(RC_LIST)/sizeof(RC_LIST[0]))

static const Pair STREAM_LIST[] = {
    // Roku (addr 0x01, 0x00, 0x11)
    {0x01,0x15},{0x01,0x14},{0x01,0x12},{0x01,0x13},{0x01,0x16},{0x01,0x17},{0x01,0x0D},{0x01,0x0E},
    {0x01,0x0F},{0x01,0x10},{0x01,0x11},{0x01,0x00},{0x01,0x01},{0x01,0x02},{0x01,0x03},{0x01,0x04},
    {0x01,0x05},{0x01,0x06},{0x01,0x07},{0x01,0x08},{0x01,0x09},{0x01,0x0A},{0x01,0x0B},{0x01,0x0C},
    {0x01,0x18},{0x01,0x19},{0x01,0x1A},{0x01,0x1B},{0x01,0x1C},{0x01,0x1D},{0x01,0x1E},{0x01,0x1F},
    {0x00,0x15},{0x00,0x14},{0x00,0x12},{0x00,0x13},{0x00,0x16},{0x00,0x17},{0x00,0x0D},{0x00,0x0E},
    {0x11,0x15},{0x11,0x14},{0x11,0x12},{0x11,0x13},{0x11,0x16},{0x11,0x17},{0x11,0x0D},{0x11,0x0E},
    // Amazon Fire TV (addr 0x04, 0x05, 0x40)
    {0x04,0x15},{0x04,0x14},{0x04,0x12},{0x04,0x13},{0x04,0x16},{0x04,0x17},{0x04,0x0D},{0x04,0x0E},
    {0x04,0x0F},{0x04,0x10},{0x04,0x1B},{0x04,0x1C},{0x04,0x1D},{0x04,0x1E},{0x04,0x1F},{0x04,0x20},
    {0x05,0x15},{0x05,0x14},{0x05,0x12},{0x05,0x13},{0x05,0x16},{0x05,0x17},{0x05,0x0D},{0x05,0x0E},
    {0x40,0x15},{0x40,0x14},{0x40,0x12},{0x40,0x13},{0x40,0x16},{0x40,0x17},{0x40,0x0D},{0x40,0x0E},
    // Apple TV (addr 0x77, 0x87, 0x0F)
    {0x77,0x15},{0x77,0x14},{0x77,0x12},{0x77,0x13},{0x77,0x16},{0x77,0x17},{0x77,0x0D},{0x77,0x0E},
    {0x77,0x00},{0x77,0x01},{0x77,0x02},{0x77,0x03},{0x77,0x04},{0x77,0x05},{0x77,0x06},{0x77,0x07},
    {0x87,0x15},{0x87,0x14},{0x87,0x12},{0x87,0x13},{0x87,0x16},{0x87,0x17},{0x87,0x0D},{0x87,0x0E},
    {0x0F,0x15},{0x0F,0x14},{0x0F,0x12},{0x0F,0x13},{0x0F,0x16},{0x0F,0x17},{0x0F,0x0D},{0x0F,0x0E},
    // Android TV (addr 0x02, 0x03, 0x06)
    {0x02,0x15},{0x02,0x14},{0x02,0x12},{0x02,0x13},{0x02,0x16},{0x02,0x17},{0x02,0x0D},{0x02,0x0E},
    {0x02,0x00},{0x02,0x01},{0x02,0x02},{0x02,0x03},{0x02,0x04},{0x02,0x05},{0x02,0x06},{0x02,0x07},
    {0x03,0x15},{0x03,0x14},{0x03,0x12},{0x03,0x13},{0x03,0x16},{0x03,0x17},{0x03,0x0D},{0x03,0x0E},
    {0x06,0x15},{0x06,0x14},{0x06,0x12},{0x06,0x13},{0x06,0x16},{0x06,0x17},{0x06,0x0D},{0x06,0x0E},
};
#define STREAM_COUNT (sizeof(STREAM_LIST)/sizeof(STREAM_LIST[0]))

// ── Types ─────────────────────────────────────────────────────────────────────
typedef enum { ProtoNEC, ProtoSamsung, ProtoRC5, ProtoSony, ProtoCount } Protocol;
static const char* PROTO_NAMES[] = { "NEC", "Samsung", "RC5", "Sony" };
static const char  HEX_CHARS[]   = "0123456789ABCDEF";

typedef enum { ScreenMain, ScreenCustom, ScreenScan, ScreenRecent, ScreenAbout } Screen;
typedef enum { ModeTVScan, ModeLEDScan, ModeToyScan, ModeRCScan, ModeStreamScan, ModeFull, ModeCustom } ScanMode;
typedef enum { StateReady, StateRunning, StatePaused, StateDone } ScanState;

#define MENU_ITEMS 9
static const char* MENU_LABELS[MENU_ITEMS] = {
    "Full Sweep (NEC)", "TV / Displays", "LED Strips", "Toys",
    "RC Cars / Drones", "Streaming", "Custom...", "Recent Hits", "About",
};

typedef struct { Protocol proto; uint8_t addr; uint8_t cmd; bool valid; } Hit;

typedef struct {
    Screen    screen;
    uint8_t   menu_sel;
    Protocol  custom_proto;
    uint8_t   custom_addr;
    uint8_t   custom_cursor;
    bool      custom_editing;
    ScanMode  scan_mode;
    ScanState scan_state;
    Protocol  scan_proto;
    bool      scan_proto_editing;
    uint8_t   scan_cursor;
    uint8_t   addr, cmd;
    uint32_t  list_idx;
    Hit       recent[RECENT_MAX];
    uint8_t   recent_count;
    uint8_t   recent_sel;
    bool      save_status;
    bool      exit;
    FuriMutex*        mutex;
    FuriMessageQueue* queue;
    NotificationApp*  notif;
} App;

// ── Hint bar ──────────────────────────────────────────────────────────────────
static int hint(Canvas* c, int x, int y, const Icon* icon, const char* lbl) {
    canvas_draw_icon(c, x, y, icon);
    x += icon_get_width(icon) + 2;
    canvas_draw_str(c, x, y+9, lbl);
    x += (int)strlen(lbl)*6 + 5;
    return x;
}

// ── IR send ───────────────────────────────────────────────────────────────────
static void proto_send(Protocol proto, uint8_t addr, uint8_t cmd) {
    InfraredMessage msg;
    switch(proto) {
        case ProtoNEC:     msg.protocol = InfraredProtocolNEC;      break;
        case ProtoSamsung: msg.protocol = InfraredProtocolSamsung32; break;
        case ProtoRC5:     msg.protocol = InfraredProtocolRC5;      break;
        case ProtoSony:    msg.protocol = InfraredProtocolSIRC;     break;
        default:           msg.protocol = InfraredProtocolNEC;      break;
    }
    msg.address = addr;
    msg.command = cmd;
    msg.repeat  = false;
    infrared_send(&msg, 1);
}

// ── SD save ───────────────────────────────────────────────────────────────────
static bool save_hit(Protocol proto,uint8_t addr,uint8_t cmd){
    Storage* s=furi_record_open(RECORD_STORAGE);
    File* f=storage_file_alloc(s);
    bool ok=false;
    if(storage_file_open(f,LOG_PATH,FSAM_WRITE,FSOM_OPEN_APPEND)){
        char line[48];
        int len=snprintf(line,sizeof(line),"proto=%s addr=0x%02X cmd=0x%02X\n",
                         PROTO_NAMES[proto],addr,cmd);
        uint16_t w=storage_file_write(f,line,(uint16_t)len);
        ok=(w==(uint16_t)len);
        storage_file_close(f);
    }
    storage_file_free(f); furi_record_close(RECORD_STORAGE);
    return ok;
}

static void push_recent(App* app,Protocol proto,uint8_t addr,uint8_t cmd){
    for(int i=RECENT_MAX-1;i>0;i--) app->recent[i]=app->recent[i-1];
    app->recent[0].proto=proto; app->recent[0].addr=addr;
    app->recent[0].cmd=cmd;     app->recent[0].valid=true;
    if(app->recent_count<RECENT_MAX) app->recent_count++;
}

// ── Draw ──────────────────────────────────────────────────────────────────────
static void draw_cb(Canvas* canvas,void* ctx){
    App* app=(App*)ctx;
    furi_mutex_acquire(app->mutex,FuriWaitForever);
    canvas_clear(canvas);

    if(app->screen==ScreenMain){
        canvas_set_font(canvas,FontPrimary);
        canvas_draw_str(canvas,2,10,"IR Bruteforcer QE");
        canvas_draw_line(canvas,0,13,128,13);
        canvas_set_font(canvas,FontSecondary);
        int visible=4,start=0;
        if(app->menu_sel>=visible) start=app->menu_sel-visible+1;
        for(int i=0;i<visible;i++){
            int idx=start+i; if(idx>=MENU_ITEMS) break;
            int y=22+i*12;
            if(i==(app->menu_sel-start)){canvas_draw_box(canvas,0,y-9,120,11);canvas_invert_color(canvas);}
            canvas_draw_str(canvas,4,y,MENU_LABELS[idx]);
            if(i==(app->menu_sel-start)) canvas_invert_color(canvas);
        }
        int bh=49,th=bh/MENU_ITEMS;
        int ty=13+(app->menu_sel*(bh-th))/(MENU_ITEMS-1);
        canvas_draw_frame(canvas,122,13,6,bh); canvas_draw_box(canvas,123,ty,4,th);

    } else if(app->screen==ScreenCustom){
        canvas_set_font(canvas,FontPrimary);
        canvas_draw_str(canvas,2,10,"Custom Config");
        canvas_draw_line(canvas,0,13,128,13);
        canvas_set_font(canvas,FontSecondary);

        char pbuf[16]; snprintf(pbuf,sizeof(pbuf),"%s",PROTO_NAMES[app->custom_proto]);
        if(app->custom_cursor==0){
            canvas_draw_box(canvas,2,19,44,11); canvas_invert_color(canvas);
            canvas_draw_str(canvas,4,28,pbuf); canvas_invert_color(canvas);
            if(app->custom_editing) canvas_draw_str(canvas,50,28,"*");
        } else canvas_draw_str(canvas,2,28,pbuf);

        canvas_draw_str(canvas,2,44,"Addr: 0x");
        char hn[2]={HEX_CHARS[(app->custom_addr>>4)&0xF],0};
        char ln[2]={HEX_CHARS[ app->custom_addr    &0xF],0};
        if(app->custom_cursor==1){
            canvas_draw_box(canvas,50,35,6,10); canvas_invert_color(canvas);
            canvas_draw_str(canvas,50,44,hn); canvas_invert_color(canvas);
        } else canvas_draw_str(canvas,50,44,hn);
        if(app->custom_cursor==2){
            canvas_draw_box(canvas,56,35,6,10); canvas_invert_color(canvas);
            canvas_draw_str(canvas,56,44,ln); canvas_invert_color(canvas);
        } else canvas_draw_str(canvas,56,44,ln);

        { int x=2;
          if(app->custom_editing){
              x=hint(canvas,x,52,&I_arrow_updown_11x11,"change");
              hint(canvas,x,52,&I_btn_ok_11x11,"done");
          } else {
              x=hint(canvas,x,52,&I_btn_right_6x11,"move");
              x=hint(canvas,x,52,&I_btn_ok_11x11,"edit");
              hint(canvas,x,52,&I_btn_ok_11x11,"start");
          }
        }

    } else if(app->screen==ScreenAbout){
        canvas_draw_frame(canvas,2,2,124,60);
        canvas_draw_frame(canvas,4,4,120,56);
        canvas_set_font(canvas,FontPrimary);
        canvas_draw_str(canvas,20,18,"IR Bruteforcer QE");
        canvas_set_font(canvas,FontSecondary);
        canvas_draw_str(canvas,22,30,"by RaccoonFacts");
        canvas_draw_str(canvas,14,42,"/@RaccoonFacts");
        canvas_draw_str(canvas,28,54,"Subscribe :)");

    } else if(app->screen==ScreenRecent){
        canvas_set_font(canvas,FontPrimary);
        canvas_draw_str(canvas,2,10,"Recent Hits");
        canvas_draw_line(canvas,0,13,128,13);
        canvas_set_font(canvas,FontSecondary);
        if(app->recent_count==0){
            canvas_draw_str(canvas,2,32,"No hits yet");
        } else {
            for(int i=0;i<app->recent_count;i++){
                int y=22+i*10; if(y>54) break;
                Hit* h=&app->recent[i];
                char buf[32]; snprintf(buf,sizeof(buf),"[%s] A:%02X C:%02X",PROTO_NAMES[h->proto],h->addr,h->cmd);
                if(i==app->recent_sel){canvas_draw_box(canvas,0,y-8,128,10);canvas_invert_color(canvas);}
                canvas_draw_str(canvas,2,y,buf);
                if(i==app->recent_sel) canvas_invert_color(canvas);
            }
            if(app->save_status) canvas_draw_str(canvas,60,62,"Saved!");
            int x=2;
            x=hint(canvas,x,52,&I_arrow_updown_11x11,"move");
            x=hint(canvas,x,52,&I_btn_ok_11x11,"resend");
            hint(canvas,x,52,&I_btn_right_6x11,"save");
        }

    } else { // ScreenScan
        canvas_set_font(canvas,FontPrimary);
        const char* titles[]={"TV Scan","LED Scan","Toy Scan","RC Scan","Streaming","Full Sweep","Custom"};
        canvas_draw_str(canvas,2,10,titles[app->scan_mode]);
        canvas_draw_line(canvas,0,13,128,13);
        canvas_set_font(canvas,FontSecondary);

        char pbuf[12]; snprintf(pbuf,sizeof(pbuf),"%s",PROTO_NAMES[app->scan_proto]);
        if(app->scan_cursor==0){
            canvas_draw_box(canvas,2,15,44,11); canvas_invert_color(canvas);
            canvas_draw_str(canvas,4,24,pbuf); canvas_invert_color(canvas);
        } else canvas_draw_str(canvas,2,24,pbuf);

        char abuf[16]; snprintf(abuf,sizeof(abuf),"A:%02X C:%02X",app->addr,app->cmd);
        canvas_draw_str(canvas,52,24,abuf);

        uint32_t total,cur;
        if(app->scan_mode==ModeFull){total=65536;cur=(uint32_t)app->addr*256+app->cmd;}
        else if(app->scan_mode==ModeCustom){total=256;cur=app->cmd;}
        else{uint32_t counts[]={TV_COUNT,LED_COUNT,TOY_COUNT,RC_COUNT,STREAM_COUNT};total=counts[app->scan_mode];cur=app->list_idx;}
        int prog=(int)((cur*124)/total);
        canvas_draw_frame(canvas,2,28,124,7);
        if(prog>0) canvas_draw_box(canvas,2,28,prog,7);

        const char* btn=app->scan_state==StateRunning?"Pause":
                        app->scan_state==StatePaused?"Resume":
                        app->scan_state==StateDone?"Restart":"Start";
        if(app->scan_cursor==1){
            canvas_draw_box(canvas,2,38,44,11); canvas_invert_color(canvas);
            canvas_draw_str(canvas,4,47,btn); canvas_invert_color(canvas);
        } else canvas_draw_str(canvas,4,47,btn);

        if(app->recent_count>0){char rb[12];snprintf(rb,sizeof(rb),"Hits:%d",app->recent_count);canvas_draw_str(canvas,80,47,rb);}

        { int x=2;
          if(app->scan_proto_editing){
              x=hint(canvas,x,52,&I_arrow_updown_11x11,"change");
              hint(canvas,x,52,&I_btn_ok_11x11,"done");
          } else {
              x=hint(canvas,x,52,&I_arrow_updown_11x11,"move");
              x=hint(canvas,x,52,&I_btn_ok_11x11,"select");
              hint(canvas,x,52,&I_btn_right_6x11,"save");
          }
        }
    }

    furi_mutex_release(app->mutex);
}

static void input_cb(InputEvent* e,void* ctx){
    furi_message_queue_put((FuriMessageQueue*)ctx,e,0);
}

static const Pair* get_list(ScanMode m,uint32_t* count){
    switch(m){
        case ModeTVScan:     *count=TV_COUNT;     return TV_LIST;
        case ModeLEDScan:    *count=LED_COUNT;    return LED_LIST;
        case ModeToyScan:    *count=TOY_COUNT;    return TOY_LIST;
        case ModeRCScan:     *count=RC_COUNT;     return RC_LIST;
        case ModeStreamScan: *count=STREAM_COUNT; return STREAM_LIST;
        default: *count=0; return NULL;
    }
}

int32_t ir_bruteforce_app(void* p){
    UNUSED(p);
    App* app=malloc(sizeof(App));
    memset(app,0,sizeof(App));
    app->screen=ScreenMain; app->scan_state=StateDone; app->scan_proto=ProtoNEC;
    app->mutex=furi_mutex_alloc(FuriMutexTypeNormal);
    app->queue=furi_message_queue_alloc(8,sizeof(InputEvent));
    app->notif=furi_record_open(RECORD_NOTIFICATION);

    ViewPort* vp=view_port_alloc();
    view_port_draw_callback_set(vp,draw_cb,app);
    view_port_input_callback_set(vp,input_cb,app->queue);
    Gui* gui=furi_record_open(RECORD_GUI);
    gui_add_view_port(gui,vp,GuiLayerFullscreen);
    view_port_update(vp);

    InputEvent ev;
    while(!app->exit){
        if(furi_message_queue_get(app->queue,&ev,10)==FuriStatusOk){
            if(ev.type==InputTypeShort){
                furi_mutex_acquire(app->mutex,FuriWaitForever);

                if(app->screen==ScreenMain){
                    if(ev.key==InputKeyUp   && app->menu_sel>0)            app->menu_sel--;
                    if(ev.key==InputKeyDown && app->menu_sel<MENU_ITEMS-1) app->menu_sel++;
                    if(ev.key==InputKeyBack) app->exit=true;
                    if(ev.key==InputKeyOk){
                        if(app->menu_sel==0){
                            app->screen=ScreenScan; app->scan_mode=ModeFull;
                            app->scan_proto=ProtoNEC; app->scan_state=StateReady;
                            app->scan_proto_editing=false; app->scan_cursor=0;
                            app->addr=0; app->cmd=0;
                        } else if(app->menu_sel==6){
                            app->screen=ScreenCustom; app->custom_cursor=0; app->custom_editing=false;
                        } else if(app->menu_sel==7){
                            app->screen=ScreenRecent; app->recent_sel=0; app->save_status=false;
                        } else if(app->menu_sel==8){
                            app->screen=ScreenAbout;
                        } else {
                            // 1=TV 2=LED 3=Toy 4=RC 5=Stream → ScanMode 0-4
                            app->screen=ScreenScan; app->scan_mode=(ScanMode)(app->menu_sel-1);
                            app->scan_proto=ProtoNEC; app->scan_state=StateReady;
                            app->scan_proto_editing=false; app->scan_cursor=0; app->list_idx=0;
                        }
                    }

                } else if(app->screen==ScreenCustom){
                    if(ev.key==InputKeyBack){
                        if(app->custom_editing) app->custom_editing=false;
                        else app->screen=ScreenMain;
                    }
                    if(!app->custom_editing){
                        if(ev.key==InputKeyLeft  && app->custom_cursor>0) app->custom_cursor--;
                        if(ev.key==InputKeyRight && app->custom_cursor<2) app->custom_cursor++;
                        if(ev.key==InputKeyOk) app->custom_editing=true;
                    } else {
                        if(app->custom_cursor==0){
                            if(ev.key==InputKeyUp)   app->custom_proto=(Protocol)((app->custom_proto+ProtoCount-1)%ProtoCount);
                            if(ev.key==InputKeyDown) app->custom_proto=(Protocol)((app->custom_proto+1)%ProtoCount);
                            if(ev.key==InputKeyOk)   app->custom_editing=false;
                        } else if(app->custom_cursor==1){
                            if(ev.key==InputKeyUp)   app->custom_addr=(app->custom_addr&0x0F)|(((( app->custom_addr>>4)+1)&0xF)<<4);
                            if(ev.key==InputKeyDown) app->custom_addr=(app->custom_addr&0x0F)|((((app->custom_addr>>4)-1)&0xF)<<4);
                            if(ev.key==InputKeyOk)   app->custom_editing=false;
                        } else {
                            if(ev.key==InputKeyUp)   app->custom_addr=(app->custom_addr&0xF0)|(((app->custom_addr&0xF)+1)&0xF);
                            if(ev.key==InputKeyDown) app->custom_addr=(app->custom_addr&0xF0)|(((app->custom_addr&0xF)-1)&0xF);
                            if(ev.key==InputKeyOk){
                                app->custom_editing=false;
                                app->screen=ScreenScan; app->scan_mode=ModeCustom;
                                app->scan_proto=app->custom_proto; app->scan_proto_editing=false;
                                app->scan_cursor=0; app->addr=app->custom_addr; app->cmd=0;
                                app->scan_state=StateReady;
                            }
                        }
                    }

                } else if(app->screen==ScreenAbout){
                    if(ev.key==InputKeyBack||ev.key==InputKeyOk) app->screen=ScreenMain;

                } else if(app->screen==ScreenRecent){
                    if(ev.key==InputKeyBack){ app->screen=ScreenMain; app->save_status=false; }
                    if(ev.key==InputKeyUp   && app->recent_sel>0)                   app->recent_sel--;
                    if(ev.key==InputKeyDown && app->recent_sel<app->recent_count-1) app->recent_sel++;
                    if(ev.key==InputKeyOk && app->recent_count>0){
                        Hit* h=&app->recent[app->recent_sel];
                        proto_send(h->proto,h->addr,h->cmd);
                        notification_message(app->notif,&sequence_blink_blue_10);
                    }
                    if(ev.key==InputKeyRight && app->recent_count>0){
                        Hit* h=&app->recent[app->recent_sel];
                        app->save_status=save_hit(h->proto,h->addr,h->cmd);
                        notification_message(app->notif,app->save_status?&sequence_blink_green_100:&sequence_blink_red_100);
                    }

                } else { // ScreenScan
                    if(app->scan_proto_editing){
                        if(ev.key==InputKeyUp)   app->scan_proto=(Protocol)((app->scan_proto+ProtoCount-1)%ProtoCount);
                        if(ev.key==InputKeyDown) app->scan_proto=(Protocol)((app->scan_proto+1)%ProtoCount);
                        if(ev.key==InputKeyOk||ev.key==InputKeyBack){
                            app->scan_proto_editing=false;
                            if(app->scan_state==StatePaused) app->scan_state=StateRunning;
                        }
                    } else {
                        if(ev.key==InputKeyBack){ app->screen=ScreenMain; app->scan_state=StateDone; }
                        if(ev.key==InputKeyUp   && app->scan_cursor>0) app->scan_cursor--;
                        if(ev.key==InputKeyDown && app->scan_cursor<1) app->scan_cursor++;
                        if(ev.key==InputKeyOk){
                            if(app->scan_cursor==0){
                                app->scan_proto_editing=true;
                                if(app->scan_state==StateRunning) app->scan_state=StatePaused;
                            } else {
                                if(app->scan_state==StateReady||app->scan_state==StatePaused){
                                    app->scan_state=StateRunning;
                                } else if(app->scan_state==StateRunning){
                                    app->scan_state=StatePaused;
                                } else if(app->scan_state==StateDone){
                                    app->cmd=0; app->list_idx=0; app->addr=0;
                                    app->scan_state=StateRunning;
                                }
                            }
                        }
                        if(ev.key==InputKeyRight){
                            push_recent(app,app->scan_proto,app->addr,app->cmd);
                            bool ok=save_hit(app->scan_proto,app->addr,app->cmd);
                            notification_message(app->notif,ok?&sequence_blink_green_100:&sequence_blink_red_100);
                        }
                    }
                }

                furi_mutex_release(app->mutex);
                view_port_update(vp);
            }
        }

        // ── Scan tick ─────────────────────────────────────────────────────────
        furi_mutex_acquire(app->mutex,FuriWaitForever);
        bool run=(app->screen==ScreenScan&&app->scan_state==StateRunning);
        ScanMode sm=app->scan_mode; Protocol sp=app->scan_proto;
        furi_mutex_release(app->mutex);

        if(run){
            if(sm==ModeFull||sm==ModeCustom){
                furi_mutex_acquire(app->mutex,FuriWaitForever);
                uint8_t a=app->addr,c=app->cmd; furi_mutex_release(app->mutex);
                proto_send(sp,a,c);
                furi_delay_ms(POST_TX_DELAY_MS);
                notification_message(app->notif,&sequence_blink_cyan_10);
                furi_delay_ms(SEND_DELAY_MS);
                furi_mutex_acquire(app->mutex,FuriWaitForever);
                if(sm==ModeCustom){
                    if(app->cmd<0xFF) app->cmd++; else app->scan_state=StateDone;
                } else {
                    if(app->cmd<0xFF){ app->cmd++; }
                    else if(app->addr<0xFF){ app->cmd=0; app->addr++; }
                    else app->scan_state=StateDone;
                }
                furi_mutex_release(app->mutex);
            } else {
                uint32_t count=0; const Pair* list=get_list(sm,&count);
                furi_mutex_acquire(app->mutex,FuriWaitForever);
                uint32_t idx=app->list_idx; furi_mutex_release(app->mutex);
                if(idx<count){
                    proto_send(sp,list[idx].addr,list[idx].cmd);
                    furi_delay_ms(POST_TX_DELAY_MS);
                    notification_message(app->notif,&sequence_blink_cyan_10);
                    furi_mutex_acquire(app->mutex,FuriWaitForever);
                    app->addr=list[idx].addr; app->cmd=list[idx].cmd; app->list_idx++;
                    furi_mutex_release(app->mutex);
                    furi_delay_ms(SEND_DELAY_MS);
                } else {
                    furi_mutex_acquire(app->mutex,FuriWaitForever);
                    app->scan_state=StateDone; furi_mutex_release(app->mutex);
                }
            }
            view_port_update(vp);
        }
    }

    gui_remove_view_port(gui,vp); view_port_free(vp);
    furi_record_close(RECORD_GUI); furi_record_close(RECORD_NOTIFICATION);
    furi_message_queue_free(app->queue); furi_mutex_free(app->mutex);
    free(app); return 0;
}