//============================================================================
// Name        : Suspect.cpp
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
// Description : A Suspect object with identifying information and traffic
//					features so that each entity can be monitored and classified.
//============================================================================/*

#include "SerializationHelper.h"
#include "FeatureSet.h"
#include "Suspect.h"
#include "Logger.h"
#include "Config.h"

#include <errno.h>
#include <sstream>

using namespace std;

namespace Nova{


Suspect::Suspect()
{
	m_hostileNeighbors = 0;
	m_classification = -1;
	m_needsClassificationUpdate = false;
	m_flaggedByAlarm = false;
	m_isHostile = false;
	m_isLive = false;
	m_lastPacketTime = 0;
	m_classificationNotes = "";

	for(int i = 0; i < DIM; i++)
	{
		m_featureAccuracy[i] = 0;
	}
}

Suspect::~Suspect()
{
}

string Suspect::GetIpString()
{
	stringstream ss;
	ss << m_id.m_interface << " ";
	ss << ((m_id.m_ip & 0xFF000000) >> 24) << ".";
	ss << ((m_id.m_ip & 0x00FF0000) >> 16) << ".";
	ss << ((m_id.m_ip & 0x0000FF00) >> 8) << ".";
	ss << ((m_id.m_ip & 0x000000FF) >> 0);
	return ss.str();
}

string Suspect::GetInterface()
{
	return m_id.m_interface;
}

string Suspect::ToString()
{
	stringstream ss;
	ss << "Suspect: "<< GetIpString() << "\n";
	ss << " Suspect is ";
	if(!m_isHostile)
		ss << "not ";
	ss << "hostile\n";
	ss <<  " Classification: ";

	if(m_classification == -2)
	{
		ss << " Not enough data\n";
	}
	else
	{
		ss << m_classification <<  "\n";
	}

	ss <<  " Hostile neighbors: " << m_hostileNeighbors << "\n";

	ss << "Classification notes: " << endl << m_classificationNotes << endl;


	for(int i = 0; i < DIM; i++)
	{
		if(Config::Inst()->IsFeatureEnabled(i))
		{
			ss << FeatureSet::m_featureNames[i] << ": " << m_features.m_features[i] << "\n";
		}
	}

	return ss.str();
}

//Just like Consume but doesn't deallocate
void Suspect::ReadEvidence(Evidence *evidence, bool deleteEvidence)
{
	if(m_id.m_ip == 0)
	{
		m_id.m_ip = evidence->m_evidencePacket.ip_src;
		m_id.m_interface = evidence->m_evidencePacket.interface;
	}

	Evidence *curEvidence = evidence, *tempEv = NULL;
	while(curEvidence != NULL)
	{
		m_unsentFeatures.UpdateEvidence(curEvidence);
		m_features.UpdateEvidence(curEvidence);
		m_id.m_interface = curEvidence->m_evidencePacket.interface;

		if(m_lastPacketTime < evidence->m_evidencePacket.ts)
		{
			m_lastPacketTime = evidence->m_evidencePacket.ts;
		}

		tempEv = curEvidence;
		curEvidence = tempEv->m_next;

		if (deleteEvidence)
		{
			delete tempEv;
		}
	}
}

//Calculates the suspect's features based on it's feature set
void Suspect::CalculateFeatures()
{
	m_features.CalculateAll();
}

// Stores the Suspect information into the buffer, retrieved using deserializeSuspect
//		buf - Pointer to buffer where serialized data will be stored
// Returns: number of bytes set in the buffer
uint32_t Suspect::Serialize(u_char *buf, uint32_t bufferSize, SerializeFeatureMode whichFeatures)
{
	uint32_t offset = 0;

	//Copies the value and increases the offset
	SerializeChunk(buf, &offset,(char*)&m_classification, sizeof m_classification, bufferSize);
	SerializeChunk(buf, &offset,(char*)&m_isHostile, sizeof m_isHostile, bufferSize);
	SerializeChunk(buf, &offset,(char*)&m_flaggedByAlarm, sizeof m_flaggedByAlarm, bufferSize);
	SerializeChunk(buf, &offset,(char*)&m_isLive, sizeof m_isLive, bufferSize);
	SerializeChunk(buf, &offset,(char*)&m_hostileNeighbors, sizeof m_hostileNeighbors, bufferSize);
	SerializeChunk(buf, &offset,(char*)&m_lastPacketTime, sizeof m_lastPacketTime, bufferSize);

	SerializeString(buf, &offset, m_classificationNotes, bufferSize);

	offset += m_id.Serialize(buf + offset, bufferSize - offset);


	//Copies the value and increases the offset
	for(uint32_t i = 0; i < DIM; i++)
	{
		SerializeChunk(buf, &offset,(char*)&m_featureAccuracy[i], sizeof m_featureAccuracy[i], bufferSize);
	}

	//Stores the FeatureSet information into the buffer, retrieved using deserializeFeatureSet
	//	returns the number of bytes set in the buffer
	offset += m_features.SerializeFeatureSet(buf+offset, bufferSize - offset);
	switch(whichFeatures)
	{
		case UNSENT_FEATURE_DATA:
		{
			if(offset + m_unsentFeatures.GetFeatureDataLength() >= SANITY_CHECK)
			{
				return 0;
			}
			offset += m_unsentFeatures.SerializeFeatureData(buf + offset, bufferSize - offset);
			break;
		}
		case MAIN_FEATURE_DATA:
		{
			if(offset + m_features.GetFeatureDataLength() >= SANITY_CHECK)
			{
				return 0;
			}
			offset += m_features.SerializeFeatureData(buf + offset, bufferSize - offset);
			break;
		}
		case ALL_FEATURE_DATA:
		{
			if(offset + m_features.GetFeatureDataLength() >= SANITY_CHECK)
			{
				return 0;
			}
			offset += m_features.SerializeFeatureData(buf + offset, bufferSize - offset);
			if(offset + m_unsentFeatures.GetFeatureDataLength() >= SANITY_CHECK)
			{
				return 0;
			}
			offset += m_unsentFeatures.SerializeFeatureData(buf + offset, bufferSize - offset);
			break;
		}
		case NO_FEATURE_DATA:
		default:
		{
			break;
		}
	}

	return offset;
}

uint32_t Suspect::GetSerializeLength(SerializeFeatureMode whichFeatures)
{
	//Adds the sizeof results for the static required fields to messageSize
	uint32_t messageSize =
		m_id.GetSerializationLength()
		+ sizeof(m_classification)
		+ sizeof(m_isHostile)
		+ sizeof(m_flaggedByAlarm)
		+ sizeof(m_isLive)
		+ sizeof(m_hostileNeighbors)
		+ sizeof(m_lastPacketTime);
	//Adds the dynamic elements to the messageSize
	for(uint32_t i = 0; i < DIM; i++)
	{
		messageSize += sizeof(m_featureAccuracy[i]) + sizeof(m_features.m_features[i]);
	}
	switch(whichFeatures)
	{
		case UNSENT_FEATURE_DATA:
		{
			messageSize += m_unsentFeatures.GetFeatureDataLength();
			break;
		}
		case MAIN_FEATURE_DATA:
		{
			messageSize += m_features.GetFeatureDataLength();
			break;
		}
		case ALL_FEATURE_DATA:
		{
			messageSize += m_features.GetFeatureDataLength();
			messageSize += m_unsentFeatures.GetFeatureDataLength();
			break;
		}
		case NO_FEATURE_DATA:
		default:
		{
			break;
		}
	}

	int sizeOfNotes = m_classificationNotes.size();
	messageSize += sizeof(sizeOfNotes) + sizeOfNotes*sizeof(char);
	return messageSize;
}

// Reads Suspect information from a buffer originally populated by serializeSuspect
//		buf - Pointer to buffer where the serialized suspect is
// Returns: number of bytes read from the buffer
uint32_t Suspect::Deserialize(u_char *buf, uint32_t bufferSize, SerializeFeatureMode whichFeatures)
{
	uint32_t offset = 0;

	DeserializeChunk(buf, &offset,(char*)&m_classification, sizeof m_classification, bufferSize);
	DeserializeChunk(buf, &offset,(char*)&m_isHostile, sizeof m_isHostile, bufferSize);
	DeserializeChunk(buf, &offset,(char*)&m_flaggedByAlarm, sizeof m_flaggedByAlarm, bufferSize);
	DeserializeChunk(buf, &offset,(char*)&m_isLive, sizeof m_isLive, bufferSize);
	DeserializeChunk(buf, &offset,(char*)&m_hostileNeighbors, sizeof m_hostileNeighbors, bufferSize);
	DeserializeChunk(buf, &offset,(char*)&m_lastPacketTime, sizeof m_lastPacketTime, bufferSize);

	m_classificationNotes = DeserializeString(buf, &offset, bufferSize);

	offset += m_id.Deserialize(buf + offset, bufferSize - offset);

	//Copies the value and increases the offset
	for(uint32_t i = 0; i < DIM; i++)
	{
		DeserializeChunk(buf, &offset,(char*)&m_featureAccuracy[i], sizeof m_featureAccuracy[i], bufferSize);
	}

	//Reads FeatureSet information from a buffer originally populated by serializeFeatureSet
	//	returns the number of bytes read from the buffer
	offset += m_features.DeserializeFeatureSet(buf+offset, bufferSize - offset);

	switch(whichFeatures)
	{
		case UNSENT_FEATURE_DATA:
		{
			FeatureSet deserializedFs;

			offset += deserializedFs.DeserializeFeatureData(buf+offset, bufferSize - offset);
			m_unsentFeatures += deserializedFs;
			break;
		}
		case MAIN_FEATURE_DATA:
		{
			FeatureSet deserializedFs;

			offset += deserializedFs.DeserializeFeatureData(buf+offset, bufferSize - offset);
			m_features += deserializedFs;
			break;
		}
		case ALL_FEATURE_DATA:
		{
			FeatureSet deserializedUnsentFs;
			FeatureSet deserializedFs;

			offset += deserializedFs.DeserializeFeatureData(buf+offset, bufferSize - offset);
			m_features += deserializedFs;

			offset += deserializedUnsentFs.DeserializeFeatureData(buf+offset, bufferSize - offset);
			m_unsentFeatures += deserializedUnsentFs;
			break;
		}
		case NO_FEATURE_DATA:
		default:
		{
			break;
		}
	}
	return offset;
}

//Returns a copy of the suspects in_addr.s_addr
//Returns: Suspect's in_addr.s_addr
in_addr_t Suspect::GetIpAddress()
{
	return m_id.m_ip;
}

SuspectIdentifier Suspect::GetIdentifier()
{
	return m_id;
}

//Sets the suspects in_addr
void Suspect::SetIpAddress(in_addr_t ip)
{
	m_id.m_ip= ip;
}

//Returns a copy of the Suspects classification double
// Returns -1 on failure
double Suspect::GetClassification()
{
	return m_classification;
}

//Sets the suspect's classification
void Suspect::SetClassification(double n)
{
	m_classification = n;
}


//Returns the number of hostile neighbors
int Suspect::GetHostileNeighbors()
{
	return m_hostileNeighbors;
}
//Sets the number of hostile neighbors
void Suspect::SetHostileNeighbors(int i)
{
	m_hostileNeighbors = i;
}


//Returns the hostility bool of the suspect
bool Suspect::GetIsHostile()
{
	return m_isHostile;
}
//Sets the hostility bool of the suspect
void Suspect::SetIsHostile(bool b)
{
	m_isHostile = b;
}


//Returns the flagged by silent alarm bool
bool Suspect::GetFlaggedByAlarm()
{
	return m_flaggedByAlarm;
}
//Sets the flagged by silent alarm bool
void Suspect::SetFlaggedByAlarm(bool b)
{
	m_flaggedByAlarm = b;
}


//Returns the 'from live capture' bool
bool Suspect::GetIsLive()
{
	return m_isLive;
}
//Sets the 'from live capture' bool
void Suspect::SetIsLive(bool b)
{
	m_isLive = b;
}

//Clears the FeatureData of a suspect
// whichFeatures: specifies which FeatureSet's Data to clear
void Suspect::ClearFeatureData(FeatureMode whichFeatures)
{
	switch(whichFeatures)
	{
		default:
		case MAIN_FEATURES:
		{
			m_features.ClearFeatureData();
			break;
		}
		case UNSENT_FEATURES:
		{
			m_unsentFeatures.ClearFeatureData();
			break;
		}
	}
}

//Returns a copy of the suspects FeatureSet
FeatureSet Suspect::GetFeatureSet(FeatureMode whichFeatures)
{
	switch(whichFeatures)
	{
		default:
		case MAIN_FEATURES:
		{
			return m_features;
			break;
		}
		case UNSENT_FEATURES:
		{
			return m_unsentFeatures;
			break;
		}
	}
}

//Sets or overwrites the suspects FeatureSet
void Suspect::SetFeatureSet(FeatureSet *fs, FeatureMode whichFeatures)
{
	switch(whichFeatures)
	{
		default:
		case MAIN_FEATURES:
		{
			m_features.operator =(*fs);
			break;
		}
		case UNSENT_FEATURES:
		{
			m_unsentFeatures.operator =(*fs);
			break;
		}
	}

}

//Adds the feature set 'fs' to the suspect's feature set
void Suspect::AddFeatureSet(FeatureSet *fs, FeatureMode whichFeatures)
{
	switch(whichFeatures)
	{
		default:
		case MAIN_FEATURES:
		{
			m_features += *fs;
			break;
		}
		case UNSENT_FEATURES:
		{
			m_unsentFeatures += *fs;
			break;
		}
	}

}

//Clears the feature set of the suspect
// whichFeatures: specifies which FeatureSet to clear
void Suspect::ClearFeatureSet(FeatureMode whichFeatures)
{
	switch(whichFeatures)
	{
		case MAIN_FEATURES:
		{
			m_features.ClearFeatureSet();
			break;
		}
		case UNSENT_FEATURES:
		{
			m_unsentFeatures.ClearFeatureSet();
			break;
		}
		default:
		{
			break;
		}
	}
}

//Returns the accuracy double of the feature using featureIndex 'fi'
double Suspect::GetFeatureAccuracy(featureIndex fi)
{
	return m_featureAccuracy[fi];
}
//Sets the accuracy double of the feature using featureIndex 'fi'
void Suspect::SetFeatureAccuracy(featureIndex fi, double d)
{
	m_featureAccuracy[fi] = d;
}

Suspect& Suspect::operator=(const Suspect &rhs)
{
	m_features = rhs.m_features;
	m_unsentFeatures = rhs.m_unsentFeatures;
	m_lastPacketTime = rhs.m_lastPacketTime;
	for(uint i = 0; i < DIM; i++)
	{
		m_featureAccuracy[i] = rhs.m_featureAccuracy[i];
	}

	m_id = rhs.m_id;
	m_classification = rhs.m_classification;
	m_needsClassificationUpdate = rhs.m_needsClassificationUpdate;
	m_hostileNeighbors = rhs.m_hostileNeighbors;
	m_isHostile = rhs.m_isHostile;
	m_flaggedByAlarm = rhs.m_flaggedByAlarm;
	m_isLive = rhs.m_isLive;
	m_classificationNotes = rhs.m_classificationNotes;
	return *this;
}

bool Suspect::operator==(const Suspect &rhs) const
{
	if(m_features != rhs.m_features)
	{
		return false;
	}
	if(m_id != rhs.m_id)
	{
		return false;
	}

	if(m_classification != rhs.m_classification)
	{
		return false;
	}

	if(m_hostileNeighbors != rhs.m_hostileNeighbors)
	{
		return false;
	}

	for(int i = 0; i < DIM; i++)
	{
		if(m_featureAccuracy[i] != rhs.m_featureAccuracy[i])
		{
			return false;
		}
	}

	if(m_isHostile != rhs.m_isHostile)
	{
		return false;
	}

	if(m_flaggedByAlarm != rhs.m_flaggedByAlarm)
	{
		return false;
	}

	if(m_lastPacketTime != rhs.m_lastPacketTime)
	{
		return false;
	}
	return true;
}

long int Suspect::GetLastPacketTime()
{
	return m_lastPacketTime;
}

bool Suspect::operator !=(const Suspect &rhs) const
{
	return !(*this == rhs);
}

Suspect::Suspect(const Suspect &rhs)
{
	m_features = rhs.m_features;
	m_unsentFeatures = rhs.m_unsentFeatures;
	m_lastPacketTime = rhs.m_lastPacketTime;

	for(uint i = 0; i < DIM; i++)
	{
		m_featureAccuracy[i] = rhs.m_featureAccuracy[i];
	}

	m_id = rhs.m_id;
	m_classification = rhs.m_classification;
	m_hostileNeighbors = rhs.m_hostileNeighbors;
	m_isHostile = rhs.m_isHostile;
	m_flaggedByAlarm = rhs.m_flaggedByAlarm;
	m_isLive = rhs.m_isLive;
	m_needsClassificationUpdate = rhs.m_needsClassificationUpdate;
	m_classificationNotes = rhs.m_classificationNotes;
}

}
