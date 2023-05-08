#include "CTetris.h"

using namespace std;

///**************************************************************
/// static member variables and functions
///**************************************************************

Matrix *** CTetris::setOfColorBlockObjects = NULL;

void CTetris::init(int **setOfColorBlockArrays, int nTypes, int nDegrees) {
  if (setOfColorBlockObjects != NULL) // already allocated?
    deinit();

  Tetris::init(setOfColorBlockArrays, nTypes, nDegrees); // call superclass' function
  
  setOfColorBlockObjects=new Matrix**[numTypes];

  for (int t=0; t<numTypes; t++)
    setOfColorBlockObjects[t]=new Matrix *[numDegrees];

  for (int t=0; t<numTypes; t++){
    int *array =setOfColorBlockArrays[numDegrees*t];
    int idx, size;
    for (idx=0; array[idx]!=-1; idx++);
    for (size=0; size*size<idx; size++);
    wallDepth= (size>wallDepth ? size : wallDepth);
    for (int d=0; d<numDegrees; d++){
      int *array2=new int[size*size+1];
      int k;
      for (k=0; k<size*size; k++)
        array2[k]=(setOfColorBlockArrays[numDegrees*t+d][k]==0 ? 0 : 10*(t+1)); //블럭만들때 ct가 0부터 시작이라 1더해줌
      array2[k]=-1;
      setOfColorBlockObjects[t][d]=new Matrix (array2, size, size);
      delete[] array2;
      //cout << *setOfColorBlockObjects[t][d] <<endl;
    }
  }

}

void CTetris::deinit(void) {
  Tetris::deinit(); // call superclass' function

  for (int t = 0; t < numTypes; t++)
    for (int d = 0; d < numDegrees; d++)
      delete setOfColorBlockObjects[t][d]; // deallocate matrix objects

  for (int t = 0; t < numTypes; t++)
    delete [] setOfColorBlockObjects[t]; // deallocate 1d array of Matrix pointers

  delete []  setOfColorBlockObjects; // deallocate 1d array of pointers to Matrix pointer arrays

  setOfColorBlockObjects = NULL;
}

///**************************************************************
/// dynamic member variables and functions
///**************************************************************

/// constructors and destructor

CTetris::CTetris(int cy, int cx) : Tetris(cy, cx) {
  iCScreen = new Matrix(iScreen);
  oCScreen = new Matrix(oScreen);
  currCBlk = NULL;
}

CTetris::~CTetris() {
  delete iCScreen;
  delete oCScreen;
}

/// mutators
TetrisState CTetris::accept(char key) {

  TetrisState _state = Tetris::accept(key); // call superclass' function
  
  currCBlk=setOfColorBlockObjects[type][degree];
  Matrix *tempCBlk = iCScreen->clip(top, left, top + currCBlk->get_dy(), left + currCBlk->get_dx());
  Matrix *tempCBlk2 = tempCBlk->add(currCBlk);
  delete tempCBlk;
  oCScreen->paste(iCScreen, 0, 0);
  oCScreen->paste(tempCBlk2, top, left);
  delete tempCBlk2;

  if (_state == TetrisState:: NewBlock){
    oCScreen = deleteFullLines(oCScreen, currCBlk, top, wallDepth);
    iCScreen->paste(oCScreen,0,0);
  }


  // you can use the following code if you want to
  // oCScreen = deleteFullLines(oCScreen, currCBlk, top, wallDepth);

  return _state; // not reachable
}
