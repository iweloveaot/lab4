// Generator.h
#ifndef _GENERATOR_H_
#define _GENERATOR_H_

#include "base/exceptions.h"


template<typename T>
class IGenerator {
public:
    virtual ~IGenerator() {}
    virtual T Generate(int index) const = 0;
    virtual IGenerator<T>* Clone() const = 0;
};


template<typename T>
class GeneratorRule : public IGenerator<T> {
private:
    T (*rule_func)(const T&);
public:
    GeneratorRule(T (*func)(const T&)) : rule_func(func) {}
    
    T Generate(int index) const override {
        return rule_func(index);
    }
    
    IGenerator<T>* Clone() const override {
        return new GeneratorRule<T>(rule_func);
    }
};


template<typename T>
class MapGenerator : public IGenerator<T> {
private:
    IGenerator<T>* source_gen;
    T (*map_func)(const T&);
public:
    MapGenerator(const IGenerator<T>* src, T (*func)(const T&)) 
        : source_gen(src->Clone()), map_func(func) {}
    
    MapGenerator(const MapGenerator& other)
        : source_gen(other.source_gen->Clone()), map_func(other.map_func) {}
    
    ~MapGenerator() { delete source_gen; }
    
    T Generate(int index) const override {
        return map_func(source_gen->Generate(index));
    }
    
    IGenerator<T>* Clone() const override {
        return new MapGenerator<T>(*this);
    }
};


template<typename T>
class WhereGenerator : public IGenerator<T> {
private:
    IGenerator<T>* source_gen;
    bool (*predicate)(const T&);
    mutable MutableArraySequence<int>* valid_indices; 
    mutable int last_computed_index;
    
    void ComputeUntil(int target_index) const {
        while (valid_indices->GetLength() <= target_index) {
            int src_index = last_computed_index + 1;
            while (true) {
                T value = source_gen->Generate(src_index);
                if (predicate(value)) {
                    valid_indices->Append(src_index);
                    break;
                }
                src_index++;
            }
            last_computed_index = src_index;
        }
    }
    
public:
    WhereGenerator(const IGenerator<T>* src, bool (*pred)(const T&))
        : source_gen(src->Clone()), predicate(pred), 
          valid_indices(new MutableArraySequence<int>()), last_computed_index(-1) {}
    
    WhereGenerator(const WhereGenerator& other)
        : source_gen(other.source_gen->Clone()), predicate(other.predicate),
          valid_indices(new MutableArraySequence<int>()), last_computed_index(-1) 
    {
        // копируем уже вычисленные индексы
        for (int i = 0; i < other.valid_indices->GetLength(); ++i)
            valid_indices->Append(other.valid_indices->Get(i));
        last_computed_index = other.last_computed_index;
    }
    
    ~WhereGenerator() { 
        delete source_gen; 
        delete valid_indices;
    }
    
    T Generate(int index) const override {
        ComputeUntil(index);
        int src_index = valid_indices->Get(index);
        return source_gen->Generate(src_index);
    }
    
    IGenerator<T>* Clone() const override {
        return new WhereGenerator<T>(*this);
    }
};


template<typename T>
class AppendGenerator : public IGenerator<T> {
private:
    IGenerator<T>* source_gen;
    T appended_value;
    int source_length; 
    
public:
    AppendGenerator(const IGenerator<T>* src, T value, int src_len)
        : source_gen(src->Clone()), appended_value(value), source_length(src_len) {}
    
    AppendGenerator(const AppendGenerator& other)
        : source_gen(other.source_gen->Clone()), appended_value(other.appended_value), 
          source_length(other.source_length) {}
    
    ~AppendGenerator() { delete source_gen; }
    
    T Generate(int index) const override {
        if (index < source_length)
            return source_gen->Generate(index);
        else if (index == source_length)
            return appended_value;
        else
            throw IndexOutOfRangeException("Index out of range in AppendGenerator");
    }
    
    IGenerator<T>* Clone() const override {
        return new AppendGenerator<T>(*this);
    }
};


template<typename T>
class PrependGenerator : public IGenerator<T> {
private:
    IGenerator<T>* source_gen;
    T prepended_value;
    
public:
    PrependGenerator(const IGenerator<T>* src, T value)
        : source_gen(src->Clone()), prepended_value(value) {}
    
    PrependGenerator(const PrependGenerator& other)
        : source_gen(other.source_gen->Clone()), prepended_value(other.prepended_value) {}
    
    ~PrependGenerator() { delete source_gen; }
    
    T Generate(int index) const override {
        if (index == 0)
            return prepended_value;
        else
            return source_gen->Generate(index - 1);
    }
    
    IGenerator<T>* Clone() const override {
        return new PrependGenerator<T>(*this);
    }
};


template<typename T>
class InsertAtGenerator : public IGenerator<T> {
private:
    IGenerator<T>* source_gen;
    T inserted_value;
    int insert_index;
    
public:
    InsertAtGenerator(const IGenerator<T>* src, T value, int idx)
        : source_gen(src->Clone()), inserted_value(value), insert_index(idx) {}
    
    InsertAtGenerator(const InsertAtGenerator& other)
        : source_gen(other.source_gen->Clone()), inserted_value(other.inserted_value),
          insert_index(other.insert_index) {}
    
    ~InsertAtGenerator() { delete source_gen; }
    
    T Generate(int index) const override {
        if (index < insert_index)
            return source_gen->Generate(index);
        else if (index == insert_index)
            return inserted_value;
        else
            return source_gen->Generate(index - 1);
    }
    
    IGenerator<T>* Clone() const override {
        return new InsertAtGenerator<T>(*this);
    }
};


template<typename T>
class RemoveAtGenerator : public IGenerator<T> {
private:
    IGenerator<T>* source_gen;
    int remove_index;
    
public:
    RemoveAtGenerator(const IGenerator<T>* src, int idx)
        : source_gen(src->Clone()), remove_index(idx) {}
    
    RemoveAtGenerator(const RemoveAtGenerator& other)
        : source_gen(other.source_gen->Clone()), remove_index(other.remove_index) {}
    
    ~RemoveAtGenerator() { delete source_gen; }
    
    T Generate(int index) const override {
        if (index < remove_index)
            return source_gen->Generate(index);
        else
            return source_gen->Generate(index + 1);
    }
    
    IGenerator<T>* Clone() const override {
        return new RemoveAtGenerator<T>(*this);
    }
};


template<typename T>
class ConcatGenerator : public IGenerator<T> {
private:
    IGenerator<T>* source_gen;
    const Sequence<T>* other_seq; 
    int source_length;
    
public:
    ConcatGenerator(const IGenerator<T>* src, const Sequence<T>* other, int src_len)
        : source_gen(src->Clone()), other_seq(other), source_length(src_len) {}
    
    ConcatGenerator(const ConcatGenerator& other)
        : source_gen(other.source_gen->Clone()), 
          other_seq(other.other_seq), 
          source_length(other.source_length) {}
    
    ~ConcatGenerator() { 
        delete source_gen;
    }
    
    T Generate(int index) const override {
        if (index < source_length)
            return source_gen->Generate(index);
        else {
            int other_index = index - source_length;
            return other_seq->Get(other_index);
        }
    }
    
    IGenerator<T>* Clone() const override {
        return new ConcatGenerator<T>(*this);
    }
};

#endif // _GENERATOR_H_