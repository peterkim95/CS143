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

// my includes
#include <iostream>
static const PageId ROOT = 0;	//this will be my root

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    treeHeight = 0;	//tree of depth zero
    rootPid = -1;	//invalid pid
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
    RC ret = pf.open(indexname, mode);	//attempt to open
    if (ret < 0) 
    {
	rootPid = -1;		//reset to invalid
	return ret;		//something happened
    }
    rootPid = ROOT;
    ret = pf.endPid();
    if (ret <= 0)		//index not initialized...write leaf
    {
	BTLeafNode l;
	return l.write(rootPid, pf);
    }
    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    rootPid = -1;		//reset to invalid
    RC ret = pf.close();	//return code of 'close' method
    return ret;
}

//helper function to create a root node
RC createRoot(int key, const RecordId& rid)
{
    BTLeafNode root;		//create root node
    root.insert(key, rid);
    if (pf.endPid() == 0) rootPid = 1; //file just created
    else rootPid = pf.endPid();
    ++treeHeight;		//increment depth of node
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
    RC ret;
    if (key < 0) return RC_INVALID_ATTRIBUTE;
    if (!treeHeight) return createRoot(key, rid);//empty tree case
    int ins_key = -1;				//nonempty tree case
    PageId ins_pid = -1;
    ret = insertRec(key, rid, 1, rootPid, ins_key, ins_pid);
    return ret != 0 ? ret : 0; 
}

void splitRoot(PageId pid, int split_key, int end_pid)
{
    BTNonLeafNode root;
    root.initializeRoot(pid, split_key, end_pid);
    rootPid = pf.endPid();
    root.write(rootPid, pf);
    ++treeHeight;
}

//helper function to insert a leaf node
RC insertLeaf(const RecordId& rid, int key, PageId pid, int& k, PageId& p)
{
    RC ret;
    BTLeafNode l;
    l.read(pid, pf);		//read in the contents
    if (!l.insert(key, rid)) 	// successful insert
    {
	l.write(pid, pf);
	return 0;
    }
    
    BTLeafNode split_l;		//prev insert unsucessful, must split
    int split_key;
    ret = l.insertAndSplit(key, rid, split_l, split_key);
    if (ret) return ret;	// something went wrong
    
    int end_pid = pf.endPid();	//split_key needs to be placed in parent
    k = split_key;
    p = end_pid;
    split_l.setNextNodePtr(l.getNextNodePtr());
    l.setNextNodePtr(end_pid);
    
    ret = l.write(pid, pf);		//error checking
    if (ret) return ret;		//return error
    ret = split_l.write(end_pid, pf);	//error checking
    if (ret) return ret;		//return error

    if (treeHeight == 1) splitRoot(pid, split_key, end_pid);
    return 0;
}

RC recursiveInsert(const RecordId& rid,
		   int key,
		   int& sibkey,
		   PageId pid,	
		   PageId& sibpid,
		   int height)
{
    RC ret;
    if (height==treeHeight) return insertLeaf(rid, key, pid, sibkey, sibpid);
    int ins_key = -1;		//else, we must handle recursive case
    int ins_pid = -1;
    BTNonLeafNode n;
    PageId child = -1;
    n.read(pid, pf); 
    n.locateChildPtr(key, child);
    recursiveInsert(rid, key, ins_key, pid, ins_pid, 1 + height);
    if (ins_key != -1 || ins_pid != -1) return -1;	//error
    if (!n.insert(ins_key, ins_pid)) //successful insert
    {
	n.write(pid, pf);
	return 0;
    }
    BTNonLeafNode n2;		//nonsuccessful insert, must split again
    int key2;
    n.insertAndSplit(ins_key, ins_pid, n2, key2);
    
    int end_pid = pf.endPid();
    sibkey = key2;
    sibpid = end_pid;
    
    ret = n2.write(end_pid, pf);	//error checking
    if (!ret) return ret;		//something went wrong
    ret = n.write(pid, pf);		//error checking
    if (!ret) return ret;		//something went wrong

    if (treeHeight == 1) splitRoot(pid, key2, end_pid);
    return 0;
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
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
    RC ret = 0;
    cursor.pid = rootPid;
    BTRawNonLeaf n;		//I'll use this to read data
    while (1)			//continuously try to read...
    {
	ret = n.read(cursor.pid, pf);
	if (ret < 0) return ret;	//finished reading
	if (n.isLeaf())		//handle leaf case
	{
	    BTLeafNode l(searchKey, cursor.pid);//leaf node
	    return l.locate(searchKey, cursor.eid);
	}
	else			//it's a non leaf node...
	{
	    BTNonLeafNode nl(n, cursor.pid);	//nonleaf node
	    ret = nl.locateChildPtr(searchKey, cursor.pid);//recursively locate
	    if (ret < 0) return ret;			//first entry in B+tree
	}
    } 
    return ret;
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

    ret = n.read(c_pid, pf);
    if (ret) return ret;		//encountered error

    ret = n.find(c_eid, key, rid);	//find key and rid
    if (ret) return ret;		//encountered error

    if (c_pid <= 0) return RC_INVALID_CURSOR; //out of bounds

    if (c_eid+1 < n.getKeyCount()) ++c_eid;
    else	//we have gone out of bounds
    {
	c_eid = 0;
	c_pid = n.getNextNodePtr();
    }
    cursor.eid = c_eid;//write the information back to the cursor
    cursor.pid = c_pid;
    return ret;
}
