#ifndef _ARRAY_SEQUENCE_H_
#define _ARRAY_SEQUENCE_H_

#include "exceptions.h"
#include "dynamic_array.h"
#include "sequence.h"

template <typename T>
class ArraySequence : public Sequence<T> {
private:
    DynamicArray<T> array;

protected:
    virtual Sequence<T>* CreateSequence(const DynamicArray<T> &arr) const = 0;
    virtual Sequence<T>* Instance() = 0;

    virtual Sequence<T>* AppendImplict(const T &item) override;
    virtual Sequence<T>* PrependImplict(const T &item) override;
    virtual Sequence<T>* InsertAtImplict(const T &item, int index) override;
    virtual Sequence<T>* RemoveAtImplict(int index) override;
    virtual Sequence<T>* ConcatImplict(const Sequence<T>* other) override;

public:
    ArraySequence();
    ArraySequence(const T* items, int count);
    explicit ArraySequence(int size);
    ArraySequence(const DynamicArray<T> &arr);
    ArraySequence(const ArraySequence<T> &other);
    ~ArraySequence() override;

    ArraySequence<T>& operator=(const ArraySequence<T>& other);

    const T& GetFirst() const override;
    const T& GetLast() const override;
    const T& Get(int index) const override;
    int GetLength() const override;
    Sequence<T>* GetSubsequence(int startIndex, int endIndex) const override;
    Sequence<T>* Map(T (*func)(const T&)) const override;
    Sequence<T>* Where(bool (*pred)(const T&)) const override;
    void Reduce(T (*func)(const T&, const T&), const T &init, T *result) const override;

    Option<T> TryGetFirst(bool (*pred)(const T&) = nullptr) const;
    Option<T> TryGetLast(bool (*pred)(const T&) = nullptr) const;

    class ArrayEnumerator : public IEnumerator<T> {
    private:
        const ArraySequence<T>& arr_seq;
        int currentIndex;
    public:
        ArrayEnumerator(const ArraySequence<T>& s);
        bool HasNext() override;
        const T& GetCurrent() const override;
        void Reset() override;
    };

    IEnumerator<T>* GetEnumerator() const override;
};

#include "array_sequence.tpp"

#endif // _ARRAY_SEQUENCE_H_