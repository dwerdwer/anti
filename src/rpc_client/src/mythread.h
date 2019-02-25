
#ifndef _MYTHREAD_H_
#define _MYTHREAD_H_







enum Thread_State
{
	Thread_Idle	= 0,	//¿ÕÏÐ
	Thread_Busy	= 1,	//Ã¦Âµ
	Thread_Hup	= 2,	//¹ÒÆð
	Thread_Exit	= 3	//ÖÕÖ¹
};



//end


typedef struct
{
	unsigned int errorno;
	char errormsg[512];
}thread_error_t;



class CMyThread
{
public:
	CMyThread();
	virtual ~CMyThread();
	//int CreateThread(void(*proc)(void *), void *pargs);
	bool    	Start();        	//Æô¶¯
	bool    	Terminate();    	//ÖÕÖ¹

	void EndThread();
	unsigned int GetCurrentThreadId();
	unsigned int GetThreadID() const {return m_ThreadID;}
	void	SetState(Thread_State state) { m_State = state; }
	Thread_State GetState(){return m_State;}
	void Sleep(unsigned int milliseconds);
	void DiscardTimeSlice();

	virtual	void Run(void) = 0;


private:

	unsigned long 	m_ThreadID;
	Thread_State	m_State;		//Ïß³Ì×´Ì¬
};





#if 0
class CThreadError
{
	typedef struct
	{
		thread_error_t threaderror;
		unsigned int threadid;
		void * next;
	} internal_thread_error_t;

public:
	CThreadError();
	~CThreadError();
	void operator=(int errorno);
	void operator=(const char * msg);
	void operator=(thread_error_t & st);
	unsigned int GetLastErrorNo();
	const char *GetLastErrorMsg();
	const thread_error_t *GetLastErrorStruct();
private:
	internal_thread_error_t* m_pStart;
	internal_thread_error_t* allocMemory(unsigned int tid);
	internal_thread_error_t * search(unsigned int tid);
};
#endif




#endif
