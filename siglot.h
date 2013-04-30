#ifndef __SIGLOT__
#define __SIGLOT__

#include <set>
#include <functional>

//=============================================
// @filename     siglot.h
// @author       Sheljohn (Jonathan H)
// @contact      sh3ljohn+siglot [at] gmail
//=============================================

namespace siglot
{

/**
 * Empty data structure for Signals without data.
 */
struct VoidData {};



/**
 * The Slot interface as seen by a Signal object.
 */
template <typename T>
class CallbackInterface
{
public:

	typedef T data_type;
	typedef const data_type& data_input;

protected:

	template <typename U> friend class Signal;

	// Notify Slots if their Signal is detroyed
	virtual void disable_signal() =0;

	// Trigger the callback function
	virtual void operator() ( data_input data ) =0;
};



/**
 * The Signal interface as seen by a Slot object.
 * 
 * A Signal stores its listeners (Slots) as a set of pointers to 
 * CallbackInterfaces. This allows to avoid duplicates, and provides 
 * an efficient way of "detaching" a specific Slot.
 *
 * This interface provides the methods:
 * - attach: a new Slot is listening to this Signal
 * - detach: a specific Slot goes off
 * - count : returns the number of currently attached Slots
 */
template <typename T>
class SlotList
{
public:

	typedef CallbackInterface<T> slot_type;
	typedef slot_type* slot_ptr;

protected:

	template <typename U> friend class ListenerInterface;

	std::set<slot_ptr> slots;
	inline void attach( slot_ptr s ) { slots.insert(s); }
	inline void detach( slot_ptr s ) { slots.erase(s); }
	inline const unsigned& count() const { return slots.size(); }
};



/**
 * This interface defines the Slot-actions which are essentially related 
 * to a Signal:
 * - detach   : breaks the connection Slot-Signal
 * - listen_to: attach this Slot to the input Signal
 * ----
 * - detach_signal : delete pointer to this Slot in the Signal's set
 * - disable_signal: set the local pointer to the attached Signal to 0
 */
template <typename T>
class ListenerInterface : public CallbackInterface<T>
{
public:

	typedef T data_type;
	typedef SlotList<data_type> signal_type;
	typedef signal_type* signal_ptr;

	void detach()
		{
			detach_signal();
			disable_signal();
		}

	void listen_to( signal_ptr s )
		{
			signal = s;
			s->attach(this);
		}

protected:

	signal_ptr signal;

	inline void detach_signal()  { if ( signal ) signal->detach(this); }
	inline void disable_signal() { signal = nullptr; }
};



	/********************     **********     ********************/
	/********************     **********     ********************/



/**
 * The Signal class to use in your code.
 * The template argument corresponds to the type of the "event data structure".
 * The method "invoke" triggers the callback functions of all attached Slots.
 */
template <typename T = VoidData>
struct Signal : public SlotList<T>
{
	T data;

	~Signal() { clear(); }

	void clear()
		{ 
			for ( auto it : this->slots ) it->disable_signal();
			this->slots.clear();
		}

	void invoke()
		{
			for ( auto it : this->slots ) (*it)(data);
		}
};



/**
 * The Slot class is to be used with non-member functions callbacks:
 * - bind     : bind the slot to a callback function
 * - is_active: tells if the Slot is connected to a Signal
 *
 * NOTE:
 * The callback function must have the following signature
 * void callback_function( const T& data )
 */
template <typename T = VoidData>
class Slot : public ListenerInterface<T>
{
public:

	typedef const T& data_input;
	typedef void (*callback_type)(data_input);

	Slot() { clear(); }
	Slot( callback_type f ) { bind(f); }
	~Slot() { clear(); }

	inline void bind( callback_type f ) { callback = f; }
	inline void clear() { this->detach(); }
	inline bool is_active() const { return this->signal; }

protected:

	inline void operator() ( data_input data ) { callback(data); }
	callback_type callback;
};



/**
 * Template specialization for Signals without data.
 * To be declared as follows: Slot<> s( callback_function );
 *
 * NOTE:
 * In this case, the callback function must have the following signature:
 * void callback_function()
 */
template <>
class Slot<VoidData> : public ListenerInterface<VoidData>
{
public:

	typedef const VoidData& data_input;
	typedef void (*callback_type)();

	Slot() { clear(); }
	Slot( callback_type f ) { bind(f); }
	~Slot() { clear(); }

	inline void bind( callback_type f ) { callback = f; }
	inline void clear() { this->detach(); }
	inline bool is_active() const { return this->signal; }

protected:

	inline void operator() ( data_input data ) { callback(); }
	callback_type callback;
};



/**
 * The MemberSlot class is to be used with member-function callbacks:
 * - bind     : bind the slot to the current instance and the member callback function
 * - is_active: tells if the Slot is connected to a Signal
 *
 * NOTE: The binding function is used as follows:
 * MemberSlot<R,T> slot;
 * slot.bind(this, &R::callback_function);
 */
template <typename R, typename T = VoidData>
class MemberSlot : public ListenerInterface<T>
{
public:

	typedef R handle_type;
	typedef handle_type* handle_ptr;
	typedef const T& data_input;
	typedef void (R::*callback_type)(data_input);

	MemberSlot() { clear(); }
	MemberSlot( handle_ptr h, callback_type f ) { bind(h,f); }
	~MemberSlot() { clear(); }

	void clear()
		{
			this->detach();
			handle = nullptr;
		}

	void bind( handle_ptr h, callback_type f ) 
		{ 
			handle   = h;
			callback = f;
		}

	inline bool is_active() const { return handle && this->signal; }

protected:

	inline void operator() ( data_input data ) { (handle->*callback)(data); }
	callback_type callback;
	handle_ptr handle;
};



/**
 * Partial Template Specialization for Signals without data.
 * To be declared by omitting the second template parameter.
 *
 * NOTE: Same remark as before for the member callback function signature.
 */
template <typename R>
class MemberSlot<R,VoidData> : public ListenerInterface<VoidData>
{
public:

	typedef R handle_type;
	typedef handle_type* handle_ptr;
	typedef const VoidData& data_input;
	typedef void (R::*callback_type)();

	MemberSlot() { clear(); }
	MemberSlot( handle_ptr h, callback_type f ) { bind(h,f); }
	~MemberSlot() { clear(); }

	void clear()
		{
			this->detach();
			handle = nullptr;
		}

	void bind( handle_ptr h, callback_type f ) 
		{ 
			handle   = h;
			callback = f;
		}

	inline bool is_active() const { return handle && this->signal; }

protected:

	inline void operator() ( data_input data ) { (handle->*callback)(); }
	callback_type callback;
	handle_ptr handle;
};

}

#endif