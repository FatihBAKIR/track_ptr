#include <utility>
#include <cstring>
#include <type_traits>
#include <cstddef>

template <class T>
class tracked;

template <class T>
class track_ptr
{
	friend class tracked<T>;

	T* obj;
	track_ptr* next;
	track_ptr* prev;

	explicit track_ptr(T& obj) noexcept : obj {&obj}, next{}, prev{}
	{
		static_assert(std::is_base_of<tracked<T>, T>::value, "Type does not inherit from tracked!");
	}

	void update_obj(T* new_obj)
	{
		// set everyone ahead of us to the new_obj
		for (auto ptr = this; ptr; ptr = ptr->next)
		{
			ptr->obj = new_obj;
		}
	}

	void detach()
	{
		if (prev)
		{
			prev->next = next;
		}
		if (next)
		{
			next->prev = prev;
		}
	}

	void move_from(track_ptr&& rhs)
	{
		std::memcpy(this, &rhs, sizeof(rhs));

		if (next)
		{
			next->prev = this;
		}
		if (prev)
		{
			prev->next = this;
		}

		std::memset(&rhs, 0, sizeof(rhs));
	}

	void copy_from(track_ptr& rhs)
	{
		obj = rhs.obj;
		next = rhs.next;
		prev = &rhs;

		if (next)
		{
			next->prev = this;
		}

		rhs.next = this;
	}

public:

	track_ptr() noexcept : obj{}, next{}, prev{} 
	{
		static_assert(std::is_base_of<tracked<T>, T>::value, "Type does not inherit from tracked!");
	}

	explicit track_ptr(T* ptr) noexcept
	{
		move_from(ptr->get_ptr());
	}

	track_ptr(track_ptr& rhs) noexcept
	{
		copy_from(rhs);
	}

	track_ptr(track_ptr&& rhs) noexcept
	{
		move_from(std::move(rhs));
	}

	track_ptr& operator=(track_ptr& rhs) noexcept
	{
		detach();
		copy_from(rhs);
	}

	track_ptr& operator=(track_ptr&& rhs) noexcept
	{
		detach();
		move_from(std::move(rhs));
	}

	T* get() noexcept { return obj; }
	T& operator*() noexcept { return *obj; }
	T* operator->() noexcept { return obj; }

	bool operator==(const track_ptr& rhs) const noexcept
	{
		return rhs.obj == obj;
	}

	bool operator!=(const track_ptr& rhs) const noexcept
	{
		return rhs.obj != obj;
	}

	bool operator==(const nullptr_t&) const noexcept
	{
		return nullptr == obj;
	}

	bool operator!=(const nullptr_t&) const noexcept
	{
		return nullptr != obj;
	}

	~track_ptr()
	{
		// detach from the link upon destruction
		detach();
	}
};

template <class T>
class tracked
{
	// we have 2 redundant words here, maybe optimize it?
	track_ptr<T> _head;

protected:
	tracked() noexcept : _head(static_cast<T&>(*this)) {}

	/*
		copy constructing a tracked object doesn't retain any of the
		pointers to the original object
	*/
	tracked(const tracked& rhs) noexcept : _head(static_cast<T&>(*this)) {}

	tracked(tracked&& rhs) noexcept : _head(std::move(rhs._head))
	{
		_head.update_obj(static_cast<T*>(this)); // set all the pointers in the link to us
	}

	tracked& operator=(const tracked&)
	{
		_head.update_obj(nullptr); // set all the links to null, so when they try to access it, they'll presumably die
		_head = track_ptr<T>(static_cast<T&>(*this));
	}

	tracked& operator=(tracked&& rhs)
	{
		_head.update_obj(nullptr); // set all the links to null, so when they try to access it, they'll presumably die
		_head = std::move(rhs._head);
		_head.update_obj(static_cast<T*>(this)); // set all the pointers in the link to us
	}

	~tracked()
	{
		_head.update_obj(nullptr); // set all the links to null, so when they try to access it, they'll presumably die
	}

public:

	/*
		get a tracking pointer to this object
	*/
	track_ptr<T> get_ptr() noexcept
	{
		return _head; // creates a copy of the head, which adds a new link
	}
};
