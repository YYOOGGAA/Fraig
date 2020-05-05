/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_DEF_H
#define CIR_DEF_H

#include <vector>
#include "myHashMap.h"
#include "cirGate.h"

using namespace std;

// TODO: define your own typedef or enum

class CirGate;
class CirMgr;
class Moni;

typedef vector<CirGate*>           GateList;
typedef vector<unsigned>           IdList;
typedef vector<CirGate*>           FECGrp;

enum GateType
{
   UNDEF_GATE = 0,
   PI_GATE    = 1,
   PO_GATE    = 2,
   AIG_GATE   = 3,
   CONST_GATE = 4,

   TOT_GATE
};
class SatSolver;

class Fanins
{
public:
   Fanins(size_t f1, size_t f2) : fanin1(f1), fanin2(f2) {}
   ~Fanins() {}

   size_t operator() () const
   { 
      size_t a = fanin1*1.625;
      size_t b = fanin2*1.625;
      return (a ^ b); 
   } 
   bool operator == (const Fanins& k) const 
   { 
     if(fanin1 / 2 == k.fanin1 / 2 && fanin2 / 2 == k.fanin2 / 2)
        if((fanin1 % 2 == k.fanin1 % 2) && (fanin2 % 2 == k.fanin2 % 2))
          return true;
     if(fanin1 / 2 == k.fanin2 / 2 && fanin2 / 2 == k.fanin1 / 2)
        if((fanin1 % 2 == k.fanin2 % 2) && (fanin2 % 2 == k.fanin1 % 2))
          return true;
     return false; 
   }

private:
   size_t fanin1;
   size_t fanin2;

};

class Moni
{
public:
   Moni(size_t m) : moni(m) {}
   ~Moni() {}

   size_t operator() () const{return moni;}
   bool operator == (const Moni& k) const {return (moni == k.moni);}
   
   size_t getMoni() {return moni;}
   void setMoni(size_t& m) {moni = m;}

private:
   //GateList fecGate;
   size_t moni;
};


#endif // CIR_DEF_H
