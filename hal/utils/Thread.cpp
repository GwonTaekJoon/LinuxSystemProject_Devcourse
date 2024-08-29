/*
 * thread.cpp, thread class
 *
 * Copyright (c) 2009-2010 Wind River Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <pthread.h>
#include <Thread.h>

Thread::Thread()
{
    r = NULL;
    created = false;
    id = 0;

    pthread_mutex_init(&lock, NULL);
}

Thread::Thread(RunnableInterface *r)
{
    this->r = r;
    created = false;
    id = 0;

    pthread_mutex_init(&lock, NULL);
}

Thread::~Thread()
{
    Join();

    pthread_mutex_destroy(&lock);
}

int Thread::Start(void)
{
    int ret = 0;
    struct sched_param param;
    pthread_attr_t thread_attrs;

    pthread_mutex_lock(&lock);
    if (!created) {
        pthread_attr_init(&thread_attrs);
        ret = pthread_create(&id, &thread_attrs, Instance, this);
        if (!ret)
            created = true;
    }
    pthread_mutex_unlock(&lock);

    return ret;
}

int Thread::Join(void)
{
    int ret = 0;

    pthread_mutex_lock(&lock);
    if (created) {
        ret = pthread_join(id, NULL);
        created = false;
    }
    pthread_mutex_unlock(&lock);

    return ret;
}

void *Thread::Instance(void *p)
{
    Thread *t = static_cast<Thread *>(p);

    t->threadLoop();

    return NULL;
}

void Thread::threadLoop(void)
{
    if (r)
        r->threadLoop();
    else
        return;
}
