/*
********************************************************************
(c) MSU Video Group, http://compression.ru/video/
This source code is property of MSU Graphics and Media Lab

This code may be distributed under LGPL
(see http://www.gnu.org/licenses/lgpl.html for more details).

E-mail: video-measure@compression.ru
Author: Georgy Osipov
********************************************************************
*/  

/**
*  \file json.h
*  \brief Header-only library for JSON operations:
*     parsing JSON
*     serializing JSON
*     operations with JSON-data
*/

#pragma once

#include <vector>
#include <map>
#include <set>
#include <list>
#include <memory>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <assert.h>
#include <cmath>
#include <locale>
#include <iomanip>
#include <codecvt>
#include <stdint.h>
#include <initializer_list>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#define YUVsoft_THROW(x)

namespace YUVsoft {
    class JSON;
    namespace JSONElements {
		template<class K, class V>
		class OrderedMap {
			using Order = std::list<std::pair<K, V>>;
			using Map = std::map<K, typename Order::iterator>;
		public:
			using iterator = typename Order::iterator;
			using const_iterator = typename Order::const_iterator;

			iterator find(const K& k) {
				auto iter = map.find(k);
				if (iter == map.end()) return order.end();
				return iter->second;
			}

			const_iterator find(const K& k) const {
				auto iter = map.find(k);
				if (iter == map.end()) return order.end();
				return iter->second;
			}

			iterator begin() {
				return order.begin();
			}

			const_iterator begin() const {
				return order.begin();
			}

			iterator end() {
				return order.end();
			}

			const_iterator end() const {
				return order.end();
			}

			V& operator [] (const K& k) {
				auto iter = map.find(k);
				if (iter == map.end())
					iter = _insert(k, V());

				return iter->second->second;
			}

			typename Order::size_type size() const {
				return order.size();
			}

			void insert(const std::pair<K, V>& p) {
				_insert(p.first, p.second);
			}

			bool empty() const {
				return order.empty();
			}

		private:
			typename Map::iterator _insert(const K& k, const V& v) {
				order.push_back({ k, v });
				return map.insert(map.end(), { k, --order.end() });
			}

		private:
			Order order;
			Map map;
		};

		class ElemTypeId{};
		
        class Elem {
		public:
		protected:
            friend class YUVsoft::JSON;
			using Map = OrderedMap<std::string, JSON>;
			using Array = std::vector<JSON>;
			
            virtual bool isArray () const { return false; }
            virtual bool isObject() const { return false; }
            virtual bool hasIndex( int i ) const { return false; }
            virtual bool hasIndex( const std::string& i ) const { return false; }
            virtual JSON* getByIndex( int i ) { return NULL; }
            virtual JSON* getByIndex( const std::string& i ) { return NULL; }
            virtual const JSON* getByIndex( int i ) const { return NULL; }
            virtual const JSON* getByIndex( const std::string& i ) const  { return NULL; }
            virtual void insert(const std::string& key, const JSON&) {}
            virtual void append(const JSON&) {}
			virtual size_t length() const { return size_t(-1); }
			virtual ElemTypeId* elemTypeId() const = 0;
			virtual int compare(const Elem* r) const = 0;
			virtual std::shared_ptr<Elem> convert(const JSONElements::ElemTypeId* tag) const { return{}; };

			virtual Map::iterator begin() { return Map::iterator(); }
			virtual Map::iterator end()   { return Map::iterator(); }
			virtual Map::const_iterator begin() const { return Map::const_iterator(); }
			virtual Map::const_iterator end()   const { return Map::const_iterator(); }
			virtual Array::iterator arrBegin() { return Array::iterator(); }
			virtual Array::iterator arrEnd()   { return Array::iterator(); }
			virtual Array::const_iterator arrBegin() const { return Array::const_iterator(); }
			virtual Array::const_iterator arrEnd()   const { return Array::const_iterator(); }

		private:
			

			//return count of symbols, read from buff, forming controll sequence. If > 0 must fill output
			static int utf8ControlLength(const char* buff, int buffLength, uint16_t* output ) {
				if(buffLength<=0) return 0;
				unsigned char c = buff[0];
				if((c<=0x1f) || c==0x7f) {
					*output = c;
					return 1;
				}
				if(c!=0xc2 || buffLength <=1) return 0;
				unsigned char d = buff[1];

				if(d>=0x80 && d<=0x9f) {
					*output = d;
					return 2;
				}

				return 0;
			}

			static char hexDigit(int digit) {
				if(digit>=10) return char('a' + (digit-10));
				return char('0' + digit);
			}
        public:
            virtual ~Elem() {}
            virtual void writeToStream(std::ostream&, int prettyDepth, int offset) const =0;
            virtual Elem* clone() const = 0;

            static std::string escaped(const std::string& input) {
                std::ostringstream ss;
				const char* str = input.c_str();
				int sz = int(input.size());
				uint16_t unicodeOutput;

                for (int i=0;i<sz;++i) {
					unsigned char ch = str[i];
                    switch (ch) {
                        case '\\': ss << "\\\\"; break;
                        case '"': ss << "\\\""; break;
                        //case '/': ss << "\\/"; break;
                        case '\b': ss << "\\b"; break;
                        case '\f': ss << "\\f"; break;
                        case '\n': ss << "\\n"; break;
                        case '\r': ss << "\\r"; break;
                        case '\t': ss << "\\t"; break;
                        default: 
							int controlLen = utf8ControlLength(str+i, sz - i, &unicodeOutput);
							if(controlLen==0) {ss << ch; break;}
							ss<<"\\u"<<hexDigit((unicodeOutput>>12)%16)<<hexDigit((unicodeOutput>> 8)%16)
								     <<hexDigit((unicodeOutput>> 4)%16)<<hexDigit((unicodeOutput>> 0)%16);
                    }
                }
                return ss.str();
            }
        };

        class String : public Elem {
            std::string value;
        public:
            String() {}
			String(const std::string& val) : value(val) {}

			static ElemTypeId* getElemTypeId() {
				static ElemTypeId res;
				return &res;
			}
			virtual ElemTypeId* elemTypeId() const override {
				return getElemTypeId();
			}
			virtual int compare(const Elem* r) const override {
				const String* pr = dynamic_cast<const String*>(r);
				assert(pr);

				if( value > pr->value ) return  1;
				if( value < pr->value ) return -1;
				return 0;
			}

			void writeToStream(std::ostream& stream, int prettyDepth, int offset) const override {
				stream<<"\""<<escaped(value)<<"\"";
			}

			virtual std::shared_ptr<Elem> convert(const JSONElements::ElemTypeId* tag) const override;

            Elem* clone() const {
                return new String(value);
            }

            typedef std::string NativeType;
            typedef const std::string& NativeTypeRet;

            const std::string& getVal() const { return value; }
        };

        class Boolean : public Elem {
            bool value;
        public:
            Boolean() {}
            Boolean(bool val) : value(val) {}

			static ElemTypeId* getElemTypeId() {
				static ElemTypeId res;
				return &res;
			}
			virtual ElemTypeId* elemTypeId() const override {
				return getElemTypeId();
			}
			virtual int compare(const Elem* r) const override {
				const Boolean* pr = dynamic_cast<const Boolean*>(r);
				assert(pr);

				if( value > pr->value ) return  1;
				if( value < pr->value ) return -1;
				return 0;
			}

			void writeToStream(std::ostream& stream, int prettyDepth, int offset) const override {
				stream<<(value ? "true" : "false");
            }

			virtual std::shared_ptr<Elem> convert(const JSONElements::ElemTypeId* tag) const override;

            Elem* clone() const {
                return new Boolean(value);
            }


            typedef bool NativeType;
            typedef bool NativeTypeRet;
            
            bool getVal() const { return value; }
        };

        class Integer : public Elem {
            int64_t value;
        public:
            Integer() {}
			Integer(int64_t val) : value(val) {}

			void writeToStream(std::ostream& stream, int prettyDepth, int offset) const override {
				stream<<value;
			}
			
			static ElemTypeId* getElemTypeId() {
				static ElemTypeId res;
				return &res;
			}
			virtual ElemTypeId* elemTypeId() const override {
				return getElemTypeId();
			}
			virtual int compare(const Elem* r) const override {
				const Integer* pr = dynamic_cast<const Integer*>(r);
				assert(pr);

				if( value > pr->value ) return  1;
				if( value < pr->value ) return -1;
				return 0;
			}

			virtual std::shared_ptr<Elem> convert(const JSONElements::ElemTypeId* tag) const override;

			Elem* clone() const {
                return new Integer(value);
            }
            
            typedef int64_t NativeType;
            typedef int64_t NativeTypeRet;

            int64_t getVal() const { return value; }
        };

        class Float : public Elem {
            double value;
        public:
            Float() {}
			Float(double val) : value(val) {}
			
			static ElemTypeId* getElemTypeId() {
				static ElemTypeId res;
				return &res;
			}
			virtual ElemTypeId* elemTypeId() const override {
				return getElemTypeId();
			}
			virtual int compare(const Elem* r) const override {
				const Float* pr = dynamic_cast<const Float*>(r);
				assert(pr);

				if( value > pr->value ) return  1;
				if( value < pr->value ) return -1;
				return 0;
			}

			virtual std::shared_ptr<Elem> convert(const JSONElements::ElemTypeId* tag) const override;

			void writeToStream(std::ostream& stream, int prettyDepth, int offset) const override {
				stream << std::setprecision(std::numeric_limits<double>::digits10+1) << std::fixed << std::showpoint;
				if (std::isnormal(value))
					stream<<value;
				else
					stream << 0.;
			}

            Elem* clone() const {
                return new Float(value);
            }

            typedef double NativeType;
            typedef double NativeTypeRet;

            double getVal() const { return value; }
        };

        class Null : public Elem {
        public:
			Null() {}

			void writeToStream(std::ostream& stream, int prettyDepth, int offset) const override {
				stream<<"null";
			}
			
			static ElemTypeId* getElemTypeId() {
				static ElemTypeId res;
				return &res;
			}
			virtual ElemTypeId* elemTypeId() const override {
				return getElemTypeId();
			}
			virtual int compare(const Elem* r) const override {
				assert(dynamic_cast<const Null*>(r));

				return 0;
			}

            Null* clone() const {
                return new Null;
            }
        };
    } // namespace JSONElements

	inline std::string utf16_to_utf8(const std::wstring& text_utf16) {
		static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes(text_utf16.c_str());
	}

	inline std::wstring utf8_to_utf16(const std::string& text_utf8) {
		static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return converter.from_bytes(text_utf8.c_str(), text_utf8.c_str() + text_utf8.size());
	}

    class JSON {
        JSONElements::Elem* elem;

        template <class T>
        bool isT () const {
            return dynamic_cast<T*>(elem) != NULL;
        }

        template <class T>
        typename T::NativeTypeRet asT() const {
            T* t = dynamic_cast<T*>(elem);
            if(!t ) throw InvalidType();

            return t->getVal();
        }

        template <class T>
        typename T::NativeTypeRet asT(const typename T::NativeType& byDef) const {
            T* t = dynamic_cast<T*>(elem);
            if(!t ) return byDef;

            return t->getVal();
        }
    public:
        JSON() : elem(new JSONElements::Null) {}
        ~JSON() { delete elem; }

		static JSON value (JSON val) { return val; }

        static JSON value (bool val) { return JSON(JSONElements::Boolean(val)); }

        static JSON value (char val) { return JSON(JSONElements::Integer(val)); }
        static JSON value (unsigned char val) { return JSON(JSONElements::Integer(val)); }
        static JSON value (short val) { return JSON(JSONElements::Integer(val)); }
        static JSON value (unsigned short val) { return JSON(JSONElements::Integer(val)); }
        static JSON value (int val) { return JSON(JSONElements::Integer(val)); }
        static JSON value (unsigned int val) { return JSON(JSONElements::Integer(val)); }
        static JSON value (long val) { return JSON(JSONElements::Integer(val)); }
        static JSON value (unsigned long val) { return JSON(JSONElements::Integer(val)); }
        static JSON value (long long val) { return JSON(JSONElements::Integer(val)); }
        static JSON value (unsigned long long val) { return JSON(JSONElements::Integer(val)); }

        static JSON value (double val) { return JSON(JSONElements::Float  (val)); }
        static JSON value (float  val) { return JSON(JSONElements::Float  (val)); }

        static JSON value (const std::string& val) { return JSON(JSONElements::String(val)); }
		static JSON value (const std::wstring& val) { return JSON(JSONElements::String(utf16_to_utf8(val))); }
		static JSON value (const char* val) { return JSON(JSONElements::String(val)); }
		static JSON value (const wchar_t* val) { return JSON(JSONElements::String(utf16_to_utf8(val))); }

		template<class T>
		static JSON value(const std::list<T>& list) { return arrayFromIterable(list); }
		template<class T>
		static JSON value(const std::set<T>& list) { return arrayFromIterable(list); }
		template<class T>
		static JSON value(const std::vector<T>& list) { return arrayFromIterable(list); }
		template<class T>
		static JSON value(const std::map<std::string, T>& map) { return oblectFromIterable(map); }
		//template<class T>
		static JSON value(const std::initializer_list<std::pair<std::string, YUVsoft::JSON>>& map) { return oblectFromIterable(map); }

		template<class T>
		static JSON arrayFromIterable(const T& list) {
			JSON res = array();
			for(auto& val : list) {
				res(value(val));
			}
			return res;
		}

		template<class T>
		static JSON oblectFromIterable(const T& map) {
			JSON res = object();
			for(auto& val : map) {
				res(val.first, value(val.second));
			}
			return res;
		}

        static JSON null() { return JSON(); }

        static JSON object();
        static JSON array();

        JSON(const JSONElements::Elem& r) : elem(NULL) {
            set(r);
        }
        JSON(const JSON& r) : elem(NULL) {
            set(*r.elem);
        }
        JSON& operator = (const JSONElements::Elem& r) {
            set(r);
            return *this;
        }
        JSON& operator = (const JSON& r) {
            set(*r.elem);
            return *this;
        }
        void set(const JSONElements::Elem& r) {
            if( &r==elem ) return;
            delete elem;
            elem = r.clone();
        }
        JSONElements::Elem& get() {
            return *elem;
        }
        const JSONElements::Elem& get() const {
            return *elem;
        }

        class Error : public std::exception {};
        class IndexOutRange : public Error {};
        class InvalidType   : public Error {};

        bool isArray () const { return elem->isArray (); }
        bool isObject() const { return elem->isObject(); }

        bool isNull() const { return dynamic_cast<JSONElements::Null*>(elem) != NULL; }

        bool isString () const { return dynamic_cast<JSONElements::String *>(elem) != NULL; }
        bool isInteger() const { return dynamic_cast<JSONElements::Integer*>(elem) != NULL; }
        bool isBoolean() const { return dynamic_cast<JSONElements::Boolean*>(elem) != NULL; }
        bool isFloat  () const { return dynamic_cast<JSONElements::Float  *>(elem) != NULL; }

        const std::string& asString()                         const { return asT<JSONElements::String>(); }
        const std::string& asString(const std::string& byDef) const { return asT<JSONElements::String>(byDef); }

        int64_t asInteger()              const { return asT<JSONElements::Integer>(); }
        int64_t asInteger(int64_t byDef) const { return asT<JSONElements::Integer>(byDef); }

        bool   asBoolean()           const { return asT<JSONElements::Boolean>(); }
        bool   asBoolean(bool byDef) const { return asT<JSONElements::Boolean>(byDef); }

        double asFloat()             const { return asT<JSONElements::Float>(); }
        double asFloat(double byDef) const { return asT<JSONElements::Float>(byDef); }

		JSONElements::Elem::Map::iterator begin() {
			if (!elem->isObject()) throw InvalidType();
			return elem->begin();
		}
		JSONElements::Elem::Map::const_iterator begin() const {
			if (!elem->isObject()) throw InvalidType();
			return elem->begin();
		}
		JSONElements::Elem::Map::iterator end() {
			if (!elem->isObject()) throw InvalidType();
			return elem->end();
		}
		JSONElements::Elem::Map::const_iterator end() const {
			if (!elem->isObject()) throw InvalidType();
			return elem->end();
		}

		class IterableArray {
		public:
			IterableArray(JSON& j) : json(j) {}
			IterableArray& operator =(const IterableArray&) = delete;
			
			JSONElements::Elem::Array::iterator begin() {
				if (!json.isArray()) throw InvalidType();
				return json.elem->arrBegin();
			}
			JSONElements::Elem::Array::iterator end() {
				if (!json.isArray()) throw InvalidType();
				return json.elem->arrEnd();
			}

		private:
			JSON& json;
		};
		class ConstIterableArray {
		public:
			ConstIterableArray(const JSON& j) : json(j) {}
			ConstIterableArray& operator =(const IterableArray&) = delete;
			
			JSONElements::Elem::Array::const_iterator begin() {
				if (!json.isArray()) throw InvalidType();
				return json.elem->arrBegin();
			}
			JSONElements::Elem::Array::const_iterator end() {
				if (!json.isArray()) throw InvalidType();
				return json.elem->arrEnd();
			}

		private:
			const JSON& json;
		};
		IterableArray iterableArray() { return IterableArray(*this); }
		ConstIterableArray iterableArray() const { return ConstIterableArray(*this); }
		
        bool in(const std::string& index) const {
            if(!elem->isObject() ) throw InvalidType();
            return elem->hasIndex(index);
        }
        bool in(int index) const {
            if(!elem->isArray() ) throw InvalidType();
            return elem->hasIndex(index);
        }

        JSON& operator [] (const std::string& index) {
            if(!elem->isObject() ) throw InvalidType();
            JSON* res = elem->getByIndex(index);
            //if(!res ) throw IndexOutRange(); //never true only this case

            return *res;
        }
        JSON& operator [] (int index) {
            if(!elem->isArray() ) throw InvalidType();
            JSON* res = elem->getByIndex(index);
            if(!res ) throw IndexOutRange();

            return *res;
        }

        const JSON& operator [] (const std::string& index) const {
            if(!elem->isObject() ) throw InvalidType();
            JSON* res = elem->getByIndex(index);
            if(!res ) throw IndexOutRange();

            return *res;
        }
        const JSON& operator [] (int index) const {
            if(!elem->isArray() ) throw InvalidType();
            JSON* res = elem->getByIndex(index);
            if(!res ) throw IndexOutRange();

            return *res;
        }

        size_t length() const {
            size_t res = elem->length();
            if( res==size_t(-1) ) throw InvalidType();

            return res;
        }

		JSON convert(const JSONElements::ElemTypeId* tag) const {
			if (getTypeTag() == tag) return *this;

			auto res = elem->convert(tag);
			if (!res) return JSON();

			return JSON(*res);
		}

		//same for elements of same types, different for different types
		const JSONElements::ElemTypeId* getTypeTag() const {
			return elem->elemTypeId();
		}

        //insert into object
        JSON& operator () (const std::string& key, const JSON& value) {
            if(!elem->isObject() ) throw InvalidType();
            elem->insert(key,value);

            return *this;
        }

        //append to array
        JSON& operator () (const JSON& value) {
            if(!elem->isArray() ) throw InvalidType();
            elem->append(value);

            return *this;
        }

		friend std::ostream& operator << (std::ostream& s, const JSON& r) {
			r.elem->writeToStream(s, -1, 0);
			return s;
		}

		bool operator < (const JSON& r) const {
			if( elem->elemTypeId() < r.elem->elemTypeId() ) return true;
			if( elem->elemTypeId() > r.elem->elemTypeId() ) return false;
			return elem->compare(r.elem) < 0;
		}

		bool operator <= (const JSON& r) const {
			if( elem->elemTypeId() < r.elem->elemTypeId() ) return true;
			if( elem->elemTypeId() > r.elem->elemTypeId() ) return false;
			return elem->compare(r.elem) <= 0;
		}

		bool operator > (const JSON& r) const {
			return r < *this;
		}

		bool operator >= (const JSON& r) const {
			return r <= *this;
		}

		bool operator == (const JSON& r) const {
			if( elem->elemTypeId() != r.elem->elemTypeId() ) return false;
			return elem->compare(r.elem) == 0;
		}

		bool operator != (const JSON& r) const {
			return ! operator == (r);
		}

		//combine arrays and objects
		JSON operator + (const JSON& r) const {
			JSON res(*this);

			if( isArray() && r.isArray()) {
				for(auto i : r.iterableArray()) res(i);
			} else if( isObject() && r.isObject() ) {
				for(auto i : r) res(i.first, i.second);
			} else {
				throw InvalidType();
			}

			return res;
		}

		std::string serialize(int prettyDepth = -1, int offset = 0) const {
			std::ostringstream s;
			writeToStream(s, prettyDepth, offset);

			return s.str();
		}

		//pretty depth == -1 means all pretty
		void writeToStream(std::ostream& s, int prettyDepth, int offset = 0) const {
			elem->writeToStream(s, prettyDepth, offset);
		}
	};

	namespace JSONElements {
        class Array : public Elem {
            typedef std::vector<JSON> List;
            List elements;

            bool isArray () const override { return true; }
            bool hasIndex( int i ) const override { return i>=0 && i < int(elements.size()); }
			bool hasIndex( const std::string& i ) const override { return false; }
            JSON* getByIndex( int i ) { 
                if(!hasIndex(i) ) return NULL;
                return &elements[i];
            }
            virtual const JSON* getByIndex( int i ) const { 
                if(!hasIndex(i) ) return NULL;
                return &elements[i];
            }
            virtual JSON* getByIndex( const std::string& i ) { 
                return nullptr;
            }
            virtual const JSON* getByIndex( const std::string& i ) const { 
                return nullptr;
            }
            virtual void append(const JSON& val) { 
                elements.push_back(val);
            }
            virtual size_t length() const {
                return elements.size();
            }

        public:
            Array* clone() const {
                return new Array(*this);
            }

			static ElemTypeId* getElemTypeId() {
				static ElemTypeId res;
				return &res;
			}
			virtual ElemTypeId* elemTypeId() const override {
				return getElemTypeId();
			}
			virtual int compare(const Elem* r) const override {
				const Array* pr = dynamic_cast<const Array*>(r);
				assert(pr);

				for(int i=0;i<int(std::min(elements.size(), pr->elements.size()));++i) {
					if(elements[i] < pr->elements[i]) return -1;
					if(elements[i] > pr->elements[i]) return 1;
				}
				if(elements.size() < pr->elements.size()) return -1;
				if(elements.size() > pr->elements.size()) return 1;
				return 0;
			}

            void swap(Array& r) {
                std::swap(elements,r.elements);
            }

            void push(const Elem& elem) {
                elements.push_back(*elem.clone());
            }

            Array() {}

            Array& operator () (const Elem& elem) {
                push(elem);
                return *this;
            }

            Array(const Array& r) {
                for(List::const_iterator iter = r.elements.begin(); iter!=r.elements.end(); ++iter) {
                    elements.push_back(*iter);
                }
            }

            Array& operator = (const Array& r) {
                Array copy(r);
                swap(copy);

                return *this;
			}

			void writeToStream(std::ostream& stream, int prettyDepth, int offset) const override {
				if( elements.empty() ) {
					stream << "[]";
					return;
				}

				stream << "[";
				for(List::const_iterator iter = elements.begin(); iter!=elements.end(); ++iter) {
					if( iter != elements.begin() ) stream << ",";
					if( prettyDepth!=0 )stream << "\n" << std::string(offset + 2, ' ');
					iter->get().writeToStream(stream, prettyDepth>0 ? prettyDepth-1 : prettyDepth , offset + 2);
				}
				if( prettyDepth!=0 ) stream << "\n" << std::string(offset, ' ');
				stream << "]";
			}

			List::iterator arrBegin() override { return elements.begin(); }
			List::iterator arrEnd()   override { return elements.end(); }
			List::const_iterator arrBegin() const  override { return elements.begin(); }
			List::const_iterator arrEnd()   const  override { return elements.end(); }
        };

        class Object : public Elem {
			//typedef OrderedMap<std::string, JSON> Map;
            Map elements;

            virtual bool isObject() const { return true; }
            virtual bool hasIndex( int  ) const { 
                return false;
            }
            virtual bool hasIndex( const std::string& i ) const { 
                return elements.find(i) != elements.end();
            }
            virtual JSON* getByIndex( int ) { 
                return nullptr;
            }
            virtual const JSON* getByIndex( int ) const { 
                return nullptr;
            }
            virtual JSON* getByIndex( const std::string& i ) { 
                /*Map::iterator iter = elements.find(i);
                if( iter == elements.end() ) return NULL;
                return &iter->second;*/
                return &elements[i];
            }
            virtual const JSON* getByIndex( const std::string& i ) const { 
                Map::const_iterator iter = elements.find(i);
                if( iter == elements.end() ) return NULL;
                return &iter->second;
            }
            virtual void insert(const std::string& key, const JSON& val) { 
                elements[key] = val;
            }
            virtual size_t length() const {
                return elements.size();
            }
        public:
            Object* clone() const {
                return new Object(*this);
            }
			
			static ElemTypeId* getElemTypeId() {
				static ElemTypeId res;
				return &res;
			}
			virtual ElemTypeId* elemTypeId() const override {
				return getElemTypeId();
			}
			virtual int compare(const Elem* r) const override {
				const Object* pr = dynamic_cast<const Object*>(r);
				assert(pr);

				auto il = elements.begin(), ir = pr->elements.begin();
				for(;il!=elements.end() && ir != pr->elements.end(); ++il, ++ir) {
					if(*il < *ir) return -1;
					if(*il > *ir) return 1;
				}
				if(il==elements.end() && ir != pr->elements.end()) return -1;
				if(il!=elements.end() && ir == pr->elements.end()) return 1;
				return 0;
			}

            void swap(Object& r) {
                std::swap(elements,r.elements);
            }

            void set(const std::string key, const JSON& elem) {
                //delete elements[key];
                elements[key] = elem;
            }

            Object& operator () (const std::string key, const JSON& elem) {
                set(key,elem);
                return *this;
            }

            Object() {}

            Object(const Object& r) {
                for(Map::const_iterator iter = r.elements.begin(); iter!=r.elements.end(); ++iter) {
					elements.insert({ iter->first, iter->second });
                }
            }

            Object& operator = (const Object& r) {
                Object copy(r);
                swap(copy);

                return *this;
			}


			virtual Map::iterator begin() override { return elements.begin(); }
			virtual Map::iterator end()   override { return elements.end();   }
			virtual Map::const_iterator begin() const override { return elements.begin(); }
			virtual Map::const_iterator end()   const override { return elements.end();   }

			void writeToStream(std::ostream& stream, int prettyDepth, int offset) const override {
				if( elements.empty() ) {
					stream << "{}";
					return;
				}

				int maxKeyLength = 0;
				if(prettyDepth==1) {
					for(Map::const_iterator iter = elements.begin(); iter!=elements.end(); ++iter) {
						int keyLength = int(escaped(iter->first).size());
						maxKeyLength = std::max(keyLength, maxKeyLength);
					}
				}

				stream << "{";
				for(Map::const_iterator iter = elements.begin(); iter!=elements.end(); ++iter) {
					if( iter != elements.begin() ) stream << ",";
					if( prettyDepth!=0 ) stream << "\n" << std::string(offset + 2, ' ');
					std::string key = escaped(iter->first);
					stream << "\"" << key << "\"";
					if(prettyDepth==1) stream << std::string(maxKeyLength - key.size(), ' ');
					stream << " : ";
					iter->second.get().writeToStream(stream, prettyDepth>0 ? prettyDepth-1 : prettyDepth, offset + 2);
				}
				if( prettyDepth!=0 ) stream << "\n" << std::string(offset, ' ');
				stream << "}";
			}
		};

		inline std::shared_ptr<Elem> Boolean::convert(const JSONElements::ElemTypeId* tag) const {
			if (tag == Integer::getElemTypeId()) {
				return std::make_shared<Integer>(value ? 1 : 0);
			}
			if (tag == String::getElemTypeId()) {
				return std::make_shared<String>(value ? "true" : "false");
			}
			return{};
		}
		inline std::shared_ptr<Elem> Integer::convert(const JSONElements::ElemTypeId* tag) const {
			if (tag == String::getElemTypeId()) {
				return std::make_shared<String>(std::to_string(value));
			}
			if (tag == Boolean::getElemTypeId()) {
				return std::make_shared<Boolean>(value != 0);
			}
			if (tag == Float::getElemTypeId()) {
				return std::make_shared<Float>(double(value));
			}
			return{};
		}
		inline std::shared_ptr<Elem> String::convert(const JSONElements::ElemTypeId* tag) const {
			if (tag == Integer::getElemTypeId()) {
				long long val;
				try {
					size_t pos = 0;
					val = std::stoll(value, &pos);
					if (pos != value.size()) throw std::exception();
				}
				catch (...) { return{}; }
				return std::make_shared<Integer>(val);
			}
			if (tag == Boolean::getElemTypeId()) {
				bool val;
				if (value == "true") val = true;
				else if (value == "false") val = false;
				else return{};
				return std::make_shared<Boolean>(val);
			}
			if (tag == Float::getElemTypeId()) {
				double val;
				try {
					size_t pos = 0;
					val = std::stod(value, &pos);
					if (pos != value.size()) throw std::exception();
				}
				catch (...) { return{}; }
				return std::make_shared<Float>(val);
			}
			return{};
		}
		inline std::shared_ptr<Elem> Float::convert(const JSONElements::ElemTypeId* tag) const {
			if (tag == String::getElemTypeId()) {
				return std::make_shared<String>(std::to_string(value));
			}
			if (tag == Integer::getElemTypeId()) {
				return std::make_shared<Integer>(int64_t(value));
			}
			return{};
		}
    }

    inline JSON JSON::array () { return JSON(JSONElements::Array ()); }
    inline JSON JSON::object() { return JSON(JSONElements::Object()); }

    class GeneralParser {
        struct CompiledState;
    public:
        typedef unsigned int StateIdx;
        class ParseTree;
        class Unicode {
            typedef unsigned long Int4bytes;
            Int4bytes data;
        public:
            Unicode() {
                clear();
            }
            void clear() {
                data = 0;
            }
            void setErr() {
                data = Int4bytes(-1);
            }
            bool isNull() const {
                return data == 0;
            }
            bool isErr() const {
                return data >= 0x80000000;
            }
            static Unicode byCode(Int4bytes code) {
                Unicode u;
                u.data = code;
                return u;
            }
            const char* readUTF8Char(const char* utf8data, size_t maxsz) {
                if (maxsz==0) {
                    setErr();
                    return utf8data;
                }
                if (*utf8data<0x80) {
                    data = *utf8data;
                    return utf8data + 1;
                }
                int size;
                for(size=1; size<=6;++size) {
                    if (((*utf8data) >> (7-size)) == ((1<<(size+1)) - 2)) break;
                }
                if( size==1 || size>6 || maxsz < size_t(size) ) {
                    setErr();
                    return utf8data;
                }
                static const Int4bytes offset[] = {0, 0x00000080, 0x00000800, 0x00010000, 0x00200000, 0x04000000};
                //int curBits = 7-size;
                data = *utf8data & ((1<<(8-size))-1);

                for(int i=1;i<size;++i) {
                    if( utf8data[i] >> 6 != 0x2 ) {
                        setErr();
                        return utf8data;
                    }
                    data = (data << 6) + (utf8data[i] & 0x7F);
                }

                data += offset[size - 1];
                return utf8data + size;
            }
            int getSize() const {
                static const Int4bytes offset[] = {0, 0x00000080, 0x00000800, 0x00010000, 0x00200000, 0x04000000, 0x80000000};
                for(int i=1;i<=6;++i) {
                    if( data < offset[i] ) return i;
                }
                return -1;
            }
            char* writeUTF8Char(char* utf8data, size_t maxsz) const {
                static const Int4bytes offset[] = {0, 0x00000080, 0x00000800, 0x00010000, 0x00200000, 0x04000000, 0x80000000};
                if( data >= 0x80000000) {
                    return utf8data;
                }

                size_t size = size_t(getSize());
                if (maxsz<size) return utf8data;

                if (data<offset[1]) {
                    *utf8data = char(data);
                    return utf8data + 1;
                }

                Int4bytes unoffData = data - offset[size - 1];

                int curShift = int(6*(size-1));
                *utf8data = char((((1<<(size+1)) - 2) << (7-size)) + (unoffData >> curShift));
                for(size_t i=1;i<size;++i) {
                    curShift -= 6;
                    utf8data[i] = 0x80 + ((data >> curShift) & 0x7F );
                }

                return utf8data + size;
            }
        };
        class Parsing {
        protected:
            template<class T>
            class Stack {
                std::vector<T> data;
                size_t pointer;
            public:
                Stack() : pointer(0) {}
                void push(T elem) {
                    if (pointer>=int(data.size())) data.resize(data.size()*2 + 1);
                    data[pointer++] = elem;
                }
                T pop() {
                    assert(pointer > 0);
                    return data[--pointer];
                }
                T& top() {
                    assert(pointer > 0);
                    return data[pointer-1];
                }
                const T& top() const {
                    assert(pointer > 0);
                    return data[pointer-1];
                }
                bool empty() const {
                    return pointer == 0;
                }
                T* front() {
					return data.empty() ? nullptr : &data[0];
                }
                size_t size() const {
                    return pointer;
                }
                void clear() {
                    pointer = 0;
                }
            };

        private:
            Stack<char> charBuffer;
            Stack<long long> intBuff;
            Stack<StateIdx> returnStack;

            long long lastInt;
            char lastChar;

            GeneralParser* parser;
            CompiledState* currentState;

            bool errorParsing;
            bool finishedParsing;

            long long charsRead;
        public:
            Parsing(ParseTree& tree) : lastInt(0), parser(tree.getParser()), charsRead(0),
                currentState(tree.getCompiledFirst()), errorParsing(false), finishedParsing(false) {
            }
            virtual ~Parsing() {}

            //return number of character parsed
            size_t parseString(const std::string& string) {
                const char* start = string.c_str();
                const char* end = start+string.size();
                const char* parseEnd = feedChars(start,string.size());
                if( parseEnd == end ) {
                    feedEOF();
                }
                return parseEnd - start;
            }

            //postcondition: ret = chars + len OR finishedParsing OR errorParsing
            const char* feedChars(const char* chars, size_t len) {
                while(len>0 && !finishedParsing) {
                    StateIdx nextIdx = currentState->charNextState[(unsigned char)(*chars)];
                    if( nextIdx == StateIdx(-1) ) {
                        nextIdx = currentState->implicitCome;
                    } else {
                        lastChar = *chars;
                        chars++;len--;
                        charsRead++;
                        //std::cout<<"READ "<<lastChar<<std::endl;
                    }
                    if( nextIdx == StateIdx(-1) ) {
                        errorParsing = true;
                        return chars;
                    }
                    currentState = &parser->allStates[nextIdx];
                    currentState->semantic.exec(this);
                }

                return chars;
            }
            long long getCharsRead() const {
                return charsRead;
            }
            char getLastCharacter() const {
                return lastChar;
            }
            long long& getLastInt() {
                return lastInt;
            }
            std::string popString() {
                std::string str(charBuffer.front(), charBuffer.size());
                charBuffer.clear();
                return str;
            }
            long long popInt() {
                return intBuff.pop();
            }
            void feedEOF() {
                if( errorParsing ) return;
                while(!finishedParsing ) {
                    if( currentState->implicitCome == StateIdx(-1) ) {
                        errorParsing = true;
                        return;
                    }
                    currentState =&parser->allStates[currentState->implicitCome];
                    currentState->semantic.exec(this);
                };
            }
            void doReturn() {
                //std::cout<<"RETURN"<<std::endl;
                if( returnStack.empty() ) {
                    finishedParsing = true;
                    currentState =&parser->allStates[0];
                    return;
                }
                currentState =&parser->allStates[returnStack.pop()];
                currentState->semantic.exec(this);
            }
            void pushReturnState(StateIdx state) {
                //std::cout<<"CALL "<<(currentState - &parser->allStates[0])<<std::endl;
                returnStack.push(state);
            }
            void pushChar(char c) {
                charBuffer.push(c);
            }
            void pushInt(long long c) {
                intBuff.push(c);
            }

            bool inErrorState() const {
                return errorParsing;
            }
            bool inFinishedState() const {
                return finishedParsing;
            }
        };

		struct AbsId {
			AbsId() {
				static int absid = 0;
				val = absid++;
			}
			int val;
		};

        class State;
        class ParseTree {
            State *firstState, *lastState;
            GeneralParser* parser;
            CompiledState *compiledFirst, *compiledLast;
        public:
            ParseTree() : firstState(NULL), lastState(NULL) {}

            void setStates(State& first, State& last) {
                firstState = &first;
                lastState  = &last;
                parser = first.getParser();

                getParser()->regTree(this);

                last>>Semantic::returnState();
            }

            GeneralParser* getParser() {
                return parser;
            }

            void optimize() {
                firstState->optimize();
            }

            void compile() {
                compiledFirst = firstState->compiledState();
                compiledLast  =  lastState->compiledState();
            }

            State* getFirstState() const {
                return firstState;
            }
            State* getLastState() const {
                return firstState;
            }
            CompiledState* getCompiledFirst() const {
                return compiledFirst;
            }

			AbsId absId;
        };

        class Semantic {
        public:
            class ExtentionData {
            public:
                virtual ExtentionData* clone() const =0;
                virtual ~ExtentionData(){}
            };
            class Action : public ExtentionData {
            public:
                virtual void call(Parsing* parsing) = 0;
                virtual ~Action() {}
            };
            typedef void (UserProc) (Parsing* parsing, ExtentionData*);
        private:
            struct StringData : public ExtentionData {
                StringData(const std::string& r) : data(r) {}
                std::string data;
                ExtentionData* clone() const {
                    return new StringData(data);
                }
            };
            enum PreservedSemantics {
                S_RELAX,
                S_PUSH_LAST_CHAR,
                S_PUSH_LAST_NUMBER,
                S_PUSH_CHAR,
                S_PUSH_INT,
                S_PUSH_DIGIT_IN_STACK,
                S_PUSH_HEXDIGIT_IN_STACK,
                S_PUSH_UNICODE,
                S_RETURN,
                S_DEBUG,
                S_CALL,
                S_CALL_USER_PROC,
                S_DO_USER_ACTION
            } semantic;
            union {
                char character;
                long long integer;
                StateIdx state;
                UserProc* userProc;
            } actionData;
            ExtentionData* extention;
        public:
            Semantic() : semantic(S_RELAX), extention(NULL) {}
            Semantic(const Semantic& r) 
                : semantic(r.semantic), actionData(r.actionData), extention(r.extention ? r.extention->clone() : NULL) {
            }
            ~Semantic() {
                delete extention;
            }
            Semantic& operator = (const Semantic& r)  {
                Semantic l(r);

                std::swap(semantic  ,l.semantic  );
                std::swap(actionData,l.actionData);
                std::swap(extention ,l.extention );

                return *this;
            }

			friend std::ostream& operator << (std::ostream& o, const Semantic& semantic) {
				const char* lbl[] = { "REL", "LASTCH", "LASTNMB", "CHAR", "INT", "DIGST", "HEXST", "U", "RET", "DBG", "CALL", "USERP", "USERA" };

				o << lbl[semantic.semantic];
				switch (semantic.semantic) {
				case S_PUSH_CHAR:
					o << (int)semantic.actionData.character; break;
				case S_PUSH_INT:
					o << semantic.actionData.integer; break;
				case S_DEBUG:
					o << dynamic_cast<StringData*>(semantic.extention)->data << std::endl;
					break;
				case S_CALL:
					o << semantic.actionData.state;
					break;
				}

				return o;
			}

            void exec(Parsing* parsing) {
                switch(semantic) {
                case S_RELAX: break;
                case S_PUSH_LAST_CHAR:
                    parsing->pushChar(parsing->getLastCharacter());
                    break;
                case S_PUSH_LAST_NUMBER:
                    parsing->pushInt(parsing->getLastInt());
                    parsing->getLastInt() = 0;
                    break;
                case S_PUSH_CHAR:
                    parsing->pushChar(actionData.character);
                    break;
                case S_PUSH_INT:
                    parsing->pushInt(actionData.integer);
                    break;
                case S_PUSH_DIGIT_IN_STACK:
                    {
                        char c = parsing->getLastCharacter();
                        assert(c>='0' && c<='9');
                        parsing->getLastInt() *= 10;
                        parsing->getLastInt() += c-'0';
                    }
                    break;
                case S_DEBUG:
                    std::cout<<dynamic_cast<StringData*>(extention)->data<<std::endl;
                    break;
                case S_PUSH_HEXDIGIT_IN_STACK: 
                    {
                        char c = parsing->getLastCharacter();
                        int digit;
                            
                        if( c>='0' && c<='9' ) digit = c - '0';
                        else if( c>='a' && c<='f' ) digit = c - 'a' + 10;
                        else if( c>='A' && c<='F' ) digit = c - 'A' + 10;
                        else assert(false);

                        parsing->getLastInt() *= 16;
                        parsing->getLastInt() += digit;
                    }
                    break;
                case S_PUSH_UNICODE:
                    {
                        unsigned long i = (unsigned long)(parsing->getLastInt());
                        parsing->pushChar((i>> 0) & 0xFF);
                        parsing->pushChar((i>> 8) & 0xFF);
                        parsing->pushChar((i>>16) & 0xFF);
                        parsing->pushChar((i>>24) & 0xFF);
                        parsing->getLastInt() = 0;
                    }
                    break;
                case S_RETURN:
                    parsing->doReturn();
                    break;
                case S_CALL:
                    parsing->pushReturnState(actionData.state);
                    break;
                case S_CALL_USER_PROC: 
                    (*actionData.userProc)(parsing, extention);
                    break;
                case S_DO_USER_ACTION:
                    Action* act = dynamic_cast<Action*>(extention);
                    assert(act);
					act->call(parsing);
					break;
                }
            }
            bool isRelax() const {
                return semantic == S_RELAX;
            }
            static Semantic call(StateIdx returnState) {
                Semantic obj;
                obj.semantic = S_CALL;
                obj.actionData.state = returnState;
                return obj;
            }
            static Semantic returnState() {
                Semantic obj;
                obj.semantic = S_RETURN;
                return obj;
            }
            static Semantic placeLastCharInStack() {
                Semantic obj;
                obj.semantic = S_PUSH_LAST_CHAR;
                return obj;
            }
            static Semantic placeInStack(char c) {
                Semantic obj;
                obj.semantic = S_PUSH_CHAR;
                obj.actionData.character = c;
                return obj;
            }
            static Semantic placeIntInStack(long long nmb) {
                Semantic obj;
                obj.semantic = S_PUSH_INT;
                obj.actionData.integer = nmb;
                return obj;
            }
            static Semantic pushDigit() {
                Semantic obj;
                obj.semantic = S_PUSH_DIGIT_IN_STACK;
                return obj;
            }
            static Semantic pushHexDigit() {
                Semantic obj;
                obj.semantic = S_PUSH_HEXDIGIT_IN_STACK;
                return obj;
            }
            static Semantic pushUnicodeChar() {
                Semantic obj;
                obj.semantic = S_PUSH_UNICODE;
                return obj;
            }
            static Semantic placeLastNumberInStack() {
                Semantic obj;
                obj.semantic = S_PUSH_LAST_NUMBER;
                return obj;
            }
            static Semantic debug(const std::string& str) {
                Semantic obj;
                obj.semantic = S_DEBUG;
                obj.extention = new StringData(str);
                return obj;
            }
            static Semantic userProc(UserProc proc) {
                Semantic obj;
                obj.semantic = S_CALL_USER_PROC;
                obj.actionData.userProc = proc;
                return obj;
            }
            static Semantic doAction(const Action& action) {
                Semantic obj;
                obj.semantic = S_DO_USER_ACTION;
                obj.extention = action.clone();
                return obj;
            }
        };
            
        class SemanticVector {
            std::vector<Semantic> vector;
        public:
            bool isRelax() const {
                if( vector.empty() ) return true;
                const Semantic* data = &vector[0];
                for(size_t i=0, end = vector.size();i<end;++i) {
                    if(!data[i].isRelax()) return false;
                }
                return true;
            }
            void exec(Parsing* parsing) {
                if( vector.empty() ) return;

                Semantic* data = &vector[0];
                for(size_t i=0, end = vector.size();i<end;++i) {
                    data[i].exec(parsing);
                }
            }
            void insert(const Semantic& r) {
                vector.push_back(r);
            }
            void concat(const SemanticVector& r) {
                vector.insert(vector.end(),r.vector.begin(),r.vector.end());
            }
            Semantic& operator [] (int i) {
                return vector.at(i);
            }

			friend std::ostream& operator << (std::ostream& o, const SemanticVector& semantic) {
				o << "S:";
				for (const Semantic& s : semantic.vector) {
					o << " " << s;
				}
				return o;
			}
        };

        class CharacterStep;
        class DigitStep;
        class HexdigitStep;
        class StringsStep;

        class Step {
        protected:
            State* leadingState;
        public:
            virtual ~Step() {}

            //typedef std::auto_ptr<Step> Ptr;

            void setLeadingState(State* state) {
                leadingState = state;
            }
            State* getLeadingState() {
                return leadingState;
            }

            virtual void preCompile(GeneralParser* parser) {};
            virtual void compile(State* from) =0;
            virtual Step* clone() const =0;

            static CharacterStep character(char c) {
                return CharacterStep(c);
            }
            static CharacterStep character(char c1,char c2) {
                return CharacterStep(c1,c2);
            }
            /*static UnicodeStep unicodeExcept(Unicode c) {
                return UnicodeStep(c);
            }*/
            static CharacterStep character() {
                return CharacterStep();
            }
            static DigitStep digit() {
                return DigitStep();
            }
            static HexdigitStep hexDigit() {
                return HexdigitStep();
            }
            static StringsStep string(const std::string& str) {
                return StringsStep(str);
            }
        };

        class CharacterStep : public Step {
            std::list<unsigned char> chars;
            CharacterStep(const std::list<unsigned char>& list) : chars(list) {}
        public:
            CharacterStep() {
                for(int i=0;i<=int((unsigned char)(-1));++i) {
                    chars.push_back(char(i));
                }
            }
            CharacterStep(char c) {
                operator () (c);
            }
                
            CharacterStep(char c1,char c2) {
                for(char c=c1;;++c) {
                    chars.push_back(c);
                    if( c==c2 ) break;
                }
            }
            CharacterStep& operator () (char c) {
                chars.push_back(c);
                return *this;
            }
            CharacterStep* clone() const override {
                return new CharacterStep(chars);
            }
            void compile(State* from) {
                for(std::list<unsigned char>::iterator iter = chars.begin();iter!=chars.end();++iter) {
                    from->compiledState()->charNextState[*iter] = leadingState->getIdx();
                }
            }
            virtual ~CharacterStep() {}
        };

        class DigitStep : public Step {
        public:
            DigitStep() {}
            DigitStep* clone() const override {
                return new DigitStep();
            }
            void compile(State* from) {
                for(unsigned char i='0';i<='9';++i) {
                    from->compiledState()->charNextState[i] = leadingState->getIdx();
                }
            }
            virtual ~DigitStep() {}
        };
        class HexdigitStep : public Step {
        public:
            HexdigitStep() {}
            HexdigitStep* clone() const override {
                return new HexdigitStep();
            }
            void compile(State* from) {
                for(unsigned char i='0';i<='9';++i) {
                    from->compiledState()->charNextState[i] = leadingState->getIdx();
                }
                for(unsigned char i='a';i<='f';++i) {
                    from->compiledState()->charNextState[i] = leadingState->getIdx();
                }
                for(unsigned char i='A';i<='F';++i) {
                    from->compiledState()->charNextState[i] = leadingState->getIdx();
                }
            }
            virtual ~HexdigitStep() {}
        };
        class StringsStep : public Step {
            typedef std::set<std::string> Set;
            Set allStrings;
            std::list<State*> tmpState;

            Set::iterator createTmpStatesForPrefix(GeneralParser* parser, const std::string& prefix) {
                Set::iterator iter;
                for(iter = allStrings.lower_bound(prefix);
                    iter!=allStrings.end() && iter->substr(0,prefix.size()) == prefix;) {
                    if( iter->size() > prefix.size() + 1 ) {
                        tmpState.push_back(&parser->allocTempState());
                        iter = createTmpStatesForPrefix( parser, iter->substr(0,prefix.size()+1) );
                    } else if( iter->size() > prefix.size() ) {
                        iter++;
                    }
                }
                return iter;
            }
            Set::iterator makeConnectionsForPrefix(State* init, State* fin, 
                std::list<State*>::iterator& stateIter, const std::string& prefix) {
                Set::iterator iter;
                for(iter = allStrings.lower_bound(prefix);
                    iter!=allStrings.end() && iter->substr(0,prefix.size()) == prefix;) {
                    if( iter->size() > prefix.size() + 1 ) {
                        State* curFin = *stateIter++;
                        init->compiledState()->charNextState[(unsigned char)((*iter)[prefix.size()])] = curFin->getIdx();
                        iter = makeConnectionsForPrefix( curFin, fin, stateIter, iter->substr(0,prefix.size()+1) );
                    } else if( iter->size() > prefix.size() ) {
                        init->compiledState()->charNextState[(unsigned char)((*iter)[prefix.size()])] = fin->getIdx();
                        iter++;
                    }
                }
                return iter;
            }
        public:
            StringsStep(const std::string& string) {
                operator () (string);
            }
            StringsStep(const std::set<std::string>& set) : allStrings(set) {
            }
            StringsStep& operator () (const std::string& string) {
                assert(string.size()>0);
                allStrings.insert(string);
                return *this;
            }
            StringsStep* clone() const override {
                return new StringsStep(allStrings);
            }
            void preCompile(GeneralParser* parser) override {
                if(!tmpState.empty() ) return;
                createTmpStatesForPrefix(parser,"");
            }
            void compile(State* from) override {
                std::list<State*>::iterator iter = tmpState.begin();
                makeConnectionsForPrefix(from,leadingState,iter,"");
            }
        };
        class StringStep : public Step {
            std::string str;
            std::vector<State*> tmpState;
        public:
            StringStep(const std::string& string) : str(string) {
                assert(string.size()>0);
            }
            StringStep* clone() const override {
                return new StringStep(str);
            }
            void preCompile(GeneralParser* parser) {
                if( tmpState.size()>0 ) return;

                tmpState.resize(str.size()+1);
                for(size_t i=0;i<str.size()-1;++i) {
                    tmpState[i+1] = &parser->allocTempState();
                }
            }
            void compile(State* from) {
                tmpState[0] = from;
                tmpState[str.size()] = leadingState;

                for(size_t i=0;i<str.size();++i) {
                    tmpState[i]->compiledState()->charNextState[(unsigned char)(str[i])] = tmpState[i+1]->getIdx();
                }
            }
            virtual ~StringStep() {}
        };

		struct AbsIdComarator {
			template<class T>
			bool operator () (T* l, T* r) const {
				return l->absId.val < r->absId.val;
			}
		};

        class State {
            GeneralParser* parser;

            SemanticVector semantic;

            //the State for using steps and calls and implicitCome fields from
            State* optimizedTo;
            StateIdx stateIdx;

            typedef std::list<Step*> Steps;
            Steps steps;

            class Call {
                ParseTree* tree;
                State* returnState;
                bool unrolled;

                std::set<State*, AbsIdComarator > newStates;
            public:
                Call(ParseTree* tree_, State* returnState_) : tree(tree_), returnState(returnState_), unrolled(false) {}
                void optimize() {
                    tree->optimize();
                    returnState->optimize();
                }
                void unroll(State* from) {
                    if (unrolled) return;
                    unrolled = true;

                    State* firstState = tree->getFirstState();
                    firstState->unrollCalls();
                        
                    assert(!from->implicitCome ||!firstState->optimizedTo->implicitCome);
                    if(!from->implicitCome && firstState->optimizedTo->implicitCome) {
                        State* newState = &from->parser->allocTempState();
                        newState->semantic.insert(Semantic::call(0));
                        newState->noOpt = true;
                        newStates.insert(newState);

                        from->implicitCome = newState;
                        newState->implicitCome = firstState->optimizedTo->implicitCome;
                    }

                    //assert(firstState->semantic.isRelax());
                    from->semantic.concat(firstState->semantic);

                    for(Steps::iterator iter=firstState->optimizedTo->steps.begin();iter!=firstState->optimizedTo->steps.end();++iter) {
                        Step* newStep = (*iter)->clone();
                        State* newState = &from->parser->allocTempState();
                        newState->implicitCome = (*iter)->getLeadingState();
                        newState->semantic.insert(Semantic::call(0)); //set dummy semantic to forbid optimize
                        newState->noOpt = true;
                        newStates.insert(newState);
                        newStep->setLeadingState(newState);
                            
                        from->steps.push_back(newStep);
                    }
                }
                void compile() {
                    for(State* state : newStates) {
                        state->semantic[0] = Semantic::call(returnState->stateIdx);
                    }
                }
            };

            typedef std::list<Call> Calls;
            Calls calls;

            State* implicitCome;

            bool wasOptimized;
            bool noOpt;

            State& operator = (const State);

            bool implicitOptimization() {
                if(!optimizedTo->steps.empty()) return false;
                if(!optimizedTo->calls.empty()) return false;
                if(!optimizedTo->implicitCome ) return false;
                if( optimizedTo->implicitCome->noOpt ) return false;
                //if(!optimizedTo->implicitCome->semantic.isRelax() ) {
                //    if(!semantic.isRelax()) return false;
                        
                //}

                semantic.concat(optimizedTo->implicitCome->semantic);
                optimizedTo = optimizedTo->implicitCome;
                return true;
            }
        public:
            State(GeneralParser* parser_) : parser(parser_), implicitCome(NULL), wasOptimized(false), optimizedTo(this), noOpt(false) {
                parser->statePointers.insert(this);
            }
            State(const State& r) : parser(r.parser), implicitCome(NULL), wasOptimized(false), optimizedTo(this), noOpt(false) {
                parser->statePointers.insert(this);
                assert(!r.implicitCome);
                assert(r.calls.empty());
                assert(r.steps.empty());
            }
            ~State() {
                parser->statePointers.erase(this);
                for(Steps::iterator iter = steps.begin();iter!=steps.end();++iter) {
                    delete *iter;
                }
            }

            void setIdx(StateIdx idx) {
                stateIdx = idx;
            }
            StateIdx getIdx() const {
                return stateIdx;
            }

            GeneralParser* getParser() {
                return parser;
            }

            CompiledState* compiledState() {
                return &parser->allStates[stateIdx];
            }

            //optimize and register this state in the parser
            void optimize() {
                if( wasOptimized ) return;
                wasOptimized = true;

                while(implicitOptimization());
                if( optimizedTo->implicitCome ) optimizedTo->implicitCome->optimize();
                for(Steps::iterator iter = optimizedTo->steps.begin(); iter!=optimizedTo->steps.end();++iter) {
                    (*iter)->getLeadingState()->optimize();
                }
                for(Calls::iterator iter = optimizedTo->calls.begin(); iter!=optimizedTo->calls.end();++iter) {
                    iter->optimize();
                }
            }

            /*
                * first call unrollCalls for all states
                * than preCompile for all states
                * than allocation compiledStates and assigning stateIdx
                * than compile for all states
                */
            void unrollCalls() {
                if( optimizedTo!=this ) return optimizedTo->unrollCalls();

                for(Calls::iterator iter = calls.begin(); iter!=calls.end();++iter) {
                    iter->unroll(this);
                }
            }

            /* allocate additional Stated needed for Steps */
            void preCompile() {
                for(Steps::iterator iter = optimizedTo->steps.begin(); iter!=optimizedTo->steps.end();++iter) {
                    (*iter)->preCompile(parser);
                }
            }

            /* can be called after stateIdx set for all States*/
            void compile() {
                parser->allStates[stateIdx].semantic = semantic;
                if( optimizedTo->implicitCome ) 
                    parser->allStates[stateIdx].implicitCome = optimizedTo->implicitCome->stateIdx;

                for(Steps::iterator iter = optimizedTo->steps.begin(); iter!=optimizedTo->steps.end();++iter) {
                    (*iter)->compile(this);
                }
                for(Calls::iterator iter = optimizedTo->calls.begin(); iter!=optimizedTo->calls.end();++iter) {
                    iter->compile();
                }
            }

            friend State& operator >> (State& l, State& r) {
                assert(!l.implicitCome);
                l.implicitCome = &r;

                return r;
            }
            friend State& operator >> (State& l, const Step& r) {
                State* next = &l.parser->allocTempState();

                Step* step = r.clone();
                step->setLeadingState(next);
                l.steps.push_back(step);

                return *next;
            }
            friend State& operator >> (State& l, const Semantic& r) {
                //if( l.semantic.isRelax() ) {
                    l.semantic.insert(r);
                    return l;
                /*}

                State* next = &l.parser->allocTempState();
                next->semantic = r;

                assert(!l.implicitCome);
                l.implicitCome = next;

                return *next;*/
            }
            friend State& operator >> (State& l, ParseTree& r) {
                State* next = &l.parser->allocTempState();

                l.calls.push_back(Call(&r,next));

                return *next;
            }

			AbsId absId;
        };

        void compile() {
            for(State* state : statePointers) {
                state->optimize();
            }
            for(State* state : statePointers) {
                state->unrollCalls();
            }
            for(State* state : statePointers) {
                state->preCompile();
            }

            allStates.resize(statePointers.size() + reservedStates);

			StateIdx i = reservedStates;
            for(State* state : statePointers) {
                state->setIdx(i++);
            }
            for(ParseTree* tree: allTrees) {
                tree->compile();
            }
            for(State* state : statePointers) {
                state->compile();
            }

			//createVisualization("c:\\temp\\bbb.gml");

            statePointers.clear();
            tempStates.clear();
        }

		void createVisualization(const std::string& fileName) {
			std::ofstream graph(fileName.c_str());

			graph << "graph\n[\ndirected 1\n";
			int i = 0;
			for (CompiledState& state : allStates) {
				graph << "node\n[\nid " << i << "\nlabel \"" << i << " " << state.semantic <<"\"\n]\n";

				i++;
			}
			i = 0;
			for (CompiledState& state : allStates) {
				std::map<int, std::list<int>> comes;
				for (int j = 0; j < 256; ++j) {
					if (state.charNextState[j]!=StateIdx(-1))
					comes[state.charNextState[j]].push_back(j);
				}
				if (state.implicitCome != StateIdx(-1))
					comes[state.implicitCome].push_back(-1);

				for (auto& come : comes) {
					graph << "edge\n[\nid " << (i + allStates.size()) << "\nsource " << i << "\ntarget " << come.first << "\nlabel \"";
					
					for (int targ : come.second) {
						if (targ == -1) graph << "%%";
						//else if (targ > 32) graph << (char)targ;
						else graph << 'x' << targ;
					}

					graph  << "\"\n]\n";
				}
				
				i++;
			}
			graph << "]\n";
		}

        State& allocTempState() {
            tempStates.push_back(State(this));
            State& result = *--tempStates.end();
            //statePointers.insert(&result);
            return result;
        }
        void regTree(ParseTree* tree) {
            allTrees.insert(tree);
        }
    private:
		std::set<State*, AbsIdComarator> statePointers;
        std::list<State> tempStates;
		std::set<ParseTree*, AbsIdComarator> allTrees;

        class SemanticList {
            struct ListElem {
                Semantic obj;
                ListElem* next;
                ListElem(const Semantic& s) : obj(s), next(NULL) {}
            };
            ListElem *first, *last;
        public:
            SemanticList() : first(NULL), last(NULL) {}
            SemanticList(const SemanticList& r) {
                ListElem* iter = r.first;
                while( iter ) {
                    insert(iter->obj);
                }
            }
            SemanticList& operator = (const SemanticList& r) {
                SemanticList copy(r);
                swap(copy);
                return *this;
            }
            void swap(SemanticList& r) {
                std::swap(first,r.first);
                std::swap(last,r.last);
            }
            ~SemanticList() {
                clear();
            }
            void clear() {
                while(first) {
                    ListElem* prev = first;
                    first = prev->next;
                    delete prev;
                }
            }
            void insert(const Semantic& data) {
                if(!first ) {
                    first = last = new ListElem(data);
                    return;
                }
                last->next = new ListElem(data);
                last = last->next;
            }
        };

        struct CompiledState {
            StateIdx charNextState[256];

            //UnicodeChar* unicodeExceptions;
            //StateIdx     unicodeNextState;

            //StateIdx hexDigit;
            //StateIdx digit;

            StateIdx implicitCome;

            SemanticVector semantic;

            CompiledState() : /*unicodeExceptions(NULL), unicodeNextState(-1), hexDigit(-1), digit(-1),*/ implicitCome(StateIdx(-1)) {
                std::fill(charNextState,charNextState + sizeof(charNextState)/sizeof(*charNextState), StateIdx(-1));
            }
        };
        static const StateIdx reservedStates = 2;
        std::vector<CompiledState> allStates;
    };

    class ParseWrapper;
    class JSONparser : public GeneralParser {
        friend class ParseWrapper;

        ParseTree string;
        ParseTree number;
        ParseTree value;
        ParseTree array;
        ParseTree object;
        ParseTree ws;

	public:
        struct Error {
            enum ErrorType {
                ERR_OK,
                ERR_EXPECTED_VALUE,
                ERR_EXPECTED_END_OF_OBJECT_OR_OBJECT_ELEMENT,
                ERR_EXPECTED_END_OF_ARRAY_OR_ARRAY_ELEMENT,
                ERR_EXPECTED_COMMA_OR_END_OF_OBJECT,
                ERR_EXPECTED_COMMA_OR_END_OF_ARRAY,
                ERR_EXPECTED_OBJECT_ELEMENT,
                ERR_EXPECTED_ARRAY_ELEMENT,
                ERR_EXPECTED_COLON,
                ERR_EXPECTED_WHITESPACE,
                ERR_EXPECTED_MODIFIER_AFTER_SLASH_IN_STR,
                ERR_EXPECTED_NOT_CONTROL_CHARACTER_IN_STR,
                ERR_CONTROL_CHAR_IN_STR,
                ERR_EXPECTED_FOUR_HEX_DIGITS_AFTER_SLASH_U_IN_STR,
                ERR_EXPECTED_DIGIT_OR_SIGN_AFTER_EXP_IN_NUMBER,
                ERR_EXPECTED_DIGIT_IN_NUMBER,
                ERR_EXPECTED_END_OF_FILE,
                ERR_UNKNOWN
            } type;
            long long offset;

            long long line;
            long long offsetInLine;

            Error() : type(ERR_OK), offset(0), line(0), offsetInLine(0) {}
            Error(ErrorType type, long long line, long long offsetInLine, long long offset) {
                Error::type = type;
                Error::line = line;
                Error::offsetInLine = offsetInLine;
                Error::offset = offset;
            }
			void raise() const YUVsoft_THROW(Error) {
                if( type==ERR_OK ) return;
                throw *this;
            }
        };

	private:
        class Parsing : public GeneralParser::Parsing {
            typedef GeneralParser::Parsing Base;
            struct NumberParsing {
                bool hasMinus;
                bool hasDot;
                bool hasExp;
                bool expMinus;

                long long wholePart;
                long long fraqPart;
                long long fraqPartLength;
                long long expPart;

                NumberParsing() : 
                    hasMinus(false),
                    hasDot(false),
                    hasExp(false),
                    expMinus(false),
                    wholePart(0),
                    fraqPart(0),
                    fraqPartLength(0),
                    expPart(0) {}
            } numberParsing;

            Stack<JSON> objectStack;
            Stack<JSON>  arrayStack;

            Stack<JSON> stackJSON;

            Error::ErrorType errorType;

            long long lineNum;
            long long lineOff;
        public:
            Parsing(ParseTree& tree) : Base(tree), lineNum(0), lineOff(0) {}
            const JSON& result() const {
                return stackJSON.top();
            }
            bool hasResult() const {
                return !stackJSON.empty();
            }
            void setErrorType(Error::ErrorType type) {
                errorType = type;
            }

            //never return ERR_OK error in error state
            Error getError() const {
                if(!inErrorState() ) return Error(Error::ERR_OK, lineNum, lineOff, getCharsRead());
                return Error( errorType==Error::ERR_OK ? Error::ERR_UNKNOWN : errorType , lineNum, lineOff, getCharsRead());
            }
            static void numberMinus(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->numberParsing.hasMinus = true;
            }
            static void numberDot(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->numberParsing.hasDot = true;
            }
            static void numberExp(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->numberParsing.hasExp = true;
            }
            static void numberWholePart(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->numberParsing.wholePart = p->getLastInt();
                p->getLastInt() = 0;
            }
            static void numberFraqPart(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->numberParsing.fraqPart = p->getLastInt();
                p->getLastInt() = 0;
            }
            static void numberFraqInc(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->numberParsing.fraqPartLength++;
            }
            static void numberExtPart(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->numberParsing.expPart = p->getLastInt();
                p->getLastInt() = 0;
            }
            static void numberEnd(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);

                if( p->numberParsing.hasExp || p->numberParsing.hasDot ) {
                    double number = (double)(p->numberParsing.hasMinus ?  -p->numberParsing.wholePart : p->numberParsing.wholePart);
                    number += p->numberParsing.fraqPart / std::pow(10,p->numberParsing.fraqPartLength);
                    if( p->numberParsing.hasExp ) {
                        number *= std::pow(10,p->numberParsing.expMinus ? -p->numberParsing.expPart : p->numberParsing.expPart);
                    }
                    p->stackJSON.push( JSON::value( 
                        number
                    ));
                } else {
                    p->stackJSON.push( JSON::value( 
                        p->numberParsing.hasMinus ?  -p->numberParsing.wholePart : p->numberParsing.wholePart 
                    ));
                }

                p->numberParsing = NumberParsing();
            }
            static void stringEnd(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->stackJSON.push( JSON::value( 
                    p->popString()
                ));
            }
            static void arrayPush(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->arrayStack.push(JSON::array());
            }
            static void arrayNext(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->arrayStack.top()(p->stackJSON.pop());
            }
            static void arrayPop(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->stackJSON.push(p->arrayStack.pop());
            }
            static void objectPush(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->objectStack.push(JSON::object());
            }
            static void objectNext(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                JSON val = p->stackJSON.pop();
                std::string key = p->stackJSON.pop().asString();
                p->objectStack.top()(key, val);
            }
            static void objectPop(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->stackJSON.push(p->objectStack.pop());
            }
            static void valueTrue(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->stackJSON.push(JSON::value(true));
            }
            static void valueFalse(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->stackJSON.push(JSON::value(false));
            }
            static void valueNull(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->stackJSON.push(JSON::null());
            }
            static void incLine(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->lineNum++;
                p->lineOff = p->getCharsRead();
            }
            static void resetLineOff(Base* parsing, Semantic::ExtentionData*) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                assert(p);
                p->lineOff = p->getCharsRead();
            }
        };

        class SetError : public Semantic::Action {
            Error::ErrorType errorType;
        public:
            SetError(Error::ErrorType type) : errorType(type) {
            }
            SetError* clone() const override {
                return new SetError(errorType);
            }
            void call(GeneralParser::Parsing* parsing) {
                Parsing* p = dynamic_cast<Parsing*>(parsing);
                p->setErrorType(errorType);
            }
        };

        void initializeParser() {
            State stringInit(this);
            State stringControlExpecting(this);
            State stringAfterStart(this);
            State stringAfterSlash(this);
            State stringAfterU(this);
            State stringEnd(this);

            stringInit >> Step::character('"') >> stringAfterStart;
            stringAfterStart >> Semantic::doAction(SetError(Error::ERR_EXPECTED_NOT_CONTROL_CHARACTER_IN_STR));
            stringAfterStart >> Step::character() >> Semantic::placeLastCharInStack() >> stringAfterStart;
            stringAfterStart >> Step::character(0,0x1f)(0x7f) >> Semantic::doAction(SetError(Error::ERR_CONTROL_CHAR_IN_STR));
            stringAfterStart >> Step::character('\xc2') >> Semantic::placeLastCharInStack() >> stringControlExpecting;
            stringControlExpecting >> Step::character('\x80','\x9f') >> Semantic::doAction(SetError(Error::ERR_CONTROL_CHAR_IN_STR));
            stringControlExpecting >> stringAfterStart;
            stringAfterStart >> Step::character('"') >> Semantic::userProc(Parsing::stringEnd) >> stringEnd;
            stringAfterStart >> Step::character('\\') >> stringAfterSlash;
            stringAfterSlash >> Semantic::doAction(SetError(Error::ERR_EXPECTED_MODIFIER_AFTER_SLASH_IN_STR));
            stringAfterSlash >> Step::character('"')  >> Semantic::placeInStack('"' ) >> stringAfterStart;
            stringAfterSlash >> Step::character('\\') >> Semantic::placeInStack('\\') >> stringAfterStart;
            stringAfterSlash >> Step::character('/')  >> Semantic::placeInStack('/' ) >> stringAfterStart;
            stringAfterSlash >> Step::character('b')  >> Semantic::placeInStack('\b') >> stringAfterStart;
            stringAfterSlash >> Step::character('f')  >> Semantic::placeInStack('\f') >> stringAfterStart;
            stringAfterSlash >> Step::character('n')  >> Semantic::placeInStack('\n') >> stringAfterStart;
            stringAfterSlash >> Step::character('r')  >> Semantic::placeInStack('\r') >> stringAfterStart;
            stringAfterSlash >> Step::character('t')  >> Semantic::placeInStack('\t') >> stringAfterStart;
            stringAfterSlash >> Step::character('u')  >> stringAfterU;
            stringAfterU >> Semantic::doAction(SetError(Error::ERR_EXPECTED_FOUR_HEX_DIGITS_AFTER_SLASH_U_IN_STR)) >>
                Step::hexDigit() >> Semantic::pushHexDigit() >> 
                Step::hexDigit() >> Semantic::pushHexDigit() >> 
                Step::hexDigit() >> Semantic::pushHexDigit() >> 
                Step::hexDigit() >> Semantic::pushHexDigit() >> Semantic::pushUnicodeChar() >>
                stringAfterStart;
            string.setStates(stringInit,stringEnd);

            State numberInit(this);
            State numberAfterMinus(this);
            State numberBeforeDot(this);
            State numberAfterDot(this);
            State numberAfterDotAndDigit(this);
            State numberAfterDigit(this);
            State numberBeforeExp(this);
            State numberAfterExp(this);
            State numberAfterExpSign(this);
            State numberAfterExpDigit(this);
            State numberEnd(this);

            numberInit       >> Step::character('-') >> Semantic::userProc(Parsing::numberMinus) >> numberAfterMinus;

            numberInit       >> Step::digit() >> Semantic::pushDigit() >> numberAfterDigit;
            numberInit       >> Step::character('0') >> numberBeforeDot;

            numberAfterMinus >> Semantic::doAction(SetError(Error::ERR_EXPECTED_DIGIT_IN_NUMBER));
            numberAfterMinus >> Step::digit() >> Semantic::pushDigit() >> numberAfterDigit;
            numberAfterMinus >> Step::character('0') >> numberBeforeDot;

            numberAfterDigit >> Step::digit() >> Semantic::pushDigit() >> numberAfterDigit;
            numberAfterDigit >> numberBeforeDot >> Semantic::userProc(Parsing::numberWholePart);

            numberBeforeDot  >> Step::character('.') >> Semantic::userProc(Parsing::numberDot) >> numberAfterDot;
            numberBeforeDot  >> numberBeforeExp;
            numberAfterDot   >> Semantic::doAction(SetError(Error::ERR_EXPECTED_DIGIT_IN_NUMBER));
            numberAfterDot   >> Step::digit() >> 
                Semantic::pushDigit() >> Semantic::userProc(Parsing::numberFraqInc) >> numberAfterDotAndDigit;
            numberAfterDotAndDigit >> Step::digit() >> 
                Semantic::pushDigit() >> Semantic::userProc(Parsing::numberFraqInc) >> numberAfterDotAndDigit;
            numberAfterDotAndDigit >> numberBeforeExp >> Semantic::userProc(Parsing::numberFraqPart);
            numberBeforeExp >> Step::character('e')('E') >> numberAfterExp >> Semantic::userProc(Parsing::numberExp);
            numberBeforeExp >> numberEnd;
            numberAfterExp  >> Semantic::doAction(SetError(Error::ERR_EXPECTED_DIGIT_OR_SIGN_AFTER_EXP_IN_NUMBER));
            numberAfterExp  >> Step::character('+') >> numberAfterExpSign;
            numberAfterExp  >> Step::character('-') >> Semantic::userProc(Parsing::numberMinus) >> numberAfterExpSign;
            numberAfterExp  >> numberAfterExpSign;
            numberAfterExpSign  >> Semantic::doAction(SetError(Error::ERR_EXPECTED_DIGIT_IN_NUMBER));
            numberAfterExpSign  >> Step::digit() >> Semantic::pushDigit() >> numberAfterExpDigit;
            numberAfterExpDigit >> Step::digit() >> Semantic::pushDigit() >> numberAfterExpDigit;
            numberAfterExpDigit >> numberEnd  >> Semantic::userProc(Parsing::numberExtPart) >> Semantic::userProc(Parsing::numberEnd) ;
            number.setStates(numberInit,numberEnd);

            State arrayInit(this);
            State arrayAfterOpen(this);
            State arrayAfterComma(this);
            State arrayAfterValue(this);
            State arrayEnd(this);

            arrayInit >> Step::character('[') >> Semantic::userProc(Parsing::arrayPush) >> ws >> arrayAfterOpen;
            arrayAfterOpen >> Semantic::doAction(SetError(Error::ERR_EXPECTED_END_OF_ARRAY_OR_ARRAY_ELEMENT));
            arrayAfterOpen >> Step::character(']') >> Semantic::userProc(Parsing::arrayPop) >> arrayEnd;
            arrayAfterOpen  >> value >> Semantic::userProc(Parsing::arrayNext) >> ws >> arrayAfterValue;
            arrayAfterComma >> value >> Semantic::userProc(Parsing::arrayNext) >> ws >> arrayAfterValue;
            arrayAfterValue >> Semantic::doAction(SetError(Error::ERR_EXPECTED_COMMA_OR_END_OF_ARRAY));
            arrayAfterValue >> Step::character(',') >> ws >> arrayAfterComma >> 
                Semantic::doAction(SetError(Error::ERR_EXPECTED_ARRAY_ELEMENT));
            arrayAfterValue >> Step::character(']') >> Semantic::userProc(Parsing::arrayPop) >> arrayEnd;
            array.setStates(arrayInit,arrayEnd);

            State objectInit(this);
            State objectAfterOpen(this);
            State objectAfterValue(this);
            State objectBeforeValue(this);
            State objectEnd(this);

            objectInit >> Step::character('{') >> Semantic::userProc(Parsing::objectPush) >> ws >> objectAfterOpen;
            objectAfterOpen >> Semantic::doAction(SetError(Error::ERR_EXPECTED_END_OF_OBJECT_OR_OBJECT_ELEMENT));
            objectAfterOpen >> Step::character('}') >> Semantic::userProc(Parsing::objectPop) >> objectEnd;
            objectAfterOpen >> objectBeforeValue;
            objectBeforeValue >> string >> ws >> Semantic::doAction(SetError(Error::ERR_EXPECTED_COLON)) >>
                Step::character(':')  >> ws >> Semantic::doAction(SetError(Error::ERR_EXPECTED_VALUE)) >>
                value >> Semantic::userProc(Parsing::objectNext) >> ws >> objectAfterValue;
            objectAfterValue >> Semantic::doAction(SetError(Error::ERR_EXPECTED_COMMA_OR_END_OF_OBJECT));
            objectAfterValue >> Step::character(',') >> ws >> 
                Semantic::doAction(SetError(Error::ERR_EXPECTED_OBJECT_ELEMENT)) >> 
                objectBeforeValue;
            objectAfterValue >> Step::character('}') >> Semantic::userProc(Parsing::objectPop) >> objectEnd;
            object.setStates(objectInit,objectEnd);

            State valueInit(this);
            State valueWs(this);
            State valueEnd(this);

            valueInit >> ws >> valueWs >> Semantic::doAction(SetError(Error::ERR_EXPECTED_VALUE));
            valueWs >> string >> valueEnd;
            valueWs >> number >> valueEnd;
            valueWs >> object >> valueEnd;
            valueWs >> array  >> valueEnd;
            valueWs >> Step::string("true" ) >> Semantic::userProc(Parsing::valueTrue ) >> valueEnd;
            valueWs >> Step::string("false") >> Semantic::userProc(Parsing::valueFalse) >> valueEnd;
            valueWs >> Step::string("null" ) >> Semantic::userProc(Parsing::valueNull ) >> valueEnd;
            value.setStates(valueInit,valueEnd);

            State wsInit(this);
            State wsAfter0A(this);
            State wsAfter0D(this);
            State wsEnd(this);

            wsInit >> Semantic::doAction(SetError(Error::ERR_EXPECTED_WHITESPACE));
            wsInit >> Step::character('\x0A') >> Semantic::userProc(Parsing::incLine) >> wsAfter0A;
            wsInit >> Step::character('\x0D') >> Semantic::userProc(Parsing::incLine) >> wsAfter0D;
            wsAfter0A >> Step::character('\x0D') >> Semantic::userProc(Parsing::resetLineOff) >> wsInit;
            wsAfter0D >> Step::character('\x0A') >> Semantic::userProc(Parsing::resetLineOff) >> wsInit;
            wsAfter0A >> wsInit;
            wsAfter0D >> wsInit;
            wsInit >> Step::string
                ("\x09")("\x0B")("\x0C")("\x20")("\xc2\x85")("\xc2\xa0")
                ("\xe1\x9a\x80")
                ("\xe2\x80\x80")("\xe2\x80\x81")("\xe2\x80\x82")("\xe2\x80\x83")("\xe2\x80\x84")("\xe2\x80\x85")
                ("\xe2\x80\x86")("\xe2\x80\x87")("\xe2\x80\x88")("\xe2\x80\x89")("\xe2\x80\x8a")("\xe2\x80\xa8")
                ("\xe2\x80\xa9")("\xe2\x80\xaf")("\xe2\x81\x9f")
                ("\xe3\x80\x90") >> wsInit;
            wsInit >> wsEnd;
            ws.setStates(wsInit,wsEnd);

            compile();
        }
    public:
        JSONparser() {
            initializeParser();
        }
        void parseInt() {
            Parsing parsing(value);
            parsing.parseString("[[\r\n[{\"a\\t\":-6-,\"\":true}],[]]]");
            std::cout<<parsing.result()<<std::endl;
        }
    };

    class ParseWrapper {
        static JSONparser& getParser() {
            static JSONparser obj;
            return obj;
        }
        //JSONparser::Parsing parsing;
    public:
        typedef JSONparser::Error Error;
		static JSON parse(const std::string& string) YUVsoft_THROW(Error) {
            const char* start = string.c_str();
            const char* end = start+string.size();

            return parse(start,end);
        }

		static JSON parse(const char* start, const char* end) YUVsoft_THROW(Error) {
            //ParseWrapper wrapper;
            JSONparser::Parsing jsonParsing(getParser().value);

            const char* parseEnd = jsonParsing.feedChars(start,end-start);

            if( parseEnd == end ) {
                jsonParsing.feedEOF();
            }

            jsonParsing.getError().raise();

            JSONparser::Parsing wsParsing(getParser().ws);
            parseEnd = wsParsing.feedChars(parseEnd,end - parseEnd);

            //TODO: wsParsing has wrong offsets
            wsParsing.getError().raise();

            if( parseEnd != end ) {
                Error err = wsParsing.getError();
                err.type = Error::ERR_EXPECTED_END_OF_FILE;
                err.raise();
            }

            wsParsing.feedEOF();
            if(!jsonParsing.hasResult() ) {
                Error err = wsParsing.getError();
                err.type = Error::ERR_UNKNOWN;
                err.raise();
            }

            return jsonParsing.result();
        }

		static JSON parse(std::istream& input) YUVsoft_THROW(Error) {
            JSONparser::Parsing jsonParsing(getParser().value);

            char next;
            while(input.get(next)) {
                jsonParsing.feedChars(&next,1);
                jsonParsing.getError().raise();
                if( jsonParsing.inFinishedState() ) break;
            }
            if( input.eof() ) {
                jsonParsing.feedEOF();
                jsonParsing.getError().raise();
            }
            if(!jsonParsing.inFinishedState() ||!jsonParsing.hasResult() ) {
                Error err = jsonParsing.getError();
                err.type = Error::ERR_UNKNOWN;
                err.raise();
            }
            
            return jsonParsing.result();
        }
    };
}
