#include "BTreeNode.h"
#include <iostream>
#include <stdio.h>

using namespace std;

//Leaf node constructor
BTLeafNode::BTLeafNode()
{
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
	int keyCount = 0;

	for (int i = 0; i < PageFile::PAGE_SIZE - sizeof(PageId); i+=BTLeafNode::ENTRY_SIZE) {
		int currentKey;

		memcpy(&currentKey, buffer+i, sizeof(int)); // get key in entry

		if (currentKey != 0)
			keyCount++;
		else
			break;
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
	if (getKeyCount() >= BTLeafNode::MAX_ENTRIES)
		return RC_NODE_FULL;

	// i will point to the index at which we need to insert the new entry
	int i = 0;
	while (i < PageFile::PAGE_SIZE - sizeof(PageId) - BTLeafNode::ENTRY_SIZE) {
		int currentKey;

		memcpy(&currentKey, buffer+i, sizeof(int)); // get key in entry

		if (currentKey == 0 || key <= currentKey) // if currentKey is null (0) or insertion key is smaller or equal to the iterating key
			break;

		i += BTLeafNode::ENTRY_SIZE;
	}

	// define tmp buffer that will be the updated buffer after insertion
	char* tmpBuffer = (char*) malloc(PageFile::PAGE_SIZE);
	fill(tmpBuffer, tmpBuffer+PageFile::PAGE_SIZE, 0);

	memcpy(tmpBuffer, buffer, i);	// up to i, copy everything from original buffer

	memcpy(tmpBuffer + i + sizeof(int), &rid, sizeof(RecordId));	// copy in given record
	memcpy(tmpBuffer + i, &key, sizeof(int));	// copy in given key

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
	RC ret = -1;

	if (getKeyCount() < BTLeafNode::MAX_ENTRIES)	// adding in entry should cause overflow. Otherwise, this method should not be called as no need to split
		ret = RC_INVALID_FILE_FORMAT;

	if (sibling.getKeyCount() != 0)	// sibling node must be empty
		ret = RC_INVALID_ATTRIBUTE;

	if (ret != -1)
		return ret;

	fill(sibling.buffer, sibling.buffer + PageFile::PAGE_SIZE, 0); // format sibling buffer

	int hi = BTLeafNode::ENTRY_SIZE * ((getKeyCount() + 1) / 2);

	// printf ("half index: %d \n", halfIndex);

	char* b = buffer;
	memcpy(sibling.buffer, b+hi, PageFile::PAGE_SIZE-hi-sizeof(PageId)); // copy everything on the latter half to the sibling buffer
	sibling.setNextNodePtr(getNextNodePtr());	// set nextnodeptr for sibling

	fill(b+hi,b + PageFile::PAGE_SIZE - sizeof(PageId),0);	// clear latter half in the original buffer (except PageId)

	int skBeforeInsert;
	memcpy(&skBeforeInsert, sibling.buffer, sizeof(int));
	skBeforeInsert <= key ? sibling.insert(key, rid) : insert(key, rid);

	RecordId sibRid;
	memcpy(&sibRid, sibling.buffer+sizeof(int), sizeof(RecordId));

	memcpy(&siblingKey, sibling.buffer, sizeof(int));	// copy sibling's first key

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
	for (int i = 0; i < getKeyCount() * BTLeafNode::ENTRY_SIZE; i+=BTLeafNode::ENTRY_SIZE) {
		int currentKey;

		memcpy(&currentKey, buffer+i, sizeof(int)); // get key in entrys

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
	if (eid < 0 || eid >= getKeyCount())
		return RC_NO_SUCH_RECORD;

	memcpy(&key, buffer+(eid*BTLeafNode::ENTRY_SIZE), sizeof(int));
	memcpy(&rid, buffer+(eid*BTLeafNode::ENTRY_SIZE)+sizeof(int), sizeof(RecordId));

	return 0;
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{
	PageId pid = 0;
	memcpy(&pid, buffer-sizeof(PageId)+PageFile::PAGE_SIZE, sizeof(PageId));	// last PageId section of leafnode that points to the next leaf node
	return pid;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	if (pid >= 0) {
		memcpy(buffer+PageFile::PAGE_SIZE-sizeof(PageId), &pid, sizeof(PageId));
		return 0;
	} else {
		return RC_INVALID_PID;
	}
	return RC_INVALID_PID;
}

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
	int keyCount = 0;

	// first 8 bytes (4 bytes pid, 4 bytes empty)
	for (int i = 8; i <= PageFile::PAGE_SIZE - 8; i+=BTNonLeafNode::ENTRY_SIZE) {
		int currentKey;
		memcpy(&currentKey, buffer+i, sizeof(int)); // get key in entry

		if (currentKey != 0)
			keyCount++;
		else
			break;
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
	if (getKeyCount() + 1 > BTNonLeafNode::MAX_ENTRIES)
		return RC_NODE_FULL;

	// i will point to the index at which we need to insert the new entry
	int i = 8;
	while (i < PageFile::PAGE_SIZE - BTNonLeafNode::ENTRY_SIZE) {
		int currentKey;

		memcpy(&currentKey, buffer+i, sizeof(int)); // get key in entry

		if (key <= currentKey || currentKey == 0) // if currentKey is null (0) or insertion key is smaller or equal to the iterating key
			break;

		i += BTNonLeafNode::ENTRY_SIZE;
	}

	// define tmp buffer that will be the updated buffer after insertion
	char* tmpBuffer = (char*) malloc(PageFile::PAGE_SIZE);
	fill(tmpBuffer, tmpBuffer+PageFile::PAGE_SIZE, 0);

	memcpy(tmpBuffer, buffer, i);	// up to i, copy everything from original buffer

	memcpy(tmpBuffer + i, &key, sizeof(int));	// copy in given key
	memcpy(tmpBuffer + sizeof(int) + i, &pid, sizeof(PageId));	// copy in given pageid

	memcpy(tmpBuffer + BTNonLeafNode::ENTRY_SIZE +i , buffer + i, BTNonLeafNode::ENTRY_SIZE * getKeyCount() - i + 8);	// add on rest of existing buffer
	
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
	RC ret = -1;

	if (getKeyCount() < BTNonLeafNode::MAX_ENTRIES)	// adding in entry should cause overflow. Otherwise, this method should not be called as no need to split
		ret = RC_INVALID_FILE_FORMAT;
	else if (sibling.getKeyCount() != 0)	// sibling node must be empty
		ret = RC_INVALID_ATTRIBUTE;

	if (ret != -1)
		return ret;

	char* b = buffer;
	int hi = 8 + (BTNonLeafNode::ENTRY_SIZE * ((getKeyCount() + 1) / 2));	// first 8 byte offset

	// Find median key
	int leftKey, rightKey;	// last key of first half
	memcpy(&leftKey, b+hi-8, sizeof(int));
	memcpy(&rightKey, b+hi, sizeof(int));

	fill(sibling.buffer, sibling.buffer + PageFile::PAGE_SIZE, 0); // format sibling buffer

	if (key > rightKey) {
		midKey = rightKey;
		memcpy(sibling.buffer+8, b+hi+BTNonLeafNode::ENTRY_SIZE, PageFile::PAGE_SIZE - hi - BTNonLeafNode::ENTRY_SIZE);	// copy latter half into sibling's buffer EXCEPT first node
		memcpy(sibling.buffer, b+hi+4, sizeof(PageId));	// set initial pid of sibling buffer 
		fill(b+hi, b+PageFile::PAGE_SIZE, 0);  // clear right half of original buffer
		sibling.insert(key, pid);	// insert it in newly created sibling node
	} else if (key < leftKey) {
		midKey = leftKey;
		memcpy(sibling.buffer+8, b+hi, PageFile::PAGE_SIZE - hi);	// copy latter half into sibling's buffer
		memcpy(sibling.buffer, b+hi-4, sizeof(PageId));	// set initial pid of sibling buffer last pid that is deleted 
		fill(b+hi-8, b+PageFile::PAGE_SIZE, 0); // clear right half of original buffer along with the last entry since it's the median key
		insert(key, pid);	// insert it in original leaf node
	} else {
		midKey = leftKey;
		memcpy(sibling.buffer+8, b+hi, PageFile::PAGE_SIZE - hi);	// copy latter half into sibling's buffer
		fill(b+hi, b+PageFile::PAGE_SIZE, 0);  // clear right half of original buffer
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
	char* b = buffer;
	int i = 8;

	while (i < BTNonLeafNode::ENTRY_SIZE*getKeyCount() + 8) {
		int currentKey;
		memcpy(&currentKey, b+i, sizeof(int)); // get key in entry

		if (currentKey > searchKey) {
			if (i == 8)
				memcpy(&pid, b, sizeof(PageId));	// if first key is larger than search key, return initial pid
			else
				memcpy(&pid, b+i-4, sizeof(PageId));
			return 0;
		}
		i += BTNonLeafNode::ENTRY_SIZE;
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
	memcpy(buffer, &pid1, sizeof(PageId)); 
	return insert(key, pid2);
}

