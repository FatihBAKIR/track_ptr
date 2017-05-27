#include <utility>
#include <cstring>
#include <type_traits>
#include <cstddef>

namespace malt {
    class tracked_base;

    template<class T>
    class track_ptr;

    template<class T>
    track_ptr<T> get_ptr(T& obj);

    template<class T>
    track_ptr<const T> get_ptr(const T& obj);

    namespace detail {
        class track_ptr
        {
            friend class malt::tracked_base;

            tracked_base* obj;
            track_ptr* next;
            track_ptr* prev;

            void update_obj(std::nullptr_t)
            {
                update_obj((tracked_base*) nullptr);
            }

            template<class T>
            void update_obj(T* new_obj)
            {
                for (auto ptr = this; ptr; ptr = ptr->next) {
                    std::cout << ptr << '\n';
                    ptr->obj = new_obj;
                }
            }

            void detach()
            {
                if (prev) {
                    prev->next = next;
                }
                if (next) {
                    next->prev = prev;
                }
            }

            void move_from(track_ptr&& rhs)
            {
                std::memcpy(this, &rhs, sizeof(rhs));

                if (next) {
                    next->prev = this;
                }
                if (prev) {
                    prev->next = this;
                }

                std::memset(&rhs, 0, sizeof(rhs));
            }

            void copy_from(track_ptr& rhs)
            {
                obj = rhs.obj;
                next = rhs.next;
                prev = &rhs;

                if (next) {
                    next->prev = this;
                }

                rhs.next = this;
            }

        public:
            template <class T>
            explicit track_ptr(T* obj) : obj(obj), next{}, prev{} {}

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
                return *this;
            }

            track_ptr& operator=(track_ptr&& rhs) noexcept
            {
                detach();
                move_from(std::move(rhs));
                return *this;
            }

            bool operator==(const track_ptr& rhs) const noexcept
            {
                return rhs.obj==obj;
            }

            bool operator!=(const track_ptr& rhs) const noexcept
            {
                return rhs.obj!=obj;
            }

            bool operator==(const std::nullptr_t&) const noexcept
            {
                return nullptr==obj;
            }

            bool operator!=(const std::nullptr_t&) const noexcept
            {
                return nullptr!=obj;
            }

            ~track_ptr()
            {
                // detach from the link upon destruction
                detach();
            }

        protected:
            tracked_base* get_base_ptr()
            {
                return obj;
            }
        };
    }

    template<class T>
    class track_ptr : public detail::track_ptr
    {
        static_assert(std::is_base_of<tracked_base, T>::value, "Type does not inherit from tracked!");
    public:

        T* get() noexcept
        { return static_cast<T*>(get_base_ptr()); }

        T& operator*() noexcept
        {
            return static_cast<T*>(get_base_ptr());
        }

        T* operator->() noexcept
        {
            return static_cast<T*>(get_base_ptr());
        }

        using detail::track_ptr::operator=;
        using detail::track_ptr::operator==;
        using detail::track_ptr::operator!=;
        using detail::track_ptr::track_ptr;
    };

    class tracked_base
    {
    protected:
        mutable detail::track_ptr m_head;

        tracked_base() noexcept
            : m_head(this)
        {}

        tracked_base(const tracked_base& rhs) noexcept
            : m_head(this)
        {
        }

        tracked_base(tracked_base&& rhs) noexcept
            : m_head(std::move(rhs.m_head))
        {
            m_head.update_obj(this); // set all the pointers in the link to us
        }

        tracked_base& operator=(const tracked_base&)
        {
            m_head.update_obj(
                    nullptr); // set all the links to null, so when they try to access it, they'll presumably die
            m_head = detail::track_ptr(this);
            return *this;
        }

        tracked_base& operator=(tracked_base&& rhs)
        {
            m_head.update_obj(
                    nullptr); // set all the links to null, so when they try to access it, they'll presumably die
            m_head = std::move(rhs.m_head);
            m_head.update_obj(this); // set all the pointers in the link to us
            return *this;
        }

        ~tracked_base()
        {
            m_head.update_obj(
                    nullptr); // set all the links to null, so when they try to access it, they'll presumably die
        }
    };

    template <class T>
    track_ptr<T> get_ptr(T& obj)
    {
        return track_ptr<T>(&obj);
    }
}

