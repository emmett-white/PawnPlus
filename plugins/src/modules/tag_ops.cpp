#include "tags.h"
#include "modules/strings.h"
#include "modules/variants.h"
#include "modules/containers.h"
#include "modules/tasks.h"
#include "objects/stored_param.h"
#include "fixes/linux.h"
#include <vector>
#include <unordered_map>
#include <cmath>
#include <cstring>

using cell_string = strings::cell_string;

template <class T>
inline void hash_combine(size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct null_operations : public tag_operations
{
	cell tag_uid;

	null_operations(cell tag_uid) : tag_uid(tag_uid)
	{

	}

	virtual cell add(tag_ptr tag, cell a, cell b) const override
	{
		return 0;
	}

	virtual cell sub(tag_ptr tag, cell a, cell b) const override
	{
		return 0;
	}

	virtual cell mul(tag_ptr tag, cell a, cell b) const override
	{
		return 0;
	}

	virtual cell div(tag_ptr tag, cell a, cell b) const override
	{
		return 0;
	}

	virtual cell mod(tag_ptr tag, cell a, cell b) const override
	{
		return 0;
	}

	virtual cell neg(tag_ptr tag, cell a) const override
	{
		return 0;
	}

	virtual cell_string to_string(tag_ptr tag, cell arg) const override
	{
		cell_string str;
		if(tag->uid != tag_uid)
		{
			str.append(strings::convert(tag->name));
			str.append({':'});
		}
		append_string(tag, arg, str);
		return str;
	}

	virtual cell_string to_string(tag_ptr tag, const cell *arg, cell size) const override
	{
		cell_string str;
		if(tag->uid != tag_uid)
		{
			str.append(strings::convert(tag->name));
			str.append({':'});
		}
		str.append({'{'});
		bool first = true;
		for(cell i = 0; i < size; i++)
		{
			if(first)
			{
				first = false;
			} else {
				str.append({',', ' '});
			}
			append_string(tag, arg[i], str);
		}
		str.append({'}'});
		return str;
	}

	virtual bool equals(tag_ptr tag, cell a, cell b) const override
	{
		return a == b;
	}

	virtual bool equals(tag_ptr tag, const cell *a, const cell *b, cell size) const override
	{
		for(cell i = 0; i < size; i++)
		{
			if(!equals(tag, a[i], b[i]))
			{
				return false;
			}
		}
		return true;
	}

	virtual char format_spec(tag_ptr tag, bool arr) const override
	{
		return arr ? 'a' : 'i';
	}

	virtual bool del(tag_ptr tag, cell arg) const override
	{
		return false;
	}

	virtual bool free(tag_ptr tag, cell arg) const override
	{
		return del(tag, arg);
	}

	virtual cell copy(tag_ptr tag, cell arg) const override
	{
		return arg;
	}

	virtual cell clone(tag_ptr tag, cell arg) const override
	{
		return copy(tag, arg);
	}

	virtual size_t hash(tag_ptr tag, cell arg) const override
	{
		return std::hash<cell>()(arg);
	}

protected:
	virtual void append_string(tag_ptr tag, cell arg, cell_string &str) const
	{
		str.append(strings::convert(std::to_string(arg)));
	}
};

struct cell_operations : public null_operations
{
	cell_operations(cell tag_uid) : null_operations(tag_uid)
	{

	}

	virtual cell add(tag_ptr tag, cell a, cell b) const override
	{
		return a + b;
	}

	virtual cell sub(tag_ptr tag, cell a, cell b) const override
	{
		return a - b;
	}

	virtual cell mul(tag_ptr tag, cell a, cell b) const override
	{
		return a * b;
	}

	virtual cell div(tag_ptr tag, cell a, cell b) const override
	{
		return a / b;
	}

	virtual cell mod(tag_ptr tag, cell a, cell b) const override
	{
		return a % b;
	}

	virtual cell neg(tag_ptr tag, cell a) const override
	{
		return -a;
	}

	virtual bool equals(tag_ptr tag, cell a, cell b) const override
	{
		return a == b;
	}

	virtual bool equals(tag_ptr tag, const cell *a, const cell *b, cell size) const override
	{
		return !std::memcmp(a, b, size * sizeof(cell));
	}
};

struct bool_operations : public cell_operations
{
	bool_operations() : cell_operations(tags::tag_bool)
	{

	}

protected:
	virtual void append_string(tag_ptr tag, cell arg, cell_string &str) const override
	{
		static auto str_true = strings::convert("true");
		static auto str_false = strings::convert("false");
		static auto str_bool = strings::convert("bool:");

		if(arg == 1)
		{
			str.append(str_true);
		}else if(arg == 0)
		{
			str.append(str_false);
		}else{
			if(tag->uid == tag_uid)
			{
				str.append(str_bool);
			}
			str.append(strings::convert(std::to_string(arg)));
		}
	}
};

struct char_operations : public cell_operations
{
	char_operations() : cell_operations(tags::tag_char)
	{

	}

	virtual cell_string to_string(tag_ptr tag, cell arg) const override
	{
		cell_string str;
		append_string(tag, arg, str);
		return str;
	}

	virtual cell_string to_string(tag_ptr tag, const cell *arg, cell size) const override
	{
		cell_string str(arg, size);
		return str;
	}

	virtual char format_spec(tag_ptr tag, bool arr) const override
	{
		return arr ? 's' : 'c';
	}

protected:
	virtual void append_string(tag_ptr tag, cell arg, cell_string &str) const override
	{
		str.append({arg});
	}
};

struct float_operations : public cell_operations
{
	float_operations() : cell_operations(tags::tag_float)
	{

	}

	virtual cell add(tag_ptr tag, cell a, cell b) const override
	{
		float result = amx_ctof(a) + amx_ctof(b);
		return amx_ftoc(result);
	}

	virtual cell sub(tag_ptr tag, cell a, cell b) const override
	{
		float result = amx_ctof(a) - amx_ctof(b);
		return amx_ftoc(result);
	}

	virtual cell mul(tag_ptr tag, cell a, cell b) const override
	{
		float result = amx_ctof(a) * amx_ctof(b);
		return amx_ftoc(result);
	}

	virtual cell div(tag_ptr tag, cell a, cell b) const override
	{
		float result = amx_ctof(a) / amx_ctof(b);
		return amx_ftoc(result);
	}

	virtual cell mod(tag_ptr tag, cell a, cell b) const override
	{
		float result = std::fmod(amx_ctof(a), amx_ctof(b));
		return amx_ftoc(result);
	}

	virtual cell neg(tag_ptr tag, cell a) const override
	{
		float result = -amx_ctof(a);
		return amx_ftoc(result);
	}

	virtual bool equals(tag_ptr tag, cell a, cell b) const override
	{
		return amx_ctof(a) == amx_ctof(b);
	}

	virtual bool equals(tag_ptr tag, const cell *a, const cell *b, cell size) const override
	{
		return null_operations::equals(tag, a, b, size);
	}

	virtual char format_spec(tag_ptr tag, bool arr) const override
	{
		return arr ? 'a' : 'f';
	}

protected:
	virtual void append_string(tag_ptr tag, cell arg, cell_string &str) const override
	{
		str.append(strings::convert(std::to_string(amx_ctof(arg))));
	}
};

struct string_operations : public null_operations
{
	string_operations() : null_operations(tags::tag_string)
	{

	}

	virtual cell add(tag_ptr tag, cell a, cell b) const override
	{
		cell_string *str1, *str2;
		if((!strings::pool.get_by_id(a, str1) && str1 != nullptr) || (!strings::pool.get_by_id(b, str2) && str2 != nullptr))
		{
			return 0;
		}
		if(str1 == nullptr && str2 == nullptr)
		{
			return strings::pool.get_id(strings::pool.add(true));
		}
		if(str1 == nullptr)
		{
			return strings::pool.get_id(strings::pool.add(cell_string(*str2), true));
		}
		if(str2 == nullptr)
		{
			return strings::pool.get_id(strings::pool.add(cell_string(*str1), true));
		}

		auto str = *str1 + *str2;
		return strings::pool.get_id(strings::pool.add(std::move(str), true));
	}

	virtual cell mod(tag_ptr tag, cell a, cell b) const override
	{
		return add(tag, a, b);
	}

	virtual bool equals(tag_ptr tag, cell a, cell b) const override
	{
		cell_string *str1;
		if(!strings::pool.get_by_id(a, str1) && str1 != nullptr) return false;
		cell_string *str2;
		if(!strings::pool.get_by_id(b, str2) && str2 != nullptr) return false;

		if(str1 == nullptr && str2 == nullptr) return true;
		if(str1 == nullptr)
		{
			return str2->size() == 0;
		}
		if(str2 == nullptr)
		{
			return str1->size() == 0;
		}
		return *str1 == *str2;
	}

	virtual char format_spec(tag_ptr tag, bool arr) const override
	{
		return arr ? 'a' : 'S';
	}

	virtual bool del(tag_ptr tag, cell arg) const override
	{
		cell_string *str;
		if(strings::pool.get_by_id(arg, str))
		{
			return strings::pool.remove(str);
		}
		return false;
	}

	virtual bool free(tag_ptr tag, cell arg) const override
	{
		return del(tag, arg);
	}

	virtual cell copy(tag_ptr tag, cell arg) const override
	{
		cell_string *str;
		if(strings::pool.get_by_id(arg, str))
		{
			return strings::pool.get_id(strings::pool.clone(str));
		}
		return 0;
	}

	virtual cell clone(tag_ptr tag, cell arg) const override
	{
		return copy(tag, arg);
	}

	virtual size_t hash(tag_ptr tag, cell arg) const override
	{
		cell_string *str;
		if(strings::pool.get_by_id(arg, str))
		{
			size_t seed = 0;
			for(size_t i = 0; i < str->size(); i++)
			{
				hash_combine(seed, (*str)[i]);
			}
			return seed;
		}
		return null_operations::hash(tag, arg);
	}

protected:
	virtual void append_string(tag_ptr tag, cell arg, cell_string &str) const override
	{
		cell_string *ptr;
		if(strings::pool.get_by_id(arg, ptr))
		{
			str.append(*ptr);
		}
	}
};

struct variant_operations : public null_operations
{
	variant_operations() : null_operations(tags::tag_variant)
	{

	}

	template <dyn_object(dyn_object::*op)(const dyn_object&) const>
	cell op(tag_ptr tag, cell a, cell b) const
	{
		dyn_object *var1;
		if(!variants::pool.get_by_id(a, var1)) return 0;
		dyn_object *var2;
		if(!variants::pool.get_by_id(b, var2)) return 0;
		auto var = (var1->*op)(*var2);
		if(var.empty()) return 0;
		return variants::create(std::move(var));
	}

	virtual cell add(tag_ptr tag, cell a, cell b) const override
	{
		return op<&dyn_object::operator+>(tag, a, b);
	}

	virtual cell sub(tag_ptr tag, cell a, cell b) const override
	{
		return op<&dyn_object::operator- >(tag, a, b);
	}

	virtual cell mul(tag_ptr tag, cell a, cell b) const override
	{
		return op<&dyn_object::operator*>(tag, a, b);
	}

	virtual cell div(tag_ptr tag, cell a, cell b) const override
	{
		return op<&dyn_object::operator/>(tag, a, b);
	}

	virtual cell mod(tag_ptr tag, cell a, cell b) const override
	{
		return op<&dyn_object::operator% >(tag, a, b);
	}

	virtual cell neg(tag_ptr tag, cell a) const override
	{
		dyn_object *var;
		if(!variants::pool.get_by_id(a, var)) return 0;
		auto result = -(*var);
		if(result.empty()) return 0;
		return variants::create(std::move(result));
	}

	virtual bool equals(tag_ptr tag, cell a, cell b) const override
	{
		dyn_object *var1;
		if(!variants::pool.get_by_id(a, var1) && var1 != nullptr) return false;
		dyn_object *var2;
		if(!variants::pool.get_by_id(b, var2) && var2 != nullptr) return false;
		if(var1 == nullptr || var1->empty()) return var2 == nullptr || var2->empty();
		if(var2 == nullptr) return false;
		return *var1 == *var2;
	}

	virtual char format_spec(tag_ptr tag, bool arr) const override
	{
		return arr ? 'a' : 'V';
	}

	virtual bool del(tag_ptr tag, cell arg) const override
	{
		dyn_object *var;
		if(variants::pool.get_by_id(arg, var))
		{
			return variants::pool.remove(var);
		}
		return false;
	}

	virtual bool free(tag_ptr tag, cell arg) const override
	{
		dyn_object *var;
		if(variants::pool.get_by_id(arg, var))
		{
			var->free();
			return variants::pool.remove(var);
		}
		return false;
	}

	virtual cell copy(tag_ptr tag, cell arg) const override
	{
		dyn_object *var;
		if(variants::pool.get_by_id(arg, var))
		{
			return variants::pool.get_id(variants::pool.clone(var));
		}
		return 0;
	}

	virtual cell clone(tag_ptr tag, cell arg) const override
	{
		dyn_object *var;
		if(variants::pool.get_by_id(arg, var))
		{
			return variants::pool.get_id(variants::pool.clone(var, [](const dyn_object &obj) {return obj.clone(); }));
		}
		return 0;
	}

	virtual size_t hash(tag_ptr tag, cell arg) const override
	{
		dyn_object *var;
		if(variants::pool.get_by_id(arg, var))
		{
			return var->get_hash();
		}
		return null_operations::hash(tag, arg);
	}

protected:
	virtual void append_string(tag_ptr tag, cell arg, cell_string &str) const override
	{
		str.append({'('});
		dyn_object *var;
		if(variants::pool.get_by_id(arg, var))
		{
			str.append(var->to_string());
		}
		str.append({')'});
	}
};

struct list_operations : public null_operations
{
	list_operations() : null_operations(tags::tag_list)
	{

	}

	virtual bool equals(tag_ptr tag, cell a, cell b) const override
	{
		return a == b;
	}

	virtual char format_spec(tag_ptr tag, bool arr) const override
	{
		return arr ? 'a' : 'l';
	}

	virtual bool del(tag_ptr tag, cell arg) const override
	{
		list_t *l;
		if(list_pool.get_by_id(arg, l))
		{
			return list_pool.remove(l);
		}
		return false;
	}

	virtual bool free(tag_ptr tag, cell arg) const override
	{
		list_t *l;
		if(list_pool.get_by_id(arg, l))
		{
			for(auto &obj : *l)
			{
				obj.free();
			}
			return list_pool.remove(l);
		}
		return false;
	}

	virtual cell copy(tag_ptr tag, cell arg) const override
	{
		list_t *l;
		if(list_pool.get_by_id(arg, l))
		{
			list_t *l2 = list_pool.add().get();
			*l2 = *l;
			return list_pool.get_id(l2);
		}
		return 0;
	}

	virtual cell clone(tag_ptr tag, cell arg) const override
	{
		list_t *l;
		if(list_pool.get_by_id(arg, l))
		{
			list_t *l2 = list_pool.add().get();
			for(auto &obj : *l)
			{
				l2->push_back(obj.clone());
			}
			return list_pool.get_id(l2);
		}
		return 0;
	}
};

struct map_operations : public null_operations
{
	map_operations() : null_operations(tags::tag_map)
	{

	}

	virtual bool equals(tag_ptr tag, cell a, cell b) const override
	{
		return a == b;
	}

	virtual char format_spec(tag_ptr tag, bool arr) const override
	{
		return arr ? 'a' : 'm';
	}

	virtual bool del(tag_ptr tag, cell arg) const override
	{
		map_t *m;
		if(map_pool.get_by_id(arg, m))
		{
			return map_pool.remove(m);
		}
		return false;
	}

	virtual bool free(tag_ptr tag, cell arg) const override
	{
		map_t *m;
		if(map_pool.get_by_id(arg, m))
		{
			for(auto &pair : *m)
			{
				pair.first.free();
				pair.second.free();
			}
			return map_pool.remove(m);
		}
		return false;
	}

	virtual cell copy(tag_ptr tag, cell arg) const override
	{
		map_t *m;
		if(map_pool.get_by_id(arg, m))
		{
			map_t *m2 = map_pool.add().get();
			*m2 = *m;
			return map_pool.get_id(m2);
		}
		return 0;
	}

	virtual cell clone(tag_ptr tag, cell arg) const override
	{
		map_t *m;
		if(map_pool.get_by_id(arg, m))
		{
			map_t *m2 = map_pool.add().get();
			for(auto &pair : *m)
			{
				m2->insert(std::make_pair(pair.first.clone(), pair.second.clone()));
			}
			return map_pool.get_id(m2);
		}
		return 0;
	}
};

struct iter_operations : public null_operations
{
	iter_operations() : null_operations(tags::tag_iter)
	{

	}

	virtual bool equals(tag_ptr tag, cell a, cell b) const override
	{
		dyn_iterator *iter1;
		if(!iter_pool.get_by_id(a, iter1)) return 0;
		dyn_iterator *iter2;
		if(!iter_pool.get_by_id(b, iter2)) return 0;
		return *iter1 == *iter2;
	}

	virtual bool del(tag_ptr tag, cell arg) const override
	{
		dyn_iterator *iter;
		if(iter_pool.get_by_id(arg, iter))
		{
			return iter_pool.remove(iter);
		}
		return false;
	}

	virtual bool free(tag_ptr tag, cell arg) const override
	{
		return del(tag, arg);
	}

	virtual cell copy(tag_ptr tag, cell arg) const override
	{
		dyn_iterator *iter;
		if(iter_pool.get_by_id(arg, iter))
		{
			return iter_pool.get_id(iter_pool.clone(iter));
		}
		return 0;
	}

	virtual cell clone(tag_ptr tag, cell arg) const override
	{
		return copy(tag, arg);
	}
};

struct ref_operations : public null_operations
{
	ref_operations() : null_operations(tags::tag_ref)
	{

	}
	
protected:
	virtual void append_string(tag_ptr tag, cell arg, cell_string &str) const override
	{
		auto base = tags::find_tag(tag_uid);
		if(tag != base && tag->inherits_from(base))
		{
			auto subtag = tags::find_tag(tag->name.substr(base->name.size()+1).c_str());
			str.append(subtag->get_ops().to_string(subtag, arg));
		}else{
			null_operations::append_string(tag, arg, str);
		}
	}
};

struct task_operations : public null_operations
{
	task_operations() : null_operations(tags::tag_task)
	{

	}

	virtual bool del(tag_ptr tag, cell arg) const override
	{
		tasks::task *task;
		if(tasks::get_by_id(arg, task))
		{
			return tasks::remove(task);
		}
		return false;
	}

	virtual bool free(tag_ptr tag, cell arg) const override
	{
		return del(tag, arg);
	}

	virtual cell copy(tag_ptr tag, cell arg) const override
	{
		tasks::task *task;
		if(tasks::get_by_id(arg, task))
		{
			return tasks::remove(task);
		}
		return 0;
	}

	virtual cell clone(tag_ptr tag, cell arg) const override
	{
		return copy(tag, arg);
	}
};

auto op_map([]()
{
	std::unordered_map<cell, std::unique_ptr<tag_operations>> m;
	m.insert(std::make_pair(tags::tag_unknown, std::make_unique<null_operations>(tags::tag_unknown)));
	m.insert(std::make_pair(tags::tag_cell, std::make_unique<cell_operations>(tags::tag_cell)));
	m.insert(std::make_pair(tags::tag_bool, std::make_unique<bool_operations>()));
	m.insert(std::make_pair(tags::tag_char, std::make_unique<char_operations>()));
	m.insert(std::make_pair(tags::tag_float, std::make_unique<float_operations>()));
	m.insert(std::make_pair(tags::tag_string, std::make_unique<string_operations>()));
	m.insert(std::make_pair(tags::tag_variant, std::make_unique<variant_operations>()));
	m.insert(std::make_pair(tags::tag_list, std::make_unique<list_operations>()));
	m.insert(std::make_pair(tags::tag_map, std::make_unique<map_operations>()));
	m.insert(std::make_pair(tags::tag_iter, std::make_unique<iter_operations>()));
	m.insert(std::make_pair(tags::tag_ref, std::make_unique<ref_operations>()));
	m.insert(std::make_pair(tags::tag_task, std::make_unique<task_operations>()));
	return m;
}());

const tag_operations &tag_info::get_ops() const
{
	auto it = op_map.find(uid);
	if(it != op_map.end())
	{
		return *it->second;
	}
	if(base)
	{
		return base->get_ops();
	}
	return *op_map[tags::tag_unknown];
}

class dynamic_operations : public null_operations, public tag_control
{
	class op_handler
	{
		op_type _type;
		amx::handle _amx;
		std::vector<stored_param> _args;
		std::string _handler;

	public:
		op_handler() = default;

		op_handler(op_type type, AMX *amx, const char *handler, const char *add_format, const cell *args, int numargs)
			: _type(type), _amx(amx::load(amx)), _handler(handler)
		{
			if(add_format)
			{
				size_t argi = -1;
				size_t len = std::strlen(add_format);
				for(size_t i = 0; i < len; i++)
				{
					_args.push_back(stored_param::create(amx, add_format[i], args, argi, numargs));
				}
			}
		}

		cell invoke(tag_ptr tag, cell arg1, cell arg2) const
		{
			if(auto lock = _amx.lock())
			{
				if(lock->valid())
				{
					auto amx = lock->get();
					int pub;
					if(amx_FindPublic(amx, _handler.c_str(), &pub) == AMX_ERR_NONE)
					{
						amx_Push(amx, arg2);
						amx_Push(amx, arg1);
						for(auto it = _args.rbegin(); it != _args.rend(); it++)
						{
							it->push(amx, static_cast<int>(_type));
						}
						cell ret;
						if(amx_Exec(amx, &ret, pub) == AMX_ERR_NONE)
						{
							return ret;
						}
					}
				}
			}
			return 0;
		}

		cell invoke(tag_ptr tag, cell arg) const
		{
			if(auto lock = _amx.lock())
			{
				if(lock->valid())
				{
					auto amx = lock->get();
					int pub;
					if(amx_FindPublic(amx, _handler.c_str(), &pub) == AMX_ERR_NONE)
					{
						amx_Push(amx, arg);
						for(auto it = _args.rbegin(); it != _args.rend(); it++)
						{
							it->push(amx, static_cast<int>(_type));
						}
						cell ret;
						if(amx_Exec(amx, &ret, pub) == AMX_ERR_NONE)
						{
							return ret;
						}
					}
				}
			}
			return 0;
		}
	};

	bool _locked = false;
	std::unordered_map<op_type, op_handler> dyn_ops;

public:
	dynamic_operations(cell tag_uid) : null_operations(tag_uid)
	{

	}

	cell op_bin(tag_ptr tag, op_type op, cell a, cell b) const
	{
		auto it = dyn_ops.find(op);
		if(it != dyn_ops.end())
		{
			return it->second.invoke(tag, a, b);
		}
		return 0;
	}

	cell op_un(tag_ptr tag, op_type op, cell a) const
	{
		auto it = dyn_ops.find(op);
		if(it != dyn_ops.end())
		{
			return it->second.invoke(tag, a);
		}
		return 0;
	}

	virtual cell add(tag_ptr tag, cell a, cell b) const override
	{
		return op_bin(tag, op_type::add, a, b);
	}

	virtual cell sub(tag_ptr tag, cell a, cell b) const override
	{
		return op_bin(tag, op_type::sub, a, b);
	}

	virtual cell mul(tag_ptr tag, cell a, cell b) const override
	{
		return op_bin(tag, op_type::mul, a, b);
	}

	virtual cell div(tag_ptr tag, cell a, cell b) const override
	{
		return op_bin(tag, op_type::div, a, b);
	}

	virtual cell mod(tag_ptr tag, cell a, cell b) const override
	{
		return op_bin(tag, op_type::mod, a, b);
	}

	virtual cell neg(tag_ptr tag, cell a) const override
	{
		return op_un(tag, op_type::neg, a);
	}

	virtual bool equals(tag_ptr tag, cell a, cell b) const override
	{
		return op_bin(tag, op_type::equals, a, b);
	}

	virtual bool del(tag_ptr tag, cell arg) const override
	{
		return op_un(tag, op_type::del, arg);
	}

	virtual bool free(tag_ptr tag, cell arg) const override
	{
		return op_un(tag, op_type::free, arg);
	}

	virtual cell copy(tag_ptr tag, cell arg) const override
	{
		return op_un(tag, op_type::copy, arg);
	}

	virtual cell clone(tag_ptr tag, cell arg) const override
	{
		return op_un(tag, op_type::clone, arg);
	}

	virtual size_t hash(tag_ptr tag, cell arg) const override
	{
		return op_un(tag, op_type::hash, arg);
	}

	virtual bool set_op(op_type type, AMX *amx, const char *handler, const char *add_format, const cell *args, int numargs) override
	{
		if(_locked) return false;
		try {
			dyn_ops[type] = op_handler(type, amx, handler, add_format, args, numargs);
		}catch(std::nullptr_t)
		{
			return false;
		}
		return true;
	}

	virtual bool lock() override
	{
		if(_locked) return false;
		_locked = true;
		return true;
	}

protected:
	virtual void append_string(tag_ptr tag, cell arg, cell_string &str) const override
	{
		auto it = dyn_ops.find(op_type::string);
		if(it != dyn_ops.end())
		{
			cell result = it->second.invoke(tag, arg);
			cell_string *ptr;
			if(strings::pool.get_by_id(result, ptr))
			{
				str.append(*ptr);
			}
			return;
		}
		return null_operations::append_string(tag, arg, str);
	}
};

tag_control *tag_info::get_control() const
{
	auto it = op_map.find(uid);
	if(it != op_map.end())
	{
		return dynamic_cast<dynamic_operations*>(it->second.get());
	}
	auto op = std::make_unique<dynamic_operations>(uid);
	return static_cast<dynamic_operations*>((op_map[uid] = std::move(op)).get());
}