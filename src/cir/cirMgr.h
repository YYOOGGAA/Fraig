/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include "cirGate.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr() {}
   ~CirMgr() {} 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const {
      if(gid > ric.size() || ric[gid] == 0) return 0;
      return ric[gid];
    }

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*);

   vector<FECGrp*>  fecList;

private:
   ofstream           *_simLog;

   bool simulated;

   map<char,int> header;
   map<int,pair<int,int>> _and;
   map<int,int> aigline;
   vector<string> _symbol;
   vector<int> input;
   vector<int> output;
   vector<vector<size_t>> PObuf;

   IdList   notUsedGate;
   IdList   UDId;

   GateList ric;
   GateList PI;
   GateList PO;
   GateList _dfsList;

   void readIO(fstream&, int, vector<int>&);
   void readAnd(fstream&, int);
   void readSymbol(fstream&, vector<string>&);
   void createGate();
   void connectGate();
   void gateNotUsed();

   void DFS(CirGate*);
   void DFS();
   void resetDFS();
   void resetColor();
   void DFS4write(CirGate*&, vector<CirGate*>&, vector<int>&);

   void deleteFanin(CirGate*&, CirGate*);
   void deleteFanout(CirGate*&, CirGate*);
   void deleteAllFanin(CirGate*&);
   void deleteAllFanout(CirGate*&);
   void deleteUNDEF(int);
   void reconnect(CirGate*&, CirGate*&);

   void simulate(vector<size_t>&);
   void writeSim(vector<string>&, int&);
   void fuck();
   void divideFEC();
   void initializeFEC();
   void collectValidFecGrp(HashMap<Moni,FECGrp*>&, vector<FECGrp*>&);
   void sortFEC();
};

#endif // CIR_MGR_H
