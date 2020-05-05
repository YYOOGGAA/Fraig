/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;
class Edge;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------

class Edge
{
public:
   Edge(CirGate* c, bool inv): 
      connect(c), invert(inv)  {}
   ~Edge() {}
   
   CirGate* getConnect() {return connect;}
   bool getInv() {return invert;}
   void setInv(bool i) {invert = i;}
   size_t ChangeEdge2Num();
   
private:
   CirGate* connect;
   bool invert;
};

class CirGate
{
public:
   CirGate(int id, unsigned l, string type) : 
      gateId(id), lineNo(l), typeStr(type), color("white"), pre(-1), inDFS(false) {}
   virtual ~CirGate() {}
   
   friend class CirMgr;
   // Basic access methods
   string getTypeStr() const { return typeStr; }
   unsigned getLineNo() const { return lineNo; }
   int  getGateId() const {return gateId;}
   bool getInDFS() const {return inDFS;}
   size_t getSimValue() const {return simvalue;}

   bool symbolExist() const{if(gateSymbol != "\0") return true; return false;}
   virtual bool isAig() const { return false; }

   virtual Fanins HashKey() {return Fanins(0,0);}

   // Printing functions
   virtual void printGate() const = 0;
   void reportGate();
   void reportFanin(int level);
   void reportFanout(int level);

private:
   string typeStr;
   int gateId;
   int flevel;
   string gateSymbol;
   unsigned lineNo;
   bool inDFS;

   size_t simvalue;
   vector<size_t> simbit;
   IdList fecPartner;


   //DFS
   string color;
   int pre;
   void preorderCirWalk_in(vector<Edge*>, int, int, string, bool, string&);
   void preorderCirWalk_out(vector<Edge*>, int, int, string,bool, string&);
   void colorIn(vector<Edge*>, int, int, string type, bool, string&);
   void colorOut(vector<Edge*>, int, int, string, bool, string&);

   void convertSimValue();
   void findPartner();

protected:
   vector<Edge*> fanin;
   vector<Edge*> fanout;
};

class CirPI : public CirGate
{
public:
   CirPI(int id, unsigned l): CirGate(id, l, "PI") {}
   ~CirPI() {}

   void printGate() const {}
};

class CirPO : public CirGate
{
public:
   CirPO(int id, unsigned l): CirGate(id, l, "PO") {}
   ~CirPO() {}

   void printGate() const {}

};

class CirAIG : public CirGate
{
public:
   CirAIG(int id, unsigned l): CirGate(id, l, "AIG") {}
   ~CirAIG() {}

   void printGate() const {}
   Fanins HashKey() 
   {
    size_t f1 = fanin[0]->ChangeEdge2Num();
    size_t f2 = fanin[1]->ChangeEdge2Num();
    return Fanins(f1,f2);
   }

   bool isAig() const {return true;}

};

class CirConst : public CirGate
{
public:
   CirConst(): CirGate(0, 0, "CONST") {}
   ~CirConst() {}

   void printGate() const {}

};

class CirUnDef : public CirGate
{
public:
   CirUnDef(int id): CirGate(id, 0, "UNDEF") {}
   ~CirUnDef() {}

   void printGate() const {}

};


#endif // CIR_GATE_H
