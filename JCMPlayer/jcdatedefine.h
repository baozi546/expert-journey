#ifndef JCDATEDEFINE_H
#define JCDATEDEFINE_H

#include <QList>
#include <mutex>

template<class T>
class JCDateDefine : public QList<T>
{
public:
    JCDateDefine(){};
    ~JCDateDefine(){};

    void enqueue (const T a)
    {
        std::lock_guard<std::mutex>lk(m_mutex);
        QList<T>::append(a);
    }

    T dequeue(){
        std::lock_guard<std::mutex>lk(m_mutex);
        T t=nullptr;
        if(!QList<T>::isEmpty())
        {
            t=QList<T>::takeFirst();
        }
        return t;
    }
    void Clear(){
         std::lock_guard<std::mutex>lk(m_mutex);
         while(!QList<T>::isEmpty()) QList<T>::takeFirst();
    }
    bool isEmpty(){
        std::lock_guard<std::mutex>lk(m_mutex);
        bool t=QList<T>::isEmpty();
        return t;
    }
    int size(){
        std::lock_guard<std::mutex>lk(m_mutex);
        return QList<T>::size();
    }
private:
    std::mutex m_mutex;
};

#endif // JCDATEDEFINE_H
