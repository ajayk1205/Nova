// ============================================================================
// Name        : SuspectTable.h
// Copyright   : DataSoft Corporation 2011-2013
// 	Nova is free software: you can redistribute it and/or modify
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
//   along with Nova.  If not, see <http:// www.gnu.org/licenses/>.
// Description : Wrapper class for the SuspectHashMap object used to maintain a
// 		list of suspects.
// ============================================================================

#ifndef SUSPECTTABLE_H_
#define SUSPECTTABLE_H_

#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "Suspect.h"
#include "protobuf/marshalled_classes.pb.h"

#define EMPTY_SUSPECT_CLASSIFICATION -1337

namespace std
{
	template<>
	struct hash< Nova::SuspectID_pb >
	{
		std::size_t operator()( const Nova::SuspectID_pb &c ) const
		{
			return hash<uint32_t>()(c.m_ip());
		}
	};
}

namespace Nova
{

struct SuspectIDEq
{
	bool operator()(Nova::SuspectID_pb k1, Nova::SuspectID_pb k2) const
	{
		if((k1.m_ip() == k2.m_ip()) && (k1.m_ifname() == k2.m_ifname()))
		{
			return true;
		}
		return false;
	}
};

typedef Nova::HashMap<Nova::SuspectID_pb, Nova::Suspect *, std::hash<Nova::SuspectID_pb>, Nova::SuspectIDEq> SuspectHashTable;

struct SuspectLock
{
	int ref_cnt;
	pthread_mutex_t lock;
	bool deleted;
};

typedef Nova::HashMap<Nova::SuspectID_pb,SuspectLock, std::hash<Nova::SuspectID_pb>, Nova::SuspectIDEq> SuspectLockTable;

enum SuspectTableRet : int32_t
{
	SUSPECT_KEY_INVALID = -2, //The key cannot be associated with (or assigned to) a recognized suspect
	SUSPECT_NOT_CHECKED_OUT = -1, //The suspect isn't checked out by this thread
	SUSPECT_TABLE_CALL_SUCCESS = 0, //The call succeeded
};

class SuspectTable
{

public:
	SuspectTable();
	~SuspectTable();

	// Adds the Suspect pointed to in 'suspect' into the table using suspect->GetIPAddress() as the key;
	// 		suspect: pointer to the Suspect you wish to add
	// Returns true on Success, and false if the suspect already exists
	bool AddNewSuspect(Suspect *suspect);

	// Adds the Suspect pointed to in 'suspect' into the table using the source of the evidence as the key;
	// 		evidence: copy of the packet you whish to create a suspect from
	// Returns true on Success, and false if the suspect already exists
	//bool AddNewSuspect(const Evidence& evidence);

	bool ClassifySuspect(Nova::SuspectID_pb key);

	void UpdateAllSuspects();

	void SetHaystackNodes(std::vector<uint32_t> nodes);

	// Copies the suspect pointed to in 'suspect', into the table location associated with key
	// 		suspect: pointer to the Suspect you wish to copy in
	// Returns (0) on Success, (-1) if the Suspect is Checked Out by another thread
	// and (-2) if the Suspect does not exist (Key invalid or suspect was deleted)
	// Note:  This function blocks until it can acquire a write lock on the suspect
	// IP address must be the same as key checked out with
	SuspectTableRet CheckIn(Suspect *suspect);

	//Releases the lock on the suspect at key allowing another thread to check it out
	//		key: IP address of the suspect as a uint value (host byte order)
	// Returns (0) on Success, (-1) if the Suspect is checked out by someone else
	// and (1) if the Suspect is not checked out
	// Note:  This function blocks until it can acquire a write lock on the suspect
	SuspectTableRet CheckIn(Nova::SuspectID_pb key);

	// Copies out a suspect and marks the suspect so that it cannot be written or deleted
	// 		key: IP address of the suspect as a uint value (host byte order)
	// Returns empty Suspect on failure.
	// Note: A 'Checked Out' Suspect cannot be modified until is has been replaced by the suspect 'Checked In'
	// 		However the suspect can continue to be read. It is similar to having a read lock.
	Suspect CheckOut(Nova::SuspectID_pb key);

	// Lookup and get an Asynchronous copy of the Suspect
	// 		key: IP address of the suspect as a uint value (host byte order)
	// Returns an empty suspect on failure
	// Note: To modify or lock a suspect use CheckOut();
	// Note: This is the same as GetSuspectStatus except it copies the feature set object which can grow very large.
	Suspect GetSuspect(Nova::SuspectID_pb key);
	Suspect GetShallowSuspect(Nova::SuspectID_pb key);

	//Erases a suspect from the table if it is not locked
	// 		key: IP address of the suspect as a uint value (host byte order)
	// Returns (true) on success, (false) if the suspect does not exist (key is invalid)
	bool Erase(Nova::SuspectID_pb key);

	// Clears the Suspect Table of all entries
	// Note: Locks may still persist until all threads unlock or return from blocking on them.
	void EraseAllSuspects();

	// This function returns a vector of suspects keys the caller can iterate over to access the table.
	// Returns a std::vector of every suspect currently in the table
	std::vector<SuspectID_pb> GetAllKeys();

	// This function returns a vector of suspects keys the caller can iterate over to access the table.
	// Returns a std::vector containing all hostile suspect keys
	std::vector<SuspectID_pb> GetKeys_of_HostileSuspects();

	// This function returns a vector of suspects keys the caller can iterate over to access the table.
	// Returns a std::vector containing all benign suspect keys
	std::vector<SuspectID_pb> GetKeys_of_BenignSuspects();

	// This function returns a vector of suspects keys the caller can iterate over to access the table.
	// Returns a std::vector containing the keys of all suspects that need a classification update.
	std::vector<SuspectID_pb> GetKeys_of_ModifiedSuspects();

	// Get the size of the Suspect Table
	// Returns the size of the Table
	uint Size();

	// Saves suspectTable to a human readable text file
	void SaveSuspectsToFile(std::string filename);

	//Iterates over the table, serializing each suspect and dumping the raw data to out
	//		out: ofstream you wish to write the contents to
	// Returns: size in bytes of data written
	//Note: Information can be retrieved by deserializing at the beginning of the dump and using the value returned
	// as an offset to start the next deserialization
	uint32_t DumpContents(std::ofstream *out, time_t timestamp = 0);

	//Iterates over the table, serializing each suspect and dumping the raw data to out
	//		out: ofstream you wish to write the contents to
	// Returns: size in bytes of data written
	//Note: Information can be retrieved by deserializing at the beginning of the dump and using the value returned
	// as an offset to start the next deserialization
	uint32_t ReadContents(std::ifstream *in, time_t timestamp = 0); //XXX

	// Checks the validity of the key - public thread-safe version
	// 		key: IP address of the suspect as a uint value (host byte order)
	// Returns true if there is a suspect associated with the given key, false otherwise
	bool IsValidKey(Nova::SuspectID_pb key);

	bool IsEmptySuspect(Suspect *suspect);

	//Consumes the linked list of evidence objects, extracting their information and inserting them into the Suspects.
	// evidence: Evidence object, if consuming more than one piece of evidence this is the start
	//				of the linked list.
	// Note: Every evidence object contained in the list is deallocated after use, invalidating the pointers,
	//		this is a specialized function designed only for use by Consumer threads.
	void ProcessEvidence(Evidence *evidence, bool readOnly = false);

	// Mark all suspects to be reclassified next classification cycle
	void SetEveryoneNeedsClassificationUpdate();

	Suspect m_emptySuspect;

private:

	// Hashmap used for constant time key lookups
	SuspectHashTable m_suspectTable;
	std::vector<Nova::SuspectID_pb> m_suspectsNeedingUpdate;
	SuspectLockTable m_lockTable;

	// List of haystack nodes, cached in the suspectTable
	// and passed to featureSets when a new suspect is created
	std::vector<uint32_t> m_haystackNodesCached;

	// Lock used to maintain concurrency between threads
	pthread_rwlock_t m_lock;
	pthread_mutex_t m_needsUpdateLock;

	std::vector<Nova::SuspectID_pb> m_keys;

	// Marks a suspect to be reclassified at some point
	void SetNeedsClassificationUpdate(Nova::SuspectID_pb key);
	void SetNeedsClassificationUpdate_noLocking(Nova::SuspectID_pb key);

	// Checks the validity of the key - private use non-locking version
	// 		key: IP address of the suspect as a uint value (host byte order)
	// Returns true if there is a suspect associated with the given key, false otherwise
	// *Note: Assumes you have already locked the table
	bool IsValidKey_NonBlocking(Nova::SuspectID_pb key);

	//Used by threads about to block on a suspect
	// 		key: IP address of the suspect as a uint value (host byte order)
	// Note: lock won't be deleted until the ref count is 0, but the Suspect can still be.
	bool LockSuspect(Nova::SuspectID_pb key);

	//Used by threads done blocking on a suspect
	// 		key: IP address of the suspect as a uint value (host byte order)
	// Returns (true) if the Lock could be unlocked and still exists and
	// (false) if the Suspect has been deleted or could not be unlocked.
	// Note: automatically deletes the lock if the suspect has been deleted and the ref count is 0
	bool UnlockSuspect(Nova::SuspectID_pb key);

};

}

#endif /* SUSPECTTABLE_H_ */
