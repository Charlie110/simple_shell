//fuheng zhao
#include <errno.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#define TOKENMAX 32
#define MAXLENGTH 512

using namespace std;


struct sigaction sigchld_action = {
	sigchld_action.sa_handler = SIG_DFL,
	sigchld_action.sa_flags = SA_NOCLDWAIT
};


void siginthandler(int sig_num){

	signal(SIGINT, siginthandler);
	
}

char *strremove(char *str, char *sub){
	size_t len=strlen(sub);
	if(len>0){
		char*p=str;
		while((p=strstr(p,sub))!=NULL){
			memmove(p, p+len, strlen(p+len)+1);
		}
		
	}
	return str;

}

int parseError(vector<char*> allline){
	int countofless=0;
	int countofmore=0;
	int countofand=0;
	if(allline.size()==1){
		char *line=allline[0];
		for(size_t i=0; i<strlen(line); i++){
			if(line[i]=='<'){
				countofless+=1;
			}
			else if(line[i]=='>'){
				countofmore+=1;
			}
			else if(line[i]=='&'){
				countofand+=1;
			}
		}
		if(countofmore>1 || countofless>1 || countofand >1){
			return -1;
		}
		if(countofand==1){
			if(line[strlen(line)-1] != '&'){
				return -1;
			}
		}
		return 0;
	}
	else{
		//mutilple commands
		char* firstline=allline[0];
		for(size_t i=0; i<strlen(firstline);i++){
			if(firstline[i]=='>'){
				return -1;
			}
			else if(firstline[i]=='<'){
				countofless+=1;
			}
			else if(firstline[i]=='&'){
				return -1;
			}
		}
		//middle ones
		for(size_t j=1; j<allline.size()-1;j++){
			char *line=allline[j];
			for(size_t k=0; k<strlen(line);k++){
				if(line[k] == '<' || line[k] == '>' || line[k] == '&'){
					return -1;
				}
			
			}
		}
		char *lastline=allline[allline.size()-1];
		for(size_t i=0; i<strlen(lastline); i++){
			if(lastline[i]=='<'){
				return -1;
			}
			else if(lastline[i]=='>'){
				countofmore+=1;
			}
			else if(lastline[i]=='&'){
				countofand+=1;
			}
		}
		
		if(countofmore>1 || countofless>1 || countofand >1){
                        return -1;
                }
                
		if(countofand==1){
                        if(lastline[strlen(lastline)-1] != '&'){
                                return -1;
                        }
                }
                return 0;	
	
	
	}
	return 0;


}

int main(int argc, char** arg){
	
	int status;
	const char s[2]="|";
	int pip='|';
	char *found; 
	int count =0;
	signal(SIGINT, siginthandler);
	vector<pid_t> rest;
	while(1){
		int isbackground=0;
		//printf("beginning \n");
		if(count){
			for(size_t t=0; t<rest.size(); t++){
				waitpid(rest[t], &status, 0);
			}
			rest.clear();
			count=0;

		}
		sigaction(SIGCHLD,&sigchld_action, NULL );
		if(argc<2){
			printf("shell: ");
		}
		found=NULL;
		char *line=NULL;
        	char *sub=NULL;
		size_t size;
		
		int stdout_copy=dup(STDOUT_FILENO);

        	if(getline(&line,&size,stdin)==-1){
				printf("\nExiting \n");
				return 0;
			}
			
			int len=strlen(line);
			if(len>0 && line[len-1]=='\n'){
				line[len-1]='\0';
			}

		found=strchr(line,pip);
		
		vector<char*> stmts;
		if(found!=NULL){
			//has pipe
        		sub=strtok(line,s);    
        		while(sub!=NULL){
                		stmts.push_back(sub);
               			sub=strtok(NULL,s);
        		}	
		}
		else{
			stmts.push_back(line);
			
		}

		int error=parseError(stmts);
		if(error==-1){
			printf("ERROR: Parse Error \n");
		}
		else if(stmts.size()==1){
			char *l=stmts[0];
			char *token=NULL;
			std::string input="";
			std::string output="";
			if(l[strlen(l)-1]=='&'){
				isbackground=1;
			}
			for(size_t i=0; i<strlen(l);i++){
				if(l[i]=='<'){
					for(size_t j=i+1; j<strlen(l);j++){
						if((l[j]==' ')){
						}
						else if(l[j]=='>'|| l[j]=='&'){
							break;
						}
						else{
							input+=l[j];	}
					}
				}
			}
			for(size_t i=0; i<strlen(l); i++){
				//std::cout<<l[i]<<" \n";
				if(l[i]=='>'){
					for(size_t j=i+1; j<strlen(l);j++){
						if((l[j]==' ')){
						
						}
						else if(l[j]=='<' || l[j] =='&'){
                                                        break;
                                                }
                                                else{
							output+=l[j];
                                                }
                                        }
				}	
			}

			//std::cout<<"output file is : "<<output<<" \n";

			char temp1[2]="<";
			char *in=new char[input.length()+1];
			char temp2[2]=">";
			char *out=new char[output.length()+1];
			char andchar[2]="&";
			if(input.length()>0){
				std::strcpy(in, input.c_str());
				l=strremove(l, temp1);	
				l=strremove(l, in);
				//printf("l is now in removed input %s \n", l);
			
			}
			if(output.length()>0){
                                std::strcpy(out, output.c_str());
				l=strremove(l,temp2);
				l=strremove(l,out);
				//printf("l is now in removed output %s \n", l);
				
			}
			if(isbackground){
				l=strremove(l,andchar);
			}
				

			vector<char*> tokens;

			token=strtok(l," ");
			while(token!=NULL){
				tokens.push_back(token);
				token=strtok(NULL, " ");
			}
			
			char *argv[MAXLENGTH];
			size_t t=0;
			for(t; t<tokens.size();t++){
				argv[t]=tokens[t];
			}
			argv[t]=NULL;
		
			pid_t pid=fork();
			if(pid==-1){
				printf("ERROR: %s \n", strerror(errno));
			}
			else if(pid==0){
				if(input.length()>0){
					int istr=open(in, O_RDONLY);
					if(istr==-1){perror(in);return 255;}
					dup2(istr,0);
				
				}
				if(output.length()>0){
					int ostr=creat(out, 0644);
					if(ostr==-1){perror(out); return 255;}
					dup2(ostr,1);
				
				}
				delete []in;
				delete []out;
				if(-1 == execvp(argv[0], argv)){
					printf("ERROR: %s \n", strerror(errno));
					exit(1);	
				}
			
			}
			else{	
				if(isbackground){
					waitid(P_PID, pid, NULL, WNOHANG);
				}
				else{
					waitpid(pid, &status, 0);}
			}
		}
		else{
			//handle pip
			
			size_t length=0;
			vector<pid_t> allpid;
			vector<vector<int>> allpd;
			char* lastcmd = stmts[stmts.size()-1];
			if(lastcmd[strlen(lastcmd)-1]=='&'){
				isbackground=1;
			}
			while(length<stmts.size()){
				char *l=stmts[length];
                        	char *token=NULL;
                        	std::string input="";
                        	std::string output="";
                        	for(size_t i=0; i<strlen(l);i++){
                                	if(l[i]=='<'){
                                        	for(size_t j=i+1; j<strlen(l);j++){
                                                	if(l[j]==' '){
								
							}
							else if(l[j]=='>' || l[j]=='&' ){
                                                        	break;
                                                	}
                                              		else{
                                                        	input+=l[j];  }
                                        	}
                                	}
                        	}
                        	for(size_t i=0; i<strlen(l); i++){
                                	if(l[i]=='>'){
                                        	for(size_t j=i+1; j<strlen(l);j++){
                                                	if(l[j]==' '){
							}
							else if(l[j]=='<' || l[j]=='&' ){
                                                        	break;
                                                	}
                                                	else{
                                                        	output+=l[j];
                                                	}
                                        	}
                                	}
                        	}
				char *in=new char[input.length()+1];
				char *out=new char[output.length()+1];
				if(length==0){
					//first command
					char temp1[2]="<";
					if(input.length()>0){
                                		std::strcpy(in, input.c_str());
                                		l=strremove(l, temp1);
                                		l=strremove(l, in);
                                
					}

				
				}
				else if(length==stmts.size()-1){
					//last command
					char temp2[2]=">";
					if(output.length()>0){
                                		std::strcpy(out, output.c_str());
                                		l=strremove(l,temp2);
                                		l=strremove(l,out);

                        		}
					if(isbackground){
						char temp3[2]="&";
						l=strremove(l,temp3);
					}

				}
				vector<char*> tokens;

                        	token=strtok(l," ");
                        	while(token!=NULL){
                                	tokens.push_back(token);
                                	token=strtok(NULL, " ");
                        	}

                        	char *argv[MAXLENGTH];
                        	size_t t=0;
                        	for(t; t<tokens.size();t++){
					//printf("argv put in : %s \n", tokens[t]);
                                	argv[t]=tokens[t];
                        	}
				//printf("now t is %d \n", t);
                        	argv[t]=0;
				
				int pd[2];
				pipe(pd);
				vector<int> used_for_temp;
				used_for_temp.push_back(pd[0]);
				used_for_temp.push_back(pd[1]);
				allpd.push_back(used_for_temp);
				
				pid_t pid=fork();
				if(pid!=0){
					allpid.push_back(pid);
				}

				if(pid==0){
					if(length==0){
						if(input.length()>0){
							int istr=open(in, O_RDONLY);
                                        		if(istr==-1){perror(in);return 255;}
                                        		dup2(istr,0);
						}
						//printf("first argument: redirect wirte %d \n", allpd[0][1]);
						dup2(allpd[length][1],1);

						close(allpd[0][0]);
						close(allpd[0][1]);
						delete []in;
						delete []out;
						execvp(argv[0], argv);
					}
					else if(length==stmts.size()-1){
						//printf("allpd[0][0] %d \n", allpd[0][0]);

						dup2(allpd[length-1][0],0);
						
						for(size_t k = 0; k<allpd.size(); k++){
							close(allpd[k][0]);
							close(allpd[k][1]);
						}

						if(output.length()>0){
							int ostr=creat(out, 0644);
                                       			if(ostr==-1){perror(out); return 255;}
                                        		dup2(ostr,1);
							delete []in;
							delete []out;
							execvp(argv[0], argv);
						}
						else{ 
							//printf("in the last argument redirect read is: %d \n", allpd[length-1][0]);
							//printf("the last argument is : %s \n", argv[0]);
							//printf("second argument is : %s \n", argv[1]);	
							delete []in;
							delete []out;
							if(-1==execvp(argv[0], argv)){
								printf("ERROR %s \n", strerror(errno));
								exit(1);
							}
							//printf("shoud no be here \n");	
						}
					}
					else{	
					
						//printf("now in second cmd \n");
						//printf("read from %d \n", allpd[length-1][0]);
						//printf("write from %d \n", allpd[length][1]);
						dup2(allpd[length-1][0],0);
						dup2(allpd[length][1],1);
						for(size_t k=0; k<allpd.size();k++){
							
							close(allpd[k][0]);
							close(allpd[k][1]);
							
						
						}
						delete[] in;
						delete[] out;
						if(-1==execvp(argv[0], argv)){
							dup2(stdout_copy,1);
							printf("ERROR: %s \n", strerror(errno));
							exit(1);
						}
						
					}	
				}
				else if(pid==-1){
                                	printf("ERROR: %s \n", strerror(errno));
                        	}		
				
				//printf("end of while look \n");
				length+=1;
			}

			if(isbackground){
				for(size_t k=0; k<stmts.size();k++){
					waitpid(allpid[k], &status, WNOHANG);
				}
			}
			else{		
				//waitpid(-1, &status, 0);	
				//printf("the pid of the last: %d \n", allpid[stmts.size()-1]);
				for(size_t k=0; k<stmts.size();k++){
					rest.push_back(allpid[k]);
					waitid(P_PID, allpid[k],NULL, WNOWAIT);
					count+=1;
				}
			}
			for(size_t k=0; k<allpd.size(); k++){
				close(allpd[k][0]);
				close(allpd[k][1]);
			}


		}
	
	}


	return 0;
}
