//============================================================================
// Name        : PersonalityTree.cpp
// Copyright   : DataSoft Corporation 2011-2012
//	Nova is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Nova is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Nova.  If not, see <http://www.gnu.org/licenses/>.
// Description : TODO
//============================================================================

#include "PersonalityTree.h"

using namespace std;

namespace Nova
{

PersonalityTree::PersonalityTree(PersonalityTable *persTable)
{
	m_root = PersonalityNode("root");
	if(persTable != NULL)
	{
		LoadTable(persTable);
	}
}

PersonalityTree::~PersonalityTree()
{
}

void PersonalityTree::LoadTable(PersonalityTable *persTable)
{
	Personality_Table *pTable = &persTable->m_personalities;
	for(Personality_Table::iterator it = pTable->begin(); it != pTable->end(); it++)
	{
		InsertPersonality(it->second);
	}
}

void PersonalityTree::InsertPersonality(Personality *pers)
{
	Personality temp = * pers;
	UpdatePersonality(&temp, &m_root);
}

void PersonalityTree::UpdatePersonality(Personality *pers, PersonalityNode *parent)
{
	string cur = pers->m_personalityClass.back();

	uint i = 0;
	for(; i < parent->m_children.size(); i++)
	{
		if(!cur.compare(parent->m_children[i].first))
		{
			break;
		}
	}

	pair<string, PersonalityNode *> *tablePair = NULL;

	//If node not found
	if(i == parent->m_children.size())
	{
		tablePair = new pair<string, PersonalityNode *>();
		tablePair->first = cur;
		tablePair->second = new PersonalityNode(cur);
	}
	else
	{
		tablePair = &parent->m_children[i];
	}

	//Insert or count port occurrences
	for(Port_Table::iterator it = pers->m_ports.begin(); it != pers->m_ports.end(); it++)
	{
		//If we found a corresponding entry
		if(tablePair->second->m_ports.find(it->first) != tablePair->second->m_ports.end())
		{
			tablePair->second->m_ports[it->first] += it->second;
		}
		else
		{
			tablePair->second->m_ports[it->first] = it->second;
		}
	}

	//Insert or count MAC vendor occurrences
	for(MAC_Table::iterator it = pers->m_vendors.begin(); it != pers->m_vendors.end(); it++)
	{
		//If we found a corresponding entry
		if(tablePair->second->m_vendors.find(it->first) != tablePair->second->m_vendors.end())
		{
			tablePair->second->m_vendors[it->first] += it->second;
		}
		else
		{
			tablePair->second->m_vendors[it->first] = it->second;
		}
	}
	pers->m_personalityClass.pop_back();

	//Recursively descend until all nodes updated
	if(!pers->m_personalityClass.empty())
	{
		UpdatePersonality(pers, tablePair->second);
	}
}

void PersonalityTree::ToString()
{
	for(uint i = 0; i < m_root.m_children.size(); i++)
	{
		RecursiveToString(m_root.m_children[i].second);
	}
}

void PersonalityTree::RecursiveToString(PersonalityNode * persNode)
{
	cout << persNode->ToString() << endl;
	for(uint i = 0; i < persNode->m_children.size(); i++)
	{
		RecursiveToString(persNode->m_children[i].second);
	}

}



}
