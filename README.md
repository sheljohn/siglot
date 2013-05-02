#Siglot: Signals and Slots just got simpler

Siglot is a clean, easy to read/understand, single-header, STL-only, C++ implementation of Signals and Slots.

### About

I started this little project to prove that Signals and Slots could be implemented easily and efficiently in C++ without calling heavy libraries like Qt, Boost or complicated headers like FastDelegates. I think I reached a satisfactory result with a clean 300-lines header (with comments), which includes only _one_ STL header.

### Bug reports

Please send bug reports with minimal examples to the address displayed in the header preamble. The plus sign in the e-mail is a real plus sign (I know, it's not common).

### License

For my own curiosity only, it would also be nice to know what you use this for! Send me a short message if you can; whatever you send me _will not_ be disclosed under any circumstance (except torture maybe...).

<a rel="license" href="http://creativecommons.org/licenses/by-sa/3.0/deed.en_US"><img alt="Creative Commons License" style="border-width:0" src="http://i.creativecommons.org/l/by-sa/3.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-sa/3.0/deed.en_US">Creative Commons Attribution-ShareAlike 3.0 Unported License</a>.

---

## Documentation

### Examples

Examples of usage are provided and commented in the `example.cpp` source file. You can compile it using the `makefile` provided.
_**Note:** As it is, you will need g++ 4.7.2+ or another recent compiler supporting C++11 to compile these sources. However, adapting these sources for older C++ versions is be possible with almost no effort; contact me if you need to do it, I'll gladly help._

### Useful Classes

Three useful classes are defined in this header:

| Class Name   | Template Parameters | Description |
|:---:|---|---|
| `Signal`     | `data_type` | Defines a signal sending data of type `data_type`. |
| `Slot`       | `data_type` | Defines a slot bound to a non-member callback function, which receives data of type `const data_type&`. |
| `MemberSlot` | `handle_type`, `data_type` | Defines a slot bound to an instance of type `handle_type`, and to a member callback function which receives data of type `const data_type&`. |

### Basic Rules

1. Event data should be accessed/modified via the corresponding `Signal` instance, using the member `data` of type `data_type`.
1. `data_type` defaults to `VoidData`, which is an empty `struct`. Signals and slots with void data should be declared respectively as `Signal<> signal;`, and `Slot<> slot;` or `MemberSlot<handle_type> slot;` (_i.e._ omitting the template argument, but without removing the angular brackets).
1. The prototype of callback functions/methods must be `void callback_function( const data_type& data );`, __unless__ `data_type = VoidData`, in which case it is simply `void callback_function();`.
1. In this library, __slots listen to signals__ meanning that the connections between signals and slots are made from the _slots_ interfaces (either `Slot` or `MemberSlot`), using the method `void listen_to( signal_ptr s )` to connect, or `void detach()` to disconnect.
1. `Slot`s are used only with non-member callback functions; `MemberSlot`s only with member callback functions.
1. Slots can only be attached to signals with the __same__ `data_type`.
1. Boths `Slot`s and `MemberSlot`s can listen to the same `Signal` ( _i.e._ both can be stored in the signal's set).

### The `Signal` class

A `Signal` instance stores its listeners (either `Slot`s or `MemberSlot`s) in a `std::set`. That way, handling duplicates is supported without affecting iteration complexity, and specific listener access/deletion are logarithmic in the number of listeners.

| Element | Description |
|---|---|
| `data_type data;` | [Member] Use to access/modify data before triggering an event. |
| `void clear();` | Clear the list of listeners. _Complexity:_ linear in the current number of listeners. |
| `unsigned count() const;` | Return the current number of listeners. _Complexity:_ constant. |
| `void invoke() const;` | Trigger all attached callback functions. _Complexity:_ linear in the number of listeners. |

More precisely, the `invoke` method loops over the set of slots and triggers the corresponding callback function for each slot. In effect, each of these trigger is equivalent to one indirection and a function call (which is nearly optimal?).

### The `Slot` class

A `Slot` instance can be thought of as a relay between the signal and a callback function. The most important method is `inline void bind( callback_type f );` which binds the instance to a non-member callback function of type `void (*callback_type)( const data_type& data )` when `data_type` is not `VoidData`, and `void (*callback_type)()` otherwise (hence the prototype restrictions mentionned above).

Additionally, both `Slot` and `MemberSlot` classes provide the following methods/features:
+ Binding upon construction: an instance can be constructed with the same inputs than that of the corresponding `bind` function;
+ Rebinding to another callback/instance of same type is also supported; simply use `bind` again to overwrite current settings;
+ `bool is_active() const`: tells if the slot is attached to an active signal;
+ `void clear()`: detach the slot from the corresponding signal.

### The `MemberSlot` class

A `MemberSlot` instance can be thought of as a relay between the signal and a callback method. The most important method is `inline void bind( handle_ptr h, callback_type f );` which binds the slot instance to:

+ An instance of type `handle_type` which defines the callback method; 
+ The corresponding callback method of type `void (handle_type::*callback_type)( const data_type& data )` when `data_type` is not `VoidData`, and `void (handle_type::*callback_type)()` otherwise (hence the prototype restrictions mentionned above).

See the above description of `Slot` for additional methods/features.