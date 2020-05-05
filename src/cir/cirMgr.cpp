/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include <algorithm>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   fstream file;
   file.open(fileName);
   if (!file) return false;
   string k;
   int m,i,l,o,a;
   file >> k;
   file >> m >> i >> l >> o >> a;
   header.insert(pair<char, int>('M', m));
   header.insert(pair<char, int>('I', i));
   header.insert(pair<char, int>('O', o));
   header.insert(pair<char, int>('A', a));

   readIO(file, header['I'], input);
   readIO(file, header['O'], output);
   readAnd(file, header['A']);

   createGate();

   readSymbol(file, _symbol);
   
   connectGate();

   DFS();

   file.close();
   return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   int total = header.at('I') + header.at('O') + header.at('A');
   cout << endl;
   cout << "Circuit Statistics" << endl;
   cout << "==================" << endl;
   cout << setw(5) << "PI "     << setw(11) << header.at('I') << endl;
   cout << setw(5) << "PO "     << setw(11) << header.at('O') << endl;
   cout << setw(5) << "AIG"     << setw(11) << header.at('A') << endl;
   cout << "------------------" << endl;
   cout << setw(7) << "Total"   << setw(9)  << total;
   cout << endl;
}

void
CirMgr::printNetlist() const
{
   cout << endl;
   for(int i = 0; i < _dfsList.size(); i++)
   {
      cout << "[" << i << "] ";
      cout << _dfsList[i]->getTypeStr();
      if(_dfsList[i]->getTypeStr() == "PI")
      {
         cout << "  " << _dfsList[i]->gateId;
      }
      if(_dfsList[i]->getTypeStr() == "AIG")
      {
         cout << " " << _dfsList[i]->gateId << " ";
         Edge* edge1 = _dfsList[i]->fanin[0];
         Edge* edge2 = _dfsList[i]->fanin[1];
         if(edge1->getConnect()->getTypeStr() == "UNDEF")
            cout << "*";
         if(edge1->getInv() == true)
            cout << "!";
         cout << edge1->getConnect()->gateId << " ";

         if(edge2->getConnect()->getTypeStr() == "UNDEF")
            cout << "*";
         if(edge2->getInv() == true)
            cout << "!";
         cout << edge2->getConnect()->gateId;
      }
      else if(_dfsList[i]->getTypeStr() == "PO")
      {
         cout << "  " << _dfsList[i]->gateId << " ";
         Edge* edge1 = _dfsList[i]->fanin[0];
         if(edge1->getInv() == true)
            cout << "!";
         cout << edge1->getConnect()->gateId;
      }
      else if(_dfsList[i]->getTypeStr() == "CONST")
      {
         cout << _dfsList[i]->gateId;
      }
      if(_dfsList[i]->symbolExist())
      {
         cout << " (" << _dfsList[i]->gateSymbol << ")";
      }
      cout << endl;
   }
/*
   cout << endl;
   for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _dfsList[i]->printGate();
   }
*/
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(int i = 0; i < header.at('I'); i++)
   {
      cout << " " << input[i]/2;
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(int i = 0; i < header.at('O'); i++)
   {
      cout << " " << header.at('M')+1+i;
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   if(!UDId.empty())
   {
      cout << "Gates with floating fanin(s):";
      for(int i =0; i < UDId.size(); i++)
      {
         if(!ric[UDId[i]]->fanout.empty())
         {
            for(int j = 0; j < ric[UDId[i]]->fanout.size(); j++)
               cout << " " << ric[UDId[i]]->fanout[j]->getConnect()->gateId;
         }
      }
      cout << endl;
   }
   if(!notUsedGate.empty())
   {
      cout << "Gates defined but not used  :";
      for(int i = 0; i < notUsedGate.size(); i++)
      {
         cout << " " << notUsedGate[i];
      }
      cout << endl;
   }
}

void
CirMgr::printFECPairs() const
{
   for(int i = 0; i < fecList.size(); i++)
   {
      cout << "[" << i << "]";
      bool inv = false;
      for(int j = 0; j < fecList[i]->size(); j++)
      {
         size_t g = size_t((*fecList[i])[j]);
         cout << " " ;
         if(size_t((*fecList[i])[0]) % 2 != 0) 
         {
            inv = true;
          }
         if(g % 2 == 0)
         {
            if(inv) cout << "!";
            cout << (*fecList[i])[j]->getGateId();
         }
         else
         {
            CirGate* valid = (CirGate*)(g-1);
            if(!inv) cout << "!";
            cout << valid->getGateId();
         }
      }
      cout << endl;
   }

}

void
CirMgr::writeAag(ostream& outfile) const
{
   int aig = header.at('A');
   for(int i = 0; i < ric.size(); i++)
   {
      if(ric[i]->isAig())
      {
        if(!ric[i]->getInDFS()) aig--;
      }
   }

   outfile << "aag "<< header.at('M') << " " << header.at('I') << " 0 ";
   outfile << header.at('O') << " " << aig << endl;
   for(int i = 0; i < header.at('I'); i++)
   {
      outfile << input[i] << endl;
   }
   for(int i = 0; i < header.at('O'); i++)
   {
      outfile << output[i] << endl;
   }
   for(int i = 0; i < _dfsList.size(); i++)
   {
      if(_dfsList[i]->getTypeStr() == "AIG")
      {
         outfile << (_dfsList[i]->gateId)*2 << " ";
         outfile << _and.at((_dfsList[i]->gateId)*2).first << " ";
         outfile << _and.at((_dfsList[i]->gateId)*2).second << endl;
      }
   }
   for(int i = 0; i < _symbol.size(); i++)
   {
      outfile << _symbol[i] << endl;
   }
   outfile << 'c' << endl;
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) 
{
   vector<CirGate*> _input;
   vector<int> aig;
   aig.push_back(g->getGateId());
   DFS4write(g,_input,aig);
   resetColor();
   outfile << "aag "<< header.at('M') << " " << _input.size() << " 0 1 ";
   outfile << aig.size()-1 << endl;

   for(int i = _input.size()-1; i >= 0; i--)
   {
      outfile << 2*(_input[i]->getGateId()) << endl;
   }

   outfile << 2*(g->getGateId()) << endl;

   for(int i = aig.size()-1; i >= 0; i--)
   {
       outfile << (aig[i])*2 << " ";
       outfile << _and.at((aig[i])*2).first << " ";
       outfile << _and.at((aig[i])*2).second << endl;
   }

   for(int i = _input.size()-1; i >= 0; i--)
   {
      outfile << "i" << _input.size()-i-1 << " ";
      outfile << _input[i]->gateSymbol << endl;
   }

   outfile << "o0 " << g->getGateId() << endl;
   outfile << "c" << endl;
}

/*********************************/
/*   Private member functions    */
/*********************************/


void 
CirMgr::readIO(fstream& file, int k, vector<int>& g)
{
   int i = 0;
   int in;
   while(i < k)
   {
      file >> in;
      g.push_back(in);
      //cout << in << " ";
      i++;
   }
   //cout << endl;
}

void 
CirMgr::readAnd(fstream& file, int k)
{
   int i = 0;
   int line = 1;
   int g, in1, in2;
   while(i < k)
   {
      file >> g >> in1 >> in2;
      _and.insert(pair<int,pair<int,int>>(g,pair<int,int>(in1,in2)));
      aigline.insert(pair<int,int>(g,line));
      i++;
      line++;
   }
}

void
CirMgr::readSymbol(fstream& file, vector<string>& sym)
{
   char symbol[256] = "\0";
   string tok1;
   string tok2;
   string numS;
   int num;
   file.getline(symbol,256);
   while(tok1 != "c" && file) 
   {
      file.getline(symbol,256);
      size_t end = myStrGetTok(symbol, tok1);
      if (tok1 != "c" && symbol[0] != 0)
      {
         end = myStrGetTok(symbol, tok2, end);
         //cerr << tok1 << " " << tok2 <<endl;
         string tok1n = tok1;
         numS = tok1n.erase(0,1);
         myStr2Int(numS,num);
         if(tok1.front() == 'i')
         {
            ric[input[num]/2]->gateSymbol = tok2;
            //cerr << tok1 << " " << tok2;
         }
         else if(tok1.front() == 'o')
         {
            ric[header['M']+num+1]->gateSymbol = tok2;
            //cerr << header['M']+num+1 << endl;
            //cerr << tok1 << " " << tok2;
         }
         sym.push_back(symbol);
      }
   }
}

void 
CirMgr::createGate()
{
   int line = 1;
   ric.resize(header['M']+1+header['O']);
   ric[0] = new CirConst();
   for(int i = 0; i < header['I']; i++)
   {
      line++;
      //gatePI.push_back(new CirPI(input[i]/2, line));
      ric[input[i]/2] = new CirPI(input[i]/2, line);
      PI.push_back(ric[input[i]/2]);
      //cout << "PI " << gatePI[i]->gateId << " " << line << endl;
   }
   for(int i = 0; i < header['O']; i++)
   {
      line++;
      //gatePO.push_back(new CirPO(header['M']+1+i,line));
      ric[header['M']+1+i] = new CirPO(header['M']+1+i,line);
      PO.push_back(ric[header['M']+1+i]);
      //cout << "PO " << gatePO[i]->gateId << " " << line << endl;
   }
   
   map<int,pair<int,int>>::iterator it = _and.begin();
   CirGate* aig;
   for(int i = 0; i < header['A']; i++)
   {
      int newline = line + aigline[it->first];
      aig = new CirAIG((it->first)/2, newline);
      //gateAIG.push_back(aig);
      ric[(it->first)/2] = aig;
      //cout << "AIG " << aig->gateId << " " << newline << endl;
      it++;
   }
   it = _and.begin();
   CirGate* ud;
   for(int i = 0; i < header['A']; i++)
   {
      if(ric[((it->second).first)/2] == 0 && ((it->second).first)/2 != 0)
      {
         ud = new CirUnDef(((it->second).first)/2);
         UDId.push_back(((it->second).first)/2);
         ric[((it->second).first)/2] = ud;
         //cout << "UNDEF " << ric[((it->second).first)/2]->gateId << " " << ric[((it->second).first)/2]->lineNo << endl;
      }
      if(ric[((it->second).second)/2] == 0 && ((it->second).second)/2 != 0)
      {
         ud = new CirUnDef(((it->second).second)/2);
         UDId.push_back(((it->second).second)/2);
         ric[((it->second).second)/2] = ud;
         //cout << "UNDEF " << ric[((it->second).second)/2]->gateId << " " << ric[((it->second).second)/2]->lineNo << endl;
      }
      it++;
   }
   sort(UDId.begin(),UDId.begin()+UDId.size());
}

void 
CirMgr::connectGate()
{
   Edge* edge1;
   Edge* edge2;
   for(int i = 0; i < header['O']; i++)
   {
      if(output[i] % 2 != 0)
      {
         edge1 = new Edge(ric[output[i]/2], true);
         edge2 = new Edge(ric[header['M']+1+i], true);
      }
      else 
      {
         edge1 = new Edge(ric[output[i]/2], false);
         edge2 = new Edge(ric[header['M']+1+i], false);
      }
      ric[header['M']+1+i]->fanin.push_back(edge1);
      ric[output[i]/2]->fanout.push_back(edge2);
   }

   map<int,pair<int,int>>::iterator it = _and.begin();
   for(int i = 0; i < header['A']; i++)
   {
      CirGate* in1 = ric[(it->second).first/2];
      CirGate* in2 = ric[(it->second).second/2];
      if((it->second).first % 2 != 0)
      {
         edge1 = new Edge(in1, true);
         edge2 = new Edge( ric[(it->first)/2], true);
      }
      else
      { 
         edge1 = new Edge(in1, false);
         edge2 = new Edge(ric[(it->first)/2], false);
      }
      ric[(it->first)/2]->fanin.push_back(edge1);
      in1->fanout.push_back(edge2);

      if((it->second).second % 2 != 0)
      {
         edge1 = new Edge(in2, true);
         edge2 = new Edge(ric[(it->first)/2], true);
      }
      else 
      {
         edge1 = new Edge(in2, false);
         edge2 = new Edge(ric[(it->first)/2], false);
      }
      ric[(it->first)/2]->fanin.push_back(edge1);
      in2->fanout.push_back(edge2);
      it++;
   }
   gateNotUsed();

}

void 
CirMgr::gateNotUsed()
{
   for(int i = 1; i < header['M']+1; i++)
   {
      if(ric[i] != 0)
      {
         if(ric[i]->fanout.empty())
            notUsedGate.push_back(i);
      }
   }
   sort(notUsedGate.begin(),notUsedGate.begin()+notUsedGate.size());
}

void
CirMgr::DFS(CirGate* start)
{
   if(start->color == "black") return;
   if(start->color == "white")//not discovered yet
   {
      start->color = "gray";
   }
   for(int i = 0; i < start->fanin.size(); i++)
   {
      CirGate* _new = start->fanin[i]->getConnect();
      if(_new->color == "white" && _new->typeStr != "UNDEF")
      {
         _new->pre = start->gateId;
         _new->color = "gray";
         DFS(_new);
      }
   }
   start->color = "black";
   _dfsList.push_back(start);
   start->inDFS = true;
}

void
CirMgr::DFS()
{
   resetDFS();

   for(int i = 0; i < header['O']; i++)
   {
      DFS(ric[header['M']+1+i]);
   }

   resetColor();
}

void
CirMgr::resetColor()
{
   for(int i = 0; i < _dfsList.size(); i++)
   {
      _dfsList[i]->color = "white";
   }
}

void
CirMgr::resetDFS()
{
   for(int i = 0; i < _dfsList.size(); i++)
   {
      if(_dfsList[i] != 0)
        _dfsList[i]->inDFS = false;
   }
   _dfsList.clear();
}

void
CirMgr::DFS4write(CirGate*& start, vector<CirGate*>& in, vector<int>& aig)
{
   if(start->color == "black") return;
   if(start->color == "white")//not discovered yet
   {
      start->color = "gray";
   }
   for(int i = 0; i < start->fanin.size(); i++)
   {
      CirGate* _new = start->fanin[i]->getConnect();
      if(_new->color == "white" && _new->typeStr != "UNDEF")
      {
         _new->color = "gray";
         if(_new->isAig()) aig.push_back(_new->getGateId());
         else in.push_back(_new);
         DFS4write(_new,in,aig);
      }
   }
   start->color = "black";
}

