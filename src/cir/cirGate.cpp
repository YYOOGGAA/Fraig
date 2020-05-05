/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include <vector>
#include <algorithm>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

size_t
Edge::ChangeEdge2Num()
{
   int id = connect->getGateId();
   if(invert) return size_t(2*id+1);
   return size_t(2*id);
}

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() 
{
   int Idl = 1;
   int linel = 1;
   int space;
   int k = gateId;
   while(k/10 != 0) {Idl++; k /= 10;}
   k = getLineNo();
   while(k/10 != 0) {linel++; k /= 10;}

   for(int i = 0; i < 80; i++)
   {
      cout << "=";
   }
   cout << endl;
   cout << "= " << getTypeStr() << "(" << gateId << ")";
   if(symbolExist())
   {
      cout << "\"" << gateSymbol << "\"";
   }
   cout << ", line " << getLineNo() << endl;

   findPartner();
   cout << "= FECs:";
   for(int i = 0; i < fecPartner.size(); i++)
   {
   	  cout << " ";
      if(fecPartner[i] % 2 == 1) cout << "!";
      cout << fecPartner[i]/2;
    
   }
   cout << endl;

   convertSimValue();
   cout << "= Value: ";
   for(int i = 0; i < simbit.size(); i++)
   {
   		if(i % 8 == 0 && i != 0) cout << "_";
   		cout << simbit[i];
   }
   cout << endl;
   
   for(int i = 0; i < 80; i++)
   {
      cout << "=";
   }
   cout << endl;
}

void
CirGate::reportFanin(int level)
{
   assert (level >= 0);
   flevel = level;
   preorderCirWalk_in(fanin,level,gateId,getTypeStr(),false,color);
   colorIn(fanin,level,gateId,getTypeStr(),false,color);
}

void
CirGate::reportFanout(int level)
{
   assert (level >= 0);
   flevel = level;
   preorderCirWalk_out(fanout,level,gateId,getTypeStr(),false,color);
   colorOut(fanout,level,gateId,getTypeStr(),false,color);
}

void
CirGate::preorderCirWalk_in(vector<Edge*> fin, int l, int id, string type, bool inv, string& gatecolor)
{
   CirGate* in;
   if(l < 0) return;

   for(int i = 0; i < flevel-l; i++) cout << "  ";
   if(inv == true) cout << "!";
   cout << type << " " << id;
   if(gatecolor == "gray" && l != 0 && !fin.empty())
   {
      cout << " (*)" << endl;
      return;
   }
   cout << endl;
   if(l != 0) gatecolor = "gray";

   if(fin.empty()) return;
   for(int i = 0; i < fin.size(); i++)
   {
      in = fin[i]->getConnect();
      preorderCirWalk_in(in->fanin, l-1, in->gateId, in->getTypeStr(), 
   	                     fin[i]->getInv(), in->color);
   }
}

void
CirGate::colorIn(vector<Edge*> fin, int l, int id, string type, bool inv, string& gatecolor)
{
   CirGate* in;
   if(l < 0) return;
   if(gatecolor == "white" && l != 0 && !fin.empty()) return;
   if(l != 0) gatecolor = "white";
   if(fin.empty()) return;
   for(int i = 0; i < fin.size(); i++)
   {
      in = fin[i]->getConnect();
      colorIn(in->fanin, l-1, in->gateId, in->getTypeStr(), 
   	          fin[i]->getInv(), in->color);
   }
}

void
CirGate::preorderCirWalk_out(vector<Edge*> fout, int l, int id, string type, bool inv, string& gatecolor)
{
   CirGate* out;
   if(l < 0) return;

   for(int i = 0; i < flevel-l; i++) cout << "  ";
   if(inv == true) cout << "!";
   cout << type << " " << id ;
   if(gatecolor == "gray" && l != 0 && !fout.empty())
   {
      cout << " (*)" << endl;
      return;
   }
   cout << endl;
   if(l != 0) gatecolor = "gray";

   if(fout.empty()) return;
   for(int i = 0; i < fout.size(); i++)
   {
   	  out = fout[i]->getConnect();
      preorderCirWalk_out(out->fanout, l-1, out->gateId, out->getTypeStr(), 
   	                      fout[i]->getInv(), out->color);
   }
}

void
CirGate::colorOut(vector<Edge*> fout, int l, int id, string type, bool inv, string& gatecolor)
{
   CirGate* out;
   if(l < 0) return;
   if(gatecolor == "white" && l != 0 && !fout.empty()) return;
   if(l != 0) gatecolor = "white";
   if(fout.empty()) return;
   for(int i = 0; i < fout.size(); i++)
   {
   	  out = fout[i]->getConnect();
      colorOut(out->fanout, l-1, out->gateId, out->getTypeStr(), 
   	           fout[i]->getInv(), out->color);
   }
}

void
CirGate::convertSimValue()
{
	size_t num = sizeof(size_t)*8;
	size_t q = simvalue;
	while(num != 0)
	{
		size_t bit = q % 2;
		simbit.push_back(bit);
		q = q / 2;
		num--;
	}
}

void
CirGate::findPartner()
{
	int size = cirMgr->fecList.size();
	for(int i = 0; i < size; i++)
	{
		bool found1 = false;
		bool found2 = false;
		for(int j = 0; j < cirMgr->fecList[i]->size(); j++)
		{
			CirGate* compare = (*cirMgr->fecList[i])[j];
			if(found1 == true || found2 == true)
			{
				if(found1 && compare != this)
				{
					if(size_t(compare) % 2 == 0)
					   fecPartner.push_back(2*(compare->getGateId()));
					else
					{
						size_t g = size_t(compare)-1;
						CirGate* valid = (CirGate*)g;
						fecPartner.push_back(2*(valid->getGateId())+1);
					}
				}
				else if(found2 && size_t(this) != (size_t(compare)-1))
				{
					if(size_t(compare) % 2 == 0)
					   fecPartner.push_back(2*(compare->getGateId())+1);
					else
					{
						size_t g = size_t(compare)-1;
						CirGate* valid = (CirGate*)g;
						fecPartner.push_back(2*(valid->getGateId()));
					}
				}
			}
			else if(compare == this)
			{
				j = -1;
				found1 = true;
			}
			else if(size_t(this) == size_t(compare)-1)
			{
				j = -1;
				found2 = true;
			}
		}
	}
}
