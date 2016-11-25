#include "BTreeNode.h"

using namespace std;

BTLeafNode::BTLeafNode() 
{
	fill(buffer, buffer + PageFile::PAGE_SIZE, 0);
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ 
	return pf.read(pid, buffer);
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{
	return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
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
	if (getKeyCount() + 1 <= BTLeafNode::MAX_ENTRIES)	// adding in entry should cause overflow. Otherwise, this method should not be called as no need to split
		return RC_INVALID_FILE_FORMAT;
	
	if (sibling.getKeyCount() != 0)	// sibling node must be empty
		return RC_INVALID_ATTRIBUTE;

	fill(sibling.buffer, sibling.buffer + PageFile::PAGE_SIZE, 0); // format sibling buffer

	int halfKeyCount = (int) ((getKeyCount() + 1) / 2);
	int halfIndex = halfKeyCount * BTLeafNode::ENTRY_SIZE;

	char* b = buffer;
	memcpy(sibling.buffer, b+halfIndex, PageFile::PAGE_SIZE-halfIndex-sizeof(PageId)); // copy everything on the latter half to the sibling buffer
	sibling.setNextNodePtr(getNextNodePtr());	// set nextnodeptr for sibling

	fill(b+halfIndex,b+PageFile::PAGE_SIZE-halfIndex-sizeof(PageId),0);	// clear latter half in the original buffer (except PageId)

	int skBeforeInsert;
	memcpy(&skBeforeInsert, sibling.buffer, sizeof(int));
	if (skBeforeInsert <= key)
		sibling.insert(key, rid);
	else
		insert(key, rid);

	memcpy(&siblingKey, sibling.buffer, sizeof(int));	// copy sibling's first key

	return 0; 
}

/**
 * If searchKey exists in the node, set eid to the index entry
 * with searchKey and return 0. If not, set eid to the index entry
 * immediately after the largest index key that is smaller than searchKey,
 * and return the error code RC_NO_SUCH_RECORD.
 * Remember that keys inside a B+tree node are always kept sorted.
 * @param searchKey[IN] the key to search for.
 * @param eid[OUT] the index entry number with searchKey or immediately
                   behind the largest key smaller than searchKey.
 * @return 0 if searchKey is found. Otherwise return an error code.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
	char* b = buffer;
	int index_entry = 0;
	for (int i = 0; i < PageFile::PAGE_SIZE - sizeof(PageId); i+=BTLeafNode::ENTRY_SIZE) {
		int currentKey;

		memcpy(&currentKey, b+i, sizeof(int)); // get key in entry

		if (currentKey == searchKey) {
			eid = index_entry;
			return 0;
		}

		if (currentKey > searchKey) {
			eid = index_entry;
			return RC_NO_SUCH_RECORD; 
		}

		index_entry++;
	}


	return RC_NO_SUCH_RECORD; 
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



// NON LEAF NODE //

BTNonLeafNode::BTNonLeafNode() 
{
	fill(buffer, buffer + PageFile::PAGE_SIZE, 0);
}
/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ 
	return pf.read(pid, buffer);
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ 
	return pf.write(pid, buffer); 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
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
	memcpy(buffer, &pid1, sizeof(PageId)); 
	return insert(key, pid2);
}
