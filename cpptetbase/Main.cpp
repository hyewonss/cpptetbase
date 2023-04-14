#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <termios.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "colors.h"
#include "Matrix.h"

using namespace std;


/**************************************************************/
/**************** Linux System Functions **********************/
/**************************************************************/

char saved_key = 0;
int tty_raw(int fd);	/* put terminal into a raw mode */
int tty_reset(int fd);	/* restore terminal's mode */
  
/* Read 1 character - echo defines echo mode */
char getch() {
  char ch;
  int n;
  while (1) {
    tty_raw(0);
    n = read(0, &ch, 1);
    tty_reset(0);
    if (n > 0)
      break;
    else if (n < 0) {
      if (errno == EINTR) {
        if (saved_key != 0) {
          ch = saved_key;
          saved_key = 0;
          break;
        }
      }
    }
  }
  return ch;
}

void sigint_handler(int signo) {
  // cout << "SIGINT received!" << endl;
  // do nothing;
}

void sigalrm_handler(int signo) {
  alarm(1);
  saved_key = 's';
}

void registerInterrupt() {
  struct sigaction act, oact;
  act.sa_handler = sigint_handler;
  sigemptyset(&act.sa_mask);
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGINT, &act, &oact) < 0) {
    cerr << "sigaction error" << endl;
    exit(1);
  }
}

void registerAlarm() {
  struct sigaction act, oact;
  act.sa_handler = sigalrm_handler;
  sigemptyset(&act.sa_mask);
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGALRM, &act, &oact) < 0) {
    cerr << "sigaction error" << endl;
    exit(1);
  }
  alarm(1);
}

/**************************************************************/
/**************** Tetris Blocks Definitions *******************/
/**************************************************************/
#define MAX_BLK_TYPES 7
#define MAX_BLK_DEGREES 4

int T0D0[] = { 1, 1, 1, 1, -1 };
int T0D1[] = { 1, 1, 1, 1, -1 };
int T0D2[] = { 1, 1, 1, 1, -1 };
int T0D3[] = { 1, 1, 1, 1, -1 };

int T1D0[] = { 0, 1, 0, 1, 1, 1, 0, 0, 0, -1 };
int T1D1[] = { 0, 1, 0, 0, 1, 1, 0, 1, 0, -1 };
int T1D2[] = { 0, 0, 0, 1, 1, 1, 0, 1, 0, -1 };
int T1D3[] = { 0, 1, 0, 1, 1, 0, 0, 1, 0, -1 };

int T2D0[] = { 1, 0, 0, 1, 1, 1, 0, 0, 0, -1 };
int T2D1[] = { 0, 1, 1, 0, 1, 0, 0, 1, 0, -1 };
int T2D2[] = { 0, 0, 0, 1, 1, 1, 0, 0, 1, -1 };
int T2D3[] = { 0, 1, 0, 0, 1, 0, 1, 1, 0, -1 };

int T3D0[] = { 0, 0, 1, 1, 1, 1, 0, 0, 0, -1 };
int T3D1[] = { 0, 1, 0, 0, 1, 0, 0, 1, 1, -1 };
int T3D2[] = { 0, 0, 0, 1, 1, 1, 1, 0, 0, -1 };
int T3D3[] = { 1, 1, 0, 0, 1, 0, 0, 1, 0, -1 };

int T4D0[] = { 0, 1, 0, 1, 1, 0, 1, 0, 0, -1 };
int T4D1[] = { 1, 1, 0, 0, 1, 1, 0, 0, 0, -1 };
int T4D2[] = { 0, 1, 0, 1, 1, 0, 1, 0, 0, -1 };
int T4D3[] = { 1, 1, 0, 0, 1, 1, 0, 0, 0, -1 };

int T5D0[] = { 0, 1, 0, 0, 1, 1, 0, 0, 1, -1 };
int T5D1[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, -1 };
int T5D2[] = { 0, 1, 0, 0, 1, 1, 0, 0, 1, -1 };
int T5D3[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, -1 };

int T6D0[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1 };
int T6D1[] = { 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1 };
int T6D2[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1 };
int T6D3[] = { 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1 };
  
int *setOfBlockArrays[] = {
  T0D0, T0D1, T0D2, T0D3,
  T1D0, T1D1, T1D2, T1D3,
  T2D0, T2D1, T2D2, T2D3,
  T3D0, T3D1, T3D2, T3D3,
  T4D0, T4D1, T4D2, T4D3,
  T5D0, T5D1, T5D2, T5D3,
  T6D0, T6D1, T6D2, T6D3,
};

void drawScreen(Matrix *screen, int wall_depth)
{
  int dy = screen->get_dy();
  int dx = screen->get_dx();
  int dw = wall_depth;
  int **array = screen->get_array();

  for (int y = 0; y < dy - dw + 1; y++) {
    for (int x = dw - 1; x < dx - dw + 1; x++) {
      if (array[y][x] == 0)
	      cout << "□ ";
      else if (array[y][x] == 1)
	      cout << "■ ";
      else if (array[y][x] == 10)
	      cout << "◈ ";
      else if (array[y][x] == 20)
	      cout << "★ ";
      else if (array[y][x] == 30)
	      cout << "● ";
      else if (array[y][x] == 40)
	      cout << "◆ ";
      else if (array[y][x] == 50)
	      cout << "▲ ";
      else if (array[y][x] == 60)
	      cout << "♣ ";
      else if (array[y][x] == 70)
	      cout << "♥ ";
      else
	      cout << "XX";
    }
    cout << endl;
  }
}

void deleteFullLines(Matrix *screen){ //sum밑줄부터 확인하기
  int left=3; int right=13;
  int zeroo[]={0,0,0,0,0,0,0,0,0,0};
  Matrix *zero= new Matrix(zeroo,1,10);
  for (int top=10; top>0; top--){
    Matrix *checkBLk=screen->clip(top-1, left, top, right);
    int s=checkBLk->sum();
    if (s==10){
      delete checkBLk;
      checkBLk=screen->clip(0, left, top-1, right);
      screen->paste(checkBLk, 1, left); //paste를 한 줄 내려서
      screen->paste(zero, 0, left); //0번째 행 0으로
    }
    delete checkBLk;
    checkBLk=nullptr; 
  }
  delete zero;
  zero=nullptr;
}
  
/**************************************************************/
/******************** Tetris Main Loop ************************/
/**************************************************************/

#define SCREEN_DY  10
#define SCREEN_DX  10
#define SCREEN_DW  1

#define ARRAY_DY (SCREEN_DY + 4*SCREEN_DW)
#define ARRAY_DX (SCREEN_DX + 6*SCREEN_DW)

int arrayScreen[ARRAY_DY][ARRAY_DX] = {
  { 70, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 70},
  { 70, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 70},
  { 70, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 70},
  { 70, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 70},
  { 70, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 70},
  { 70, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 70},
  { 70, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 70},
  { 70, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 70},
  { 70, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 70},
  { 70, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 70},
  { 70, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 70},
  { 70, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 70},
  { 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70},
  { 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70} 
};

int arrayBlk[3][3] = {
  { 0, 1, 0 },
  { 1, 1, 1 },
  { 0, 0, 0 },
};

int main(int argc, char *argv[]) {
  char key;
  int blkType;
  int top = 0, left = 4;
  int crash=0; //충돌확인 변수
  Matrix *setOfBlockObjects[7][4];
  srand((unsigned int)time(NULL));

  for(int t=0; t<1; t++){
    for(int d=0; d<4; d++){
      setOfBlockObjects[t][d]=new Matrix (setOfBlockArrays[4*t+d],2,2); // new로 할당받는 변수는 포인터니까 setOFObjects는 Matrix의 포인터 변수이다. &클래스의 생성자함수에서 매개변수 형식보고 따옴
    }
  }
  for(int t=1; t<6; t++){
    for(int d=0; d<4; d++){
    setOfBlockObjects[t][d]=new Matrix (setOfBlockArrays[4*t+d],3,3); 
    } 
  }
  for(int t=6; t<7; t++){
    for(int d=0; d<4; d++){
    setOfBlockObjects[t][d]=new Matrix (setOfBlockArrays[4*t+d],4,4);
    } 
  }

  // Matrix A((int*) arrayBlk,3,3);
  // Matrix B(A);
  // Matrix C(A);
  // Matrix D;
  // D=A+B+C;
  // cout << D << endl;
  // exit(0);
  int idxBlockDegree= rand() %MAX_BLK_DEGREES;
  int idxBlockType= rand() %MAX_BLK_TYPES;
  

  Matrix *iScreen = new Matrix((int *) arrayScreen, ARRAY_DY, ARRAY_DX);
  Matrix *currBlk = setOfBlockObjects[idxBlockType][idxBlockDegree];
  Matrix *tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx()); //아무것도 없는 iscreen에서 curr만한 블럭 뜯어내기
  Matrix *tempBlk2 = tempBlk->add(currBlk); //(5,4)일때 tempBlk란 add된 새로운 객체생성, 이전 객체의 주소값이 없어져 잃어버림-> 새로운 객체를 tempBlk2로 바꿈
  delete tempBlk; 
  /*Matrix W, X, Y, Z;
  Z=X+Y+W; //add를 여러개 한번에, 코드 짧아짐 굿*/
  // *tempBlk2= *tempBlk + *currBlk; //힙할당을 했기 때문에 포인트변수 , 깔끔하려면 스택할당

  Matrix *oScreen = new Matrix(iScreen);
  oScreen->paste(tempBlk2, top, left);
  delete tempBlk2;
  drawScreen(oScreen, SCREEN_DW); //테트리스 블럭출현
  delete oScreen;

  while ((key = getch()) != 'q') {
    switch (key) {
      case 'a': left--; break;
      case 'd': left++; break;
      case 's': top++; break;
      case 'w': {idxBlockDegree=(idxBlockDegree+1) % 4; currBlk = setOfBlockObjects[idxBlockType][idxBlockDegree];} break;
      case ' ': while(1){
                  top++;
                  tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
                  tempBlk2 = tempBlk->add(currBlk);
                  delete tempBlk;
                  if(tempBlk2->anyGreaterThan(1))  break;
                  delete tempBlk2;
                }
                delete tempBlk2;  
                break;
      default: cout << "wrong key input" << endl;
    }
    tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx()); //hard drop말고 다른 case출력들
    tempBlk2 = tempBlk->add(currBlk);
    delete tempBlk;

    if (tempBlk2->anyGreaterThan(1)){
      delete tempBlk2;
      switch (key) {
        case 'a': left++; break; //벽에 충돌하면 블록의 값이 바뀌는(빈상자에서 겹치면 1, 채워진 상자에서 겹치면 2됨) 걸 이용해 벽에 충돌하면 제자리로 돌려줌
        case 'd': left--; break;
        case 's': {crash=1;
                   top--;
                  } break;
        case 'w': {idxBlockDegree=(idxBlockDegree+3) % 4; //돌면서 충돌할때 충돌하면 반대로 돌리기
                  currBlk = setOfBlockObjects[idxBlockType][idxBlockDegree];} break;
        case ' ': {crash=1;
                  top--;
                } 
                  break;
      }
      tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx()); //방향키 누르면 움직이도록!
      tempBlk2 = tempBlk->add(currBlk);
      delete tempBlk;
    }

    oScreen = new Matrix(iScreen);
    oScreen->paste(tempBlk2, top, left);
    delete tempBlk2;
    drawScreen(oScreen, SCREEN_DW); //테트리스 블럭출현

    if(crash>0){ //새로운 블럭 생성
      idxBlockDegree= rand() %MAX_BLK_DEGREES;
      idxBlockType= rand() %MAX_BLK_TYPES;
      currBlk = setOfBlockObjects[idxBlockType][idxBlockDegree];
      deleteFullLines(oScreen);
      delete iScreen;
      iScreen= new Matrix(oScreen);
      top=0; left=4;
      tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
      tempBlk2 = tempBlk->add(currBlk);
      delete tempBlk;
      delete oScreen;
      if (tempBlk2->anyGreaterThan(1)){
          oScreen = new Matrix(iScreen);
          oScreen->paste(tempBlk2, top, left);
          drawScreen(oScreen, SCREEN_DW);
          delete oScreen;
          cout << "Game over" << endl;
          delete tempBlk2;
          goto skip;  
      }
      oScreen = new Matrix(iScreen);
      oScreen->paste(tempBlk2, top, left);
      delete tempBlk2;
      drawScreen(oScreen, SCREEN_DW);
      delete oScreen;
      crash=0; //다른 케이스에서 안걸리게 초기화!!!!!!!!!!
      
    }
    else delete oScreen;
  }

  

  skip:
    delete iScreen;
    for (int t=0; t<MAX_BLK_TYPES; t++){
      for (int d=0; d<MAX_BLK_DEGREES; d++){
        delete setOfBlockObjects[t][d];
      } //setOfBlockObjects 배열(28개 객체들을 가리키는 포인터 배열)을 이용해서 28개 객체를 소멸시키면, 당연히 currBlk 이 가리키는 객체(28개 중의 하나)도 당연히 소멸됨
    }
    cout << "(nAlloc, nFree) = (" << Matrix::get_nAlloc() << ',' << Matrix::get_nFree() << ")" << endl;  
    cout << "Program terminated!" << endl;


  return 0;
}

 