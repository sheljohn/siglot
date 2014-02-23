#Siglot (beta): easy Signals and Slots

Siglot is a neat, easy to read/understand, single-header, STL-only, C++11 implementation of Signals and Slots (suitablle for event-programming in C++).

### About

I began Siglot thinking that signals and slots ought to be implemented simply and efficiently in C++ without calling heavy libraries like Qt, Boost or complicated headers like FastDelegates. I was surprised to find that actually, no easy standalone solution existed. I think I reached a satisfactory result with a clean 300-lines header (with comments), which includes only _two_ STL headers (standard 2011). The model follows a Pub/Sub pattern (publish & subscribe), and a single event propagation incurs only one indirection followed by a function call (scales linearly with the number of subscribers).

### Bug reports

Please send bug reports with minimal examples to the address displayed in the header preamble.

### License

<a rel="license" href="http://creativecommons.org/licenses/by-sa/3.0/deed.en_US"><img alt="Creative Commons License" style="border-width:0" src="http://i.creativecommons.org/l/by-sa/3.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-sa/3.0/deed.en_US">Creative Commons Attribution-ShareAlike 3.0 Unported License</a>.

---

## Documentation

_**Note:** due to a recent update, the example file does not cover all of the features available in this library. An extended version might be provided at a later point in time, but in the meantime, we invite users to dig into the following documentation and in the comments of the header instead, or to contact the author for more information if needed. (Feb 22nd 2014)_

### Useful Classes

Three useful classes are defined in this header:

| Class Name   | Template Parameters | Description |
|:---:|---|---|
| `Signal`     | `data_type` | Defines a signal sending data of type `data_type`. |
| `Slot`       | `data_type` | Defines a slot bound to a non-member callback function, which receives data of type `const data_type&`. |
| `MemberSlot` | `handle_type`, `data_type` | Defines a slot bound to an instance of type `handle_type`, and to a member callback function which receives data of type `const data_type&`. |

### Basic Rules

1. Event data is defined, modified and accessed via the member `data` of `Signal` instances.
1. The template arguments `data_type` default to `VoidData` (empty structure). Signals and slots with void data should be declared respectively as `Signal<> signal`, and `Slot<> slot` or `MemberSlot<handle_type> slot` ( _i.e._ omitting the template argument, but not removing the angular brackets).
1. The prototype of callback functions/methods must be `void function(const data_type& data)`, **except** with `VoidData` in which case it is simply `void function()` ( _i.e._ no input at all).
1. In this library **slots listen to signals**, meaning that connections between signals and slots are established from _slots_ interfaces (either `Slot` or `MemberSlot`), using their methods `void subscribe(signal_ptr s)` to connect, or `void unsubscribe()` to disconnect.
1. `Slot` is for non-member callback functions; `MemberSlot` only with member callback functions.
1. Slots can only be attached to signals with **same** data type.
1. Both types of slots can listen to the same `Signal` ( _i.e._ both can be stored in the signal's set).

### The `Signal` class

Signals store their subscribers (`Slot` or `MemberSlot`) in a `std::set`. That way, handling duplicates is supported without affecting iteration complexity, and specific subscriber access/deletion are logarithmic.

| Element | Description |
|---|---|
| `data_type data;` | [Member] Use to access/modify data before triggering an event. |
| `void clear();` | Clear the list of subscribers. _Complexity:_ linear in number of subscribers. |
| `unsigned count() const;` | Return current number of subscribers. _Complexity:_ constant. |
| `void invoke() const;` | Trigger all attached callback functions. _Complexity:_ linear in number of subscribers. |

Specifically, the `invoke` method loops over the set of slots and triggers the corresponding callback function for each slot. In effect, each of these trigger is equivalent to one indirection and a function call (which is optimal?).

### The `Slot` class

Slots relay a signal to a callback function. The most important method `void bind(callback_type f)` binds the instance to a non-member callback function. For example, the function `void function(const data_tyep& data)` will be bound by a `Slot<data_type> s` as `s.bind( function )`. Note that if the data type is void, the callback function should _not_ take _any_ input.

Additionally, both `Slot` and `MemberSlot` classes provide the following methods/features:
+ Bind on construct: constructor overload expecting the same inputs than the `bind` function;
+ Rebinding to another callback using `bind` again works;
+ `bool is_active() const`: tesst if we are bound to an active signal;
+ `void clear()`: detach from signal and callback.

### The `MemberSlot` class

Member slots relay a signal to a callback member function. The most important method `void bind( handle_ptr h, callback_type f )` binds the slot instance to:

+ An instance of type `handle_type` that defines the member callback method; 
+ The callback method of type `void (handle_type::*callback_type)( const data_type& data )` when `data_type` is not `VoidData`, and `void (handle_type::*callback_type)()` otherwise.

For example, if the member slot is defined as an attribute `mslot` of a class `A` that defines the method `void membercb(const data_type&  data)`, then it can be initialized from within the constructor like this: `mslot.bind(this, &A::membercb)`.

### Copying Signals and Slots

The behavior of the above classes via assignment operator or copy constructor are described here:

+ `Signal`: by default, don't copy the list of subscribed slots. Use method `void copy( const self& other )` explicitely to copy the list of subscribers.
+ `Slot`: copy constructor and the assignment operator both copy the callback function and the signal subscribed to by the other instance.
+ `MemberSlot`: the copy constructor expects **two** inputs -- a `handle_ptr` (pointer to class instance) and the other member slot `const self& other`. The assignment operator is available only if the `MemberSlot` has previously been assigned to a valid instance (cf. method `bind`).

### Examples

Examples of usage are provided and commented in the `example.cpp` source file. You can compile it using the `makefile` provided.
_**Note:** you will need g++ 4.7.2+ or another recent compiler supporting C++11 to compile these sources._
