/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"
using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    treeHeight = 0;	//tree width
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
    RC rc;	//attempt to open
    if (rc = pf.open(indexname, mode)) return rc; //error in opening
	if (rc = pf.read(0, buf)) return rc; //error in reading
	int t_pid, t_height; // initialize pid and height
	memcpy(&t_pid, buf, 4); //transfer the info
	memcpy(&t_height, buf+4, 4);
	if (t_pid > 0 && t_height >= 0)
		rootPid = t_pid; treeHeight = t_height;
	return rc;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	RC ret;
    memcpy(buf, &rootPid, 4); //save info 
	memcpy(buf+4, &treeHeight, 4); // before closing
    if (ret = pf.write(0, buf)) return ret;
	return pf.close(); //try closing
}

//helper function to create a root node
RC BTreeIndex::createRoot(int key, const RecordId& rid)
{
    BTLeafNode root;		//create new tree's root node
    root.insert(key, rid);
    if (!pf.endPid()) rootPid = 1; //file was just created
    else rootPid = pf.endPid();
    ++treeHeight;		//increment depth
    return root.write(rootPid, pf);	//write into pid
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
    RC rc;
    if (!treeHeight) return createRoot(key, rid);//empty tree case
    int ins_key = -1; PageId ins_pid = -1; //initialize variables
    rc = recursiveInsert(rid, key, ins_key, rootPid, ins_pid, 1, treeHeight);
    return rc != 0 ? rc : 0; //if the tree is nonempty, recursively insert
}

void BTreeIndex::createNonLeaf(PageId pid, int split_key, int end_pid)
{
    BTNonLeafNode rt; // creation of root
    rt.initializeRoot(pid, split_key, end_pid);
    rootPid = pf.endPid();
    rt.write(rootPid, pf);
    ++treeHeight; // increment depth
}

//helper function to insert a leaf node
RC BTreeIndex::insertLeaf(const RecordId& rid, 
			  int key, 
			  PageId pid, 
			  int& sibkey, 
			  PageId& sibpid)
{
    RC rc;
	BTLeafNode l; // create a leaf node and grab contents
	l.read(pid, pf);

	if(!l.insert(key, rid)) // attempt to insert
	{
		l.write(pid, pf); 
		return 0; 
	} //skip if insert unsucessful...

	BTLeafNode l2; // attempt to insert with spiltting
	int key2;
	if(rc = l.insertAndSplit(key, rid, l2, key2)) return rc; //failed
	
	int last = pf.endPid(); // post-split handling...
	sibkey = key2; //this needs to be given to parent...
	sibpid = last; // save these variables for recursive pop
	l2.setNextNodePtr(l.getNextNodePtr()); // adjusting new information
	l.setNextNodePtr(last);
	
	if(rc = l2.write(last, pf)) return rc;
	if(rc = l.write(pid, pf)) return rc;
	
	//require a new BTNonLeafNode...
	if(treeHeight==1) createNonLeaf(pid, key2, last); //will need to split
	return 0;
}

RC BTreeIndex::recursiveInsert(const RecordId& rid, int key, int& sibkey, PageId pid, PageId& sibpid, int height, int tot_height)
{
	RC ret;
	sibkey = -1; sibpid = -1;
	if(height==treeHeight) insertLeaf(rid, key, pid, sibkey, sibpid);
	else
	{   //case where we are currently in interior node
		int ins_key = -1; PageId ins_pid = -1, child = -1;;
		BTNonLeafNode n;
		n.read(pid, pf); // grab contents of interior node
		n.locateChildPtr(key, child); // also grab the chidl...
		recursiveInsert(rid, key, ins_key, child, ins_pid, height+1, treeHeight); // after grabbing child key, recursively insert
		
		if(ins_key != -1 || ins_pid != -1) 
		{	// if we reached here...somewhere, a node was split
			if(!n.insert(ins_key, ins_pid))
			{//if we could insert the child's key, we must write
				n.write(pid, pf);
				return 0;
			}

			BTNonLeafNode n2; //unsuccessful, must split once again
			int key2;
			n.insertAndSplit(ins_key, ins_pid, n2, key2);

			int lastPid = pf.endPid(); // same deal as before...
			sibkey = key2;
			sibpid = lastPid;
			
			if(ret = n.write(pid, pf)) return ret; // same error checking
			if(ret = n2.write(lastPid, pf)) return ret;
			if(treeHeight==1) createNonLeaf(pid, key2, lastPid);//split
		}
		return 0;
	}
}

/**
 * Run the standard B+Tree key search algorithm and identify the
 * leaf node where searchKey may exist. If an index entry with
 * searchKey exists in the leaf node, set IndexCursor to its location
 * (i.e., IndexCursor.pid = PageId of the leaf node, and
 * IndexCursor.eid = the searchKey index entry number.) and return 0.
 * If not, set IndexCursor.pid = PageId of the leaf node and
 * IndexCursor.eid = the index entry immediately after the largest
 * index key that is smaller than searchKey, and return the error
 * code RC_NO_SUCH_RECORD.
 * Using the returned "IndexCursor", you will have to call readForward()
 * to retrieve the actual (key, rid) pair from the index.
 * @param key[IN] the key to find
 * @param cursor[OUT] the cursor pointing to the index entry with
 *                    searchKey or immediately behind the largest key
 *                    smaller than searchKey.
 * @return 0 if searchKey is found. Othewise an error code
 * This will serve as a helper function to recursively locate
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
    RC rc; //return code
    PageId nxt_pid = rootPid;
    BTLeafNode l;
    BTNonLeafNode nl;
    int height = 0;    
    while (++height < treeHeight)
    {	//continue searching until we reach max depth of tree
		if(rc = nl.read(nxt_pid, pf)) return rc; //look @ nxt searchkey
		if(rc = nl.locateChildPtr(searchKey, nxt_pid)) return rc;
    }
    if(rc = l.read(nxt_pid, pf)) return rc;
    int eid; //set the eid after we found the BTLeafNode
    if(rc = l.locate(searchKey, eid)) return rc;

    cursor.eid = eid; //save the info back into the cursor
    cursor.pid = nxt_pid;
    return 0; //successful
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
    RC ret = 0;
    BTLeafNode n;
    int c_eid = cursor.eid;		//grab info from cursor
    PageId c_pid = cursor.pid;

    if (ret = n.read(c_pid, pf)) return ret; // try to get BTLeafNode data
    if (ret = n.readEntry(c_eid, key, rid)) return ret;//try get key,rid
    if (c_pid <= 0) return RC_INVALID_CURSOR; //out of bounds

    if (c_eid+1 < n.getKeyCount()) ++c_eid; // ok to increment
    else	//otherwise handle out of bounds case
    {
		c_eid = 0;
		c_pid = n.getNextNodePtr();
    }

    cursor.eid = c_eid;//write info back to cursor
    cursor.pid = c_pid;
    return ret;
}