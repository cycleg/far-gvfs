#pragma once

#include <chrono>
#include <condition_variable>
#include <list>
#include <mutex>

/// 
/// Шаблонный класс очереди задач.

/// @author golovin
///
template <class Job> class JobUnitQueue
{
  public:
    ///
    /// Поместить очередное задание в очередь.
    ///
    /// @param [in] job Помещаемое задание
    ///
    inline void put(const Job& job)
    {
      std::unique_lock<std::mutex> lock(mutex);
      jobs.push_back(job);
      (void)lock;
    }

    ///
    /// Извлечь очередное задание из очереди.
    ///
    /// @return Задание.
    ///
    /// Если в очереди заданий нет, возвращает "пустое" задание,
    /// созданное конструктором по умолчанию класса Job.
    /// 
    inline Job get()
    {
      std::unique_lock<std::mutex> lock(mutex);

      Job job;
      if (!jobs.empty())
      {
        job = jobs.front();
        jobs.pop_front();
      }
      (void)lock;
      return job;
    }

    ///
    /// Ожидать появления задания в очереди с таймаутом.
    ///
    /// @param [in] abs_time Таймаут в миллисекундах.
    /// @return Возвращает true, если время ожидания истекло.
    ///
    inline bool wait_for(std::chrono::milliseconds const& abs_time)
    {
      std::unique_lock<std::mutex> lock(mutex);
      if (!jobs.empty()) return false;
      return (cond.wait_for(lock, abs_time) == std::cv_status::timeout);
    }

    ///
    /// Отправить уведомление о появлении задания в очереди.
    ///
    /// Уведомление следует делать явно. При добавлении задания в очередь
    /// методом put() этот метод не вызывается.
    ///
    inline void notify_one()
    {
      cond.notify_one();
    }

  private:
    std::list<Job> jobs; ///< Очередь заданий.
    std::mutex mutex; ///< Мутекс списка заданий и переменной состояния.
    std::condition_variable cond; ///< Переменная состояния для сигнала о
                                  ///< появлении нового задания.
};
