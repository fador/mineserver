/*
   Copyright (c) 2016, The Mineserver Project
   All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the The Mineserver Project nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <chrono>
#include <exception>

enum ThreadType { THREAD_MAPGEN, THREAD_LIGHTGEN, THREAD_VALIDATEUSER };

class User;

struct ThreadTask {

  ThreadTask() {};
  ThreadTask(ThreadType _type, User* _user) : type(_type), user(_user) {};
  ThreadType type;
  User *user;

  uint32_t taskID;

  struct {
    std::string accessToken;
  } taskValidate;
};

class ThreadPool{
public:
    ThreadPool() : running(true),taskID(0) { }

    // Only increase, cannot decrease yet
    // ToDo: decrease thread count
    bool setThreadCount(uint32_t val) {
      while (threads.size() < val) {
        // Create a thread with a lambda function looping execute();
        threads.push_back(std::thread(
              [&] {
                    while (running) {
                      try {
                        execute(); 
                      } catch (std::exception e) {}
                    }
                  }));
      }
      return true;
    }

    // Add a new task to the pool
    void newTask(ThreadTask* task){      
        std::unique_lock<std::mutex> lock(inputMutex);
        task->taskID = taskID++;
        tasks.push(task);
        // Use condition to notify a thread to handle this task
        taskCondition.notify_one();
    }

    // Main function when running a thread
    void execute(){

        std::unique_lock<std::mutex> inputLock(inputMutex);

        // Wait if there's no tasks in the queue
        taskCondition.wait(inputLock, [&](){ return !tasks.empty() || !running; });

        if (!running){
            throw std::exception();
        }

        ThreadTask* task = tasks.front();
        tasks.pop();
        inputLock.unlock();

        // Handle task
        if (task->type == THREAD_VALIDATEUSER)
        {
         taskValidateUser(task);
        }
        else
        {
          // Simulate random delay
          std::this_thread::sleep_for(std::chrono::milliseconds{ rand()%500 });
        }

        std::unique_lock<std::mutex> outputLock(outputMutex);
        std::cout << "Thread " << std::this_thread::get_id() << " handled task " << task->taskID << std::endl;
        outputLock.unlock();
        delete task;
    }

    // 
    void shutdown(){
        running = false;
        taskCondition.notify_all();
    }

private:
    std::queue<ThreadTask*> tasks;
    std::condition_variable taskCondition;
    std::mutex inputMutex;
    std::mutex outputMutex;
    std::vector<std::thread> threads;
    bool running;
    uint32_t taskID;

    // Tasks
    void taskValidateUser(ThreadTask* task);
};
