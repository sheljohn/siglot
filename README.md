#Siglot: Signal and Slots made simpler for everyone

Siglot is a clean, easy to read/understand, standalone, single-header, STL-only, C++11 implementation of Signals and Slots.

### About

I started this little project to prove that Signals and Slots could be implemented easily and efficiently in C++11 without calling heavy libraries like Qt, Boost or complicated headers like FastDelegates. I think I reached a satisfactory result with a clean header less than 300 lines long, and STL-only.

### Bug reports

Please send bug reports with minimal examples to the address displayed in the header preamble. The plus sign in the e-mail is a real plus sign (I know, it's not common).

### License

Creative Commons, Attribution-ShareAlike Unported 3.0
Please send me a short message as well to tell me if you use this, and for what. This is for my own curiosity only, and whatever you send me remains private and _will not_ be disclosed in any circumstance.

---

## Documentation

### Examples

Examples of usage are provided and commented in the `example.cpp` source file.

### Useful Classes

Three useful classes are defined in this header:

| Class Name   | Template Parameters | Description |
|:---:|---|---|
| `Signal`     | `data_type` | Defines a signal sending data of type `data_type`. |
| `Slot`       | `data_type` | Defines a slot bound to a non-member callback function, which receives data of type `const data_type&`. |
| `MemberSlot` | `handle_type`, `data_type` | Defines a slot bound to an instance of type `handle_type`, and to a member callback function which receives data of type `const data_type&`. |

### Basic Rules

+ Event data should be accessed/modified via the corresponding `Signal` instance, using the member `data` of type `data_type`.
+ `data_type` defaults to `VoidData`, which is an empty `struct`. Signals and slots void data should be declared respectively as `Signal<> signal;` and `Slot<> slot` or `MemberSlot<handle_type> slot;`.
+ The `Slot` class should be used only with non-member callback functions, whereas the `MemberSlot` class should be used only with member callback functions.
+ The generic signature of any callback function is `void callback_function( const data_type& data );`, __unless__ slots without data are used, in which case the prototype/signature of callback functions is simply `void callback_function();`.
+ Slots can only be attached to signals with the __same__ `data_type`.

### The `Signal` class

A `Signal` instance stores its listeners (either `Slot`s or `MemberSlot`s) in a `std::set`. That way, duplicates are handled transparently without affecting the iteration complexity, and specific listener access/deletion are logarithmic in the number of listeners.

| Element | Description |
|---|---|
| `data_type data;` | [Member] Use to access/modify data before triggering an event. |
| `void clear();` | Clear the list of listeners. _Complexity:_ linear in the current number of listeners. |
| `unsigned count() const;` | Return the current number of listeners. _Complexity:_ constant. |
| `void invoke();` | Trigger all attached callback functions. _Complexity:_ linear in the number of listeners. |

More precisely, the `invoke` method loops over the set of slots and triggers the corresponding callback function. In effect, each trigger is equivalent to one indirection and a function call.

### The `Slot` class


### The `MemberSlot` class

