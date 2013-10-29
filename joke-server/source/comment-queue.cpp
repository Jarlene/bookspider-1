#include "comment-queue.h"
#include "sys/sync.h"
#include "time64.h"
#include "thread-pool.h"
#include <list>

enum { TASK_IDLE=0, TASK_DOING, TASK_DONE };
enum { PRIORITY_IDLE=0, PRIORITY_LOWEST, PRIORITY_NORMAL, PRIORITY_CRITICAL };

typedef struct
{
	comment_callback callback;
	void* param;
} comment_action_t;

std::list<comment_action_t> comment_actions_t;

typedef struct
{
	unsigned int id;
	comment_actions_t actions;

	time64_t time;
	int status;
	int priority;
} comment_task_t;

typedef std::list<comment_task_t> comment_tasks_t;

struct
{
	ThreadLocker locker;
	thread_t thread_schedule;
	semaphore_t sema_worker;
	semaphore_t sema_request;
	comment_tasks_t tasks;
	comment_tasks_t tasks_recycle;
} g_ctx;

static void JsonEncode(const Comments& comments, std::string& comment)
{
	jsonarray jarr;
	Comments::const_iterator it;
	for(it = comments.begin(); it != comments.end(); ++it)
	{
		const Comment& comment = *it;

		jsonobject json;
		json.add("icon", comment.icon);
		json.add("user", comment.user);
		json.add("comment", comment.content);
		jarr.add(json);
	}

	comment = jarr.json();
}

static int GetComment(unsigned int id, std::string& comment)
{
	IJokeSpider* spider = NULL;
	if(id / JOKE_SITE_ID == 1)
		spider = new CQiuShiBaiKe();
	else if(id / JOKE_SITE_ID == 2)
		spider = new CBaiSiBuDeJie(1);
	else
		return ERROR_PARAM;

	Comments comments;
	int r = spider->GetComment(comments, id % JOKE_SITE_ID);
	if(r < 0)
		return r;

	JsonEncode(comments, comment);
	return r;
}

static void comment_queue_action(const comment_task_t* task, 
								 int code, 
								 const std::string& comment)
{
	comment_actions_t::iterator it;
	AutoThreadLocker locker(g_ctx.locker);
	for(it=task->actions.begin(); it!=task->actions.end(); ++it)
	{
		const comment_action_t& action = *it;
		action.callback(action.param, task->id, code, comment);
	}
}

static void comment_queue_worker(void* param)
{
	comment_task_t* task;
	task = (comment_task_t*)param;

	std::string comment;
	int r = GetComment(task->id, comment);
	comment_queue_action(task, r, comment);

	semaphore_post(&g_ctx.sema_worker); // add worker
}

extern thread_pool_t g_pool;
static int comment_queue_scheduler(void* param)
{
	int r;
	comment_task_t* task;
	while(1)
	{
		r = semaphore_wait(&g_ctx.sema_request);
		r = semaphore_timewait(&g_ctx.sema_worker, 1000);
		if(0 == r)
		{
			r = 1;
			comment_tasks_t::iterator it;
			AutoThreadLocker locker(g_ctx.locker);
			for(it=g_ctx.tasks.begin(); it!=g_ctx.tasks.end(); ++it)
			{
				comment_task_t& task = *it;
				if(TASK_IDLE == task.status)
				{
					r = thread_pool_push(g_pool, comment_queue_worker, task);
					break;
				}
			}

			// idle
			if(1 == r)
				semaphore_post(&g_ctx.sema_worker);
		}
		else
		{
			// timeout
			semaphore_post(&g_ctx.sema_worker);

			time64_t lt = time64_now();
			comment_tasks_t::iterator it, it0;
			AutoThreadLocker locker(g_ctx.locker);
			it = g_ctx.tasks.begin();
			while(it != g_ctx.tasks.end())
			{
				it0 = it++;
				comment_task_t& task = *it0;
				if(task.time + 5000 > lt)
				{
					comment_queue_action(task, ETIMEDOUT, "");
					g_ctx.tasks.erase(it0);
				}
			}
		}
	}

	return 0;
}

int comment_queue_create()
{
	int r;

	r = locker_create(&g_ctx.locker);
	assert(0 == r);

	// create 20-work thread
	r = semaphore_create(&g_ctx.sema_worker, NULL, 20);
	assert(0 == r);

	// create request semaphore
	r = semaphore_create(&g_ctx.sema_request, NULL, 0);
	assert(0 == r);

	// create schedule thread
	return thread_create(&g_ctx.thread_schedule, comment_queue_scheduler, NULL);
}

int comment_queue_destroy()
{
	thread_destroy(g_ctx.thread_schedule);
	semaphore_destroy(&g_ctx.sema_request);
	semaphore_destroy(&g_ctx.sema_worker);
	return 0;
}

static comment_task_t* comment_queue_alloc()
{
	comment_task_t* task;
	AutoThreadLocker locker(g_ctx.locker);
	if(g_ctx.tasks_recycle.size())
	{
		task = g_ctx.tasks_recycle.front();
		g_ctx.tasks_recycle.pop_front();
	}
	else
	{
		task = new comment_task_t;
	}
	return task;
}

static void comment_queue_recycle(comment_task_t* task)
{
	//AutoThreadLocker locker(g_ctx.locker);
	g_ctx.tasks_recycle.push_back(task);
}

static void comment_queue_insert(comment_task_t* task)
{
	comment_tasks_t::iterator it;
	AutoThreadLocker locker(g_ctx.locker);
	for(it=g_ctx.tasks.begin(); it!=g_ctx.tasks.end(); ++it)
	{
		comment_task_t& task = *it;
		if(task.id == id)
		{
			task.request.push_back(item);
			comment_queue_recycle(task);
			return 0;
		}
	}

	g_ctx.tasks.push_back(task);
}

int comment_queue_post(unsigned int id, comment_callback cb, void* param)
{
	comment_task_t* task;
	task = comment_queue_alloc();
	if(!task)
		return -ENOMEM;

	comment_action_t action;
	action.callback = cb;
	action.param = param;

	task->id = id;
	task->time = time64_now();
	task->status = TASK_IDLE;
	task->priority = 0;
	task->actions.push_back(action);
	comment_queue_insert(task);

	return semaphore_post(&g_ctx.sema_request);
}
