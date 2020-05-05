/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
	for(int i = 0; i < ric.size(); i++)
	{
		if (ric[i] == 0) continue;
		if(ric[i]->isAig() || ric[i]->getTypeStr() == "UNDEF")
		{
			if(ric[i]->getInDFS() == false)
			{
				deleteAllFanin(ric[i]);
				deleteAllFanout(ric[i]);
				cerr << "Sweeping: " << ric[i]->getTypeStr() << "("<<ric[i]->gateId<<") removed..." <<endl;
				if(ric[i]->getTypeStr() == "AIG") --header['A'];
				else deleteUNDEF(ric[i]->gateId);
				delete ric[i];
				ric[i] = 0;
			}
		}
	}

	notUsedGate.clear();
	gateNotUsed();
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
   for(int i = 0; i < _dfsList.size(); i++)
   {
   	  if(_dfsList[i]->isAig())
   	  {
   	  	bool run = false;
   	  	Edge* e;
   	  	CirGate* g1, *g2 = 0;
   	  	if (_dfsList[i]->fanin[0]->getConnect() == _dfsList[i]->fanin[1]->getConnect())
   	  	{
   	  		if(_dfsList[i]->fanin[0]->getInv() == _dfsList[i]->fanin[1]->getInv())
   	  		{
   	  		   //cerr << 1 << endl;
   	  		   e = _dfsList[i]->fanin[0];
   	  		   g1 = _dfsList[i]->fanin[1]->getConnect();
   	  		   delete _dfsList[i]->fanin[1];
   	  		   run = true;

   	  		}
   	  		else
   	  		{
   	  			//cerr << 2 << endl;
   	  			e = new Edge(ric[0],false);
   	  			g1 = _dfsList[i]->fanin[0]->getConnect();
   	  			g2 = _dfsList[i]->fanin[1]->getConnect();
   	  			delete _dfsList[i]->fanin[0];
   	  			delete _dfsList[i]->fanin[1];
   	  			run = true;
   	  		}
   	  	}
   	  	else if(_dfsList[i]->fanin[0]->getConnect()->getTypeStr() == "CONST")
   	  	{
   	  		if(_dfsList[i]->fanin[0]->getInv() == true) 
   	  		{
   	  			//cerr << 3 << endl;
   	  			e = _dfsList[i]->fanin[1];
   	  			g1 = _dfsList[i]->fanin[0]->getConnect();
   	  			delete _dfsList[i]->fanin[0];
   	  			run = true;
   	  		}
   	  		else 
   	  		{
   	  			//cerr << 4 << endl;
   	  			e = _dfsList[i]->fanin[0];
   	  			g1 = _dfsList[i]->fanin[1]->getConnect();
   	  			delete _dfsList[i]->fanin[1];
   	  			run = true;
   	  		};
   	  	}
   	  	else if(_dfsList[i]->fanin[1]->getConnect()->getTypeStr() == "CONST")
   	  	{
   	  		if(_dfsList[i]->fanin[1]->getInv() == true) 
   	  		{
   	  			//cerr << 5 << endl;
   	  			e = _dfsList[i]->fanin[0];
   	  			g1 = _dfsList[i]->fanin[1]->getConnect();
   	  			delete _dfsList[i]->fanin[1];
   	  			run = true;
   	  		}
   	  		else 
   	  		{
   	  			//cerr << 6 << endl;
   	  			e = _dfsList[i]->fanin[1];
   	  			g1 = _dfsList[i]->fanin[0]->getConnect();
   	  			delete _dfsList[i]->fanin[0];
   	  			run = true;
   	  		}
   	  	}

   	  	if(run == true)
   	  	{
   	  		CirGate* merge = e->getConnect();
   	  		deleteFanout(merge,_dfsList[i]);
   	  		if(merge->getTypeStr() == "UNDEF" && merge->fanout.empty())
   	  			deleteUNDEF(merge->gateId);

   	  		while(!_dfsList[i]->fanout.empty())
			{
				CirGate* g = _dfsList[i]->fanout[0]->getConnect();
				for(int j = 0; j < g->fanin.size(); j++)
				{
					if(g->fanin[j]->getConnect() == _dfsList[i])
					{
						bool inv;
						if(g->fanin[j]->getInv() == e->getInv())
							inv = false;
						else inv = true;

						delete g->fanin[j];
						g->fanin[j] = new Edge(merge, inv);
						Edge* _new = new Edge(g, inv);
						merge->fanout.push_back(_new);
						break;
					}
				}
				delete _dfsList[i]->fanout[0];
				_dfsList[i]->fanout.erase(_dfsList[i]->fanout.begin());
			}

			deleteFanout(g1, _dfsList[i]);

			if(g2 != 0)
			{
				deleteFanout(g2, _dfsList[i]);
			}

			cerr << "Simplifying: ";
			cerr << merge->gateId;
   	  		cerr << " merging ";
   	  		if(e->getInv()) cerr << "!";
   	  		cerr << _dfsList[i]->gateId << "..." << endl;
			--header['A'];
			ric[_dfsList[i]->gateId] = 0;
			delete _dfsList[i];
			delete e;
			_dfsList[i] = 0;

   	  	}
   	  	
   	  }
   }

   notUsedGate.clear();
   gateNotUsed();
   DFS();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/

void
CirMgr::deleteFanin(CirGate*& g, CirGate* r)
{
	for(int j = 0; j < g->fanin.size(); j++)
	{
		if(g->fanin[j]->getConnect() == r)
		{
			delete g->fanin[j];
			g->fanin.erase(g->fanin.begin()+j);
			break;
		}
	}

}

void
CirMgr::deleteFanout(CirGate*& g, CirGate* r)
{
	for(int j = 0; j < g->fanout.size(); j++)
	{
		if(g->fanout[j]->getConnect() == r)
		{
			delete g->fanout[j];
			g->fanout.erase(g->fanout.begin()+j);
			break;
		}
	}

}

void
CirMgr::deleteUNDEF(int k)
{
	for(int i = 0; i < UDId.size(); i++)
	{
		if(UDId[i] == k) 
		{
			UDId.erase(UDId.begin()+i);
			return;
		}
	}
}

void
CirMgr::deleteAllFanin(CirGate*& g)
{
	while(!g->fanin.empty())
	{
		CirGate* c = g->fanin[0]->getConnect();
		deleteFanout(c, g);
		delete g->fanin[0];
		g->fanin.erase(g->fanin.begin());
	}
}

void
CirMgr::deleteAllFanout(CirGate*& g)
{
	while(!g->fanout.empty())
	{
		CirGate* c = g->fanout[0]->getConnect();
		deleteFanin(c, g);
		delete g->fanout[0];
		g->fanout.erase(g->fanout.begin());
	}
}
