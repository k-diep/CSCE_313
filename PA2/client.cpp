/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/20
 */
#include "common.h"
#include "FIFOreqchannel.h"

#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/wait.h>
#include <math.h>

using namespace std;


int main(int argc, char *argv[]){
    
    
    //reading
    //default values
    int patient = 0;
    double time = 0.0;
    int ecgno = 0;
    string fileName = "";
    int bufferCapacity = MAX_MESSAGE;
    bool newChannel = false;
    char* bufferCapSTR = "256";
    

    int c;
    while((c = getopt(argc, argv, "p: t: e: f: m: c")) != -1){
  	    switch(c){
		    case 'p':
			    patient = atoi(optarg);
			break;

		    case 't':
			    time = atoi(optarg);
			break;

            case 'e':
			    ecgno = atoi(optarg);
			break;

            case 'f':
			    fileName = optarg;
			break;

            case 'm':
			    bufferCapacity = atoi(optarg);
                bufferCapSTR = optarg;
			break;

            case 'c':
			    newChannel = true;
			break;
	    }
    }
    //cout << bufferCapSTR << endl;
    int pid = fork();
    //child (runs the server in parallel)
    if (pid == 0){
        char* arglist[] = {"null", "-m", bufferCapSTR, NULL};
        execvp("./server", arglist);   
    }
    //parent
    else{
    int n = 100;    // default number of requests per "patient"
	int p = 15;		// number of patients
    srand(time_t(NULL));

    FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);

    if (patient!=0 && ecgno!=0){
        //finding a single point using command line
        datamsg singleData = datamsg(patient, time, ecgno);
        char* buffer = new char [sizeof(datamsg)];
        chan.cwrite(&singleData, sizeof(singleData));
        int test = chan.cread(buffer, sizeof(singleData));
        double data1 = *(double*)buffer;
        cout << "Single point using from the command line" << endl;
        cout << data1 << " is the data."<< endl;
        cout << endl;
    }
        //requesting all data point for person 1
    if (patient != 0 && ecgno == 0 && time == 0){
        timeval start; //timing
        timeval finish;
        gettimeofday(&start, NULL);
        ofstream ost;
        ost.open("received/x1.csv");
    
        for(double i =0; i<59.996; i += .004){
            ost << i << ",";

            datamsg dataEc1 = datamsg(1 , i , 1);
            datamsg dataEc2 = datamsg(1 , i , 2);

            char* buffer2 = new char [sizeof(datamsg)];

            chan.cwrite(&dataEc1, sizeof(dataEc1));
            chan.cread(buffer2, sizeof(dataEc1));
            double data2 = *(double*) buffer2;
            //cout << data2 << endl;
            ost << data2 << ",";

            chan.cwrite(&dataEc2, sizeof(dataEc2));
            chan.cread(buffer2, sizeof(dataEc2));
            double data3 = *(double*) buffer2;
            //cout << data3 << endl;
            ost << data3 << endl;
        }
         
        ost.close();
        gettimeofday(&finish, NULL);
        
        double timeElaspedSecs, timeElaspedMusecs;
        timeElaspedSecs = (finish.tv_sec - start.tv_sec);
        timeElaspedMusecs = (finish.tv_usec - start.tv_usec);
        if (timeElaspedMusecs < 0) { //PA1 code
            timeElaspedMusecs += (int)1e6;
            timeElaspedMusecs--;
        }
        cout << "Time elasped for the transfering 1.csv data: " <<  timeElaspedSecs << " secs, " << timeElaspedMusecs << " musecs." << endl;
    }

    //requesting files
    if (fileName != ""){
        //cout << fileName << endl;
        filemsg f = filemsg(0,0);
        char* buffer3 = new char[sizeof(filemsg)+fileName.size()+1];
        char* buffer4 = new char[sizeof(__int64_t)];
       
        *(filemsg*)buffer3 = f;
        strcpy(buffer3 + sizeof(filemsg), fileName.c_str());
        chan.cwrite(buffer3, sizeof(filemsg)+fileName.size()+1);
        chan.cread(buffer4, sizeof(__int64_t));
        __int64_t fileLength = *(__int64_t*)buffer4;
        
        //cout << fileLength << endl;
    
        timeval start2; //timing
        timeval finish2;
        gettimeofday(&start2, NULL);
        
        int numRequests = ceil((double)fileLength/(double)bufferCapacity);
        cout << numRequests << endl;
        int offset = 0;
        int bufferInitial = bufferCapacity;
        char* buffer5 = new char[sizeof(filemsg)+fileName.size()+1];
        char* buffer6 = new char[bufferCapacity];
        FILE * file1;
        file1 = fopen("y1.csv", "w");
        //cout << bufferCapacity << endl;
       
        for(int i = 0; i<numRequests; ++i){
            if((fileLength - offset) < bufferCapacity){
                bufferCapacity = fileLength - offset;
            } 
            filemsg f2 = filemsg(offset, bufferCapacity);
            *(filemsg*)buffer5 = f2;
            strcpy(buffer5 + sizeof(filemsg), fileName.c_str());
            chan.cwrite(buffer5, sizeof(filemsg)+fileName.size()+1);
            chan.cread(buffer6, bufferCapacity);
            fwrite(buffer6, 1, bufferCapacity, file1);
            offset = offset + bufferCapacity;
        }
        
        fclose(file1);
        bufferCapacity = bufferInitial;
        gettimeofday(&finish2, NULL);
        
        double timeElaspedSecs, timeElaspedMusecs;
        timeElaspedSecs = (finish2.tv_sec - start2.tv_sec);
        timeElaspedMusecs = (finish2.tv_usec - start2.tv_usec);
        if (timeElaspedMusecs < 0) { //PA1 code
            timeElaspedMusecs += (int)1e6;
            timeElaspedMusecs--;
        }
        cout << "Time elasped for the transfering file: " <<  timeElaspedSecs << " secs, " << timeElaspedMusecs << " musecs." << endl;
    }

    //opening a new channel

    if(newChannel){
        MESSAGE_TYPE n = NEWCHANNEL_MSG;
        chan.cwrite (&n, sizeof (MESSAGE_TYPE));
        FIFORequestChannel chan2 ("data1_", FIFORequestChannel::CLIENT_SIDE);
        
        
        datamsg singleData = datamsg(1, 0.0, 1);
        char* buffer = new char [sizeof(datamsg)];
        chan2.cwrite(&singleData, sizeof(singleData));
        int test = chan2.cread(buffer, sizeof(singleData));
        double data1 = *(double*)buffer;
        cout << "Single point using from the command line in the new channel" << endl;
        cout << data1 << " is the data."<< endl;
        cout << endl;
        
        singleData = datamsg(1, 0.0, 2);
        chan2.cwrite(&singleData, sizeof(singleData));
        test = chan2.cread(buffer, sizeof(singleData));
        data1 = *(double*)buffer;
        cout << "Single point using from the command line in the new channel" << endl;
        cout << data1 << " is the data."<< endl;
        cout << endl;

        MESSAGE_TYPE m2 = QUIT_MSG;
        chan2.cwrite (&m2, sizeof (MESSAGE_TYPE));
        
    }

    // sending a non-sense message, you need to change this
    /*
    char buf [MAX_MESSAGE];
    char x = 55;
    chan.cwrite (&x, sizeof (x));
    int nbytes = chan.cread (buf, MAX_MESSAGE);
    */
    // closing the channel  

    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite (&m, sizeof (MESSAGE_TYPE));
    wait(NULL);
    }
}
