//
// Created by zr on 23-3-6.
//

#ifndef TANK_TROUBLE_SERVER_MESSAGES_H
#define TANK_TROUBLE_SERVER_MESSAGES_H
#include <cstdint>
#include <iostream>
#include <cstring>
#include <vector>
#include <tuple>
#include "net/Buffer.h"
#include "net/Endian.h"
#include <unordered_map>
#include <memory>

using namespace ev::net;
#define HeaderLen   3

namespace TankTrouble
{
    template<typename T>
    inline T hostToNetwork(T val) {return val;}
    template<>
    inline uint16_t hostToNetwork<uint16_t>(uint16_t val){return hostToNetwork16(val);}
    template<>
    inline uint32_t hostToNetwork<uint32_t>(uint32_t val){return hostToNetwork32(val);}
    template<>
    inline uint64_t hostToNetwork<uint64_t>(uint64_t val){return hostToNetwork64(val);}

    template<typename T>
    inline T networkToHost(T val) {return val;}
    template<>
    inline uint16_t networkToHost<uint16_t>(uint16_t val){return networkToHost16(val);}
    template<>
    inline uint32_t networkToHost<uint32_t>(uint32_t val){return networkToHost32(val);}
    template<>
    inline uint64_t networkToHost<uint64_t>(uint64_t val){return networkToHost64(val);}

    struct FixHeader
    {
        uint8_t messageType;
        uint16_t messageLen;

        FixHeader() = default;
        FixHeader(uint8_t mt, uint16_t ml): messageType(mt), messageLen(ml) {}

        void toByteArray(Buffer* buf) const
        {
            buf->appendInt8(static_cast<int8_t>(messageType));
            buf->appendInt16(static_cast<int16_t>(messageLen));
        }
    };

    inline FixHeader getHeader(const Buffer* buf)
    {
        assert(buf->readableBytes() >= HeaderLen);
        FixHeader header{};
        header.messageType = *static_cast<const uint8_t*>(static_cast<const void*>(buf->peek()));
        header.messageLen = *static_cast<const uint16_t*>(static_cast<const void*>(buf->peek() + 1));
        header.messageLen = networkToHost(header.messageLen);
        return header;
    }

    struct FieldBase
    {
        typedef bool _isField;
        explicit FieldBase(const std::string& name): name_(name) {};
        FieldBase() = default;
        virtual ~FieldBase() = default;
        [[nodiscard]] std::string name() const {return name_;}
        void setName(const std::string& name) {name_ = name;}
        virtual void fill(Buffer* buf) = 0;
        virtual void toByteArray(Buffer* buf) const = 0;
        [[nodiscard]] virtual size_t size() const = 0;

    protected:
        std::string name_;
    };

    template<typename T>
    struct Field : public FieldBase
    {
        using type = T;
        explicit Field(const std::string& name): FieldBase(name) {}
        Field() = default;
        void fill(Buffer* buf) override
        {
            memcpy(&data_, buf->peek(), sizeof(T));
            networkToHost(data_);
            buf->retrieve(sizeof(T));
        };

        T get() {return data_;}
        void set(T val) {data_ = val;}

        void toByteArray(Buffer* buf) const override
        {
            T netOrder = hostToNetwork(data_);
            buf->append((char*)&netOrder, sizeof(T));
        }

        [[nodiscard]] size_t size() const override { return sizeof(T);}

    private:
        T data_;
    };

    template<>
    struct Field<std::string> : public FieldBase
    {
        using type = std::string;
        explicit Field(const std::string& name): FieldBase(name) {}
        Field() = default;
        void fill(Buffer* buf) override
        {
            data_ = std::string(buf->peek());
            buf->retrieve(data_.size() +1);
        };

        std::string get() {return data_;}
        void set(const std::string& val) {data_ = val;}

        void toByteArray(Buffer* buf) const override
        {buf->append(data_.c_str(), data_.size() + 1);}

        [[nodiscard]] size_t size() const override {return data_.size() + 1;}

    private:
        std::string data_;
    };

    template<typename... Ts>
    struct StructField : public FieldBase
    {
        explicit StructField(const std::string& name, const std::vector<std::string>& fieldNames)
            : FieldBase(name),
            fields_(std::make_tuple(Field<Ts>()...)) { setFieldsName(fieldNames); }

        StructField()
        : FieldBase(""),
        fields_(std::make_tuple(Field<Ts>()...)) {}

        StructField(const StructField& s):
            fields_(s.fields_)
        {
            std::apply([this](auto&&... f){
                ((basesPtr_[f.name()] = &f, names_.push_back(f.name())), ...);
            }, fields_);
        }

        void setFieldsName(const std::vector<std::string>& fieldNames)
        {
            int index = 0;
            std::apply([&fieldNames, &index](auto&&... f){(f.setName(fieldNames[index++]), ...);}, fields_);
            std::apply([this](auto&&... f){
                    ((basesPtr_[f.name()] = &f, names_.push_back(f.name())), ...);
                }, fields_);
        }

        void fill(Buffer* buf) override
        {
            //std::apply([buf](auto&&... f){((f.fill(buf)), ...);}, fields_);
            for(const std::string& name: names_)
                basesPtr_[name]->fill(buf);
        }

        template<typename T>
        T get(const std::string& fieldName)
        {
            if(basesPtr_.find(fieldName) == basesPtr_.end())
                return T();
            return ((Field<T>*)(basesPtr_[fieldName]))->get();
        }

        template<typename T>
        void set(const std::string& fieldName, T val)
        {
            if(basesPtr_.find(fieldName) == basesPtr_.end())
                return;
            ((Field<T>*)(basesPtr_[fieldName]))->set(val);
        }

        void toByteArray(Buffer* buf) const override
        {
            //std::apply([buf](auto&&... f){((f.toByteArray(buf)), ...);}, fields_);
            for(const std::string& name: names_)
            {
                auto it = basesPtr_.find(name);
                assert(it != basesPtr_.end());
                it->second->toByteArray(buf);
            }
        }

        size_t size() const override
        {
            size_t sz = 0;
            for(const std::string& name: names_)
            {
                auto it = basesPtr_.find(name);
                assert(it != basesPtr_.end());
                sz += it->second->size();
            }
            return sz;
        }

    private:
        std::tuple<Field<Ts>...> fields_;
        std::unordered_map<std::string, FieldBase*> basesPtr_;
        std::vector<std::string> names_;
    };

    template<typename Elem>
    struct ArrayField : public FieldBase
    {
        explicit ArrayField(const std::string& name):
                FieldBase(name) {}

        void fill(Buffer* buf) override
        {
            uint8_t len = *(const uint8_t*)(buf->peek());
            buf->retrieve(1);
            fields_.resize(len);
            for(Elem& f: fields_)
                f.fill(buf);
        }

        size_t length() {return fields_.size();}

        Elem get(int index) {return fields_[index];}

        void addElement(const Elem& e) {fields_.push_back(e);}

        void toByteArray(Buffer* buf) const override
        {
            uint8_t len = fields_.size();
            buf->append(&len, 1);
            for(Elem& f: fields_)
                f.toByteArray(buf);
        }

        [[nodiscard]] size_t size() const override
        {
            size_t sz = 1;
            for(Elem& f: fields_)
                sz += f.size();
            return sz;
        }

    private:
        std::vector<Elem> fields_;
    };

    template<typename... Ts>
    struct ArrayField<StructField<Ts...>> : public FieldBase
    {
        explicit ArrayField(const std::string& name, std::vector<std::string> fieldNames):
                FieldBase(name),
                fieldNames_(std::move(fieldNames)) {}

        void fill(Buffer* buf) override
        {
            uint8_t len = *(const uint8_t*)(buf->peek());
            buf->retrieve(1);
            fields_.resize(len);
            for(auto& f: fields_)
            {
                f.setFieldsName(fieldNames_);
                f.fill(buf);
            }
        }

        size_t length() {return fields_.size();}

        StructField<Ts...> get(int index) {return fields_[index];}

        void addElement(const StructField<Ts...>& sf) {fields_.push_back(sf);}

        void toByteArray(Buffer* buf) const override
        {
            uint8_t len = fields_.size();
            buf->append(&len, 1);
            for(auto& f: fields_)
                f.toByteArray(buf);
        }

        [[nodiscard]] size_t size() const override
        {
            size_t sz = 1;
            for(auto& f: fields_)
                sz += f.size();
            return sz;
        }

    private:
        std::vector<StructField<Ts...>> fields_;
        std::vector<std::string> fieldNames_;
    };

    struct FieldTemplateBase
    {
        explicit FieldTemplateBase(const std::string& name): name_(name) {};
        FieldTemplateBase() = default;
        virtual ~FieldTemplateBase() = default;
        [[nodiscard]] std::string name() const {return name_;}
        void setName(const std::string& name) {name_ = name;}
        virtual FieldBase* getField() = 0;

    protected:
        std::string name_;
    };

    template<typename T>
    struct FieldTemplate : public FieldTemplateBase
    {
        explicit FieldTemplate(const std::string& name): FieldTemplateBase(name) {}
        FieldTemplate() = default;
        FieldBase* getField() override {return new Field<T>(name_); }
    };

    template<>
    struct FieldTemplate<std::string> : public FieldTemplateBase
    {
        explicit FieldTemplate(const std::string& name): FieldTemplateBase(name) {}
        FieldTemplate() = default;
        FieldBase* getField() override {return new Field<std::string>(name_); }
    };

    template<typename... Ts>
    struct StructFieldTemplate : public FieldTemplateBase
    {
        explicit StructFieldTemplate(const std::string& name, std::vector<std::string> fieldNames)
            :FieldTemplateBase(name),
            fieldNames_(std::move(fieldNames)){}
        StructFieldTemplate() = default;
        FieldBase* getField() override {return new StructField<Ts...>(name_, fieldNames_); }

    private:
        std::vector<std::string> fieldNames_;
    };

    template<typename Elem>
    struct ArrayFieldTemplate : public FieldTemplateBase
    {
        explicit ArrayFieldTemplate(const std::string& name):
                FieldTemplateBase(name) {}

        FieldBase* getField() override {return new ArrayField<Elem>(name_); }
    };

    template<typename... Ts>
    struct ArrayFieldTemplate<StructField<Ts...>> : public FieldTemplateBase
    {
        explicit ArrayFieldTemplate(const std::string& name, std::vector<std::string> fieldNames):
                FieldTemplateBase(name),
                fieldNames_(std::move(fieldNames)) {}

        FieldBase* getField() override {return new ArrayField<StructField<Ts...>>(name_, fieldNames_); }

    private:
        std::vector<std::string> fieldNames_;
    };

    struct Message
    {
        Message() = default;
        Message(const Message&) = delete;
        Message(Message&& m) noexcept
        {
            names_.swap(m.names_);
            fields_.swap(m.fields_);
        }

        void addField(FieldBase* fb)
        {
            names_.push_back(fb->name());
            fields_[fb->name()] = std::move(std::unique_ptr<FieldBase>(fb));
        }

        void fill(Buffer* buf)
        {
            for(const std::string& name: names_)
                fields_[name]->fill(buf);
        }

        template<typename T, typename T::_isField = true>
        T getField(const std::string& name)
        {
            if(fields_.find(name) == fields_.end())
                return T();
            return *(T*)(fields_[name].get());
        }

        template<typename F, typename T>
        void setField(const std::string& fieldName, const std::string& structFieldName, T val)
        {
            if(fields_.find(fieldName) == fields_.end())
                return;
            F* f = (F*)(fields_[fieldName].get());
                f->set(structFieldName, val);
        }

        template<typename F>
        void setField(const std::string& fieldName, typename F::type val)
        {
            if(fields_.find(fieldName) == fields_.end())
                return;
            F* f = (F*)(fields_[fieldName].get());
            f->set(val);
        }

        template<typename T>
        ArrayField<T> getArray(const std::string& name) { return *(ArrayField<T>*)(fields_[name].get()); }

        template<typename T>
        void addArrayElement(const std::string& name, const T& elem)
        {
            ((ArrayField<T>*)(fields_[name].get()))->addElement(elem);
        }

        void toByteArray(Buffer* buf) const {
            for(const std::string& name: names_)
            {
                auto it = fields_.find(name);
                assert(it != fields_.end());
                it->second->toByteArray(buf);
            }
        }

        size_t size() const {
            size_t sz = 0;
            for(const std::string& name: names_)
            {
                auto it = fields_.find(name);
                assert(it != fields_.end());
                sz += it->second->size();
            }
            return sz;
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<FieldBase>> fields_;
        std::vector<std::string> names_;
    };

    struct MessageTemplate
    {
        MessageTemplate() = default;
        MessageTemplate(std::initializer_list<FieldTemplateBase*> fts)
        {
            for(FieldTemplateBase* ft: fts)
                fieldTemplates_.push_back(std::move(std::unique_ptr<FieldTemplateBase>(ft)));
        }
        MessageTemplate(const MessageTemplate&) = delete;
        MessageTemplate(MessageTemplate&& mt)  noexcept {fieldTemplates_.swap(mt.fieldTemplates_);}
        MessageTemplate& operator=(MessageTemplate&& mt)
        {
            fieldTemplates_.swap(mt.fieldTemplates_);
            return *this;
        }

        Message getMessage()
        {
            Message message;
            for(const auto& ft: fieldTemplates_)
                message.addField(ft->getField());
            return message;
        }

    private:
        std::vector<std::unique_ptr<FieldTemplateBase>> fieldTemplates_;
    };
}

#endif //TANK_TROUBLE_SERVER_MESSAGES_H
