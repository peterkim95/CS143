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
  string valEqConditions = "";

  int vitalIndex = -1;

  bool valConflictExists = false;
  bool usesIndex = false;   // ?
  bool hasValidSelCond = false;
  bool hasValidValCond = false;

  int eqVal = - 1;  // ?

  int min = -1;
  int max = 1;

  bool GEexists = false;
  bool LEexists = false;
  

  // iterate through all SelConds in cond to determine what the given select conditions are
  for (int i = 0; i < cond.size(); i++) {

    SelCond csc = cond[i];  // csc = currrent sel cond
    int selCondVal = atoi(csc.value);

    if (csc.attr == 1 && csc.comp != SelCond::NE) { // NE comparators are irrelevant as they don't use tree
      hasValidSelCond = true;

      if (csc.comp == SelCond::EQ) {
        eqVal = selCondVal;
        vitalIndex = i;
        break;
      } else if (csc.comp == SelCond::GT) { // >
        if (min == -1 || selCondVal >= min) {
          min = selCondVal;
          GEexists = false;
        }
      } else if (csc.comp == SelCond::GE) { // >=
        if (min == -1 || selCondVal > min) {
          min = selCondVal;
          GEexists = true;
        }
      } else if (csc.comp == SelCond::LT) { // <
        if (max == -1 || selCondVal <= max) {
          max = selCondVal;
          LEexists = false;
        }
      } else if (csc.comp == SelCond::LE) { // <=
        if (max == -1 || selCondVal < max) {
          max = selCondVal;
          LEexists = true;
        }
      }


    } else if (csc.attr == 2) {
      hasValidValCond = true;

      if (csc.comp == SelCond::EQ) {
        if (strcmp(csc.value, value.c_str()) == 0 || valEqConditions == "") {
          valEqConditions = selCondVal;
        } else {
          valConflictExists = true;
        }
      }

    }

  }

  if (min==max && min != -1 && max != -1 && !LEexists && !GEexists)
    goto early_termination;

  if ((max!=-1 && min!=-1 && max<min) || valConflictExists)
    goto early_termination;

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
    rid.sid = 0;
    rid.pid = 0;
    usesIndex = true;

    if (eqVal != -1) {
      bt.locate(eqVal, cur);
    } else if (GEexists && min != -1) {
      bt.locate(min, cur);
    } else if (!GEexists && min != -1) {
      bt.locate(min+1, cur);
    } else {
      bt.locate(0,cur);
    }

    while (bt.readForward(cur, key, rid) == 0) {
      if (attr == 4 && !hasValidValCond) {
        if (key!= eqVal && eqVal != -1) {
          goto early_termination;
        }

        if (min != -1) {
          if (GEexists && key < min)
            goto early_termination;
          if (!GEexists && key <= min)
            goto early_termination;
        }

        if (max != -1) {
          if (LEexists && key > max)
            goto early_termination;
          if (!LEexists && key >= min)
            goto early_termination;
        }

        count++;
        // continue;
        goto bt_continue_forward;
      }

      // read the tuple
      if ((rc = rf.read(rid, key, value)) < 0) {
        fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
        goto exit_select;
      }

      for (int i = 0; i < cond.size(); i++) {
        SelCond csc = cond[i];  // csc = currrent sel cond
        int selCondVal = atoi(csc.value);
        if (csc.attr == 1) {
          diff = key - selCondVal;
        } else if (csc.attr == 2) {
          diff = strcmp(value.c_str(), csc.value);
        }

        switch (csc.comp) {
          case SelCond::EQ:
            if (diff != 0) {
              if (csc.attr == 1)
                goto early_termination;
              goto bt_continue_forward;
            }
            break;
          case SelCond::NE:
            if (diff == 0) goto bt_continue_forward;
            break;
          case SelCond::GT:
            if (diff <= 0) goto bt_continue_forward;
            break;
          case SelCond::LT:
            if (diff >= 0) {
              if (csc.attr == 1)
                goto early_termination;
              goto bt_continue_forward;
            }
            break;
          case SelCond::GE:
            if (diff < 0) goto bt_continue_forward;
            break;
          case SelCond::LE:
            if (diff > 0) {
              if (csc.attr == 1)
                goto early_termination;
              goto bt_continue_forward;
            }
            break;
        }

      }
      count++;

      if (attr == 1) {
        fprintf(stdout, "%d\n", key);
      } else if (attr == 2) {
        fprintf(stdout, "%s\n", value.c_str());
      } else if (attr == 3) {
        fprintf(stdout, "%d '%s'\n", key, value.c_str());
      }

      bt_continue_forward: cout << "bt moving readForward";
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
  //variables
  RecordFile recf;  //RecordFile holding the table
  RecordId rid;   //cursor
  string in;    //reading in rows from 'loadfile'
  int key;    //key from input
  string val;     //value from input
  RC ret_code;    //return code

  //setting up file reads
  ifstream data(loadfile.c_str());  //create input stream from filepath
  string tablename = table + ".tbl";  //style for the table name 
  ret_code = recf.open(tablename, 'w');  //begin writing to the file

  BTreeIndex bt;
  if (!index)
  {
    //read each line into the table
    while (getline(data, in))
    {
      parseLoadLine(in, key, val);
      ret_code = recf.append(key, val, rid);
    }
  }
  else //part 2D -- check index for btree 
  {
    string tabind = table + ".idx";
    bt.open(tabind, 'w');
    while (getline(data, in))
    {
      parseLoadLine(in, key, val);
      if(recf.append(key, val, rid)) return RC_FILE_WRITE_FAILED; //nonzero
      if(bt.insert(key, rid)) return RC_FILE_WRITE_FAILED; //failed insert
    }
    bt.close();
  }

  recf.close();   //close RecordFile
  data.close();   //close datafile

  return ret_code;
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
