/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
	HashMap<Fanins, CirGate*> hash(_dfsList.size());
	for(int i = 0; i < _dfsList.size(); i++)
	{
		if(_dfsList[i]->isAig())
		{
			Fanins f = _dfsList[i]->HashKey();
			CirGate* save;
			if(hash.query(f, save))
			{
				//say bye to _dfsList[i]
				cerr << "Strashing: ";
				cerr << save->gateId << " merging ";
				cerr << _dfsList[i]->gateId << "..." << endl;
				deleteAllFanin(_dfsList[i]);
				reconnect(save,_dfsList[i]);
				while(!_dfsList[i]->fanout.empty())
				{
					delete _dfsList[i]->fanout[0];
					_dfsList[i]->fanout.erase(_dfsList[i]->fanout.begin());
				}
				ric[_dfsList[i]->gateId] = 0;
				delete _dfsList[i];
				_dfsList[i] = 0;
				--header['A'];
			}
			hash.insert(f, _dfsList[i]);
		}
	}

	notUsedGate.clear();
    gateNotUsed();
    DFS();
}

void
CirMgr::fraig()
{
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

void
CirMgr::reconnect(CirGate*& out, CirGate*& bye)
{
	for(int i = 0; i < bye->fanout.size(); i++)
	{
		CirGate* in = bye->fanout[i]->getConnect();
		Edge* fout = new Edge(in, bye->fanout[i]->getInv());
		Edge* fin  = new Edge(out, bye->fanout[i]->getInv());
		if(in->fanin[0]->getConnect() == bye)
		{
			delete in->fanin[0];
			in->fanin[0] = fin;
		}
		else 
		{
			delete in->fanin[1];
			in->fanin[1] = fin;
		}
		out->fanout.push_back(fout);
	}
}
