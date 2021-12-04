#include "include/common_tongbu_server.h"
#include "include/common_md5.h"
#include "include/common_RIO_server.h"
#include "include/common_order.h"

map<string, set<Socket_RIO*> > all_rio;

int main(int argc,char ** argv){
    signal(SIGPIPE, SIG_IGN);
	// init_daemon();
    if(argc!=2){
        cout<<"参数有误"<<endl;
        exit(0);
    }
    
    map<int,Socket_RIO* > mp;

    /* 作为服务端，与客户端的链接初始化 */
    int server_fd,connect_fd;
    server_init_noacc(server_fd,atoi(argv[1]),"0.0.0.0",NONBLOCK_AFTER_SOCKET);

    struct sockaddr_in client_addr,socket_temp;
    socklen_t cLen = sizeof(socket_temp);
	socklen_t length = sizeof(client_addr);

    int cntSendbyte=0,cntReadbyte=0;

	int cnt, fd_now;
	int epfd = epoll_create1(0);
	if (epfd == -1) {
        Myexit ("epoll_create");
    }
	struct epoll_event events[128],event;
	event.events =  EPOLLIN | EPOLLOUT;
	event.data.fd = server_fd;
	my_epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &event);
	memset(events,0,sizeof(events));

    REQUEST response;
    ofstream fout;
    
    while(1){
		cnt = epoll_wait(epfd, events, 128, 0);
		for(int i=0;i<cnt;i++){
			if(events[i].data.fd==server_fd) {
				my_accept(server_fd, connect_fd , client_addr, length);
				
                event.events = EPOLLIN;
				event.data.fd = connect_fd;
				my_epoll_ctl(epfd, EPOLL_CTL_ADD, connect_fd, &event);

                fout.open("log.log",ofstream::app);
                fout<<"fd:"<<connect_fd<<" connect"<<endl;
                fout.close();

                Socket_RIO* rio =new Socket_RIO(connect_fd);
                mp[connect_fd]=rio;

			}
			else if(events[i].events & EPOLLIN){
                fd_now=events[i].data.fd;
                Socket_RIO* rio=mp[fd_now];

                int lnow=rio->rlen();
                if(lnow==0){//断开连接
                    fout.open("log.log",ofstream::app);
                    fout<<"user "<<rio->user_id<<" "<<rio->cp_ip<<" logout"<<endl;
                    fout.close();

                    string now=rio->user_id;
                    all_rio[now].erase(rio);
                    delete[] rio;

                    mp.erase(fd_now);
                    event.events = EPOLLIN;
                    event.data.fd = fd_now;
                    my_epoll_ctl(epfd, EPOLL_CTL_DEL, fd_now, &event);
                    continue;
                }
                
                int length;
                rio->peek(&length);
                
                if(lnow<length){
                    continue;
                }

                while(lnow>=length){
                    
                    rio->read(&response,length);
                    if(response.base.type==delFile){
                        fout.open("log.log",ofstream::app);
                        printTime(time(0),fout);
                        fout<<"recv delFile from"<<rio->user_id<<"-"<<rio->cp_ip<<endl;
                        fout.close();

                        delfile(response,fd_now,rio,all_rio);
                    }
                    else if(response.base.type==addFile){
                        fout.open("log.log",ofstream::app);
                        printTime(time(0),fout);
                        fout<<"recv addFile from"<<rio->user_id<<"-"<<rio->cp_ip<<endl;
                        fout.close();
                        addfile(response,fd_now,rio,all_rio);
                    }
                    else if(response.base.type==sendFile){
                        fout.open("log.log",ofstream::app);
                        printTime(time(0),fout);
                        fout<<"recv sendFile from"<<rio->user_id<<"-"<<rio->cp_ip<<endl;
                        fout.close();
                        sendfile(response,fd_now,rio);
                    }
                    else if(response.base.type==modifyFileName){
                        fout.open("log.log",ofstream::app);
                        printTime(time(0),fout);
                        fout<<"recv modifyFileName from"<<rio->user_id<<"-"<<rio->cp_ip<<endl;
                        fout.close();
                        modifyfilename(response,fd_now,rio,all_rio);
                    }
                    else if(response.base.type==Register){
                        fout.open("log.log",ofstream::app);
                        printTime(time(0),fout);
                        fout<<"recv Register"<<endl;
                        fout.close();
                        register_in(response,rio);
                    }
                    else if(response.base.type==Login){
                        fout.open("log.log",ofstream::app);
                        printTime(time(0),fout);
                        fout<<"recv Login"<<endl;
                        fout.close();
                        login_in(response,rio,all_rio);
                    }
                    else if(response.base.type==sendUnbundling){
                        fout.open("log.log",ofstream::app);
                        printTime(time(0),fout);
                        fout<<"recv Unbundling from"<<rio->user_id<<"-"<<rio->cp_ip<<endl;
                        fout.close();
                        unbundling(response,rio);
                    }
                    else if(response.base.type==sendBundling){
                        fout.open("log.log",ofstream::app);
                        printTime(time(0),fout);
                        fout<<"recv Bundling from"<<rio->user_id<<"-"<<rio->cp_ip<<endl;
                        fout.close();
                        bundling(response,rio);
                    }
                    else if(response.base.type==need_init_synchronous){
                        fout.open("log.log",ofstream::app);
                        printTime(time(0),fout);
                        fout<<"recv server_init_order from"<<rio->user_id<<"-"<<rio->cp_ip<<endl;
                        fout.close();
                        init_synchronous(rio);
                        fout.open("log.log",ofstream::app);
                        fout<<"server_init success"<<endl;
                        fout.close();
                    }
                    else if(response.base.type==responseSend){
                        fout.open("log.log",ofstream::app);
                        printTime(time(0),fout);
                        fout<<"recv responseSend from"<<rio->user_id<<"-"<<rio->cp_ip<<endl;
                        fout.close();
                        responsesend(response,fd_now,rio,all_rio);
                    }
                    else if(response.base.type==responseBase){
                        fout.open("log.log",ofstream::app);
                        printTime(time(0),fout);
                        fout<<"recv responseBase"<<endl;
                        fout.close();
                        responsebase(response,fd_now);
                    }   
                    
                    lnow-=length; 
                    rio->peek(&length);
                }
                
			}
		}
    }
    close(server_fd);
    return 0;
}



