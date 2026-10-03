#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#define TOP_W 400.0f
#define TOP_H 240.0f
#define BOT_W 320.0f
#define BOT_H 240.0f
#define SAVE_PATH "sdmc:/3ds/ScarletSkips/save.dat"
#define TAU 6.28318530718f

typedef struct {
    unsigned int magic;
    unsigned int skips;
    unsigned int coins;
    unsigned int scarletLevel;
    unsigned int bestCombo;
    unsigned int ropeLevel;
    unsigned int coinLevel;
} SaveData;

static SaveData save = {0x53534B50, 0, 0, 0, 0, 0, 0};
static C3D_RenderTarget *top, *bottom;
static C2D_TextBuf textBuf;
static C2D_Font font;
static float ropeAngle = 0.0f;
static int combo = 0;
static int screen = 0; /* 0 game, 1 shop */
static int flashTimer = 0;
static bool perfect = false;
static float runTime = 0.0f;

static u32 rgba(u8 r, u8 g, u8 b, u8 a) { return C2D_Color32(r,g,b,a); }
static void rect(float x,float y,float w,float h,u32 c){ C2D_DrawRectSolid(x,y,0,w,h,c); }
static void circle(float x,float y,float r,u32 c){ C2D_DrawCircleSolid(x,y,0,r,c); }
static void line(float x1,float y1,float x2,float y2,float width,u32 c){ C2D_DrawLine(x1,y1,0,c,x2,y2,0,c,width); }

static void saveGame(void){
    mkdir("sdmc:/3ds",0777); mkdir("sdmc:/3ds/ScarletSkips",0777);
    FILE *f=fopen(SAVE_PATH,"wb"); if(f){ fwrite(&save,sizeof(save),1,f); fclose(f); }
}
static void loadGame(void){
    FILE *f=fopen(SAVE_PATH,"rb");
    if(f){ SaveData t; if(fread(&t,sizeof(t),1,f)==1 && t.magic==0x53534B50) save=t; fclose(f); }
}
static void text(C2D_Text *t,const char *s,float x,float y,float sx,float sy,u32 c){
    C2D_TextParse(t,textBuf,font,s); C2D_TextOptimize(t); C2D_DrawText(t,C2D_WithColor,x,y,0,sx,sy,c);
}
static void centerText(C2D_Text *t,const char *s,float y,float scale,u32 c){
    C2D_TextParse(t,textBuf,font,s); C2D_TextOptimize(t); float w=C2D_TextGetWidth(t,scale); C2D_DrawText(t,C2D_WithColor,(400-w)/2,y,0,scale,scale,c);
}

static void drawSkyline(void){
    /* Sunset sky */
    rect(0,0,400,145,rgba(93,76,126,255));
    rect(0,40,400,105,rgba(122,93,134,255));
    rect(0,90,400,55,rgba(215,126,102,255));
    circle(305,92,26,rgba(255,210,125,255));
    /* distant hills */
    for(int i=0;i<10;i++){
        float x=i*52.0f-25.0f;
        circle(x,153,48,rgba(58,73,87,255));
    }
    /* city silhouette */
    u32 city=rgba(43,46,61,255);
    rect(0,138,45,35,city); rect(48,126,28,47,city); rect(79,143,38,30,city);
    rect(122,119,34,54,city); rect(161,137,52,36,city); rect(219,129,25,44,city);
    rect(249,143,40,30,city); rect(294,124,35,49,city); rect(334,136,29,37,city); rect(367,116,33,57,city);
    /* water tower */
    rect(70,112,5,38,rgba(69,76,89,255));
    circle(72,108,13,rgba(84,91,103,255));
    /* trees */
    for(int x=18;x<400;x+=72){
        rect((float)x,132,5,43,rgba(54,45,39,255));
        circle((float)x,119,19,rgba(35,76,62,255));
        circle((float)x+12,127,15,rgba(42,91,68,255));
    }
    /* park grass */
    rect(0,173,400,67,rgba(39,77,56,255));
    rect(0,188,400,52,rgba(48,91,60,255));
    /* path */
    rect(0,201,400,39,rgba(132,113,94,255));
    line(0,201,400,201,2,rgba(174,149,120,255));
    /* benches */
    for(int x=25;x<380;x+=110){
        rect((float)x,182,43,4,rgba(103,66,47,255));
        rect((float)x,188,43,4,rgba(119,75,51,255));
        line(x+5,192,x+2,202,3,rgba(58,49,44,255));
        line(x+37,192,x+40,202,3,rgba(58,49,44,255));
    }
}

static void drawPerson(float cx,float ground,float jump){
    float gy=ground-jump;
    u32 skin=rgba(246,198,153,255), hoodie=rgba(214,43,67,255), dark=rgba(29,25,36,255), shoe=rgba(238,238,242,255);
    circle(cx,gy-92,14,skin);
    circle(cx-5,gy-100,8,dark); circle(cx+6,gy-101,7,dark);
    rect(cx-12,gy-77,24,31,hoodie);
    line(cx-9,gy-46,cx-19,gy-17,6,dark); line(cx+9,gy-46,cx+19,gy-17,6,dark);
    line(cx-19,gy-17,cx-24,gy-3,5,shoe); line(cx+19,gy-17,cx+24,gy-3,5,shoe);
    line(cx-10,gy-70,cx-27,gy-52,5,skin); line(cx+10,gy-70,cx+27,gy-52,5,skin);
    circle(cx-27,gy-52,4,skin); circle(cx+27,gy-52,4,skin);
    line(cx-12,gy-76,cx-4,gy-55,2,rgba(255,113,129,255));
}

static void drawRope(float cx,float ground,float jump){
    u32 rope = save.scarletLevel ? rgba(255,57,83,255) : rgba(238,238,245,255);
    u32 glow = save.scarletLevel ? rgba(255,105,120,130) : rgba(255,255,255,70);
    float phase=ropeAngle;
    float squash=0.56f+0.28f*fabsf(cosf(phase));
    float rx=79.0f, ry=55.0f*squash;
    float cy=ground-45-jump*0.35f;
    float prevX=cx+rx*cosf(0), prevY=cy+ry*sinf(0);
    for(int i=1;i<=40;i++){
        float a=TAU*(float)i/40.0f + phase;
        float x=cx+rx*cosf(a), y=cy+ry*sinf(a);
        line(prevX,prevY,x,y,5,glow); line(prevX,prevY,x,y,2.3f,rope);
        prevX=x; prevY=y;
    }
    circle(cx-rx*cosf(phase),cy+ry*sinf(phase),4,rope);
    circle(cx+rx*cosf(phase),cy+ry*sinf(phase),4,rope);
}

static void drawHud(C2D_Text *t){
    char b[64];
    rect(10,10,138,34,rgba(25,25,36,220));
    text(t,"SKIPS",20,16,0.38f,0.38f,rgba(190,190,205,255));
    snprintf(b,sizeof(b),"%u",save.skips); text(t,b,20,29,0.63f,0.63f,rgba(255,255,255,255));
    rect(158,10,96,34,rgba(25,25,36,220));
    text(t,"COMBO",168,16,0.34f,0.34f,rgba(190,190,205,255));
    snprintf(b,sizeof(b),"x%d",combo); text(t,b,168,29,0.55f,0.55f,rgba(255,78,104,255));
    rect(264,10,126,34,rgba(25,25,36,220));
    text(t,"COINS",274,16,0.34f,0.34f,rgba(190,190,205,255));
    snprintf(b,sizeof(b),"%u",save.coins); text(t,b,274,29,0.55f,0.55f,rgba(255,211,91,255));
}

static void drawGame(void){
    C2D_Text t; char b[96];
    C2D_TargetClear(top,rgba(20,24,35,255)); C2D_SceneBegin(top);
    drawSkyline();
    drawHud(&t);
    float jump=0.0f;
    /* Character leaves the ground as the rope rises. */
    float low=fabsf(sinf(ropeAngle));
    if(low>0.35f) jump=9.0f+18.0f*low;
    drawRope(200,203,jump); drawPerson(200,203,jump);
    if(save.scarletLevel){
        text(&t,"SCARLET",164,53,0.34f,0.34f,rgba(255,75,98,255));
        text(&t,"SKIPS",190,66,0.34f,0.34f,rgba(255,255,255,255));
    }
    if(flashTimer>0){
        centerText(&t,perfect?"PERFECT!":"MISS",91,0.78f,perfect?rgba(101,255,180,255):rgba(255,95,112,255));
    }
    rect(95,213,210,21,rgba(22,23,31,210));
    text(&t,"A  SKIP",113,218,0.43f,0.43f,rgba(255,255,255,255));
    text(&t,"B  SHOP",228,218,0.43f,0.43f,rgba(255,78,104,255));

    C2D_TargetClear(bottom,rgba(22,20,31,255)); C2D_SceneBegin(bottom);
    rect(15,15,290,48,rgba(33,30,45,255));
    centerText(&t,"SCARLET SKIPS",29,0.63f,rgba(255,78,104,255));
    snprintf(b,sizeof(b),"BEST COMBO   %u",save.bestCombo); text(&t,b,28,83,0.50f,0.50f,rgba(255,255,255,255));
    snprintf(b,sizeof(b),"ROPE LEVEL   %u",save.ropeLevel); text(&t,b,28,111,0.43f,0.43f,rgba(190,185,205,255));
    snprintf(b,sizeof(b),"SCARLET LV   %u",save.scarletLevel); text(&t,b,28,137,0.43f,0.43f,rgba(255,78,104,255));
    text(&t,"Tap A when the rope is underneath you.",28,172,0.39f,0.39f,rgba(205,200,218,255));
    text(&t,"B = shop     START = save & quit",44,201,0.39f,0.39f,rgba(255,255,255,255));
}

static unsigned cost(unsigned base,unsigned level){ return base*(level+1); }
static void drawShop(void){
    C2D_Text t; char b[96];
    C2D_TargetClear(top,rgba(17,16,27,255)); C2D_SceneBegin(top);
    rect(0,0,400,240,rgba(17,16,27,255)); rect(0,0,400,7,rgba(235,55,82,255));
    text(&t,"UPGRADE SHOP",20,18,0.72f,0.72f,rgba(255,255,255,255));
    snprintf(b,sizeof(b),"COINS  %u",save.coins); text(&t,b,276,22,0.42f,0.42f,rgba(255,211,91,255));
    struct Row { const char *name; unsigned level; unsigned price; } rows[3] = {
        {"Rope Speed",save.ropeLevel,cost(50,save.ropeLevel)},
        {"Coin Bonus",save.coinLevel,cost(75,save.coinLevel)},
        {"Scarlet Skips",save.scarletLevel,cost(100,save.scarletLevel)}
    };
    for(int i=0;i<3;i++){
        float y=62+i*49;
        rect(18,y,364,39,rgba(31,29,43,255));
        text(&t,rows[i].name,30,y+9,0.45f,0.45f,i==2?rgba(255,78,104,255):rgba(255,255,255,255));
        snprintf(b,sizeof(b),"LV %u",rows[i].level); text(&t,b,184,y+9,0.39f,0.39f,rgba(185,180,198,255));
        snprintf(b,sizeof(b),"%u",rows[i].price); text(&t,b,275,y+9,0.42f,0.42f,rgba(255,211,91,255));
        text(&t,"A",350,y+7,0.52f,0.52f,rgba(101,255,180,255));
    }
    rect(18,215,364,17,rgba(235,55,82,255));
    text(&t,"A BUY SELECTED   D-PAD SELECT   B BACK",31,216,0.34f,0.34f,rgba(255,255,255,255));

    C2D_TargetClear(bottom,rgba(22,20,31,255)); C2D_SceneBegin(bottom);
    centerText(&t,"SCARLET UPGRADES",28,0.60f,rgba(255,78,104,255));
    text(&t,"UP / DOWN = choose",57,75,0.46f,0.46f,rgba(255,255,255,255));
    text(&t,"A = buy",112,107,0.46f,0.46f,rgba(101,255,180,255));
    text(&t,"B = return",102,139,0.46f,0.46f,rgba(255,255,255,255));
    text(&t,"Every level makes the run",68,176,0.40f,0.40f,rgba(195,190,210,255));
    text(&t,"faster and more rewarding.",64,195,0.40f,0.40f,rgba(195,190,210,255));
}

int main(int argc,char **argv){
    gfxInitDefault(); C3D_Init(C3D_DEFAULT_CMDBUF_SIZE); C2D_Init(C2D_DEFAULT_MAX_OBJECTS); C2D_Prepare();
    hidSetRepeatParameters(20,5);
    top=C2D_CreateScreenTarget(GFX_TOP,GFX_LEFT); bottom=C2D_CreateScreenTarget(GFX_BOTTOM,GFX_LEFT);
    textBuf=C2D_TextBufNew(8192); font=C2D_FontLoadSystem(CFG_REGION_USA); loadGame();
    u64 prev=osGetTime(); int selected=0;
    while(aptMainLoop()){
        hidScanInput(); u32 down=hidKeysDown();
        if(down&KEY_START){saveGame();break;}
        if(screen==0){
            if(down&KEY_B){screen=1; selected=0;}
            u64 now=osGetTime(); float dt=(float)(now-prev)/1000.0f; prev=now; if(dt>0.08f)dt=0.08f;
            runTime+=dt;
            float speed=2.5f + save.ropeLevel*0.16f + save.scarletLevel*0.24f;
            ropeAngle+=dt*speed; if(ropeAngle>TAU)ropeAngle-=TAU;
            if(flashTimer>0) flashTimer--;
            if(down&KEY_A){
                float timing=fabsf(sinf(ropeAngle));
                if(timing<0.22f){
                    save.skips++; combo++; if((unsigned)combo>save.bestCombo)save.bestCombo=combo;
                    save.coins += 1 + save.coinLevel + save.scarletLevel;
                    perfect=(timing<0.08f); flashTimer=24;
                } else { combo=0; perfect=false; flashTimer=24; }
            }
        } else {
            if(down&KEY_B)screen=0;
            if(down&KEY_DOWN){selected=(selected+1)%3;}
            if(down&KEY_UP){selected=(selected+2)%3;}
            if(down&KEY_A){
                unsigned *level = selected==0?&save.ropeLevel:(selected==1?&save.coinLevel:&save.scarletLevel);
                unsigned base = selected==0?50:(selected==1?75:100);
                unsigned price=cost(base,*level);
                if(save.coins>=price && *level<9){save.coins-=price;(*level)++;saveGame();}
            }
        }
        C2D_TextBufClear(textBuf); C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        if(screen==0)drawGame(); else drawShop();
        C3D_FrameEnd(0);
    }
    saveGame(); C2D_FontFree(font); C2D_TextBufDelete(textBuf); C2D_Fini(); C3D_Fini(); gfxExit(); return 0;
}
