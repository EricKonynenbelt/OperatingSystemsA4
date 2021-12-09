/************************************************************************************
 * NAME: Eric Konynenbelt															*
 * CLASS: CSC 456																	*
 * ASSIGNMENT: 4																	*
 * DUE DATE: 12/07/2021																*
 * INSTRUCTOR: Kwanghee Won															*
 *																					*
 * DESCRIPTION: Create a paging-based memory system utilizing the LRU algorithm		*
 ************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <map>

using namespace std;

    int* memory;
    vector<int> freeFrameList;
    vector<int> processList;
    vector<pair<int, int>> LRUstack;
    map<int, int*>pageTables;

    //creates memory space with frames
    void memoryManager(int memSize, int frameSize = 1)
    {
        memory = new int[memSize];
        for(int i = 0; i < memSize; i++)
        {
            memory[i] = 0;
            freeFrameList.push_back(i);
        }
    }

        int deallocateFrame(int pid, int frame)
    {
        for(int i = 0; i < processList.size(); i++)
        {
            if (pid == processList[i])
            {
                processList[i+1]--;
                if (processList[i+1] == 0)
                {
                    processList.erase(processList.begin() + i);
                    processList.erase(processList.begin() + i);
                }
                freeFrameList.push_back(frame);
                return 1;
            }
            i++;
        }
        return -1;
    }

    //creates pages with specified size and process ID
    int allocate(int allocSize, int pid)
    {
        int *arr;                   //pointer to allocated space
        arr = new int[allocSize];    //initializes space
        int i, frame, numFrames, framePid;
        bool exists = false;
        if (freeFrameList.size() < allocSize)
        {
            numFrames = allocSize - freeFrameList.size();
            for(int i = 0; i < numFrames; i++)
            {
                framePid = LRUstack[0].first;
                frame = LRUstack[0].second;
                deallocateFrame(framePid, frame);
                LRUstack.erase(LRUstack.begin());
            }
        }
        if (freeFrameList.size() >= allocSize)
        {
            pageTables[pid] = arr;          //initializes pageTables map
            for(int i = 0; i < processList.size(); i++)
            {
                if(pid == processList[i])
                {
                    processList[i+1]+=allocSize;
                    exists = true;
                }
                i++;
            }
            if(!exists)
            {
                processList.push_back(pid);
                processList.push_back(allocSize);
            }

            for(i = 0; i < allocSize; i++)
            {
                random_device generator;        //randomly generates which frame to use
                mt19937 generated(generator());
                shuffle(freeFrameList.begin(), freeFrameList.end(), generated);
                frame = freeFrameList.back();   //stores generated frame and pops from freeFrameList
                pageTables[pid][i] = frame;
                LRUstack.push_back({pid, frame});
                freeFrameList.pop_back();
            }
        return 1;
        }
        else return -1;
    }

    int deallocate(int pid)
    {
        int processSize = 0, vectorIndex = 0, frame = 0;
        bool found = false;

        //Find pid and allocated frames in processList
        for(int i = 0; i < processList.size(); i++)
        {
            if(pid == processList[i])
            {
                vectorIndex = i;
                processSize = processList[i+1];
                found = true;
            }
        }

        //Free frames and set to 0 in memory
        for(int j = 0; j < processSize; j++)
        {
            frame = pageTables[pid][j];
            freeFrameList.push_back(frame);
            memory[frame] = 0;
        }

        //Delete pid from pageTables and processList
        if(found)
        {
            delete [] pageTables[pid];
            processList.erase(processList.begin() + vectorIndex);
            processList.erase(processList.begin() + vectorIndex);

            return 1;
        }
        else return -1;
    }

    //Writes a '1' to the frame specified by given pid and page number (logical_address)
    int write(int pid, int logical_address)
    {
        for(int i = 0; i < LRUstack.size(); i++)
        {
            if(LRUstack[i].first == pid && LRUstack[i].second == logical_address)
            {
                LRUstack.erase(LRUstack.begin() + i);
                LRUstack.push_back({pid, logical_address});
            }
        }

        int frame = pageTables[pid][logical_address];

        if(memory[frame] == 0)
        {
            memory[frame] = 1;
            return 1;
        }
        return 0;
    }

    //Returns value at frame specified by given pid and page number (logical_address)
    int read(int pid, int logical_address)
    {
        for(int i = 0; i < LRUstack.size(); i++)
        {
            if(LRUstack[i].first == pid && LRUstack[i].second == logical_address)
            {
                LRUstack.erase(LRUstack.begin() + i);
                LRUstack.push_back({pid, logical_address});
            }
        }
        return memory[pageTables[pid][logical_address]];
    }

    //Displays free frames as well as current processes and their respective sizes and id's
    void printMemory()
    {
        cout << "Free Frame List:\n";
        for(int i = 0; i < freeFrameList.size(); i++)
            cout << freeFrameList[i] << " ";

        cout << "\nProcess List:\n";
        for(int j = 0; j < processList.size(); j++)
        {
            cout << "PID: " << processList[j];
            j++;
            cout << "\tProcess Size: " << processList[j] << endl;
        }
    }

    void split(const string &s, char delim, vector<string> &elems) 
    {
        stringstream ss(s);
        string item;
        while (getline(ss, item, delim))
            elems.push_back(item);
    }

    vector<string> split(const string &s, char delim) 
    {
        vector<string> elems;
        split(s, delim, elems);
        return elems;
    }

    int main()
    {
        string line = "";
        vector<string> args;
        int arg1_int = 0, arg2_int = 0;
        bool run = true;
        while(run)
        {
            args.clear();
            cout << "Please enter a command ('E' to exit): ";
            getline(cin, line);
            args = split(line,' ');
            if(args.size() == 3)
            {
                arg2_int = stoi(args[2]);
                arg1_int = stoi(args[1]);
            }
            else if (args.size() == 2)
            {
                arg1_int = stoi(args[1]);
            }
            if(line[0] == 'M')
                memoryManager(arg1_int, arg2_int);
            else if(line[0] == 'A')
                allocate(arg1_int, arg2_int);
            else if(line[0] == 'W')
                write(arg1_int, arg2_int);
            else if(line[0] == 'R')
                read(arg1_int, arg2_int);
            else if(line[0] == 'D')
                deallocate(arg1_int);
            else if(line[0] == 'P')
                printMemory();
            else if(line[0] == 'E')
                run = false;
        }
    }