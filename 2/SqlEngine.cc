/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  IndexCursor cur;  // cursor for navigating through the B+ tree
  BTreeIndex bt;  // the B+ tree
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning

  RC     rc;
  int    key;     
  string value;
  int    count = 0;
  int    diff;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }
  bool hasValidSelCond = false;
  bool hasValidValCond = false;

  int v = - 1;

  int min = -1;
  int max = -1;

  bool GEexists = false;
  bool LEexists = false;
  
  // iterate through all SelConds in cond to determine what the given select conditions are
  for (int i = 0; i < cond.size(); i++) {

    switch (cond[i].attr) {
    	case 1:	// key
    		if (cond[i].comp!=SelCond::NE) {
    			hasValidSelCond = true;

			      if (cond[i].comp == SelCond::EQ) { //=
			        v = atoi(cond[i].value);
			        break;
			      } else if (cond[i].comp == SelCond::GT) { // >
			        if (min == -1 || atoi(cond[i].value) >= min) {
			          min = atoi(cond[i].value);
			          GEexists = false;
			        }
			      } else if (cond[i].comp == SelCond::GE) { // >=
			        if (min == -1 || atoi(cond[i].value) > min) {
			          min = atoi(cond[i].value);
			          GEexists = true;
			        }
			      } else if (cond[i].comp == SelCond::LT) { // <
			        if (max == -1 || atoi(cond[i].value) <= max) {
			          max = atoi(cond[i].value);
			          LEexists = false;
			        }
			      } else if (cond[i].comp == SelCond::LE) { // <=
			        if (max == -1 || atoi(cond[i].value) < max) {
			          max = atoi(cond[i].value);
			          LEexists = true;
			        }
			      }
    		}
    		break;
    	case 2:	// val
		    hasValidValCond = true;
    		break;
    }

    if ((max != -1 && min != -1) && (max < min || max == min && !LEexists && !GEexists))
    	goto early_termination;
  }


  if ((attr!=4 && !hasValidSelCond) || bt.open(table+".idx", 'r')!=0) {
      // scan the table file from the beginning
    rid.pid = rid.sid = 0;
    count = 0;
    while (rid < rf.endRid()) {
      // read the tuple
      if ((rc = rf.read(rid, key, value)) < 0) {
        fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
        goto exit_select;
      }

      // check the conditions on the tuple
      for (unsigned i = 0; i < cond.size(); i++) {
        // compute the difference between the tuple value and the condition value
        switch (cond[i].attr) {
	        case 1:
			    diff = key - atoi(cond[i].value);
			    break;
	        case 2:
			    diff = strcmp(value.c_str(), cond[i].value);
			    break;
        }

        // skip the tuple if any condition is not met
        switch (cond[i].comp) {
	        case SelCond::EQ:
			    if (diff != 0) goto next_tuple;
			    break;
	        case SelCond::NE:
			    if (diff == 0) goto next_tuple;
			    break;
	        case SelCond::GT:
			    if (diff <= 0) goto next_tuple;
			    break;
	        case SelCond::LT:
			    if (diff >= 0) goto next_tuple;
			    break;
	        case SelCond::GE:
			    if (diff < 0) goto next_tuple;
			    break;
	        case SelCond::LE:
			    if (diff > 0) goto next_tuple;
			    break;
        }
      }

      // the condition is met for the tuple. 
      // increase matching tuple counter
      count++;

      // print the tuple 
      switch (attr) {
      case 1:  // SELECT key
        fprintf(stdout, "%d\n", key);
        break;
      case 2:  // SELECT value
        fprintf(stdout, "%s\n", value.c_str());
        break;
      case 3:  // SELECT *
        fprintf(stdout, "%d '%s'\n", key, value.c_str());
        break;
      }

      // move to the next tuple
      next_tuple:
      ++rid;
    }
  } else {
    
    // setting indexcursor
    if (v != -1) bt.locate(v, cur);
    else if (min!=-1) GEexists ? bt.locate(min, cur) : bt.locate(min+1, cur);	// key is either greater than or bigger than min
    else bt.locate(0,cur);

    while (!bt.readForward(cur, key, rid)) {
		if (attr == 4 && !hasValidValCond) {	// count(*) no need to read disk
			// three fail conditions
			if (key!= v && v != -1) goto early_termination;	// condition on equality does not fit
			if ((min != -1) && ((GEexists && key < min) || (!GEexists && key <= min))) goto early_termination;	// GT or GE fail
			if ((max != -1) && ((LEexists && key > max) || (!LEexists && key >= min))) goto early_termination;	// LE or LT fail

			++count;
			continue;
		}

		// read the tuple
		if ((rc = rf.read(rid, key, value)) < 0) {
			fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
			goto exit_select;
		}

		// from given skelton code modified
    	for (unsigned int i = 0; i < cond.size(); i++) {
	      	if (cond[i].attr == 1) diff = key - atoi(cond[i].value);
	        else if (cond[i].attr == 2) diff = strcmp(value.c_str(), cond[i].value);
	        

	        switch (cond[i].comp) {
				case SelCond::EQ:
					if (diff != 0)
						if (cond[i].attr == 1) goto early_termination;	//key faliure
						else goto bt_next_cursor;
					break;
				case SelCond::LE:
					if (diff > 0)
						if (cond[i].attr == 1) goto early_termination;	// key failure
						else goto bt_next_cursor;
					break;
				case SelCond::LT:
					if (diff >= 0)
						if (cond[i].attr == 1) goto early_termination; // key failure
						else goto bt_next_cursor;
					break;
				case SelCond::NE:
					if (diff == 0) goto bt_next_cursor;	// go to next cursor
					break;
				case SelCond::GT:
					if (diff <= 0) goto bt_next_cursor;
					break;
				case SelCond::GE:
					if (diff < 0) goto bt_next_cursor;
					break;
	        }
      	}

    ++count;

	if (attr == 1) fprintf(stdout, "%d\n", key);
	else if (attr == 2) fprintf(stdout, "%s\n", value.c_str());
	else if (attr == 3) fprintf(stdout, "%d '%s'\n", key, value.c_str());
	
    bt_next_cursor: continue;

    }
  }

  early_termination:
  
  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  rc = 0;

  // close the table file and return
  exit_select:
  rf.close();
  return rc;

}

// ** ONLY MODIFY THIS FUNCTION FOR PART A
// This function is invoked when user issues LOAD command
// It will load tuples into the table from the load file...do not worry about
//  the index part for now...
// Created Recordfile should be named as tablename + ".tbl" in current working 
//  directory. If *.tbl file already exists, append rows.
// Make use of SqlEngine::parseLoadLine()

// Testing: sample data file -- "movie.del" and RecordFile "movie.tbl"
RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
	RecordFile recf;  //Reocrd file holding the info
	RecordId rid;   //iterator
	RC rc;    //return code

	//setting up file reads
	ifstream data(loadfile.c_str());  //create input stream from filepath
	rc = recf.open(table + ".tbl", 'w');
	
	string in;    //input str
	int key;      //read in key
	string val;   //read in value
	
	if (!index) // part 2A
	{ //read each line into the table
		while (getline(data, in))
		{
			parseLoadLine(in, key, val);
			rc = recf.append(key, val, rid);
		}
	}
	else //part 2D -- check index for btree if idx file exists.
	{
		BTreeIndex bt;
		rc = bt.open(table + ".idx", 'w'); //read the idx file
		while (getline(data, in))
		{
			parseLoadLine(in, key, val);
			if (recf.append(key, val, rid)) return RC_FILE_WRITE_FAILED; 
			if(bt.insert(key, rid)) return RC_FILE_WRITE_FAILED; //failed insert
		}
		bt.close();
	}
	recf.close();   //close RecordFile
	data.close();   //close datafile

	return rc;
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}
