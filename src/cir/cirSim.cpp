/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <vector>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
}

void
CirMgr::fileSim(ifstream& patternFile)
{
	simulated =false;
	
	string p;
	int lineNum = 0;
	bool gua = false;

	while(true)
	{
		vector<size_t> pattern;
		vector<string> input;
		int time = 0;
		bool run = false;

		while(true)
		{
			if(time == sizeof(size_t)*8) break;
			patternFile >> p;
			string buf = p;

			if(!patternFile) 
			{
				if(time < sizeof(size_t)*8 && time != 0)
				{
					
					int k = time;
					while(k != sizeof(size_t)*8)
					{
						for(int j = 0; j < pattern.size(); j++)
						{
							pattern[j] <<= 1;
						}
						k++;
					}
					
					gua = true;
				}
				else run = true;
				break;
			}
			input.push_back(p);
			if(p.size() != header['I'])
			{
				cerr << "Error: ";
				cerr << "Pattern(" << p << ") length(" << p.size() << ") ";
				cerr << "does not match the number of inputs(" << header['I'] << ") ";
				cerr << "in a circuit!!" << endl;
				run = true;
				break;
			} 
			int i = 0;
			while(p.size())
			{
				if(p.front() != '1' && p.front() != '0')
				{
					cerr << "Error: ";
					cerr << "Pattern(" << buf << ") contains a non-0/1 character";
					cerr << "(" << p.front() << ")." << endl;
					run = true;
					break;
				}
				size_t bit = size_t(p.front()) - '0';
				if(pattern.size() < header['I']) pattern.push_back(bit);
				else
				{
					pattern[i] <<= 1;
					pattern[i] = pattern[i] + bit;
				}
				p.erase(p.begin());
				i++;
			}
			if(run) break;
			time++;
		}
		if(run) break;
		simulate(pattern);
		writeSim(input, time);
		cout << '\r';
		divideFEC();
		cout << '\r';
		if(gua) {cout << lineNum*sizeof(size_t)*8+time; break;}
		lineNum++;
		simulated = true;
	}
	if(!gua) {cout << lineNum*sizeof(size_t)*8;}
	cout << " patterns simulated." << endl;
	sortFEC();
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

void
CirMgr::simulate(vector<size_t>& p)
{
	for(int i = 0; i < header['I']; i++)
	{
		//cerr << p[i] << " ";
		PI[i]->simvalue = p[i];
		//cerr << PI[i]->simvalue << endl;
	}
	for(int i = 0; i < _dfsList.size(); i++)
	{
		if(_dfsList[i]->getTypeStr() == "PI") continue;

		size_t input1 = _dfsList[i]->fanin[0]->getConnect()->simvalue;
		if(_dfsList[i]->fanin[0]->getInv())
			input1 = ~input1;

		if(_dfsList[i]->getTypeStr() == "PO")
		{
			_dfsList[i]->simvalue = input1;
			continue;
		}

		size_t input2 = _dfsList[i]->fanin[1]->getConnect()->simvalue;
		if(_dfsList[i]->fanin[1]->getInv())
			input2 = ~input2;

		_dfsList[i]->simvalue = (input1) & (input2);
	}
}

void 
CirMgr::writeSim(vector<string>& p, int& t)
{
	fuck();
	/*
	for(int i = 0; i < PObuf.size(); i++)
	{
		for(int j = 0; j < PObuf[i].size(); j++)
		{
			cerr << PObuf[i][j];
		}
		cerr << endl;
	}
	*/
	//int delta = sizeof(size_t)*8 - t;
	for(int i = 0; i < p.size(); i++)
	{
		(*_simLog) << p[i] << " ";
		for(int j = 0; j < PObuf.size(); j++)
		{
			(*_simLog) << PObuf[j][sizeof(size_t)*8-1-i];
		}
		(*_simLog) << '\n';
	}
	(*_simLog).flush();
}

void
CirMgr::fuck()
{
	PObuf.clear();
	PObuf.resize(header['O']);
	for(int i = 0; i < PObuf.size(); i++)
	{
		size_t num = sizeof(size_t)*8;
		size_t q = PO[i]->simvalue;
		while(num != 0)
		{
			size_t bit = q % 2;
			PObuf[i].push_back(bit);
			q = q / 2;
			num--;
		}
	}
}

void
CirMgr::divideFEC()
{
	if(!simulated) initializeFEC();
	vector<FECGrp*> _buf;
	//cerr << fecList.size() << endl;
	for(unsigned i = 0; i < fecList.size(); i++)
	{
		HashMap<Moni,FECGrp*> hash(fecList[i]->size());
		for(unsigned j = 0; j < fecList[i]->size(); j++)
		{
			size_t value;
			if(size_t((*fecList[i])[j]) % 2 != 0)//not
			{ 
				size_t g = size_t((*fecList[i])[j]) - 1;
				CirGate* gate = (CirGate*)(g);
				//cerr << gate->getGateId() << endl;
				value = ~(gate->simvalue);
			}
			else value = (*fecList[i])[j]->simvalue;

			//cerr << (*fecList[i])[j]->getGateId() << " " << value << " ";
			
			FECGrp* _temp;
			size_t xvalue = ~value;
			if(hash.query(value,_temp))
			{
				//cerr << "1" << endl;
				_temp->push_back((*fecList[i])[j]);
				hash.update(value,_temp);
			}
			else if(hash.query(xvalue,_temp))
			{
				//cerr << "2" << endl;
				size_t g = size_t((*fecList[i])[j]) ^ 0x1;
				CirGate* gate = (CirGate*)(g);
				_temp->push_back(gate);
				hash.update(xvalue,_temp);
			}
			else
			{
				//cerr << "3" << endl;
				FECGrp* newFEC = new FECGrp;
				newFEC->push_back((*fecList[i])[j]);
				hash.insert(value,newFEC);
			}
		}
		collectValidFecGrp(hash, _buf);
		delete fecList[i];
	}
	fecList.clear();
	fecList = _buf;
	//cerr << _buf[0]->size() << endl;
	cout << "Total #FEC Group = " << fecList.size();
	cout.flush();
}

void
CirMgr::initializeFEC()
{
	if(fecList.size() != 0)
	{
		for(int i = 0; i < fecList.size(); i++)
		{
			if(fecList[i] != 0) delete fecList[i];
		}
	}
	fecList.clear();

	FECGrp* f = new FECGrp;
	f->push_back(ric[0]);
	for(int i = 0; i < _dfsList.size(); i++)
	{
		if(_dfsList[i]->isAig())
		{
			f->push_back(_dfsList[i]);
			/*
			size_t g = size_t(_dfsList[i])+1;
			CirGate* ng = (CirGate*)(g);
			f->push_back(ng);
			*/
		}

	}
	fecList.push_back(f);

}

void
CirMgr::collectValidFecGrp(HashMap<Moni,FECGrp*>& h, vector<FECGrp*>& b)
{
	HashMap<Moni,FECGrp*>::iterator it = h.begin();
	for(; it != h.end(); it++)
	{
		if((*it).second->size() > 1)
		{
			b.push_back((*it).second);
		}
		else (*it).second->clear();
	}
}

void
CirMgr::sortFEC()
{
	struct GateCompare
	{
		bool operator() (CirGate* a, CirGate* b) 
		{
			if(size_t(a) % 2 != 0)
			{
				a = (CirGate*)(size_t(a)-1);
			}
			if(size_t(b) % 2 != 0)
			{
				b = (CirGate*)(size_t(b)-1);
			}
			return (a->getGateId())<(b->getGateId());
		}
	} gateCompare;

	struct FECCompare
	{
		bool operator() (FECGrp* a, FECGrp* b) 
		{
			CirGate* ga = (*a)[0];
			CirGate* gb = (*b)[0];
			if(size_t(ga) % 2 != 0)
			{
				ga = (CirGate*)(size_t(ga)-1);
			}
			if(size_t(gb) % 2 != 0)
			{
				gb = (CirGate*)(size_t(gb)-1);
			}
			return (ga->getGateId())<(gb->getGateId());
		}
	} fecCompare;

	for(int i = 0; i < fecList.size(); i++)
	{
		sort(fecList[i]->begin(), fecList[i]->end(), gateCompare);
	}
	sort(fecList.begin(), fecList.end(), fecCompare);
}
