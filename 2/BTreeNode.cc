#include "BTreeNode.h"
#include <iostream>
#include <stdio.h>

using namespace std;

//Leaf node constructor
BTLeafNode::BTLeafNode()
{
	// numKeys=0;
	std::fill(buffer, buffer + PageFile::PAGE_SIZE, 0); //clear the buffer if necessary
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf) { return pf.read(pid, buffer); }
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf) { return pf.write(pid, buffer); }

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	//return numKeys;
	
	//This is the size in bytes of an entry pair
	// int pairSize = sizeof(RecordId) + sizeof(int);
	// int count=0;
	// char* temp = buffer;
	
	// //Loop through all the indexes in the temp buffer; increment by 12 bytes to jump to next key
	// //1008 is the largest possible index of the next inserted pair (since we already know we can fit another pair)
	// int i;
	// for(i=0; i<=1008; i+=pairSize)
	// {
	// 	int insideKey;
	// 	memcpy(&insideKey, temp, sizeof(int)); //Save the current key inside buffer as insideKey
	// 	if(insideKey==0) //Once we hit key of 0, we break
	// 		break;
	// 	//Otherwise, increment count
	// 	count++;
		
	// 	temp += pairSize; //Jump temp over to the next key
	// }
	
	// return count;
	int keyCount = 0;
	char* b = buffer;

	for (int i = 0; i < PageFile::PAGE_SIZE - sizeof(PageId); i+=BTLeafNode::ENTRY_SIZE) {
		int currentKey;

		memcpy(&currentKey, b+i, sizeof(int)); // get key in entry

		if (currentKey == 0)
			break;

		keyCount++;
	}
	return keyCount;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{
	// //Save last 4 bytes (the pid) for reconstructing the inserted leaf
	// PageId nextNodePtr = getNextNodePtr();
	
	// //This is the size in bytes of an entry pair
	// int pairSize = sizeof(RecordId) + sizeof(int);
	
	// //Return error if no more space in this node
	// //Page has 1024 bytes, we need to store 12 bytes (key, rid)
	// //That means we can fit 85 with 4 bytes left over for pid pointer to next leaf node
	// //Check if adding one more (key, rid) pair will exceed the size limit of 85
	// int numTotalPairs = (PageFile::PAGE_SIZE-sizeof(PageId))/pairSize;
	// if(getKeyCount()+1 > numTotalPairs) //if(getKeyCount()+1 > 85)
	// {
	// 	//cout << "Cannot insert anymore: this node is full!" << endl;
	// 	return RC_NODE_FULL;
	// }
	
	// //Now we must go through the buffer's sorted keys to see where the new key goes
	// char* temp = buffer;
	
	// //Loop through all the indexes in the temp buffer; increment by 12 bytes to jump to next key
	// //1008 is the largest possible index of the next inserted pair (since we already know we can fit another pair)
	
	// int i;
	// for(i=0; i<1008; i+=pairSize)
	// {
	// 	int insideKey;
	// 	memcpy(&insideKey, temp, sizeof(int)); //Save the current key inside buffer as insideKey
		
	// 	//Once the insideKey is null or key is smaller than some inside key, we stop
	// 	if(insideKey==0 || !(key > insideKey))
	// 		break;
		
	// 	temp += pairSize; //Jump temp over to the next key
	// }
	
	// //At this point, variable i holds the index to insert the pair and temp is the buffer at that index
	// char* newBuffer = (char*)malloc(PageFile::PAGE_SIZE);
	// std::fill(newBuffer, newBuffer + PageFile::PAGE_SIZE, 0); //clear the buffer if necessary
	
	// //Copy all values from buffer into newBuffer up until i
	// memcpy(newBuffer, buffer, i);
	
	// //Values to insert as new (key, rid) pair
	// PageId pid = rid.pid;
	// int sid = rid.sid;
	
	// memcpy(newBuffer+i, &key, sizeof(int));
	// memcpy(newBuffer+i+sizeof(int), &rid, sizeof(RecordId));
	
	// //INCREMENTAL POINTER METHOD:
	// //char* insertPos = newBuffer+i;
	// //memcpy(insertPos, &pid, sizeof(PageId));
	// //insertPos += sizeof(PageId);
	// //memcpy(insertPos, &sid, sizeof(int));
	// //insertPos += sizeof(int);
	// //memcpy(insertPos, &key, sizeof(int));
	
	// //Copy the rest of the values into newBuffer
	// //Notice that we are neglecting nextNodePtr, so we'll manually copy that in
	// memcpy(newBuffer+i+pairSize, buffer+i, getKeyCount()*pairSize - i);
	// memcpy(newBuffer+PageFile::PAGE_SIZE-sizeof(PageId), &nextNodePtr, sizeof(PageId));
	
	// //Copy newBuffer into buffer, then delete temporary newBuffer to prevent memory leak
	// memcpy(buffer, newBuffer, PageFile::PAGE_SIZE);
	// free(newBuffer);
	
	// //Successfully inserted leaf node, so we increment number of keys
	// // numKeys++;
	
	// return 0;
	// check if there is enough space to insert a new entry in this leaf node
	if (getKeyCount() + 1 > BTLeafNode::MAX_ENTRIES)
		return RC_NODE_FULL;

	char* b = buffer;

	// i will point to the index at which we need to insert the new entry
	int i = 0;
	while (i < PageFile::PAGE_SIZE - sizeof(PageId) - BTLeafNode::ENTRY_SIZE) {
		int currentKey;

		memcpy(&currentKey, b+i, sizeof(int)); // get key in entry

		if (currentKey == 0 || key <= currentKey) // if currentKey is null (0) or insertion key is smaller or equal to the iterating key
			break;

		i += BTLeafNode::ENTRY_SIZE;
	}

	// define tmp buffer that will be the updated buffer after insertion
	char* tmpBuffer = (char*) malloc(PageFile::PAGE_SIZE);
	fill(tmpBuffer, tmpBuffer+PageFile::PAGE_SIZE, 0);

	memcpy(tmpBuffer, buffer, i);	// up to i, copy everything from original buffer

	memcpy(tmpBuffer + i, &key, sizeof(int));	// copy in given key
	memcpy(tmpBuffer + i + sizeof(int), &rid, sizeof(RecordId));	// copy in given record

	memcpy(tmpBuffer + i + BTLeafNode::ENTRY_SIZE, buffer + i, getKeyCount() * BTLeafNode::ENTRY_SIZE - i);	// add on rest of existing buffer
	
	PageId nextSiblingPtr = getNextNodePtr();
	memcpy(tmpBuffer + PageFile::PAGE_SIZE - sizeof(PageId), &nextSiblingPtr, sizeof(PageId));	// finally, add on the next sibling node ptr

	memcpy(buffer, tmpBuffer, PageFile::PAGE_SIZE);	// update buffer
	free(tmpBuffer);

	return 0;
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{
	// //Save last 4 bytes (the pid) for reconstructing the inserted leaf
	// PageId nextNodePtr = getNextNodePtr();
	
	// int pairSize = sizeof(RecordId) + sizeof(int);
	// int numTotalPairs = (PageFile::PAGE_SIZE-sizeof(PageId))/pairSize;
	
	// //Only split if inserting will cause an overflow; otherwise, return error
	// if(!(getKeyCount() >= numTotalPairs))
	// 	return RC_INVALID_FILE_FORMAT;
	
	// //If sibling node is not empty, return error
	// if(sibling.getKeyCount()!=0)
	// 	return RC_INVALID_ATTRIBUTE;
	
	// //Clear sibling buffer just in case
	// std::fill(sibling.buffer, sibling.buffer + PageFile::PAGE_SIZE, 0); //clear the buffer if necessary
	
	// //Calculate keys to remain in the first half
	// int numHalfKeys = ((int)((getKeyCount()+1)/2));
	
	// //Find the index at which to split the node's buffer
	// int halfIndex = numHalfKeys*pairSize;
	
	// //Copy everything on the right side of halfIndex into sibling's buffer (ignore the pid)
	// memcpy(sibling.buffer, buffer+halfIndex, PageFile::PAGE_SIZE-sizeof(PageId)-halfIndex);
	
	// //Update sibling's number of keys and set pid to current node's pid ptr
	// // sibling.numKeys = getKeyCount() - numHalfKeys;
	// sibling.setNextNodePtr(getNextNodePtr());
	
	// //Clear the second half of current buffer except for pid; update number of keys
	// std::fill(buffer+halfIndex, buffer + PageFile::PAGE_SIZE - sizeof(PageId), 0); 
	// // numKeys = numHalfKeys;
	
	// //Check which buffer to insert new (key, rid) into
	// int firstHalfKey;
	// memcpy(&firstHalfKey, sibling.buffer, sizeof(int));
	
	// //Insert pair and increment number of keys
	// if(key>=firstHalfKey) //If our key belongs in the second buffer (since it's sorted)...
	// {
	// 	sibling.insert(key, rid);
	// }
	// else //Otherwise, place it in the first half
	// {
	// 	insert(key, rid);
	// }
	
	// //Copy over sibling's first key and rid
	// memcpy(&siblingKey, sibling.buffer, sizeof(int));
	
	// RecordId siblingRid;
	// siblingRid.pid = -1;
	// siblingRid.sid = -1;
	// memcpy(&siblingRid, sibling.buffer+sizeof(int), sizeof(RecordId));
	
	// //Remember not to touch the next node pointer
	// //Since we use it later, changing this will destroy the index tree's leaf node mapping
	
	// return 0;
	if (getKeyCount() < BTLeafNode::MAX_ENTRIES)	// adding in entry should cause overflow. Otherwise, this method should not be called as no need to split
		return RC_INVALID_FILE_FORMAT;

	
	if (sibling.getKeyCount() != 0)	// sibling node must be empty
		return RC_INVALID_ATTRIBUTE;

	fill(sibling.buffer, sibling.buffer + PageFile::PAGE_SIZE, 0); // format sibling buffer

	int halfKeyCount = (int) ((getKeyCount() + 1) / 2);
	int halfIndex = halfKeyCount * BTLeafNode::ENTRY_SIZE;

	// printf ("half key count: %d \n", halfKeyCount);
	// printf ("half index: %d \n", halfIndex);

	char* b = buffer;
	memcpy(sibling.buffer, b+halfIndex, PageFile::PAGE_SIZE-halfIndex-sizeof(PageId)); // copy everything on the latter half to the sibling buffer
	sibling.setNextNodePtr(getNextNodePtr());	// set nextnodeptr for sibling

	// printf("%s\n", "hello");

	fill(b+halfIndex,b + PageFile::PAGE_SIZE - sizeof(PageId),0);	// clear latter half in the original buffer (except PageId)
	// printf("%s\n", "my");
	int skBeforeInsert;
	memcpy(&skBeforeInsert, sibling.buffer, sizeof(int));
	if (skBeforeInsert <= key) {
		sibling.insert(key, rid);
		// printf("%s\n", "me");
	} else {
		insert(key, rid);
		// printf("%s\n", "you");
	}
	// printf("%s\n", "world");
	memcpy(&siblingKey, sibling.buffer, sizeof(int));	// copy sibling's first key

	RecordId sibRid;
	sibRid.pid = -1;
	sibRid.sid = -1;
	memcpy(&sibRid, sibling.buffer+sizeof(int), sizeof(RecordId));

	return 0; 
}
/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{
	// //This is the size in bytes of an entry pair
	// int pairSize = sizeof(RecordId) + sizeof(int);
	
	// char* temp = buffer;
	
	// //Loop through all the indexes in the temp buffer; increment by 12 bytes to jump to next key	
	// int i;
	// for(i=0; i<getKeyCount()*pairSize; i+=pairSize)
	// {
	// 	int insideKey;
	// 	memcpy(&insideKey, temp, sizeof(int)); //Save the current key inside buffer as insideKey
		
	// 	//Once insideKey is larger than or equal to searchKey
	// 	if(insideKey >= searchKey)
	// 	{
	// 		//Set eid to the current byte index divided by size of a pair entry
	// 		//This effectively produces eid
	// 		eid = i/pairSize;
	// 		return 0;
	// 	}
		
	// 	temp += pairSize; //Jump temp over to the next key
	// }
	
	// //If we get here, all of the keys inside the buffer were less than searchKey
	// eid = getKeyCount();
	// return 0;
	char* b = buffer;
	for (int i = 0; i < getKeyCount() * BTLeafNode::ENTRY_SIZE; i+=BTLeafNode::ENTRY_SIZE) {
		int currentKey;

		memcpy(&currentKey, b+i, sizeof(int)); // get key in entrys

		if (currentKey >= searchKey) {
			eid = i / BTLeafNode::ENTRY_SIZE;
			return 0; 
		}
	}

	// at this point, all keys inside the buffer were less than the given search key
	eid = getKeyCount();
	return 0;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{
	// //This is the size in bytes of an entry pair
	// int pairSize = sizeof(RecordId) + sizeof(int);
	
	// //If eid is out of bounds (negative or more than the number of keys we have), return error
	// if(eid >= getKeyCount() || eid < 0)
	// 	return RC_NO_SUCH_RECORD; //Not sure which error to return...RC_INVALID_CURSOR?
	
	// //This is the position in bytes of the entry
	// int bytePos = eid*pairSize;
	
	// char* temp = buffer;
	
	// //Copy the data into parameters
	// memcpy(&key, temp+bytePos, sizeof(int));
	// memcpy(&rid, temp+bytePos+sizeof(int), sizeof(RecordId));
	
	// return 0;
	if (eid < 0 || eid >= getKeyCount())
		return RC_NO_SUCH_RECORD;

	char* b = buffer;

	memcpy(&key, b+(eid*BTLeafNode::ENTRY_SIZE), sizeof(int));
	memcpy(&rid, b+(eid*BTLeafNode::ENTRY_SIZE)+sizeof(int), sizeof(RecordId));

	return 0;
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{
	// //Initialize a PageId; assume there's no next node by default
	// PageId pid = 0; 
	// char* temp = buffer;
	
	// //Find the last PageId section of the buffer and copy data over to pid
	// memcpy(&pid, temp+PageFile::PAGE_SIZE-sizeof(PageId), sizeof(PageId));
	
	// return pid;
	PageId pid = 0;
	char* b = buffer;

	memcpy(&pid, b+PageFile::PAGE_SIZE-sizeof(PageId), sizeof(PageId));	// last PageId section of leafnode that points to the next leaf node

	return pid;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	if (pid < 0)
		return RC_INVALID_PID;

	char* b = buffer;

	memcpy(b+PageFile::PAGE_SIZE-sizeof(PageId), &pid, sizeof(PageId));

	return 0;
}

// /*
//  * Print the keys of the node to cout
//  */
// void BTLeafNode::print()
// {
// 	//This is the size in bytes of an entry pair
// 	int pairSize = sizeof(RecordId) + sizeof(int);
	
// 	char* temp = buffer;
	
// 	for(int i=0; i<getKeyCount()*pairSize; i+=pairSize)
// 	{
// 		int insideKey;
// 		memcpy(&insideKey, temp, sizeof(int)); //Save the current key inside buffer as insideKey
		
// 		cout << insideKey << " ";
		
// 		temp += pairSize; //Jump temp over to the next key
// 	}
	
// 	cout << "" << endl;
// }

// //----------------------------------------------------------------------------------------------------
// //----------------------------------------------------------------------------------------------------
// //----------------------------------------------------------------------------------------------------


//Nonleaf node constructor
BTNonLeafNode::BTNonLeafNode() {
	std::fill(buffer, buffer + PageFile::PAGE_SIZE, 0); //clear the buffer if necessary
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf) { return pf.read(pid, buffer); }
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf) { return pf.write(pid, buffer); }

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
	//return numKeys;
	
	//This is the size in bytes of an entry pair
	// int pairSize = sizeof(PageId) + sizeof(int);
	// int numTotalPairs = (PageFile::PAGE_SIZE-sizeof(PageId))/pairSize; //127
	// int count=0;
	// //Now we must go through the buffer's sorted keys to see where the new key goes
	// //For nonleaf nodes only, remember to skip the first 8 bytes (4 bytes pid, 4 bytes empty)
	// char* temp = buffer+8;
	
	// //Loop through all the indexes in the temp buffer; increment by 8 bytes to jump to next key
	// //1016 is the largest possible index of the next inserted pair (since we already know we can fit another pair)
	// //For nonleaf nodes only, remember that we start the (key,pid) entries at index 8
	// int i;
	// for(i=8; i<=1016; i+=pairSize)
	// {
	// 	int insideKey;
	// 	memcpy(&insideKey, temp, sizeof(int)); //Save the current key inside buffer as insideKey
	// 	if(insideKey==0) //Once we hit key of 0, we break
	// 		break;
	// 	//Otherwise, increment count
	// 	count++;

	// 	temp += pairSize; //Jump temp over to the next key
	// }
	
	// return count;
	int keyCount = 0;
	char* b = buffer ;	

	// first 8 bytes (4 bytes pid, 4 bytes empty)
	for (int i = 8; i <= PageFile::PAGE_SIZE - 8; i+=BTNonLeafNode::ENTRY_SIZE) {
		int currentKey;
		memcpy(&currentKey, b+i, sizeof(int)); // get key in entry

		if (currentKey == 0)
			break;

		keyCount++;
	}
	return keyCount;
}

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
	// //Nonleaf nodes have pairs of integer keys and PageIds, with another PageId at the front
	// int pairSize = sizeof(PageId) + sizeof(int);
	// int numTotalPairs = (PageFile::PAGE_SIZE-sizeof(PageId))/pairSize; //127
	
	// //Return error if no more space in this node
	// //Page has 1024 bytes, we need to store 8 bytes (key, pid)
	// //That means we can fit 127 with 8 bytes left over; the first 4 will be used for pid
	// //Check if adding one more (key, pid) pair will exceed the size limit of 127
	// if(getKeyCount()+1 > numTotalPairs)
	// {
	// 	//cout << "Cannot insert anymore: this node is full!" << endl;
	// 	return RC_NODE_FULL;
	// }

	// //Now we must go through the buffer's sorted keys to see where the new key goes
	// //For nonleaf nodes only, remember to skip the first 8 bytes (4 bytes pid, 4 bytes empty)
	// char* temp = buffer+8;
	
	// //Loop through all the indexes in the temp buffer; increment by 8 bytes to jump to next key
	// //1016 is the largest possible index of the next inserted pair (since we already know we can fit another pair)
	// //For nonleaf nodes only, remember that we start the (key,pid) entries at index 8
	// int i;
	// for(i=8; i<1016; i+=pairSize)
	// {
	// 	int insideKey;
	// 	memcpy(&insideKey, temp, sizeof(int)); //Save the current key inside buffer as insideKey
		
	// 	//Once the insideKey is null or key is smaller than some inside key, we stop
	// 	if(insideKey==0 || !(key > insideKey))
	// 		break;
		
	// 	temp += pairSize; //Jump temp over to the next key
	// }
	
	// //At this point, variable i holds the index to insert the pair and temp is the buffer at that index
	// char* newBuffer = (char*)malloc(PageFile::PAGE_SIZE);
	// std::fill(newBuffer, newBuffer + PageFile::PAGE_SIZE, 0); //clear the buffer if necessary
	
	// //Copy all values from buffer into newBuffer up until i
	// memcpy(newBuffer, buffer, i);
	
	// //Copy key and pid into newBuffer
	// memcpy(newBuffer+i, &key, sizeof(int));
	// memcpy(newBuffer+i+sizeof(int), &pid, sizeof(PageId));
	
	// //Copy the rest of the values into newBuffer
	// //For nonleaf nodes only, remember that we must add in 8 bytes extra
	// //Otherwise we would be counting the initial (pid, empty) as a key
	// memcpy(newBuffer+i+pairSize, buffer+i, getKeyCount()*pairSize - i + 8);
	
	
	// //Copy newBuffer into buffer, then delete temporary newBuffer to prevent memory leak
	// memcpy(buffer, newBuffer, PageFile::PAGE_SIZE);
	// free(newBuffer);
	
	// //Successfully inserted leaf node, so we increment number of keys
	// // numKeys++;	
	// return 0;
	if (getKeyCount() + 1 > BTNonLeafNode::MAX_ENTRIES)
		return RC_NODE_FULL;

	char* b = buffer;
	// i will point to the index at which we need to insert the new entry
	int i = 8;
	while (i < PageFile::PAGE_SIZE - BTNonLeafNode::ENTRY_SIZE) {
		int currentKey;

		memcpy(&currentKey, b+i, sizeof(int)); // get key in entry

		if (currentKey == 0 || key <= currentKey) // if currentKey is null (0) or insertion key is smaller or equal to the iterating key
			break;

		i += BTNonLeafNode::ENTRY_SIZE;
	}

	// define tmp buffer that will be the updated buffer after insertion
	char* tmpBuffer = (char*) malloc(PageFile::PAGE_SIZE);
	fill(tmpBuffer, tmpBuffer+PageFile::PAGE_SIZE, 0);

	memcpy(tmpBuffer, buffer, i);	// up to i, copy everything from original buffer

	memcpy(tmpBuffer + i, &key, sizeof(int));	// copy in given key
	memcpy(tmpBuffer + i + sizeof(int), &pid, sizeof(PageId));	// copy in given pageid

	memcpy(tmpBuffer + i + BTNonLeafNode::ENTRY_SIZE, buffer + i, getKeyCount() * BTNonLeafNode::ENTRY_SIZE - i + 8);	// add on rest of existing buffer
	
	memcpy(buffer, tmpBuffer, PageFile::PAGE_SIZE);	// update buffer
	free(tmpBuffer);

	return 0; 
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{
	// //Nonleaf nodes have pairs of integer keys and PageIds, with another PageId at the front
	// int pairSize = sizeof(PageId) + sizeof(int);
	// int numTotalPairs = (PageFile::PAGE_SIZE-sizeof(PageId))/pairSize; //127
	
	// //Only split if inserting will cause an overflow; otherwise, return error
	// if(!(getKeyCount() >= numTotalPairs))
	// 	return RC_INVALID_FILE_FORMAT;
	
	// //If sibling node is not empty, return error
	// if(sibling.getKeyCount()!=0)
	// 	return RC_INVALID_ATTRIBUTE;

	// //Clear sibling buffer just in case
	// std::fill(sibling.buffer, sibling.buffer + PageFile::PAGE_SIZE, 0); //clear the buffer if necessary

	// //Calculate keys to remain in the first half
	// int numHalfKeys = ((int)((getKeyCount()+1)/2));

	// //Find the index at which to split the node's buffer
	// //For nonleaf nodes only, remember to add an offset of 8 for initial pid
	// int halfIndex = numHalfKeys*pairSize + 8;
	
	// //REMOVING THE MEDIAN KEY
	
	// //Find the last key of the first half and the first key of the second half
	// int key1 = -1;
	// int key2 = -1;
	
	// memcpy(&key1, buffer+halfIndex-8, sizeof(int));
	// memcpy(&key2, buffer+halfIndex, sizeof(int));
	
	// if(key < key1) //key1 is the median key to be removed
	// {
	// 	//Copy everything on the right side of halfIndex into sibling's buffer (ignore the pid)
	// 	//For nonleaf nodes only, remember to add an offset of 8 for initial pid
	// 	memcpy(sibling.buffer+8, buffer+halfIndex, PageFile::PAGE_SIZE-halfIndex);
	// 	//Update sibling's number of keys
	// 	// sibling.numKeys = getKeyCount() - numHalfKeys;
		
	// 	//Copy down the median key before getting rid of it in buffer
	// 	memcpy(&midKey, buffer+halfIndex-8, sizeof(int));
		
	// 	//Also set the sibling pid from buffer before getting rid of it
	// 	memcpy(sibling.buffer, buffer+halfIndex-4, sizeof(PageId));
		
	// 	//Clear the second half of current buffer; update number of keys
	// 	//Also clear the last key of the first half, since this is the median key
	// 	std::fill(buffer+halfIndex-8, buffer + PageFile::PAGE_SIZE, 0); 
	// 	// numKeys = numHalfKeys - 1;
		
	// 	//Insert the (key, pid) pair into buffer, since it's key is smaller than the median
	// 	insert(key, pid);		
	// }
	// else if(key > key2) //key2 is the median key to be removed
	// {
	// 	//Copy everything on the right side of halfIndex EXCEPT FOR THE FIRST KEY into sibling's buffer (ignore the pid)
	// 	//The first key on the right side here is the median key, which will be removed
	// 	//For nonleaf nodes only, remember to add an offset of 8 for initial pid
	// 	memcpy(sibling.buffer+8, buffer+halfIndex+8, PageFile::PAGE_SIZE-halfIndex-8);
	// 	//Update sibling's number of keys
	// 	// sibling.numKeys = getKeyCount() - numHalfKeys - 1;
		
	// 	//Copy down the median key before getting rid of it in buffer
	// 	memcpy(&midKey, buffer+halfIndex, sizeof(int));
		
	// 	//Also set the sibling pid from buffer before getting rid of it
	// 	memcpy(sibling.buffer, buffer+halfIndex+4, sizeof(PageId));
		
	// 	//Clear the second half of current buffer; update number of keys
	// 	std::fill(buffer+halfIndex, buffer + PageFile::PAGE_SIZE, 0); 
	// 	// numKeys = numHalfKeys;
		
	// 	//Insert the (key, pid) pair into sibling, since it's key is larger than the median
	// 	sibling.insert(key, pid);
		
	// }
	// else //key is the median key to be removed
	// {
	// 	//Copy everything on the right side of halfIndex into sibling's buffer (ignore the pid)
	// 	//For nonleaf nodes only, remember to add an offset of 8 for initial pid
	// 	memcpy(sibling.buffer+8, buffer+halfIndex, PageFile::PAGE_SIZE-halfIndex);
	// 	//Update sibling's number of keys
	// 	// sibling.numKeys = getKeyCount() - numHalfKeys;
		
	// 	//Clear the second half of current buffer; update number of keys
	// 	std::fill(buffer+halfIndex, buffer + PageFile::PAGE_SIZE, 0); 
	// 	// numKeys = numHalfKeys;
		
	// 	//The key we're inserting IS the median key, so we stop the insertion process and return
	// 	midKey = key;
		
	// 	//Set the sibling pid from the median key parameter
	// 	memcpy(sibling.buffer, &pid, sizeof(PageId));
	// }

	// //If we reach this, then we have somehow split the node and inserted the (key, pid) pair
	// return 0;
	if (getKeyCount() + 1 <= BTNonLeafNode::MAX_ENTRIES)	// adding in entry should cause overflow. Otherwise, this method should not be called as no need to split
		return RC_INVALID_FILE_FORMAT;
	
	if (sibling.getKeyCount() != 0)	// sibling node must be empty
		return RC_INVALID_ATTRIBUTE;

	fill(sibling.buffer, sibling.buffer + PageFile::PAGE_SIZE, 0); // format sibling buffer

	int halfKeyCount = (int) ((getKeyCount() + 1) / 2);
	int halfIndex = halfKeyCount * BTNonLeafNode::ENTRY_SIZE + 8;	// first 8 byte offset

	char* b = buffer;

	// Find median key
	int k1 = -1;	// last key of first half
	memcpy(&k1, b+halfIndex-8, sizeof(int));
	int k2 = -1;	// first key of second half
	memcpy(&k2, b+halfIndex, sizeof(int));

	if (key < k1) { // k1 becomes median key
		memcpy(sibling.buffer+8, b+halfIndex, PageFile::PAGE_SIZE - halfIndex);	// copy latter half into sibling's buffer
		midKey = k1;
		memcpy(sibling.buffer, b+halfIndex-4, sizeof(PageId));	// set initial pid of sibling buffer last pid that is deleted 
		fill(b+halfIndex-8, b+PageFile::PAGE_SIZE, 0); // clear right half of original buffer along with the last entry since it's the median key
		insert(key, pid);	// insert it in original leaf node
	} else if (key > k2) {	// k2 becomes median key
		memcpy(sibling.buffer+8, b+halfIndex+BTNonLeafNode::ENTRY_SIZE, PageFile::PAGE_SIZE - halfIndex - BTNonLeafNode::ENTRY_SIZE);	// copy latter half into sibling's buffer EXCEPT first node
		midKey = k2;
		memcpy(sibling.buffer, b+halfIndex+4, sizeof(PageId));	// set initial pid of sibling buffer 
		fill(b+halfIndex, b+PageFile::PAGE_SIZE, 0);  // clear right half of original buffer
		sibling.insert(key, pid);	// insert it in newly created sibling node
	} else {	// key inserted is the median key
		memcpy(sibling.buffer+8, b+halfIndex, PageFile::PAGE_SIZE - halfIndex);	// copy latter half into sibling's buffer
		midKey = key;
		fill(b+halfIndex, b+PageFile::PAGE_SIZE, 0);  // clear right half of original buffer
		memcpy(sibling.buffer, &pid, sizeof(PageId));
	}

	return 0; 
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{
	// //This is the size in bytes of an entry pair
	// int pairSize = sizeof(PageId) + sizeof(int);
	
	// //Skip the first 8 offset bytes, since there's no key there
	// char* temp = buffer+8;	
	
	// //Loop through all the indexes in the temp buffer; increment by 8 bytes to jump to next key	
	// int i;	
	// for(i=8; i<getKeyCount()*pairSize+8; i+=pairSize)
	// {
	// 	int insideKey;
	// 	memcpy(&insideKey, temp, sizeof(int)); //Save the current key inside buffer as insideKey
				
	// 	if(i==8 && insideKey > searchKey) //If searchKey is less than first key, we need to return initial pid
	// 	{
	// 		//A special check is necessarily since the initial pid is in a different buffer position from the rest
	// 		memcpy(&pid, buffer, sizeof(PageId));
	// 		return 0;
	// 	}
	// 	else if(insideKey > searchKey)
	// 	{
	// 		//Set pid to be the left pid (that is, the pid on the small side of insideKey)
	// 		memcpy(&pid, temp-4, sizeof(PageId));
	// 		return 0;
	// 	}
		
	// 	//Otherwise, searchKey is greater than or equal to insideKey, so we keep checking
	// 	temp += pairSize; //Jump temp over to the next key
		
	// }
	
	// //If we get here, searchKey was greater than all instances of insideKey
	// //Copy over the last, right-most pid before returning
	// //Remember that temp is now on the next non-existent node's position, so we need to decrement by 4 bytes
	// memcpy(&pid, temp-4, sizeof(PageId));
	// return 0;
	char* b = buffer;
	int i = 8;
	for (; i < getKeyCount()*BTNonLeafNode::ENTRY_SIZE + 8; i+=BTNonLeafNode::ENTRY_SIZE) {
		int currentKey;
		memcpy(&currentKey, b+i, sizeof(int)); // get key in entry

		if (currentKey > searchKey) {
			if (i == 8)
				memcpy(&pid, b, sizeof(PageId));	// if first key is larger than search key, return initial pid
			else
				memcpy(&pid, b+i-4, sizeof(PageId));
			return 0;
		}
	}

	// reaching here implies that all keys were smaller than searchKey. So use the right most pid
	memcpy(&pid, b+i-4, sizeof(PageId));
	return 0;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{
	// RC error;

	// std::fill(buffer, buffer + PageFile::PAGE_SIZE, 0); //clear the buffer if necessary
	
	// //This time, don't skip the first 8 offset bytes
	// //We're actually initializing it to something explicitly
	// char* temp = buffer;
	
	// //Copy over the initial pid into buffer
	// memcpy(temp, &pid1, sizeof(PageId));
	
	// //Copy the first pair into buffer
	// //memcpy(temp+8, &key, sizeof(int));
	// //memcpy(temp+12, &pid2, sizeof(PageId));
	// error = insert(key, pid2);
	
	// if(error!=0)
	// 	return error;
	
	// //Set number of (key, pid) pairs to 1
	// //Only need this if we dont use insert to set (key, pid2) pair
	// //numKeys = 1;
	
	// return 0;
	memcpy(buffer, &pid1, sizeof(PageId)); 
	return insert(key, pid2);
}

