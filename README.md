#Siglot: Signal and Slots made simpler for everyone

Siglot is a clean, easy to read/understand, standalone, single-header, STL-only, C++11 implementation of Signals and Slots.

### About

I started this little project to prove that Signals and Slots could be implemented easily and efficiently in C++11 without calling heavy libraries like Qt, Boost or complicated headers like FastDelegates. I think I reached a satisfactory result with a clean header less than 300 lines long, and STL-only.

### Bug reports

Please send bug reports with minimal examples to the address displayed in the header preamble. The plus sign in the e-mail is a real plus sign (I know, it's not common).

---

## Usage

### Useful Classes

Three useful classes are defined in this header:

| Class Name   | Template Parameters | Description |
|:---:|---|---|
| `Signal`     | `data_type` | Defines a signal sending data of type `data_type`. |
| `Slot`       | `data_type` | Defines a slot bound to a non-member callback function, which receives data of type `const data_type&`. |
| `MemberSlot` | `handle_type`, `data_type` | Defines a slot bound to an instance of type `handle_type`, and to a member callback function which receives data of type `const data_type&`. |

### Basic Rules

+ The event data should be accessed and modified directly via the corresponding `Signal` instance, using the member `data` of type `data_type`.
+ The default data type is `VoidData`, which is an empty `struct`. Signals and slots without data should be declared respectively as `Signal<> signal;` and `Slot<> slot` or `MemberSlot<handle_type> slot;`.
+ The `Slot` class should be used only with non-member callback functions, whereas the `MemberSlot` class should be used only with member callback functions.
+ The generic signature of any callback function is `void callback_function( const data_type& data );`, __UNLESS__ slots without data are used, in which case the prototype/signature of callback functions is simply `void callback_function();`.
+ Slots can only be attached to signals with the __same__ `data_type`.