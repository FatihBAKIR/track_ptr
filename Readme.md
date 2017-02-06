# `track_ptr`
_the zero-overhead smart pointer that's never invalid_

```cpp
std::vector<Widget> widgets(1);
widgets.front().val = "hello";

track_ptr<Widget> w_p (&widgets.front());
Widget* danger = &widgets.front();

std::cout << w_p->val << '\n'; // prints "hello"
std::cout << danger->val << '\n'; // prints "hello"

widgets.resize(1000);
widgets.front().val += " world";

std::cout << w_p->val << '\n'; // prints "hello world"
std::cout << danger->val << '\n'; // probably dies
```

**never invalid**:
+ `track_ptr`s are updated when the object they point to is moved from
+ If a tracked object is destructed before any pointer to it, those pointers point automatically to `nullptr`, not to a segmentation fault

**zero overhead**: 
+ `track_ptr`s are linked together, so no matter how many pointers point to the same object, there is no dynamic memory allocation
+ Objects of types that can be tracked store 3 pointers

---

## Usage

To allow the objects of a type `T` to be tracked, make it inherit from `tracked<T>`:

```cpp
class Widget : public tracked<Widget>
{
  ...
}
```

`tracked` is a non-polymorphic class template that notifies pointers upon moving or destruction

**warning:** make sure you call its assignment operators if you are implementing your own assignment operators like:

```cpp
Widget& Widget::operator=(Widget&& rhs) //ditto with copy assingment
{
  ...
  tracked<Widget>::operator=(std::move(rhs));
}
```

Inheriting from `tracked` exposes a single member: `get_ptr`. This method returns a `track_ptr` to the object it's called on.