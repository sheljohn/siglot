#Siglot: Signals and Slots just got simpler

Siglot is a clean, easy to read/understand, standalone, single-header, STL-only, C++11 implementation of Signals and Slots.

### About

I started this little project to prove that Signals and Slots could be implemented easily and efficiently in C++11 without calling heavy libraries like Qt, Boost or complicated headers like FastDelegates. I think I reached a satisfactory result with a clean header less than 300 lines long, and STL-only.

### Bug reports

Please send bug reports with minimal examples to the address displayed in the header preamble. The plus sign in the e-mail is a real plus sign (I know, it's not common).

### License

For my own curiosity only, please send me a short message as well to tell me if you use this, and what for. Rest assured that whatever you send me will remain private and _will not_ be disclosed under any circumstance.

<a rel="license" href="http://creativecommons.org/licenses/by-sa/3.0/deed.en_US"><img alt="Creative Commons License" style="border-width:0" src="http://i.creativecommons.org/l/by-sa/3.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-sa/3.0/deed.en_US">Creative Commons Attribution-ShareAlike 3.0 Unported License</a>.

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

More precisely, the `invoke` method loops over the set of slots and triggers the corresponding callback function for each slot. In effect, each of these trigger is equivalent to one indirection and a function call (which is nearly optimal?).

### The `Slot` class

A `Slot` instance can be thought of as a relay between the signal and a callback function. The most important method is `inline void bind( callback_type f );` which binds the instance to a non-member callback function of type `void (*function)( const data_type& data )` when `data_type` is not `VoidData`, and `void (*function)()` otherwise; hence the prototype restrictions mentionned above.

Both the `Slot` and `MemberSlot` provide the following methods/features as well:
+ Binding upon construction: an instance can be constructed with the same inputs than that of the corresponding `bind` function;
+ Rebinding to another callback/instance of same type is also supported, using the method `bind` to overwrite previous data;
+ `bool is_active() const`: tells if the slot is attached to a signal;
+ `void clear()`: detach the slot from the corresponding signal.

### The `MemberSlot` class

A `MemberSlot` instance can be thought of as a relay between the signal and a callback method. The most important method is `inline void bind( handle_ptr h, callback_type f );` which binds the slot instance to:

+ An instance of type `handle_type` which defines the callback method; 
+ The corresponding callback method of type `void (handle_type::*function)( const data_type& data )` when `data_type` is not `VoidData`, and `void (handle_type::*function)()` otherwise; hence the prototype restrictions mentionned above.

See the description of `Slot`, second paragraph, for additional methods/features.