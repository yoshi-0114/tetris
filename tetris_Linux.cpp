#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define screen_width 10
#define screen_height 20
#define shape_max 4
#define fps 1*1000+500
#define interval 1000/fps

int count=0;

enum {
    SHAPE_I,
    SHAPE_O,
    SHAPE_S,
    SHAPE_Z,
    SHAPE_J,
    SHAPE_L,
    SHAPE_T,
    SHAPE_MAX
};

typedef struct {
    int width,height;
    int pattern[shape_max][shape_max];
}SHAPE;

typedef struct {
    int x,y;
    SHAPE shape;
}MINO;

SHAPE shapes[SHAPE_MAX] = {
    4,4, //SHAPE_I
    {
        {0,0,0,0},
        {1,1,1,1},
        {0,0,0,0},
        {0,0,0,0}
    },
    2,2, //SHAPE_O
    {
        {1,1},
        {1,1}
    },
    3,3, //SHAPE_S
    {
        {0,1,1},
        {1,1,0},
        {0,0,0}
    },
    3,3, //SHAPE_Z
    {
        {1,1,0},
        {0,1,1},
        {0,0,0}
    },
    3,3, //SHAPE_J
    {
        {1,0,0},
        {1,1,1},
        {0,0,0}
    },
    3,3, //SHAPE_L
    {
        {0,0,1},
        {1,1,1},
        {0,0,0}
    },
    3,3, //SHAPE_T
    {
        {0,1,0},
        {1,1,1},
        {0,0,0}
    }
};

int field[screen_height][screen_width];
int screen[screen_height][screen_width];
MINO mino;

bool MinoIntersectField() {
    for(int y=0;y<mino.shape.height;y++) {
        for(int x=0;x<mino.shape.width;x++) {
            if(mino.shape.pattern[y][x]) {
                if((mino.y+y<0) || (mino.y+y>=screen_height)
                || (mino.x+x<0) || (mino.x+x>=screen_width)) {
                    return true;
                }
                if(field[mino.y+y][mino.x+x]) {
                    return true;
                }
            }
        }
    }
    return false;
}

int kbhit(void) {
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF) {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}

void drawscreen() {
    memcpy(screen,field,sizeof field);

    for(int y=0;y<mino.shape.height;y++) {
        for(int x=0;x<mino.shape.width;x++) {
            if(mino.shape.pattern[y][x]) {
                screen[mino.y+y][mino.x+x] |= 1;
            }
        }
    }

    system("clear");

    for(int i=0;i<screen_height;i++) {
        printf("□");
        for(int j=0;j<screen_width;j++) {
            printf("%s",screen[i][j] ? "■" : " ");
        }
        printf("□");
        printf("\n");
    }
    for(int i=0;i<screen_width+2;i++) {
        printf("―");
    }
    printf("\n");
    printf("消したライン数：%d\n",count);
}

void init_mino() {
    mino.shape = shapes[rand() % SHAPE_MAX];
    mino.x = (screen_width - mino.shape.width) / 2;
    mino.y = 0;
}

void init() {
    memset(field,0,sizeof field);
    
    init_mino();
    drawscreen();
}

bool GameOver() {
    for(int x=0;x<screen_width;x++) {
        if(field[0][x]) {
            return true;
        }
    }
    return false;
}

int main() {
    srand((unsigned) time(NULL));
    init();

    clock_t lastClock = clock();
    for(;;) {
        clock_t nowClock = clock();
        if(nowClock >= lastClock+interval) {
            lastClock = nowClock;
            
            MINO lastMino = mino;
            mino.y++;

            if(MinoIntersectField()) {
                mino = lastMino;

                for(int y=0;y<mino.shape.height;y++) {
                    for(int x=0;x<mino.shape.width;x++) {
                        if(mino.shape.pattern[y][x]) {
                            field[mino.y+y][mino.x+x] |= 1;
                        }
                    }
                }

                for(int y=0;y<screen_height;y++) {
                    bool completed = true;
                    for(int x=0;x<screen_width;x++) {
                        if(!field[y][x]) {
                            completed = false;
                            break;
                        }
                    }
                    if(completed) {
                        count++;
                        for(int x=0;x<screen_width;x++) {
                            field[y][x] = 0;
                        }
                        for(int i=y;i>=1;i--) {
                            for(int x=0;x<screen_width;x++) {
                                field[i][x] = field[i-1][x];
                                field[i-1][x] = 0;
                            }
                        }
                    }
                }
                init_mino();
            }
            drawscreen();
            
            if(GameOver()) {
                printf("\n\nGAME OVER\n\n");
                printf("Score: %d\n",count);
                return 0;
            }

        }
        if(kbhit()) {
            MINO lastMino = mino;
            switch(getchar()) {
                case 'w':
                    break;
                case 'a':
                    mino.x--;
                    break;
                case 's':
                    mino.y++;
                    break;
                case 'd':
                    mino.x++;
                    break;
                default:
                    {
                        MINO newMino = mino;
                        for(int y=0;y<mino.shape.height;y++) {
                            for(int x=0;x<mino.shape.width;x++) {
                                newMino.shape.pattern[mino.shape.width-1-x][y] = mino.shape.pattern[y][x];
                            }
                        }
                        mino = newMino;
                    }
                    break;
            }
            if(MinoIntersectField()) {
                mino = lastMino;
            }
            drawscreen();  
        }
    }
}
