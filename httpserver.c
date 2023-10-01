#include"http.h"

i32 server_fd = -1;
i32 epoll_fd = -1;
FILE* source = NULL;

void MainExit(i32 sig)
{
	_LogFileWrite(_LOGPATH,1,"\nMainEixt start\n");
	_CallDebug(1,"\nMainEixt start\n");

	close(epoll_fd);
	close(server_fd);

	_LogFileWrite(_LOGPATH,1,"MainEixt end\n");
	_CallDebug(1,"MainEixt end\n");
	exit(0);
}

int main()
{
	i32 i = 0;

	while(i < 100){ signal(i,SIG_IGN); ++i; }
	signal(2,MainExit);
	signal(15,MainExit);

	if(-1 == (server_fd = ServerSocketConstruct(80)))
	{
		_LogFileWrite(_LOGPATH,1,"main: ServerSocketConstruct() Error\n");
		_CallDebug(1,"main: ServerSocketConstruct() Error\n");
		return 0;
	}

	epoll_fd = epoll_create(1);

	struct epoll_event ev;
	u32 epoll_eventSize = sizeof(struct epoll_event);

	ev.data.fd = server_fd;
	ev.events = EPOLLIN;
	epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_fd,&ev);

	_LogFileWrite(_LOGPATH,1,"waiting server connect...\n");
	_CallDebug(1,"waiting server connect...\n");

	while(true)
	{
		struct epoll_event events[MAXCLIENTS];
		i32 eventNums = 0;
		
		if((eventNums = epoll_wait(epoll_fd,events,MAXCLIENTS,-1)) < 0)
		{
			_LogFileWrite(_LOGPATH,1,"main: epoll_wait() Error\n");
			_CallDebug(1,"main: epoll_wait() Error\n");
			break;
		}

		for(i = 0;i < eventNums;++i)
		{
			if((server_fd == events[i].data.fd) && (events[i].events & EPOLLIN))
			{		
				i32 socklen = sizeof(struct sockaddr_in);
				struct sockaddr_in client_addr;
				i32 accept_fd = accept(server_fd,(struct sockaddr*)&client_addr,(socklen_t*)&socklen);
				
				memset(&ev,0,epoll_eventSize);
				ev.data.fd = accept_fd;
				ev.events = EPOLLIN;
				epoll_ctl(epoll_fd,EPOLL_CTL_ADD,accept_fd,&ev);
			}
			else
			{
				HttpAccept(events[i].data.fd);
				close(events[i].data.fd);
			}
		}		
	}
	close(epoll_fd);
	close(server_fd);
	
	return 0;
}